/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file cpssAppPlatFormMpdAndPp.c
*
* @brief  hold function for the board config files to use to get MPD to 'synch' with the PP.
*
*   NOTE: this file compiled only when 'EZ_BRINGUP' (and DX code) and 'INCLUDE_MPD' is enabled.
*   meaning that 'stub' (if needed) are implemented elsewhere.
*
* @version   1
********************************************************************************
*/

#include <ezbringup/cpssAppPlatformEzBringupTools.h>
#include <ezbringup/cpssAppPlatformMpdTool.h>
#include <ezbringup/cpssAppPlatformMpdAndPp.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmiPreInit.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
* @internal mpdPpPortInit_part1 function
* @endinternal
*
* @brief  init all the potrs in all the devices to bind those that supports PHY
*   to the MPD (PHY driver) and set all port to 'admin down' (not force link up)
*   (prvAppDemoEzbMpdPortInit)
*   also set the PP MAC ports with : SMI addresses and 'inband AN' with the phy
*
*/
static GT_STATUS mpdPpPortInit_part1
(
    IN GT_U32                   devIndex,
    IN GT_SW_DEV_NUM            devNum,
    INOUT GT_U32                *initDonePtr
)
{
    GT_STATUS   rc;
    EZB_PP_CONFIG *ezbPpConfigPtr;
    EZB_GEN_MAC_INFO_STC   *macGenInfoPtr;
    EZB_GEN_PHY_INFO_STC   *phyInfoPtr;
    GT_U32  ii;
    GT_U32  portNum;
    GT_BOOL didInit;

    ezbPpConfigPtr = &ezbPpConfigList[devIndex];

    macGenInfoPtr = ezbPpConfigPtr->macGenInfoPtr;

    if(GT_FALSE == ezbIsXmlWithDevNum(devNum))
    {
        /* we not have PHYs info , and not did 'phase1' init for the MPD (mpdDriverInitDb(...)) */
        return GT_OK;
    }
    else
    if((!macGenInfoPtr) || (0 == ezbPpConfigPtr->numOfMacPorts))
    {
        /* we not have PHYs info , and not did 'phase1' init for the MPD (mpdDriverInitDb(...)) */
        return GT_OK;
    }

    for(ii = 0 ; ii < ezbPpConfigPtr->numOfMacPorts; ii++,macGenInfoPtr++)
    {
        if(GT_FALSE == ezbMpdIsPortWithIfIndex(devNum,ii))
        {
            continue;
        }
        rc = prvCpssAppPlatformEzbMpdPortInit(devNum,ii,&didInit);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssAppPlatformEzbMpdPortInit);

        if(didInit == GT_FALSE)
        {
            continue;
        }

        (*initDonePtr) ++;

        portNum     = macGenInfoPtr->physicalPort;
        phyInfoPtr  = &macGenInfoPtr->phyInfo;

        /*****************************************************/
        /* do switch side SMI/XSMI interface info for inband */
        /* autoneg with the PHY ports                        */
        /*****************************************************/
        rc = cpssDxChPhyPortSmiInterfaceSet(CAST_SW_DEVNUM(devNum), portNum,
             phyInfoPtr->smiXmsiInterface.interfaceId);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiInterfaceSet);

        rc = cpssDxChPhyPortAddrSet(CAST_SW_DEVNUM(devNum), portNum,
             (GT_U8)phyInfoPtr->smiXmsiInterface.address);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortAddrSet);
        /* Auto-Negotiation sequence for 1G QSGMII/SGMII */
        if (((macGenInfoPtr->defaultSpeedAndIfMode.ifMode == CPSS_PORT_INTERFACE_MODE_SGMII_E)
                || (CPSS_PORT_IF_MODE_QSGMII_USX_QUSGMII_CHECK_MAC(macGenInfoPtr->defaultSpeedAndIfMode.ifMode)))
                && (!(PRV_CPSS_SIP_6_CHECK_MAC(devNum))))
        {
            rc = cpssDxChPortInbandAutoNegEnableSet(CAST_SW_DEVNUM(devNum), portNum, GT_TRUE);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortInbandAutoNegEnableSet);
        }
    }

    return GT_OK;
}

/**
* @internal mpdPpPortInit_part2 function
* @endinternal
*
* @brief  init all the potrs in all the devices to bind those that supports PHY
*   to the MPD (PHY driver) and set all port to 'admin down' (not force link up)
*   (prvAppDemoEzbMpdPortInit)
*   also set the PP MAC ports with : SMI addresses and 'inband AN' with the phy
*
*/
static GT_STATUS mpdPpPortInit_part2
(
    IN GT_U32                   devIndex,
    IN GT_SW_DEV_NUM            devNum
)
{
    GT_STATUS   rc;
    EZB_PP_CONFIG *ezbPpConfigPtr;
    EZB_GEN_MAC_INFO_STC   *macGenInfoPtr;
    GT_U32  ii;

    ezbPpConfigPtr = &ezbPpConfigList[devIndex];

    macGenInfoPtr = ezbPpConfigPtr->macGenInfoPtr;

    if(GT_FALSE == ezbIsXmlWithDevNum(devNum))
    {
        /* we not have PHYs info , and not did 'phase1' init for the MPD (mpdDriverInitDb(...)) */
        return GT_OK;
    }
    else
    if((!macGenInfoPtr) || (0 == ezbPpConfigPtr->numOfMacPorts))
    {
        /* we not have PHYs info , and not did 'phase1' init for the MPD (mpdDriverInitDb(...)) */
        return GT_OK;
    }


    /* after prvAppDemoEzbMpdInitHw() we can do 'ADMIN' link down on all ports ...
       till admin decide otherwise */
    for(ii = 0 ; ii < ezbPpConfigPtr->numOfMacPorts; ii++,macGenInfoPtr++)
    {
        if(GT_FALSE == ezbMpdIsPortWithIfIndex(devNum,ii))
        {
            continue;
        }
        /* Admin down */
        rc = ezbMpdPortAdminStatusSet(devNum,ii, GT_FALSE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, ezbMpdPortAdminStatusSet);
    }

    return GT_OK;
}
/**
* @internal cpssAppPlatformEzbMpdPpPortInit function
* @endinternal
*
* @brief  init all the potrs in all the devices to bind those that supports PHY
*   to the MPD (PHY driver) and set all port to 'admin down' (not force link up)
*   (prvAppDemoEzbMpdPortInit)
*   also set the PP MAC ports with : SMI addresses and 'inband AN' with the phy
*
*/
GT_STATUS cpssAppPlatformEzbMpdPpPortInit
(
    CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC *trafficEnablePtr
)
{
    GT_STATUS rc;
    GT_U32 devIndex = 0;
    GT_U32  counterInit = 0;
    CPSS_NULL_PTR_CHECK_MAC(trafficEnablePtr);
    rc = mpdPpPortInit_part1(devIndex,trafficEnablePtr->devNum,&counterInit);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, mpdPpPortInit_part1);
    if(counterInit == 0)
    {
        /* no port with PHY */
        return GT_OK;
    }

    /* done after loop on all port on all devices */
    rc = prvCpssAppPlatformEzbMpdInitHw();
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssAppPlatformEzbMpdInitHw);

    rc = mpdPpPortInit_part2(devIndex,trafficEnablePtr->devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, mpdPpPortInit_part2);

    return GT_OK;
}

/**
* @internal cpssAppPlatformEzbSwitchMacAutoNegCompleted function
* @endinternal
*
* @brief  notify that got event of 'CPSS_PP_PORT_AN_COMPLETED_E' from the port in the PP
*       and need to synch the PP with the MPD info (with the PHY)
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] portNum              - the physical port number
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
*/
GT_STATUS cpssAppPlatformEzbSwitchMacAutoNegCompleted(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS               rc;
    CPSS_PORT_SPEED_ENT     phy_speed;
    GT_BOOL                 phy_isDuplex;
    GT_BOOL                 phy_adminStatus;
    GT_U32                  macPort;

    rc = prvCpssDxChPortEventPortMapConvert(devNum,
        PRV_CPSS_EV_CONVERT_DIRECTION_UNI_EVENT_TO_HW_INTERRUPT_E, portNum, &macPort);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* check if this port hold MPD ifIndex (hold PHY) */
    if(GT_FALSE == ezbMpdIsPortWithIfIndex(devNum,macPort))
    {
        /* the port not valid for the operations */
        return  GT_OK;
    }

    /* check if the PHY did 'AM competed' and get few parameters */
    rc = ezbMpdPortAutoNegResolvedCheck(devNum,macPort,
                    &phy_speed, &phy_isDuplex,&phy_adminStatus);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* apply on the Switch MAC the speed and duplex */
    return rc;
}


/**
* @internal cpssAppPlatformEzbPortSpeedSet function
* @endinternal
*
* @brief  Power, Set duplex mode and Set the provided speed of the PHY
*
* @param[in] trafficEnablePtr     - pointer to Traffic profile.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
*/
GT_STATUS cpssAppPlatformEzbPortSpeedSet
(
    IN CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC *trafficEnablePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_SW_DEV_NUM            devNum;
    GT_PHYSICAL_PORT_NUM     portNum;
    EZB_PP_CONFIG *ezbPpConfigPtr;
    EZB_GEN_MAC_INFO_STC   *macGenInfoPtr;

    CPSS_NULL_PTR_CHECK_MAC(trafficEnablePtr);
    devNum = trafficEnablePtr->devNum;
    ezbPpConfigPtr = &ezbPpConfigList[devNum];

    macGenInfoPtr = ezbPpConfigPtr->macGenInfoPtr;

    for(portNum = 0 ; portNum < ezbPpConfigPtr->numOfMacPorts; portNum++,macGenInfoPtr++)
    {
        if(macGenInfoPtr->isPhyUsed == GT_TRUE)
        {
            rc = ezbMpdPortAdminStatusSet(devNum, portNum, GT_TRUE);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, ezbMpdPortAdminStatusSet);

            rc = ezbMpdPortDuplexSet(devNum, portNum, GT_TRUE);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, ezbMpdPortDuplexSet);

            rc = ezbMpdPortSpeedSet(devNum, portNum, macGenInfoPtr->defaultSpeedAndIfMode.speed);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, ezbMpdPortSpeedSet);
        }
    }
    return rc;
}

