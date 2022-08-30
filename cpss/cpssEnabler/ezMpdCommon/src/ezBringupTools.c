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
* @file appDemoBoardConfig_ezBringupTools.c
*
* @brief  hold common function (tools) for the board config files to use to get
*   info from the 'ez_bringup' xml
*
*   NOTE: this file compiled only when 'EZ_BRINGUP' (and DX code) is enabled.
*   meaning that 'stub' (if needed) are implemented elsewhere.
*
* @version   1
********************************************************************************
*/

#include <ezBringupTools.h>
#include <mpdTools.h>
#if (defined EZ_BRINGUP) && (defined CHX_FAMILY)
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#endif
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <cpss/common/smi/cpssGenSmi.h>

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

#ifdef ASIC_SIMULATION
    #include <asicSimulation/SEmbedded/simFS.h>
#endif /*ASIC_SIMULATION*/


#if (defined EZ_BRINGUP) && (defined CHX_FAMILY)
#include <pdl/init/pdlInit.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <pdl/serdes/pdlSerdes.h>
#include <iDbgPdl/init/iDbgPdlInit.h>

/* indication that XML was loaded */
static volatile GT_BOOL xmlLoaded = GT_FALSE;
/* the name of the xml that was loaded */
static char* xmlFileName = NULL;
EZB_PP_CONFIG ezbPpConfigList[EZB_MAX_DEV_NUM];

/**
* @internal ezbXmlName function
* @endinternal
*
* @brief  get the XML name use for the EZ_BRINGUP info loaded .
*
*
* @retval 'string' - the name of the XML or "no XML used"
*/
char* ezbXmlName(GT_VOID)
{
    return xmlFileName ? xmlFileName : "no XML used";
}
/**
* @internal ezbIsXmlLoaded function
* @endinternal
*
* @brief  is XML of EZ_BRINGUP info loaded to be used.
*
*
* @retval GT_TRUE / GT_FALSE
*/
GT_BOOL ezbIsXmlLoaded(GT_VOID)
{
    PDL_STATUS        rc;
    PDL_PP_XML_ATTRIBUTES_STC ppAttributes;

    if(xmlLoaded == GT_FALSE)
    {
        return GT_FALSE;
    }

    rc = pdlPpDbAttributesGet(&ppAttributes);
    if(rc == GT_OK && ppAttributes.numOfPps != 0)
    {
        return GT_TRUE;
    }

    return GT_FALSE;
}

static CPSS_PORT_SPEED_ENT ezbMpdSpeedToCpss(
    IN PDL_PORT_SPEED_ENT speed
)
{
    switch (speed)
    {
        case PDL_PORT_SPEED_10_E:
            return CPSS_PORT_SPEED_10_E;
        case PDL_PORT_SPEED_100_E:
            return CPSS_PORT_SPEED_100_E;
        case PDL_PORT_SPEED_1000_E:
            return CPSS_PORT_SPEED_1000_E;
        case PDL_PORT_SPEED_2500_E:
            return CPSS_PORT_SPEED_2500_E;
        case PDL_PORT_SPEED_5000_E:
            return CPSS_PORT_SPEED_5000_E;
        case PDL_PORT_SPEED_10000_E:
            return CPSS_PORT_SPEED_10000_E;
        case PDL_PORT_SPEED_20000_E:
            return CPSS_PORT_SPEED_20000_E;
        case PDL_PORT_SPEED_25000_E:
            return CPSS_PORT_SPEED_25000_E;
        case PDL_PORT_SPEED_40000_E:
            return CPSS_PORT_SPEED_40000_E;
        case PDL_PORT_SPEED_50000_E:
            return CPSS_PORT_SPEED_50000_E;
        case PDL_PORT_SPEED_100000_E:
            return CPSS_PORT_SPEED_100G_E;
        /*No mapping for 24g*/
        case PDL_PORT_SPEED_24000_E:
        case PDL_PORT_SPEED_LAST_E:
        default:
            return CPSS_PORT_SPEED_NA_E;
    }
}

static CPSS_PORT_INTERFACE_MODE_ENT ezbMpdModeToCpss(
    IN PDL_INTERFACE_MODE_ENT mode
)
{
    switch (mode)
    {
        case PDL_INTERFACE_MODE_SGMII_E:
            return CPSS_PORT_INTERFACE_MODE_SGMII_E;
        case PDL_INTERFACE_MODE_XGMII_E:
            return CPSS_PORT_INTERFACE_MODE_XGMII_E;
        case PDL_INTERFACE_MODE_1000BASE_X_E:
            return CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
        case PDL_INTERFACE_MODE_QX_E:
            return CPSS_PORT_INTERFACE_MODE_QX_E;
        case PDL_INTERFACE_MODE_HX_E:
            return CPSS_PORT_INTERFACE_MODE_HX_E;
        case PDL_INTERFACE_MODE_RXAUI_E:
            return CPSS_PORT_INTERFACE_MODE_RXAUI_E;
        case PDL_INTERFACE_MODE_100BASE_FX_E:
            return CPSS_PORT_INTERFACE_MODE_100BASE_FX_E;
        case PDL_INTERFACE_MODE_KR_E:
            return CPSS_PORT_INTERFACE_MODE_KR_E;
        case PDL_INTERFACE_MODE_SR_LR_E:
            return CPSS_PORT_INTERFACE_MODE_SR_LR_E;
        case PDL_INTERFACE_MODE_QSGMII_E:
            return CPSS_PORT_INTERFACE_MODE_QSGMII_E;
        case PDL_INTERFACE_MODE_MII_E:
            return CPSS_PORT_INTERFACE_MODE_MII_E;
        case PDL_INTERFACE_MODE_XHGS_E:
            return CPSS_PORT_INTERFACE_MODE_XHGS_E;
        case PDL_INTERFACE_MODE_KR2_E:
            return CPSS_PORT_INTERFACE_MODE_KR2_E;
        case PDL_INTERFACE_MODE_CR4_E:
            return CPSS_PORT_INTERFACE_MODE_CR4_E;
        case PDL_INTERFACE_MODE_NA_E:
        case PDL_INTERFACE_MODE_LAST_E:
        default:
            return CPSS_PORT_INTERFACE_MODE_NA_E;
    }
}

/**
* @internal cpssAppPlatformEzbIsXmlWithDevNum function
* @endinternal
*
* @brief  is XML of EZ_BRINGUP hold info about the device.
*
*
* @retval GT_TRUE / GT_FALSE
*/
GT_BOOL ezbIsXmlWithDevNum
(
    IN GT_U32  devNum
)
{
    PDL_STATUS        rc;
    PDL_PP_XML_ATTRIBUTES_STC ppAttributes;
    GT_U32  xmlDevNum;  /* the devNum as appear in the XML */

    xmlDevNum = devNum;

    rc = pdlPpDbAttributesGet(&ppAttributes);
    if(rc != GT_OK)
    {
        return GT_FALSE;
    }
    if(xmlDevNum >= ppAttributes.numOfPps)
    {
        return GT_FALSE;
    }

    return GT_TRUE;
}


/**
* @internal ezbMacAttributesGet function
* @endinternal
*
* @brief  get EZ_BRINGUP info related to the specific MAC of specific device
*
* @param[in] xmlDevNum             - The XML device number.
* @param[in] cpssDevNum            - The CPSS SW device number.
* @param[in] macNum                - The mac number of the port. (not physical port number)
*
* @param[out] xmlPortInfoPtr       - the generic port info extracted from the XML for the port.
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_NOT_INITIALIZED       - the XML was not loaded.
* @retval GT_NOT_FOUND             - the devNum or the macPort not exists in the xml.
* @retval GT_NOT_SUPPORTED         - the XML not hold port mapping info for the device.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS ezbMacAttributesGet
(
    IN GT_U32                     xmlDevNum,
    IN GT_U8                      cpssDevNum,
    IN GT_U32                     macNum,
    OUT EZB_XML_MAC_INFO_STC   *xmlPortInfoPtr
)
{
    PDL_PP_XML_NETWORK_PORT_ATTRIBUTES_STC portAttr;
    PDL_PP_XML_ATTRIBUTES_STC              ppAttr;
    PDL_PP_XML_B2B_ATTRIBUTES_STC          b2bLinkPortAttr;
    PDL_PORT_LANE_DATA_STC                 serdesInfo;
    PDL_PHY_CONFIGURATION_STC              phy_configuration;
    PDL_STATUS status;
    GT_U32 ii;
    UINT_32  dummyDevNum;
    UINT_32  pdlLogicalPort;
    PDL_PORT_SPEED_ENT          pdl_speed = PDL_PORT_SPEED_LAST_E;
    PDL_INTERFACE_MODE_ENT      pdl_mode  = PDL_INTERFACE_MODE_LAST_E;

    /*function is currently 'static' so this check not needed */
    if(NULL == xmlPortInfoPtr)
    {
        /*CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEzbMacAttributesGet : xmlPortInfoPtr = NULL ", GT_BAD_PTR);*/
        return GT_BAD_PTR;
    }

    cpssOsMemSet(&portAttr,0,sizeof(portAttr));

    xmlPortInfoPtr->macNum    = macNum;
    xmlPortInfoPtr->isMacUsed = GT_FALSE;

    status = pdlPpDbAttributesGet(&ppAttr);
    if (status != PDL_OK)
    {
        return GT_NOT_FOUND;
    }

    if(ppAttr.numOfBackToBackLinksPerPp)/* check for cascade ports info */
    {
        for(ii = 0 ; ii < ppAttr.numOfBackToBackLinksPerPp ; ii++)
        {
            if (PDL_OK != pdlPpDbB2bAttributesGet(ii, &b2bLinkPortAttr))
            {
                return GT_FAIL;
            }

            if(xmlDevNum == b2bLinkPortAttr.firstDev  &&
               macNum    == b2bLinkPortAttr.firstPort )
            {
                /* cascade port */
            }
            else
            if(xmlDevNum == b2bLinkPortAttr.secondDev  &&
               macNum    == b2bLinkPortAttr.secondPort )
            {
                /* cascade port */
            }
            else
            {
                continue;
            }
            /* this mac is Cascade port and may be also in cascade trunk */
            xmlPortInfoPtr->isCascadeValid           = GT_TRUE;
            pdl_speed = b2bLinkPortAttr.maxSpeed;
            pdl_mode  = b2bLinkPortAttr.interfaceMode;
            break;
        }
    }

    /* cascade port : port mapping does not exists in 'front panel' info */
    /* but we must have it                                               */

    /* convert the MAC to physical port number */
    status = pdlPpPortConvertMacToLogical (xmlDevNum,macNum,
        &pdlLogicalPort,
        &dummyDevNum);
    if (PDL_OK != status)
    {
        return GT_FAIL;
    }

    xmlPortInfoPtr->isMacUsed = GT_TRUE;

    xmlPortInfoPtr->physicalPort = pdlLogicalPort;
    if(xmlPortInfoPtr->isCascadeValid == GT_TRUE)
    {
        /* init with dummy values */
        xmlPortInfoPtr->frontPanelInfo.groupId           = GT_NA;
        xmlPortInfoPtr->frontPanelInfo.localIndexInGroup = GT_NA;
    }
    else
    {
        xmlPortInfoPtr->frontPanelInfo.groupId           = portAttr.frontPanelNumber;/* the group ID */
        xmlPortInfoPtr->frontPanelInfo.localIndexInGroup = portAttr.portNumberInGroup;/*the local index in the group */
    }

    /* get port info from the XML */
    status = pdlPpDbPortAttributesGet(xmlDevNum, pdlLogicalPort, &portAttr);
    if (PDL_OK != status)
    {
        return GT_FAIL;
    }

    xmlPortInfoPtr->transceiverType = portAttr.transceiverType;
    xmlPortInfoPtr->isSerdesInfoValid = GT_FALSE;/* was not implemented */

    if(xmlPortInfoPtr->isCascadeValid == GT_TRUE)
    {
        /* already got speed and mode */
    }
    else
    {
        switch(portAttr.transceiverType)
        {
            case PDL_TRANSCEIVER_TYPE_FIBER_E:
            /*TBD: *ifType = ; */
            /*for (ii = 0; ii < portAttr.numOfFiberModes; ii++)*/
            ii = 0;
            {
                pdl_speed = portAttr.fiberModesArr[ii].speed;
                pdl_mode  = portAttr.fiberModesArr[ii].mode;
            }
            break;
            case PDL_TRANSCEIVER_TYPE_COPPER_E:
            /*TBD: *ifType = ; */
            /*for (ii = 0; ii < portAttr.numOfCopperModes; ii++)*/
            ii = 0;
            {
                pdl_speed = portAttr.copperModesArr[ii].speed;
                pdl_mode  = portAttr.copperModesArr[ii].mode;
            }
            break;
            case PDL_TRANSCEIVER_TYPE_COMBO_E:
            /*TBD: *ifType = ; */
            /*
             *  TBD:
             */
#if 0
            for (ii = 0; ii < portAttr.numOfFiberModes; ii++)
            {
                break;
            }
            for (ii = 0; ii < portAttr.numOfCopperModes; ii++)
            {
                break;
            }
#endif /*0*/
            break;
            default:
                break;
        }
    }/* not cascade */

    xmlPortInfoPtr->defaultSpeedAndIfMode.ifMode = ezbMpdModeToCpss(pdl_mode);
    if (CPSS_PORT_INTERFACE_MODE_NA_E == xmlPortInfoPtr->defaultSpeedAndIfMode.ifMode)
    {
        cpssOsPrintf("Failed to convert port pdl mode to CPSS, [xmlDevNum=%u], [macNum=%u]",
                          xmlDevNum, macNum);
        return GT_FAIL;
    }

    xmlPortInfoPtr->defaultSpeedAndIfMode.speed = ezbMpdSpeedToCpss(pdl_speed);
    if (CPSS_PORT_SPEED_NA_E == xmlPortInfoPtr->defaultSpeedAndIfMode.speed)
    {
        cpssOsPrintf("Failed to convert port pdl speed to CPSS, [xmlDevNum=%u], [macNum=%u]",
                          xmlDevNum, macNum);
        return GT_FAIL;
    }

    xmlPortInfoPtr->isPhyUsed         = portAttr.isPhyExists;
    if(portAttr.isPhyExists)
    {
        xmlPortInfoPtr->phyInfo.phyNumber = portAttr.phyData.phyNumber;
        xmlPortInfoPtr->phyInfo.phyPosition = portAttr.phyData.phyPosition;
        xmlPortInfoPtr->phyInfo.smiXmsiInterface.interfaceType  = portAttr.phyData.smiXmsiInterface.interfaceType;
        xmlPortInfoPtr->phyInfo.smiXmsiInterface.devNum         = portAttr.phyData.smiXmsiInterface.dev;
        xmlPortInfoPtr->phyInfo.smiXmsiInterface.interfaceId    = portAttr.phyData.smiXmsiInterface.interfaceId;
        xmlPortInfoPtr->phyInfo.smiXmsiInterface.address        = portAttr.phyData.smiXmsiInterface.address;
        xmlPortInfoPtr->phyInfo.smiXmsiInterface.secondaryAddressSupported = portAttr.phyData.smiXmsiInterface.secondaryAddressSupported;
        xmlPortInfoPtr->phyInfo.smiXmsiInterface.secondaryAddress = portAttr.phyData.smiXmsiInterface.secondaryAddress;
        status = PdlPhyDbPhyConfigurationGet(portAttr.phyData.phyNumber, &phy_configuration);
        if (PDL_OK != status)
        {
            cpssOsPrintf("Failed to convert port pdl phyNumber to phy_configuration [xmlDevNum=%u], [macNum=%u]",
                              xmlDevNum, macNum);
            return GT_FAIL;
        }
        xmlPortInfoPtr->phyInfo.phyType         = phy_configuration.phyType;
        xmlPortInfoPtr->phyInfo.phyDownloadType = phy_configuration.phyDownloadType;

#ifdef INCLUDE_MPD
        xmlPortInfoPtr->mpd_ifIndex = prvEzbMpdGetNextFreeGlobalIfIndex(cpssDevNum,macNum);
        ezbMpdDevMacToIfIndexAdd(xmlPortInfoPtr->mpd_ifIndex, cpssDevNum, macNum);
#else /*!INCLUDE_MPD*/
        xmlPortInfoPtr->mpd_ifIndex = (cpssDevNum << 16) | macNum;/* DUMMY ! */
#endif  /*!INCLUDE_MPD*/
    }

    /* check if we have SERDES info for this MAC : SERDES muxing info */
    status = pdlSerdesPortDbSerdesInfoGetFirst(xmlDevNum, macNum, &serdesInfo);
    if (PDL_OK == status)
    {
        xmlPortInfoPtr->isSerdesInfoValid = GT_TRUE;/* at least single serdes info for this mac */
        xmlPortInfoPtr->macSerdesInfo.firstSerdesId = serdesInfo.absSerdesNum;
        xmlPortInfoPtr->macSerdesInfo.holdSerdesMuxingInfo = GT_TRUE;
        xmlPortInfoPtr->macSerdesInfo.firstMacPtr = xmlPortInfoPtr;/*TBD*/
        xmlPortInfoPtr->macSerdesInfo.numOfSerdeses = 0;
        do
        {
            xmlPortInfoPtr->macSerdesInfo.macToSerdesMap.serdesLanes[xmlPortInfoPtr->macSerdesInfo.numOfSerdeses] = serdesInfo.relSerdesNum;
            xmlPortInfoPtr->macSerdesInfo.numOfSerdeses++;

            if(serdesInfo.absSerdesNum < xmlPortInfoPtr->macSerdesInfo.firstSerdesId)
            {
                xmlPortInfoPtr->macSerdesInfo.firstSerdesId = serdesInfo.absSerdesNum;
            }
        }
        while(PDL_OK == pdlSerdesPortDbSerdesInfoGetNext(xmlDevNum, macNum, &serdesInfo, &serdesInfo));

    }/* end of first SERDES info */


    return GT_OK;
}

/**
* @internal ezbSerdesAttributesGet function
* @endinternal
*
* @brief  get EZ_BRINGUP info related to the specific SERDES of specific device
*
* @param[in] xmlDevNum             - The XML device number.
* @param[in] serdesNum             - The SERDES number of the port.
*
* @param[out] xmlSerdesInfoPtr     - the generic SERDES info extracted from the XML for the port.
*                                    (if the XML hold such info)
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS ezbSerdesAttributesGet
(
    IN GT_U32       xmlDevNum,
    IN GT_U32       serdesNum,
    OUT EZB_XML_SERDES_INFO_STC   *xmlSerdesInfoPtr
)
{
    PDL_STATUS status;
    PDL_LANE_POLARITY_ATTRIBUTES_STC    lanePolarityInfo;

    /* check that this info did not come from other 'non-XML' info */
    if(xmlSerdesInfoPtr->polarityValid == GT_FALSE)
    {
        cpssOsMemSet(&lanePolarityInfo,0,sizeof(lanePolarityInfo));

        status = pdlSerdesDbPolarityAttrGet(xmlDevNum,serdesNum,&lanePolarityInfo);
        if(status == PDL_OK)
        {
            cpssOsMemSet(&xmlSerdesInfoPtr->polarity,0,sizeof(xmlSerdesInfoPtr->polarity));

            xmlSerdesInfoPtr->polarityValid = GT_TRUE;
            xmlSerdesInfoPtr->polarity.laneNum  = serdesNum;
            xmlSerdesInfoPtr->polarity.invertTx = lanePolarityInfo.txSwap ? GT_TRUE : GT_FALSE;
            xmlSerdesInfoPtr->polarity.invertRx = lanePolarityInfo.rxSwap ? GT_TRUE : GT_FALSE;
        }
    }

    return GT_OK;
}

/*****************************************************************************
* FUNCTION NAME: util_smi_read
*
* DESCRIPTION:         implementation of smi read for pdl
*
*
*****************************************************************************/
static GT_STATUS util_smi_read
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupsBmp,
    IN  GT_U32      smiInterface,
    IN  GT_U32      smiAddr,
    IN  GT_U8       phyPageSelReg,
    IN  GT_U8       phyPage,
    IN  GT_U32      regAddr,
    OUT GT_U16     *dataPtr
)
{
    GT_U16    old_page=0;
    GT_STATUS rc;
    /* TODO: PPU disable before & enable after */
    rc = cpssSmiRegisterReadShort (devNum, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, &old_page);
    rc |= cpssSmiRegisterWriteShort(devNum, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, phyPage);
    rc |= cpssSmiRegisterReadShort (devNum, portGroupsBmp, smiInterface, smiAddr, regAddr, dataPtr);
    rc |= cpssSmiRegisterWriteShort(devNum, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, old_page);
    return rc;
}
/*****************************************************************************
* FUNCTION NAME: util_smi_write
*
* DESCRIPTION:         implementation of smi write for pdl
*
*
*****************************************************************************/
static GT_STATUS util_smi_write
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupsBmp,
    IN  GT_U32      smiInterface,
    IN  GT_U32      smiAddr,
    IN  GT_U8       phyPageSelReg,
    IN  GT_U8       phyPage,
    IN  GT_U32      regAddr,
    IN  GT_U16      data
)
{
    GT_U16 old_page=0;
    GT_STATUS rc;
    /* TODO: PPU disable before & enable after */
    rc = cpssSmiRegisterReadShort (devNum, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, &old_page);
    rc |= cpssSmiRegisterWriteShort(devNum, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, phyPage);
    rc |= cpssSmiRegisterWriteShort(devNum, portGroupsBmp, smiInterface, smiAddr, regAddr, data);
    rc |= cpssSmiRegisterWriteShort(devNum, portGroupsBmp, smiInterface, smiAddr, phyPageSelReg, old_page);

    return rc;
}
/*****************************************************************************
* FUNCTION NAME: util_cm_register_read
*
* DESCRIPTION:          implementation of c&m register read for pdl
*
*
*****************************************************************************/
static GT_STATUS util_cm_register_read
(
    IN  GT_U8       devNum,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    OUT GT_U32    * dataPtr
)
{
    GT_U32    data;
    GT_STATUS status;
    status = cpssDrvPpHwInternalPciRegRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, &data);
    data &= mask;
    *dataPtr = data;
    return status;
}

/*****************************************************************************
* FUNCTION NAME: util_cm_register_write
*
* DESCRIPTION:  implementation of c&m register write for pdl
*
*
*****************************************************************************/
static GT_STATUS util_cm_register_write
(
    IN  GT_U8       devNum,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    IN  GT_U32      data
)
{
    GT_U32          oldValue, newValue;
    GT_STATUS       status;
    status = cpssDrvPpHwInternalPciRegRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, &oldValue);
    if (status == GT_OK)
    {
        newValue = (~mask & oldValue) | (mask  & data);
        status = cpssDrvPpHwInternalPciRegWrite (devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, newValue);
    }
    return status;
}

/*****************************************************************************
* FUNCTION NAME: util_register_read
*
* DESCRIPTION: implementation of register read for pdl
*
*
*****************************************************************************/
static GT_STATUS util_register_read
(
    IN  GT_U8       devNum,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    OUT GT_U32    * dataPtr
)
{
    GT_U32    data;
    GT_STATUS status;
    status = cpssDrvPpHwRegisterRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, &data);
    data &= mask;
    *dataPtr = data;
    return status;
}

/*****************************************************************************
* FUNCTION NAME: util_register_write
*
* DESCRIPTION:          implementation of register write for pdl
*
*
*****************************************************************************/
static GT_STATUS util_register_write
(
    IN  GT_U8       devNum,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    IN  GT_U32      data
)
{
    GT_U32     oldValue, newValue;
    GT_STATUS  status;
    status = cpssDrvPpHwRegisterRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, &oldValue);
    if (status == GT_OK)
    {
        newValue = (~mask & oldValue) | (mask  & data);
        status = cpssDrvPpHwRegisterWrite (devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, newValue);
    }
    return status;
}
/*****************************************************************************
* FUNCTION NAME: util_malloc
*
* DESCRIPTION:         implementation of malloc for pdl
*
*
*****************************************************************************/
static void * util_malloc
(
    SIZE_T  size
)
{
    return cpssOsMalloc(size);
}
static GT_STATUS util_pdlXsmiRegRead (
    IN  GT_U8      devIdx,
    IN  GT_U32     xsmiInterface,
    IN  GT_U32     xsmiAddr,
    IN  GT_U32     regAddr,
    IN  GT_U32     phyDev,
    OUT GT_U16     *dataPtr
)
{
    return cpssXsmiRegisterRead(devIdx,xsmiInterface,xsmiAddr,regAddr,phyDev,dataPtr);
}

static GT_STATUS util_pdlXsmiRegWrite(
    IN  GT_U8      devIdx,
    IN  GT_U32     xsmiInterface,
    IN  GT_U32     xsmiAddr,
    IN  GT_U32     regAddr,
    IN  GT_U32     phyDev,
    IN  GT_U16     data
)
{
    return cpssXsmiRegisterWrite(devIdx,xsmiInterface,xsmiAddr,regAddr,phyDev,data);
}

/*****************************************************************************
* FUNCTION NAME: util_archive_uncompress
*
* DESCRIPTION:  Uncompress xml and md5 files from archive.
*
*
*****************************************************************************/
static BOOLEAN util_archive_uncompress(
    IN  char  * ar_filename_PTR,
    OUT char * xml_filename_PTR,
    OUT char * signature_filename_PTR        /* ignored */
)
{
    char   *tmpStr;
    FILE   *xmlFilePtr;
    if (ar_filename_PTR == NULL || xml_filename_PTR == NULL)
    {
        cpssOsPrintf("No filename string was supplied\n");
        return FALSE;
    }
#ifdef ASIC_SIMULATION
    cpssOsPrintf("looking for file: [%s %s] \n",
        simFSiniFileDirectory,
        ar_filename_PTR);
#endif /*ASIC_SIMULATION*/
    xml_filename_PTR[0] = '\0';
    /* already an XML file */
    if (strstr(ar_filename_PTR, ".xml") != NULL) {
        strcpy(xml_filename_PTR, ar_filename_PTR);
        strcpy(signature_filename_PTR, ar_filename_PTR);
        tmpStr = strstr(signature_filename_PTR, ".xml");
        if (!tmpStr)
        {
            cpssOsPrintf("Can't change md5 filename string content\n");
            return FALSE;
        }
        cpssOsSprintf(tmpStr, ".md5");
        xmlFilePtr = fopen(xml_filename_PTR, "rb");
        if (!xmlFilePtr)
        {
            cpssOsPrintf("File [%s] not found \n",xml_filename_PTR);
            return FALSE;
        }
        fclose(xmlFilePtr);
        return TRUE;
    }

    return FALSE;
}


/*****************************************************************************
* FUNCTION NAME: utils_pdl_init
*
* DESCRIPTION:         init the pdl utility
*
*
*****************************************************************************/
static PDL_STATUS utils_pdl_init
(
    IN char*       fileNamePtr,
    IN char*       xmlPrefix
)
{
    PDL_STATUS                      pdlStatus;
    PDL_OS_CALLBACK_API_STC         callbacks;

    cpssOsMemSet(&callbacks,0,sizeof(callbacks));

    callbacks.printStringPtr        = cpssOsPrintf;
    callbacks.mallocPtr             = util_malloc;
    callbacks.freePtr               = cpssOsFree;
    callbacks.smiRegReadClbk        = util_smi_read;
    callbacks.smiRegWriteClbk       = util_smi_write;
    callbacks.xsmiRegReadClbk       = util_pdlXsmiRegRead;
    callbacks.xsmiRegWriteClbk      = util_pdlXsmiRegWrite;
    callbacks.ppCmRegReadClbk       = util_cm_register_read;
    callbacks.ppCmRegWriteClbk      = util_cm_register_write;
    callbacks.ppRegReadClbk         = util_register_read;
    callbacks.ppRegWriteClbk        = util_register_write;
    callbacks.arXmlUncompressClbk   = util_archive_uncompress;
    callbacks.vrXmlVerifyClbk       = NULL;

    pdlStatus = pdlInit (fileNamePtr, xmlPrefix, &callbacks, PDL_OS_INIT_TYPE_FULL_E);
    if (pdlStatus == PDL_OK)
    {
       pdlStatus = iDbgPdlInit(&callbacks,"tests");
       if (pdlStatus != PDL_OK)
       {
           cpssOsPrintf("Can't initialize. iDbgPdlInit returned #[%d]", pdlStatus);
       }

    }
    else
    {
        cpssOsPrintf("Can't initialize. pdlInit returned #[%d]", pdlStatus);
    }

    pdlInitDone();

    return pdlStatus;
}

/**
* @internal ezbXmlLoad function
* @endinternal
*
* @brief  load XML for EZ_BRINGUP about the board.
*         if already loaded , the operation will destroy the previous one and then load the new one.
*
* @retval
*/
GT_STATUS ezbXmlLoad(
    IN char*       fileNamePtr
)
{
    PDL_STATUS pdl;

    if(xmlLoaded == GT_TRUE)
    {
        /* already loaded so first destroy the old one */
        ezbXmlDestroy();
    }

    pdl = utils_pdl_init(fileNamePtr,"mtsplt");
    if(pdl != PDL_OK)
    {
        cpssOsPrintf(" -- ezb : FAILED to load [%s] \n",fileNamePtr);
        return GT_FAIL;
    }

    cpssOsPrintf(" -- ezb : Successfully loaded [%s] \n",fileNamePtr);
#ifdef INCLUDE_MPD
    /* PHY init */
    if (GT_OK != prvEzbMpdLoad())
    {
        cpssOsPrintf(" -- ezb : ERROR : Can't initialize PHY. prvAppDemoEzbMpdLoad failed \n");
        return GT_FAIL;
    }
#else /*!INCLUDE_MPD*/
    cpssOsPrintf(" -- ezb  : NOTE : Can't initialize PHY. 'MPD' was not compiled # \n");
#endif /*!INCLUDE_MPD*/
    xmlLoaded = GT_TRUE;
    xmlFileName = cpssOsMalloc(cpssOsStrlen(fileNamePtr) + 1);
    cpssOsStrCpy(xmlFileName,fileNamePtr);

    cpssOsPrintf(" -- ezb : Successfully loaded [%s] \n",fileNamePtr);

    return GT_OK;
}

/**
* @internal ezbXmlDestroy function
* @endinternal
*
* @brief  unload XML for EZ_BRINGUP .
*
*
* @retval
*/
GT_STATUS  ezbXmlDestroy(GT_VOID)
{
    PDL_STATUS pdl;

    xmlLoaded = GT_FALSE;
    FREE_PTR_MAC(xmlFileName);

    pdl = pdlDestroy();
    if(pdl != PDL_OK)
    {
        return GT_FAIL;
    }

    return GT_OK;
}
#endif
