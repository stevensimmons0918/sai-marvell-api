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
* @file cpssAppPlatformPpUtils.h
*
* @brief Prestera Devices PP Utility module.
*
* @version   1
********************************************************************************
*/

#ifndef __CPSS_APP_PLATFORM_PPUTILS_H
#define __CPSS_APP_PLATFORM_PPUTILS_H

#include <profiles/cpssAppPlatformProfile.h>
#include <cpssAppPlatformSysConfig.h>

extern GT_U32 cpssCapMaxPortNum[CPSS_APP_PLATFORM_MAX_PP_CNS];

#define RX_BUFF_SIZE_DEF        1548
#define APP_PLATFORM_RX_BUFF_ALLIGN_BITS_DEF  7
#define APP_PLATFORM_RX_BUFF_ALLIGN_DEF      (1 << APP_PLATFORM_RX_BUFF_ALLIGN_BITS_DEF)
#define APP_PLATFORM_RX_BUFF_SIZE_DEF        800

#define APP_PLATFORM_BAD_VALUE (GT_U32)(~0)

#ifdef CHX_FAMILY
/* check that port has valid mapping or not CPU port */
#define CPSS_APPLICATION_PORT_SKIP_CHECK(dev,port)                                                   \
    if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(dev) == GT_TRUE)                    \
    {                                                                                            \
        GT_BOOL   _isCpu, _isValid;                                                                        \
        GT_STATUS _rc, _rc1;                                                                           \
        if ((port) >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev))                                \
            break; /* no more */                                                                 \
        _rc = cpssDxChPortPhysicalPortMapIsCpuGet(dev, port, &_isCpu);                           \
        _rc1 = cpssDxChPortPhysicalPortMapIsValidGet(dev, port, &_isValid);                      \
        if((_rc != GT_OK) || (_isCpu == GT_TRUE) ||(_rc1 != GT_OK) || (_isValid != GT_TRUE ) )   \
            continue;                                                                            \
    }                                                                                            \
    else                                                                                         \
    {                                                                                            \
        if(PRV_CPSS_PP_MAC(dev)->phyPortInfoArray[port].portType == PRV_CPSS_PORT_NOT_EXISTS_E)  \
            continue;                                                                            \
    }
#endif

/**
* @internal cpssAppPlatformPpPortMapGet function
* @endinternal
*
* @brief   Get port mapping details for a specific device.
*
* @param[in] devNum                   - device number
* @param[out] mapArrPtr               - pointer to portMap pointer/Array
* @param[out] mapArrLen               - pointer to store portMap array length
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on not filling the out params
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS cpssAppPlatformPpPortMapGet
(
   IN  GT_U8                    devNum,
   OUT CPSS_DXCH_PORT_MAP_STC **mapArrPtr,
   OUT GT_U32                  *mapArrLen
);

/**
* @internal cpssAppPlatformAfterInitConfig function
* @endinternal
*
* @brief   After init configurations of PP device.
*
* @param[in] devNum                - Device Number.
* @param [in] *ppProfileInfo       - PP profile
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/

GT_STATUS cpssAppPlatformAfterInitConfig
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
);

/**
* @internal appPlatformLedInterfacesInit function
* @endinternal
*
* @brief   LED Init configurations
*
* @param [in] deviceNumber       - CPSS device number,
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                  - on success,
* @retval GT_FAIL                - otherwise.
*/

GT_STATUS appPlatformLedInterfacesInit
(
    IN GT_U8        devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC  *ppProfilePtr
);


/**
* @internal prvCpssAppPlatformIpLpmLibReset function
* @endinternal
*
* @brief   IP LPM module reset
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/

GT_STATUS prvCpssAppPlatformIpLpmLibReset();

GT_STATUS prvCpssAppPlatformDeAllocateDmaMem
(
    IN      GT_U8  devNum
);

GT_STATUS prvCpssAppPlatformPpPhase1ConfigClear
(
    IN  GT_U8               devNum
);

#endif /* __CPSS_APP_PLATFORM_PPUTILS_H*/
