/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssDxChHitlessStartup.c
*
* @brief CPSS DxCh hitless startup facility API.
*
* @version   2
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/hitlessStartup/cpssDxChHitlessStartup.h>
#include <cpss/generic/version/cpssGenStream.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/private/prvCpssDxChSystemRecoveryLog.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManagerSamples.h>

extern GT_STATUS cpssGetMaxPortNumber
(
    IN   GT_U8 devNum,
    OUT  GT_U16 *maxPortNumPtr
);

extern GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPortDown
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum
);

typedef struct{
    GT_CHAR*   miFwVersion;
    GT_CHAR*   cpssVersion;
}VERSION_SUPPORT_STC;
static const VERSION_SUPPORT_STC    version_arr[] = {
     /*miFwVersion*/    /*cpssVersion*/
    {"1.4.3"            ,"CPSS 4.3.3"},
    {"1.6.4"            ,"CPSS 4.3.5"},
    {"1.6.5"            ,"CPSS 4.3.5"},
    /*must be last*/
    {NULL,NULL}
};

extern GT_STATUS mvHwsAnpEmulatorOwHCDResolution
(
    GT_U8                devNum,
    GT_U32               portNum,
    MV_HWS_PORT_STANDARD portMode
);


/**
* @internal internal_cpssDxChHitlessStartupMiCompatibilityCheck function
* @endinternal
*
* @brief  This function gets micro init version and check if compatible with cpss version
*
* @note   APPLICABLE DEVICES:       AC5P;AC5X;HARRIER;
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon .
*
* @param[in]  devNum               - device number
* @param[in]  miVersionPtr         - (pointer to) MI version .
* @param[OUT] isCompatiblePtr      - (pointer to) if MI and cpss version are compatible .
*
* @retval GT_OK                      - on success
* @retval GT_FAIL                    - on case of error
* @retval GT_NOT_APPLICABLE_DEVICE   - not applicable device
* @retval GT_BAD_PARAM               - on wrong input
*
*/
GT_STATUS internal_cpssDxChHitlessStartupMiCompatibilityCheck
(
    IN   GT_U8                     devNum,
    IN   GT_CHAR                  *miVersionPtr,
    OUT  GT_BOOL                  *isCompatiblePtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_VERSION_INFO_STC superImageVersion,miVersion,bootonVersion,cpssVersion;
    GT_BOOL compatible =GT_FALSE;
    GT_U32  ii,verLen;

    CPSS_NULL_PTR_CHECK_MAC(isCompatiblePtr);

    if ( (miVersionPtr == NULL ) || (miVersionPtr[0] == 0) )
    {
        PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
        PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_HARRIER_E | CPSS_AC5P_E | CPSS_AC5X_E);


        /*no given version , need to get it from MI*/
        rc = prvCpssDxChBootChannelHandlerVersionGet(devNum, &superImageVersion, &miVersion, &bootonVersion);
        if (rc != GT_OK)
        {
            return rc;
        }
        CPSS_LOG_INFORMATION_MAC("got info from MI: \n superImageVersion %s .\nmiVersion %s .\nbootonVersion %s .\n",
                     superImageVersion.version,miVersion.version,bootonVersion.version);
    }
    else
    {
        /*copy string from input into local param*/
        cpssOsMemCpy(miVersion.version, miVersionPtr, sizeof(CPSS_VERSION_INFO_STC));
    }

    /*get cpss version*/
    rc = cpssDxChVersionGet(&cpssVersion);
    if (rc != GT_OK)
    {
        return rc;
    }
    CPSS_LOG_INFORMATION_MAC("\n cpssVersion %s .\n", cpssVersion.version);
    CPSS_LOG_INFORMATION_MAC("miVersionPtr %s .\n", miVersion.version);

    for(ii = 0 ; version_arr[ii].miFwVersion ; ii++)
    {
        verLen = cpssOsStrlen(version_arr[ii].miFwVersion);
        /* check MI version prefix */
        if (cpssOsStrNCmp((GT_CHAR*)miVersion.version,version_arr[ii].miFwVersion,verLen) == 0)
        {
            /* check CPSS version 'starting' supporting version */
            if (cpssOsStrCmp((GT_CHAR*)cpssVersion.version,version_arr[ii].cpssVersion) >= 0)
            {
                CPSS_LOG_INFORMATION_MAC("versions compatible ! for FW[%s] (prefix[%s]) with CPSS[%s] >= [%s] \n",
                    miVersion.version,
                    version_arr[ii].miFwVersion ,
                    cpssVersion.version ,
                    version_arr[ii].cpssVersion);

                compatible=GT_TRUE;
                break;
            }
        }
    }

    *isCompatiblePtr = compatible;

    return GT_OK;
}

/**
* @internal cpssDxChHitlessStartupMiCompatibilityCheck function
* @endinternal
*
* @brief  This function gets micro init version and check if compatible with cpss version
*
* @note   APPLICABLE DEVICES:       AC5P;AC5X;HARRIER;
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon .
*
* @param[in]  devNum               - device number
* @param[in]  miVersionPtr         - (pointer to) MI version .
* @param[OUT] isCompatiblePtr      - (pointer to) if MI and cpss version are compatible .
*
* @retval GT_OK                      - on success
* @retval GT_FAIL                    - on case of error
* @retval GT_NOT_APPLICABLE_DEVICE   - not applicable device
* @retval GT_BAD_PARAM               - on wrong input
*
*/
GT_STATUS cpssDxChHitlessStartupMiCompatibilityCheck
(
    IN   GT_U8        devNum,
    IN   GT_CHAR     *miVersionPtr,
    OUT  GT_BOOL     *isCompatiblePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHitlessStartupMiCompatibilityCheck);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum,miVersionPtr, isCompatiblePtr));

    rc = internal_cpssDxChHitlessStartupMiCompatibilityCheck(devNum,miVersionPtr, isCompatiblePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId,rc,devNum,miVersionPtr, isCompatiblePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHitlessStartupPortParamsGet function
* @endinternal
*
* @brief  This function gets link Status and port's params of the port configured by MI.
*
* @note   APPLICABLE DEVICES:       AC5P;AC5X;HARRIER;
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon .
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[out] portInfoPtr             - (pointer to) port info stc
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_SUPPORTED         - on not supported port mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChHitlessStartupPortParamsGet
(
    IN  GT_U8                                  devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum,
    OUT PRV_CPSS_BOOT_CH_PORT_STATUS_STC       *portInfoPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_SYSTEM_RECOVERY_INFO_STC systemRecovery_temp;

    rc = cpssSystemRecoveryStateGet(&systemRecovery_temp);
    if (rc != GT_OK)
    {
      return rc;
    }

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_HARRIER_E | CPSS_AC5P_E | CPSS_AC5X_E);

    if (systemRecovery_temp.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"cpssDxChHitlessStartupSyncPortManager must be called only under HS\n");
    }
    rc = prvCpssDxChBootChannelHandlerPortStatusGet(devNum, portNum, portInfoPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return rc ;
}


/**
* @internal cpssDxChHitlessStartupPortParamsGet function
* @endinternal
*
* @brief  This function gets link Status and port's params of the port configured by MI.
*
* @note   APPLICABLE DEVICES:       AC5P;AC5X;HARRIER;
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon .
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[out] portInfoPtr             - (pointer to) port info stc
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_SUPPORTED         - on not supported port mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHitlessStartupPortParamsGet
(
    IN  GT_U8                                  devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum,
    OUT PRV_CPSS_BOOT_CH_PORT_STATUS_STC       *portInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHitlessStartupPortParamsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum,portNum, portInfoPtr));

    rc = internal_cpssDxChHitlessStartupPortParamsGet(devNum,portNum, portInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId,rc,devNum,portNum, portInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHitlessStartupSyncPortManager function
* @endinternal
*
* @brief  This function sync port manager DB using data from MI.
*
* @note   APPLICABLE DEVICES:       AC5P;AC5X;HARRIER;
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon .
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - in case of error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS internal_cpssDxChHitlessStartupSyncPortManager
(
    IN  GT_U8                                  devNum
)
{
    GT_STATUS rc = GT_OK;

    GT_PHYSICAL_PORT_NUM                  portNum;
    CPSS_PORT_MANAGER_STC                 portEventStc;
    CPSS_SYSTEM_RECOVERY_INFO_STC         oldSystemRecoveryInfo,newSystemRecoveryInfo;
    GT_U16                                portMacNum; /* MAC number */
    PRV_CPSS_BOOT_CH_PORT_STATUS_STC      portInfo ;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_HARRIER_E | CPSS_AC5P_E | CPSS_AC5X_E);

    rc = cpssSystemRecoveryStateGet(&oldSystemRecoveryInfo);
    if (rc != GT_OK)
    {
      return rc;
    }
    if (oldSystemRecoveryInfo.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"cpssDxChHitlessStartupSyncPortManager must be called only under HS\n");
    }

    newSystemRecoveryInfo = oldSystemRecoveryInfo;
    /*sync port manager as in HA mode -HW write disable*/
    newSystemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;

    for (portMacNum = 0; portMacNum < PRV_CPSS_PP_MAC(devNum)->numOfPorts; portMacNum++)
    {
        PRV_CPSS_SKIP_NOT_EXIST_PORT_MAC(devNum,portMacNum);

        /* convert MAC number to physical port number */
        rc = prvCpssDxChPortPhysicalPortMapReverseMappingGet(devNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E,
            portMacNum, &portNum);
        if(GT_OK != rc)
        {
            return rc;
        }

        rc = cpssDxChHitlessStartupPortParamsGet(devNum, portNum, &portInfo);
        if (rc != GT_OK)
        {
            if (rc == GT_NOT_FOUND)
            {
                /*clear pizza resources configure by static configuration in MI */
                rc = prvCpssDxChTxPortSpeedPizzaResourcesPortDown(devNum, portNum);
                if (rc != GT_OK)
                {
                        return rc;
                }
                CPSS_LOG_INFORMATION_MAC("\n port %d (mac %d) is not configured by mi - clear HW  ",portNum,portMacNum);
                continue; /*port is not configured by mi - skip*/
            }
            else
            {
                return rc;
            }
        }
        CPSS_LOG_INFORMATION_MAC("\n port %d (mac %d) is configured by mi - continue \n ",portNum,portMacNum);
        CPSS_LOG_INFORMATION_MAC("portNum %d  isLinkUpPtr %d .ifModePtr %d  speedPtr %d fecMode %d apMode %d.\n",portNum,
                     portInfo.isLinkUp,portInfo.ifMode,portInfo.speed,portInfo.fecMode,portInfo.apMode);

        if (portInfo.apMode == GT_FALSE)
        {
            prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);
            rc = cpssDxChSamplePortManagerMandatoryParamsSet(devNum, portNum,portInfo.ifMode, portInfo.speed, portInfo.fecMode);
            if (GT_OK != rc)
            {
                prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"cpssDxChSamplePortManagerMandatoryParamsSet FAILED:rc-%d,portNum-%d,ifMode-%d,speed-%d fecMode %d\n",
                                                        rc, portNum, portInfo.ifMode, portInfo.speed,portInfo.fecMode);
            }
        }
        else /*AP mode*/
        {
            if(CPSS_DEV_IS_WM_NATIVE_OR_ASIM_MAC(devNum))
            {
                MV_HWS_PORT_STANDARD portMode;
                rc = prvCpssCommonPortIfModeToHwsTranslate(devNum, portInfo.ifMode, portInfo.speed, &portMode);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssCommonPortIfModeToHwsTranslate - failed to translate port mode");
                }
                /* fill more info that the MI of WM did not have all this intelligence */
                CHECK_STATUS(mvHwsAnpEmulatorOwHCDResolution(devNum, portMacNum, portMode));
            }
            if (portInfo.isLinkUp == GT_FALSE)
            {
                CPSS_LOG_INFORMATION_MAC("\nap port in link down - clear pizza   ");
                /*clear pizza resources configure by static configuration in MI
                  pizza reconfigure when ap port link up */
                rc = prvCpssDxChTxPortSpeedPizzaResourcesPortDown(devNum, portNum);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);
            rc = cpssDxChSamplePortManagerApMandatoryParamsSet(devNum, portNum, portInfo.ifMode, portInfo.speed,
                                                               portInfo.fecMode, portInfo.fecMode);
                if (GT_OK != rc)
                {
                    prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"cpssDxChSamplePortManagerApMandatoryParamsSet FAILED:rc-%d,portNum-%d,ifMode-%d,speed-%d fecMode %d\n",
                                                            rc, portNum, portInfo.ifMode, portInfo.speed,portInfo.fecMode);
                }
        }
        /*recreate port*/
        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
        rc = cpssDxChPortManagerEventSet(devNum,portNum, &portEventStc);
        if (rc !=GT_OK)
        {
            prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"FAIL to recreate port %d\n",portNum);
        }
        prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
    }

    return rc ;
}


/**
* @internal cpssDxChHitlessStartupSyncPortManager function
* @endinternal
*
* @brief  This function sync port manager DB using data from MI.
*
* @note   APPLICABLE DEVICES:       AC5P;AC5X;HARRIER;
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon .
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - in case of error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChHitlessStartupSyncPortManager
(
    IN  GT_U8                                  devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,cpssDxChHitlessStartupSyncPortManager);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChHitlessStartupSyncPortManager(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId,rc,devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

