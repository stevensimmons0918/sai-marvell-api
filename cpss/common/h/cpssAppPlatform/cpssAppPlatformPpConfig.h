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
* @file cpssAppPlatformPpConfig.h
*
* @brief CPSS Application platform - PP initialization function
*
* @version   1
********************************************************************************
*/

#ifndef __CPSS_APP_PLATFORM_PPCONFIG_H
#define __CPSS_APP_PLATFORM_PPCONFIG_H

#include <profiles/cpssAppPlatformProfile.h>

typedef struct CPSS_APP_PLATFORM_XPHY_INFO_STCT
{
    GT_U32                              phyIndex;
    GT_U32                              portGroupId;
    CPSS_PHY_XSMI_INTERFACE_ENT         xsmiInterface;
    GT_U16                              phyAddr;
    GT_U32                              phyType;
    GT_U32                              hostDevNum;
    GT_VOID_PTR                         driverObj;
} CPSS_APP_PLATFORM_XPHY_INFO_STC;

/* CPSS Applicaiton Platform - PP configuration APIs */
/**
* @internal cpssAppPlatformPhyPortOperGet function
* @endinternal
*
* @brief   Initialize a specific PP based on profile.
*
* @param [in] devNum           - CPSS device Number,
* @param [in] portNum          - Removal type: 0 - Managed Removal,
*                                              1 - Unmanaged Removal,
*                                              2 - Managed Reset
* @param [out] enable          - CPSS device Number,
* @param [out] speed           - CPSS device Number,
* @param [out] duplex          - CPSS device Number,
*
* @retval GT_OK                - on success,
* @retval GT_BAD_PARAM         - if devNum > 31 or device not present.
* @retval GT_FAIL              - otherwise.
*/
GT_STATUS cpssAppPlatformPhyPortOperGet
(
   IN  GT_SW_DEV_NUM            devNum,
   IN  GT_U32                   portNum,
   OUT GT_BOOL                  *enable,
   OUT GT_U32                   *speed,
   OUT GT_BOOL                  *duplex
);

/*
* @internal cpssAppPlatformPpInsert function
* @endinternal
*
* @brief   initialize a specific packet processor.
*
* @param [in] devNum           - CPSS device Number,
* @param [in] *ppMapPtr        - PP_MAP board profile,
* @param [in] *ppProfileInfo   - Pp Profile,
* @param [in] *systemRecovery  - System Recovery mode
*
* @retval GT_OK                - on success.
* @retval GT_BAD_PARAM         - if devNum > 31 or device not present.
* @retval GT_FAIL              - otherwise.
*/
GT_STATUS cpssAppPlatformPpInsert
(
    IN GT_U8                                     devNum,
    IN CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC *ppMapPtr,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC         *ppProfilePtr,
    IN CPSS_SYSTEM_RECOVERY_INFO_STC            *systemRecovery
);

/*
* @internal cpssAppPlatformPpHwInit function
* @endinternal
*
* @brief   initialize Pp phase1 init and WAs.
*
* @param [in] *ppMapPtr          - PCI/SMI info of the device,
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if devNum > 31 or device not present.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformPpHwInit
(
    IN CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC *ppMapPtr,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC         *ppProfilePtr
);

/*
* @internal cpssAppPlatformPpPortsInit function
* @endinternal
*
* @brief  Set port mapping. (after Phase1 Init)
*
* @param[in] devNum              - CPSS device number.
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if devNum > 31 or device not present.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformPpPortsInit
(
    IN CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC *ppMapPtr
);

/**
* @internal cpssAppPlatformPhase2Init function
* @endinternal
*
* @brief   Phase2 PP configurations
*
* @param [in] deviceNumber       - CPSS device number,
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                  - on success,
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformPhase2Init
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
);

/*
* @internal cpssAppPlatformPpLibInit function
* @endinternal
*
* @brief   initialize CPSS LIBs in correct order.
*
* @param[in] devNum              - CPSS device number.
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if devNum > 31 or device not present.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformPpLibInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
);

/**
* @internal cpssAppPlatformPpLogicalInit function
* @endinternal
*
* @brief logical initialization for a specific Pp.
*
* @param[in] devNum                - Device Number.
* @param [in] *ppProfileInfo       - PP profile
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformPpLogicalInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
);

/**
* @internal cpssAppPlatformPpGeneralInit function
* @endinternal
*
* @brief General PP configurations.
*
* @param[in] devNum                - Device Number.
* @param [in] *ppProfileInfo       - PP profile
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformPpGeneralInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
);

/**
* @internal cpssAppPlatformPpRemove function
* @endinternal
*
* @brief   Initialize a specific PP based on profile.
*
* @param [in] devNum           - CPSS device Number,
* @param [in] ppRemovalType    - Removal type: 0 - Managed Removal,
*                                              1 - Unmanaged Removal,
*                                              2 - Managed Reset
*
* @retval GT_OK                - on success,
* @retval GT_BAD_PARAM         - if devNum > 31 or device not present.
* @retval GT_FAIL              - otherwise.
*/
GT_STATUS cpssAppPlatformPpRemove
(
    IN GT_SW_DEV_NUM                             devNum,
    IN CPSS_APP_PLATFORM_PP_REMOVAL_TYPE_ENT     ppRemovalType
);

/**
* @internal appPlatformDbEntryAdd function
* @endinternal
*
* @brief   Set App DataBase value.This value will be considered during system
*         initialization process.
* @param[in] namePtr                  - points to parameter name
* @param[in] value                    - parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_RESOURCE           - if Database is full
*/
GT_STATUS appPlatformDbEntryAdd
(
    IN  GT_CHAR *namePtr,
    IN  GT_U32  value
);

/**
* @internal appPlatformDbEntryGet function
* @endinternal
*
* @brief   Get parameter value from App DataBase.
*
* @param[in] namePtr                  - points to parameter name
*
* @param[out] valuePtr                 - points to parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_SUCH               - there is no such parameter in Database
*/
GT_STATUS appPlatformDbEntryGet
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
);

/**
* @internal appPlatformDbBlocksAllocationMethodGet function
* @endinternal
*
* @brief   Get the blocks allocation method configured in the Init System
*/
GT_STATUS appPlatformDbBlocksAllocationMethodGet
(
    IN  GT_U8                                               dev,
    OUT CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT  *blocksAllocationMethodGet
);

/**
* @internal appPlatformBc2IpLpmRamDefaultConfigCalc function
* @endinternal
*
* @brief   This function calculate the default RAM LPM DB configuration for LPM management.
*
* @param[in] devNum                   - The Pp device number to get the parameters for.
* @param[in] maxNumOfPbrEntries       - number of PBR entries to deduct from the LPM memory calculations
*
* @param[out] ramDbCfgPtr              - (pointer to) ramDbCfg structure to hold the defaults calculated
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appPlatformBc2IpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
);

/**
* @internal appPlatformFalconIpLpmRamDefaultConfigCalc function
* @endinternal
*
* @brief   This function calculate the default RAM LPM DB configuration for LPM management.
*
* @param[in] devNum                   - The Pp device number to get the parameters for.
* @param[in] sharedTableMode          - shared tables mode
* @param[in] maxNumOfPbrEntries       - number of PBR entries to deduct from the LPM memory calculations
*
* @param[out] ramDbCfgPtr              - (pointer to) ramDbCfg structure to hold the defaults calculated
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appPlatformFalconIpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT      sharedTableMode,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
);

/**
* @internal appPlatformLpmRamConfigSet function
* @endinternal
*
* @brief  Set given list of device type and Shared memory configuration mode.
*         Should be called before cpssInitSystem().
*
* @param[in] lpmRamConfigInfoArray          - array of pairs: devType+Shared memory configuration mode
* @param[in] lpmRamConfigInfoNumOfElements  - number of valid pairs

* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on lpmRamConfigInfoNumOfElements bigger than array size
*/
GT_STATUS appPlatformLpmRamConfigSet
(
    IN CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC    lpmRamConfigInfoArray[CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS],
    IN GT_U32                               lpmRamConfigInfoNumOfElements
);

/**
* @internal appPlatformLpmRamConfigGet function
* @endinternal
*
* @brief  Get given list of device type and Shared memory configuration mode.
*
* @param[out] lpmRamConfigInfoArray             - array of pairs: devType+Shared memory configuration mode
* @param[out] lpmRamConfigInfoNumOfElementsPtr  - (pointer to) number of valid pairs
* @param[out] lpmRamConfigInfoSetFlagPtr        - (pointer to) Flag specifying that the configuration was set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS appPlatformLpmRamConfigGet
(
    OUT CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC    lpmRamConfigInfoArray[CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS],
    OUT GT_U32                               *lpmRamConfigInfoNumOfElementsPtr,
    OUT GT_BOOL                              *lpmRamConfigInfoSetFlagPtr
);
#endif /* __CPSS_APP_PLATFORM_PPCONFIG_H */
