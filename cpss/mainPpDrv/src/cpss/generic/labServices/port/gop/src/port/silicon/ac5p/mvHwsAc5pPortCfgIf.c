/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* mvHwsAc5pPortCfgIf.c
*
* DESCRIPTION:
*           This file contains API for port configuartion and tuning parameters
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
******************************************************************************/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/generic/labservices/port/gop/silicon/ac5p/mvHwsAc5pPortIf.h>
#include <cpss/generic/labservices/port/gop/silicon/falcon/mvHwsFalconPortIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApDefs.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortAnp.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>


typedef enum {
    MV_HWS_MIF_PORT_TYPE_8_E,
    MV_HWS_MIF_PORT_TYPE_32_E,
    MV_HWS_MIF_PORT_TYPE_128_E,
    MV_HWS_MIF_PORT_TYPE_8_CPU_E
} MV_HWS_MIF_PORT_TYPE_ENT;

/**
* @internal mvHwsExtIfFirstInPortGroupCheck function
* @endinternal
*
* @brief  Shared resources validation check - per port group (8 ports)
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] *isFisrtPtr    - result if port is first or not
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsExtIfFirstInPortGroupCheck
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    OUT MV_HWS_PORT_IN_GROUP_ENT    *isFisrtPtr
)
{
    GT_STATUS rc;
    GT_U32 regData, i;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32 firstIndex, iterNum;
    MV_HWS_MTIP_USX_EXT_UNITS_FIELDS_E usxExtField;

    if(isFisrtPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    if (mvHwsUsxModeCheck(devNum, portNum, portMode) == GT_TRUE)
    {
        firstIndex = 0;
        iterNum = 8;

        for(i = firstIndex; i < iterNum; i++)
        {
            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_PORT0_MAC_CLK_EN_E + i;
            CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, &regData, NULL));
            if(regData != 0)
            {
                break;
            }
        }

        *isFisrtPtr = (i == iterNum) ? MV_HWS_PORT_IN_GROUP_FIRST_E : MV_HWS_PORT_IN_GROUP_EXISTING_E;
    }
    else if(mvHwsMtipIsReducedPort(devNum, portNum))
    {
        *isFisrtPtr = MV_HWS_PORT_IN_GROUP_FIRST_E;
    }
    else /* MTI_400/100 */
    {
        usxExtField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_MAC_CLK_EN_E : PHOENIX_MTIP_EXT_UNITS_GLOBAL_CLOCK_ENABLE_MAC_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_EXT_UNIT, usxExtField, &regData, NULL));
        *isFisrtPtr = (regData == 0) ? MV_HWS_PORT_IN_GROUP_FIRST_E : MV_HWS_PORT_IN_GROUP_EXISTING_E;

        /* Check also if segmented port is configured */
        if(*isFisrtPtr == MV_HWS_PORT_IN_GROUP_FIRST_E)
        {
            if(HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum))
            {
                usxExtField = HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_SEG_MAC_CLK_EN_E;
                CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_EXT_UNIT, usxExtField, &regData, NULL));
                *isFisrtPtr = (regData == 0) ? MV_HWS_PORT_IN_GROUP_FIRST_E : MV_HWS_PORT_IN_GROUP_EXISTING_E;
            }
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsExtIfLastInPortGroupCheck function
* @endinternal
*
* @brief  Shared resources validation check - per port group (8 ports)
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] *isLastPtr     - result if port is last or not
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsExtIfLastInPortGroupCheck
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    OUT MV_HWS_PORT_IN_GROUP_ENT   *isLastPtr
)
{
    GT_STATUS rc;
    GT_U32 regData, i, localPortIndex, enaCounter = 0;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32 iterNum, groupStartPortNum, step;
    MV_HWS_MTIP_USX_EXT_UNITS_FIELDS_E usxExtField;

    if(isLastPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    if (mvHwsUsxModeCheck(devNum, portNum, portMode) == GT_TRUE)
    {
        localPortIndex = convertIdx.ciderIndexInUnit % 8;

        CHECK_STATUS(mvHwsUsxExtSerdesGroupParamsGet(devNum,portNum,_20G_OXGMII,&groupStartPortNum,&step,&iterNum));

        for(i = 0; i < iterNum; i++)
        {
            if(localPortIndex == i)
            {
                continue;
            }

            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_PORT0_MAC_CLK_EN_E + i;
            CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, &regData, NULL));
            if(regData != 0)
            {
                if(portMode == QSGMII || portMode == _10G_OUSGMII)
                {
                    enaCounter = 1;
                    break;
                }
                else
                {
                    usxExtField = MTIP_USX_EXT_UNITS_CONTROL_P0_TX_LOC_FAULT_E;
                    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, (groupStartPortNum+i), MTI_USX_EXT_UNIT, usxExtField, &regData, NULL));
                    if(regData == 0) /* if local fault is not configured - port was created */
                    {
                        enaCounter = 1;
                        break;
                    }
                }
            }
        }

        *isLastPtr = (enaCounter == 0) ? MV_HWS_PORT_IN_GROUP_LAST_E : MV_HWS_PORT_IN_GROUP_EXISTING_E;
    }
    else if(mvHwsMtipIsReducedPort(devNum, portNum))
    {
        *isLastPtr = MV_HWS_PORT_IN_GROUP_LAST_E;
    }
    else /* MTI 100 */
    {
        localPortIndex = convertIdx.ciderIndexInUnit % 8;
        usxExtField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_MAC_CLK_EN_E: PHOENIX_MTIP_EXT_UNITS_GLOBAL_CLOCK_ENABLE_MAC_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_EXT_UNIT, usxExtField, &regData, NULL));
        /*remove current port from bitmap*/
        regData &= ~(1<<localPortIndex);
        *isLastPtr = (regData == 0) ? MV_HWS_PORT_IN_GROUP_LAST_E : MV_HWS_PORT_IN_GROUP_EXISTING_E;

        /* Check also if segmented port is configured */
        if(*isLastPtr == MV_HWS_PORT_IN_GROUP_LAST_E)
        {
            if(HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum))
            {
                usxExtField = HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_SEG_MAC_CLK_EN_E;
                CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_EXT_UNIT, usxExtField, &regData, NULL));
                /*remove current port from bitmap*/
                localPortIndex = localPortIndex / 4;
                regData &= ~(1<<localPortIndex);
                *isLastPtr = (regData == 0) ? MV_HWS_PORT_IN_GROUP_LAST_E : MV_HWS_PORT_IN_GROUP_EXISTING_E;
            }
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsExtIfFirstInSerdesGroupCheck function
* @endinternal
*
* @brief  Shared resources validation check - per SD group (4 or 8 ports)
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] *isFisrtPtr    - result if port is first or not
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsExtIfFirstInSerdesGroupCheck
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    OUT MV_HWS_PORT_IN_GROUP_ENT    *isFisrtPtr
)
{
    GT_STATUS rc;
    GT_U32 regData, i, localPortIndex;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32 firstIndex, iterNum;
    MV_HWS_MTIP_USX_EXT_UNITS_FIELDS_E usxExtField;

    if(isFisrtPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    if (mvHwsUsxModeCheck(devNum, portNum, portMode) == GT_TRUE)
    {
        if(AC5X == HWS_DEV_SILICON_TYPE(devNum))
        {
            return mvHwsExtIfFirstInPortGroupCheck(devNum,portNum,portMode,isFisrtPtr);
        }

        if (HWS_USX_O_MODE_CHECK(portMode))
        {
            firstIndex = 0;
            iterNum = 8;
        }
        else
        {
            firstIndex = convertIdx.ciderIndexInUnit % 8;
            firstIndex = firstIndex & 0x4; /* 0 or 4*/
            iterNum = 4;
        }

        /*localPortIndex = convertIdx.ciderIndexInUnit%8;*/

        for(i = firstIndex; i < (firstIndex+iterNum); i++)
        {
            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_PORT0_MAC_CLK_EN_E + i;
            CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, &regData, NULL));
            if(regData != 0)
            {
                break;
            }
        }

        *isFisrtPtr = (i == (firstIndex+iterNum)) ? MV_HWS_PORT_IN_GROUP_FIRST_E : MV_HWS_PORT_IN_GROUP_EXISTING_E;
    }
    else if(mvHwsMtipIsReducedPort(devNum, portNum))
    {
        *isFisrtPtr = MV_HWS_PORT_IN_GROUP_FIRST_E;
    }
    else /* MTI_400/100 */
    {
        if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
        {
            if(HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum))
            {
                usxExtField = HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_SEG_MAC_CLK_EN_E;
                CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_EXT_UNIT, usxExtField, &regData, NULL));
                localPortIndex = convertIdx.ciderIndexInUnit / 4;
                regData &= ~(1<<localPortIndex);
                *isFisrtPtr = (regData == 0) ? MV_HWS_PORT_IN_GROUP_FIRST_E : MV_HWS_PORT_IN_GROUP_EXISTING_E;
            }
        }
        else
        {
            usxExtField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_MAC_CLK_EN_E : PHOENIX_MTIP_EXT_UNITS_GLOBAL_CLOCK_ENABLE_MAC_CLK_EN_E;
            CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_EXT_UNIT, usxExtField, &regData, NULL));

            if(convertIdx.ciderIndexInUnit < 4)
            {
                regData = regData & 0xF;
            }
            else
            {
                regData = regData & 0xF0;
            }

            *isFisrtPtr = (regData == 0) ? MV_HWS_PORT_IN_GROUP_FIRST_E : MV_HWS_PORT_IN_GROUP_EXISTING_E;
        }
    }
    return GT_OK;
}


/**
* @internal mvHwsExtIfLastInSerdesGroupCheck function
* @endinternal
*
* @brief  Shared resources validation check - per SD group (4 or 8 ports)
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] *isLastPtr     - result if port is last or not
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsExtIfLastInSerdesGroupCheck
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    OUT MV_HWS_PORT_IN_GROUP_ENT    *isLastPtr
)
{
    GT_STATUS rc;
    GT_U32 regData, i, j, localPortIndex, enaCounter = 0;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32 firstIndex, iterNum, groupStartPortNum, step;
    MV_HWS_MTIP_USX_EXT_UNITS_FIELDS_E usxExtField;

    if(isLastPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    if (mvHwsUsxModeCheck(devNum, portNum, portMode) == GT_TRUE)
    {
        if(AC5X == HWS_DEV_SILICON_TYPE(devNum))
        {
            return mvHwsExtIfLastInPortGroupCheck(devNum, portNum, portMode, isLastPtr);
        }

        if (HWS_USX_O_MODE_CHECK(portMode))
        {
            firstIndex = 0;
        }
        else
        {
            firstIndex = convertIdx.ciderIndexInUnit % 8;
            firstIndex = firstIndex & 0x4; /* 0 or 4*/
        }

        CHECK_STATUS(mvHwsUsxExtSerdesGroupParamsGet(devNum,portNum,portMode,&groupStartPortNum,&step,&iterNum));

        localPortIndex = convertIdx.ciderIndexInUnit % 8;

        for(i = firstIndex, j = 0; iterNum > 0; i++, j+=step)
        {
            if(localPortIndex == i)
            {
                iterNum--;
                continue;
            }

            /*
                AC5P Note:  for double USX mode (for ex. 20G_DXGMII) active ports are 2 and 4.
                            port 2: to check MAC_CLK_EN - need to access to bit 0 (ciderIndexInUnit = 0)
                            port 4: to check MAC_CLK_EN - need to access to bit 1 (ciderIndexInUnit = 1)

                Thats why loop iterator i is incremented by '1' and not by 'step'
            */
            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_PORT0_MAC_CLK_EN_E + i;
            CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, &regData, NULL));
            if(regData != 0)
            {
                if(portMode == QSGMII || portMode == _10G_OUSGMII)
                {
                    enaCounter = 1;
                    break;
                }
                else
                {
                    usxExtField = MTIP_USX_EXT_UNITS_CONTROL_P0_TX_LOC_FAULT_E;
                    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, (groupStartPortNum+j), MTI_USX_EXT_UNIT, usxExtField, &regData, NULL));
                    if(regData == 0) /* if local fault is not configured - port was created */
                    {
                        enaCounter = 1;
                        break;
                    }
                }
            }
            iterNum--;
        }

        *isLastPtr = (enaCounter == 0) ? MV_HWS_PORT_IN_GROUP_LAST_E : MV_HWS_PORT_IN_GROUP_EXISTING_E;
    }
    else if(mvHwsMtipIsReducedPort(devNum, portNum))
    {
        *isLastPtr = MV_HWS_PORT_IN_GROUP_LAST_E;
    }
    else /* MTI 100 */
    {
        if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
        {
            if(HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum))
            {
                usxExtField = HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_SEG_MAC_CLK_EN_E;
                CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_EXT_UNIT, usxExtField, &regData, NULL));
                /*remove current port from bitmap*/
                localPortIndex = convertIdx.ciderIndexInUnit / 4;
                regData &= ~(1<<localPortIndex);
                *isLastPtr = (regData == 0) ? MV_HWS_PORT_IN_GROUP_LAST_E : MV_HWS_PORT_IN_GROUP_EXISTING_E;
            }
        }
        else
        {
            localPortIndex = convertIdx.ciderIndexInUnit % 8;
            usxExtField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_MAC_CLK_EN_E: PHOENIX_MTIP_EXT_UNITS_GLOBAL_CLOCK_ENABLE_MAC_CLK_EN_E;
            CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_EXT_UNIT, usxExtField, &regData, NULL));

            if(convertIdx.ciderIndexInUnit < 4)
            {
                regData = regData & 0xF;
            }
            else
            {
                regData = regData & 0xF0;
            }

            /*remove current port from bitmap*/
            regData &= ~(1<<localPortIndex);
            *isLastPtr = (regData == 0) ? MV_HWS_PORT_IN_GROUP_LAST_E : MV_HWS_PORT_IN_GROUP_EXISTING_E;
        }
    }

    return GT_OK;
}


/**
* @internal mvHwsMifInitPortPrv function
* @endinternal
*
* @brief  MIF unit init pre port and portType
*
* @param[in] devNum                - system device number
* @param[in] mifPortType           - port mif type
* @param[in] port                  - port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMifInitPortPrv
(
    IN GT_U8 devNum,
    IN MV_HWS_MIF_PORT_TYPE_ENT mifPortType,
    IN GT_U32 port
)
{
    GT_STATUS rc;
    GT_U32 txCreditAlloc  = 120, fieldNameTxCreditAlloc, fieldNameTxChannelId, fieldNameRxChannelId;
    GT_U32 chTag = 0, fieldNameChTag;
    GT_U32 eopLinkDown = 1, fieldNameEopLinkDown;
    GT_U32 rxGracefulMode = 0, fieldNameRxGracefulMode;
    GT_U32 clkEn = 1, fieldNameClkEn;
    GT_U32 fieldNameIgnoreMif, fieldNameIgnoreMac, ignoreMif = 1, ignoreMac = 0;
    MV_HWS_UNITS_ID unitId;
    MV_HWS_HAWK_CONVERT_STC convertIdx;

    if (hwsIsIronmanAsPhoenix())
    {
        /* Ironman's MIF unit supportded by specific CPSS code */
        return GT_OK;
    }

    if(hwsIsIronmanAsPhoenix() && mifPortType != MV_HWS_MIF_PORT_TYPE_8_E)
    {
        return GT_OK;
    }

    if (HWS_DEV_SILICON_TYPE(devNum) == AC5X)
    {
        ignoreMif = 0;
        ignoreMac = 1;
    }

    hwsOsMemSetFuncPtr(&convertIdx, 0, sizeof(MV_HWS_HAWK_CONVERT_STC));

    switch (mifPortType) {
        case MV_HWS_MIF_PORT_TYPE_32_E:

            if(mvHwsMtipIsReducedPort(devNum, port))
            {
                return GT_NO_SUCH;
            }
            if ((HWS_DEV_SILICON_TYPE(devNum) == AC5X) && (port <= MV_HWS_AC5X_GOP_PORT_CPU_1_CNS))
            {
                return GT_NO_SUCH;
            }

            rc = mvHwsGlobalMacToLocalIndexConvert(devNum, port, _10GBase_KR /*NON SEG PORT MODE*/, &convertIdx);
            if(rc != GT_OK)
            {
                return rc;
            }
            if (HWS_DEV_SILICON_TYPE(devNum) == AC5X)
            {
                if (convertIdx.ciderIndexInUnit == 0)
                    txCreditAlloc = 128;
                else if (convertIdx.ciderIndexInUnit == 2)
                    txCreditAlloc = 80;
                else
                    txCreditAlloc = 72;
            }
            fieldNameTxCreditAlloc = MIF_TX_REGISTER_MIF_T32_TX_CREDIT_REGISTER0_T32_TX_CREDIT_ALLOC_E;
            fieldNameTxChannelId = MIF_GLOBAL_REGISTER_MIF_T32_CHANNEL_MAPPING_REGISTER0_T32_TX_CHID_NUM_E;
            fieldNameRxChannelId = MIF_GLOBAL_REGISTER_MIF_T32_CHANNEL_MAPPING_REGISTER0_T32_RX_CHID_NUM_E;
            fieldNameChTag = MIF_GLOBAL_REGISTER_MIF_T32_CHANNEL_MAPPING_REGISTER0_T32_CH_TAG_E;
            fieldNameEopLinkDown = MIF_TX_REGISTER_MIF_T32_LINK_FSM_CONTROL_REGISTER0_T32_EOP_AT_LINK_DOWN_E;
            fieldNameRxGracefulMode = MIF_GLOBAL_REGISTER_MIF_T32_CHANNEL_MAPPING_REGISTER0_T32_RX_GRACEFUL_MODE_E;
            fieldNameClkEn = MIF_GLOBAL_REGISTER_MIF_T32_CHANNEL_MAPPING_REGISTER0_T32_CLK_EN_E;
            fieldNameIgnoreMac = MIF_TX_REGISTER_MIF_T32_TX_CREDIT_REGISTER0_T32_IGNORE_MAC_FILL_LEVEL_E;
            fieldNameIgnoreMif = MIF_TX_REGISTER_MIF_T32_TX_CREDIT_REGISTER0_T32_IGNORE_MIF_FILL_LEVEL_E;
            unitId = MIF_400_UNIT;
            break;

        case MV_HWS_MIF_PORT_TYPE_128_E:
            rc = mvHwsGlobalMacToLocalIndexConvert(devNum, port, _400GBase_KR8 /*SEG PORT MODE*/, &convertIdx);
            if(rc != GT_OK)
            {
                return rc;
            }
            fieldNameTxCreditAlloc = MIF_TX_REGISTER_MIF_T128_TX_CREDIT_REGISTER0_T128_TX_CREDIT_ALLOC_E;
            fieldNameTxChannelId = MIF_GLOBAL_REGISTER_MIF_T128_CHANNEL_MAPPING_REGISTER0_T128_TX_CHID_NUM_E;
            fieldNameRxChannelId = MIF_GLOBAL_REGISTER_MIF_T128_CHANNEL_MAPPING_REGISTER0_T128_RX_CHID_NUM_E;
            fieldNameChTag = MIF_GLOBAL_REGISTER_MIF_T128_CHANNEL_MAPPING_REGISTER0_T128_CH_TAG_E;
            fieldNameEopLinkDown = MIF_TX_REGISTER_MIF_T128_LINK_FSM_CONTROL_REGISTER0_T128_EOP_AT_LINK_DOWN_E;
            fieldNameRxGracefulMode = MIF_GLOBAL_REGISTER_MIF_T128_CHANNEL_MAPPING_REGISTER0_T128_RX_GRACEFUL_MODE_E;
            fieldNameClkEn = MIF_GLOBAL_REGISTER_MIF_T128_CHANNEL_MAPPING_REGISTER0_T128_CLK_EN_E;
            fieldNameIgnoreMac = MIF_TX_REGISTER_MIF_T128_TX_CREDIT_REGISTER0_T128_IGNORE_MAC_FILL_LEVEL_E;
            fieldNameIgnoreMif = MIF_TX_REGISTER_MIF_T128_TX_CREDIT_REGISTER0_T128_IGNORE_MIF_FILL_LEVEL_E;
            unitId = MIF_400_SEG_UNIT;
            break;

        case MV_HWS_MIF_PORT_TYPE_8_CPU_E:
            rc = mvHwsGlobalMacToLocalIndexConvert(devNum, port, _25GBase_KR, &convertIdx);
            if(rc != GT_OK)
            {
                return rc;
            }
            if (HWS_DEV_SILICON_TYPE(devNum) == AC5X)
            {
                txCreditAlloc = 176;
            }
            fieldNameTxCreditAlloc  = MIF_TX_REGISTER_MIF_T8_TX_CREDIT_REGISTER0_T8_TX_CREDIT_ALLOC_E;
            fieldNameIgnoreMac      = MIF_TX_REGISTER_MIF_T8_TX_CREDIT_REGISTER0_T8_IGNORE_MAC_FILL_LEVEL_E;
            fieldNameIgnoreMif      = MIF_TX_REGISTER_MIF_T8_TX_CREDIT_REGISTER0_T8_IGNORE_MIF_FILL_LEVEL_E;
            fieldNameTxChannelId    = MIF_GLOBAL_REGISTER_MIF_T8_CHANNEL_MAPPING_REGISTER0_T8_TX_CHID_NUM_E;
            fieldNameRxChannelId    = MIF_GLOBAL_REGISTER_MIF_T8_CHANNEL_MAPPING_REGISTER0_T8_RX_CHID_NUM_E;
            fieldNameChTag          = MIF_GLOBAL_REGISTER_MIF_T8_CHANNEL_MAPPING_REGISTER0_T8_CH_TAG_E;
            fieldNameRxGracefulMode = MIF_GLOBAL_REGISTER_MIF_T8_CHANNEL_MAPPING_REGISTER0_T8_RX_GRACEFUL_MODE_E;
            fieldNameClkEn          = MIF_GLOBAL_REGISTER_MIF_T8_CHANNEL_MAPPING_REGISTER0_T8_CLK_EN_E;
            fieldNameEopLinkDown    = MIF_TX_REGISTER_MIF_T8_LINK_FSM_CONTROL_REGISTER0_T8_EOP_AT_LINK_DOWN_E;
            unitId = MIF_CPU_UNIT;
            break;

        case MV_HWS_MIF_PORT_TYPE_8_E:
            rc = mvHwsGlobalMacToLocalIndexConvert(devNum, port, _10G_OUSGMII /*USX PORT MODE*/, &convertIdx);
            if(rc != GT_OK)
            {
                return rc;
            }
            if (HWS_DEV_SILICON_TYPE(devNum) == AC5X)
            {
                if ((convertIdx.ciderIndexInUnit == 0) )
                    txCreditAlloc = 176;
                else
                    txCreditAlloc = 112;
            }

            fieldNameTxCreditAlloc  = MIF_TX_REGISTER_MIF_T8_TX_CREDIT_REGISTER0_T8_TX_CREDIT_ALLOC_E;
            fieldNameIgnoreMac      = MIF_TX_REGISTER_MIF_T8_TX_CREDIT_REGISTER0_T8_IGNORE_MAC_FILL_LEVEL_E;
            fieldNameIgnoreMif      = MIF_TX_REGISTER_MIF_T8_TX_CREDIT_REGISTER0_T8_IGNORE_MIF_FILL_LEVEL_E;
            fieldNameTxChannelId    = MIF_GLOBAL_REGISTER_MIF_T8_CHANNEL_MAPPING_REGISTER0_T8_TX_CHID_NUM_E;
            fieldNameRxChannelId    = MIF_GLOBAL_REGISTER_MIF_T8_CHANNEL_MAPPING_REGISTER0_T8_RX_CHID_NUM_E;
            fieldNameChTag          = MIF_GLOBAL_REGISTER_MIF_T8_CHANNEL_MAPPING_REGISTER0_T8_CH_TAG_E;
            fieldNameRxGracefulMode = MIF_GLOBAL_REGISTER_MIF_T8_CHANNEL_MAPPING_REGISTER0_T8_RX_GRACEFUL_MODE_E;
            fieldNameClkEn          = MIF_GLOBAL_REGISTER_MIF_T8_CHANNEL_MAPPING_REGISTER0_T8_CLK_EN_E;
            fieldNameEopLinkDown    = MIF_TX_REGISTER_MIF_T8_LINK_FSM_CONTROL_REGISTER0_T8_EOP_AT_LINK_DOWN_E;
            unitId = MIF_USX_UNIT;
            break;


        default:
            return GT_BAD_PARAM;
    }

    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameTxCreditAlloc, txCreditAlloc, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameTxChannelId, convertIdx.expressChannelId, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameRxChannelId, convertIdx.expressChannelId, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameChTag, chTag, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameEopLinkDown, eopLinkDown, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameRxGracefulMode, rxGracefulMode, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameClkEn, clkEn, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameIgnoreMac, ignoreMac, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameIgnoreMif, ignoreMif, NULL));

    if ((convertIdx.preemptionChannelId != 0x3F) && (unitId == MIF_400_UNIT))
    {
        unitId = MIF_400_BR_UNIT;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameTxCreditAlloc, txCreditAlloc, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameTxChannelId, 0x3f, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameRxChannelId, 0x3f, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameChTag, chTag, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameEopLinkDown, eopLinkDown, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameRxGracefulMode, rxGracefulMode, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameClkEn, clkEn, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameIgnoreMac, ignoreMac, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, port, unitId, fieldNameIgnoreMif, ignoreMif, NULL));
    }

    return GT_OK;
}


/*********** MIF 400 *************************************/
/*********** T32 ports ***********************************/
/* Mif idx    | 0  | 1  | 2  | 3  | 4  | 5  | 6  | 7  |  */
/*-------------------------------------------------------*/
/* Channel Id | 0  | 1  | 2  | 6  | 10 | 14 | 18 | 22 |  */
/*********************************************************/
/*********** T32 ports with preemption **********************************************************/
/* Mif idx    | 0  | 1  | 2  | 3  | 4  | 5  | 6  | 7  |  8  | 9  | 10 | 11 | 12 | 13 | 14 | 15 |*/
/*----------------------------------------------------------------------------------------------*/
/* Channel Id | 0  | 1  | 2  | 6  | 10 | 14 | 18 | 22 |  5  | 9  | 3  |  7 | 11 | 15 | 19 | 23 |*/
/************************************************************************************************/
/*********** T128 ports **********************************/
/* Mif idx    | 0  | 1  |                                */
/*-------------------------------------------------------*/
/* Channel Id | 0  | 10 |                                */
/*********************************************************/
/*********** MIF USX *************************************/
/*********** T8 ports ************************************/
/* Mif idx    | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8  | 9  | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 |*/
/*-----------------------------------------------------------------------------------------------------------------------------*/
/* Channel Id | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 |*/
/*******************************************************************************************************************************/

/**
* @internal mvHwsMifInit function
* @endinternal
*
* @brief  MIF unit init
*
* @param[in] devNum                - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMifInit
(
    IN GT_U8 devNum
)
{
    GT_U32 port, numPorts;
    GT_STATUS rc;

    if (hwsIsIronmanAsPhoenix())
    {
        /* Ironman's MIF unit supportded by specific CPSS code           */
        /* MIF devices initialized by prvCpssDxChPortDpIronmanDeviceInit */
        return GT_OK;
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMifInit ******\n");
    }
#endif
    numPorts = hwsDeviceSpecInfo[devNum].portsNum;

   /* for each channel init the MIF parameters */
    for (port = 0; port < numPorts; port++)
    {
        /* Regular port */
        rc = mvHwsMifInitPortPrv(devNum, MV_HWS_MIF_PORT_TYPE_32_E,port);
        if((rc != GT_OK) && (rc != GT_NO_SUCH))
            return rc;

        if (HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum))
        {        /* SEG port */
            rc = mvHwsMifInitPortPrv(devNum, MV_HWS_MIF_PORT_TYPE_128_E, port);
            if((rc != GT_OK) && (rc != GT_NO_SUCH))
                return rc;
        }

        if (HWS_DEV_SILICON_TYPE(devNum) != Harrier) /* no USX unit in Harrier */
        {
            /* USX port */
            rc = mvHwsMifInitPortPrv(devNum, MV_HWS_MIF_PORT_TYPE_8_E, port);
            if((rc != GT_OK) && (rc != GT_NO_SUCH))
                return rc;
        }
    }

    if(HWS_DEV_SILICON_TYPE(devNum) == Harrier)
    {
        /* Harrier : no CPU ports */
    }
    else
    if (HWS_DEV_SILICON_TYPE(devNum) == AC5X)
    {
        /* cpu port */
        rc = mvHwsMifInitPortPrv(devNum, MV_HWS_MIF_PORT_TYPE_8_CPU_E, MV_HWS_AC5X_GOP_PORT_CPU_0_CNS);
        if((rc != GT_OK) && (rc != GT_NO_SUCH))
            return rc;
        rc = mvHwsMifInitPortPrv(devNum, MV_HWS_MIF_PORT_TYPE_8_CPU_E, MV_HWS_AC5X_GOP_PORT_CPU_1_CNS);
        if((rc != GT_OK) && (rc != GT_NO_SUCH))
            return rc;
    }
    if (HWS_DEV_SILICON_TYPE(devNum) == AC5P)
    {
        CHECK_STATUS(mvHwsMifInitPortPrv(devNum, MV_HWS_MIF_PORT_TYPE_8_CPU_E, MV_HWS_AC5P_GOP_PORT_CPU_CNS));
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMifInit ******\n");
    }
#endif

    return GT_OK;
}



/**
* @internal mvHwsMifChannelEnable function
* @endinternal
*
* @brief  MIF channel enable
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical port number
* @param[in] portMode              - port mode
* @param[in] enable                - port enable
* @param[in] isPreemptionEnabled   - preemption enable flag
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMifChannelEnable
(
    IN GT_U8           devNum,
    IN GT_U32          portNum,
    IN MV_HWS_PORT_STANDARD portMode,
    IN GT_BOOL         enable,
    IN GT_BOOL         isPreemptionEnabled
)
{
    GT_STATUS   rc;
    MV_HWS_UNITS_ID unitId;
    GT_U32      fieldNameTxEn, fieldNameRxEn;
    GT_U32      fieldNameTxEn1, fieldNameRxEn1;
    MV_HWS_HAWK_CONVERT_STC convertIdx;


    if (hwsIsIronmanAsPhoenix())
    {
        /* Ironman's MIF unit supportded by specific CPSS code */
        return GT_OK;
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMifChannelEnable ******\n");
    }
#endif
    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    if (rc != GT_OK)
    {
        return GT_BAD_PARAM;
    }
    if ((mvHwsUsxModeCheck(devNum, portNum, portMode)) || (mvHwsMtipIsReducedPort(devNum, portNum)))
    {
        if (mvHwsMtipIsReducedPort(devNum, portNum))
        {
            unitId = MIF_CPU_UNIT;
        }
        else
        {
            unitId = MIF_USX_UNIT;
        }
        if(HWS_DEV_SILICON_TYPE(devNum) == AC5X)
        {
            if (mvHwsMtipIsReducedPort(devNum, portNum))
            {
                MV_HWS_SET_FIELD_IDX0_24_MAC(fieldNameTxEn, convertIdx.ciderUnit, MIF_TX_REGISTER_MIF_T8_TX_CONTROL_REGISTERS_MIF_T8_TX_EN, _E);
                MV_HWS_SET_FIELD_IDX0_24_MAC(fieldNameRxEn, convertIdx.ciderUnit, MIF_RX_REGISTER_MIF_T8_RX_CONTROL_REGISTER_MIF_T8_RX_EN, _E);
            }
            else
            {
                MV_HWS_SET_FIELD_IDX0_24_MAC(fieldNameTxEn, convertIdx.ciderIndexInUnit + 8 * (convertIdx.ciderUnit % 2), MIF_TX_REGISTER_MIF_T8_TX_CONTROL_REGISTERS_MIF_T8_TX_EN, _E);
                MV_HWS_SET_FIELD_IDX0_24_MAC(fieldNameRxEn, convertIdx.ciderIndexInUnit + 8 * (convertIdx.ciderUnit % 2), MIF_RX_REGISTER_MIF_T8_RX_CONTROL_REGISTER_MIF_T8_RX_EN, _E);
            }
        }
        else
        {
            MV_HWS_SET_FIELD_IDX0_24_MAC(fieldNameTxEn, convertIdx.ciderIndexInUnit, MIF_TX_REGISTER_MIF_T8_TX_CONTROL_REGISTERS_MIF_T8_TX_EN, _E);
            MV_HWS_SET_FIELD_IDX0_24_MAC(fieldNameRxEn, convertIdx.ciderIndexInUnit, MIF_RX_REGISTER_MIF_T8_RX_CONTROL_REGISTER_MIF_T8_RX_EN, _E);
        }
    }
    else if (HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        unitId = MIF_400_SEG_UNIT;
        MV_HWS_SET_FIELD_IDX0_1_MAC(fieldNameTxEn, (convertIdx.ciderIndexInUnit/4), MIF_TX_REGISTER_MIF_T128_TX_CONTROL_REGISTERS_MIF_T128_TX_EN, _E);
        MV_HWS_SET_FIELD_IDX0_1_MAC(fieldNameRxEn, (convertIdx.ciderIndexInUnit/4), MIF_RX_REGISTER_MIF_T128_PORT_RX_CONTROL_REGISTER_MIF_T128_RX_EN, _E);
    }
    else
    {
        unitId = MIF_400_UNIT;
        MV_HWS_SET_FIELD_IDX0_15_MAC(fieldNameTxEn, convertIdx.ciderIndexInUnit, MIF_TX_REGISTER_MIF_T32_TX_CONTROL_REGISTERS_MIF_T32_TX_EN, _E);
        MV_HWS_SET_FIELD_IDX0_15_MAC(fieldNameRxEn, convertIdx.ciderIndexInUnit, MIF_RX_REGISTER_MIF_T32_RX_CONTROL_REGISTER_MIF_T32_RX_EN, _E);

        if ((convertIdx.preemptionChannelId != 0x3f) && (isPreemptionEnabled == GT_TRUE))
        {
            GT_U32 regValue = (enable == GT_TRUE) ? convertIdx.preemptionChannelId : 0x3F;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MIF_400_BR_UNIT,
                                                 MIF_GLOBAL_REGISTER_MIF_T32_CHANNEL_MAPPING_REGISTER0_T32_TX_CHID_NUM_E,
                                                 regValue, NULL));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MIF_400_BR_UNIT,
                                                 MIF_GLOBAL_REGISTER_MIF_T32_CHANNEL_MAPPING_REGISTER0_T32_RX_CHID_NUM_E,
                                                 regValue, NULL));

            MV_HWS_SET_FIELD_IDX0_15_MAC(fieldNameTxEn1, (convertIdx.ciderIndexInUnit+8), MIF_TX_REGISTER_MIF_T32_TX_CONTROL_REGISTERS_MIF_T32_TX_EN, _E);
            MV_HWS_SET_FIELD_IDX0_15_MAC(fieldNameRxEn1, (convertIdx.ciderIndexInUnit+8), MIF_RX_REGISTER_MIF_T32_RX_CONTROL_REGISTER_MIF_T32_RX_EN, _E);

            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MIF_400_BR_UNIT, fieldNameTxEn1, enable, NULL));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MIF_400_BR_UNIT, fieldNameRxEn1, enable, NULL));
        }
    }
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, fieldNameTxEn, enable, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, fieldNameRxEn, enable, NULL));
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMifChannelEnable ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsMifOpenDrainSet function
* @endinternal
*
* @brief   Set 'open drain' in MIF for specific port
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] openDrain       - GT_TRUE  -     open drain --> KILL  the traffic from going to the MAC of MTI
*                              GT_FALSE - NOT open drain --> allow the traffic going to the MAC of MTI
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsMifOpenDrainSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 openDrain
)
{
    GT_STATUS   rc;
    MV_HWS_UNITS_ID unitId;
    MV_HWS_MIF_REGISTERS     mifFieldName;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32 regValue;

    if (hwsIsIronmanAsPhoenix())
    {
        /* Ironman's MIF unit supportded by specific CPSS code */
        return GT_OK;
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMifOpenDrainSet ******\n");
    }
#endif

    regValue = (openDrain == GT_TRUE) ? 1 : 0;

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    if (rc != GT_OK)
    {
        return GT_BAD_PARAM;
    }

    if ((mvHwsUsxModeCheck(devNum, portNum, portMode)) || (mvHwsMtipIsReducedPort(devNum, portNum)))
    {
        if (mvHwsMtipIsReducedPort(devNum, portNum))
        {
            unitId = MIF_CPU_UNIT;
        }
        else
        {
            unitId = MIF_USX_UNIT;
        }
        mifFieldName = MIF_TX_REGISTER_MIF_T8_LINK_FSM_CONTROL_REGISTER0_T8_FORCE_LINK_STATUS_E;
    }
    else if (HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        unitId = MIF_400_SEG_UNIT;
        mifFieldName = MIF_TX_REGISTER_MIF_T128_LINK_FSM_CONTROL_REGISTER0_T128_FORCE_LINK_STATUS_E;
    }
    else
    {
        unitId = MIF_400_UNIT;
        mifFieldName = MIF_TX_REGISTER_MIF_T32_LINK_FSM_CONTROL_REGISTER0_T32_FORCE_LINK_STATUS_E;
    }

    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, mifFieldName, regValue, NULL));

    if ((convertIdx.preemptionChannelId != 0x3F) && (unitId == MIF_400_UNIT))
    {
        unitId = MIF_400_BR_UNIT;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, mifFieldName, regValue, NULL));
    }


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMifOpenDrainSet ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsExtSetLaneWidth function
* @endinternal
*
* @brief  Set port lane width (20/40/80)
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] width           - lane width
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsExtSetLaneWidth
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN MV_HWS_SERDES_BUS_WIDTH_ENT width
)
{
    GT_STATUS rc;
    GT_U32 baseAddr, unitIndex, unitNum, i;
    GT_U32 regAddr, regDataSd8X, regDataSdN2, fieldStart, fieldLength;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;

/*
    m_EXT.SDSetSD8X (portIndex, x);
    m_EXT.SDSetSDN2 (portIndex, x);
*/

/*
    TBD according to EPI for 1000Base-X:
    m_EXT.SDSetSDN2 (portIndex, 1); -> 20 bit
*/

    if (mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        return GT_OK;
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsExtSetLaneWidth ******\n");
    }
#endif

    if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
    {
        /* Do nothing regarding cpu_ext_units */

#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsExtSetLaneWidth ******\n");
        }
#endif
        return GT_OK;
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PARAM;
    }

    regDataSd8X = 0;
    regDataSdN2 = 0;
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        switch (width) {
            case _80BIT_ON:
                regDataSdN2 = 0;
                regDataSd8X |= (1 <<(i));
                break;
            case _40BIT_ON:
            case _10BIT_OFF:
                break;
            case _20BIT_ON:
            case _10BIT_ON:
                regDataSdN2 |=  (0x1<<(i));
                regDataSd8X = 0;
                break;
            default:
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsExtSetLaneWidth hws bad width param"));
        }
    }
    fieldLength = curPortParams.numOfActLanes;

    if (HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum))
    {
        /*override field length data with number of lanes parameter (by default from DB - 1) */
        rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode, HAWK_MTIP_EXT_BR_UNITS_GLOBAL_PMA_CONTROL_GC_SD_8X_E, &fieldReg, &convertIdx);
        CHECK_STATUS(rc);
        regAddr = baseAddr + fieldReg.regOffset;

        /* PMA Lane Wide Data Width - gc_sd_8x */
        fieldStart = fieldReg.fieldStart + convertIdx.ciderIndexInUnit;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldStart, fieldLength, regDataSd8X));
    }

    /* PMA Lane Narrow Data Width - gc_sd_n2 */
    rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode,
                              (HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum)) ? HAWK_MTIP_EXT_BR_UNITS_GLOBAL_PMA_CONTROL_GC_SD_N2_E: PHOENIX_MTIP_EXT_UNITS_GLOBAL_PMA_CONTROL_GC_SD_N2_E,
                              &fieldReg, &convertIdx);
    CHECK_STATUS(rc);
    regAddr = baseAddr + fieldReg.regOffset;
    fieldStart = fieldReg.fieldStart + convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldStart, fieldLength, regDataSdN2));


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsExtSetLaneWidth ******\n");
    }
#endif
    return GT_OK;
}


/**
* @internal mvHwsExtFecTypeGet function
* @endinternal
*
* @brief  RS Fec configure get
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical port number
* @param[in] portMode              - port mode
*
* @param[out] fecTypePtr       - pointer to fec mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsExtFecTypeGet
(
    IN GT_U8                   devNum,
    IN  GT_U32                 portGroup,
    IN GT_U32                  portNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_FEC_MODE   *fecTypePtr
)
{
    GT_STATUS                           rc;
    GT_U32                              regData = 0;
    MV_HWS_HAWK_CONVERT_STC             convertIdx;
    MV_HWS_PORT_INIT_PARAMS             curPortParams;  /* current port parameters */
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E  extField;

    GT_UNUSED_PARAM(portGroup);
    HWS_NULL_PTR_CHECK_MAC(fecTypePtr);

    *fecTypePtr = FEC_OFF;

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    if (rc != GT_OK)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams));

    if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
    {
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_RSFEC_UNIT,
                                              CPU_PCS_RSFEC_UNITS_RSFEC_VENDOR_CONTROL_RS_FEC_ENABLE_E, &regData, NULL));
        if (regData == 0x1)
        {
            *fecTypePtr = RS_FEC;
            return GT_OK;
        }

        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_RSFEC_UNIT,
                                              CPU_PCS_RSFEC_UNITS_RSFEC_VENDOR_FEC_CONTROL_FEC_CONTROL_E, &regData, NULL));
        if (regData == 0x1)
        {
            *fecTypePtr = FC_FEC;
            return GT_OK;
        }

        return GT_OK;
    }

    if (mvHwsUsxModeCheck(devNum, portNum, portMode) == GT_TRUE)
    {
        if(HWS_USX_SLOW_MODE_CHECK(portMode) == GT_TRUE)
        {
            /* slow speed don't use fec */
            *fecTypePtr = FEC_OFF;
            return GT_OK;
        }

        CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, MTI_USX_RSFEC_UNIT, portMode,
                                                      USX_RSFEC_UNITS_RSFEC_CONTROL_FEC91_ENA_E, &regData, NULL));
        if (regData == 0x1)
        {
            *fecTypePtr = RS_FEC;
            return GT_OK;
        }

        CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, MTI_USX_RSFEC_UNIT, portMode,
                                                      USX_RSFEC_UNITS_BASE_R_FEC_CONTROL_FEC_ENABLE_E, &regData, NULL));
        if (regData == 0x1)
        {
            *fecTypePtr = FC_FEC;
            return GT_OK;
        }

        return GT_OK;
    }

    /* only for AC5P */
    if (HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_EXT_UNIT,
                                       HAWK_MTIP_EXT_BR_UNITS_GLOBAL_FEC_CONTROL_GC_FEC91_ENA_IN_E, &regData, NULL));

        if(regData & (1 << convertIdx.ciderIndexInUnit))
        {
            CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_EXT_UNIT, HAWK_MTIP_EXT_BR_UNITS_GLOBAL_FEC_CONTROL_GC_KP_MODE_IN_E, &regData, NULL));
            if(regData & (1 << convertIdx.ciderIndexInUnit))
            {
                *fecTypePtr = RS_FEC_544_514;
                return GT_OK;
            }
        }

        return GT_NOT_SUPPORTED;
    }
    else /* regular port */
    {
        extField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ?
             HAWK_MTIP_EXT_BR_UNITS_GLOBAL_FEC_CONTROL_GC_FEC91_ENA_IN_E: PHOENIX_MTIP_EXT_UNITS_GLOBAL_FEC_CONTROL_GC_FEC91_ENA_IN_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_EXT_UNIT, extField, &regData, NULL));

        if(regData & (1 << convertIdx.ciderIndexInUnit))
        {
            if (HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum))
            {
                extField = HAWK_MTIP_EXT_BR_UNITS_GLOBAL_FEC_CONTROL_GC_KP_MODE_IN_E;
                CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_EXT_UNIT, extField, &regData, NULL));
                if(regData & (1 << convertIdx.ciderIndexInUnit))
                {
                    *fecTypePtr = RS_FEC_544_514;
                    return GT_OK;
                }
            }

            *fecTypePtr = RS_FEC;
            return GT_OK;
        }

        extField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ?
             HAWK_MTIP_EXT_BR_UNITS_GLOBAL_FEC_CONTROL_GC_FEC_ENA_E: PHOENIX_MTIP_EXT_UNITS_GLOBAL_FEC_CONTROL_GC_FEC_ENA_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_EXT_UNIT, extField, &regData, NULL));
        if(regData & (1 << convertIdx.ciderIndexInUnit))
        {
             *fecTypePtr = FC_FEC;
             return GT_OK;
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsExtFecTypeSet function
* @endinternal
*
* @brief  Set port fec type
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] fecType         - port fec type
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsExtFecTypeSet
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN MV_HWS_PORT_FEC_MODE        fecType
)
{
    GT_STATUS rc;
    GT_U32 baseAddr, unitIndex, unitNum, i, fieldStart, fieldLength;
    GT_U32 fecRegAddr;
    GT_U32 fecRegData_gc_fec91_ena = 0;
    GT_U32 fecRegData_gc_kp_mode_in = 0;
    GT_U32 fecRegData_gc_fec_ena = 0;
    GT_U32 channelRegData_GC_FEC91 = 0, channelRegData_GC_RXLAUI_ENA = 0;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E extField;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;

/*
    m_EXT.SetFEC91  (portIndex, x);
    m_EXT.SetKP     (portIndex, x);
*/

    if (mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        return GT_OK;
    }

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsExtFecTypeSet ******\n");
    }
#endif

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PARAM;
    }

    if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
    {
#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsExtFecTypeSet ******\n");
        }
#endif

        return GT_OK;
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    CHECK_STATUS(mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx));

    fieldLength = curPortParams.numOfActLanes;

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        switch (fecType) {
            case RS_FEC_544_514:
                if (HWS_DEV_SILICON_TYPE(devNum) == AC5X)
                {
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetFecType hws bad width param"));
                }

                if ((!HWS_PAM4_MODE_CHECK(portMode)) && !((portMode == _200GBase_KR8)  || (portMode == _200GBase_CR8 ) || (portMode == _200GBase_SR_LR8 ))){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetFecType hws bad fec type param"));
                }
                /* set gc_fec91_ena_in[23,16] and gc_kp_mode_in[31,24]*/
                fecRegData_gc_fec91_ena |=  (0x1 << (i));
                fecRegData_gc_kp_mode_in = fecRegData_gc_fec91_ena;
                break;

            case RS_FEC:
                if ((!HWS_25G_MODE_CHECK(portMode)) && (!HWS_TWO_LANES_MODE_CHECK(portMode)) &&
                   (!HWS_100G_R4_MODE_CHECK(portMode))){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetFecType hws bad fec type param"));
                }
                fecRegData_gc_fec91_ena |=  (0x1<<(i));
                break;

            case FC_FEC:
                if ((portMode != _5GBaseR ) && (!HWS_10G_MODE_CHECK(portMode)) &&
                    (!HWS_25G_MODE_CHECK(portMode)) &&(!HWS_40G_R4_MODE_CHECK(portMode)) &&
                    (!HWS_TWO_LANES_MODE_CHECK(portMode))){
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetFecType hws bad fec type param"));
                }
                fecRegData_gc_fec_ena |=  (0x1<<(i));
                break;

            case FEC_OFF:
            case FEC_NA:
                break;
            default:
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsMtipExtSetFecType hws bad fec type param"));
        }
    }

    rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode,
                              HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_MTIP_EXT_BR_UNITS_GLOBAL_FEC_CONTROL_GC_FEC91_ENA_IN_E: PHOENIX_MTIP_EXT_UNITS_GLOBAL_FEC_CONTROL_GC_FEC91_ENA_IN_E,
                              &fieldReg, &convertIdx);
    CHECK_STATUS(rc);
    fecRegAddr = baseAddr + fieldReg.regOffset;
    fieldStart = fieldReg.fieldStart + convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, fecRegAddr, fieldStart, fieldLength, fecRegData_gc_fec91_ena));

    if (HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum))
    {
        rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode, HAWK_MTIP_EXT_BR_UNITS_GLOBAL_FEC_CONTROL_GC_KP_MODE_IN_E, &fieldReg, &convertIdx);
        CHECK_STATUS(rc);
        fieldStart = fieldReg.fieldStart + convertIdx.ciderIndexInUnit;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, fecRegAddr, fieldStart, fieldLength, fecRegData_gc_kp_mode_in));
    }

    rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode,
                              HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_MTIP_EXT_BR_UNITS_GLOBAL_FEC_CONTROL_GC_FEC_ENA_E: PHOENIX_MTIP_EXT_UNITS_GLOBAL_FEC_CONTROL_GC_FEC_ENA_E,
                              &fieldReg, &convertIdx);
    CHECK_STATUS(rc);
    fieldStart = fieldReg.fieldStart + convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, fecRegAddr, fieldStart, fieldLength, fecRegData_gc_fec_ena));

/*
    m_EXT.RXLAUIEnable (portIndex);
    m_EXT.SetFEC91_1LANE_in(portIndex, 0);
*/
    if ((convertIdx.ciderIndexInUnit % 2) == 0) {
        if (HWS_TWO_LANES_MODE_CHECK(portMode))
        {
            switch (fecType) {
                case RS_FEC:
                    channelRegData_GC_FEC91 = 0;
                    channelRegData_GC_RXLAUI_ENA = 0;
                    break;
                case FEC_NA: /* when disabling port - return to default*/
                    channelRegData_GC_FEC91 = 1;
                    channelRegData_GC_RXLAUI_ENA = 0;
                    break;
                default:
                    channelRegData_GC_FEC91 = 0;
                    channelRegData_GC_RXLAUI_ENA = 1;
                    break;
            }
            extField = (HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CHANNEL_CONTROL_GC_FEC91_1LANE_IN0_E: PHOENIX_MTIP_EXT_UNITS_GLOBAL_CHANNEL_CONTROL_GC_FEC91_1LANE_IN0_E) + convertIdx.ciderIndexInUnit/2;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, channelRegData_GC_FEC91, NULL));
            extField = (HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CHANNEL_CONTROL_GC_RXLAUI_ENA_IN0_E: PHOENIX_MTIP_EXT_UNITS_GLOBAL_CHANNEL_CONTROL_GC_RXLAUI_ENA_IN0_E) + convertIdx.ciderIndexInUnit/2;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, channelRegData_GC_RXLAUI_ENA, NULL));
        }
    }

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsExtFecTypeSet ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsExtLowJitterEnable function
* @endinternal
*
* @brief  EXT unit set low jitter for 10G port
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsExtLowJitterEnable
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN GT_BOOL                     enable
)
{
    GT_STATUS rc;
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, fieldStart, fieldLength;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;

    if (HWS_DEV_SILICON_TYPE(devNum) == AC5X)
    {
        return GT_OK;
    }

    if (mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        return GT_OK;
    }

    if (portMode != _10GBase_KR)
    {
        return GT_OK;
    }
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsExtLowJitterEnable ******\n");
    }
#endif

    if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
    {
#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsMtipExtLowJitterEnable ******\n");
        }
#endif
        return GT_OK;
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));
    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode, HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_CONTROL_GC_PACER_10G_E, &fieldReg, &convertIdx);
    CHECK_STATUS(rc);
    fieldStart = fieldReg.fieldStart + convertIdx.ciderIndexInUnit;
    regAddr = baseAddr + fieldReg.regOffset;
    regData = (enable == GT_TRUE) ? 1 : 0;
    fieldLength = 1; /* CIDER field length is 8 - here we need to configure 1 port */
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldStart, fieldLength, regData));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsExtLowJitterEnable ******\n");
    }
#endif

    return GT_OK;
}


/**
* @internal mvHwsExtPcsClockEnable function
* @endinternal
*
* @brief  EXT unit xpcs clock enable + EXT xpcs release
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
* @param[in] serdesFullConfig- the port place in the serdes group
* @param[in] portFullConfig  - the port place in the port group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsExtPcsClockEnable
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      enable,
    IN MV_HWS_PORT_IN_GROUP_ENT     serdesFullConfig,
    IN MV_HWS_PORT_IN_GROUP_ENT     portFullConfig
)
{
    GT_STATUS rc;
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, fieldStart, fieldLength, quadPort;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;
    MV_HWS_MTIP_USX_EXT_UNITS_FIELDS_E usxExtField;
    MV_HWS_PHOENIX_MTIP_EXT_UNIT_FIELDS_E extField;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    regData = (enable == GT_TRUE) ? 1 : 0;

/*
    m_EXT.XPCSClockEnable(portIndex);
    m_EXT.PCS000ClockEnable(portIndex);
*/

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsExtPcsClockEnable ******\n");
    }
#endif

    if (mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE)
    {
        /* Do nothing regarding cpu_ext_units */
#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsExtPcsClockEnable ******\n");
        }
#endif
        return GT_OK;
    }

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);
    quadPort = convertIdx.ciderIndexInUnit / 4;

    if (mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        /* m_EXT.PCSClocksAndResetsEnable(.sd_num(m_SD_num), .low_rate(IsLowRate), .rsfec_enable(fecType == FEC_RS_528_514)); */

        if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
        {
            return GT_BAD_PARAM;
        }

        if((portFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E) || (portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E))
        {
            /* set shared pcs clk & clk */
            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_MAC_CLK_EN_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
        }

        /* Set if any of low rates ports are active*/
        if(HWS_USX_SLOW_MODE_CHECK(portMode) == GT_TRUE)
        {
            if(enable == GT_TRUE)
            {
                usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_SG_MAC_CLK_EN_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
            }
        }

        /* Set on RS-FEC only */
        if (RS_FEC == curPortParams.portFecMode)
        {
            if(enable == GT_TRUE)
            {
                usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_FEC_MAC_CLK_EN_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
            }
        }

        if(portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E)
        {
            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_SG_MAC_CLK_EN_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_FEC_MAC_CLK_EN_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
        }

        return GT_OK;
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));

    if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode, HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_PCS000_CLK_EN_E, &fieldReg, &convertIdx);
        if(GT_OK != rc) return rc;

        fieldLength = fieldReg.fieldLen;
        fieldStart = fieldReg.fieldStart;
        regAddr = baseAddr + fieldReg.regOffset;
    }
    else
    {
        fieldLength = 1; /* CIDER field length is 2 - here we need to configure 1 port */

        rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode,
                                  HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_XPCS_CLK_EN_E: PHOENIX_MTIP_EXT_UNITS_GLOBAL_CLOCK_ENABLE_XPCS_CLK_EN_E,
                                  &fieldReg, &convertIdx);
        if(GT_OK != rc)
            return rc;
        fieldStart = fieldReg.fieldStart;
        fieldStart += quadPort;
        regAddr = baseAddr + fieldReg.regOffset;
    }

    if((serdesFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E) || (serdesFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E))
    {
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldStart, fieldLength, regData));
    }


/*
    Moved from mvHwsExtMacClockEnable():
    m_EXT.SGMIIClockEnable(portIndex);
*/

    /* Low speed PCS MAC clock should be enabled if at least 1 port from 8
         is configured.
         Disable configuration should be done only when power down of last
         low speed port is performed - this part still TBD!!!! */
    if(portMode == _1000Base_X || portMode == _2500Base_X || portMode == SGMII  || portMode == SGMII2_5 )
    {
        if (enable == GT_TRUE)
        {
            extField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ?
                HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_SGREF_CLK_EN_E:
                PHOENIX_MTIP_EXT_UNITS_GLOBAL_CLOCK_ENABLE_SGREF_CLK_EN_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, regData, NULL));
        }
    }
    if (portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E)
    {
        extField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ?
            HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_SGREF_CLK_EN_E:
            PHOENIX_MTIP_EXT_UNITS_GLOBAL_CLOCK_ENABLE_SGREF_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, regData, NULL));
    }


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsExtPcsClockEnable ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsExtPcsResetRelease function
* @endinternal
*
* @brief  EXT unit xpcs reset release/enable
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
* @param[in] serdesFullConfig- the port place in the serdes group
* @param[in] portFullConfig  - the port place in the port group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsExtPcsResetRelease
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      enable,
    IN MV_HWS_PORT_IN_GROUP_ENT     serdesFullConfig,
    IN MV_HWS_PORT_IN_GROUP_ENT     portFullConfig
)
{
    GT_STATUS rc;
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, fieldStart, fieldLength, quadPort;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;
    MV_HWS_MTIP_USX_EXT_UNITS_FIELDS_E usxExtField;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    regData = (enable == GT_TRUE) ? 1 : 0;

/*
    m_EXT.XPCSResetRelease(portIndex);
    m_EXT.PCS000ResetRelease(portIndex);
*/

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsExtPcsResetRelease ******\n");
    }
#endif

    if (mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE)
    {
        /* Do nothing regarding cpu_ext_units */
#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsExtPcsResetRelease ******\n");
        }
#endif
        return GT_OK;
    }

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);
    quadPort = convertIdx.ciderIndexInUnit / 4;

    if (mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        /* m_EXT.PCSClocksAndResetsEnable(.sd_num(m_SD_num), .low_rate(IsLowRate), .rsfec_enable(fecType == FEC_RS_528_514)); */

        if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
        {
            return GT_BAD_PARAM;
        }

        if((portFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E) || (portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E))
        {
            /* set shared pcs clk & clk */
            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_MAC_CLK_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
        }

        /* Set if any of low rates ports are active*/
        if(HWS_USX_SLOW_MODE_CHECK(portMode) == GT_TRUE)
        {
            if(enable == GT_TRUE)
            {
                usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_SG_MAC_CLK_RESET__E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
            }
        }

        /* Set on RS-FEC only */
        if (RS_FEC == curPortParams.portFecMode)
        {
            if(enable == GT_TRUE)
            {
                usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_FEC_MAC_CLK_RESET__E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
            }
        }

        if(portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E)
        {
            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_SG_MAC_CLK_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_FEC_MAC_CLK_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
        }

        return GT_OK;
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));

    if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode, HAWK_MTIP_EXT_BR_UNITS_GLOBAL_RESET_CONTROL_GC_PCS000_RESET__E, &fieldReg, &convertIdx);
        if(GT_OK != rc) return rc;

        fieldLength = fieldReg.fieldLen;
        fieldStart = fieldReg.fieldStart;
        regAddr = baseAddr + fieldReg.regOffset;
    }
    else
    {
        fieldLength = 1; /* CIDER field length is 2 - here we need to configure 1 port */

        rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode,
                                  HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_MTIP_EXT_BR_UNITS_GLOBAL_RESET_CONTROL_GC_XPCS_RESET__E: PHOENIX_MTIP_EXT_UNITS_GLOBAL_RESET_CONTROL_GC_XPCS_RESET__E,
                                  &fieldReg, &convertIdx);
        if(GT_OK != rc)
            return rc;
        fieldStart = fieldReg.fieldStart;
        fieldStart += quadPort;
        regAddr = baseAddr + fieldReg.regOffset;
    }

    if((serdesFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E) || (serdesFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E))
    {
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldStart, fieldLength, regData));
    }


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsExtPcsResetRelease ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsExtFecClockEnable function
* @endinternal
*
* @brief  EXT unit fec clock enable + EXT fec release
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] fecType         - port fec type
* @param[in] enable          - port channel enable /disable
* @param[in] portFullConfig  - the port place in the port group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsExtFecClockEnable
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN MV_HWS_PORT_FEC_MODE         fecType,
    IN GT_BOOL                      enable,
    IN MV_HWS_PORT_IN_GROUP_ENT     portFullConfig
)
{
    GT_U32 regData;
    MV_HWS_PHOENIX_MTIP_EXT_UNIT_FIELDS_E extField;

/*
    m_EXT.FEC91ClockEnable (portIndex);
*/

    if (mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        return GT_OK;
    }

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsExtFecClockEnable ******\n");
    }

    if(mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE)
    {
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsExtFecClockEnable ******\n");
        }
        return GT_OK;
    }
#endif

    portMode = portMode;
    if ((fecType != RS_FEC) && (fecType != RS_FEC_544_514) && (fecType != FC_FEC))
    {
        return GT_OK;
    }

    regData = (enable == GT_TRUE) ? 1 : 0;

    if((enable == GT_TRUE) || (portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E))
    {
        extField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_FEC91_CLK_EN_E: PHOENIX_MTIP_EXT_UNITS_GLOBAL_CLOCK_ENABLE_FEC91_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, regData, NULL));
    }

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsExtFecClockEnable ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsExtFecResetRelease function
* @endinternal
*
* @brief  EXT unit rs-fec reset release/enable
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] fecType         - port fec type
* @param[in] enable          - port channel enable /disable
* @param[in] portFullConfig  - the port place in the port group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsExtFecResetRelease
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN MV_HWS_PORT_FEC_MODE         fecType,
    IN GT_BOOL                      enable,
    IN MV_HWS_PORT_IN_GROUP_ENT     portFullConfig
)
{
    GT_U32 regData;
    MV_HWS_PHOENIX_MTIP_EXT_UNIT_FIELDS_E extField;

/*
    m_EXT.FEC91ResetRelease(portIndex);
*/

    if (mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        return GT_OK;
    }

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsExtFecResetRelease ******\n");
    }

    if(mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE)
    {
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsExtFecResetRelease ******\n");
        }
        return GT_OK;
    }
#endif

    portMode = portMode;
    if ((fecType != RS_FEC) && (fecType != RS_FEC_544_514) && (fecType != FC_FEC))
    {
        return GT_OK;
    }

    regData = (enable == GT_TRUE) ? 1 : 0;

    if((enable == GT_TRUE) || (portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E))
    {
        extField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_MTIP_EXT_BR_UNITS_GLOBAL_RESET_CONTROL_GC_F91_RESET__E: PHOENIX_MTIP_EXT_UNITS_GLOBAL_RESET_CONTROL_GC_F91_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, regData, NULL));
    }

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsExtFecResetRelease ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsExtSetSpeedResolution function
* @endinternal
*
* @brief   set mti speed resolution.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - physical number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsExtSetSpeedResolution
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portNum,
    IN  MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_UNITS_ID unitId;
    GT_U32          speedRes;
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E extField;
/*
    m_RAL.mtip_ext_units_RegFile.MTIP_Port_Control[portIndex].p_port_res_speed.set(miiType);
*/

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsExtSetSpeedResolution ******\n");
    }
#endif

    switch (portMode) {
        case SGMII:
        case QSGMII:
        case SGMII2_5:
        case HWS_D_USX_MODE_CASE:
        case _5G_QUSGMII:
        case _10G_OUSGMII:
        case HWS_Q_USX_MODE_CASE:
        case HWS_SX_MODE_CASE:
            speedRes = 1;
            break;

        case _1000Base_X:
        case _2500Base_X:
            speedRes = 3;
            break;

        case _5GBaseR:
        case HWS_10G_MODE_CASE:
            speedRes = 5;
            break;

        case HWS_25G_MODE_CASE:
            speedRes = 6;
            break;

        case HWS_40G_R1_MODE_CASE:
        case HWS_40G_R2_MODE_CASE:
        case HWS_40G_R4_MODE_CASE:
            speedRes = 7;
            break;

        case HWS_50G_R1_MODE_CASE:
        case HWS_50G_R2_MODE_CASE:
            speedRes = 8;
            break;

        case HWS_100G_PAM4_MODE_CASE:
        case HWS_100G_R4_MODE_CASE:
            speedRes = 9;
            break;

        case HWS_200G_PAM4_MODE_CASE:
        case HWS_200G_R8_MODE_CASE:
            speedRes = 11;
            break;

        case HWS_400G_PAM4_MODE_CASE:
            speedRes = 10;
            break;
        default:
#ifndef RAVEN_DEV_SUPPORT
            if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
                hwsOsPrintf("****** End of mvHwsExtSetSpeedResolution ******\n");
            }
#endif
            return GT_BAD_PARAM;
    }

    if(mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE)
    {
        unitId = MTI_CPU_EXT_UNIT;
        extField = MTIP_CPU_EXT_UNITS_CONTROL_PORT_RES_SPEED_E;
    }
    else if (mvHwsUsxModeCheck(devNum, portNum, portMode) == GT_TRUE)
    {
        unitId = MTI_USX_EXT_UNIT;
        extField =  MTIP_USX_EXT_UNITS_CONTROL1_P0_PORT_RES_SPEED_E;
    }
    else
    {
        unitId = MTI_EXT_UNIT;
        if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
        {
            extField = HAWK_MTIP_EXT_BR_UNITS_CONTROL_SEG_P0_PORT_RES_SPEED_E;

        }
        else
        {
            extField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_MTIP_EXT_BR_UNITS_CONTROL_P0_PORT_RES_SPEED_E : PHOENIX_MTIP_EXT_UNITS_CONTROL_P0_PORT_RES_SPEED_E;
        }
    }

    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, speedRes, NULL));

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsExtSetSpeedResolution ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsExtMacClockEnable function
* @endinternal
*
* @brief  EXT unit clock enable + EXT MAC release
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
* @param[in] portFullConfig  - the port place in the group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsExtMacClockEnable
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      enable,
    IN MV_HWS_PORT_IN_GROUP_ENT     portFullConfig
)
{
    GT_STATUS rc;
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, fieldStart, fieldLength;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;
    MV_HWS_MTIP_USX_EXT_UNITS_FIELDS_E usxExtField;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_MTIP_CPU_EXT_UNITS_FIELDS_E pcsField;
    MV_HWS_PHOENIX_MTIP_EXT_UNIT_FIELDS_E extField;

/*
    m_EXT.MACClockEnable(portIndex);
    m_EXT.MAC400ClockEnable(portIndex);
*/

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsExtMacClockEnable ******\n");
    }
#endif

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    regData = (enable == GT_TRUE) ? 1 : 0;

    if (mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        /* m_EXT.MACClocksAndResetsEnable(.portIndex(portIndex)); */
        if((portFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E) || (portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E))
        {
            /* configure common mac reset & clock */
            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_MAC_CLK_EN_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_APP_CLK_EN_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
        }
        usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_PORT0_MAC_CLK_EN_E + convertIdx.ciderIndexInUnit % 8;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));

        return GT_OK;
    }

    if(mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE)
    {
        /*
            m_RAL.mtip_cpu_ext_units_RegFile.Port_Clocks.pcs_10_25g_clk_en.set(1'b1);
            m_RAL.mtip_cpu_ext_units_RegFile.Port_Clocks.sgpcs_clk_en.set(1'b1);
            m_RAL.mtip_cpu_ext_units_RegFile.Port_Clocks.sd_pcs_rx_clk_en.set(1'b1);
            m_RAL.mtip_cpu_ext_units_RegFile.Port_Clocks.sd_pcs_tx_clk_en.set(1'b1);
            m_RAL.mtip_cpu_ext_units_RegFile.Port_Clocks.app_clk_en.set(1'b1);
            m_RAL.mtip_cpu_ext_units_RegFile.Port_Clocks.mac_clk_en.set(1'b1);
        */

        if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        pcsField = MTIP_CPU_EXT_UNITS_CLOCKS_PCS_10_25G_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_EXT_UNIT, pcsField, regData, NULL));

        pcsField = MTIP_CPU_EXT_UNITS_CLOCKS_SGPCS_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_EXT_UNIT, pcsField, regData, NULL));

        pcsField = MTIP_CPU_EXT_UNITS_CLOCKS_SD_PCS_TX_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_EXT_UNIT, pcsField, regData, NULL));

        pcsField = MTIP_CPU_EXT_UNITS_CLOCKS_SD_PCS_RX_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_EXT_UNIT, pcsField, regData, NULL));

        pcsField = MTIP_CPU_EXT_UNITS_CLOCKS_APP_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_EXT_UNIT, pcsField, regData, NULL));

        pcsField = MTIP_CPU_EXT_UNITS_CLOCKS_MAC_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_EXT_UNIT, pcsField, regData, NULL));

#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsExtMacClockEnable ******\n");
        }
#endif
        return GT_OK;
    }

    if((portFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E) || (portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E))
    {
        /* first will enter with enable, anf last will enter with disable */
        extField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ?
            HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_MAC_APP_CMN_CLK_EN_E:
            PHOENIX_MTIP_EXT_UNITS_GLOBAL_CLOCK_ENABLE_MAC_APP_CMN_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, regData, NULL));

        extField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ?
            HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_MAC_CMN_CLK_EN_E:
            PHOENIX_MTIP_EXT_UNITS_GLOBAL_CLOCK_ENABLE_MAC_CMN_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, regData, NULL));

        extField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ?
            HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_MAC_MAC_CMN_CLK_EN_E:
            PHOENIX_MTIP_EXT_UNITS_GLOBAL_CLOCK_ENABLE_MAC_MAC_CMN_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, regData, NULL));
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));

    if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode,HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_SEG_MAC_CLK_EN_E, &fieldReg, &convertIdx);
        if(GT_OK != rc) return rc;

        fieldLength = 1; /* CIDER field length is 2 - here we need to configure 1 port */
        fieldStart = fieldReg.fieldStart + (convertIdx.ciderIndexInUnit / 4);
        regAddr = baseAddr + fieldReg.regOffset;
    }
    else
    {
        rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode,
                                  HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_MAC_CLK_EN_E: PHOENIX_MTIP_EXT_UNITS_GLOBAL_CLOCK_ENABLE_MAC_CLK_EN_E,
                                  &fieldReg, &convertIdx);
        if(GT_OK != rc) return rc;

        fieldLength = 1; /* CIDER field length is 8 - here we need to configure 1 port */
        fieldStart = fieldReg.fieldStart + convertIdx.ciderIndexInUnit;
        regAddr = baseAddr + fieldReg.regOffset;
    }

    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldStart, fieldLength, regData));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsExtMacClockEnable ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsExtMacClockEnableGet function
* @endinternal
*
* @brief  EXT unit clock enable get
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] enablePtr      - (pointer to) port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsExtMacClockEnableGet
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      *enablePtr
)
{
    GT_STATUS rc;
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, fieldStart, fieldLength;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;
    MV_HWS_MTIP_USX_EXT_UNITS_FIELDS_E usxExtField;
    MV_HWS_MTIP_CPU_EXT_UNITS_FIELDS_E pcsField;


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsExtMacClockEnableGet ******\n");
    }
#endif

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    if (mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_PORT0_MAC_CLK_EN_E + convertIdx.ciderIndexInUnit % 8;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, &regData, NULL));

        *enablePtr = (regData != 0) ? GT_TRUE : GT_FALSE;
        return GT_OK;
    }

    if(mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE)
    {
        pcsField = MTIP_CPU_EXT_UNITS_CLOCKS_MAC_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, MTI_CPU_EXT_UNIT, pcsField, &regData, NULL));
        *enablePtr = (regData != 0) ? GT_TRUE : GT_FALSE;

#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsExtMacClockEnableGet ******\n");
        }
#endif
        return GT_OK;
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));

    if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode,HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_SEG_MAC_CLK_EN_E, &fieldReg, &convertIdx);
        if(GT_OK != rc) return rc;

        fieldLength = 1; /* CIDER field length is 2 - here we need to configure 1 port */
        fieldStart = fieldReg.fieldStart + (convertIdx.ciderIndexInUnit / 4);
        regAddr = baseAddr + fieldReg.regOffset;
    }
    else
    {
        rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode,
                                  HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CLOCK_ENABLE_MAC_CLK_EN_E: PHOENIX_MTIP_EXT_UNITS_GLOBAL_CLOCK_ENABLE_MAC_CLK_EN_E,
                                  &fieldReg, &convertIdx);
        if(GT_OK != rc) return rc;

        fieldLength = 1; /* CIDER field length is 8 - here we need to configure 1 port */
        fieldStart = fieldReg.fieldStart + convertIdx.ciderIndexInUnit;
        regAddr = baseAddr + fieldReg.regOffset;
    }

    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, fieldStart, fieldLength, &regData));

    *enablePtr = (regData != 0) ? GT_TRUE : GT_FALSE;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsExtMacClockEnableGet ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsExtMacResetRelease function
* @endinternal
*
* @brief  EXT unit MAC release
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
* @param[in] portFullConfig  - the port place in the group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsExtMacResetRelease
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      enable,
    IN MV_HWS_PORT_IN_GROUP_ENT     portFullConfig

)
{
    GT_STATUS rc;
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr, regData, fieldStart, fieldLength;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;
    MV_HWS_MTIP_USX_EXT_UNITS_FIELDS_E usxExtField;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_MTIP_CPU_EXT_UNITS_FIELDS_E cpuExtField;
    MV_HWS_PHOENIX_MTIP_EXT_UNIT_FIELDS_E extField;

/*
    m_EXT.MACResetRelease(portIndex);
    m_EXT.MAC400ResetRelease(portIndex);
*/

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsExtMacResetRelease ******\n");
    }
#endif

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    regData = (enable == GT_TRUE) ? 1 : 0;

    if (mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        /* m_EXT.MACClocksAndResetsEnable(.portIndex(portIndex)); */
        if((portFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E) || (portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E))
        {
            /* configure common mac reset & clock */
            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_MAC_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_APP_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
        }
        usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_PORT0_MAC_RESET__E + convertIdx.ciderIndexInUnit % 8;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));

#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsExtMacResetRelease ******\n");
        }
#endif
        return GT_OK;
    }

    if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
    {
        /*
            m_RAL.mtip_cpu_ext_units_RegFile.Port_Reset.port_reset_.set(1'b1);
            m_RAL.mtip_cpu_ext_units_RegFile.Port_Reset.update(status);
        */

        if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        cpuExtField = MTIP_CPU_EXT_UNITS_RESET_PORT_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_EXT_UNIT, cpuExtField, regData, NULL));
#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsExtMacResetRelease ******\n");
        }
#endif

        return GT_OK;
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));

    if((portFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E) || (portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E))
    {
        extField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ?
            HAWK_MTIP_EXT_BR_UNITS_GLOBAL_RESET_CONTROL2_GC_MAC_APP_CMN_RESET__E:
            PHOENIX_MTIP_EXT_UNITS_GLOBAL_RESET_CONTROL2_GC_MAC_APP_CMN_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, regData, NULL));
        extField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ?
            HAWK_MTIP_EXT_BR_UNITS_GLOBAL_RESET_CONTROL2_GC_MAC_MAC_CMN_RESET__E:
            PHOENIX_MTIP_EXT_UNITS_GLOBAL_RESET_CONTROL2_GC_MAC_MAC_CMN_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, regData, NULL));
    }

    if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode, HAWK_MTIP_EXT_BR_UNITS_GLOBAL_RESET_CONTROL_GC_SEG_PORT_RESET__E, &fieldReg, &convertIdx);
        if(GT_OK != rc) return rc;

        fieldLength = 1; /* CIDER field length is 2 - here we need to configure 1 port */
        fieldStart = fieldReg.fieldStart + (convertIdx.ciderIndexInUnit / 4);
        regAddr = baseAddr + fieldReg.regOffset;
    }
    else
    {
        rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode,
                                  HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? HAWK_MTIP_EXT_BR_UNITS_GLOBAL_RESET_CONTROL_GC_PORT_RESET__E: PHOENIX_MTIP_EXT_UNITS_GLOBAL_RESET_CONTROL_GC_PORT_RESET__E,
                                  &fieldReg, &convertIdx);
        if(GT_OK != rc) return rc;

        fieldLength = 1; /* CIDER field length is 8 - here we need to configure 1 port */
        fieldStart = fieldReg.fieldStart + convertIdx.ciderIndexInUnit;
        regAddr = baseAddr + fieldReg.regOffset;
    }

    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldStart, fieldLength, regData));

    /* m_EXT.SGMIIResetRelease(portIndex);

       Port power reduction (part1):
         Low speed PCS MAC clock should be enabled if at least 1 port from 8
         is configured.
         Disable configuration should be done only when power down of last
         low speed port is performed */
    if((portMode == _1000Base_X || portMode == _2500Base_X || portMode == SGMII  || portMode == SGMII2_5))
    {
        if(enable == GT_TRUE)
        {
            extField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ?
                HAWK_MTIP_EXT_BR_UNITS_GLOBAL_RESET_CONTROL_GC_SG_RESET__E:
                PHOENIX_MTIP_EXT_UNITS_GLOBAL_RESET_CONTROL_GC_SG_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, regData, NULL));
        }
    }
    if(portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E)
    {
        extField = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ?
            HAWK_MTIP_EXT_BR_UNITS_GLOBAL_RESET_CONTROL_GC_SG_RESET__E:
            PHOENIX_MTIP_EXT_UNITS_GLOBAL_RESET_CONTROL_GC_SG_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, extField, regData, NULL));
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsExtMacResetRelease ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsExtSetChannelMode function
* @endinternal
*
* @brief  for speeds 100G/200G/400G Set port channel mode
*         (20/40/80)
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsExtSetChannelMode
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN GT_BOOL                     enable
)
{
    GT_STATUS rc;
    GT_U32 baseAddr, unitIndex, unitNum, fieldStart, fieldLength;
    GT_U32 regAddr, regData, evenPort, quadPort;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;

/*
    m_EXT.PCS100Enable  (portIndex);
    m_EXT.Mode40GEnable (portIndex);
    m_EXT.PCS200Enable  (portIndex);
    m_EXT.PCS400Enable();
*/

    if (mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        return GT_OK;
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsExtSetChannelMode ******\n");
    }
#endif
    if ((!HWS_40G_R4_MODE_CHECK(portMode)) &&
        (!HWS_100G_R4_MODE_CHECK(portMode)) &&
        (portMode != _100GBase_KR2) &&
        (portMode != _106GBase_KR2) &&
        (portMode != _102GBase_KR2) &&
        (portMode != _100GBase_CR2) &&
        (portMode != _100GBase_SR_LR2) &&
        (portMode != _200GBase_KR4) &&
        (portMode != _212GBase_KR4) &&
        (portMode != _200GBase_CR4) &&
        (portMode != _200GBase_SR_LR4) &&
        (portMode != _200GBase_KR8) &&
        (portMode != _200GBase_CR8) &&
        (portMode != _200GBase_SR_LR8) &&
        (portMode != _400GBase_CR8) &&
        (portMode != _400GBase_KR8) &&
        (portMode != _424GBase_KR8) &&
        (portMode != _400GBase_SR_LR8))
    {
#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of   mvHwsExtSetChannelMode ******\n");
        }
#endif
        return GT_OK;
    }
    else
    {
        rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
        CHECK_STATUS(rc);

        if (convertIdx.ciderIndexInUnit%2 != 0)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsExtSetChannelMode hws bad port param"));
        }
        else if (!(HWS_100G_R2_MODE_CHECK(portMode)) && (convertIdx.ciderIndexInUnit%4 != 0))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsExtSetChannelMode hws bad port param"));
        }
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));

    regData = (enable == GT_TRUE) ? 1 : 0;
    evenPort = convertIdx.ciderIndexInUnit / 2;
    quadPort = convertIdx.ciderIndexInUnit / 4;

    switch (portMode){
        case _40GBase_KR4:
        case _40GBase_CR4:
        case _40GBase_SR_LR4:
            rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode,
                                      HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? (HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CHANNEL_CONTROL_GC_MODE40_ENA_IN0_E + quadPort): PHOENIX_MTIP_EXT_UNITS_GLOBAL_CHANNEL_CONTROL_GC_MODE40_ENA_IN_E,
                                      &fieldReg, &convertIdx);
            if(GT_OK != rc) return rc;

            regAddr = baseAddr + fieldReg.regOffset;
            fieldStart = fieldReg.fieldStart;
            fieldLength = fieldReg.fieldLen;
            break;

        case _100GBase_CR2:
        case _100GBase_KR2:
        case _102GBase_KR2:
        case _106GBase_KR2:
        case _100GBase_SR_LR2:
        case _106GBase_KR4:
        case _100GBase_KR4:
        case _107GBase_KR4:
        case _100GBase_CR4:
        case _100GBase_SR4:
            rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode,
                                      HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? (HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CHANNEL_CONTROL_GC_PCS100_ENA_IN0_E + evenPort): PHOENIX_MTIP_EXT_UNITS_GLOBAL_CHANNEL_CONTROL_GC_PCS100_ENA_IN_E,
                                       &fieldReg, &convertIdx);
            if(GT_OK != rc) return rc;

            regAddr = baseAddr + fieldReg.regOffset;
            fieldStart = fieldReg.fieldStart;
            fieldLength = fieldReg.fieldLen;
            break;

        case _200GBase_CR4:
        case _200GBase_KR4:
        case _212GBase_KR4:
        case _200GBase_SR_LR4:
            rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode, (HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CHANNEL_CONTROL_GC_PCS400_ENA_IN_E), &fieldReg, &convertIdx);
            if(GT_OK != rc) return rc;

            regAddr = baseAddr + fieldReg.regOffset;
            fieldStart = quadPort + fieldReg.fieldStart;
            fieldLength = 1;
            break;

        case _200GBase_CR8:
        case _200GBase_KR8:
        case _200GBase_SR_LR8:
        case _400GBase_KR8:
        case _424GBase_KR8:
        case _400GBase_CR8:
        case _400GBase_SR_LR8:
            rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode, (HAWK_MTIP_EXT_BR_UNITS_GLOBAL_CHANNEL_CONTROL_GC_PCS400_ENA_IN_E), &fieldReg, &convertIdx);
            if(GT_OK != rc) return rc;

            regAddr = baseAddr + fieldReg.regOffset;
            fieldStart = fieldReg.fieldStart;
            fieldLength = fieldReg.fieldLen;
            regData = regData | (regData << 1);
            break;

        default:
            return GT_BAD_PARAM;
    }
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldStart, fieldLength, regData));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsExtSetChannelMode ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsExtReset function
* @endinternal
*
* @brief  Reset/unreset EXT SD RX/TX unit
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] reset           - true = reset/ false = unreset
* @param[in] portFullConfig  - the port place in the group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsExtReset
(
    IN GT_U8                   devNum,
    IN GT_U32                  portNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 reset,
    IN MV_HWS_PORT_IN_GROUP_ENT portFullConfig
)
{
    GT_STATUS rc;
    GT_U32 baseAddr, unitIndex, unitNum, i;
    GT_U32 regAddr, regData = 0, fieldStart, fieldLength;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;
    GT_U32 sdIndex;
    MV_HWS_MTIP_USX_EXT_UNITS_FIELDS_E usxExtField;

/*
    m_EXT.SDTxResetRelease(portIndex);
    m_EXT.SDRxResetRelease(portIndex);
*/

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsExtReset ******\n");
    }
#endif

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    if (mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        regData = (reset == GT_FALSE) ? 1 : 0;

        /* Power down may be executed only for the last port in group */
        if(regData == 0)
        {
            if(portFullConfig != MV_HWS_PORT_IN_GROUP_LAST_E)
            {
                return GT_OK;
            }
        }

        if (HWS_USX_O_MODE_CHECK(portMode))
        {
            sdIndex = 0;
        }
        else
        {
            /* USX_MAC CIDER defined with portions of 8 x 3different base addresses,
               therefore it is needed to normalize it to %8 and add appropriate base address via
               mvUnitExtInfoGet()*/
            sdIndex = convertIdx.ciderIndexInUnit % 8;
            sdIndex /= 4;
        }

        /* <MTIP IP USX PCS> USXM/USXM Units/CONTROL */
        if(regData == 1)
        {
            /* For the case of single Base_R channel need to use reset in PCS reg file, and not in USXM unit */
            if((portMode == _5GBaseR) ||
               (portMode == _10GBase_KR) ||
               (portMode == _10GBase_SR_LR) ||
               (portMode == _2_5G_SXGMII) ||
               (portMode == _5G_SXGMII) ||
               (portMode == _10G_SXGMII) ||
               ((portMode == _25GBase_KR) && (curPortParams.portFecMode == FEC_OFF)) ||
               ((portMode == _25GBase_SR) && (curPortParams.portFecMode == FEC_OFF)))
            {
                CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, MTI_USX_PCS_UNIT, portMode, USX_PCS_UNITS_CONTROL1_P0_RESET_E, 0x1, NULL));
            }
            else if(portFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E)
            {
                /* reset USXM unit (self cleared configuration) */
                CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, MTI_USX_MULTIPLEXER_UNIT, portMode, USXM_UNITS_CONTROL_RESET_E, 0x1, NULL));
            }
        }

        if(sdIndex == 0)
        {
            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_SD0_TX_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_SD0_RX_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));

            if(HWS_USX_SLOW_MODE_CHECK(portMode) == GT_FALSE)
            {
                usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_FEC_SD0_TX_CLK_EN_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
                usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_FEC_SD0_RX_CLK_EN_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
                usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_FEC_SD0_TX_CLK_RESET__E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
                usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_FEC_SD0_RX_CLK_RESET__E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
            }
        }
        else
        {
            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_SD1_TX_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
            usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_SD1_RX_RESET__E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));

            if(HWS_USX_SLOW_MODE_CHECK(portMode) == GT_FALSE)
            {
                usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_FEC_SD1_TX_CLK_EN_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
                usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_FEC_SD1_RX_CLK_EN_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
                usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_FEC_SD1_TX_CLK_RESET__E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
                usxExtField = MTIP_USX_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_PCS_FEC_SD1_RX_CLK_RESET__E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_USX_EXT_UNIT, usxExtField, regData, NULL));
            }
        }

        return GT_OK;
    }

    if(mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE)
    {
        /*
            m_EXT.SDTxResetRelease();
            m_EXT.SDRxResetRelease();

            m_RAL.mtip_cpu_ext_units_RegFile.Port_Reset.sd_tx_reset_.set(1'b1);
            m_RAL.mtip_cpu_ext_units_RegFile.Port_Reset.sd_rx_reset_.set(1'b1);
        */

        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_EXT_UNIT, MTIP_CPU_EXT_UNITS_RESET_SD_TX_RESET__E, 1, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_EXT_UNIT, MTIP_CPU_EXT_UNITS_RESET_SD_RX_RESET__E, 1, NULL));

#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsExtReset ******\n");
        }
#endif

        return GT_OK;
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, (portNum), &baseAddr, &unitIndex, &unitNum ));

    if(reset == GT_FALSE)
    {
        for (i = 0; i < curPortParams.numOfActLanes; i++)
        {
            regData |= (1 <<(i));
        }
    }
    else
    {
        regData = 0;
    }

    /*override field length data with number of lanes parameter (by default from DB - 1) */
    fieldLength = curPortParams.numOfActLanes;

    /* gc_sd_tx_reset_  */
    rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode,
                              HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? (HAWK_MTIP_EXT_BR_UNITS_GLOBAL_RESET_CONTROL_GC_SD_TX_RESET__E): PHOENIX_MTIP_EXT_UNITS_GLOBAL_RESET_CONTROL_GC_SD_TX_RESET__E,
                              &fieldReg, &convertIdx);
    if(GT_OK != rc)
        return rc;
    regAddr = baseAddr + fieldReg.regOffset;
    fieldStart = fieldReg.fieldStart + convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldStart, fieldLength, regData));

    /* gc_sd_rx_reset_  */
    rc = genUnitRegDbEntryGet(devNum, portNum, MTI_EXT_UNIT, portMode,
                              HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? (HAWK_MTIP_EXT_BR_UNITS_GLOBAL_RESET_CONTROL_GC_SD_RX_RESET__E): PHOENIX_MTIP_EXT_UNITS_GLOBAL_RESET_CONTROL_GC_SD_RX_RESET__E,
                              &fieldReg, &convertIdx);
    if(GT_OK != rc)
        return rc;
    regAddr = baseAddr + fieldReg.regOffset;
    fieldStart = fieldReg.fieldStart + convertIdx.ciderIndexInUnit;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldStart, fieldLength, regData));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsExtReset ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsRsFecConfigure function
* @endinternal
*
* @brief  RS Fec configure
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical port number
* @param[in] portMode              - port mode
* @param[in] fecType               - fec type
* @param[in] portFullConfig        - port configure status in the group
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsRsFecConfigure
(
    IN GT_U8                    devNum,
    IN GT_U32                   portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    IN MV_HWS_PORT_FEC_MODE     fecType,
    IN MV_HWS_PORT_IN_GROUP_ENT portFullConfig
)
{
    GT_STATUS                   rc;
    GT_U32                      regData;
    MV_HWS_HAWK_CONVERT_STC     convertIdx;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;  /* current port parameters */
    MV_HWS_RSFEC_UNIT_FIELDS_E  rsFecField;
    MV_HWS_USX_RSFEC_UNITS_FIELDS_E usxRsFecField;
    MV_HWS_CPU_PCS_RSFEC_UNITS_FIELDS_E cpuRsFecField;
    MV_HWS_USXM_UNITS_FIELDS_E  usxmField;

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    if (rc != GT_OK)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsRsFecConfigure ******\n");
    }
#endif

    if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
    {
        if((FEC_NA == fecType) || (FEC_OFF == fecType))
        {
            cpuRsFecField = CPU_PCS_RSFEC_UNITS_RSFEC_VENDOR_CONTROL_RS_FEC_ENABLE_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_RSFEC_UNIT, cpuRsFecField, 0, NULL));
            cpuRsFecField = CPU_PCS_RSFEC_UNITS_RSFEC_VENDOR_FEC_CONTROL_FEC_CONTROL_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_RSFEC_UNIT, cpuRsFecField, 0, NULL));
        }
        if(FC_FEC == fecType)
        {
            cpuRsFecField = CPU_PCS_RSFEC_UNITS_RSFEC_VENDOR_FEC_CONTROL_FEC_CONTROL_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_RSFEC_UNIT, cpuRsFecField, 1, NULL));
        }
        if(RS_FEC == fecType)
        {
            cpuRsFecField = CPU_PCS_RSFEC_UNITS_RSFEC_VENDOR_CONTROL_RS_FEC_ENABLE_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_RSFEC_UNIT, cpuRsFecField, 1, NULL));
        }

#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsRsFecConfigure ******\n");
        }
#endif

        return GT_OK;
    }

    if (mvHwsUsxModeCheck(devNum, portNum, portMode) == GT_TRUE)
    {
        if(HWS_USX_SLOW_MODE_CHECK(portMode) == GT_TRUE)
        {
#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsRsFecConfigure ******\n");
        }
#endif
            return GT_OK;
        }

        if((fecType == RS_FEC) && (portFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E))
        {
            usxRsFecField = USX_RSFEC_UNITS_RSFEC_CONTROL_FEC91_ENA_E;
            CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, MTI_USX_RSFEC_UNIT, portMode, usxRsFecField, 1, NULL));

            if (HWS_25G_MODE_CHECK(portMode))
            {
                /* USXM::ConfigureBipFor25GRsfec() */
                usxmField = USXM_UNITS_VL0_1_VL0_1_E;
                CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, MTI_USX_MULTIPLEXER_UNIT, portMode, usxmField, 0x3321, NULL));
                usxmField = USXM_UNITS_VL1_1_VL1_1_E;
                CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, MTI_USX_MULTIPLEXER_UNIT, portMode, usxmField, 0x33E6, NULL));
                usxmField = USXM_UNITS_VL2_1_VL2_1_E;
                CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, MTI_USX_MULTIPLEXER_UNIT, portMode, usxmField, 0x339B, NULL));
                usxmField = USXM_UNITS_VL3_1_VL3_1_E;
                CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, MTI_USX_MULTIPLEXER_UNIT, portMode, usxmField, 0x333D, NULL));
            }

        }
        else if(((fecType == FEC_OFF) || (fecType == FEC_NA)) && (portFullConfig == MV_HWS_PORT_IN_GROUP_LAST_E))
        {

            usxRsFecField = USX_RSFEC_UNITS_BASE_R_FEC_CONTROL_FEC_ENABLE_E;
            CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, MTI_USX_RSFEC_UNIT, portMode, usxRsFecField, 0, NULL));

            usxRsFecField = USX_RSFEC_UNITS_RSFEC_CONTROL_FEC91_ENA_E;
            CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, MTI_USX_RSFEC_UNIT, portMode, usxRsFecField, 0, NULL));

            usxmField = USXM_UNITS_VL0_1_VL0_1_E;
            CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, MTI_USX_MULTIPLEXER_UNIT, portMode, usxmField, 0x21, NULL));
            usxmField = USXM_UNITS_VL1_1_VL1_1_E;
            CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, MTI_USX_MULTIPLEXER_UNIT, portMode, usxmField, 0xE6, NULL));
            usxmField = USXM_UNITS_VL2_1_VL2_1_E;
            CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, MTI_USX_MULTIPLEXER_UNIT, portMode, usxmField, 0x9B, NULL));
            usxmField = USXM_UNITS_VL3_1_VL3_1_E;
            CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, MTI_USX_MULTIPLEXER_UNIT, portMode, usxmField, 0x3D, NULL));

        }
        else if((fecType == FC_FEC) && (portFullConfig == MV_HWS_PORT_IN_GROUP_FIRST_E))
        {
            /*
                m_RSFEC.SetFEC74();
                    m_RAL.BASE_R_FEC_CONTROL.FEC_enable.set(1'b1);
            */
            usxRsFecField = USX_RSFEC_UNITS_BASE_R_FEC_CONTROL_FEC_ENABLE_E;
            CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, MTI_USX_RSFEC_UNIT, portMode, usxRsFecField, 1, NULL));
        }

#ifndef  MV_HWS_FREE_RTOS
        if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
            hwsOsPrintf("****** End of mvHwsRsFecConfigure ******\n");
        }
#endif
        return GT_OK;
    }

    if (HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        if (fecType != RS_FEC_544_514)
        {
            return GT_OK;
        }

        if (HWS_IS_PORT_MODE_400G(portMode))
        {
           /* m_RAL.VENDOR_VL0_0.Um0.set(8'h01);
            m_RAL.VENDOR_VL0_0.Um1.set(8'h71);
            m_RAL.VENDOR_VL0_1.Um2.set(8'hf3);*/
            regData = 0x01;
            rsFecField = RSFEC_UNIT_VENDOR_VL0_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0x71;
            rsFecField = RSFEC_UNIT_VENDOR_VL0_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0xf3;
            rsFecField = RSFEC_UNIT_VENDOR_VL0_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        }
        else
        {
           /* m_RAL.VENDOR_VL0_0.Um0.set(8'hb3);
            m_RAL.VENDOR_VL0_0.Um1.set(8'hc0);
            m_RAL.VENDOR_VL0_1.Um2.set(8'h8c);*/
            regData = 0xB3;
            rsFecField = RSFEC_UNIT_VENDOR_VL0_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0xC0;
            rsFecField = RSFEC_UNIT_VENDOR_VL0_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0x8C;
            rsFecField = RSFEC_UNIT_VENDOR_VL0_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        }

        /*m_RAL.VENDOR_VL1_0.Um0.set(8'h5a);
        m_RAL.VENDOR_VL1_0.Um1.set(8'hde);
        m_RAL.VENDOR_VL1_1.Um2.set(8'h7e);*/
        regData = 0x5A;
        rsFecField = RSFEC_UNIT_VENDOR_VL1_0_UM0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        regData = 0xDE;
        rsFecField = RSFEC_UNIT_VENDOR_VL1_0_UM1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        regData = 0x7E;
        rsFecField = RSFEC_UNIT_VENDOR_VL1_1_UM2_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));

        /*m_RAL.VENDOR_VL2_0.Um0.set(8'h3e);
        m_RAL.VENDOR_VL2_0.Um1.set(8'hf3);
        m_RAL.VENDOR_VL2_1.Um2.set(8'h56);*/
        regData = 0x3E;
        rsFecField = RSFEC_UNIT_VENDOR_VL2_0_UM0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        regData = 0xF3;
        rsFecField = RSFEC_UNIT_VENDOR_VL2_0_UM1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        regData = 0x56;
        rsFecField = RSFEC_UNIT_VENDOR_VL2_1_UM2_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));

        /*m_RAL.VENDOR_VL3_0.Um0.set(8'h86);
        m_RAL.VENDOR_VL3_0.Um1.set(8'h80);
        m_RAL.VENDOR_VL3_1.Um2.set(8'hd0);*/
        regData = 0x86;
        rsFecField = RSFEC_UNIT_VENDOR_VL3_0_UM0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        regData = 0x80;
        rsFecField = RSFEC_UNIT_VENDOR_VL3_0_UM1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        regData = 0xD0;
        rsFecField = RSFEC_UNIT_VENDOR_VL3_1_UM2_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));

        /*m_RAL.VENDOR_VL4_0.Um0.set(8'h2a);
        m_RAL.VENDOR_VL4_0.Um1.set(8'h51);
        m_RAL.VENDOR_VL4_1.Um2.set(8'hf2);*/
        regData = 0x2A;
        rsFecField = RSFEC_UNIT_VENDOR_VL4_0_UM0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        regData = 0x51;
        rsFecField = RSFEC_UNIT_VENDOR_VL4_0_UM1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        regData = 0xF2;
        rsFecField = RSFEC_UNIT_VENDOR_VL4_1_UM2_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));

        /*m_RAL.VENDOR_VL5_0.Um0.set(8'h12);
        m_RAL.VENDOR_VL5_0.Um1.set(8'h4f);
        m_RAL.VENDOR_VL5_1.Um2.set(8'hd1);*/
        regData = 0x12;
        rsFecField = RSFEC_UNIT_VENDOR_VL5_0_UM0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        regData = 0x4F;
        rsFecField = RSFEC_UNIT_VENDOR_VL5_0_UM1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        regData = 0xD1;
        rsFecField = RSFEC_UNIT_VENDOR_VL5_1_UM2_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));

        /*m_RAL.VENDOR_VL6_0.Um0.set(8'h42);
        m_RAL.VENDOR_VL6_0.Um1.set(8'h9c);
        m_RAL.VENDOR_VL6_1.Um2.set(8'ha1);*/
        regData = 0x42;
        rsFecField = RSFEC_UNIT_VENDOR_VL6_0_UM0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        regData = 0x9C;
        rsFecField = RSFEC_UNIT_VENDOR_VL6_0_UM1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        regData = 0xA1;
        rsFecField = RSFEC_UNIT_VENDOR_VL6_1_UM2_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));

        /*m_RAL.VENDOR_VL7_0.Um0.set(8'hd6);
        m_RAL.VENDOR_VL7_0.Um1.set(8'h76);
        m_RAL.VENDOR_VL7_1.Um2.set(8'h5b);*/
        regData = 0xD6;
        rsFecField = RSFEC_UNIT_VENDOR_VL7_0_UM0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        regData = 0x76;
        rsFecField = RSFEC_UNIT_VENDOR_VL7_0_UM1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        regData = 0x5B;
        rsFecField = RSFEC_UNIT_VENDOR_VL7_1_UM2_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));

        if (HWS_IS_PORT_MODE_400G(portMode))
        {
            /*m_RAL.VENDOR_VL8_0.Um0.set(8'he1);
            m_RAL.VENDOR_VL8_0.Um1.set(8'h73);
            m_RAL.VENDOR_VL8_1.Um2.set(8'h75);*/
            regData = 0xE1;
            rsFecField = RSFEC_UNIT_VENDOR_VL8_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0x73;
            rsFecField = RSFEC_UNIT_VENDOR_VL8_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0x75;
            rsFecField = RSFEC_UNIT_VENDOR_VL8_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));

            /*m_RAL.VENDOR_VL9_0.Um0.set(8'h71);
            m_RAL.VENDOR_VL9_0.Um1.set(8'hc4);
            m_RAL.VENDOR_VL9_1.Um2.set(8'h3c);*/
            regData = 0x71;
            rsFecField = RSFEC_UNIT_VENDOR_VL9_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0xC4;
            rsFecField = RSFEC_UNIT_VENDOR_VL9_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0x3C;
            rsFecField = RSFEC_UNIT_VENDOR_VL9_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));

            /*m_RAL.VENDOR_VL10_0.Um0.set(8'h95);
            m_RAL.VENDOR_VL10_0.Um1.set(8'heb);
            m_RAL.VENDOR_VL10_1.Um2.set(8'hd8);*/
            regData = 0x95;
            rsFecField = RSFEC_UNIT_VENDOR_VL10_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0xEB;
            rsFecField = RSFEC_UNIT_VENDOR_VL10_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0xD8;
            rsFecField = RSFEC_UNIT_VENDOR_VL10_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));

            /*m_RAL.VENDOR_VL11_0.Um0.set(8'h22);
            m_RAL.VENDOR_VL11_0.Um1.set(8'h66);
            m_RAL.VENDOR_VL11_1.Um2.set(8'h38);*/
            regData = 0x22;
            rsFecField = RSFEC_UNIT_VENDOR_VL11_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0x66;
            rsFecField = RSFEC_UNIT_VENDOR_VL11_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0x38;
            rsFecField = RSFEC_UNIT_VENDOR_VL11_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));

           /* m_RAL.VENDOR_VL12_0.Um0.set(8'ha2);
            m_RAL.VENDOR_VL12_0.Um1.set(8'hf6);
            m_RAL.VENDOR_VL12_1.Um2.set(8'h95);*/
            regData = 0xA2;
            rsFecField = RSFEC_UNIT_VENDOR_VL12_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0xF6;
            rsFecField = RSFEC_UNIT_VENDOR_VL12_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0x95;
            rsFecField = RSFEC_UNIT_VENDOR_VL12_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));

            /*m_RAL.VENDOR_VL13_0.Um0.set(8'h31);
            m_RAL.VENDOR_VL13_0.Um1.set(8'h97);
            m_RAL.VENDOR_VL13_1.Um2.set(8'hc3);*/
            regData = 0x31;
            rsFecField = RSFEC_UNIT_VENDOR_VL13_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0x97;
            rsFecField = RSFEC_UNIT_VENDOR_VL13_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0xC3;
            rsFecField = RSFEC_UNIT_VENDOR_VL13_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));

            /*m_RAL.VENDOR_VL14_0.Um0.set(8'hca);
            m_RAL.VENDOR_VL14_0.Um1.set(8'hfb);
            m_RAL.VENDOR_VL14_1.Um2.set(8'ha6);*/
            regData = 0xCA;
            rsFecField = RSFEC_UNIT_VENDOR_VL14_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0xFB;
            rsFecField = RSFEC_UNIT_VENDOR_VL14_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0xA6;
            rsFecField = RSFEC_UNIT_VENDOR_VL14_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));

            /*m_RAL.VENDOR_VL15_0.Um0.set(8'ha6);
            m_RAL.VENDOR_VL15_0.Um1.set(8'hba);
            m_RAL.VENDOR_VL15_1.Um2.set(8'h79);*/
            regData = 0xA6;
            rsFecField = RSFEC_UNIT_VENDOR_VL15_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0xBA;
            rsFecField = RSFEC_UNIT_VENDOR_VL15_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            regData = 0x79;
            rsFecField = RSFEC_UNIT_VENDOR_VL15_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        }

    }
    else /* regular port */
    {
        GT_U32 hawkLocalIndexArr[8]    = {0,1,2,6,10,14,18,22};
        GT_U32 nextPortShift = 1;

        if (HWS_DEV_SILICON_TYPE(devNum) == AC5P)
        {
            nextPortShift = hawkLocalIndexArr[convertIdx.ciderIndexInUnit + 1] - hawkLocalIndexArr[convertIdx.ciderIndexInUnit];
        }
        else if (HWS_DEV_SILICON_TYPE(devNum) == Harrier)
        {
            nextPortShift = 2;
        }

        /* 50R2 modes */
        if (HWS_TWO_LANES_MODE_CHECK(portMode))
        {
            if(curPortParams.portFecMode != FEC_OFF) /* if FEC_OFF/FC_FEC - can't access to RSFEC_UNIT because of closed clock/reset */
            {
                regData = (fecType == RS_FEC)? 1: 0/*FEC_NA - means port power down*/;
                if(hwsDeviceSpecInfo[devNum].devType == AC5X)
                {
                    rsFecField = PHOENIX_RSFEC_UNIT_RSFEC_CONTROL_P0_TC_PAD_VALUE_E;
                }
                else
                {
                    rsFecField = RSFEC_UNIT_RSFEC_CONTROL_P0_TC_PAD_VALUE_E;
                }
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum + nextPortShift, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            }
        }
        /* 100R2 modes */
        else if(HWS_100G_R2_MODE_CHECK(portMode))
        {
            regData = (fecType == RS_FEC_544_514)? 1: 0;
            rsFecField = RSFEC_UNIT_RSFEC_CONTROL_P0_AM16_COPY_DIS_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum + nextPortShift, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
        }
        else if(HWS_100G_R4_MODE_CHECK(portMode))
        {
            if(curPortParams.portFecMode != FEC_OFF) /* if FEC_OFF/FC_FEC - can't access to RSFEC_UNIT because of closed clock/reset */
            {
                regData = (fecType == RS_FEC) ? 0x0 : 0x33/*FEC_NA - port power down*/;

                if(hwsDeviceSpecInfo[devNum].devType == AC5X)
                {
                    switch(convertIdx.ciderIndexInUnit)
                    {
                        case 0:
                            rsFecField = PHOENIX_RSFEC_UNIT_RX_RSFEC_FIFO_LVL_CH0_F91_FIFO_THRESH_0_E;
                            break;
                        case 1:
                            rsFecField = PHOENIX_RSFEC_UNIT_RX_RSFEC_FIFO_LVL_CH1_F91_FIFO_THRESH_1_E;
                            break;
                        case 2:
                            rsFecField = PHOENIX_RSFEC_UNIT_RX_RSFEC_FIFO_LVL_CH2_F91_FIFO_THRESH_2_E;
                            break;
                        case 3:
                            rsFecField = PHOENIX_RSFEC_UNIT_RX_RSFEC_FIFO_LVL_CH3_F91_FIFO_THRESH_3_E;
                            break;
                        default:
                            return GT_BAD_STATE;
                    }
                }
                else
                {
                    rsFecField = RSFEC_UNIT_RSFEC_DEC_THRESH_P0_RSFEC_DEC_THRESH_E;
                }
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_RSFEC_UNIT, rsFecField, regData, NULL));
            }
        }
    }
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsRsFecConfigure ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsMtiUsxmCfg function
* @endinternal
*
* @brief   config usxm.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiUsxmCfg
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_UNITS_ID unitId;
    GT_U32          fieldName, markerInterval=0;
    GT_U32          activePorts = 0;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(mvHwsMarkerIntervalCalc(devNum, portMode, curPortParams.portFecMode, &markerInterval));

    switch (portMode) {
        case _20G_DXGMII:
        case _10G_DXGMII:
        case _5G_DXGMII:
            if (curPortParams.portFecMode == FEC_OFF)
            {
                markerInterval /= 2;
            }
            else
            {
                markerInterval /= 4;
            }

            activePorts = 2;
            break;

        case _10G_QXGMII:
        case _20G_QXGMII:
        case QSGMII:
        case _5G_QUSGMII:
            markerInterval /= 4;
            activePorts = 4;
            break;

        case _20G_OXGMII:
        case _10G_OUSGMII:
            if (curPortParams.portFecMode == FEC_OFF)
            {
                markerInterval /= 8;
            }
            else
            {
                markerInterval /= 4;
            }
            activePorts = 8;
            break;

        default:
            activePorts = 0;
            break;
    }

    /* m_USXM.ConfigureActivePorts(5'h4);  */
    unitId = MTI_USX_MULTIPLEXER_UNIT;
    fieldName = USXM_UNITS_PORTS_ENA_ACTIVE_PORTS_USED_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, phyPortNum, unitId, portMode, fieldName, activePorts, NULL));

    if (markerInterval > 0)
    {
        /* m_USXM.Configure(usxMarkerInterval); */
        fieldName = USXM_UNITS_VL_INTVL_PORT_CYCLE_INTERVAL_E;
        CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, phyPortNum, unitId, portMode, fieldName, markerInterval, NULL));
    }

    if((portMode == _2_5G_SXGMII) || (portMode == _5G_SXGMII) || (portMode == _10G_SXGMII))
    {
        /* m_RSFEC.ConfigureToggleEn(); */
        fieldName = USX_RSFEC_UNITS_RSFEC_VENDOR_TX_FIFO_THRESHOLD_TOGGLE_EN_E;
        CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, phyPortNum, MTI_USX_RSFEC_UNIT, portMode, fieldName, 1, NULL));
    }

    return GT_OK;
}

/**
* @internal mvHwsEthPortPowerUp function
* @endinternal
*
* @brief  port init seq
*
* @param[in] devNum                 - system device number
* @param[in] phyPortNum             - physical port number
* @param[in] portMode               - port mode
* @param[in] portInitInParamPtr     - port info struct
*
* @retval 0                         - on success
* @retval 1                         - on error
*/
GT_STATUS mvHwsEthPortPowerUp
(
    IN GT_U8                            devNum,
    IN GT_U32                           phyPortNum,
    IN MV_HWS_PORT_STANDARD             portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS    *portInitInParamPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_IN_GROUP_ENT stateInSerdesGroup = MV_HWS_PORT_IN_GROUP_FIRST_E;
    MV_HWS_PORT_IN_GROUP_ENT stateInPortGroup = MV_HWS_PORT_IN_GROUP_FIRST_E;
    MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS portAttributes;
    GT_U32 fieldName;
    MV_HWS_UNITS_ID unitId;

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("\n****** Start of mvHwsEthPortPowerUp for portNum=%d ******\n", phyPortNum);
    }
#endif

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PARAM;
    }
    HWS_NULL_PTR_CHECK_MAC(portInitInParamPtr);

     /* should be done before mvHwsExtMacClockEnable() that enables clocks */
    CHECK_STATUS(mvHwsExtIfFirstInPortGroupCheck(devNum, phyPortNum, portMode, &stateInPortGroup));
    CHECK_STATUS(mvHwsExtIfFirstInSerdesGroupCheck(devNum, phyPortNum, portMode, &stateInSerdesGroup));

    CHECK_STATUS(mvHwsExtSetLaneWidth(devNum, phyPortNum, portMode, curPortParams.serdes10BitStatus));
    CHECK_STATUS(mvHwsExtFecTypeSet(devNum, phyPortNum, portMode, curPortParams.portFecMode));
    /*CHECK_STATUS(mvHwsExtLowJitterEnable(devNum, phyPortNum, portMode, GT_TRUE));*/
    CHECK_STATUS(mvHwsExtPcsClockEnable(devNum, phyPortNum, portMode, GT_TRUE, stateInSerdesGroup, stateInPortGroup));
    CHECK_STATUS(mvHwsExtPcsResetRelease(devNum, phyPortNum, portMode, GT_TRUE, stateInSerdesGroup, stateInPortGroup));
    CHECK_STATUS(mvHwsExtFecClockEnable(devNum, phyPortNum, portMode, curPortParams.portFecMode,GT_TRUE, stateInPortGroup));
    CHECK_STATUS(mvHwsExtFecResetRelease(devNum, phyPortNum, portMode, curPortParams.portFecMode,GT_TRUE, stateInPortGroup));
    CHECK_STATUS(mvHwsExtMacClockEnable(devNum, phyPortNum, portMode, GT_TRUE, stateInPortGroup));
    CHECK_STATUS(mvHwsExtMacResetRelease(devNum, phyPortNum, portMode, GT_TRUE, stateInPortGroup));
    CHECK_STATUS(mvHwsExtSetChannelMode(devNum, phyPortNum,portMode,GT_TRUE));
    CHECK_STATUS(mvHwsRsFecConfigure(devNum, phyPortNum, portMode, curPortParams.portFecMode, stateInSerdesGroup));
    CHECK_STATUS(mvHwsExtSetSpeedResolution(devNum, phyPortNum, portMode));

    if (mvHwsUsxModeCheck(devNum, phyPortNum, portMode) ||
        ((HWS_DEV_SILICON_TYPE(devNum) == Harrier) && (portMode == _10G_SXGMII || portMode == _5G_SXGMII || portMode == _2_5G_SXGMII)))
    {
        CHECK_STATUS(mvHwsMtiUsxReplicationSet(devNum, 0, phyPortNum, portMode, portInitInParamPtr, GT_FALSE));
    }

    /* For multi-USX modes (DX- QX- OX-) USM Multiplexer should be set only for the first port in SD group */
    if((mvHwsUsxModeCheck(devNum, phyPortNum, portMode)) && (stateInSerdesGroup == MV_HWS_PORT_IN_GROUP_FIRST_E))
    {
        CHECK_STATUS(mvHwsMtiUsxmCfg(devNum, 0, phyPortNum, portMode));
    }

    /**********************************************/
    /**********************************************/
    /**********************************************/
    /* Temp solution to provide additional info
       to MAC mode func via portGroup param.
       Later should be redesigned with additional
       structure for parameters. */
    /**********************************************/
    /**********************************************/
    /**********************************************/
    /* Configure MAC/PCS */
    portAttributes.preemptionEnable = portInitInParamPtr->isPreemptionEnabled;
    portAttributes.portSpeed = portInitInParamPtr->portSpeed;
    CHECK_STATUS(mvHwsPortModeCfg(devNum, 0, phyPortNum, portMode, &portAttributes));
    /* Un-Reset the port */
    CHECK_STATUS(mvHwsPortStartCfg(devNum, 0, phyPortNum, portMode));
    /* Un-Reset mtip */
    CHECK_STATUS(mvHwsExtReset(devNum, phyPortNum, portMode, GT_FALSE, stateInSerdesGroup));

    if (( portInitInParamPtr->apEnabled == GT_TRUE ) || (PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpUsedForStaticPort)))
    {
       if(mvHwsMtipIsReducedPort(devNum, phyPortNum))
        {
            unitId = ANP_CPU_UNIT;
        }
        else if(mvHwsUsxModeCheck(devNum, phyPortNum, portMode))
        {
            unitId = ANP_USX_UNIT;
        }
        else
        {
            unitId = ANP_400_UNIT;
        }
        /* Set pcs_cfg_done (ANP_RF)
           m_RAL_ANP.anp_units_RegFile.Port_Status[port_num].p_pcs_cfg_done.set(1'b1); */
       CHECK_STATUS(mvHwsAnpCfgPcsDone(devNum, phyPortNum, portMode));

        if (PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpUsedForStaticPort))
        {
            /* Enable the CH SM: /Cider/EBU-IP/GOP/EPI/ANP_IP/ANP_IP 1.0 (hawk1)/ANP_IP {RTLF 8.1 200301.0}/ANP/ANP Units/Port<%n> Control
               Bit[5] - set to 0x1.*/
            if((HWS_DEV_SILICON_TYPE(devNum) == AC5P))
            {
                fieldName = HAWK_ANP_UNITS_PORT0_CONTROL_P0_PCS_EN_E;
            }
            else if ((HWS_DEV_SILICON_TYPE(devNum) == Harrier))
            {
                fieldName = HARRIER_ANP_UNITS_PORT0_CONTROL_P0_PCS_EN_E;
            }
            else
            {
                fieldName = PHOENIX_ANP_UNITS_PORT0_CONTROL_P0_PCS_EN_E;
            }
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldName, 1, NULL));
        }

    }
#if 0
    else
    {
        CHECK_STATUS(mvHwsSerdesDigitalRfResetToggleSet(devNum, portGroup, phyPortNum, portMode, 10));
    }
#endif

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("****** End of mvHwsEthPortPowerUp for portNum=%d ******\n\n", phyPortNum);
    }
#endif

    return GT_OK;
}


/**
* @internal mvHwsEthPortPowerDown function
* @endinternal
*
* @brief  port power down seq
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] stateInPortGroup         - first/existing/last int port group
* @param[in] stateInSerdesGroup       - first/existing/last int serdes group
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsEthPortPowerDown
(
    GT_U8                       devNum,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_IN_GROUP_ENT    stateInPortGroup,
    MV_HWS_PORT_IN_GROUP_ENT    stateInSerdesGroup
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    MV_HWS_RESET pcsReset = POWER_DOWN;
    GT_U32 sdIndex, fieldName;
    MV_HWS_PORT_ACTION action = PORT_POWER_DOWN;
    MV_HWS_UNITS_ID unitId;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("******mvHwsEthPortPowerDown: port %d delete mode %d  port group %d serdes group %d ******\n", phyPortNum, portMode,stateInPortGroup,stateInSerdesGroup);
    }
#endif


    if(mvHwsUsxModeCheck(devNum, phyPortNum, portMode))
    {
        if(stateInSerdesGroup != MV_HWS_PORT_IN_GROUP_LAST_E)
        {
            pcsReset = PARTIAL_POWER_DOWN;
            action = PORT_PARTIAL_POWER_DOWN;
        }
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PARAM;
    }


    CHECK_STATUS(mvHwsRsFecConfigure(devNum, phyPortNum, portMode, FEC_NA, stateInSerdesGroup));
    CHECK_STATUS(mvHwsExtFecTypeSet(devNum, phyPortNum, portMode, FEC_NA));
    CHECK_STATUS(mvHwsExtSetChannelMode(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsExtSetLaneWidth(devNum, phyPortNum, portMode, _10BIT_OFF));
    CHECK_STATUS(mvHwsExtLowJitterEnable(devNum, phyPortNum, portMode, GT_FALSE));
    /* sd reset should be called before pcs reset (AN-154) */
    CHECK_STATUS(mvHwsExtReset(devNum, phyPortNum, portMode, GT_TRUE, stateInSerdesGroup));
    CHECK_STATUS(mvHwsPortStopCfg(devNum, 0, phyPortNum, portMode, action, &(curLanesList[0]), pcsReset, POWER_DOWN));
    CHECK_STATUS(mvHwsExtPcsResetRelease(devNum, phyPortNum, portMode, GT_FALSE, stateInSerdesGroup, stateInPortGroup));
    CHECK_STATUS(mvHwsExtPcsClockEnable(devNum, phyPortNum, portMode, GT_FALSE, stateInSerdesGroup, stateInPortGroup));
    CHECK_STATUS(mvHwsExtFecResetRelease(devNum, phyPortNum, portMode, curPortParams.portFecMode,GT_FALSE, stateInPortGroup));
    CHECK_STATUS(mvHwsExtFecClockEnable(devNum, phyPortNum, portMode, curPortParams.portFecMode,GT_FALSE, stateInPortGroup));
    CHECK_STATUS(mvHwsExtMacResetRelease(devNum, phyPortNum, portMode, GT_FALSE, stateInPortGroup));
    CHECK_STATUS(mvHwsExtMacClockEnable(devNum, phyPortNum, portMode, GT_FALSE, stateInPortGroup));

    /* AN port power down section */
    if(PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpPortDB).anpPortParamPtr[devNum][phyPortNum].modesVector == 0)
    {
        if((stateInSerdesGroup != MV_HWS_PORT_IN_GROUP_LAST_E) && (mvHwsUsxModeCheck(devNum, phyPortNum, portMode)))
        {
            /*do nothing*/
        }
        else
        {
            if(hwsIsIronmanAsPhoenix() == GT_FALSE)
            {
                for(sdIndex = 0; sdIndex < curPortParams.numOfActLanes; sdIndex++)
                {
                    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG,
                                                            curLanesList[sdIndex],
                                                            SDW_LANE_CONTROL_0,
                                                            0x1000000,
                                                            0x1000000));
                    CHECK_STATUS(mvHwsAnpSerdesSdwMuxSet(devNum, curPortParams.activeLanesList[sdIndex], GT_TRUE));

                }
            }
            if (PRV_SHARED_LAB_SERVICES_DIR_ANP_SRC_GLOBAL_VAR_GET(mvHwsAnpUsedForStaticPort))
            {
               if(mvHwsMtipIsReducedPort(devNum, phyPortNum))
                {
                    unitId = ANP_CPU_UNIT;
                }
                else if(mvHwsUsxModeCheck(devNum, phyPortNum, portMode))
                {
                    unitId = ANP_USX_UNIT;
                }
                else
                {
                    unitId = ANP_400_UNIT;
                }
                /* Enable the CH SM: /Cider/EBU-IP/GOP/EPI/ANP_IP/ANP_IP 1.0 (hawk1)/ANP_IP {RTLF 8.1 200301.0}/ANP/ANP Units/Port<%n> Control
                   Bit[5] - set to 0x0.*/
                if((HWS_DEV_SILICON_TYPE(devNum) == AC5P))
                {
                    fieldName = HAWK_ANP_UNITS_PORT0_CONTROL_P0_PCS_EN_E;
                }
                else if ((HWS_DEV_SILICON_TYPE(devNum) == Harrier))
                {
                    fieldName = HARRIER_ANP_UNITS_PORT0_CONTROL_P0_PCS_EN_E;
                }
                else
                {
                    fieldName = PHOENIX_ANP_UNITS_PORT0_CONTROL_P0_PCS_EN_E;
                }
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, fieldName, 0, NULL));
            }
        }
    }
    else
    {
        CHECK_STATUS(mvHwsAnpPortStopAn(devNum, phyPortNum));

    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("End of mvHwsEthPortPowerDown **********************\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsMtiUsxReplicationSet function
* @endinternal
*
* @brief   set usx replication.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical number
* @param[in] portMode                 - port mode
* @param[in] portInitInParamPtr       - PM DB
* @param[in] linkUp                   - port link status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiUsxReplicationSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portGroup,
    IN  GT_U32                          phyPortNum,
    IN  MV_HWS_PORT_STANDARD            portMode,
    IN  MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr,
    IN  GT_BOOL                         linkUp
)
{
    MV_HWS_UNITS_ID         unitId;
    GT_U32                  fieldNameGmiiRep, fieldNameGmii_2_5;
    GT_U32                  valGmiiRep, valGmii_2_5;
    GT_U32                  portTypeDefaultSpeed = 0, sgmiiSpeed = 2;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_FLOAT32              replication, an_step, desired_timer = 2000000.0;
    GT_U32                  final_timer;
    MV_HWS_PORT_SPEED_ENT   portSpeed = portInitInParamPtr->portSpeed;
    GT_U32                  regData = 0;
    GT_UREG_DATA            data = 0;
    GT_BOOL                 autoNegEnabled = portInitInParamPtr->autoNegEnabled;

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtiUsxReplicationSet ******\n");
    }
#endif
    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (HWS_DEV_SILICON_TYPE(devNum) == Harrier)
    {
        unitId =  MTI_LOW_SP_PCS_UNIT;
    }
    else
    {
        unitId = MTI_USX_LPCS_UNIT;
    }

    /**
     * The function is also called  in prvCpssPortManagerLinkUpCheck
     * in case of link UP
     */
    switch (linkUp)
    {
    case GT_FALSE:
        switch (portMode)
        {
                /*low speed */
            case SGMII:
            case _1000Base_X:
            case QSGMII:
            case SGMII2_5:
            case _2500Base_X:
            case _5G_QUSGMII:
            case _10G_OUSGMII:
            case _5GBaseR:
            case HWS_10G_MODE_CASE:
            case HWS_25G_MODE_CASE:
                /**
                 * For low speed, the timers must be cleard
                 * and valGmiiRep and valGmii_2_5 set to zero.
                 */
                portTypeDefaultSpeed = 0;
                desired_timer = 0;
                an_step = 12.8;
                /**
                 * For low speed, autonegotiation is configured
                 * separatly
                 */
                autoNegEnabled = GT_FALSE;
                break;

            case _20G_OXGMII:
            case _10G_QXGMII:
            case _5G_DXGMII:
            case _2_5G_SXGMII:
                portTypeDefaultSpeed = 2500;
                an_step = 25.6;
                break;

            case _20G_QXGMII:
            case _10G_DXGMII:
            case _5G_SXGMII:
                portTypeDefaultSpeed = 5000;
                an_step = 12.8;
                break;

            case _20G_DXGMII:
            case _10G_SXGMII:
                portTypeDefaultSpeed = 10000;
                an_step = 6.4;
                break;

            default:
                return GT_NOT_SUPPORTED;
        }

        /**
         * Calculate values for replication and timers for desierd
         * speed:
         *
         * if (Anenable)
         *      m_RAL.lpcs_units_RegFile.p_USXGMII_REP[portIndex].Usxgmiirep.set(1);
         *      m_RAL.lpcs_units_RegFile.p_USXGMII_REP[portIndex].p_Usxgmii2_5.set(0);
         * else
         *      m_RAL.lpcs_units_RegFile.p_USXGMII_REP[portIndex].Usxgmiirep.set(Usxgmiirep);
         *      m_RAL.lpcs_units_RegFile.p_USXGMII_REP[portIndex].p_Usxgmii2_5.set(Usxgmii2_5);
         *
         * final_timer=desired_timer/an_step*2;
         */
        replication = (autoNegEnabled) ? (0x1) : ((GT_FLOAT32)portTypeDefaultSpeed / (GT_FLOAT32)portSpeed);
        final_timer = (GT_U32)((desired_timer / an_step)*2);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, LPCS_UNITS_CONTROL_P0_ANENABLE_E, autoNegEnabled, NULL));

        data = ((autoNegEnabled) ? (final_timer & 0xFFFF) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, LPCS_UNITS_LINK_TIMER_0_P0_TIMER15_1_E, data, NULL));

        data = ((autoNegEnabled) ? ((final_timer >> 16) & 0x1F) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, LPCS_UNITS_LINK_TIMER_1_P0_TIMER20_16_E, data, NULL));

        if (GT_FALSE == autoNegEnabled)
        {
            if (portSpeed == MV_HWS_PORT_SPEED_10M_E)
            {
                sgmiiSpeed = 0;
            }
            else if (portSpeed == MV_HWS_PORT_SPEED_100M_E)
            {
                sgmiiSpeed = 1;
            }
            else
            {
                sgmiiSpeed = 2;
            }

            /* m_RAL.lpcs_units_RegFile.p_IF_MODE[portIndex].p_Sgmii_speed.set(sgmii_speed); */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, LPCS_UNITS_IF_MODE_P0_SGMII_SPEED_E, sgmiiSpeed, NULL));
        }
        else
        {
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, LPCS_UNITS_DEV_ABILITY_P0_LD_ABILITY_RSV05_E, 1, NULL));
        }

        break;

    case GT_TRUE:
        switch (portMode)
        {
            case _20G_OXGMII:
            case _10G_QXGMII:
            case _5G_DXGMII:
            case _2_5G_SXGMII:
                portTypeDefaultSpeed = 2500;
                break;

            case _20G_QXGMII:
            case _10G_DXGMII:
            case _5G_SXGMII:
                portTypeDefaultSpeed = 5000;
                break;

            case _20G_DXGMII:
            case _10G_SXGMII:
                portTypeDefaultSpeed = 10000;
                break;

            case _5G_QUSGMII:
            case _10G_OUSGMII:
                /**
                 * Adjust resolution after AN process completed.
                 *
                 * In OUSGMII, the speed field is 3bits – [11:9] in
                 *
                 * <MTIP_IP>MTIP_IP/<MTIP_IP>
                 * LPCS/LPCS Units %j/PORT<%n>_DEV_ABILITY
                 *
                 * TBD for QUSGMII
                 */
                CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, phyPortNum, unitId, LPCS_UNITS_PARTNER_ABILITY_P0_LP_PABILITY_RSV9_E, &regData, NULL));
                data |= (regData & 0x1);
                CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, phyPortNum, unitId, LPCS_UNITS_PARTNER_ABILITY_P0_LP_PABILITY_RSV10_E, &regData, NULL));
                data |= ((regData & 0x3) << 1);
                /**
                 * Set the relevant speed in
                 *
                 *       <MTIP_IP>MTIP_IP/<MTIP_IP>
                 *       LPCS/LPCS Units %j/PORT<%n>_IF_MODE
                 * In bit[3:2]
                 */
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, LPCS_UNITS_IF_MODE_P0_SGMII_SPEED_E, data, NULL));

                return GT_OK;

            default:
                return GT_NOT_SUPPORTED;
        }

        /**
         *  Get speed from link partner page in case of AN acknowledge
         */
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, phyPortNum, unitId, LPCS_UNITS_PARTNER_ABILITY_P0_LP_PABILITY_RSV9_E, &regData, NULL));
        data |= (regData & 0x1);
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, phyPortNum, unitId, LPCS_UNITS_PARTNER_ABILITY_P0_LP_PABILITY_RSV10_E, &regData, NULL));
        data |= ((regData & 0x3) << 1);
        switch (data)
        {
        case 0:
            portSpeed = MV_HWS_PORT_SPEED_10M_E;
            break;
        case 1:
            portSpeed = MV_HWS_PORT_SPEED_100M_E;
            break;
        case 2:
            portSpeed = MV_HWS_PORT_SPEED_1G_E;
            break;
        case 4:
            portSpeed = MV_HWS_PORT_SPEED_2_5G_E;
            break;
        case 5:
            portSpeed = MV_HWS_PORT_SPEED_5G_E;
            break;
        case 3:
            portSpeed = MV_HWS_PORT_SPEED_10G_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /**
         * Calculate values for replication for confirmed speed
         */
        replication = ((GT_FLOAT32)portTypeDefaultSpeed / (GT_FLOAT32)portSpeed);
        break;

    default:
        return GT_NOT_SUPPORTED;
    }

    /**
     * The replication values must be set for both the desierd and
     * confirmed speed.
     */
    if (replication == 2.5)
    {
        valGmii_2_5 = 1;
        valGmiiRep = 0;
    }
    else
    {
        valGmii_2_5 = 0;
        valGmiiRep = (GT_U32)replication;
    }

    fieldNameGmiiRep = LPCS_UNITS_USXGMII_REP_USXGMIIREP_E;
    fieldNameGmii_2_5 = LPCS_UNITS_USXGMII_REP_P0_USXGMII2_5_E;

    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, fieldNameGmiiRep, valGmiiRep, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, fieldNameGmii_2_5, valGmii_2_5, NULL));

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtiUsxReplicationSet ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsMtiUsxReplicationGet function
* @endinternal
*
* @brief   Get usx replication parameters.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical number
* @param[in] portMode                 - port mode
* @param[out] gmiiRep                 - PM DB
* @param[out] gmii_2_5                - port link status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiUsxReplicationGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portGroup,
    IN  GT_U32                          phyPortNum,
    IN  MV_HWS_PORT_STANDARD            portMode,
    OUT GT_U32                          *gmiiRepPtr,
    OUT GT_U32                          *gmii_2_5Ptr
)
{
    MV_HWS_UNITS_ID             unitId;
    MV_HWS_LPCS_UNITS_FIELDS_E  fieldNameGmiiRep, fieldNameGmii_2_5;
    MV_HWS_LPCS_UNITS_FIELDS_E  fieldSgmiiSpeed;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsMtiUsxReplicationGet ******\n");
    }
#endif

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(curPortParams.portPcsType)
    {
        case MTI_PCS_LOW_SPEED:
            unitId =  MTI_LOW_SP_PCS_UNIT;
            fieldSgmiiSpeed = LPCS_UNITS_IF_MODE_P0_SGMII_SPEED_E;
            break;
        case MTI_USX_PCS_LOW_SPEED:
        case MTI_USX_PCS:
            unitId =  MTI_USX_LPCS_UNIT;
            fieldSgmiiSpeed = LPCS_UNITS_IF_MODE_P0_SGMII_SPEED_E;
            break;
        case MTI_CPU_SGPCS:
            unitId =  MTI_CPU_SGPCS_UNIT;
            fieldSgmiiSpeed = CPU_SGPCS_UNIT_PORT_IF_MODE_PORT_SGMII_SPEED_E;
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    fieldNameGmiiRep = LPCS_UNITS_USXGMII_REP_USXGMIIREP_E;
    fieldNameGmii_2_5 = LPCS_UNITS_USXGMII_REP_P0_USXGMII2_5_E;

    *gmiiRepPtr = 0;
    *gmii_2_5Ptr = 0;

    if(unitId != MTI_CPU_SGPCS_UNIT)
    {
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, phyPortNum, unitId, fieldNameGmiiRep, gmiiRepPtr, NULL));
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, phyPortNum, unitId, fieldNameGmii_2_5, gmii_2_5Ptr, NULL));
    }

    if((*gmiiRepPtr == 0) && (*gmii_2_5Ptr == 0))
    {
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, phyPortNum, unitId, fieldSgmiiSpeed, gmiiRepPtr, NULL));
        switch(*gmiiRepPtr)
        {
            case 0:
                *gmiiRepPtr = 100;
                break;
            case 1:
                *gmiiRepPtr = 10;
                break;
            case 2:
                *gmiiRepPtr = 1;
                break;
            default:
                return GT_BAD_STATE;
        }
    }

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsMtiUsxReplicationGet ******\n");
    }
#endif
    return GT_OK;
}

static GT_STATUS mvHwsMifGlobalRegDump
(
    MV_HWS_UNITS_ID unitId
)
{
    GT_U32 portNum, localIdx = 0;
    GT_U32  unitAddr = 0, unitIndex, unitNum;
    GT_U32          fieldName;
    GT_UREG_DATA    fieldData;
    OUT MV_HWS_REG_ADDR_FIELD_STC fieldRegOut;
    GT_STATUS rc;
    MV_HWS_HAWK_CONVERT_STC convertIdx;

    if(unitId == MIF_400_UNIT)
    {
        hwsOsPrintf("\n********** MIF 400 GLOBAL DUMP ***********\n");
        for (portNum = 0; portNum < MV_HWS_AC5P_GOP_PORT_NUM_CNS; portNum++)
        {
            rc = mvHwsGlobalMacToLocalIndexConvert(0, portNum, _10GBase_KR, &convertIdx);
            if (rc != GT_OK)
            {
                continue;
            }
            localIdx = convertIdx.ciderIndexInUnit;
            hwsOsPrintf("********** Port %d ***** local idx %d*******\n", portNum, localIdx);
            CHECK_STATUS(mvUnitExtInfoGet(0, MIF_400_UNIT, portNum, &unitAddr, &unitIndex,&unitNum));

            fieldName = MIF_GLOBAL_REGISTER_MIF_T32_CHANNEL_MAPPING_REGISTER0_T32_CH_SW_RESET_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_GLOBAL_REGISTER_MIF_T32_CHANNEL_MAPPING_REGISTER0_T32_RX_GRACEFUL_MODE_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_GLOBAL_REGISTER_MIF_T32_CHANNEL_MAPPING_REGISTER0_T32_CH_LOOPBACK_EN_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_GLOBAL_REGISTER_MIF_T32_CHANNEL_MAPPING_REGISTER0_T32_CLK_EN_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_GLOBAL_REGISTER_MIF_T32_CHANNEL_MAPPING_REGISTER0_T32_PFC_SIZE_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_GLOBAL_REGISTER_MIF_T32_CHANNEL_MAPPING_REGISTER0_T32_CH_TAG_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_GLOBAL_REGISTER_MIF_T32_CHANNEL_MAPPING_REGISTER0_T32_RX_CHID_NUM_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_GLOBAL_REGISTER_MIF_T32_CHANNEL_MAPPING_REGISTER0_T32_TX_CHID_NUM_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);
        }
        hwsOsPrintf("**********************\n");
        portNum = 0;
        fieldName = MIF_GLOBAL_REGISTER_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_T32_RX_OVERWRITE_INT_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        fieldName = MIF_GLOBAL_REGISTER_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_T32_RX_PROTOCOL_VIOLATION_INT_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        fieldName = MIF_GLOBAL_REGISTER_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_T32_TX_OVERWRITE_INT_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        fieldName = MIF_GLOBAL_REGISTER_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_T32_TX_PROTOCOL_VIOLATION_INT_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        fieldName = MIF_GLOBAL_REGISTER_MIF_GLOBAL_INTERRUPT_SUMMARY_MASK_MIF_T32_RX_OVERWRITE_INT_MSK_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        fieldName = MIF_GLOBAL_REGISTER_MIF_GLOBAL_INTERRUPT_SUMMARY_MASK_MIF_T32_RX_PROTOCOL_VIOLATION_INT_MSK_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        fieldName = MIF_GLOBAL_REGISTER_MIF_GLOBAL_INTERRUPT_SUMMARY_MASK_MIF_T32_TX_OVERWRITE_INT_MSK_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        fieldName = MIF_GLOBAL_REGISTER_MIF_GLOBAL_INTERRUPT_SUMMARY_MASK_MIF_T32_TX_PROTOCOL_VIOLATION_INT_MSK_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

    }

    if(unitId == MIF_400_SEG_UNIT)
    {
        /*segmented ports */
        for (portNum = 0; portNum < MV_HWS_AC5P_GOP_PORT_NUM_CNS; portNum++)
        {
            rc = mvHwsGlobalMacToLocalIndexConvert(0, portNum, _400GBase_KR8, &convertIdx);
            if(rc != GT_OK)
            {
                continue;
            }
            localIdx = convertIdx.ciderIndexInUnit/4;

            hwsOsPrintf("********** SEG Port %d **** localIdx %d********\n", portNum, localIdx );
            mvUnitExtInfoGet(0, MIF_400_SEG_UNIT, portNum, &unitAddr, &unitIndex,&unitNum);

            fieldName = MIF_GLOBAL_REGISTER_MIF_T128_CHANNEL_MAPPING_REGISTER0_T128_CH_SW_RESET_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_GLOBAL_REGISTER_MIF_T128_CHANNEL_MAPPING_REGISTER0_T128_RX_GRACEFUL_MODE_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_GLOBAL_REGISTER_MIF_T128_CHANNEL_MAPPING_REGISTER0_T128_CH_LOOPBACK_EN_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_GLOBAL_REGISTER_MIF_T128_CHANNEL_MAPPING_REGISTER0_T128_CLK_EN_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_GLOBAL_REGISTER_MIF_T128_CHANNEL_MAPPING_REGISTER0_T128_PFC_SIZE_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_GLOBAL_REGISTER_MIF_T128_CHANNEL_MAPPING_REGISTER0_T128_CH_TAG_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_GLOBAL_REGISTER_MIF_T128_CHANNEL_MAPPING_REGISTER0_T128_RX_CHID_NUM_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_GLOBAL_REGISTER_MIF_T128_CHANNEL_MAPPING_REGISTER0_T128_TX_CHID_NUM_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        }
        localIdx = 0;
        fieldName = MIF_GLOBAL_REGISTER_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_T128_RX_OVERWRITE_INT_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        fieldName = MIF_GLOBAL_REGISTER_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_T128_RX_PROTOCOL_VIOLATION_INT_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        fieldName = MIF_GLOBAL_REGISTER_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_T128_TX_OVERWRITE_INT_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        fieldName = MIF_GLOBAL_REGISTER_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_T128_TX_PROTOCOL_VIOLATION_INT_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        fieldName = MIF_GLOBAL_REGISTER_MIF_GLOBAL_INTERRUPT_SUMMARY_MASK_MIF_T128_RX_OVERWRITE_INT_MSK_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        fieldName = MIF_GLOBAL_REGISTER_MIF_GLOBAL_INTERRUPT_SUMMARY_MASK_MIF_T128_RX_PROTOCOL_VIOLATION_INT_MSK_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        fieldName = MIF_GLOBAL_REGISTER_MIF_GLOBAL_INTERRUPT_SUMMARY_MASK_MIF_T128_TX_OVERWRITE_INT_MSK_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

        fieldName = MIF_GLOBAL_REGISTER_MIF_GLOBAL_INTERRUPT_SUMMARY_MASK_MIF_T128_TX_PROTOCOL_VIOLATION_INT_MSK_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);
    }
    return GT_OK;
}

static GT_STATUS mvHwsMifRxRegDump
(
    MV_HWS_UNITS_ID unitId
)
{
    GT_U32 portNum, localIdx = 0;
    GT_U32  unitAddr = 0, unitIndex, unitNum;
    GT_U32          fieldName;
    GT_UREG_DATA    fieldData;
    OUT MV_HWS_REG_ADDR_FIELD_STC fieldRegOut;
    GT_STATUS rc;
    MV_HWS_HAWK_CONVERT_STC convertIdx;

    if (unitId == MIF_400_UNIT)
    {
        hwsOsPrintf("\n********** MIF 400 RX DUMP ***********\n");
        for (portNum = 0; portNum < MV_HWS_AC5P_GOP_PORT_NUM_CNS; portNum++)
        {
            rc = mvHwsGlobalMacToLocalIndexConvert(0, portNum, _10GBase_KR, &convertIdx);
            if (rc != GT_OK)
            {
                continue;
            }
            localIdx = convertIdx.ciderIndexInUnit;
            hwsOsPrintf("********** Port %d ***** local idx %d*******\n", portNum, localIdx);
            mvUnitExtInfoGet(0, MIF_400_UNIT, portNum, &unitAddr, &unitIndex,&unitNum);
            MV_HWS_SET_FIELD_IDX0_15_MAC(fieldName, localIdx, MIF_RX_REGISTER_MIF_T32_RX_CONTROL_REGISTER_MIF_T32_RX_EN,_E);
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_15_MAC(fieldName, localIdx, MIF_RX_REGISTER_MIF_T32_RX_PROTOCOL_VIOLATION_INTERRUPT_CAUSE_REGISTER_T32_RX_PROTOCOL_VIOLATION_INT,_E);
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_15_MAC(fieldName, localIdx, MIF_RX_REGISTER_MIF_T32_RX_PROTOCOL_VIOLATION_INTERRUPT_MASK_REGISTER_T32_RX_PROTOCOL_VIOLATION_INT_MSK,_E);
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);


            MV_HWS_SET_FIELD_IDX0_15_MAC(fieldName, localIdx, MIF_RX_REGISTER_MIF_T32_RX_OVERWRITE_INTERRUPT_CAUSE_REGISTER_T32_RX_OVERWRITE_VIOLATION_INT,_E);
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);


            MV_HWS_SET_FIELD_IDX0_15_MAC(fieldName, localIdx, MIF_RX_REGISTER_MIF_T32_RX_OVERWRITE_INTERRUPT_MASK_REGISTER_T32_RX_OVERWRITE_VIOLATION_INT_MSK,_E);
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_RX_REGISTER_MIF_T32_RX_STATUS_REGISTER0_T32_RX_BUS_MASTER_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_RX_REGISTER_MIF_T32_RX_STATUS_REGISTER0_T32_RX_PROT_VIO_TYPE_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_RX_REGISTER_MIF_T32_RX_STATUS_REGISTER0_T32_RX_FIFO_EMPTY_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_RX_REGISTER_MIF_T32_RX_STATUS_REGISTER0_T32_RX_FIFO_FULL_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_RX_REGISTER_MIF_T32_RX_STATUS_REGISTER0_T32_RX_SC_BYTE_CNT_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_RX_REGISTER_MIF_T32_RX_GOOD_PACKETS_COUNT0_T32_RX_GOOD_PACKET_CNT_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_RX_REGISTER_MIF_T32_RX_BAD_PACKETS_COUNT0_T32_RX_BAD_PACKET_CNT_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_RX_REGISTER_MIF_T32_RX_DISCARDED_PACKETS_COUNT0_T32_RX_DISCARDED_PACKET_CNT_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_15_MAC(fieldName, localIdx, MIF_RX_REGISTER_MIF_T32_RX_PFC_CONTROL_REGISTER_MIF_T32_RX_PFC_EN, _E);
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);
        }
        hwsOsPrintf("**********************\n");
        fieldName = MIF_RX_REGISTER_MIF_RX_FIFO_READY_THRESHOLD_MIF_T32_RX_FIFO_THRESHOLD_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);
    }

    if (unitId == MIF_400_SEG_UNIT)
    {
        for (portNum = 0; portNum < MV_HWS_AC5P_GOP_PORT_NUM_CNS; portNum++)
        {
            rc = mvHwsGlobalMacToLocalIndexConvert(0, portNum, _400GBase_KR8, &convertIdx);
            if(rc != GT_OK)
            {
                continue;
            }
            localIdx = convertIdx.ciderIndexInUnit/4;
            hwsOsPrintf("********** SEG Port %d **** localIdx %d********\n", portNum, localIdx );
            mvUnitExtInfoGet(0, MIF_400_SEG_UNIT, portNum, &unitAddr, &unitIndex,&unitNum);


            MV_HWS_SET_FIELD_IDX0_1_MAC(fieldName, localIdx, MIF_RX_REGISTER_MIF_T128_PORT_RX_CONTROL_REGISTER_MIF_T128_RX_EN,_E);
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_1_MAC(fieldName, localIdx, MIF_RX_REGISTER_MIF_T128_RX_PROTOCOL_VIOLATION_INTERRUPT_CAUSE_REGISTER_T128_RX_PROTOCOL_VIOLATION_INT,_E);
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_1_MAC(fieldName, localIdx, MIF_RX_REGISTER_MIF_T128_RX_PROTOCOL_VIOLATION_INTERRUPT_MASK_REGISTER_T128_RX_PROTOCOL_VIOLATION_INT_MSK,_E);
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_1_MAC(fieldName, localIdx, MIF_RX_REGISTER_MIF_T128_RX_OVERWRITE_INTERRUPT_CAUSE_REGISTER_T128_RX_OVERWRITE_VIOLATION_INT,_E);
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_1_MAC(fieldName, localIdx, MIF_RX_REGISTER_MIF_T128_RX_OVERWRITE_INTERRUPT_MASK_REGISTER_T128_RX_OVERWRITE_VIOLATION_INT_MSK,_E);
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_RX_REGISTER_MIF_T128_RX_STATUS_REGISTER0_T128_RX_BUS_MASTER_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_RX_REGISTER_MIF_T128_RX_STATUS_REGISTER0_T128_RX_PROT_VIO_TYPE_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_RX_REGISTER_MIF_T128_RX_STATUS_REGISTER0_T128_RX_FIFO_EMPTY_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_RX_REGISTER_MIF_T128_RX_STATUS_REGISTER0_T128_RX_FIFO_FULL_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_RX_REGISTER_MIF_T128_RX_STATUS_REGISTER0_T128_RX_SC_BYTE_CNT_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_RX_REGISTER_MIF_T128_RX_GOOD_PACKETS_COUNT0_T128_RX_GOOD_PACKET_CNT_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_RX_REGISTER_MIF_T128_RX_BAD_PACKETS_COUNT0_T128_RX_BAD_PACKET_CNT_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_RX_REGISTER_MIF_T128_RX_DISCARDED_PACKETS_COUNT0_T128_RX_DISCARDED_PACKET_CNT_E;
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_1_MAC(fieldName, localIdx, MIF_RX_REGISTER_MIF_T128_RX_PFC_CONTROL_REGISTER_MIF_T128_RX_PFC_EN, _E);
            genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);
        }
        fieldName = MIF_RX_REGISTER_MIF_RX_FIFO_READY_THRESHOLD_MIF_T128_RX_FIFO_THRESHOLD_E;
        genUnitRegisterFieldGet(0, 0, portNum, MIF_400_SEG_UNIT, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);
    }
    return GT_OK;
}

static GT_STATUS mvHwsMifTxRegDump
(
    MV_HWS_UNITS_ID unitId
)
{
    GT_U32 portNum, localIdx = 0;
    GT_U32  unitAddr = 0, unitIndex, unitNum;
    GT_U32          fieldName;
    GT_UREG_DATA    fieldData;
    MV_HWS_REG_ADDR_FIELD_STC fieldRegOut;
    GT_STATUS rc;
    MV_HWS_HAWK_CONVERT_STC convertIdx;

    if (unitId == MIF_400_UNIT)
    {
        hwsOsPrintf("\n********** MIF 400 TX DUMP ***********\n");
        for (portNum = 0; portNum < MV_HWS_AC5P_GOP_PORT_NUM_CNS; portNum++)
        {
            rc = mvHwsGlobalMacToLocalIndexConvert(0, portNum, _10GBase_KR, &convertIdx);
            if (rc != GT_OK)
            {
                continue;
            }
            localIdx = convertIdx.ciderIndexInUnit;
            hwsOsPrintf("********** Port %d ***** local idx %d*******\n", portNum, localIdx);
            mvUnitExtInfoGet(0, MIF_400_UNIT, portNum, &unitAddr, &unitIndex,&unitNum);

            MV_HWS_SET_FIELD_IDX0_15_MAC(fieldName, localIdx, MIF_TX_REGISTER_MIF_T32_TX_CONTROL_REGISTERS_MIF_T32_TX_EN, _E);
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T32_LINK_FSM_CONTROL_REGISTER0_T32_EOP_AT_LINK_DOWN_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T32_LINK_FSM_CONTROL_REGISTER0_T32_LINK_STATUS_VALUE_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T32_LINK_FSM_CONTROL_REGISTER0_T32_FORCE_LINK_STATUS_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T32_LINK_FSM_CONTROL_REGISTER0_T32_LINK_FSM_DISABLE_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_15_MAC(fieldName, localIdx, MIF_TX_REGISTER_MIF_T32_TX_PROTOCOL_VIOLATION_INTERRUPT_CAUSE_REGISTER_T32_TX_PROTOCOL_VIOLATION_INT, _E);
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_15_MAC(fieldName, localIdx, MIF_TX_REGISTER_MIF_T32_TX_PROTOCOL_VIOLATION_INTERRUPT_MASK_REGISTER_T32_TX_PROTOCOL_VIOLATION_INT_MSK, _E);
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_15_MAC(fieldName, localIdx, MIF_TX_REGISTER_MIF_T32_TX_OVERWRITE_INTERRUPT_CAUSE_REGISTER_T32_TX_OVERWRITE_VIOLATION_INT, _E);
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_15_MAC(fieldName, localIdx, MIF_TX_REGISTER_MIF_T32_TX_OVERWRITE_INTERRUPT_MASK_REGISTER_T32_TX_OVERWRITE_VIOLATION_INT_MSK, _E);
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T32_TX_STATUS_REGISTER0_T32_TX_CREDIT_CNT_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T32_TX_STATUS_REGISTER0_T32_TX_PROT_VIO_TYPE_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T32_TX_STATUS_REGISTER0_T32_TX_FIFO_EMPTY_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T32_TX_STATUS_REGISTER0_T32_TX_FIFO_FULL_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T32_TX_STATUS_REGISTER0_T32_TX_SC_BYTE_CNT_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T32_TX_CREDIT_REGISTER0_T32_TX_CREDIT_ALLOC_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T32_TX_GOOD_PACKETS_COUNT0_T32_TX_GOOD_PACKET_CNT_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T32_TX_BAD_PACKETS_COUNT0_T32_TX_BAD_PACKET_CNT_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T32_TX_BAD_PACKETS_COUNT0_T32_TX_BAD_PACKET_CNT_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T32_TX_DISCARDED_PACKETS_COUNT0_T32_TX_DISCARDED_PACKET_CNT_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_15_MAC(fieldName, localIdx, MIF_TX_REGISTER_MIF_T32_TX_PFC_CONTROL_REGISTER_MIF_T32_TX_PFC_EN, _E);
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);
        }
        fieldName = MIF_TX_REGISTER_MIF_LINK_STATUS_FILTER_REGISTER_LINK_FILTER_SIZE_E;
        genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
        hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);
    }

    if (unitId == MIF_400_SEG_UNIT)
    {
        for (portNum = 0; portNum < MV_HWS_AC5P_GOP_PORT_NUM_CNS; portNum++)
        {
            rc = mvHwsGlobalMacToLocalIndexConvert(0, portNum, _400GBase_KR8, &convertIdx);
            if(rc != GT_OK)
            {
                continue;
            }
            unitId = MIF_400_SEG_UNIT;
            localIdx = convertIdx.ciderIndexInUnit/4;
            hwsOsPrintf("********** SEG Port %d **** localIdx %d********\n", portNum, localIdx );
            mvUnitExtInfoGet(0, unitId, portNum, &unitAddr, &unitIndex,&unitNum);
            MV_HWS_SET_FIELD_IDX0_1_MAC(fieldName, localIdx, MIF_TX_REGISTER_MIF_T128_TX_CONTROL_REGISTERS_MIF_T128_TX_EN,_E);
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T128_LINK_FSM_CONTROL_REGISTER0_T128_EOP_AT_LINK_DOWN_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T128_LINK_FSM_CONTROL_REGISTER0_T128_LINK_STATUS_VALUE_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);


            fieldName = MIF_TX_REGISTER_MIF_T128_LINK_FSM_CONTROL_REGISTER0_T128_FORCE_LINK_STATUS_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);


            fieldName = MIF_TX_REGISTER_MIF_T128_LINK_FSM_CONTROL_REGISTER0_T128_LINK_FSM_DISABLE_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_1_MAC(fieldName, localIdx, MIF_TX_REGISTER_MIF_T128_TX_PROTOCOL_VIOLATION_INTERRUPT_CAUSE_REGISTER_T128_TX_PROTOCOL_VIOLATION_INT,_E);
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_1_MAC(fieldName, localIdx, MIF_TX_REGISTER_MIF_T128_TX_PROTOCOL_VIOLATION_INTERRUPT_MASK_REGISTER_T128_TX_PROTOCOL_VIOLATION_INT_MSK,_E);
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_1_MAC(fieldName, localIdx, MIF_TX_REGISTER_MIF_T128_TX_OVERWRITE_INTERRUPT_CAUSE_REGISTER_T128_TX_OVERWRITE_VIOLATION_INT,_E);
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_1_MAC(fieldName, localIdx, MIF_TX_REGISTER_MIF_T128_TX_OVERWRITE_INTERRUPT_MASK_REGISTER_T128_TX_OVERWRITE_VIOLATION_INT_MSK,_E);
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T128_TX_STATUS_REGISTER0_T128_TX_CREDIT_CNT_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T128_TX_STATUS_REGISTER0_T128_TX_PROT_VIO_TYPE_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T128_TX_STATUS_REGISTER0_T128_TX_FIFO_EMPTY_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T128_TX_STATUS_REGISTER0_T128_TX_FIFO_FULL_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T128_TX_STATUS_REGISTER0_T128_TX_SC_BYTE_CNT_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T128_TX_CREDIT_REGISTER0_T128_TX_CREDIT_ALLOC_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T128_TX_GOOD_PACKETS_COUNT0_T128_TX_GOOD_PACKET_CNT_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T128_TX_BAD_PACKETS_COUNT0_T128_TX_BAD_PACKET_CNT_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T128_TX_DISCARDED_PACKETS_COUNT0_T128_TX_DISCARDED_PACKET_CNT_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            fieldName = MIF_TX_REGISTER_MIF_T128_TX_LINK_FAIL_COUNT0_T128_TX_LINK_FAIL_CNT_E;
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);

            MV_HWS_SET_FIELD_IDX0_1_MAC(fieldName, localIdx, MIF_TX_REGISTER_MIF_T128_TX_PFC_CONTROL_REGISTER_MIF_T128_TX_PFC_EN,_E);
            genUnitRegisterFieldGet(0,0, portNum, unitId, fieldName, &fieldData, &fieldRegOut);
            hwsOsPrintf("field: %d base: 0x%x offset: 0x%x start: %d len:%d value: 0x%x\n", fieldName, unitAddr, fieldRegOut.regOffset + localIdx * fieldRegOut.offsetFormula, fieldRegOut.fieldStart, fieldRegOut.fieldLen, fieldData);
        }
    }
    return GT_OK;
}

GT_STATUS mvHwsMifRegDump
(
    void
)
{


    if (hwsIsIronmanAsPhoenix())
    {
        /* Ironman's MIF unit supportded by specific CPSS code */
        return GT_OK;
    }

    mvHwsMifGlobalRegDump(MIF_400_UNIT);
    mvHwsMifGlobalRegDump(MIF_400_SEG_UNIT);
    mvHwsMifRxRegDump(MIF_400_UNIT);
    mvHwsMifRxRegDump(MIF_400_SEG_UNIT);
    mvHwsMifTxRegDump(MIF_400_UNIT);
    mvHwsMifTxRegDump(MIF_400_SEG_UNIT);
    return GT_OK;
}


GT_STATUS mvHwsUsxMacDump
(
    void
)
{
    GT_STATUS rc;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;
    GT_U32  unitAddr = 0, unitIndex, unitNum, cluster, regIdx, localIdx, regAddr;
    MV_HWS_HAWK_CONVERT_STC convertIdx;

    hwsOsPrintf("*************** USX MAC *****************\n");
    for (cluster = 0; cluster <= 1; cluster++)
    {
        for(localIdx = (2 + (cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS)); localIdx <= (25 + (cluster * MV_HWS_AC5P_GOP_PORT_NUM_CNS)); localIdx++)
        {
            hwsOsPrintf("PORT_%d\n",localIdx);
            for (regIdx = 0; regIdx < MAC100_REGISTER_LAST_E; regIdx++)
            {
                rc = genUnitRegDbEntryGet(0, localIdx, MTI_USX_MAC_UNIT, _10G_OUSGMII, regIdx, &fieldReg, &convertIdx);
                if(GT_OK != rc) return rc;

                mvUnitExtInfoGet(0, MTI_USX_MAC_UNIT, localIdx, &unitAddr, &unitIndex, &unitNum);

                regAddr = (unitAddr & 0xFFFF0000) + fieldReg.regOffset + fieldReg.offsetFormula * convertIdx.ciderIndexInUnit;
                hwsOsPrintf("field: %d  offset: 0x%x start: %d len:%d \n", regIdx, regAddr , fieldReg.fieldStart, fieldReg.fieldLen);
            }
            hwsOsPrintf("\n\n\n");
        }
    }
    return GT_OK;
}

#ifndef  MV_HWS_FREE_RTOS

typedef struct{
    GT_U32 regAddr;
    GT_U32 expectedData;
    GT_U32 mask;
}MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC;

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC hawkMti400_50G_R1_EXPRESS_RegDb[] = {

    {0x1B800000, 0x00000100, 0xFFFFFFFF},   /* m_400.m_EXT.MTIP_Global_PMA_Control */
    {0x1B800004, 0x01010000, 0xFFFFFFFF},   /* m_400.m_EXT.MTIP_Global_FEC_Control */
    {0x1B800008, 0x5a4c3c00, 0xFFFFFFFF},   /* m_400.m_EXT.MTIP_Global_Channel_Control */
    {0x1B800014, 0xa8010101, 0xFFF7FFFF},   /* m_400.m_EXT.Global_Reset_Control*/
    {0x1B800010, 0x00031c01, 0xFFFFFFFF},   /* m_400.m_EXT.Global_Clock_Enable */
    {0x1B800018, 0x00000003, 0xFFFFFFFF},   /* m_400.m_EXT.Global_Reset_Control2 */
    {0x1B800084, 0x01418010, 0xFFFF01FF},   /* m_400.m_EXT.MTIP_Port_Control[0]*/

    {0x1f0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[0] */
    {0x1f000478, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[1] */
    {0x1f00071c, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[2] */
    {0x1f0009c0, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[3] */
    {0x1f000c64, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[4] */
    {0x1f000f08, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[5] */
    {0x1f0011ac, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[6] */
    {0x1f001450, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[7] */
    {0x2a0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX0 */
    {0x2a2001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX0 */
    {0x2a4001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX1 */
    {0x2a6001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX1 */
    {0x2a8001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX2 */
    {0x2aa001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX2 */

    {0x1B804044, 0x0000000c, 0x0000003F},   /*m_400.m_MAC100_0.TX_IPG_LENGTH*/
    {0x1B804048, 0x00000000, 0xFFFFFFFF},   /*m_400.m_MAC100_0.CRC_MODE*/
    {0x1B804108, 0x00102013, 0xFFeFFFFF},   /*m_400.m_MAC100_0.EMAC_COMMAND_CONFIG*/
    {0x1B804120, 0x0000000a, 0xFFFFFFFF},   /*m_400.m_MAC100_0.EMAC_TX_FIFO_SECTIONS*/
    {0x1B80411c, 0x00000001, 0xFFFFFFFF},   /*m_400.m_MAC100_0.EMAC_RX_FIFO_SECTIONS*/
    {0x1B804080, 0x00000120, 0xFFFFFFFF},   /*m_400.m_MAC100_0.XIF_MODE*/
    {0x1B804114, 0x00003178, 0xFFFFFFFF},   /*m_400.m_MAC100_0.EMAC_FRM_LENGTH*/
    {0x1B8040a8, 0x00180000, 0xFFFFFFFF},   /*m_400.m_MAC100_0.BR_CONTROL*/
    {0x1B804008, 0x00000800, 0xFFFFFFFF},   /*m_400.m_MAC100_0.COMMAND_CONFIG*/

    {0x1B400080, 0x00060000, 0xFFFDFFFF},   /* m_MIF0.mif_t32_channel_mapping_register[0]*/
    {0x1B400804, 0x00000001, 0xFFFFFFFF},   /* m_MIF0.mif_t32_rx_control_register*/
    {0x1B401004, 0x00000001, 0xFFFFFFFF},   /* m_MIF0.mif_t32_tx_control_registers*/
    {0x1B401290, 0x00000278, 0xFFFFFFFF},   /* m_MIF0.mif_t32_tx_credit_register[0]*/

    {0x1F202808, 0x0000009f, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VENDOR_VL_INTVL */
    {0x1F202840, 0x00000300, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VENDOR_PCS_MODE */
    {0x1F202900, 0x00007690, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL0_0 */
    {0x1F202904, 0x00000047, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL0_1 */
    {0x1F202908, 0x0000c4f0, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL1_0 */
    {0x1F20290c, 0x000000e6, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL1_1 */
    {0x1F202910, 0x000065c5, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL2_0 */
    {0x1F202914, 0x0000009b, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL2_1 */
    {0x1F202918, 0x000079a2, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL3_0 */
    {0x1F20291c, 0x0000003d, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL3_1 */

};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC hawkMtiUsx_20G_OXSGMII_RegDb[] = {

    {0x1B800008, 0x5a4c3c00, 0xFFFFFFFF},   /* m_USX0.m_EXT.MTIP_Global_Channel_Control */
    {0x1B800014, 0x80000000, 0xFFF7FFFF},   /* m_USX0.m_EXT.Global_Reset_Control*/
    {0x1B800010, 0x00030400, 0xFFFFFFFF},   /* m_USX0.m_EXT.Global_Clock_Enable */
    {0x1B800018, 0x00000003, 0xFFFFFFFF},   /* m_USX0.m_EXT.Global_Reset_Control2 */

    {0x24800000, 0x000f0101, 0xFFFFFFFF},   /* m_USX0.m_EXT.Global_MAC_Clock_and_Reset_Control*/
    {0x24800004, 0x0003303F, 0xFFFFFFFF},   /* m_USX0.m_EXT.Global_PCS_Clock_and_Reset_Control*/
    {0x2480002c, 0x00030110, 0xFFFE03FF},   /* m_USX0.m_EXT.MTIP_Port_Control[0] */
    {0x24800050, 0x00000001, 0xFFFFFFFF},   /* m_USX0.m_EXT.MTIP_Port_Control1[0] */

    {0x2AC02008, 0x00000008, 0xFFFFFFFF},   /* m_USX0.m_USXM_0.PORTS_ENA */
    {0x2AC0200c, 0x0000009f, 0xFFFFFFFF},   /* m_USX0.m_USXM_0.VL_INTVL */

    {0x1f0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[0] */
    {0x1f000478, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[1] */
    {0x1f00071c, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[2] */
    {0x1f0009c0, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[3] */
    {0x1f000c64, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[4] */
    {0x1f000f08, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[5] */
    {0x1f0011ac, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[6] */
    {0x1f001450, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[7] */
    {0x2a0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX0 */
    {0x2a2001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX0 */
    {0x2a4001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX1 */
    {0x2a6001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX1 */
    {0x2a8001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX2 */
    {0x2aa001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX2 */

    {0x24802044, 0x0000000c, 0x0000003F},   /*m_USX0.m_MAC_0.TX_IPG_LENGTH*/
    {0x24802048, 0x00000000, 0xFFFFFFFF},   /*m_USX0.m_MAC_0.CRC_MODE*/
    {0x24802008, 0x00302813, 0xFFeFFFFF},   /*m_USX0.m_MAC_0.COMMAND_CONFIG*/
    {0x24802020, 0x00000015, 0xFFFFFFFF},   /*m_USX0.m_MAC_0.TX_FIFO_SECTIONS*/
    {0x2480201c, 0x00000001, 0xFFFFFFFF},   /*m_USX0.m_MAC_0.RX_FIFO_SECTIONS*/
    {0x24802080, 0x00000121, 0xFFFFFFFF},   /*m_USX0.m_MAC_0.XIF_MODE*/
    {0x24802014, 0x00003178, 0xFFFFFFFF},   /*m_USX0.m_MAC_0.FRM_LENGTH*/

    {0x24400000, 0x00060202, 0xFFFDFFFF},   /* m_MIF1.mif_t8_channel_mapping_register[0]*/
    {0x24400800, 0x00000001, 0xFFFFFFFF},   /* m_MIF1.mif_t8_rx_control_register*/
    {0x24401000, 0x00000001, 0xFFFFFFFF},   /* m_MIF1.mif_t8_tx_control_registers*/
    {0x24401200, 0x00000278, 0xFFFFFFFF},   /* m_MIF1.mif_t8_tx_credit_register[0]*/

    {0x2AC00058, 0x00000001, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_USXGMII_REP[0] */
    {0x2AC00000, 0x00008000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_CONTROL[0] */
    {0x2AC00048, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_LINK_TIMER_0[0] */
    {0x2AC0004c, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_LINK_TIMER_1[0] */

};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC hawkMti400_50G_R2_EXPRESS_RegDb[] = {

    {0x1B800000, 0x00000000, 0xFFFFFFFF},   /* m_400.m_EXT.MTIP_Global_PMA_Control */
    {0x1B800004, 0x00030000, 0xFFFFFFFF},   /* m_400.m_EXT.MTIP_Global_FEC_Control */
    {0x1B800008, 0x5a4c3800, 0xFFFFFFFF},   /* m_400.m_EXT.MTIP_Global_Channel_Control */
    {0x1B800014, 0xa8010303, 0xFFF7FFFF},   /* m_400.m_EXT.Global_Reset_Control*/
    {0x1B800010, 0x00031c01, 0xFFFFFFFF},   /* m_400.m_EXT.Global_Clock_Enable */
    {0x1B800018, 0x00000003, 0xFFFFFFFF},   /* m_400.m_EXT.Global_Reset_Control2 */
    {0x1B800084, 0x01418010, 0xFFFF01FF},   /* m_400.m_EXT.MTIP_Port_Control[0]*/

    {0x1f0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[0] */
    {0x1f000478, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[1] */
    {0x1f00071c, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[2] */
    {0x1f0009c0, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[3] */
    {0x1f000c64, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[4] */
    {0x1f000f08, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[5] */
    {0x1f0011ac, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[6] */
    {0x1f001450, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[7] */
    {0x2a0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX0 */
    {0x2a2001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX0 */
    {0x2a4001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX1 */
    {0x2a6001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX1 */
    {0x2a8001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX2 */
    {0x2aa001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX2 */

    {0x1B804044, 0x0000000c, 0x0000003F},   /*m_400.m_MAC100_0.TX_IPG_LENGTH*/
    {0x1B804048, 0x00000000, 0xFFFFFFFF},   /*m_400.m_MAC100_0.CRC_MODE*/
    {0x1B804108, 0x00102013, 0xFFeFFFFF},   /*m_400.m_MAC100_0.EMAC_COMMAND_CONFIG*/
    {0x1B804120, 0x0000000a, 0xFFFFFFFF},   /*m_400.m_MAC100_0.EMAC_TX_FIFO_SECTIONS*/
    {0x1B80411c, 0x00000001, 0xFFFFFFFF},   /*m_400.m_MAC100_0.EMAC_RX_FIFO_SECTIONS*/
    {0x1B804080, 0x00000120, 0xFFFFFFFF},   /*m_400.m_MAC100_0.XIF_MODE*/
    {0x1B804114, 0x00003178, 0xFFFFFFFF},   /*m_400.m_MAC100_0.EMAC_FRM_LENGTH*/
    {0x1B8040a8, 0x00180000, 0xFFFFFFFF},   /*m_400.m_MAC100_0.BR_CONTROL*/
    {0x1B804008, 0x00000803, 0xFFFFFFFF},   /*m_400.m_MAC100_0.COMMAND_CONFIG*/

    {0x1B400080, 0x00060000, 0xFFFDFFFF},   /* m_MIF0.mif_t32_channel_mapping_register[0]*/
    {0x1B400804, 0x00000001, 0xFFFFFFFF},   /* m_MIF0.mif_t32_rx_control_register*/
    {0x1B401004, 0x00000001, 0xFFFFFFFF},   /* m_MIF0.mif_t32_tx_control_registers*/
    {0x1B401290, 0x00000278, 0xFFFFFFFF},   /* m_MIF0.mif_t32_tx_credit_register[0]*/

    {0x1F202808, 0x0000009f, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VENDOR_VL_INTVL */
    {0x1F202840, 0x00000300, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VENDOR_PCS_MODE */
    {0x1F202900, 0x00007690, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL0_0 */
    {0x1F202904, 0x00000047, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL0_1 */
    {0x1F202908, 0x0000c4f0, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL1_0 */
    {0x1F20290c, 0x000000e6, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL1_1 */
    {0x1F202910, 0x000065c5, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL2_0 */
    {0x1F202914, 0x0000009b, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL2_1 */
    {0x1F202918, 0x000079a2, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL3_0 */
    {0x1F20291c, 0x0000003d, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL3_1 */

    {0x1F20a000, 0x00000200, 0xFFFFFFFF},   /* m_400.m_RSFEC400.p_RSFEC_CONTROL[0] */
    {0x1F20a020, 0x00000200, 0xFFFFFFFF},   /* m_400.m_RSFEC400.p_RSFEC_CONTROL[1] */

};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC hawkMti400_40G_R4_EXPRESS_RegDb[] = {

    {0x1B800000, 0x00000000, 0xFFFFFFFF},   /* m_400.m_EXT.MTIP_Global_PMA_Control */
    /*{0x1B800004, 0x00030000, 0xFFFFFFFF},  */ /* m_400.m_EXT.MTIP_Global_FEC_Control */
    {0x1B800008, 0x5a4c3c01, 0xFFFFFFFF},   /* m_400.m_EXT.MTIP_Global_Channel_Control */
    {0x1B800014, 0x88010f0f, 0xFFF7FFFF},   /* m_400.m_EXT.Global_Reset_Control*/
    {0x1B800010, 0x00031401, 0xFFFFFFFF},   /* m_400.m_EXT.Global_Clock_Enable */
    {0x1B800018, 0x00000003, 0xFFFFFFFF},   /* m_400.m_EXT.Global_Reset_Control2 */
    {0x1B800084, 0x01398010, 0xFFFF01FF},   /* m_400.m_EXT.MTIP_Port_Control[0]*/

    {0x1f0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[0] */
    {0x1f000478, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[1] */
    {0x1f00071c, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[2] */
    {0x1f0009c0, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[3] */
    {0x1f000c64, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[4] */
    {0x1f000f08, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[5] */
    {0x1f0011ac, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[6] */
    {0x1f001450, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[7] */
    {0x2a0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX0 */
    {0x2a2001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX0 */
    {0x2a4001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX1 */
    {0x2a6001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX1 */
    {0x2a8001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX2 */
    {0x2aa001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX2 */

    {0x1B804044, 0x0000000c, 0x0000003F},   /*m_400.m_MAC100_0.TX_IPG_LENGTH*/
    {0x1B804048, 0x00000000, 0xFFFFFFFF},   /*m_400.m_MAC100_0.CRC_MODE*/
    {0x1B804108, 0x00102013, 0xFFeFFFFF},   /*m_400.m_MAC100_0.EMAC_COMMAND_CONFIG*/
    {0x1B804120, 0x0000000a, 0xFFFFFFFF},   /*m_400.m_MAC100_0.EMAC_TX_FIFO_SECTIONS*/
    {0x1B80411c, 0x00000001, 0xFFFFFFFF},   /*m_400.m_MAC100_0.EMAC_RX_FIFO_SECTIONS*/
    {0x1B804080, 0x00000120, 0xFFFFFFFF},   /*m_400.m_MAC100_0.XIF_MODE*/
    {0x1B804114, 0x00003178, 0xFFFFFFFF},   /*m_400.m_MAC100_0.EMAC_FRM_LENGTH*/
    {0x1B8040a8, 0x01780a00, 0xFFFFFFFF},   /*m_400.m_MAC100_0.BR_CONTROL*/
    {0x1B804008, 0x00000803, 0xFFFFFFFF},   /*m_400.m_MAC100_0.COMMAND_CONFIG*/

    {0x1B400080, 0x00060000, 0xFFFDFFFF},   /* m_MIF0.mif_t32_channel_mapping_register[0]*/
    {0x1B400804, 0x00000001, 0xFFFFFFFF},   /* m_MIF0.mif_t32_rx_control_register*/
    {0x1B401004, 0x00000001, 0xFFFFFFFF},   /* m_MIF0.mif_t32_tx_control_registers*/
    {0x1B401290, 0x00000278, 0xFFFFFFFF},   /* m_MIF0.mif_t32_tx_credit_register[0]*/

    {0x1F202808, 0x0000007f, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VENDOR_VL_INTVL */
    {0x1F202840, 0x00000300, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VENDOR_PCS_MODE */
    {0x1F202900, 0x00007690, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL0_0 */
    {0x1F202904, 0x00000047, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL0_1 */
    {0x1F202908, 0x0000c4f0, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL1_0 */
    {0x1F20290c, 0x000000e6, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL1_1 */
    {0x1F202910, 0x000065c5, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL2_0 */
    {0x1F202914, 0x0000009b, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL2_1 */
    {0x1F202918, 0x000079a2, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL3_0 */
    {0x1F20291c, 0x0000003d, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL3_1 */

    /*{0x1F20a000, 0x00000200, 0xFFFFFFFF},*/   /* m_400.m_RSFEC400.p_RSFEC_CONTROL[0] */
    /*{0x1F20a020, 0x00000200, 0xFFFFFFFF},*/   /* m_400.m_RSFEC400.p_RSFEC_CONTROL[1] */
};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC hawkMtiUsx_20G_QXSGMII_RegDb[] = {

    {0x1B800008, 0x5a4c3c00, 0xFFFFFFFF},   /* m_USX0.m_EXT.MTIP_Global_Channel_Control */
    {0x1B800014, 0x80000000, 0xFFF7FFFF},   /* m_USX0.m_EXT.Global_Reset_Control*/
    {0x1B800010, 0x00030400, 0xFFFFFFFF},   /* m_USX0.m_EXT.Global_Clock_Enable */
    {0x1B800018, 0x00000003, 0xFFFFFFFF},   /* m_USX0.m_EXT.Global_Reset_Control2 */

    {0x24800000, 0x000f0101, 0xFFFFFFFF},   /* m_USX0.m_EXT.Global_MAC_Clock_and_Reset_Control*/
    {0x24800004, 0x0003303F, 0xFFFFFFFF},   /* m_USX0.m_EXT.Global_PCS_Clock_and_Reset_Control*/
    {0x2480002c, 0x00030110, 0xFFFE03FF},   /* m_USX0.m_EXT.MTIP_Port_Control[0] */
    {0x24800050, 0x00000001, 0xFFFFFFFF},   /* m_USX0.m_EXT.MTIP_Port_Control1[0] */

    {0x2AC02008, 0x00000004, 0xFFFFFFFF},   /* m_USX0.m_USXM_0.PORTS_ENA */
    {0x2AC0200c, 0x0000009f, 0xFFFFFFFF},   /* m_USX0.m_USXM_0.VL_INTVL */

    {0x1f0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[0] */
    {0x1f000478, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[1] */
    {0x1f00071c, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[2] */
    {0x1f0009c0, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[3] */
    {0x1f000c64, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[4] */
    {0x1f000f08, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[5] */
    {0x1f0011ac, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[6] */
    {0x1f001450, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[7] */
    {0x2a0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX0 */
    {0x2a2001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX0 */
    {0x2a4001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX1 */
    {0x2a6001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX1 */
    {0x2a8001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX2 */
    {0x2aa001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX2 */

    {0x24802044, 0x0000000c, 0x0000003F},   /*m_USX0.m_MAC_0.TX_IPG_LENGTH*/
    {0x24802048, 0x00000000, 0xFFFFFFFF},   /*m_USX0.m_MAC_0.CRC_MODE*/
    {0x24802008, 0x00302813, 0xFFeFFFFF},   /*m_USX0.m_MAC_0.COMMAND_CONFIG*/
    {0x24802020, 0x00000015, 0xFFFFFFFF},   /*m_USX0.m_MAC_0.TX_FIFO_SECTIONS*/
    {0x2480201c, 0x00000001, 0xFFFFFFFF},   /*m_USX0.m_MAC_0.RX_FIFO_SECTIONS*/
    {0x24802080, 0x00000121, 0xFFFFFFFF},   /*m_USX0.m_MAC_0.XIF_MODE*/
    {0x24802014, 0x00003178, 0xFFFFFFFF},   /*m_USX0.m_MAC_0.FRM_LENGTH*/

    {0x24400000, 0x00060202, 0xFFFDFFFF},   /* m_MIF1.mif_t8_channel_mapping_register[0]*/
    {0x24400800, 0x00000001, 0xFFFFFFFF},   /* m_MIF1.mif_t8_rx_control_register*/
    {0x24401000, 0x00000001, 0xFFFFFFFF},   /* m_MIF1.mif_t8_tx_control_registers*/
    {0x24401200, 0x00000278, 0xFFFFFFFF},   /* m_MIF1.mif_t8_tx_credit_register[0]*/

    {0x2AC00058, 0x00000001, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_USXGMII_REP[0] */
    {0x2AC00000, 0x00008000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_CONTROL[0] */
    {0x2AC00048, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_LINK_TIMER_0[0] */
    {0x2AC0004c, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_LINK_TIMER_1[0] */

};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC hawkMtiUsx_10G_OUSGMII_RegDb[] = {

    {0x1B800008, 0x5a4c3c00, 0xFFFFFFFF},   /* m_USX0.m_EXT.MTIP_Global_Channel_Control */
    {0x1B800014, 0x80000000, 0xFFF7FFFF},   /* m_USX0.m_EXT.Global_Reset_Control*/
    {0x1B800010, 0x00030400, 0xFFFFFFFF},   /* m_USX0.m_EXT.Global_Clock_Enable */
    {0x1B800018, 0x00000003, 0xFFFFFFFF},   /* m_USX0.m_EXT.Global_Reset_Control2 */

    {0x24800000, 0x000f0101, 0xFFFFFFFF},   /* m_USX0.m_EXT.Global_MAC_Clock_and_Reset_Control*/
    {0x24800004, 0x0003c009, 0xFFFFFFFF},   /* m_USX0.m_EXT.Global_PCS_Clock_and_Reset_Control*/
    {0x2480002c, 0x00030110, 0xFFFE03FF},   /* m_USX0.m_EXT.MTIP_Port_Control[0] */
    {0x24800050, 0x00000001, 0xFFFFFFFF},   /* m_USX0.m_EXT.MTIP_Port_Control1[0] */

    {0x2AC02008, 0x00000008, 0xFFFFFFFF},   /* m_USX0.m_USXM_0.PORTS_ENA */

    {0x1f0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[0] */
    {0x1f000478, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[1] */
    {0x1f00071c, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[2] */
    {0x1f0009c0, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[3] */
    {0x1f000c64, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[4] */
    {0x1f000f08, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[5] */
    {0x1f0011ac, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[6] */
    {0x1f001450, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[7] */
    {0x2a0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX0 */
    {0x2a2001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX0 */
    {0x2a4001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX1 */
    {0x2a6001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX1 */
    {0x2a8001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX2 */
    {0x2aa001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX2 */

    {0x24802044, 0x0000000c, 0x0000003F},   /*m_USX0.m_MAC_0.TX_IPG_LENGTH*/
    {0x24802048, 0x00000000, 0xFFFFFFFF},   /*m_USX0.m_MAC_0.CRC_MODE*/
    {0x24802008, 0x00302813, 0xFFeFFFFF},   /*m_USX0.m_MAC_0.COMMAND_CONFIG*/
    {0x24802020, 0x00000015, 0xFFFFFFFF},   /*m_USX0.m_MAC_0.TX_FIFO_SECTIONS*/
    {0x2480201c, 0x00000001, 0xFFFFFFFF},   /*m_USX0.m_MAC_0.RX_FIFO_SECTIONS*/
    {0x24802080, 0x00000131, 0xFFFFFFFF},   /*m_USX0.m_MAC_0.XIF_MODE*/
    {0x24802014, 0x00003178, 0xFFFFFFFF},   /*m_USX0.m_MAC_0.FRM_LENGTH*/

    {0x24400000, 0x00040202, 0xFFFDFFFF},   /* m_MIF1.mif_t8_channel_mapping_register[0]*/
    {0x24400800, 0x00000001, 0xFFFFFFFF},   /* m_MIF1.mif_t8_rx_control_register*/
    {0x24401000, 0x00000001, 0xFFFFFFFF},   /* m_MIF1.mif_t8_tx_control_registers*/
    {0x24401200, 0x00000278, 0xFFFFFFFF},   /* m_MIF1.mif_t8_tx_credit_register[0]*/

    {0x2AC00058, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_USXGMII_REP[0] */
    {0x2AC00050, 0x00000109, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.n_IF_MODE[0] */
    {0x2AC00000, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_CONTROL[0] */
    {0x2AC00048, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_LINK_TIMER_0[0] */
    {0x2AC0004c, 0x00000000, 0xFFFFFFFF},   /* m_USX0.m_LPCS.lpcs_units_RegFile.p_LINK_TIMER_1[0] */

};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC hawkMti400_1000BaseX_EXPRESS_RegDb[] = {

    {0x1B800000, 0x00000001, 0xFFFFFFFF},   /* m_400.m_EXT.MTIP_Global_PMA_Control */
    {0x1B800004, 0x00000000, 0xFFFFFFFF},   /* m_400.m_EXT.MTIP_Global_FEC_Control */
    {0x1B800008, 0x5a4c3c00, 0xFFFFFFFF},   /* m_400.m_EXT.MTIP_Global_Channel_Control */
    {0x1B800014, 0xc8010101, 0xFFF7FFFF},   /* m_400.m_EXT.Global_Reset_Control*/
    {0x1B800010, 0x00039401, 0xFFFFFFFF},   /* m_400.m_EXT.Global_Clock_Enable */
    {0x1B800018, 0x00000003, 0xFFFFFFFF},   /* m_400.m_EXT.Global_Reset_Control2 */
    {0x1B800084, 0x01198010, 0xFFFF01FF},   /* m_400.m_EXT.MTIP_Port_Control[0]*/

    {0x1f0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[0] */
    {0x1f000478, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[1] */
    {0x1f00071c, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[2] */
    {0x1f0009c0, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[3] */
    {0x1f000c64, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[4] */
    {0x1f000f08, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[5] */
    {0x1f0011ac, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[6] */
    {0x1f001450, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[7] */
    {0x2a0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX0 */
    {0x2a2001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX0 */
    {0x2a4001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX1 */
    {0x2a6001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX1 */
    {0x2a8001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX2 */
    {0x2aa001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX2 */

    {0x1B804044, 0x0000000c, 0x0000003F},   /*m_400.m_MAC100_0.TX_IPG_LENGTH*/
    {0x1B804048, 0x00000000, 0xFFFFFFFF},   /*m_400.m_MAC100_0.CRC_MODE*/
    {0x1B804108, 0x00102013, 0xFFeFFFFF},   /*m_400.m_MAC100_0.EMAC_COMMAND_CONFIG*/
    {0x1B804120, 0x0000000a, 0xFFFFFFFF},   /*m_400.m_MAC100_0.EMAC_TX_FIFO_SECTIONS*/
    {0x1B80411c, 0x00000001, 0xFFFFFFFF},   /*m_400.m_MAC100_0.EMAC_RX_FIFO_SECTIONS*/
    {0x1B804080, 0x00000130, 0xFFFFFFFF},   /*m_400.m_MAC100_0.XIF_MODE*/
    {0x1B804114, 0x00003178, 0xFFFFFFFF},   /*m_400.m_MAC100_0.EMAC_FRM_LENGTH*/
    {0x1B8040a8, 0x01780a00, 0xFFFFFFFF},   /*m_400.m_MAC100_0.BR_CONTROL*/
    {0x1B804008, 0x00000800, 0xFFFFFFFF},   /*m_400.m_MAC100_0.COMMAND_CONFIG*/

    {0x1B400080, 0x00060000, 0xFFFDFFFF},   /* m_MIF0.mif_t32_channel_mapping_register[0]*/
    {0x1B400804, 0x00000001, 0xFFFFFFFF},   /* m_MIF0.mif_t32_rx_control_register*/
    {0x1B401004, 0x00000001, 0xFFFFFFFF},   /* m_MIF0.mif_t32_tx_control_registers*/
    {0x1B401290, 0x00000278, 0xFFFFFFFF},   /* m_MIF0.mif_t32_tx_credit_register[0]*/

    {0x1F20b3e0, 0x00000001, 0xFFFFFFFF},   /* m_400.m_LPCS.GMODE */
    {0x1F20b050, 0x00000108, 0xFFFFFFFF},   /* m_400.m_LPCS.p_IF_MODE[0] */
    {0x1F20b000, 0x00000140, 0xFFFFFFFF},   /* m_400.m_LPCS.p_CONTROL[0] */

#if 0
    {0x1F202808, 0x0000009f, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VENDOR_VL_INTVL */
    {0x1F202840, 0x00000300, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VENDOR_PCS_MODE */
    {0x1F202900, 0x00007690, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL0_0 */
    {0x1F202904, 0x00000047, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL0_1 */
    {0x1F202908, 0x0000c4f0, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL1_0 */
    {0x1F20290c, 0x000000e6, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL1_1 */
    {0x1F202910, 0x000065c5, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL2_0 */
    {0x1F202914, 0x0000009b, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL2_1 */
    {0x1F202918, 0x000079a2, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL3_0 */
    {0x1F20291c, 0x0000003d, 0xFFFFFFFF},   /* m_400.m_PCS100_0.VL3_1 */
#endif

};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC hawkMti400_200G_R4_RegDb[] = {

    {0x1B800000, 0x00000f00, 0xFFFFFFFF},   /* m_400.m_EXT.MTIP_Global_PMA_Control */
    {0x1B800004, 0x0f0f0000, 0xFFFFFFFF},   /* m_400.m_EXT.MTIP_Global_FEC_Control */
    {0x1B800008, 0x5a4c3c40, 0xFFFFFFFF},   /* m_400.m_EXT.MTIP_Global_Channel_Control */
    {0x1B800014, 0xa5000f0f, 0xFFF7FFFF},   /* m_400.m_EXT.Global_Reset_Control*/
    {0x1B800010, 0x00034d00, 0xFFFFFFFF},   /* m_400.m_EXT.Global_Clock_Enable */
    {0x1B800018, 0x00000003, 0xFFFFFFFF},   /* m_400.m_EXT.Global_Reset_Control2 */
    {0x1B800084, 0x01018010, 0xFFFF01FF},   /* m_400.m_EXT.MTIP_Port_Control[0]*/
    {0x1B80005c, 0x0565f840, 0xFFFF01FF},   /* m_400.m_EXT.MTIP_Seg_Port_Control[0]*/

    {0x1f0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[0] */
    {0x1f000478, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[1] */
    {0x1f00071c, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[2] */
    {0x1f0009c0, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[3] */
    {0x1f000c64, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[4] */
    {0x1f000f08, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[5] */
    {0x1f0011ac, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[6] */
    {0x1f001450, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[7] */
    {0x2a0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX0 */
    {0x2a2001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX0 */
    {0x2a4001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX1 */
    {0x2a6001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX1 */
    {0x2a8001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX2 */
    {0x2aa001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX2 */

    {0x1B802080, 0x00000020, 0xFFFFFFFF},   /*m_400.m_MAC400_0.XIF_MODE*/
    {0x1B802008, 0x00102813, 0xFFFFFFFF},   /*m_400.m_MAC400_0.COMMAND_CONFIG*/
    {0x1B802044, 0x00000000, 0x0000003F},   /*m_400.m_MAC400_0.TX_IPG_LENGTH*/
    {0x1B802020, 0x0000000a, 0x0000003F},   /*m_400.m_MAC400_0.TX_FIFO_SECTIONS*/
    {0x1B80201c, 0x00000001, 0x0000003F},   /*m_400.m_MAC400_0.RX_FIFO_SECTIONS*/

    {0x1B400100, 0x00040000, 0xFFFDFFFF},   /* m_MIF0.mif_t128_channel_mapping_register[0]*/
    {0x1B400808, 0x00000001, 0xFFFFFFFF},   /* m_MIF0.mif_t128_rx_control_register*/
    {0x1B401008, 0x00000001, 0xFFFFFFFF},   /* m_MIF0.mif_t128_tx_control_registers*/
    {0x1B4012d0, 0x00000278, 0xFFFFFFFF},   /* m_MIF0.mif_t128_tx_credit_register[0]*/

    {0x1F20080c, 0x00000009, 0xFFFFFFFF},   /* m_400.m_PCS400.VENDOR_TX_LANE_THRESH */
    {0x1F200808, 0x00000010, 0xFFFFFFFF},   /* m_400.m_PCS400.VENDOR_VL_INTVL */
    {0x1F200820, 0x00004a9a, 0xFFFFFFFF},   /* m_400.m_PCS400.VENDOR_AM_0 */
    {0x1F200824, 0x00000026, 0xFFFFFFFF},   /* m_400.m_PCS400.VENDOR_AM_1 */
    {0x1F200000, 0x00008004, 0xFFFFFFFF},   /* m_400.m_PCS400.CONTROL1 */
    {0x1F20081c, 0x00000008, 0xFFFFFFFF},   /* m_400.m_PCS400.VENDOR_TX_CDMII_PACE */

    {0x1F20a180, 0x0000c0b3, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL0_0 */
    {0x1F20a184, 0x0000008c, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL0_1 */
    {0x1F20a188, 0x0000de5a, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL1_0 */
    {0x1F20a18c, 0x0000007e, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL1_1 */
    {0x1F20a190, 0x0000f33e, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL2_0 */
    {0x1F20a194, 0x00000056, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL2_1 */
    {0x1F20a198, 0x00008086, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL3_0 */
    {0x1F20a19c, 0x000000d0, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL3_1 */
    {0x1F20a1a0, 0x0000512a, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL4_0 */
    {0x1F20a1a4, 0x000000f2, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL4_1 */
    {0x1F20a1a8, 0x00004f12, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL5_0 */
    {0x1F20a1ac, 0x000000d1, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL5_1 */
    {0x1F20a1b0, 0x00009c42, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL6_0 */
    {0x1F20a1b4, 0x000000a1, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL6_1 */
    {0x1F20a1b8, 0x000076d6, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL7_0 */
    {0x1F20a1bc, 0x0000005b, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL7_1 */
};

static const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC hawkMti400_200G_R8_RegDb[] = {

    {0x1B800000, 0x00000000, 0xFFFFFFFF},   /* m_400.m_EXT.MTIP_Global_PMA_Control */
    {0x1B800004, 0xffff0000, 0xFFFFFFFF},   /* m_400.m_EXT.MTIP_Global_FEC_Control */
    {0x1B800008, 0x5a4c3cc0, 0xFFFFFFFF},   /* m_400.m_EXT.MTIP_Global_Channel_Control */
    {0x1B800014, 0xa500ffff, 0xFFF7FFFF},   /* m_400.m_EXT.Global_Reset_Control*/
    {0x1B800010, 0x00034d00, 0xFFFFFFFF},   /* m_400.m_EXT.Global_Clock_Enable */
    {0x1B800018, 0x00000003, 0xFFFFFFFF},   /* m_400.m_EXT.Global_Reset_Control2 */
    {0x1B800084, 0x01018010, 0xFFFF01FF},   /* m_400.m_EXT.MTIP_Port_Control[0]*/
    {0x1B80005c, 0x0545f840, 0xFFFF01FF},   /* m_400.m_EXT.MTIP_Seg_Port_Control[0]*/

    {0x1f0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[0] */
    {0x1f000478, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[1] */
    {0x1f00071c, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[2] */
    {0x1f0009c0, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[3] */
    {0x1f000c64, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[4] */
    {0x1f000f08, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[5] */
    {0x1f0011ac, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[6] */
    {0x1f001450, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP_400.m_RAL_ANP.Port_Control6[7] */
    {0x2a0001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX0 */
    {0x2a2001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX0 */
    {0x2a4001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX1 */
    {0x2a6001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX1 */
    {0x2a8001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP0_USX2 */
    {0x2aa001d4, 0x0f0000ff, 0xFFFFFFFF},   /* m_ANP1_USX2 */

    {0x1B802080, 0x00000020, 0xFFFFFFFF},   /*m_400.m_MAC400_0.XIF_MODE*/
    {0x1B802008, 0x00102813, 0xFFFFFFFF},   /*m_400.m_MAC400_0.COMMAND_CONFIG*/
    {0x1B802044, 0x00000000, 0x0000003F},   /*m_400.m_MAC400_0.TX_IPG_LENGTH*/
    {0x1B802020, 0x0000000a, 0x0000003F},   /*m_400.m_MAC400_0.TX_FIFO_SECTIONS*/
    {0x1B80201c, 0x00000001, 0x0000003F},   /*m_400.m_MAC400_0.RX_FIFO_SECTIONS*/

    {0x1B400100, 0x00040000, 0xFFFDFFFF},   /* m_MIF0.mif_t128_channel_mapping_register[0]*/
    {0x1B400808, 0x00000001, 0xFFFFFFFF},   /* m_MIF0.mif_t128_rx_control_register*/
    {0x1B401008, 0x00000001, 0xFFFFFFFF},   /* m_MIF0.mif_t128_tx_control_registers*/
    {0x1B4012d0, 0x00000278, 0xFFFFFFFF},   /* m_MIF0.mif_t128_tx_credit_register[0]*/

    {0x1F20080c, 0x00000009, 0xFFFFFFFF},   /* m_400.m_PCS400.VENDOR_TX_LANE_THRESH */
    {0x1F200808, 0x00000010, 0xFFFFFFFF},   /* m_400.m_PCS400.VENDOR_VL_INTVL */
    {0x1F200820, 0x00004a9a, 0xFFFFFFFF},   /* m_400.m_PCS400.VENDOR_AM_0 */
    {0x1F200824, 0x00000026, 0xFFFFFFFF},   /* m_400.m_PCS400.VENDOR_AM_1 */
    {0x1F200000, 0x00000004, 0xFFFFFFFF},   /* m_400.m_PCS400.CONTROL1 */
    {0x1F20081c, 0x00000008, 0xFFFFFFFF},   /* m_400.m_PCS400.VENDOR_TX_CDMII_PACE */

    {0x1F20a180, 0x0000c0b3, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL0_0 */
    {0x1F20a184, 0x0000008c, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL0_1 */
    {0x1F20a188, 0x0000de5a, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL1_0 */
    {0x1F20a18c, 0x0000007e, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL1_1 */
    {0x1F20a190, 0x0000f33e, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL2_0 */
    {0x1F20a194, 0x00000056, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL2_1 */
    {0x1F20a198, 0x00008086, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL3_0 */
    {0x1F20a19c, 0x000000d0, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL3_1 */
    {0x1F20a1a0, 0x0000512a, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL4_0 */
    {0x1F20a1a4, 0x000000f2, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL4_1 */
    {0x1F20a1a8, 0x00004f12, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL5_0 */
    {0x1F20a1ac, 0x000000d1, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL5_1 */
    {0x1F20a1b0, 0x00009c42, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL6_0 */
    {0x1F20a1b4, 0x000000a1, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL6_1 */
    {0x1F20a1b8, 0x000076d6, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL7_0 */
    {0x1F20a1bc, 0x0000005b, 0xFFFFFFFF},   /* m_400.m_RSFEC400.VENDOR_VL7_1 */
};

GT_STATUS mvHwsHawkPunktPortModeCheck(GT_U32 portType)
{
    GT_U32 i;
    GT_U32 iterNum;
    GT_U32 regData;
    const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC *regPtr;

    if(portType == 0)
    {
        hwsOsPrintf(" 1 - MTI_400 - 50G_R1_EXPRESS \n");
        hwsOsPrintf(" 2 - MTI_400 - 50G_R1_PREEMPT \n");
        hwsOsPrintf(" 3 - MTI_USX - 20G_OXGMII     \n");
        hwsOsPrintf(" 4 - MTI_400 - 50G_R2_EXPRESS \n");
        hwsOsPrintf(" 5 - MTI_400 - 40G_R4_EXPRESS \n");
        hwsOsPrintf(" 6 - MTI_USX - 20G_QXGMII     \n");
        hwsOsPrintf(" 7 - MTI_USX - 10G_OUSGMII     \n");
        hwsOsPrintf(" 8 - MTI_400 - 1000BaseX_EXPRESS \n");
        hwsOsPrintf(" 9 - MTI_400 - 200G_R4    \n");
        hwsOsPrintf(" 10- MTI_400 - 200G_R8    \n");
        hwsOsPrintf(" 11- MTI_    \n");
        hwsOsPrintf(" 12- MTI_    \n");
        hwsOsPrintf(" 13- MTI_    \n");
        hwsOsPrintf("\n");
        return GT_OK;
    }
    else if(portType == 1) /* MTI_400 - 50G_R1_EXPRESS */
    {
        iterNum = sizeof(hawkMti400_50G_R1_EXPRESS_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = hawkMti400_50G_R1_EXPRESS_RegDb;
     }
    else if(portType == 2) /* MTI_400 - 50G_R1_PREEMPT */
    {
#if 0
        iterNum = sizeof(phoenixMtiUsx_25G_R1_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = phoenixMtiUsx_25G_R1_RegDb;
#endif
        return GT_NOT_IMPLEMENTED;
    }
    else if(portType == 3) /* USX 20G_OXGMII port*/
    {
        iterNum = sizeof(hawkMtiUsx_20G_OXSGMII_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = hawkMtiUsx_20G_OXSGMII_RegDb;
    }
    else if(portType == 4) /* MTI_400 - 50G_R2_EXPRESS */
    {
        iterNum = sizeof(hawkMti400_50G_R2_EXPRESS_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = hawkMti400_50G_R2_EXPRESS_RegDb;
    }
    else if(portType == 5) /* MTI_400 40G_R4 port*/
    {
        iterNum = sizeof(hawkMti400_40G_R4_EXPRESS_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = hawkMti400_40G_R4_EXPRESS_RegDb;
    }
    else if(portType == 6) /* USX 20G_QXGMII port */
    {
        iterNum = sizeof(hawkMtiUsx_20G_QXSGMII_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = hawkMtiUsx_20G_QXSGMII_RegDb;
    }
    else if(portType == 7) /* USX 10G_OUSGMII */
    {
        iterNum = sizeof(hawkMtiUsx_10G_OUSGMII_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = hawkMtiUsx_10G_OUSGMII_RegDb;
    }
    else if(portType == 8) /* MTI400 1000BaseX port*/
    {
        iterNum = sizeof(hawkMti400_1000BaseX_EXPRESS_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = hawkMti400_1000BaseX_EXPRESS_RegDb;
    }
    else if(portType == 9) /* MTI400 - 200G_R4 */
    {
        iterNum = sizeof(hawkMti400_200G_R4_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = hawkMti400_200G_R4_RegDb;
    }
    else if(portType == 10) /* MTI400 - 200G_R8 */
    {
        iterNum = sizeof(hawkMti400_200G_R8_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = hawkMti400_200G_R8_RegDb;
    }
    else if(portType == 11) /* MTI100 10G_R1*/
    {
#if 0
        iterNum = sizeof(phoenixMti100_10G_R1_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = phoenixMti100_10G_R1_RegDb;
#endif
        return GT_NOT_IMPLEMENTED;
    }
    else if(portType == 12) /* MTI USX 10G OUSGMII */
    {
#if 0
        iterNum = sizeof(phoenixMtiUsx_10G_OUSGMII_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = phoenixMtiUsx_10G_OUSGMII_RegDb;
#endif
        return GT_NOT_IMPLEMENTED;
    }
    else if(portType == 13) /* USX 10G port*/
    {
#if 0
        iterNum = sizeof(phoenixMtiUsx_10G_R1_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = phoenixMtiUsx_10G_R1_RegDb;
#endif
        return GT_NOT_IMPLEMENTED;
    }
    else
    {
        hwsOsPrintf("ILLEGAL PORT TYPE\n");
        return GT_FAIL;
    }
    hwsOsPrintf("iterNum = %d\n\n", iterNum);

    for(i = 0; i < iterNum; i++)
    {
        CHECK_STATUS(hwsRegisterGetFuncPtr(0, 0, regPtr[i].regAddr, &regData, 0xFFFFFFFF));
        if ((regData & regPtr[i].mask) != (regPtr[i].expectedData & regPtr[i].mask))
        {
            hwsOsPrintf("Found mismatch: iter = %d, regAddr = 0x%x, data = 0x%x, expectedData = 0x%x\n", i, regPtr[i].regAddr, (regData& regPtr[i].mask), (regPtr[i].expectedData& regPtr[i].mask));
        }
    }

    return GT_OK;
}

GT_STATUS mvHwsHawkPunktUnitBaseGet
(
    GT_U32                  macPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    GT_STATUS rc;
    MV_HWS_UNITS_ID unitId;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32          unitAddr = 0;
    GT_U32          localUnitNum = 0;
    GT_U32          unitIndex = 0;

    switch (portMode)
    {
        case _50GBase_KR:
        case _10GBase_KR:
        case _25GBase_KR:
        case _100GBase_KR4:
        case _107GBase_KR4:
        case _100GBase_KR2:
        case _102GBase_KR2:
        case _106GBase_KR2:
            rc = mvHwsGlobalMacToLocalIndexConvert(0, macPortNum, _10GBase_KR /*NON SEG PORT MODE*/, &convertIdx);
            if(rc != GT_OK)
            {
                return rc;
            }
            unitId = MIF_400_UNIT;
            break;

        case _200GBase_KR4:
        case _212GBase_KR4:
        case _400GBase_KR8:
        case _424GBase_KR8:
            rc = mvHwsGlobalMacToLocalIndexConvert(0, macPortNum, _400GBase_KR8 /*SEG PORT MODE*/, &convertIdx);
            if(rc != GT_OK)
            {
                return rc;
            }
            unitId = MIF_400_SEG_UNIT;
            break;

        case _2_5G_SXGMII:
        case _5G_SXGMII:
        case _10G_SXGMII:
        case _5G_DXGMII:
        case _10G_DXGMII:
        case _20G_DXGMII:
        case _5G_QUSGMII:
        case _10G_QXGMII:
        case _20G_QXGMII:
        case _10G_OUSGMII:
        case _20G_OXGMII:
            rc = mvHwsGlobalMacToLocalIndexConvert(0, macPortNum, _10G_OUSGMII /*USX PORT MODE*/, &convertIdx);
            if(rc != GT_OK)
            {
                return rc;
            }
            unitId = MIF_USX_UNIT;
            break;

        default:
            return GT_NOT_SUPPORTED;
    }

    rc = mvUnitExtInfoGet(0, unitId, macPortNum, &unitAddr, &unitIndex, &localUnitNum);
    if((unitAddr == 0) || (rc != GT_OK))
    {
        return MV_HWS_SW_PTR_ENTRY_UNUSED;
    }

    hwsOsPrintf("MIF UNIT base addr for MAC_PORT_%d is 0x%x\n", macPortNum, unitAddr);

    if (mvHwsUsxModeCheck(0, macPortNum, portMode))
    {
        if (HWS_USX_O_MODE_CHECK(portMode))
        {
            unitId = ANP_USX_O_UNIT;
        }
        else
        {
            unitId = ANP_USX_UNIT;
        }
    }
    else
    {
        unitId = ANP_400_UNIT;
    }

    rc = mvUnitExtInfoGet(0, unitId, macPortNum, &unitAddr, &unitIndex, &localUnitNum);
    if((unitAddr == 0) || (rc != GT_OK) || (unitAddr == MV_HWS_SW_PTR_ENTRY_UNUSED))
    {
        return GT_FAIL;
    }

    hwsOsPrintf("ANP UNIT base addr for MAC_PORT_%d is 0x%x\n", macPortNum, unitAddr);

    return GT_OK;
}



GT_STATUS mvHwsShowMifCounters
(
    GT_U8           devNum,
    GT_U32          portNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    GT_STATUS   rc;
    MV_HWS_UNITS_ID unitId;
    MV_HWS_MIF_REGISTERS     mifFieldName;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32 regValue;

    if (!HWS_DEV_SIP_6_10_CHECK_MAC(devNum))
    {
        return GT_NOT_SUPPORTED;
    }

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    if (rc != GT_OK)
    {
        return GT_BAD_PARAM;
    }

    if ((mvHwsUsxModeCheck(devNum, portNum, portMode)) || (mvHwsMtipIsReducedPort(devNum, portNum)))
    {
        if (mvHwsMtipIsReducedPort(devNum, portNum))
        {
            unitId = MIF_CPU_UNIT;
        }
        else
        {
            unitId = MIF_USX_UNIT;
        }

        hwsOsPrintf("\n    MIF counters for MAC_PORT_%d:\n", portNum);
        hwsOsPrintf("RX_T8:\n");

        mifFieldName = MIF_RX_REGISTER_MIF_T8_RX_GOOD_PACKETS_COUNT0_T8_RX_GOOD_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Good: 0x%x   ", regValue);
        mifFieldName = MIF_RX_REGISTER_MIF_T8_RX_BAD_PACKETS_COUNT0_T8_RX_BAD_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Bad: 0x%x   ", regValue);
        mifFieldName = MIF_RX_REGISTER_MIF_T8_RX_DISCARDED_PACKETS_COUNT0_T8_RX_DISCARDED_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Discarded: 0x%x   \n", regValue);

        hwsOsPrintf("\nTX_T8:\n");
        mifFieldName = MIF_TX_REGISTER_MIF_T8_TX_GOOD_PACKETS_COUNT0_T8_TX_GOOD_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Good: 0x%x   ", regValue);
        mifFieldName = MIF_TX_REGISTER_MIF_T8_TX_BAD_PACKETS_COUNT0_T8_TX_BAD_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Bad: 0x%x   ", regValue);
        mifFieldName = MIF_TX_REGISTER_MIF_T8_TX_DISCARDED_PACKETS_COUNT0_T8_TX_DISCARDED_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Discarded: 0x%x   \n\n", regValue);
    }
    else if (HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        unitId = MIF_400_SEG_UNIT;

        hwsOsPrintf("\n    MIF counters for MAC_PORT_%d:\n", portNum);
        hwsOsPrintf("RX_T128:\n");
        mifFieldName = MIF_RX_REGISTER_MIF_T128_RX_GOOD_PACKETS_COUNT0_T128_RX_GOOD_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Good: 0x%x   ", regValue);
        mifFieldName = MIF_RX_REGISTER_MIF_T128_RX_BAD_PACKETS_COUNT0_T128_RX_BAD_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Bad: 0x%x   ", regValue);
        mifFieldName = MIF_RX_REGISTER_MIF_T128_RX_DISCARDED_PACKETS_COUNT0_T128_RX_DISCARDED_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Discarded: 0x%x   \n", regValue);

        hwsOsPrintf("\nTX_T128:\n");
        mifFieldName = MIF_TX_REGISTER_MIF_T128_TX_GOOD_PACKETS_COUNT0_T128_TX_GOOD_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Good: 0x%x   ", regValue);
        mifFieldName = MIF_TX_REGISTER_MIF_T128_TX_BAD_PACKETS_COUNT0_T128_TX_BAD_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Bad: 0x%x   ", regValue);
        mifFieldName = MIF_TX_REGISTER_MIF_T128_TX_DISCARDED_PACKETS_COUNT0_T128_TX_DISCARDED_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Discarded: 0x%x   \n\n", regValue);
    }
    else
    {
        hwsOsPrintf("\n    MIF counters for MAC_PORT_%d:\n", portNum);
        hwsOsPrintf("RX_T32:\n");

        unitId = MIF_400_UNIT;
        mifFieldName = MIF_RX_REGISTER_MIF_T32_RX_GOOD_PACKETS_COUNT0_T32_RX_GOOD_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Good: 0x%x   ", regValue);
        mifFieldName = MIF_RX_REGISTER_MIF_T32_RX_BAD_PACKETS_COUNT0_T32_RX_BAD_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Bad: 0x%x   ", regValue);
        mifFieldName = MIF_RX_REGISTER_MIF_T32_RX_DISCARDED_PACKETS_COUNT0_T32_RX_DISCARDED_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Discarded: 0x%x   \n", regValue);

        hwsOsPrintf("\nTX_T32:\n");
        mifFieldName = MIF_TX_REGISTER_MIF_T32_TX_GOOD_PACKETS_COUNT0_T32_TX_GOOD_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Good: 0x%x   ", regValue);
        mifFieldName = MIF_TX_REGISTER_MIF_T32_TX_BAD_PACKETS_COUNT0_T32_TX_BAD_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Bad: 0x%x   ", regValue);
        mifFieldName = MIF_TX_REGISTER_MIF_T32_TX_DISCARDED_PACKETS_COUNT0_T32_TX_DISCARDED_PACKET_CNT_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, mifFieldName, &regValue, NULL));
        hwsOsPrintf("Discarded: 0x%x   \n\n", regValue);
    }


    return GT_OK;
}

GT_STATUS mvHwsMacBrCountModeGet
(
    GT_U8                           devNum,
    GT_U8                           portGroup,
    GT_U32                          portNum,
    MV_HWS_PORT_MAC_TYPE            macType,
    MV_HWS_PM_MAC_PREEMPTION_TYPE_ENT  *countMode
)
{
    GT_STATUS rc = GT_OK;
    GT_UREG_DATA regVal;

    if ( AC5P != HWS_DEV_SILICON_TYPE(devNum) || MTI_MAC_100_BR != macType)
    {
        *countMode = MV_HWS_PM_MAC_PREEMPTION_DISABLED_E;
        return GT_OK;
    }

    /* chcek if tx preemption is enabled */
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, portGroup, portNum, MTI_MAC100_UNIT, MAC100_BR_UNITS_BR_CONTROL_P0_TX_PREEMPT_EN_E, &regVal, NULL));
    if ( 0x0 == regVal )
    {
        *countMode = MV_HWS_PM_MAC_PREEMPTION_DISABLED_E;
    }
    else    /* check if separated or aggregated */
    {
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, portGroup, portNum, MTI_MAC100_UNIT, MAC100_BR_UNITS_BR_CONTROL_P0_PMAC_TO_EMAC_STATS_E, &regVal, NULL));
        *countMode = ( 0x1 == regVal ) ? MV_HWS_PM_MAC_PREEMPTION_ENABLED_AGGREGATED_E : MV_HWS_PM_MAC_PREEMPTION_ENABLED_SEPARATED_E;
    }

    return rc;
}

GT_STATUS mvHwsMacBrCountModeSet
(
    GT_U8                           devNum,
    GT_U8                           portGroup,
    GT_U32                          portNum,
    MV_HWS_PORT_MAC_TYPE            macType,
    MV_HWS_PM_MAC_PREEMPTION_TYPE_ENT  countMode
)
{
    GT_STATUS    rc = GT_OK;
    GT_UREG_DATA regVal;

    if ( AC5P != HWS_DEV_SILICON_TYPE(devNum) || MTI_MAC_100_BR != macType )
    {
        return GT_NOT_SUPPORTED;
    }

    /* chcek if tx preemption is enabled */
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, portGroup, portNum, MTI_MAC100_UNIT, MAC100_BR_UNITS_BR_CONTROL_P0_TX_PREEMPT_EN_E, &regVal, NULL));

    if ( 0x1 != regVal )    /* mac preemption is disabled */
    {
        return GT_NOT_INITIALIZED;
    }

    regVal = (MV_HWS_PM_MAC_PREEMPTION_ENABLED_AGGREGATED_E == countMode) ? 0x1 : 0x0;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, portNum, MTI_MAC100_UNIT, MAC100_BR_UNITS_BR_CONTROL_P0_PMAC_TO_EMAC_STATS_E, regVal, NULL));

    return rc;
}


#endif

