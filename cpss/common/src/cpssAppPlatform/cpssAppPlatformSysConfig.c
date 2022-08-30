/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file cpssAppPlatformSysConfig.c
*
* @brief CPSS Application platform System initialization function
*
* @version   1
********************************************************************************
*/

#include <profiles/cpssAppPlatformProfile.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpssAppPlatformSysConfig.h>
#include <cpssAppPlatformBoardConfig.h>
#include <cpssAppPlatformPpConfig.h>
#include <cpssAppPlatformRunTimeConfig.h>
#include <cpssAppPlatformLogLib.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>
#include <ezbringup/cpssAppPlatformEzBringupTools.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/cpssDxChBrgFdbManager.h>

extern GT_BOOL eventHandlerInitDone;
extern GT_BOOL portMgr;

GT_32 isWarmboot = 0;
GT_U32 multiProcessAppDemo = 0;
/*
* @internal cpssAppPlatformSystemInit function
* @endinternal
*
* @brief   configure board, PP and runtime modules based on input profiles.
*
* @param[in] inputProfileListPtr - Input profile list containing board profile,
*                                  Pp profile and runTime profile.
*                                  Note: Pp and runTime profiles are optional.
* @param[in] systemRecovery      - system recovery mode.
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if input profile is not board profile.
* @retval GT_FAIL                - otherwise.
*/

GT_STATUS cpssAppPlatformSystemInit
(
    IN CPSS_APP_PLATFORM_PROFILE_STC *inputProfileListPtr,
    IN CPSS_SYSTEM_RECOVERY_INFO_STC *systemRecovery
)
{
    GT_STATUS rc = GT_OK;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if(inputProfileListPtr == NULL)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Input Profile list is NULL\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssAppPlatformBoardInit(inputProfileListPtr, systemRecovery);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformBoardInit);

    rc = cpssAppPlatformSystemPpAdd(inputProfileListPtr, systemRecovery);
    if(rc == GT_BAD_PARAM)
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("No PP Profile - Skipping PP Add. Setting rc = GT_OK\n");
        rc = GT_OK;
    }

    return rc;
}

GT_STATUS capSysInit
(
    IN GT_CHAR* profileName
)
{
    GT_STATUS rc = GT_FAIL;
    CPSS_APP_PLATFORM_PROFILE_STC *profilePtr = NULL;

    rc = cpssAppPlatformProfileGet(profileName, &profilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformProfileGet);

    rc = cpssAppPlatformSystemInit(profilePtr, NULL);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformSystemInit);

    return rc;
}

/*
* @internal cpssAppPlatformSystemPpAdd function
* @endinternal
*
* @brief   configure a new PP and runTime modules(if present in input profile).
*
* @param[in] inputProfileListPtr - Input profile list containing Pp profile,
*                                  runTime profile and subset of board profile
*                                  for PP mapping.
*                                  Note 1: Each valid PP profile will be mapped
*                                  to next board PP map profile in inputProfileListPtr.
*                                  Pp profiles without such PP map profile will not
*                                  be considered for initialization.
*                                  Note 2: runTime profile is optional.
* @param[in] systemRecovery      - system recovery mode.
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if input profile is not Pp profile.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformSystemPpAdd
(
    IN CPSS_APP_PLATFORM_PROFILE_STC  *inputProfileListPtr,
    IN CPSS_SYSTEM_RECOVERY_INFO_STC  *systemRecovery
)
{
    GT_STATUS                          rc             = GT_FAIL;
    GT_BOOL                            ppProfileFound = GT_FALSE;
    CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC *boardPpMapPtr  = NULL;
    CPSS_APP_PLATFORM_PP_PROFILE_STC  *ppProfilePtr   = NULL;
    CPSS_APP_PLATFORM_PROFILE_STC     *profileListPtr = inputProfileListPtr;

    (void)systemRecovery;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if(profileListPtr == NULL)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Input Profile list is NULL\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_APP_START_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_PP_E)
    {
        if(profileListPtr->profileValue.ppInfoPtr == NULL)
        {
            /* PP profile is Empty. proceed to next profile */
            continue;
        }
        ppProfilePtr = profileListPtr->profileValue.ppInfoPtr;

        /* For each valid PP profile, next profile should be Board PP map profile. */
        profileListPtr++;
        if((profileListPtr->profileType != CPSS_APP_PLATFORM_PROFILE_TYPE_BOARD_E) ||
          (profileListPtr->profileValue.boardInfoPtr == NULL))
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("Board PP_MAP profile is not found after PP profile.\n \
                                             So moving on to next PP profile on the list\n");
            continue;
        }
        if((profileListPtr->profileValue.boardInfoPtr->boardInfoType != CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_E) ||
           (profileListPtr->profileValue.boardInfoPtr->boardParam.ppMapPtr == NULL))
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("Board PP_MAP profile is not found after PP profile.\n \
                                             So moving on to next PP profile on the list\n");
            continue;
        }
        boardPpMapPtr = profileListPtr->profileValue.boardInfoPtr->boardParam.ppMapPtr;
        /* Here both ppProfilePtr and boardPpMapPtr are valid */
        ppProfileFound = GT_TRUE;

        rc = cpssAppPlatformPpInsert(boardPpMapPtr->devNum, boardPpMapPtr, ppProfilePtr, systemRecovery);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPpInsert);

        rc = cpssAppPlatformProfileDbAdd(boardPpMapPtr->devNum, inputProfileListPtr, CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_PP_E);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformProfileDbAdd);

    }
    PRV_CPSS_APP_END_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_PP_E)

    PRV_CPSS_APP_START_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_PP_E)
    {
        if(profileListPtr->profileValue.ppInfoPtr == NULL)
        {
            /* PP profile is Empty. proceed to next profile */
            continue;
        }
        ppProfilePtr = profileListPtr->profileValue.ppInfoPtr;
        /*****************************************************/
        /* Cascade ports configuration                       */
        /*****************************************************/
        rc = cpssAppPlatformCascadeInit(ppProfilePtr->cascadePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformCascadeInit);
    }
    PRV_CPSS_APP_END_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_PP_E)

    if(ppProfileFound == GT_FALSE)
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("PP profile not found\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssAppPlatformSystemRun(inputProfileListPtr);
    if(rc == GT_BAD_PARAM)
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("No Run time Profile - Skipping run time config\n");
        return GT_OK;
    }

    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformSystemRun);

    return rc;
}

/*
* @internal cpssAppPlatformSystemHaStateSet function
* @endinternal
*
* @brief   Set Ha state.
*
* @param[in] haState             - CPSS device number of PP that has to be removed.
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if haState > 3.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformSystemHaStateSet
(
    IN GT_U8   haState
)
{
    GT_STATUS                         rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC     system_recovery;


    cpssOsMemSet(&system_recovery,0,sizeof(system_recovery));
    switch (haState)
    {
    case 0:
        system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
        system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
        break;
    case 1:
    system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
        break;
    case 2:
        system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E;
        system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
        break;
    case 3:
        system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
        system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    rc =  cpssSystemRecoveryStateSet(&system_recovery);
    if (rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssSystemRecoveryStateSet ret=%d\r\n", rc);
    }
    return rc;
}

/*
* @internal cpssAppPlatformSystemHaSyncMgr function
* @endinternal
*
* @brief   Set Ha state.
*
* @param[in] fdbMgrId            - fdb manager id.
*
* @retval GT_OK                  - on success.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformSystemHaSyncMgr
(
    IN GT_U32   fdbMgrId
)
{

    GT_STATUS                         rc;
    rc =  cpssDxChBrgFdbManagerHighAvailabilityEntriesSync(fdbMgrId);
    if (rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssSystemRecoveryStateSet ret=%d\r\n", rc);
    }
    return rc;

}


/*
* @internal cpssAppPlatformSystemPpRemove function
* @endinternal
*
* @brief   removes a PP device for chassis support.
*
* @param[in] devNum              - CPSS device number of PP that has to be removed.
* @param[in] removalType         - Removal type: 0 - Managed Removal,
*                                                1 - Unmanaged Removal,
*                                                2 - Managed Reset
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if devNum > 31 or device not present.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformSystemPpRemove
(
    IN GT_U8  devNum,
    IN CPSS_APP_PLATFORM_PP_REMOVAL_TYPE_ENT removalType
)
{
    GT_STATUS rc;
    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    rc = cpssAppPlatformPpRemove(devNum, removalType);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPpRemove);

    return rc;
}

extern GT_STATUS cpssAppRefEventFuncInit
(
    GT_U8  devNum
);

/*
* @internal cpssAppPlatformSystemRun function
* @endinternal
*
* @brief   configure run time modules and enable traffic.
*
* @param[in] inputProfileListPtr - Input profile list containing run time profile.
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if input profile is not run time profile.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformSystemRun
(
    IN CPSS_APP_PLATFORM_PROFILE_STC *inputProfileListPtr
)
{
    GT_STATUS                              rc = GT_OK;
    GT_BOOL                                rtProfileFound = GT_FALSE;
    CPSS_APP_PLATFORM_PROFILE_STC         *profileListPtr = inputProfileListPtr;
    CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC *runTimeProfilePtr = NULL;
    CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC  *trafficEnablePtr = NULL;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery;
    GT_U32 devNum;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if(profileListPtr == NULL)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Input Profile list is NULL\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_APP_START_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E)
    {
        rtProfileFound = GT_TRUE;
        break;
    }
    PRV_CPSS_APP_END_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E)

    if(rtProfileFound == GT_FALSE)
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        /* Event Handler are common for all devices, so created only once */
        if(eventHandlerInitDone == GT_FALSE)
        {
            rc = cpssAppPlatformUserEventsInit(inputProfileListPtr);
            if(rc != GT_OK)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatformUserEventsInit ret=%d\r\n", rc);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
    }

    /* Port Manager Init */
    if(portMgr == GT_FALSE)
    {
        rc = cpssAppPlatformPortManagerInit(inputProfileListPtr);
        if(rc != GT_OK)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatformPortManagerInit ret=%d\r\n", rc);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    profileListPtr = inputProfileListPtr;
    PRV_CPSS_APP_START_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E)
    {

        if(profileListPtr->profileValue.runTimeInfoPtr == NULL)
        {
            /* RunTime profile is Empty. proceed to next profile */
            continue;
        }

        runTimeProfilePtr = profileListPtr->profileValue.runTimeInfoPtr;
        PRV_CPSS_APP_START_LOOP_RUNTIME_PROFILE_MAC(runTimeProfilePtr, CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E)
        {
            trafficEnablePtr = runTimeProfilePtr->runtimeInfoValue.trafficEnablePtr;
            if(trafficEnablePtr == NULL)
            {
                /* traffic enable handle is NULL. proceed to next profile */
                continue;
            }
            rc = cpssAppPlatformTrafficEnable(trafficEnablePtr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformTrafficEnable);
        }
        PRV_CPSS_APP_END_LOOP_RUNTIME_PROFILE_MAC(runTimeProfilePtr, CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E)
    }
    PRV_CPSS_APP_END_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E)

    rc = cpssAppPlatformProfileDbAdd( 0, inputProfileListPtr, CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_RT_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformProfileDbAdd);
    if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        for(devNum = 0; devNum < CPSS_APP_PLATFORM_MAX_PP_CNS; devNum++)
        {
            if(PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
            {
                rc = cpssAppRefEventFuncInit(devNum);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppRefEventFuncInit);
            }
        }
    }

    return rc;
}

/**
* @internal cpssAppPlatformCascadeInit function
* @endinternal
*
* @brief   Configure cascade ports (if they are present)
*
* @param[in] dev                      -   The PP's device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
* @note This function is called from cpssAppPlatformPpGeneralInit
*
*/
GT_STATUS cpssAppPlatformCascadeInit
(
    IN  CPSS_APP_PLATFORM_PP_PARAM_CASCADE_STC  *cascadePtr
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U8                       devNum = cascadePtr->srcDevNum;
    GT_U32                      ii;                     /* Loop index.*/
    GT_U32                      portIdx;                /* Loop index.*/
    GT_TRUNK_ID                 trunkId;                /* the cascade trunkId */
    GT_U8                       sourceDevNum;           /* the device to be reached via cascade port for analyzing */
    GT_HW_DEV_NUM               sourceHwDevNum;         /* the HW num for device to be reached via cascade port for analyzing */
    GT_U8                       targetDevNum;           /* the device to be reached via cascade port for analyzing */
    GT_HW_DEV_NUM               targetHwDevNum;         /* the HW num for device to be reached via cascade port for analyzing */
    GT_U8                       portNum;
    CPSS_CSCD_PORT_TYPE_ENT     cscdPortType;           /* Enum of cascade port or network port */
    CPSS_CSCD_LINK_TYPE_STC     cpssCascadeLink;
    CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT
                                currentSrcPortTrunkHashEn;
    GT_BOOL                     egressAttributesLocallyEn;
    CPSS_PORTS_BMP_STC          cscdTrunkBmp;           /* bmp of ports members in the cascade trunk */
    GT_U32                      stpId;
    GT_U32                      maxStp;

    if(cascadePtr == NULL)
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("Cascade Init - Skipping: No Cascade Profile found\n");
        return GT_OK;
    }
    else
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("Cascade Init\n");
    }

    /* 1. Initialize the cascade library */

    rc = cpssDxChCfgTableNumEntriesGet(0, CPSS_DXCH_CFG_TABLE_STG_E, &maxStp);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgTableNumEntriesGet);

    /* clear the local ports bmp for the new cascade trunk */
    PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&cscdTrunkBmp);
    for(ii = 0; ii < cascadePtr->numCscdTrunks; ++ii)
    {
        cscdPortType = cascadePtr->trunkInfo[ii].cscdPortType;
        /*2.1. Configure ports to be cascade ports*/
        for(portIdx = 0; portIdx< cascadePtr->trunkInfo[ii].numCscdPorts; ++portIdx)
        {
            portNum = cascadePtr->trunkInfo[ii].portList[portIdx];

            rc = cpssDxChCscdPortTypeSet(devNum, portNum,CPSS_PORT_DIRECTION_BOTH_E, cscdPortType);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCscdPortTypeSet);

            /* enable/disable policy */
            rc = cpssDxChPclPortIngressPolicyEnable(devNum, portNum, GT_FALSE);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPclPortIngressPolicyEnable);

            /* disable learning of new source MAC addresses for packets received
               on specified port*/
            rc = cpssDxChBrgFdbPortLearnStatusSet(devNum,portNum,GT_FALSE,CPSS_LOCK_FRWRD_E);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbPortLearnStatusSet);

            /* Disable New Address messages Storm Prevention */
            rc = cpssDxChBrgFdbNaStormPreventSet(devNum,portNum,GT_FALSE);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbNaStormPreventSet);

            /* set the "send to CPU AU" from this port -- fix bug #27827 */
            rc = cpssDxChBrgFdbNaToCpuPerPortSet(devNum, portNum, GT_FALSE);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbNaToCpuPerPortSet);

            for(stpId=0; stpId < maxStp; stpId++)
            {
                rc = cpssDxChBrgStpStateSet(devNum, portIdx, stpId, CPSS_STP_DISABLED_E);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgStpStateSet);
            }
            /* Configure ports to be cascade ports */
            CPSS_PORTS_BMP_PORT_SET_MAC(&cscdTrunkBmp,portNum);
        }

        trunkId = cascadePtr->trunkInfo[ii].trunkId;
        rc = cpssDxChTrunkCascadeTrunkPortsSet(devNum,trunkId,&cscdTrunkBmp);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTrunkCascadeTrunkPortsSet);

        targetDevNum = cascadePtr->trgtDevNum;
        sourceDevNum = devNum;

        rc = cpssDxChCfgHwDevNumGet(targetDevNum, &targetHwDevNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgHwDevNumGet);

        rc = cpssDxChCfgHwDevNumGet(sourceDevNum, &sourceHwDevNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgHwDevNumGet);

        rc = cpssDxChCscdDevMapTableGet(devNum,targetHwDevNum,
            sourceHwDevNum, 0, 0, &cpssCascadeLink,
            &currentSrcPortTrunkHashEn, &egressAttributesLocallyEn);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCscdDevMapTableGet);

        cpssCascadeLink.linkNum = trunkId;
        cpssCascadeLink.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
        currentSrcPortTrunkHashEn = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;
        egressAttributesLocallyEn = GT_FALSE;

        rc = cpssDxChCscdDevMapTableSet(devNum,targetHwDevNum,
            sourceHwDevNum, 0, 0, &cpssCascadeLink,
            currentSrcPortTrunkHashEn, egressAttributesLocallyEn);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCscdDevMapTableSet);
    }

    return rc;
}

