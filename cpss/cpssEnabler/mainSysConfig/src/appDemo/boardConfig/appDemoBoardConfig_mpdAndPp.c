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
* @file appDemoBoardConfig_mpdAndPp.c
*
* @brief  hold function for the board config files to use to get MPD to 'synch' with the PP.
*
*   NOTE: this file compiled only when 'EZ_BRINGUP' (and DX code) and 'INCLUDE_MPD' is enabled.
*   meaning that 'stub' (if needed) are implemented elsewhere.
*
* @version   1
********************************************************************************
*/

#include <appDemo/boardConfig/appDemoBoardConfig_ezBringupTools.h>
#include <appDemo/boardConfig/appDemoBoardConfig_mpdTools.h>
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
    EZB_PP_CONFIG *appDemoPpConfigPtr;
    EZB_GEN_MAC_INFO_STC   *macGenInfoPtr;
    EZB_GEN_PHY_INFO_STC   *phyInfoPtr;
    GT_U32  ii;
    GT_U32  portNum;
    GT_BOOL didInit;

    appDemoPpConfigPtr = &ezbPpConfigList[devIndex];

    macGenInfoPtr = appDemoPpConfigPtr->macGenInfoPtr;

    if(GT_FALSE == appDemoEzbIsXmlWithDevNum(devNum))
    {
        /* we not have PHYs info , and not did 'phase1' init for the MPD (mpdDriverInitDb(...)) */
        return GT_OK;
    }
    else
    if((!macGenInfoPtr) || (0 == appDemoPpConfigPtr->numOfMacPorts))
    {
        /* we not have PHYs info , and not did 'phase1' init for the MPD (mpdDriverInitDb(...)) */
        return GT_OK;
    }

    for(ii = 0 ; ii < appDemoPpConfigPtr->numOfMacPorts; ii++,macGenInfoPtr++)
    {
        if(GT_FALSE == ezbMpdIsPortWithIfIndex(devNum,ii))
        {
            continue;
        }
        rc = prvAppDemoEzbMpdPortInit(devNum,ii,&didInit);
        if(rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEzbMpdPortInit", rc);
            return rc;
        }

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
        if (GT_OK != rc)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiInterfaceSet", rc);
            return rc;
        }

        rc = cpssDxChPhyPortAddrSet(CAST_SW_DEVNUM(devNum), portNum,
             (GT_U8)phyInfoPtr->smiXmsiInterface.address);
        if (GT_OK != rc)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortAddrSet", rc);
            return rc;
        }
        /* Auto-Negotiation sequence for 1G QSGMII/SGMII */
        if (((macGenInfoPtr->defaultSpeedAndIfMode.ifMode == CPSS_PORT_INTERFACE_MODE_SGMII_E)
                    || (macGenInfoPtr->defaultSpeedAndIfMode.ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E))
                && (!(PRV_CPSS_SIP_6_CHECK_MAC(devNum))))
        {
            rc = cpssDxChPortInbandAutoNegEnableSet(CAST_SW_DEVNUM(devNum), portNum, GT_TRUE);
            if (GT_OK != rc)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortInbandAutoNegEnableSet", rc);
                return rc;
            }
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
    EZB_PP_CONFIG *appDemoPpConfigPtr;
    EZB_GEN_MAC_INFO_STC   *macGenInfoPtr;
    GT_U32  ii;

    appDemoPpConfigPtr = &ezbPpConfigList[devIndex];

    macGenInfoPtr = appDemoPpConfigPtr->macGenInfoPtr;

    if(GT_FALSE == appDemoEzbIsXmlWithDevNum(devNum))
    {
        /* we not have PHYs info , and not did 'phase1' init for the MPD (mpdDriverInitDb(...)) */
        return GT_OK;
    }
    else
    if((!macGenInfoPtr) || (0 == appDemoPpConfigPtr->numOfMacPorts))
    {
        /* we not have PHYs info , and not did 'phase1' init for the MPD (mpdDriverInitDb(...)) */
        return GT_OK;
    }


    /* after prvAppDemoEzbMpdInitHw() we can do 'ADMIN' link down on all ports ...
       till admin decide otherwise */
    for(ii = 0 ; ii < appDemoPpConfigPtr->numOfMacPorts; ii++,macGenInfoPtr++)
    {
        if(GT_FALSE == ezbMpdIsPortWithIfIndex(devNum,ii))
        {
            continue;
        }
        /* Admin down */
        rc = ezbMpdPortAdminStatusSet(devNum,ii, GT_FALSE);
        if (GT_OK != rc)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortInbandAutoNegEnableSet", rc);
            return rc;
        }
    }

    return GT_OK;
}
/**
* @internal appDemoEzbMpdPpPortInit function
* @endinternal
*
* @brief  init all the potrs in all the devices to bind those that supports PHY
*   to the MPD (PHY driver) and set all port to 'admin down' (not force link up)
*   (prvAppDemoEzbMpdPortInit)
*   also set the PP MAC ports with : SMI addresses and 'inband AN' with the phy
*
*/
GT_STATUS appDemoEzbMpdPpPortInit
(
    void
)
{
    GT_STATUS rc;
    GT_U32 devIndex;
    APP_DEMO_PP_CONFIG *appDemoPpConfigPtr;
    GT_U32  counterInit = 0;

    for (devIndex = SYSTEM_DEV_NUM_MAC(0);
         devIndex < SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount);
         devIndex++)
    {
        appDemoPpConfigPtr = &appDemoPpConfigList[devIndex];
        if(appDemoPpConfigPtr->valid == GT_FALSE)
        {
            continue;
        }
        rc = mpdPpPortInit_part1(devIndex,appDemoPpConfigPtr->devNum,&counterInit);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(counterInit == 0)
    {
        /* no port with PHY */
        return GT_OK;
    }

    /* done after loop on all port on all devices */
    rc = prvAppDemoEzbMpdInitHw();
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvAppDemoEzbMpdInitHw", rc);
        return rc;
    }

   for (devIndex = SYSTEM_DEV_NUM_MAC(0);
         devIndex < SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount);
         devIndex++)
    {
        appDemoPpConfigPtr = &appDemoPpConfigList[devIndex];
        if(appDemoPpConfigPtr->valid == GT_FALSE)
        {
            continue;
        }
        rc = mpdPpPortInit_part2(devIndex,appDemoPpConfigPtr->devNum);
        if(rc != GT_OK)
        {
            return rc;
        }
    }


    return GT_OK;
}

/**
* @internal appDemoEzbSwitchMacAutoNegCompleted function
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
GT_STATUS appDemoEzbSwitchMacAutoNegCompleted(
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
* @internal appDemoEzbPortSpeedSet function
* @endinternal
*
* @brief  Power, Set duplex mode and Set the provided speed of the PHY
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] portNum              - the physical port number
* @param[in] portSpeed            - the physical port speed
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
*/
GT_STATUS appDemoEzbPortSpeedSet(
        IN GT_SW_DEV_NUM            devNum,
        IN GT_PHYSICAL_PORT_NUM     portNum,
        IN CPSS_PORT_SPEED_ENT      portSpeed
)
{
    GT_STATUS rc;

    rc = ezbMpdPortAdminStatusSet(devNum, portNum, GT_TRUE);
    if (GT_OK != rc)
    {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEzbMpdPortAdminStatusSet", rc);
                return rc;
    }
    rc = ezbMpdPortDuplexSet(devNum, portNum, GT_TRUE);
    if (GT_OK != rc)
    {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEzbMpdPortDuplexSet", rc);
                return rc;
    }
    rc = ezbMpdPortSpeedSet(devNum, portNum, portSpeed);
    if (GT_OK != rc)
    {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEzbMpdPortSpeedSet", rc);
                return rc;
    }
    return rc;
}

