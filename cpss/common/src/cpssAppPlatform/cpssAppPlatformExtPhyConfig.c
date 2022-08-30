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
* @file cpssAppPlatformExtPhyConfig.c
*
* @brief Library to manage External Phy Configuration and download.
*
* @version 1
********************************************************************************
*/

#include <cpssAppPlatformSysConfig.h>
#include <profiles/cpssAppPlatformProfile.h>
#include <cpssAppPlatformExtPhyConfig.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


GT_STATUS cpssAppPlatformExtPhyConfig
(
    IN   GT_U8                               devNum,
    IN   GT_PHYSICAL_PORT_NUM                portNum,
    IN   CPSS_APP_PLATFORM_PHY_CFG_STC      *configData
)
{
    GT_STATUS                           rc             = GT_OK;      /* return code */
    CPSS_APP_PLATFORM_PHY_CFG_DATA_STC *configArrayPtr;
    GT_U32                              ii;                          /* iterator */
    GT_U8                               phyRegAddr;                  /* PHY reg address */
    GT_U16                              phyRegData;                  /* PHY reg data */
    GT_U32                              numOfConfigs;

    if (!configData || !(configData->configDataArray))
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    numOfConfigs = configData->configDataArraySize;
    configArrayPtr = configData->configDataArray;

    for (ii = 0; ii < numOfConfigs; ii++)
    {
        phyRegAddr = (GT_U8)configArrayPtr[ii].phyRegAddr;
        phyRegData = configArrayPtr[ii].phyRegData;

        rc = cpssDxChPhyPortSmiRegisterWrite(devNum, portNum, phyRegAddr, phyRegData);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiRegisterWrite);
    }

    return rc;
}

GT_STATUS cpssAppPlatformExtPhyFwDownload
(
)
{
    return GT_OK;
}
