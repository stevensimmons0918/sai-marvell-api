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
* @file cpssAppPlatformSysConfig.h
*
* @brief CPSS Application platform System initialization function
*
* @version   1
********************************************************************************
*/

#ifndef __CPSS_APP_PLATFORM_SYSCONFIG_H
#define __CPSS_APP_PLATFORM_SYSCONFIG_H

#include <profiles/cpssAppPlatformProfile.h>
#include <cpssAppPlatformLogLib.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#endif

/* CPSS App Platform CNS */
#define CPSS_APP_PLATFORM_MAX_PP_CNS 32

/* Macros to iterate profile list */
#define PRV_CPSS_APP_START_LOOP_PROFILE_MAC(_profilePtr, _profileType) \
    while(_profilePtr->profileType != CPSS_APP_PLATFORM_PROFILE_TYPE_LAST_E)\
    {\
        if(_profilePtr->profileType == _profileType)\
        {

#define PRV_CPSS_APP_END_LOOP_PROFILE_MAC(_profilePtr, _profileType) \
        }\
        _profilePtr++;\
    }

/* Macros to iterate runtime profile list */
#define PRV_CPSS_APP_START_LOOP_RUNTIME_PROFILE_MAC(_profilePtr, _profileType) \
    while(_profilePtr->runtimeInfoType != CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E)\
    {\
        if(_profilePtr->runtimeInfoType == _profileType)\
        {

#define PRV_CPSS_APP_END_LOOP_RUNTIME_PROFILE_MAC(_profilePtr, _profileType) \
        }\
        _profilePtr++;\
    }

#define PRV_IS_LAST_DEVICE_MAC(_devNum, _flag)            \
 do {                                                     \
    GT_U32 i;                                             \
    _flag = GT_TRUE;                                      \
    for (i = 0; i < CPSS_APP_PLATFORM_MAX_PP_CNS; i++)    \
    {                                                     \
        if(i != _devNum && PRV_CPSS_IS_DEV_EXISTS_MAC(i)) \
        {                                                 \
            _flag = GT_FALSE;                             \
            break;                                        \
        }                                                 \
    }                                                     \
 } while(0)

typedef struct _CPSS_APP_PLATFORM_TCAM_LIB_INFO_STC{
    GT_BOOL ttiTcamUseOffset ;
    GT_BOOL pclTcamUseIndexConversion ;
    GT_U32 tcamPclRuleBaseIndexOffset ;
    GT_U32 tcamIpcl0RuleBaseIndexOffset ;
    GT_U32 tcamIpcl1RuleBaseIndexOffset ;
    GT_U32 tcamIpcl2RuleBaseIndexOffset ;
    GT_U32 tcamEpclRuleBaseIndexOffset ;
    GT_U32 tcamIpcl0MaxNum ;
    GT_U32 tcamIpcl1MaxNum ;
    GT_U32 tcamIpcl2MaxNum ;
    GT_U32 tcamEpclMaxNum  ;
    GT_U32 tcamTtiHit0RuleBaseIndexOffset;
    GT_U32 tcamTtiHit1RuleBaseIndexOffset;
    GT_U32 tcamTtiHit2RuleBaseIndexOffset;
    GT_U32 tcamTtiHit3RuleBaseIndexOffset;
    GT_U32 tcamTtiHit0MaxNum ;
    GT_U32 tcamTtiHit1MaxNum ;
    GT_U32 tcamTtiHit2MaxNum ;
    GT_U32 tcamTtiHit3MaxNum ;
    GT_U32 save_tcamTtiHit0RuleBaseIndexOffset;
    GT_U32 save_tcamTtiHit1RuleBaseIndexOffset;
    GT_U32 save_tcamTtiHit2RuleBaseIndexOffset;
    GT_U32 save_tcamTtiHit3RuleBaseIndexOffset;
    GT_U32 save_tcamTtiHit0MaxNum;
    GT_U32 save_tcamTtiHit1MaxNum;
    GT_U32 save_tcamTtiHit2MaxNum;
    GT_U32 save_tcamTtiHit3MaxNum;
    GT_U32 save_tcamIpcl0RuleBaseIndexOffset;
    GT_U32 save_tcamIpcl1RuleBaseIndexOffset;
    GT_U32 save_tcamIpcl2RuleBaseIndexOffset;
    GT_U32 save_tcamEpclRuleBaseIndexOffset;
    GT_U32 save_tcamIpcl0MaxNum;
    GT_U32 save_tcamIpcl1MaxNum;
    GT_U32 save_tcamIpcl2MaxNum;
    GT_U32 save_tcamEpclMaxNum;
}CPSS_APP_PLATFORM_TCAM_LIB_INFO_STC;


/**
* @enum CPSS_APP_PLATFORM_PP_REMOVAL_TYPE_ENT
*
* @brief Determines the type of Removal PP device.
*/
typedef enum{
    /* Managed Removal */
    CPSS_APP_PLATFORM_PP_REMOVAL_MANAGED_E,

    /* Unexpected Removal or Crash */
    CPSS_APP_PLATFORM_PP_REMOVAL_UNMANAGED_E,

    /* Managed full Reset*/
    CPSS_APP_PLATFORM_PP_REMOVAL_RESET_E,

    /** the last parameter */
    CPSS_APP_PLATFORM_PP_REMOVAL_LAST_E

} CPSS_APP_PLATFORM_PP_REMOVAL_TYPE_ENT;

GT_STATUS cpssAppPlatformTcamLibParamsGet
(
    OUT CPSS_APP_PLATFORM_TCAM_LIB_INFO_STC *tcamInfo
);

/** Cpss Applicatoin platform APIs **/

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
);

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
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if input profile is not Pp profile.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformSystemPpAdd
(
    IN CPSS_APP_PLATFORM_PROFILE_STC *inputProfileListPtr,
    IN CPSS_SYSTEM_RECOVERY_INFO_STC *systemRecovery
);

/*
* @internal cpssAppPlatformSystemHaStateSet function
* @endinternal
*
* @brief   Set Ha state.
*
* @param[in] haState             - HA state.
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if haState > 3.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformSystemHaStateSet
(
    IN GT_U8   haState
);

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
);



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
    IN GT_U8                                 devNum,
    IN CPSS_APP_PLATFORM_PP_REMOVAL_TYPE_ENT removalType
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
);

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
);

#endif /* __CPSS_APP_PLATFORM_SYSCONFIG_H */
