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
* @file cpssAppPlatformExtPhyConfig.h
*
* @brief External Phy config related DSs and functions
*
* @version   1
********************************************************************************
*/

#ifndef __CPSS_APP_PLATFORM_EXTPHYCONFIG_H
#define __CPSS_APP_PLATFORM_EXTPHYCONFIG_H

#include <cpss/common/cpssTypes.h>
#include <cpss/common/phy/cpssGenPhySmi.h>

typedef struct _CPSS_APP_PLATFORM_PHY_CFG_DATA_STC
{
    GT_U8                 phyRegAddr;
    GT_U16                phyRegData;
} CPSS_APP_PLATFORM_PHY_CFG_DATA_STC;

typedef struct _CPSS_APP_PLATFORM_PHY_CFG_STC
{
    CPSS_APP_PLATFORM_PHY_CFG_DATA_STC *configDataArray;
    GT_U32                              configDataArraySize;
} CPSS_APP_PLATFORM_PHY_CFG_STC;

typedef enum _CPSS_APP_PLATFORM_PHY_ITF_TYPE_ENT
{
    CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E,
    CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E,
    CPSS_APP_PLATFORM_PHY_ITF_TYPE_MAX_E

} CPSS_APP_PLATFORM_PHY_ITF_TYPE_ENT;

typedef union _CPSS_APP_PLATFORM_PHY_ITF_UNT
{
    CPSS_PHY_SMI_INTERFACE_ENT   smiItf;
    CPSS_PHY_XSMI_INTERFACE_ENT  xSmiItf;
} CPSS_APP_PLATFORM_PHY_ITF_UNT;

typedef struct _CPSS_APP_PLATFORM_PHY_INFO_STC
{
    CPSS_APP_PLATFORM_PHY_ITF_TYPE_ENT     phyItfType;
    CPSS_APP_PLATFORM_PHY_ITF_UNT          phyItf;
    GT_U32                                 phyAddr;
} CPSS_APP_PLATFORM_PHY_INFO_STC;

typedef struct _CPSS_APP_PLATFORM_EXT_PHY_PORT_MAP
{
    GT_PHYSICAL_PORT_NUM                   portNum;
    CPSS_APP_PLATFORM_PHY_INFO_STC         phyInfo;
    CPSS_APP_PLATFORM_PHY_CFG_STC          phyConfigData;
} CPSS_APP_PLATFORM_EXT_PHY_PORT_MAP;


GT_STATUS cpssAppPlatformExtPhyConfig
(
    IN   GT_U8                               devNum,
    IN   GT_PHYSICAL_PORT_NUM                portNum,
    IN   CPSS_APP_PLATFORM_PHY_CFG_STC      *cfg
);

#endif /* __CPSS_APP_PLATFORM_EXTPHYCONFIG_H */
