/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file mvHwsPortModeElements.c
*/

#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>

#if defined CHX_FAMILY && !defined(MV_HWS_REDUCED_BUILD)
    #include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
    #include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwFalconInfo.h>
#endif /*CHX_FAMILY && !MV_HWS_REDUCED_BUILD */

#ifndef MV_HWS_REDUCED_BUILD

/*#define HWS_ELEMENTS_DEBUG_OPENED*/
#ifdef HWS_ELEMENTS_DEBUG_OPENED

#ifdef STR
    #undef STR
#endif /*STR*/

#define STR(strname)    \
    #strname

#define PORT_MODE_NAMES         \
    STR(_100Base_FX),               \
    STR(SGMII),               \
    STR(_1000Base_X),               \
    STR(SGMII2_5),               \
    STR(QSGMII),               \
    STR(_10GBase_KX4),               \
    STR(_10GBase_KX2),               \
    STR(_10GBase_KR),               \
    STR(_20GBase_KR2),               \
    STR(_40GBase_KR),               \
    STR(_100GBase_KR10),               \
    STR(HGL),               \
    STR(RHGL),               \
    STR(CHGL),               \
    STR(RXAUI),               \
    STR(_20GBase_KX4),               \
    STR(_10GBase_SR_LR),               \
    STR(_20GBase_SR_LR2),               \
    STR(_40GBase_SR_LR),               \
    STR(_12_1GBase_KR),               \
    STR(XLHGL_KR4),               \
    STR(HGL16G),               \
    STR(HGS),               \
    STR(HGS4),               \
    STR(_100GBase_SR10),               \
    STR(CHGL_LR12),               \
    STR(TCAM),               \
    STR(INTLKN_12Lanes_6_25G),               \
    STR(INTLKN_16Lanes_6_25G),               \
    STR(INTLKN_24Lanes_6_25G),               \
    STR(INTLKN_12Lanes_10_3125G),               \
    STR(INTLKN_16Lanes_10_3125G),               \
    STR(INTLKN_12Lanes_12_5G),               \
    STR(INTLKN_16Lanes_12_5G),               \
    STR(INTLKN_16Lanes_3_125G),               \
    STR(INTLKN_24Lanes_3_125G),               \
    STR(CHGL11_LR12),               \
    STR(INTLKN_4Lanes_3_125G),               \
    STR(INTLKN_8Lanes_3_125G),               \
    STR(INTLKN_4Lanes_6_25G),               \
    STR(INTLKN_8Lanes_6_25G),               \
    STR(_2_5GBase_QX),               \
    STR(_5GBase_DQX),               \
    STR(_5GBase_HX),               \
    STR(_12GBaseR),               \
    STR(_5_625GBaseR),               \
    STR(_48GBaseR4),               \
    STR(_12GBase_SR),               \
    STR(_48GBase_SR4),               \
    STR(_5GBaseR),               \
    STR(_22GBase_SR),               \
    STR(_24GBase_KR2),               \
    STR(_12_5GBase_KR),               \
    STR(_25GBase_KR2),               \
    STR(_50GBase_KR4),               \
    STR(_25GBase_KR),               \
    STR(_50GBase_KR2),               \
    STR(_100GBase_KR4),               \
    STR(_25GBase_SR),               \
    STR(_50GBase_SR2),               \
    STR(_100GBase_SR4),               \
    STR(_100GBase_MLG),               \
    STR(_107GBase_KR4),               \
    STR(_25GBase_KR_C),               \
    STR(_50GBase_KR2_C),               \
    STR(_40GBase_KR2),               \
    STR(_29_09GBase_SR4),               \
    STR(_40GBase_CR4),               \
    STR(_25GBase_CR),               \
    STR(_50GBase_CR2),               \
    STR(_100GBase_CR4),               \
    STR(_25GBase_KR_S),               \
    STR(_25GBase_CR_S),               \
    STR(_25GBase_CR_C),               \
    STR(_50GBase_CR2_C),               \
    STR(_50GBase_KR),               \
    STR(_100GBase_KR2),               \
    STR(_200GBase_KR4),               \
    STR(_200GBase_KR8),               \
    STR(_400GBase_KR8),               \
    STR(_102GBase_KR4),               \
    STR(_52_5GBase_KR2),               \
    STR(_40GBase_KR4),               \
    STR(_26_7GBase_KR),               \
    STR(_40GBase_SR_LR4),               \
    STR(_50GBase_CR),               \
    STR(_100GBase_CR2),               \
    STR(_200GBase_CR4),               \
    STR(_200GBase_CR8),               \
    STR(_400GBase_CR8),               \
    STR(_200GBase_SR_LR4),               \
    STR(_200GBase_SR_LR8),               \
    STR(_400GBase_SR_LR8),               \
    STR(_50GBase_SR_LR),               \
    STR(_100GBase_SR_LR2),               \
    STR(_20GBase_KR),               \
    STR(_106GBase_KR4),               \
    STR(_2_5G_SXGMII),               \
    STR(_5G_SXGMII),               \
    STR(_10G_SXGMII),               \
    STR(_5G_DXGMII),               \
    STR(_10G_DXGMII),               \
    STR(_20G_DXGMII),               \
    STR(_5G_QUSGMII),               \
    STR(_10G_QXGMII),               \
    STR(_20G_QXGMII),               \
    STR(_10G_OUSGMII),               \
    STR(_20G_OXGMII),               \
    STR(_42GBase_KR4),              \
    STR(_53GBase_KR2),              \
    STR(_424GBase_KR8),             \
    STR(_2500Base_X),              \
    STR(NON_SUP_MODE)

const char *portModeNames[LAST_PORT_MODE+1] = {PORT_MODE_NAMES};

#define MAC_TYPE_NAMES      \
    STR(MAC_NA),       \
    STR(GEMAC_X),   \
    STR(GEMAC_SG),   \
    STR(XGMAC),   \
    STR(XLGMAC),   \
    STR(HGLMAC),   \
    STR(CGMAC),   \
    STR(INTLKN_MAC),   \
    STR(TCAM_MAC),   \
    STR(GEMAC_NET_X),   \
    STR(GEMAC_NET_SG),   \
    STR(QSGMII_MAC),   \
    STR(D2D_MAC),   \
    STR(MTI_MAC_100),   \
    STR(MTI_MAC_400),   \
    STR(MTI_CPU_MAC),   \
    STR(MTI_USX_MAC),   \
    STR(MTI_MAC_100_BR), \
    STR(LAST_MAC)

const char *macTypeNames[LAST_MAC+1] = {MAC_TYPE_NAMES};

#define PCS_TYPE_NAMES      \
    STR(PCS_NA),       \
    STR(GPCS),       \
    STR(XPCS),       \
    STR(MMPCS),       \
    STR(HGLPCS),       \
    STR(CGPCS),       \
    STR(INTLKN_PCS),       \
    STR(TCAM_PCS),       \
    STR(GPCS_NET),       \
    STR(QSGMII_PCS),       \
    STR(D2D_PCS),       \
    STR(MTI_PCS_50),       \
    STR(MTI_PCS_100),       \
    STR(MTI_PCS_200),       \
    STR(MTI_PCS_400),       \
    STR(MTI_PCS_LOW_SPEED),       \
    STR(MTI_CPU_PCS),       \
    STR(MTI_USX_PCS),       \
    STR(MTI_USX_PCS_LOW_SPEED),       \
    STR(MTI_PCS_25),       \
    STR(LAST_PCS)

const char *pcsTypeNames[LAST_PCS+1] = {PCS_TYPE_NAMES};

#endif /* HWS_ELEMENTS_DEBUG_OPENED */
#endif

#ifdef MV_HWS_REDUCED_BUILD
/* Array of Devices - each device is an array of ports */
MV_HWS_DEVICE_PORTS_ELEMENTS hwsDevicesPortsElementsArray[HWS_MAX_DEVICE_NUM] = { 0 };
#endif

const MV_HWS_PORT_INIT_PARAMS hwsSingleEntryInitParams = {
    /* port mode,   MAC Type    Mac Num    PCS Type    PCS Num    FEC       Speed      FirstLane LanesList LanesNum  Media         10Bit      FecSupported */
     NON_SUP_MODE,  MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,   FEC_OFF,   SPEED_NA,  NA_NUM,   {0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON,  FEC_NA};

/**
* @internal hwsInitPortsModesParam function
* @endinternal
*
* @brief   Registers silicon ports modes parameters in the system.
*         Calls ones per active silicon type.
* @param[in] devType                  - silicon type.
* @param[in] portModes                - pointer to array of ports modes parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsInitPortsModesParam
(
    IN GT_U8                   devNum,
    IN MV_HWS_PORT_INIT_PARAMS *portModes[]
)
{
    MV_HWS_DEV_TYPE                 devType = HWS_DEV_SILICON_TYPE(devNum);
    MV_HWS_DEVICE_PORTS_ELEMENTS    curDevicePorts;
    GT_U32                          numOfPorts = hwsDeviceSpecInfo[devNum].portsNum;
    GT_U32                          portNum;
#ifndef MV_HWS_REDUCED_BUILD
    GT_U32                          lastSupMode;
#endif
    if (devType == Lion2B0 || devType == Lion2A0 || devType == HooperA0)
    {
        numOfPorts = 14;
    }
    /* Allocate ports array and save it to the general devices array*/
    curDevicePorts = (MV_HWS_DEVICE_PORTS_ELEMENTS)hwsOsMallocFuncPtr(numOfPorts*sizeof(MV_HWS_SINGLE_PORT_STC));
    if (NULL == curDevicePorts)
    {
      return GT_NO_RESOURCE;
    }
    hwsDevicesPortsElementsArray[devNum] = curDevicePorts;

    for (portNum = 0; portNum < numOfPorts ; portNum++)
    {
#ifndef MV_HWS_REDUCED_BUILD
        curDevicePorts[portNum].perModeFecList = NULL;
        curDevicePorts[portNum].supModesCatalog = NULL;
        curDevicePorts[portNum].legacySupModesCatalog = NULL;
#endif
        hwsOsMemCopyFuncPtr(&curDevicePorts[portNum].curPortParams, &hwsSingleEntryInitParams, sizeof(MV_HWS_PORT_INIT_PARAMS));
    }
#ifndef MV_HWS_REDUCED_BUILD
    for (portNum = 0; (NULL != portModes) && (portNum < numOfPorts) ; portNum++)
    { /* copy defualts portParams if configured in the **portModes array */
        if (portModes[portNum] != NULL)
        {
            lastSupMode = hwsDeviceSpecInfo[devNum].lastSupPortMode;
            curDevicePorts[portNum].legacySupModesCatalog = (MV_HWS_PORT_INIT_PARAMS*)hwsOsMallocFuncPtr((1+lastSupMode)*sizeof(MV_HWS_PORT_INIT_PARAMS));
            if (NULL == curDevicePorts[portNum].legacySupModesCatalog)
            {
              return GT_NO_RESOURCE;
            }
            hwsOsMemCopyFuncPtr(curDevicePorts[portNum].legacySupModesCatalog, portModes[portNum], (1+lastSupMode)*sizeof(MV_HWS_PORT_INIT_PARAMS));
        }
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsBuildActiveLaneList function
* @endinternal
*
* @brief   Get SD vector.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - physical port number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBuildActiveLaneList
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portNum,
    IN MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS *curPortParams;

    curPortParams = hwsPortModeParamsGet(devNum, portGroup, portNum, portMode,NULL);
    if (curPortParams == NULL || curPortParams->numOfActLanes == 0)
    {
      return GT_NOT_SUPPORTED;
    }
    return hwsPortParamsGetLanes(devNum,portGroup,portNum,curPortParams);
}

/**
* @internal mvHwsRebuildActiveLaneList function
* @endinternal
*
* @brief   Get SD vector and rebuild active lane array. Can be called during CreatePort
*         only.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] laneList                -  lane list
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsRebuildActiveLaneList
(
    IN GT_U8                 devNum,
    IN GT_U32                portGroup,
    IN GT_U32                portNum,
    IN MV_HWS_PORT_STANDARD  portMode,
    OUT GT_U32                *laneList
)
{
    GT_STATUS rc = GT_OK;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 sdVec, tmp, startSerdes;
    GT_U32 laneCtnt;

    rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams);
    if (GT_OK != rc)
    {
        return rc;
    }

    if (curPortParams.numOfActLanes != 0)
    {
        CHECK_STATUS(mvHwsRedundancyVectorGet(devNum, portGroup, &sdVec));

        startSerdes = curPortParams.activeLanesList[0];
        tmp = (sdVec >> startSerdes) & 0xF;
        for (laneCtnt = 0; (laneCtnt < curPortParams.numOfActLanes); laneCtnt++)
        {
            laneList[laneCtnt] = curPortParams.activeLanesList[laneCtnt];
            if (tmp & 1)
            {
                laneList[laneCtnt]++;
            }
            tmp = tmp >> 1;
        }
    }
    else
    if(laneList)
    {
        /* it seems that there is code that check value of laneList[0]
           without checking for numOfActLanes */
        laneList[0] = 0;/* so put here predictable value */
    }

    return GT_OK;
}
#if 0
/**
* @internal mvHwsDestroyActiveLaneList function
* @endinternal
*
* @brief   Free lane list allocated memory.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - physical port number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsDestroyActiveLaneList
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portNum,
    IN MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS *curPortParams;
    GT_U8 i;

    curPortParams = hwsPortModeParamsGet(devNum, portGroup, portNum, portMode);
    if ((curPortParams != NULL) && (curPortParams->activeLanesList != NULL))
    {
      for (i = 0; i < 4; i++)
        curPortParams->activeLanesList[i] = 0;
    }

    return GT_OK;
}
#endif
/**
* @internal mvHwsBuildDeviceLaneList function
* @endinternal
*
* @brief   Build the device lane list per port and per mort mode.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBuildDeviceLaneList
(
   IN GT_U8    devNum,
   IN GT_U32   portGroup
)
{
    GT_U32    portNum;
#ifndef MV_HWS_REDUCED_BUILD
    GT_U32    portMode;
    GT_STATUS res;

    /* init active lines in ports DB */
    for (portNum = 0; portNum < HWS_CORE_PORTS_NUM(devNum); portNum++)
    {
        for (portMode = 0; portMode < HWS_DEV_PORT_MODES(devNum); portMode++)
        {
            res = mvHwsBuildActiveLaneList(devNum, portGroup, portNum, (MV_HWS_PORT_STANDARD)portMode);
            if((res != GT_OK) && (res != GT_NOT_SUPPORTED))
            {
                return res;
            }
        }
    }
#else
    /* Internal CPU receives the port parameters from the Host so no need to hold table per port mode */
    for (portNum = 0; portNum < HWS_CORE_PORTS_NUM(devNum); portNum++)
    {
        CHECK_STATUS(mvHwsBuildActiveLaneList(devNum, portGroup, portNum, (MV_HWS_PORT_STANDARD)0 /*portMode*/));
    }
#endif

    return GT_OK;
}

#if 0
/**
* @internal mvHwsDestroyDeviceLaneList function
* @endinternal
*
* @brief   Free the device lane list allocated memory.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsDestroyDeviceLaneList
(
   IN GT_U8    devNum,
   IN GT_U32   portGroup
)
{
    GT_U32  portNum;
    GT_U32  portMode;

    /* free active lines allocated memory in ports DB */
    for (portNum = 0; portNum < HWS_CORE_PORTS_NUM(devNum); portNum++)
    {
        for (portMode = 0; portMode < HWS_DEV_PORT_MODES(devNum); portMode++)
        {
            CHECK_STATUS(mvHwsDestroyActiveLaneList(devNum, portGroup, portNum, (MV_HWS_PORT_STANDARD)portMode));
        }
    }

    return GT_OK;
}
#endif
#ifndef CM3
/**
* @internal hwsExtendedPortNumGet function
* @endinternal
*
* @brief   it port in extended mode - returns the extended port
*          number, otherwise return the same port num.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[out] isExtended              - true if is extended
* @param[in/out] portNum              - port num
*
* @retval 0                        - on success
* @retval 1                        - on error
**/
GT_STATUS hwsExtendedPortNumGet
(
   IN    GT_U8    devNum,
   IN    GT_U32   portGroup,
   OUT   GT_BOOL  *isExtended,
   INOUT GT_U32   *portNum
)
{
    MV_HWS_DEV_TYPE  devType = HWS_DEV_SILICON_TYPE(devNum);
    GT_BOOL extMode = GT_FALSE;
    GT_U32 orgPortNum;

    if (devNum >= HWS_MAX_DEVICE_NUM)
        return GT_FAIL;

    if ( portNum == NULL )
    {
        return GT_FAIL;
    }
    orgPortNum = *portNum;
    /* In Alleycat3: check if Ports #25 and #27 are in Extended mode by checking the
       EXTERNAL_CONTROL register value. If the port is in Extended mode, the related
       PortsParamsArray for Extended port will be #28/#29 */
    if ( (devType == Alleycat3A0) || (devType == AC5) || (HWS_DEV_GOP_REV(devNum) == GOP_40NM_REV1) )
    {
        if (mvHwsPortExtendedModeCfgGet(devNum, portGroup, orgPortNum, _10GBase_KR, &extMode) != GT_OK)
        {
            return GT_FAIL;
        }
        if (extMode == GT_TRUE)
        {
            if ( (devType == Alleycat3A0) || (devType == AC5) )
            {
                if (orgPortNum == 25)
                {
                    *portNum = 28;
                }
                else if (orgPortNum == 27)
                {
                    *portNum = 29;
                }
            }
            else if (HWS_DEV_GOP_REV(devNum) == GOP_40NM_REV1)
            {
                if (orgPortNum == 9)
                {
                    *portNum = 12; /* port 12 entry contains Port 9 externded mode parameters */
                }
                else if (orgPortNum == 11)
                {
                    *portNum = 13; /* port 13 entry contains Port 11 externded mode parameters */
                }
            }
        }
    }
    if (NULL != isExtended)
    {
        *isExtended = extMode;
    }
    return GT_OK;
}
#endif /* CM3 */

/**
* @internal hwsPortsParamsArrayGet function
* @endinternal
*
* @brief   port params array get.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                - port num
*
* @retval MV_HWS_PORT_INIT_PARAMS* - pointer to the
*                                    *ORIGINAL* entry in the
*                                    elements database
* @retval NULL -                     on error/not supported mode
*/
#ifndef MV_HWS_REDUCED_BUILD

MV_HWS_PORT_INIT_PARAMS *hwsPortsParamsArrayGet
(
   IN GT_U8    devNum,
   IN GT_U32   portGroup,
   IN GT_U32   portNum
)
{
    MV_HWS_DEVICE_PORTS_ELEMENTS tmp = NULL;
    MV_HWS_DEV_TYPE  devType = HWS_DEV_SILICON_TYPE(devNum);
    MV_HWS_PORT_INIT_PARAMS *portParams;

    if (devNum >= HWS_MAX_DEVICE_NUM)
        return NULL;

#ifndef CM3
    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &portNum) != GT_OK )
    {
        return NULL;
    }
#endif /* CM3 */

    tmp = (MV_HWS_DEVICE_PORTS_ELEMENTS)hwsDevicesPortsElementsArray[devNum];

    if (tmp == NULL)
    {
        return NULL;
    }
    /* Avoid warning */
    devType = devType;

    if((HWS_DEV_SIP_5_15_CHECK_MAC(devNum)) || (HWS_DEV_SILICON_TYPE(devNum) == AC5))
    {

        portParams = (MV_HWS_PORT_INIT_PARAMS *)hwsDevicePortsElementsSupModesCatalogGet(devNum,portNum);
    }
    else
    {
        portParams = tmp[portNum].legacySupModesCatalog;
    }
    return portParams;
}
#endif
#if 0 /*Not used (debug?)*/
GT_U32 hwsPortModeParamsIndexGet
(
   GT_U8                    devNum,
   GT_U32                   portGroup,
   GT_U32                   portNum,
   MV_HWS_PORT_STANDARD     portMode
)
{
#ifndef MV_HWS_REDUCED_BUILD
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    /* get func DB */
    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsIndexGetFunc != 0)
    {
        return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsIndexGetFunc(devNum, portGroup, portNum, portMode);
    }
    else
    {
        return portMode;
    }
#else
    return 0;
#endif
}
#endif

/**
* @internal hwsPortModeParamsSetMode function
* @endinternal
*
* @brief   port mode params set.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                - port num
* @param[in] portMode                - Different port standard
*       metrics
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortModeParamsSetMode
(
   IN GT_U8                    devNum,
   IN GT_U32                   portGroup,
   IN GT_U32                   portNum,
   IN MV_HWS_PORT_STANDARD     portMode
)
{
#ifndef MV_HWS_REDUCED_BUILD
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    /* get func DB */
    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);
    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsSetFunc != 0)
    {
        return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsSetFunc(devNum, portGroup, portNum, portMode);
    }
    else
    {
        MV_HWS_DEVICE_PORTS_ELEMENTS    curDevicePorts = hwsDevicesPortsElementsArray[devNum];
        MV_HWS_PORT_INIT_PARAMS         *curPortParams;
        const MV_HWS_PORT_INIT_PARAMS   *newPortParams;
        GT_U32                          portModeIndex;

        /* avoid warnings */
        portGroup = portGroup;

        if (portNum > hwsDeviceSpecInfo[devNum].portsNum)
        {
            return GT_OUT_OF_RANGE;
        }

        curPortParams = &curDevicePorts[portNum].curPortParams;
        if (NON_SUP_MODE == portMode) /* Reset port */
        {
            /* initialize entry with default port params */
            hwsOsMemCopyFuncPtr(curPortParams, &hwsSingleEntryInitParams, sizeof(MV_HWS_PORT_INIT_PARAMS));
            curPortParams->portMacNumber = (GT_U16)portNum;
            curPortParams->portPcsNumber = (GT_U16)portNum;
            curPortParams->firstLaneNum  = (GT_U16)portNum;
            return GT_OK;
        }

        newPortParams = hwsPortsElementsFindMode(curDevicePorts[portNum].supModesCatalog,portMode,&portModeIndex);
        if ( (NULL == newPortParams) || (newPortParams->numOfActLanes == 0)
           ||  ((_100GBase_MLG != portMode) && (portNum % newPortParams->numOfActLanes) != 0))
        {
            return GT_NOT_SUPPORTED;
        }
        /* Copy the right port mode line from the port's specific supported modes list*/
        hwsOsMemCopyFuncPtr(curPortParams, newPortParams, sizeof(MV_HWS_PORT_INIT_PARAMS));
        curPortParams->portMacNumber = (GT_U16)portNum;
        curPortParams->portPcsNumber = (GT_U16)portNum;
        curPortParams->firstLaneNum  = (GT_U16)portNum;
        curPortParams->portFecMode   = curDevicePorts[portNum].perModeFecList[portModeIndex];

        /* QSGMII is single lane port mode with special Serdes mapping */
        if((curPortParams->numOfActLanes == 1) && (portMode == QSGMII))
        {
            curPortParams->firstLaneNum = (GT_U16)(portNum & 0xFFFFFFFC);
            if(portNum % 4 != 0)
            {
                curPortParams->numOfActLanes = 0;
                return GT_OK;
            }
        }

        /* _100GBase_MLG is 4 lanes port mode with special Serdes mapping: 4 lanes are bound to all
           four ports in the GOP, but only first port number in the GOP has 4 active lanes.
           For other free ports in GOP, the numOfActLanes should change from NA_NUM to 0 */
        if((curPortParams->numOfActLanes == 4) && (portMode == _100GBase_MLG))
        {
            curPortParams->firstLaneNum = (GT_U16)(portNum & 0xFFFFFFFC);
            if(portNum % 4 != 0)
            {
                curPortParams->numOfActLanes = 0;
                return GT_OK;
            }
        }

        /* build active lane list */
        CHECK_STATUS(hwsPortParamsGetLanes(devNum,portGroup,portNum,curPortParams));
        return GT_OK;
    }
#else
    return GT_OK;
#endif
}

/**
* @internal hwsPortModeParamsSetFec function
* @endinternal
*
* @brief   port mode params set function.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                - port num
* @param[in] portMode                - Different port standard
*       metrics
* @param[in] fecMode                - FEC status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortModeParamsSetFec
(
   IN GT_U8                    devNum,
   IN GT_U32                   portGroup,
   IN GT_U32                   portNum,
   IN MV_HWS_PORT_STANDARD     portMode,
   IN MV_HWS_PORT_FEC_MODE     fecMode
)
{
#ifndef MV_HWS_REDUCED_BUILD
    GT_U32                  portModeIndex;
    MV_HWS_SINGLE_PORT_STC  *curPortStruct;
    portGroup = portGroup;

    if (devNum >= HWS_MAX_DEVICE_NUM || portNum >= hwsDeviceSpecInfo[devNum].portsNum ||
         NULL == hwsDevicesPortsElementsArray[devNum])
    {
        return GT_BAD_PARAM;
    }
    if ( (HWS_DEV_SIP_5_15_CHECK_MAC(devNum)) || (HWS_DEV_SILICON_TYPE(devNum) == AC5) )
    {
        if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &portNum ) != GT_OK)
        {
            return GT_FAIL;
        }

        curPortStruct =  &((hwsDevicesPortsElementsArray[devNum])[portNum]);
        if(NULL == hwsPortsElementsFindMode(hwsDevicePortsElementsSupModesCatalogGet(devNum,portNum),
                                           portMode,
                                           &portModeIndex)) /* Get the port mode index */
        {
            return GT_NOT_SUPPORTED;
        }

        curPortStruct->perModeFecList[portModeIndex] = fecMode;
    }
    else
#endif
    {
        MV_HWS_PORT_INIT_PARAMS *tmpPortParams = hwsPortModeParamsGet(devNum,portGroup,portNum,portMode,NULL);
        if ( tmpPortParams != NULL )
        {
            tmpPortParams->portFecMode = fecMode;
        }
        else
            return GT_FAIL;
    }
    return GT_OK;
}

/**
* @internal hwsPortModeParamsGet function
* @endinternal
*
* @brief   USING THIS FUNCTION MAY CORRUPT THE ELEMENTS
*          DATABASES. use hwsPortModeParamsGetToBuffer instead
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                - port num
* @param[in] portMode                - Different port standard
*       metrics
*
* @retval MV_HWS_PORT_INIT_PARAMS* - pointer to the
*                                    *ORIGINAL* entry in the
*                                    elements database
* @retval NULL -                     on error/not supported mode
*/
MV_HWS_PORT_INIT_PARAMS *hwsPortModeParamsGet
(
   IN GT_U8                    devNum,
   IN GT_U32                   portGroup,
   IN GT_U32                   portNum,
   IN MV_HWS_PORT_STANDARD     portMode,
   OUT GT_U32                   *portModeIndexPtr
)
{
    MV_HWS_DEVICE_PORTS_ELEMENTS curDevicePorts = HWS_DEV_PORTS_ARRAY(devNum);

#ifndef MV_HWS_REDUCED_BUILD
    MV_HWS_PORT_INIT_PARAMS         *retPortParams = NULL;
    GT_U32                          portModeIndex = portMode;
    GT_U32                          portToCheck = portNum;
    MV_HWS_HAWK_CONVERT_STC         convertIdx;
    GT_STATUS                       rc;

    /* SIP_5_15 and above (and AC5) - From BobK and on, each port has one line entry of its own current params (single MV_HWS_PORT_INIT_PARAMS) */
    if ( (HWS_DEV_SIP_5_15_CHECK_MAC(devNum)) || (HWS_DEV_SILICON_TYPE(devNum) == AC5) )
    {
        if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &portNum ) != GT_OK)
        {
            return NULL;
        }

        if (curDevicePorts[portNum].curPortParams.portStandard == portMode || portMode == NON_SUP_MODE)
        {
            retPortParams = &curDevicePorts[portNum].curPortParams; /* CURRENT PORT'S VALUES! (not the portMode's defaults) */
        }
        else
        { /* if hwsPortModeParamsGet is used to retrieve portMode's values other than the current port's values */
            if (NULL != PRV_PORTS_PARAMS_SUP_MODE_MAP(devNum)) /* if this is a valid port */
            {   /* find defualt values of the requested portMode */
                retPortParams = (MV_HWS_PORT_INIT_PARAMS *)hwsPortsElementsFindMode(
                        hwsDevicePortsElementsSupModesCatalogGet(devNum,portNum),
                        portMode,&portModeIndex); /* NULL if mode not supported */
                if (HWS_DEV_SIP_6_10_CHECK_MAC(devNum))
                {
                    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
                    if(rc != GT_OK)
                    {
                        return NULL;
                    }
                    portToCheck = convertIdx.ciderIndexInUnit;
                }
                else if (HWS_DEV_SILICON_TYPE(devNum) == AC5)
                {
                    if (portNum > 23)
                    {
                        portToCheck = 0;
                    }
                }

                if((retPortParams != NULL) && (_100GBase_MLG != portMode) && (portToCheck % retPortParams->numOfActLanes) != 0)
                {
                    retPortParams = NULL;
                }

                if(portModeIndexPtr)
                {
                    *portModeIndexPtr = portModeIndex;
                }
            }
        }
    }
/* Before SIP_5_15 - each port has its own copy of the entire list of all the port modes (array of MV_HWS_PORT_INIT_PARAMS's) */
    else
    {
        retPortParams = hwsPortsParamsArrayGet(devNum, portGroup, portNum);
        if (retPortParams != NULL && retPortParams[portMode].portMacNumber != NA_NUM)
        {
            retPortParams = &retPortParams[portMode];
        }
        else
        {
            retPortParams = NULL;
        }
    }
    return retPortParams;
#else
    return &curDevicePorts[portNum].curPortParams;
#endif
}

/**
* @internal hwsPortModeParamsSetFec function
* @endinternal
*
* @brief   port mode params get to buffer.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                - port num
* @param[in] portMode                - Different port standard
*       metrics
*
* @param[out] portParamsBuffer      - Port Init parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortModeParamsGetToBuffer
(
   IN GT_U8                    devNum,
   IN GT_U32                   portGroup,
   IN GT_U32                   portNum,
   IN MV_HWS_PORT_STANDARD     portMode,
   OUT MV_HWS_PORT_INIT_PARAMS  *portParamsBuffer
)
{
#ifndef MV_HWS_REDUCED_BUILD
    MV_HWS_DEVICE_PORTS_ELEMENTS    curDevicePorts = HWS_DEV_PORTS_ARRAY(devNum);
    GT_U32                          portModeIndex = portMode;
    MV_HWS_PORT_INIT_PARAMS         *srcPortParams = hwsPortModeParamsGet(devNum, portGroup, portNum, portMode,&portModeIndex);
#else
    MV_HWS_PORT_INIT_PARAMS         *srcPortParams = hwsPortModeParamsGet(devNum, portGroup, portNum, portMode,NULL);
#endif

    if (NULL == srcPortParams || NULL == portParamsBuffer)
    {
        return GT_FAIL;
    }

    /* copy the port parameters into the buffer*/
    hwsOsMemCopyFuncPtr(portParamsBuffer, srcPortParams, sizeof(MV_HWS_PORT_INIT_PARAMS));

#ifndef MV_HWS_REDUCED_BUILD
    /* for SIP_5_15 (and AC5) - fix the active lanes list (needed if hwsPortModeParamsGet returned the defualts port mode params) */
    if ( (HWS_DEV_SIP_5_15_CHECK_MAC(devNum)) || (HWS_DEV_SILICON_TYPE(devNum) == AC5) )
    {
        if (srcPortParams->firstLaneNum == NA_NUM) /* hwsPortModeParamsGet has returned defaults elements params and not the current port's params */
        {
            if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &portNum ) != GT_OK)
            {
                return GT_FAIL;
            }

            portParamsBuffer->portMacNumber =  (GT_U16)portNum;
            portParamsBuffer->portPcsNumber =  (GT_U16)portNum;
            portParamsBuffer->firstLaneNum  =  (GT_U16)portNum;
            portParamsBuffer->portFecMode   =  curDevicePorts[portNum].perModeFecList[portModeIndex];
            CHECK_STATUS(hwsPortParamsGetLanes(devNum,portGroup,portNum,portParamsBuffer));
        }
    }
#endif
    return GT_OK;
}

/**
* @internal hwsPortModeParamsSet function
* @endinternal
*
* @brief   port mode params set (FW)
*
* @param[in] devNum                  - system device number
* @param[in] portGroup               - port group (core) number
* @param[in] portNum                 - port num
* @param[in] portParams              - Port Init parameters
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortModeParamsSet
(
   IN GT_U8                    devNum,
   IN GT_U32                   portGroup,
   IN GT_U32                   portNum,
   IN MV_HWS_PORT_INIT_PARAMS  *portParams
)
{
    MV_HWS_PORT_INIT_PARAMS *portModesCurr;

    if(portParams->portStandard >= (MV_HWS_PORT_STANDARD)HWS_DEV_PORT_MODES(devNum))
    {
        return GT_BAD_PARAM;
    }

    portModesCurr = hwsPortModeParamsGet(devNum, portGroup, portNum, portParams->portStandard,NULL);
    if(portModesCurr == NULL)
    {
        return GT_NOT_INITIALIZED;
    }

    /* copy new port parameters */
    hwsOsMemCopyFuncPtr(portModesCurr, portParams, sizeof(MV_HWS_PORT_INIT_PARAMS));

    return GT_OK;
}

/**
* @internal hwsPortsParamsCfg function
* @endinternal
*
* @brief   port  params config .
*
* @param[in] devNum                  - system device number
* @param[in] portGroup               - port group (core) number
* @param[in] portNum                 - port num
* @param[in] portMode                - port standard metric
* @param[in] macNum                  - MAC number
* @param[in] pcsNum                  - physical PCS number
* @param[in] sdVecSize               - SD vector size
* @param[in] sdVector                - SD vector
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortsParamsCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_U32                  macNum,
    IN GT_U32                  pcsNum,
    IN GT_U8                   sdVecSize,
    IN GT_U8                   *sdVector
)
{
    MV_HWS_PORT_INIT_PARAMS *curPortParams;
    GT_U8 i;

    curPortParams = hwsPortModeParamsGet(devNum, portGroup, portNum, portMode,NULL);
    if (curPortParams == NULL)
    {
        return GT_NOT_SUPPORTED;
    }

    curPortParams->portMacNumber = (GT_U8)macNum;
    curPortParams->portPcsNumber = (GT_U8)pcsNum;

    for (i = 0; i < sdVecSize; i++)
    {
        curPortParams->activeLanesList[i] = sdVector[i];
    }
    curPortParams->numOfActLanes = sdVecSize;

    return GT_OK;
}

/**
* @internal hwsPortsElementsClose function
* @endinternal
*
* @brief   port  elements close .
*
* @param[in] devNum                  - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortsElementsClose
(
    IN GT_U8              devNum
)
{
#ifndef MV_HWS_REDUCED_BUILD
    MV_HWS_DEVICE_PORTS_ELEMENTS    curDevicePorts;
    GT_U32                          portNum; /* iterator */
    GT_U32                          numOfPorts;
#endif
    if (NULL == hwsDevicesPortsElementsArray[devNum])
    {
      return GT_OK;
    }
#ifndef MV_HWS_REDUCED_BUILD
    curDevicePorts = hwsDevicesPortsElementsArray[devNum];
    numOfPorts = hwsDeviceSpecInfo[devNum].portsNum;

    for (portNum = 0 ; portNum < numOfPorts ; portNum++)
    {

           if (NULL != curDevicePorts[portNum].perModeFecList)
           {
               hwsOsFreeFuncPtr(curDevicePorts[portNum].perModeFecList);
           }

    }
#endif
    hwsOsFreeFuncPtr(hwsDevicesPortsElementsArray[devNum]);
    hwsDevicesPortsElementsArray[devNum] = NULL;
    return GT_OK;
}

/**
* @internal hwsIsPortModeSupported function
* @endinternal
*
* @brief   is port mode supported
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                - port num
* @param[in] portMode                - Different port standard
*       metrics
*
* @retval GT_TRUE                    - if port mode supported
* @retval GT_FALSE                   - not supported mode
*/
GT_BOOL hwsIsPortModeSupported
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portNum,
    IN MV_HWS_PORT_STANDARD    portMode
)
{
#ifndef MV_HWS_REDUCED_BUILD
    MV_HWS_PORT_INIT_PARAMS *curPortParams = NULL;
    portGroup = portGroup;

    if (portMode == NON_SUP_MODE)
    {
        return GT_FALSE;
    }
    curPortParams = hwsPortModeParamsGet(devNum, portGroup, portNum, portMode,NULL);
    if (NULL == curPortParams || NA_NUM == curPortParams->portMacNumber)
    {
        return GT_FALSE;
    }
    return GT_TRUE;
#else
    return GT_TRUE;
#endif
}

#if defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD)
GT_STATUS hwsFalconPortParamsGetLanes
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      portNum,
    OUT MV_HWS_PORT_INIT_PARAMS*    portParams
)
{
#if defined CHX_FAMILY && !defined(MV_HWS_REDUCED_BUILD)
    MV_HWS_DEV_FUNC_PTRS            *hwsDevFunc = NULL;
    GT_U32                          deviceId ,version;
#endif
    GT_U16                          i = 0;
    /* SerDes-Muxing related variables */
    MV_HWS_PORT_SERDES_TO_MAC_MUX   hwsMacToSerdesMuxStc;
    GT_U16                          j;
    GT_U16                          portOffset = 0;
    GT_U32                          firstPortInGop = 0;

    if(GT_OK == mvHwsPortLaneMacToSerdesMuxGet(devNum, (GT_U8)portGroup, portNum, &hwsMacToSerdesMuxStc))
    {
        portOffset = portNum & 0x7;
        firstPortInGop = portNum & 0xfffffff8;
    }
    else
    {
        portOffset = 0;
        firstPortInGop = portParams->firstLaneNum;
    }

#if defined CHX_FAMILY && !defined(MV_HWS_REDUCED_BUILD)
    /* get func DB */
    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);
    /* For devices with special lanes mapping */
    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc == NULL)
    {
        return GT_BAD_STATE;
    }

    CHECK_STATUS(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc(devNum, &deviceId, &version));
    if ((deviceId == CPSS_98CX8525_CNS) ||
        (deviceId == CPSS_98CX8535_CNS) ||
        (deviceId == CPSS_98CX8535_H_CNS) ||
        (deviceId == CPSS_98CX8514_CNS) ||
        (deviceId == CPSS_98EX5614_CNS))
    {   /* 2T/4T/3.2T */
        switch(firstPortInGop/16) /* get Raven's the number and then shift the SerDes numbers accordingly to avoid gaps in the numbering */
        {
            case 0: /* 0-15 -> 0-15 */
                /*do nothing*/
                break;
            case 2: /* 32-47 -> 16-31 */
                firstPortInGop -= 16;
                break;
            case 4: /* 64-79 -> 32-47 */
                firstPortInGop -= 32;
                break;
            case 5: /* 80-95 -> 48-63 */
                firstPortInGop -= 32;
                break;
            case 7: /* 112-127 -> 64-79 */
                firstPortInGop -= 48;
                break;
            default:
                if(firstPortInGop == 133)
                {
                    firstPortInGop = 80;
                }
                else if(firstPortInGop == 135)
                {
                    firstPortInGop = 81;
                }
                else
                    return GT_BAD_STATE;
        }
    }
#endif
    for (i = 0 ; i < portParams->numOfActLanes ; i++)
    {
        j = portOffset + i; /* start from the right offset in the ports group */
        if(hwsMacToSerdesMuxStc.enableSerdesMuxing)
        {
            portParams->activeLanesList[i] = (GT_U16)firstPortInGop + (GT_U16)(hwsMacToSerdesMuxStc.serdesLanes[j]);
        }
        else
        {
            portParams->activeLanesList[i] = (GT_U16)firstPortInGop + j;
        }
    }
    portParams->firstLaneNum = portParams->activeLanesList[0];

    return GT_OK;
}
#endif

/**
* @internal hwsPortParamsGetLanes function
* @endinternal
*
* @brief   port  params get active lanes .
*
* @param[in] devNum                  - system device number
* @param[in] portGroup               - port group (core) number
* @param[in] portNum                 - port num
* @param[in,out] portParams          -  (pointer to) Port Init
*       parameters
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortParamsGetLanes
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      portNum,
    INOUT MV_HWS_PORT_INIT_PARAMS*    portParams
)
{
    MV_HWS_DEV_FUNC_PTRS            *hwsDevFunc = NULL;
    GT_U16                          i = 0;
#if defined(ALDRIN2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD)
    /* SerDes-Muxing related variables */
    MV_HWS_PORT_SERDES_TO_MAC_MUX   hwsMacToSerdesMuxStc;
    MV_HWS_DEV_TYPE                 devType = HWS_DEV_SILICON_TYPE(devNum);
    GT_U16                          j;
    GT_U16                          portOffset = 0;
    GT_U32                          firstPortInGop = 0;
#endif
    if (NULL == portParams)
    {
        return GT_BAD_PARAM;
    }

    if (portParams->portStandard == NON_SUP_MODE)
    {
        return GT_NOT_SUPPORTED;
    }
    /* get func DB */
    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);
    /* For devices with special lanes mapping */
    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsGetLanesFunc != NULL)
    {
        return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsGetLanesFunc(devNum, portGroup, portNum, portParams);
    }
    else /* Build default active lane list */
    {
#if defined(ALDRIN2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD)
        if(GT_OK == mvHwsPortLaneMacToSerdesMuxGet(devNum, (GT_U8)portGroup, portNum, &hwsMacToSerdesMuxStc))
        {
            switch(devType)
            {
            case Aldrin2:
                portOffset = portNum & 0x3;
                firstPortInGop = portNum & 0xfffffffc;
                break;
            default:
                return GT_NOT_SUPPORTED;
            }
        }
#endif

        for (i = 0 ; i < portParams->numOfActLanes ; i++)
        {
#if defined(ALDRIN2_DEV_SUPPORT)|| !defined(MV_HWS_REDUCED_BUILD)
            if(hwsMacToSerdesMuxStc.enableSerdesMuxing)
            {
                j = portOffset + i; /* start from the right offset in the ports quad */
                portParams->activeLanesList[i] = (GT_U16)firstPortInGop + (GT_U16)(hwsMacToSerdesMuxStc.serdesLanes[j]);
            }
            else
#endif
            {
                portParams->activeLanesList[i] = (GT_U16)(portParams->firstLaneNum) + i;
            }
        }
        portParams->firstLaneNum = portParams->activeLanesList[0];
    }
    return GT_OK;
}

/**
* @internal hwsIsFecModeSupported function
* @endinternal
*
* @brief   is FEC mode supported
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - port num
* @param[in] portMode                 - Different port standard
*                                       metrics
* @param[in] fecMode                  - FEC status
*
* @retval GT_TRUE                    - if FEC mode supported
* @retval GT_FALSE                   - not supported mode
*/
GT_BOOL hwsIsFecModeSupported
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_FEC_MODE    fecMode
)
{
    GT_U8                   fecSupportedModesBmp;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    portGroup = portGroup;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        return GT_FALSE;
    }
    fecSupportedModesBmp = curPortParams.fecSupportedModesBmp;

    return (GT_BOOL)((fecSupportedModesBmp) & (fecMode));
}

/**
* @internal hwsIsQuadCgMacSupported function
* @endinternal
*
* @brief   is port mode supported
*
* @param[in] devNum          - system device number
* @param[in] portNum         - port num
*
* @retval GT_TRUE            - if is quad CG mac mode supported
* @retval GT_FALSE           - not supported
*/
GT_BOOL hwsIsQuadCgMacSupported
(
    IN GT_U8                   devNum,
    IN GT_U32                  portNum
)
{
    MV_HWS_DEV_TYPE     devType = HWS_DEV_SILICON_TYPE(devNum);
    GT_U32              firstInQuad = (portNum  & HWS_2_LSB_MASK_CNS);

    /* check if given port num on given device has support for CG-MAC  */
    switch (devType)
    {
    case Bobcat3:
        /*U32 variable is always >=0 */
        if (/*(0 <= firstInQuad) && */(firstInQuad < 72)) {
            return GT_TRUE;
        }
        else
        {
            return GT_FALSE;
        }
    case Aldrin2:
        /*U32 variable is always >=0 */
        if (/*(0 <= firstInQuad) && */(firstInQuad < 24)) {
            return GT_TRUE;
        }
        else
        {
            return GT_FALSE;
        }
    case Pipe:
        if ((12 <= firstInQuad) && (firstInQuad < 16)) {
            return GT_TRUE;
        }
        else
        {
            return GT_FALSE;
        }
    case Raven:
        return GT_FALSE;
    case Falcon:
        return GT_FALSE;
    default:
        break;
    }
    return GT_FALSE;
}

/*============================== NOT DEFINED: MV_HWS_REDUCED_BUILD ==============================*/
#ifndef MV_HWS_REDUCED_BUILD

/**
* @internal hwsDevicePortsElementsCfg function
* @endinternal
*
* @brief   Init the ports' struct and initialize
*          portModesElements.
*
* @param[in] devNum          - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsDevicePortsElementsCfg
(
    IN GT_U8       devNum
)
{
    GT_U32                          portNum, portMode; /* iterators */
    GT_U32                          numOfSupModes;
    /* Get device's specific data from hwsDeviceSpecInfo Db*/
    GT_U32                          numOfPorts = hwsDeviceSpecInfo[devNum].portsNum;
    const MV_HWS_PORT_INIT_PARAMS   **hwsPortsSupModes =PRV_PORTS_PARAMS_SUP_MODE_MAP(devNum);
    MV_HWS_DEVICE_PORTS_ELEMENTS    curDevicePorts = hwsDevicesPortsElementsArray[devNum];


    for (portNum = 0; portNum < numOfPorts ; portNum++)
    {
    /* Initialize the MV_HWS_SINGLE_PORT_STC fields for each port: */
        /* (1) Initialize MV_HWS_SINGLE_PORT_STC.supModesCatalog */
        if (NULL != hwsPortsSupModes)
        {
            curDevicePorts[portNum].supModesCatalog = hwsPortsSupModes[portNum];
        }

        if(curDevicePorts[portNum].supModesCatalog  !=  NULL)
        { /* if this port is useable (in some devices,not every port number is been used) */

        /* (2) Initialize MV_HWS_SINGLE_PORT_STC.perModeFecList */
           hwsPortsElementsFindMode(curDevicePorts[portNum].supModesCatalog,NON_SUP_MODE,&numOfSupModes);
           curDevicePorts[portNum].perModeFecList = (MV_HWS_PORT_FEC_MODE*)hwsOsMallocFuncPtr(numOfSupModes*sizeof(MV_HWS_PORT_FEC_MODE));
           if (NULL == curDevicePorts[portNum].perModeFecList)
           {
               return GT_NO_RESOURCE;
           }
           for (portMode = 0; portMode < numOfSupModes; portMode++)
           { /* for each portMode, copy the defualt FEC value from the right SupModesCatalog */
               curDevicePorts[portNum].perModeFecList[portMode]=
                   curDevicePorts[portNum].supModesCatalog[portMode].portFecMode;
           }

       /* (3) Initialize MV_HWS_SINGLE_PORT_STC.curPortParams */
           hwsPortModeParamsSetMode(devNum,0,portNum,NON_SUP_MODE);
        }
    }
    return GT_OK;
}

/**
* @internal hwsPortsElementsFindMode function
* @endinternal
*
* @brief   find the port mode line in the catlog supplied in
*          hwsPortSupModes.
*
* @param[in] hwsPortSupModes          - catalog of the supported
*       modes
* @param[in] portMode                - port mode to find
* @param[out] portModeIndex           - the line number of the
*       requested port mode in the portModeIndex catalog.
*
* @retval MV_HWS_PORT_INIT_PARAMS* - pointer to the
*                                    *ORIGINAL* entry in the
*                                    portModeIndex catlog
* @retval NULL -                     on error/not supported mode
*
* @note If portMode=NON_SUP_MODE, this value is the number of
*       supported modes.not mandatory can be NULL.
*/
const MV_HWS_PORT_INIT_PARAMS *hwsPortsElementsFindMode
(
    IN const MV_HWS_PORT_INIT_PARAMS hwsPortSupModes[],
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT GT_U32                  *portModeIndex
)
{
    GT_U32 i = 0;
    if(NULL == hwsPortSupModes)
    {
        *portModeIndex = NA_NUM;
        return NULL;
    }
    while (NON_SUP_MODE != hwsPortSupModes[i].portStandard &&
            i < LAST_PORT_MODE) { /* Last line of each SupMode array MUST be NON_SUP_MODE */
        if (hwsPortSupModes[i].portStandard == portMode)
        {
            if(NULL != portModeIndex) *portModeIndex = i;
            return &hwsPortSupModes[i];
        }
        i++;
    }
    if(NULL != portModeIndex)
    {
        (NON_SUP_MODE == portMode) ? (*portModeIndex = i) /* return num of supported modes */
                                   : (*portModeIndex = NA_NUM);
    }
    return NULL;
}

/**
* @internal hwsPortModeParamsPrintCurrent function
* @endinternal
*
* @brief   print the elements device.
*
* @param[in] devNum          - system device number- use
*                              PRINT_ALL_DEVS (0xff) to print
*                              all devices
* @param[in] portNum         - port num
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortModeParamsPrintCurrent
(
    IN GT_U8                   devNum,
    IN GT_U32                  portNum
)
{
    MV_HWS_PORT_INIT_PARAMS *portParamPtr;
    if(PRINT_ALL_DEVS == devNum) /* print all ports of all devices */
    {
        devNum = 0;
        while (devNum<HWS_MAX_DEVICE_NUM)
        {
            if (NULL == hwsDevicesPortsElementsArray[devNum])
            {
                devNum++;
                continue;
            }
            hwsOsPrintf("+----------+-----------+---------+-----+---------+-----+-----+-------+-------+---------------+--------+-------+-------+-----+\n");
            hwsOsPrintf("| dev/MAC  |    Port   |   MAC   | MAC |   PCS   | PCS | FEC | Speed | First | Active Lanes  | Num Of | Media | 10Bit | FEC |\n");
            hwsOsPrintf("|          |    Mode   |   Type  | Num |   Type  | Num |     |       | Lane  |     List      | Lanes  |       | 10Bit | Bmp |\n");
            hwsOsPrintf("+----------+-----------+---------+-----+---------+-----+-----+-------+-------+---------------+--------+-------+-------+-----+\n");
            for (portNum = 0; portNum < hwsDeviceSpecInfo[devNum].portsNum; portNum++)
            {
                portParamPtr = hwsPortModeParamsGet(devNum, 0, portNum, NON_SUP_MODE,NULL);
                if(portParamPtr == NULL)
                {
                    continue;
                }
                hwsOsPrintf( "|%3d/%-6d|0x%-9x|0x%-7x|%-5d|0x%-7x|%-5d|0x%-3x|0x%-5x|%-7d|%-3d,%-3d,%-3d,%-3d|%-8d|0x%-5x|0x%-5x|0x%-3x|\n",
                             devNum, portNum,
                             portParamPtr->portStandard,
                             portParamPtr->portMacType, portParamPtr->portMacNumber,
                             portParamPtr->portPcsType, portParamPtr->portPcsNumber,
                             portParamPtr->portFecMode, portParamPtr->serdesSpeed,
                             portParamPtr->firstLaneNum,
                             portParamPtr->activeLanesList[0], portParamPtr->activeLanesList[1], portParamPtr->activeLanesList[2], portParamPtr->activeLanesList[3],
                             portParamPtr->numOfActLanes, portParamPtr->serdesMediaType,
                             portParamPtr->serdes10BitStatus,
                             portParamPtr->fecSupportedModesBmp);
            }
            hwsOsPrintf("+----------+-----------+---------+-----+---------+-----+-----+-------+-------+---------------+--------+-------+-------+-----+\n");
            devNum++;
         }
    }
    else /* print specific port */
    {
        if (devNum >= HWS_MAX_DEVICE_NUM)
        {
            return GT_OUT_OF_RANGE;
        }
        if (NULL == hwsDevicesPortsElementsArray[devNum])
        {
            return GT_NOT_INITIALIZED;
        }
        if (portNum >= hwsDeviceSpecInfo[devNum].portsNum)
        {
            return GT_OUT_OF_RANGE;
        }
        portParamPtr = hwsPortModeParamsGet(devNum, 0, portNum, NON_SUP_MODE,NULL);
        if(portParamPtr == NULL)
        {
            return GT_NOT_INITIALIZED;;
        }

        hwsOsPrintf("+----------+-----------+---------+-----+---------+-----+-----+-------+-------+---------------+--------+-------+-------+-----+\n");
        hwsOsPrintf("| dev/MAC  |    Port   |   MAC   | MAC |   PCS   | PCS | FEC | Speed | First | Active Lanes  | Num Of | Media | 10Bit | FEC |\n");
        hwsOsPrintf("|          |    Mode   |   Type  | Num |   Type  | Num |     |       | Lane  |     List      | Lanes  |       | 10Bit | Bmp |\n");
        hwsOsPrintf("+----------+-----------+---------+-----+---------+-----+-----+-------+-------+---------------+--------+-------+-------+-----+\n");
        hwsOsPrintf( "|%3d/%-6d|0x%-9x|0x%-7x|%-5d|0x%-7x|%-5d|0x%-3x|0x%-5x|%-7d|%-3d,%-3d,%-3d,%-3d|%-8d|0x%-5x|0x%-5x|0x%-3x|\n",
                     devNum, portNum,
                     portParamPtr->portStandard,
                     portParamPtr->portMacType, portParamPtr->portMacNumber,
                     portParamPtr->portPcsType, portParamPtr->portPcsNumber,
                     portParamPtr->portFecMode, portParamPtr->serdesSpeed,
                     portParamPtr->firstLaneNum,
                     portParamPtr->activeLanesList[0], portParamPtr->activeLanesList[1], portParamPtr->activeLanesList[2], portParamPtr->activeLanesList[3],
                     portParamPtr->numOfActLanes, portParamPtr->serdesMediaType,
                     portParamPtr->serdes10BitStatus,
                     portParamPtr->fecSupportedModesBmp);
        hwsOsPrintf("+----------+-----------+---------+-----+---------+-----+-----+-------+-------+---------------+--------+-------+-------+-----+\n");
    }
    return GT_OK;
}

#ifdef HWS_ELEMENTS_DEBUG_OPENED
/**
* @internal debugPrvHwsPrintCatalogDb function
* @endinternal
*
* @brief   print the elements device.
*
* @param[in] devNum          - system device number
* @param[in] portNum         - port num
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS debugPrvHwsPrintCatalogDb
(
    IN GT_U8                   devNum,
    IN GT_U32                  portNum
)
{
    MV_HWS_PORT_INIT_PARAMS *portParamPtr;
    if(PRINT_ALL_DEVS == devNum) /* print all ports of all devices */
    {
        devNum = 0;
        while (devNum<HWS_MAX_DEVICE_NUM)
        {
            if (NULL == hwsDevicesPortsElementsArray[devNum])
            {
                devNum++;
                continue;
            }
            hwsOsPrintf("+--------+----------------+-----------+-----+-----------------+-----+-----+-------+-------+---------------+--------+-------+-------+-----+\n");
            hwsOsPrintf("| dev/   |    Port        |    MAC    | MAC |       PCS       | PCS | FEC | SD    | First | Active Lanes  | Num Of | Media | 10Bit | FEC |\n");
            hwsOsPrintf("| port   |    Mode        |    Type   | Num |       Type      | Num |     | Speed | Lane  |     List      | Lanes  |       | 10Bit | Bmp |\n");
            hwsOsPrintf("+--------+----------------+-----------+-----+-----------------+-----+-----+-------+-------+---------------+--------+-------+-------+-----+\n");
            for (portNum = 0; portNum < hwsDeviceSpecInfo[devNum].portsNum; portNum++)
            {
                portParamPtr = hwsPortsParamsArrayGet(devNum, 0, portNum);
                if(portParamPtr == NULL)
                {
                    continue;
                }
                while (portParamPtr->portStandard != NON_SUP_MODE)
                {
                    hwsOsPrintf( "|%3d/%-4d|%-16s|%-11s|%-5d|%-17s|%-5d|0x%-3x|0x%-5x|%-7d|%-3d,%-3d,%-3d,%-3d|%-8d|0x%-5x|0x%-5x|0x%-3x|\n",
                                 devNum, portNum,
                                 portModeNames[portParamPtr->portStandard],
                                 macTypeNames[portParamPtr->portMacType], portParamPtr->portMacNumber,
                                 pcsTypeNames[portParamPtr->portPcsType], portParamPtr->portPcsNumber,
                                 portParamPtr->portFecMode, portParamPtr->serdesSpeed,
                                 portParamPtr->firstLaneNum,
                                 portParamPtr->activeLanesList[0], portParamPtr->activeLanesList[1], portParamPtr->activeLanesList[2], portParamPtr->activeLanesList[3],
                                 portParamPtr->numOfActLanes, portParamPtr->serdesMediaType,
                                 portParamPtr->serdes10BitStatus,
                                 portParamPtr->fecSupportedModesBmp);
                    portParamPtr++;
                }
            }
            hwsOsPrintf("+--------+----------------+-----------+-----+-----------------+-----+-----+-------+-------+---------------+--------+-------+-------+-----+\n");
            devNum++;
         }
    }
    else /* print specific port */
    {
        if (devNum >= HWS_MAX_DEVICE_NUM)
        {
            return GT_OUT_OF_RANGE;
        }
        if (NULL == hwsDevicesPortsElementsArray[devNum])
        {
            return GT_NOT_INITIALIZED;
        }
        if (portNum >= hwsDeviceSpecInfo[devNum].portsNum)
        {
            return GT_OUT_OF_RANGE;
        }
        hwsOsPrintf("+--------+----------------+-----------+-----+-----------------+-----+-----+-------+-------+---------------+--------+-------+-------+-----+\n");
        hwsOsPrintf("| dev/   |    Port        |    MAC    | MAC |       PCS       | PCS | FEC | SD    | First | Active Lanes  | Num Of | Media | 10Bit | FEC |\n");
        hwsOsPrintf("| port   |    Mode        |    Type   | Num |       Type      | Num |     | Speed | Lane  |     List      | Lanes  |       | 10Bit | Bmp |\n");
        hwsOsPrintf("+--------+----------------+-----------+-----+-----------------+-----+-----+-------+-------+---------------+--------+-------+-------+-----+\n");
        portParamPtr = hwsPortsParamsArrayGet(devNum, 0, portNum);
        if(portParamPtr == NULL)
        {
            return GT_NOT_INITIALIZED;
        }
        while (portParamPtr->portStandard != NON_SUP_MODE)
        {
            hwsOsPrintf( "|%3d/%-4d|%-16s|%-9s|%-5d|%-17s|%-5d|0x%-3x|0x%-5x|%-7d|%-3d,%-3d,%-3d,%-3d|%-8d|0x%-5x|0x%-5x|0x%-3x|\n",
                         devNum, portNum,
                         portModeNames[portParamPtr->portStandard],
                         macTypeNames[portParamPtr->portMacType], portParamPtr->portMacNumber,
                         pcsTypeNames[portParamPtr->portPcsType], portParamPtr->portPcsNumber,
                         portParamPtr->portFecMode, portParamPtr->serdesSpeed,
                         portParamPtr->firstLaneNum,
                         portParamPtr->activeLanesList[0], portParamPtr->activeLanesList[1], portParamPtr->activeLanesList[2], portParamPtr->activeLanesList[3],
                         portParamPtr->numOfActLanes, portParamPtr->serdesMediaType,
                         portParamPtr->serdes10BitStatus,
                         portParamPtr->fecSupportedModesBmp);
            portParamPtr++;
        }

        hwsOsPrintf("+--------+----------------+-----------+-----+-----------------+-----+-----+-------+-------+---------------+--------+-------+-------+-----+\n");
    }
    return GT_OK;
}
#endif /* HWS_ELEMENTS_DEBUG_OPENED */

#endif
