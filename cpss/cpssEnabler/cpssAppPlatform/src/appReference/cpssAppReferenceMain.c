/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file cpssAppPlatformReferenceMain.c
*
* @brief APIs/Entry point for Cpss application platform reference code
*
*
* @version   1
********************************************************************************
*/

#define CAP_LOG_MODULE  CPSS_APP_PLATFORM_LOG_MODULE_APPREF_E

#include <profiles/cpssAppPlatformProfile.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpssAppPlatformSysConfig.h>
#include <stdio.h>
#include <appReference/cpssAppRefUtils.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>

#include <ezbringup/cpssAppPlatformEzBringupTools.h>

#ifndef ASIC_SIMULATION
extern GT_STATUS cpssAppRefSerdesTrainingTaskDelete
(
    GT_U8 devNum
);
#endif

GT_STATUS cpssAppPlatformShowProfile
(
   IN GT_CHAR *profileName
)
{
    GT_STATUS rc = GT_OK;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if (!profileName)
       return GT_BAD_PTR;

    rc = cpssAppPlatformPrintProfile(profileName);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPrintProfile);

    return rc;
}

/**
* @internal cpssAppPlatformSysInit function
* @endinternal
*
* @brief   Init Board, PP, runtime based on input profile.
*
* @param[in] profileName           - Profile name.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS cpssAppPlatformSysInit
(
   IN GT_CHAR  *profileName
)
{
    GT_STATUS                      rc         = GT_OK;
    CPSS_APP_PLATFORM_PROFILE_STC *profilePtr = NULL;
    GT_U8                          devNum     = 0;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if (!profileName)
       return GT_BAD_PTR;

    CPSS_APP_PLATFORM_LOG_INFO_MAC("Handle cpssAppPlatformSysInit ProfileName=[%s] \n", profileName);

    rc = cpssAppPlatformProfileGet(profileName, &profilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformProfileGet);

    if(GT_TRUE == ezbIsXmlLoaded())
    {
        rc = cpssAppPlatformEzbUpdateProfile(profilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformEzbUpdateProfile);
    }

    rc = cpssAppPlatformSystemInit(profilePtr, GT_FALSE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformSystemInit);

    for(devNum = 0; devNum < CPSS_APP_PLATFORM_MAX_PP_CNS; devNum++)
    {
        if(PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
        {
            rc = cpssAppRefEventFuncInit(devNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppRefEventFuncInit);

            rc = localUtfInitRef(devNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, localUtfInitRef);
        }
    }

    return GT_OK;
}

/**
* @internal cpssInitSystem function
* @endinternal
*
* @brief   mimicing appDemo's initSystem to enable use in CI
*
* @param[in] profileName           - Profile name.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS cpssInitSystem
(
   IN GT_CHAR  *profileName
)
{
   return (cpssAppPlatformSysInit(profileName));
}

/**
* @internal cpssAppPlatformSysPpAdd function
* @endinternal
*
* @brief   initilize PP device based on input profile .
*
* @param[in] profileName           - Profile name.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS cpssAppPlatformSysPpAdd
(
   IN GT_CHAR *profileName
)
{
    GT_STATUS                      rc         = GT_OK;
    CPSS_APP_PLATFORM_PROFILE_STC *profilePtr = NULL;
    GT_U8                          devNum     = 0;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if (!profileName)
       return GT_BAD_PTR;

    CPSS_APP_PLATFORM_LOG_INFO_MAC("Handle cpssAppPlatformSysPpAdd ProfileName=[%s] \n",  profileName);

    rc = cpssAppPlatformProfileGet(profileName, &profilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformProfileGet);

    rc = cpssAppPlatformSystemPpAdd(profilePtr, NULL);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformSystemPpAdd);

    for(devNum = 0; devNum < CPSS_APP_PLATFORM_MAX_PP_CNS; devNum++)
    {
        if(PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
        {
            rc = cpssAppRefEventFuncInit(devNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppRefEventFuncInit);

            rc = localUtfInitRef(devNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, localUtfInitRef);
        }
    }

    return GT_OK;
}

/**
* @internal cpssAppPlatformSysPpRemove function
* @endinternal
*
* @brief   remove a specific PP.
*
* @param[in] devNum          - PP device number.
* @param[in] removalType     - Removal type: 0 - Managed Removal,
*                                            1 - Unmanaged Removal,
*                                            2 - Managed Reset
*
* @retval GT_OK              - on success,
* @retval GT_FAIL            - otherwise.
*
*/
GT_STATUS cpssAppPlatformSysPpRemove
(
    IN GT_U8 devNum,
    IN GT_U8 removalType
)
{
    GT_STATUS   rc = GT_OK;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    rc = cpssAppPlatformSystemPpRemove(devNum, removalType);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformSystemPpRemove);

#ifndef ASIC_SIMULATION
    rc = cpssAppRefSerdesTrainingTaskDelete(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppRefSerdesTrainingTaskDelete);
#endif

    CPSS_APP_PLATFORM_LOG_EXIT_MAC(rc);
    return rc;
}

/**
* @internal cpssAppPlatformSysRun function
* @endinternal
*
* @brief   initialize run time modules based on input Profile.
*
* @param[in] profileName           - Profile name.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*
*/
GT_STATUS cpssAppPlatformSysRun
(
   IN GT_CHAR *profileName
)
{
    GT_STATUS rc;
    CPSS_APP_PLATFORM_PROFILE_STC *profilePtr = NULL;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if (!profileName)
       return GT_BAD_PTR;

    CPSS_APP_PLATFORM_LOG_INFO_MAC("Handle cpssAppPlatformSysRun ProfileName=[%s] \n",  profileName);

    rc = cpssAppPlatformProfileGet(profileName, &profilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformProfileGet);

    rc = cpssAppPlatformSystemRun(profilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformSystemRun);

    return GT_OK;
}

/**
* @internal cpssAppPlatformLsProfile function
* @endinternal
*
* @brief   This is the function to list all available Profile .
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS cpssAppPlatformLsProfile
(
GT_VOID
)
{
    GT_STATUS rc;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    rc = cpssAppPlatformListProfiles();
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformListProfiles);

    return GT_OK;
}

/**
* @internal cpssAppPlatformLsTestProfile function
* @endinternal
*
* @brief   This is the function to list all available Profile .
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS cpssAppPlatformLsTestProfile
(
GT_VOID
)
{
    GT_STATUS rc;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    rc = cpssAppPlatformListTestProfiles();
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformListProfiles);

    return GT_OK;
}

