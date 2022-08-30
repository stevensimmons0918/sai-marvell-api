/********************************************************************************
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
* @file prvCpssCommonPortIfModeCfg.c
*
* @brief CPSS implementation for Port interface mode configuration.
*
*
* @version   116
********************************************************************************
*/
/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* check if a macNum can be USX port */
GT_U32 isSupportUsxPort(
    IN  GT_U8   devNum,
    IN  GT_U32  macNum
)
{
    if(!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        return 0;
    }
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        return 1;
    }
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        if(macNum < 48)
        {
            return 1;
        }
        return 0;
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
    {
        if(macNum >= 2  && macNum < 26)
        {
            return 1;
        }
        if(macNum >= 28 && macNum < 52)
        {
            return 1;
        }
        return 0;
    }
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
    {
        return 1;
    }

    return 0;
}
/* check if a macNum can ONLY be USX port */
GT_U32 isForceUsxPort(
    IN  GT_U8   devNum,
    IN  GT_U32  macNum
)
{
    if(!isSupportUsxPort(devNum,macNum))
    {
        return 0;
    }
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        return 1;
    }
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        /* Phoenix USX ports not hold additional MAC */
        return 1;
    }
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
    {
        GT_U32  localMac;
        /* Hawk USX ports may also be 'MTI 100' */
        localMac = (macNum % 26);/*we care about ports 2..25*/
        if((localMac-2) % 4) /*those that are not 2,6,10,14,18,22*/
        {
            return 1;
        }

        /* ports 2,6,10,14,18,22 supports also MTI 100 option */
        return 0;
    }

    return 0;
}

/**
* @internal prvCpssCommonPortInterfaceSpeedGet function
* @endinternal
*
* @brief   Check if given pair ifMode and speed supported by given port on
*         given device
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*                                      portNum   - port MAC number (not CPU port)
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @param[out] supportedPtr             ? GT_TRUE ? (ifMode; speed) supported
*                                      GT_FALSE ? (ifMode; speed) not supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if one of input parameters wrong
* @retval GT_BAD_PTR               - if supportedPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssCommonPortInterfaceSpeedGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portMacNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    OUT GT_BOOL                         *supportedPtr
)
{
    GT_PHYSICAL_PORT_NUM localPort;     /* number of port in port group */
    MV_HWS_PORT_STANDARD portMode;      /* port i/f mode and speed translated to BlackBox enum */
    GT_U32               portGroup;     /* local core number */

    *supportedPtr = GT_FALSE;

    switch(ifMode)
    {
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
            if((!PRV_CPSS_SIP_5_CHECK_MAC(devNum)) &&
              (!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
            {
                return GT_OK;
            }

            switch(speed)
            {
                case CPSS_PORT_SPEED_10_E:
                case CPSS_PORT_SPEED_100_E:
                case CPSS_PORT_SPEED_1000_E:
                    portMode = QSGMII;
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_10_E:
                case CPSS_PORT_SPEED_100_E:
                case CPSS_PORT_SPEED_1000_E:
                    portMode = SGMII;
                    break;
                case CPSS_PORT_SPEED_2500_E:
                    portMode = SGMII2_5;
                    if(((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
                        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)) &&
                            (portMacNum == 32))
                    {
                        if(PRV_CPSS_PP_MAC(devNum)->coreClock < 333)
                        {
                            /* for coreclock 200Mhz and 250Mhz port 32 in mode 2.5G SGMII is not supported*/
                            *supportedPtr = GT_FALSE;
                            return GT_OK;
                        }
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_XGMII_E:
            if(CPSS_PORT_SPEED_10000_E == speed)
                portMode = _10GBase_KX4;
            else if(CPSS_PORT_SPEED_16000_E == speed)
            {
                if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E)
                    return GT_OK;
                else
                    portMode = HGS4;
            }
            else if(CPSS_PORT_SPEED_20000_E == speed)
                portMode = _20GBase_KX4;
            else
                return GT_OK;
            break;

        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
            if(CPSS_PORT_SPEED_1000_E == speed)
                portMode = _1000Base_X;
            else
            {
                *supportedPtr = GT_FALSE;
                return GT_OK;
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_2500BASE_X_E:
            if(CPSS_PORT_SPEED_2500_E == speed)
                portMode = _2500Base_X;
            else
            {
                *supportedPtr = GT_FALSE;
                return GT_OK;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_HX_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_10000_E:
                    portMode = _10GBase_KX2;
                    break;
                case CPSS_PORT_SPEED_5000_E:
                    if((!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))&&
                       (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E))
                    {
                        return GT_OK;
                    }
                    portMode = _5GBase_HX;
                    break;
                default:
                    return GT_OK;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_RXAUI_E:
            if(CPSS_PORT_SPEED_10000_E == speed)
                portMode = RXAUI;
            else
                return GT_OK;
            break;

        case CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E:
            /* loopback port implemented over SR_LR */
        case CPSS_PORT_INTERFACE_MODE_KR_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
            /* KR and SR_LR support same speeds range */
            switch(speed)
            {
                case CPSS_PORT_SPEED_1000_E:
                    portMode = _1000Base_X;
                    break;
                case CPSS_PORT_SPEED_5000_E:
                    if((PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum) ||
                        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
                       (CPSS_PORT_INTERFACE_MODE_SR_LR_E == ifMode))
                    {
                        portMode = _5_625GBaseR;
                    }
                    else if((PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum) ||
                             PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
                            (CPSS_PORT_INTERFACE_MODE_KR_E == ifMode))
                    {
                        portMode = _5GBaseR;
                    }
                    else if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                    {
                        portMode = _5GBaseR;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_10000_E:
                    portMode = _10GBase_KR;
                    break;
                case CPSS_PORT_SPEED_12000_E:
                    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
                    {
                        portMode = _12_1GBase_KR;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_12500_E:
                    if(PRV_CPSS_SIP_5_16_CHECK_MAC(devNum))
                    {
                        portMode = _12_5GBase_KR;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_20000_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        portMode = _20GBase_KR;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;

                case CPSS_PORT_SPEED_25000_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        if(CPSS_PORT_INTERFACE_MODE_SR_LR_E == ifMode)
                        {
                            portMode = _25GBase_SR;
                        }
                        else
                        {
                            portMode = _25GBase_KR;
                        }
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;

                case CPSS_PORT_SPEED_50000_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        if(CPSS_PORT_INTERFACE_MODE_SR_LR_E == ifMode)
                        {
                            portMode = _50GBase_SR_LR;
                        }
                        else
                        {
                            if(PRV_CPSS_PP_MAC(devNum)->isGmDevice && !PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                            {
                                /* falcon GM simulation use BC3 GOP/SERDES logic */
                                portMode = _10GBase_KR;
                            }
                            else
                            {
                                portMode = _50GBase_KR;
                            }
                        }
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;

            case CPSS_PORT_SPEED_26700_E:
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                {
                    portMode = _26_7GBase_KR;
                }
                else
                {
                    return GT_OK;
                }
                break;

                default:
                    return GT_OK;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_KR_S_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_25000_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        portMode = _25GBase_KR_S;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                    return GT_OK;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_CR_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_25000_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        portMode = _25GBase_CR;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_50000_E:
                    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        portMode = _50GBase_CR;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                    return GT_OK;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_CR_S_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_25000_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        portMode = _25GBase_CR_S;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                    return GT_OK;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_KR2_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_20000_E:
                    portMode = _20GBase_KR2;
                    break;
                case CPSS_PORT_SPEED_25000_E:
                    if(PRV_CPSS_SIP_5_16_CHECK_MAC(devNum))
                    {
                        portMode = _25GBase_KR2;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_50000_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        portMode = _50GBase_KR2;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_52500_E:
                    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)
                        || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PX_FAMILY_PIPE_E))
                    {
                        portMode = _52_5GBase_KR2;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_100G_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        portMode = _100GBase_KR2;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_106G_E:
                    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                    {
                        portMode = _106GBase_KR2;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_102G_E:
                    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
                    {
                        portMode = _102GBase_KR2;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_40000_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        portMode = _40GBase_KR2;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;

                case CPSS_PORT_SPEED_53000_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
                    {
                        portMode = _53GBase_KR2;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                    return GT_OK;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_CR2_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_50000_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        portMode = _50GBase_CR2;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_100G_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        portMode = _100GBase_CR2;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                    return GT_OK;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_SR_LR2_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_20000_E:
                    portMode = _20GBase_SR_LR2;
                    break;
                case CPSS_PORT_SPEED_50000_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        portMode = _50GBase_SR2;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_100G_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        portMode = _100GBase_SR_LR2;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                    return GT_OK;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_KR4_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_50000_E:
                    if(PRV_CPSS_SIP_5_16_CHECK_MAC(devNum))
                    {
                        portMode = _50GBase_KR4;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;

                case CPSS_PORT_SPEED_40000_E:
                    if(CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode)
                    {/* as we see just 10G and 20G make sense to use for
                        loopback ports */
                        return GT_OK;
                    }
                    portMode = _40GBase_KR4;
                    break;

                case CPSS_PORT_SPEED_100G_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        portMode = _100GBase_KR4;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;

                case CPSS_PORT_SPEED_107G_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        portMode = _107GBase_KR4;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_102G_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        portMode = _102GBase_KR4;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_200G_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        portMode = _200GBase_KR4;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_212G_E:
                    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                    {
                        portMode = _212GBase_KR4;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_106G_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
                    {
                        portMode = _106GBase_KR4;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;

                case CPSS_PORT_SPEED_42000_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
                    {
                        portMode = _42GBase_KR4;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                    return GT_OK;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_CR4_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_100G_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        portMode = _100GBase_CR4;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;

                case CPSS_PORT_SPEED_40000_E:
                    portMode = _40GBase_CR4;
                    break;
                case CPSS_PORT_SPEED_200G_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        portMode = _200GBase_CR4;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                    return GT_OK;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_SR_LR4_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_29090_E:
                    /* For BobK, AC3X and Aldrin devices */
                    if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)  ||
                        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E)    ||
                        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ||
                        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E) || /*  not using SIP_5_20 macro as Falcon is not applicable here */
                       ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
                        (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)))
                    {
                        portMode = _29_09GBase_SR4;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_100G_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        portMode = _100GBase_SR4;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_40000_E:
                    if(CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode)
                    {/* as we see just 10G and 20G make sense to use for
                        loopback ports */
                        return GT_OK;
                    }
                    portMode = _40GBase_SR_LR4;
                    break;
                case CPSS_PORT_SPEED_200G_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        portMode = _200GBase_SR_LR4;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                    return GT_OK;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E:
            switch(speed)
            {/* Multi-Link Gearbox speeds: 40G, 10G, 40G, 10G */
                case CPSS_PORT_SPEED_40000_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) &&
                        ((portMacNum % 2) == 0)) /* portMacNum is even */
                    {
                        portMode = _100GBase_MLG;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_10000_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) &&
                        ((portMacNum % 2) == 1)) /* portMacNum is odd */
                    {
                        portMode = _100GBase_MLG;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                    return GT_OK;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_HGL_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_15000_E:
                    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E)
                        return GT_OK;
                    portMode = HGL;
                    break;
                case CPSS_PORT_SPEED_16000_E:
                    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E)
                        return GT_OK;
                    portMode = HGL16G;
                    break;
                case CPSS_PORT_SPEED_40000_E:
                    portMode = XLHGL_KR4;
                    break;
                default:
                    return GT_OK;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_ILKN4_E:
            if(!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
                return GT_OK;

            switch(speed)
            {
                case CPSS_PORT_SPEED_20000_E:
                    portMode = INTLKN_4Lanes_6_25G;
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_ILKN8_E:
            if(!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
                return GT_OK;

            switch(speed)
            {
                case CPSS_PORT_SPEED_20000_E:
                    portMode = INTLKN_8Lanes_3_125G;
                    break;
                case CPSS_PORT_SPEED_40000_E:
                    portMode = INTLKN_8Lanes_6_25G;
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;


        case CPSS_PORT_INTERFACE_MODE_QX_E:
            if((!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)) &&
               (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PX_FAMILY_PIPE_E))
                return GT_OK;

            switch(speed)
            {
                case CPSS_PORT_SPEED_2500_E:
                    portMode = _2_5GBase_QX;
                    break;
                case CPSS_PORT_SPEED_5000_E:
                    portMode = _5GBase_DQX;
                    break;
                default:
                    return GT_OK;
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_XHGS_SR_E:
            if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
            {
                return GT_OK;
            }

            if ((CPSS_PORT_SPEED_22000_E == speed) &&
                (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
            {
                portMode = _22GBase_SR;
                break;
            }
            GT_ATTR_FALLTHROUGH;
        case CPSS_PORT_INTERFACE_MODE_XHGS_E:
            switch (speed)
            {
                case CPSS_PORT_SPEED_11800_E:
                    portMode = (CPSS_PORT_INTERFACE_MODE_XHGS_E == ifMode) ?
                                                               _12GBaseR : _12GBase_SR;
                    break;

                case CPSS_PORT_SPEED_23600_E:

                    if(ifMode == CPSS_PORT_INTERFACE_MODE_XHGS_SR_E)
                    {
                        *supportedPtr = GT_FALSE;
                        return GT_OK;
                    }
                    portMode = _24GBase_KR2;
                    break;

                case CPSS_PORT_SPEED_47200_E:
                    if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
                    {
                        return GT_OK;
                    }
                    portMode = (CPSS_PORT_INTERFACE_MODE_XHGS_E == ifMode) ?
                                                               _48GBaseR4 : _48GBase_SR4;
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_KR_C_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_25000_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        portMode = _25GBase_KR_C;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_KR2_C_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_50000_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        portMode = _50GBase_KR2_C;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_CR_C_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_25000_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        portMode = _25GBase_CR_C;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_CR2_C_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_50000_E:
                    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        portMode = _50GBase_CR2_C;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_KR8_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_200G_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        portMode = _200GBase_KR8;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_400G_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        portMode = _400GBase_KR8;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_424G_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        portMode = _424GBase_KR8;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_CR8_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_200G_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        portMode = _200GBase_CR8;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_400G_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        portMode = _400GBase_CR8;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_SR_LR8_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_200G_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        portMode = _200GBase_SR_LR8;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                case CPSS_PORT_SPEED_400G_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        portMode = _400GBase_SR_LR8;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_USX_2_5G_SXGMII_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_10_E:
                case CPSS_PORT_SPEED_100_E:
                case CPSS_PORT_SPEED_1000_E:
                case CPSS_PORT_SPEED_2500_E:
                    if(isSupportUsxPort(devNum,portMacNum))
                    {
                        portMode = _2_5G_SXGMII;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_USX_5G_SXGMII_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_10_E:
                case CPSS_PORT_SPEED_100_E:
                case CPSS_PORT_SPEED_1000_E:
                case CPSS_PORT_SPEED_2500_E:
                case CPSS_PORT_SPEED_5000_E:
                    if(isSupportUsxPort(devNum,portMacNum))
                    {
                        portMode = _5G_SXGMII;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_USX_10G_SXGMII_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_10_E:
                case CPSS_PORT_SPEED_100_E:
                case CPSS_PORT_SPEED_1000_E:
                case CPSS_PORT_SPEED_2500_E:
                case CPSS_PORT_SPEED_5000_E:
                case CPSS_PORT_SPEED_10000_E:
                    if(isSupportUsxPort(devNum,portMacNum))
                    {
                        portMode = _10G_SXGMII;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_5G_DXGMII_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_10_E:
                case CPSS_PORT_SPEED_100_E:
                case CPSS_PORT_SPEED_1000_E:
                case CPSS_PORT_SPEED_2500_E:
                    if(isSupportUsxPort(devNum,portMacNum))
                    {
                        portMode = _5G_DXGMII;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_10G_DXGMII_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_10_E:
                case CPSS_PORT_SPEED_100_E:
                case CPSS_PORT_SPEED_1000_E:
                case CPSS_PORT_SPEED_2500_E:
                case CPSS_PORT_SPEED_5000_E:
                    if(isSupportUsxPort(devNum,portMacNum))
                    {
                        portMode = _10G_DXGMII;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_20G_DXGMII_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_10_E:
                case CPSS_PORT_SPEED_100_E:
                case CPSS_PORT_SPEED_1000_E:
                case CPSS_PORT_SPEED_2500_E:
                case CPSS_PORT_SPEED_5000_E:
                case CPSS_PORT_SPEED_10000_E:
                    if(isSupportUsxPort(devNum,portMacNum))
                    {
                        portMode = _20G_DXGMII;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_QUSGMII_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_10_E:
                case CPSS_PORT_SPEED_100_E:
                case CPSS_PORT_SPEED_1000_E:
                    if(isSupportUsxPort(devNum,portMacNum))
                    {
                        portMode = _5G_QUSGMII;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_10G_QXGMII_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_10_E:
                case CPSS_PORT_SPEED_100_E:
                case CPSS_PORT_SPEED_1000_E:
                case CPSS_PORT_SPEED_2500_E:
                    if(isSupportUsxPort(devNum,portMacNum))
                    {
                        portMode = _10G_QXGMII;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_20G_QXGMII_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_10_E:
                case CPSS_PORT_SPEED_100_E:
                case CPSS_PORT_SPEED_1000_E:
                case CPSS_PORT_SPEED_2500_E:
                case CPSS_PORT_SPEED_5000_E:
                    if(isSupportUsxPort(devNum,portMacNum))
                    {
                        portMode = _20G_QXGMII;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_10_E:
                case CPSS_PORT_SPEED_100_E:
                case CPSS_PORT_SPEED_1000_E:
                    if(isSupportUsxPort(devNum,portMacNum))
                    {
                        portMode = _10G_OUSGMII;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_10_E:
                case CPSS_PORT_SPEED_100_E:
                case CPSS_PORT_SPEED_1000_E:
                case CPSS_PORT_SPEED_2500_E:
                    if(isSupportUsxPort(devNum,portMacNum))
                    {
                        portMode = _20G_OXGMII;
                    }
                    else
                    {
                        return GT_OK;
                    }
                    break;
                default:
                {
                    *supportedPtr = GT_FALSE;
                    return GT_OK;
                }
            }
            break;

        default:
        {
            *supportedPtr = GT_FALSE;
            return GT_OK;
        }
    }

    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacNum);

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if(   (!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
       && (!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
       && (localPort >= 12))
    {/* to avoid coverity's OVERRUN_STATIC */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    *supportedPtr = hwsIsPortModeSupported(devNum,portGroup,localPort,portMode);
    return GT_OK;
}

/**
* @internal prvCpssCommonPortIfModeToHwsTranslate function
* @endinternal
*
* @brief   Translate port interface mode and speed from CPSS enum to hwService enum
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum               - device number
* @param[in] cpssIfMode               - i/f mode in CPSS format
* @param[in] cpssSpeed                - port speed in CPSS format
*
* @param[out] hwsIfModePtr             - port i/f mode in format of hwServices library
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - i/f mode/speed pair not supported
*/
GT_STATUS prvCpssCommonPortIfModeToHwsTranslate
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    cpssIfMode,
    IN  CPSS_PORT_SPEED_ENT             cpssSpeed,
    OUT MV_HWS_PORT_STANDARD            *hwsIfModePtr
)
{
    PRV_CPSS_DEV_CHECK_MAC(devNum);

    *hwsIfModePtr = NON_SUP_MODE;
    devNum = devNum;

    if(CPSS_PORT_SPEED_REMOTE_E == cpssSpeed )
    {   /*we are done*/
        return GT_OK;
    }
    switch(cpssIfMode)
    {
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
            *hwsIfModePtr = (CPSS_PORT_SPEED_2500_E == cpssSpeed) ?
                                SGMII2_5 : SGMII;
            break;
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
            *hwsIfModePtr = QSGMII;
            break;
        case CPSS_PORT_INTERFACE_MODE_XGMII_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_10000_E:
                    *hwsIfModePtr = _10GBase_KX4;
                    break;
                case CPSS_PORT_SPEED_20000_E:
                    *hwsIfModePtr = _20GBase_KX4;
                    break;
                case CPSS_PORT_SPEED_12000_E:
                    *hwsIfModePtr = HGS;
                    break;
                case CPSS_PORT_SPEED_16000_E:
                    *hwsIfModePtr = HGS4;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
            *hwsIfModePtr = _1000Base_X;
            break;
        case CPSS_PORT_INTERFACE_MODE_2500BASE_X_E:
            *hwsIfModePtr = _2500Base_X;
            break;
        case CPSS_PORT_INTERFACE_MODE_HX_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_10000_E:
                    *hwsIfModePtr = _10GBase_KX2;
                    break;
                case CPSS_PORT_SPEED_5000_E:
                    *hwsIfModePtr = _5GBase_HX;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_QX_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_2500_E:
                    *hwsIfModePtr = _2_5GBase_QX;
                    break;
                case CPSS_PORT_SPEED_5000_E:
                    *hwsIfModePtr = _5GBase_DQX;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_RXAUI_E:
            *hwsIfModePtr = RXAUI;
            break;
        case CPSS_PORT_INTERFACE_MODE_KR_E:
            switch(cpssSpeed)
            {
               case CPSS_PORT_SPEED_5000_E:
                    *hwsIfModePtr = _5GBaseR;
                    break;
                case CPSS_PORT_SPEED_10000_E:
                    *hwsIfModePtr = _10GBase_KR;
                    break;

                case CPSS_PORT_SPEED_12000_E:
                    *hwsIfModePtr = _12_1GBase_KR;
                    break;
                case CPSS_PORT_SPEED_12500_E:
                    *hwsIfModePtr = _12_5GBase_KR;
                    break;
                case CPSS_PORT_SPEED_20000_E:
                    *hwsIfModePtr = _20GBase_KR;
                    break;
                case CPSS_PORT_SPEED_25000_E:
                    *hwsIfModePtr = _25GBase_KR;
                    break;
                case CPSS_PORT_SPEED_26700_E:
                    *hwsIfModePtr = _26_7GBase_KR;
                    break;
                case CPSS_PORT_SPEED_50000_E:
                    if(PRV_CPSS_PP_MAC(devNum)->isGmDevice && !PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                    {
                        /* falcon GM simulation use BC3 GOP/SERDES logic */
                        *hwsIfModePtr = _10GBase_KR;
                    }
                    else
                    {
                        *hwsIfModePtr = _50GBase_KR;
                    }
                    break;

                case CPSS_PORT_SPEED_100G_E:
                    *hwsIfModePtr = _100GBase_KR10;
                    break;

                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_CR_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_25000_E:
                    *hwsIfModePtr = _25GBase_CR;
                    break;
                case CPSS_PORT_SPEED_50000_E:
                        *hwsIfModePtr = _50GBase_CR;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_KR_S_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_25000_E:
                    *hwsIfModePtr = _25GBase_KR_S;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_CR_S_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_25000_E:
                    *hwsIfModePtr = _25GBase_CR_S;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_KR2_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_20000_E:
                    *hwsIfModePtr = _20GBase_KR2;
                    break;
                case CPSS_PORT_SPEED_25000_E:
                    *hwsIfModePtr = _25GBase_KR2;
                    break;
                case CPSS_PORT_SPEED_50000_E:
                    *hwsIfModePtr = _50GBase_KR2;
                    break;
                case CPSS_PORT_SPEED_52500_E:
                    *hwsIfModePtr = _52_5GBase_KR2;
                    break;
                case CPSS_PORT_SPEED_40000_E:
                    *hwsIfModePtr = _40GBase_KR2;
                    break;
                case CPSS_PORT_SPEED_100G_E:
                    *hwsIfModePtr = _100GBase_KR2;
                    break;
                case CPSS_PORT_SPEED_106G_E:
                    *hwsIfModePtr = _106GBase_KR2;
                    break;
                case CPSS_PORT_SPEED_53000_E:
                    *hwsIfModePtr = _53GBase_KR2;
                    break;
                case CPSS_PORT_SPEED_102G_E:
                    *hwsIfModePtr = _102GBase_KR2;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_CR2_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_50000_E:
                    *hwsIfModePtr = _50GBase_CR2;
                    break;
                case CPSS_PORT_SPEED_100G_E:
                    *hwsIfModePtr = _100GBase_CR2;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_CR4_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_40000_E:
                    *hwsIfModePtr = _40GBase_CR4;
                    break;
                case CPSS_PORT_SPEED_100G_E:
                    *hwsIfModePtr = _100GBase_CR4;
                    break;
                case CPSS_PORT_SPEED_200G_E:
                    *hwsIfModePtr = _200GBase_CR4;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_KR4_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_40000_E:
                    *hwsIfModePtr = _40GBase_KR4;
                    break;
                case CPSS_PORT_SPEED_50000_E:
                    *hwsIfModePtr = _50GBase_KR4;
                    break;
                case CPSS_PORT_SPEED_100G_E:
                    *hwsIfModePtr = _100GBase_KR4;
                    break;
                case CPSS_PORT_SPEED_102G_E:
                    *hwsIfModePtr = _102GBase_KR4;
                    break;
                case CPSS_PORT_SPEED_107G_E:
                    *hwsIfModePtr = _107GBase_KR4;
                    break;
                case CPSS_PORT_SPEED_200G_E:
                    *hwsIfModePtr = _200GBase_KR4;
                    break;
                case CPSS_PORT_SPEED_212G_E:
                    *hwsIfModePtr = _212GBase_KR4;
                    break;
                case CPSS_PORT_SPEED_106G_E:
                    *hwsIfModePtr = _106GBase_KR4;
                    break;
                case CPSS_PORT_SPEED_42000_E:
                    *hwsIfModePtr = _42GBase_KR4;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_HGL_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_15000_E:
                    *hwsIfModePtr = HGL;
                    break;
                case CPSS_PORT_SPEED_16000_E:
                    *hwsIfModePtr = HGL16G;
                    break;
                case CPSS_PORT_SPEED_40000_E:
                    *hwsIfModePtr = XLHGL_KR4;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_CHGL_12_E:
            *hwsIfModePtr = CHGL;
            break;
        case CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_1000_E:
                    *hwsIfModePtr = _1000Base_X;
                    break;

                case CPSS_PORT_SPEED_5000_E:
                    *hwsIfModePtr = _5_625GBaseR;
                    break;

                case CPSS_PORT_SPEED_10000_E:
                    *hwsIfModePtr = _10GBase_SR_LR;
                    break;

                case CPSS_PORT_SPEED_25000_E:
                    *hwsIfModePtr = _25GBase_SR;
                    break;

                case CPSS_PORT_SPEED_50000_E:
                    *hwsIfModePtr = _50GBase_SR_LR;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_SR_LR2_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_20000_E:
                    *hwsIfModePtr = _20GBase_SR_LR2;
                    break;
                case CPSS_PORT_SPEED_50000_E:
                    *hwsIfModePtr = _50GBase_SR2;
                    break;
                case CPSS_PORT_SPEED_100G_E:
                    *hwsIfModePtr = _100GBase_SR_LR2;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_SR_LR4_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_40000_E:
                    *hwsIfModePtr = _40GBase_SR_LR4;
                    break;
                case CPSS_PORT_SPEED_29090_E:
                    *hwsIfModePtr = _29_09GBase_SR4;
                    break;
                case CPSS_PORT_SPEED_100G_E:
                    *hwsIfModePtr = _100GBase_SR4;
                    break;
                case CPSS_PORT_SPEED_200G_E:
                    *hwsIfModePtr = _200GBase_SR_LR4;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_10000_E:
                case CPSS_PORT_SPEED_40000_E:
                    *hwsIfModePtr = _100GBase_MLG;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_ILKN4_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_20000_E:
                    *hwsIfModePtr = INTLKN_4Lanes_6_25G;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_ILKN8_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_20000_E:
                    *hwsIfModePtr = INTLKN_8Lanes_3_125G;
                    break;
                case CPSS_PORT_SPEED_40000_E:
                    *hwsIfModePtr = INTLKN_8Lanes_6_25G;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_XHGS_E:
        case CPSS_PORT_INTERFACE_MODE_XHGS_SR_E:
            switch (cpssSpeed)
            {
                case CPSS_PORT_SPEED_11800_E:
                    *hwsIfModePtr = (CPSS_PORT_INTERFACE_MODE_XHGS_E == cpssIfMode) ?
                                                               _12GBaseR : _12GBase_SR;
                    break;

                case CPSS_PORT_SPEED_23600_E:
                    *hwsIfModePtr = _24GBase_KR2;
                    break;

                case CPSS_PORT_SPEED_47200_E:
                    *hwsIfModePtr = (CPSS_PORT_INTERFACE_MODE_XHGS_E == cpssIfMode) ?
                                                               _48GBaseR4 : _48GBase_SR4;
                    break;

                case CPSS_PORT_SPEED_22000_E:
                    *hwsIfModePtr = _22GBase_SR;
                    break;

                default:
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
                }
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_KR_C_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_25000_E:
                    *hwsIfModePtr = _25GBase_KR_C;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_KR2_C_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_50000_E:
                    *hwsIfModePtr = _50GBase_KR2_C;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_CR_C_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_25000_E:
                    *hwsIfModePtr = _25GBase_CR_C;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_CR2_C_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_50000_E:
                    *hwsIfModePtr = _50GBase_CR2_C;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_KR8_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_200G_E:
                    *hwsIfModePtr = _200GBase_KR8;
                    break;
                case CPSS_PORT_SPEED_400G_E:
                    *hwsIfModePtr = _400GBase_KR8;
                    break;
                case CPSS_PORT_SPEED_424G_E:
                    *hwsIfModePtr = _424GBase_KR8;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_CR8_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_200G_E:
                    *hwsIfModePtr = _200GBase_CR8;
                    break;
                case CPSS_PORT_SPEED_400G_E:
                    *hwsIfModePtr = _400GBase_CR8;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_SR_LR8_E:
            switch(cpssSpeed)
            {
                case CPSS_PORT_SPEED_200G_E:
                    *hwsIfModePtr = _200GBase_SR_LR8;
                    break;
                case CPSS_PORT_SPEED_400G_E:
                    *hwsIfModePtr = _400GBase_SR_LR8;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            break;


        case CPSS_PORT_INTERFACE_MODE_USX_2_5G_SXGMII_E:
            *hwsIfModePtr = _2_5G_SXGMII;
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_5G_SXGMII_E:
            *hwsIfModePtr = _5G_SXGMII;
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_10G_SXGMII_E:
            *hwsIfModePtr = _10G_SXGMII;
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_5G_DXGMII_E:
            *hwsIfModePtr = _5G_DXGMII;
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_10G_DXGMII_E:
            *hwsIfModePtr = _10G_DXGMII;
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_20G_DXGMII_E:
            *hwsIfModePtr = _20G_DXGMII;
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_QUSGMII_E:
            *hwsIfModePtr = _5G_QUSGMII;
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_10G_QXGMII_E:
            *hwsIfModePtr = _10G_QXGMII;
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_20G_QXGMII_E:
            *hwsIfModePtr = _20G_QXGMII;
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E:
            *hwsIfModePtr = _10G_OUSGMII;
            break;
        case CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E:
            *hwsIfModePtr = _20G_OXGMII;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "cpssIfMode %d, cpssSpeed %d",cpssIfMode, cpssSpeed);
    }

    return GT_OK;
}

/**
* @internal prvCpssCommonPortSpeedEnumToMbPerSecConvert function
* @endinternal
*
* @brief   Convert Port speed enum value to amount megabits per
*          second
*
* @param[in] cpssSpeed                - port speed in CPSS format
*
* @retval Speed in megabits per second, 0 - for wrong values
*/
GT_U32 prvCpssCommonPortSpeedEnumToMbPerSecConvert
(
    IN  CPSS_PORT_SPEED_ENT             cpssSpeed
)
{
    switch (cpssSpeed)
    {
        case CPSS_PORT_SPEED_10_E:       return 10;
        case CPSS_PORT_SPEED_100_E:      return 100;
        case CPSS_PORT_SPEED_1000_E:     return 1000;
        case CPSS_PORT_SPEED_10000_E:    return 10000;
        case CPSS_PORT_SPEED_12000_E:    return 12000;
        case CPSS_PORT_SPEED_2500_E:     return 2500;
        case CPSS_PORT_SPEED_5000_E:     return 5000;
        case CPSS_PORT_SPEED_13600_E:    return 13600;
        case CPSS_PORT_SPEED_20000_E:    return 20000;
        case CPSS_PORT_SPEED_40000_E:    return 40000;
        case CPSS_PORT_SPEED_16000_E:    return 16000;
        case CPSS_PORT_SPEED_15000_E:    return 15000;
        case CPSS_PORT_SPEED_75000_E:    return 75000;
        case CPSS_PORT_SPEED_100G_E:     return 100000;
        case CPSS_PORT_SPEED_50000_E:    return 50000;
        case CPSS_PORT_SPEED_52500_E:    return 52500;
        case CPSS_PORT_SPEED_140G_E:     return 140000;
        case CPSS_PORT_SPEED_11800_E:    return 118000;
        case CPSS_PORT_SPEED_47200_E:    return 47200;
        case CPSS_PORT_SPEED_22000_E:    return 22000;
        case CPSS_PORT_SPEED_23600_E:    return 23600;
        case CPSS_PORT_SPEED_12500_E:    return 12500;
        case CPSS_PORT_SPEED_25000_E:    return 25000;
        case CPSS_PORT_SPEED_26700_E:    return 26700;
        case CPSS_PORT_SPEED_107G_E:     return 107000;
        case CPSS_PORT_SPEED_29090_E:    return 29090;
        case CPSS_PORT_SPEED_200G_E:     return 200000;
        case CPSS_PORT_SPEED_400G_E:     return 400000;
        case CPSS_PORT_SPEED_102G_E:     return 102000;
        case CPSS_PORT_SPEED_106G_E:     return 106000;
        case CPSS_PORT_SPEED_53000_E:    return 53000;
        case CPSS_PORT_SPEED_42000_E:    return 42000;
        case CPSS_PORT_SPEED_424G_E:     return 424000;
        case CPSS_PORT_SPEED_212G_E:     return 212000;
        default: return 0;
    }
}

/**
* @internal prvCpssCommonPortModeEnumToSerdesUsageUnitsConvert function
* @endinternal
*
* @brief   Convert Port mode enum value to amount of serdes usage units
* @note    Serdes usage unit is 1/256 of one SerDes line - defined to support USX modes
*
* @param[in] portMode                - port mode in CPSS format
*
* @retval Amount of Serdes usage units occupated by the given mode
*/
GT_U32 prvCpssCommonPortModeEnumToSerdesUsageUnitsConvert
(
    IN  CPSS_PORT_INTERFACE_MODE_ENT      portMode
)
{
    switch (portMode)
    {
        case CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E:
        case CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E:
        case CPSS_PORT_INTERFACE_MODE_MII_E:
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
        case CPSS_PORT_INTERFACE_MODE_XGMII_E:
        case CPSS_PORT_INTERFACE_MODE_MGMII_E:
        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_2500BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_GMII_E:
        case CPSS_PORT_INTERFACE_MODE_MII_PHY_E:
        case CPSS_PORT_INTERFACE_MODE_QX_E:
        case CPSS_PORT_INTERFACE_MODE_HX_E:
        case CPSS_PORT_INTERFACE_MODE_RXAUI_E:
        case CPSS_PORT_INTERFACE_MODE_100BASE_FX_E:
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
        case CPSS_PORT_INTERFACE_MODE_XLG_E:
        case CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E:
        case CPSS_PORT_INTERFACE_MODE_KR_E:
        case CPSS_PORT_INTERFACE_MODE_HGL_E:
        case CPSS_PORT_INTERFACE_MODE_CHGL_12_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
        case CPSS_PORT_INTERFACE_MODE_XHGS_E:
        case CPSS_PORT_INTERFACE_MODE_XHGS_SR_E:
        case CPSS_PORT_INTERFACE_MODE_KR_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR_E:
        case CPSS_PORT_INTERFACE_MODE_KR_S_E:
        case CPSS_PORT_INTERFACE_MODE_CR_S_E:
            return (1 * 256);

        case CPSS_PORT_INTERFACE_MODE_KR2_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR2_E:
        case CPSS_PORT_INTERFACE_MODE_KR2_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR2_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR2_E:
            return (2 * 256);

        case CPSS_PORT_INTERFACE_MODE_KR4_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR4_E:
        case CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E:
        case CPSS_PORT_INTERFACE_MODE_CR4_E:
            return (4 * 256);

        case CPSS_PORT_INTERFACE_MODE_KR8_E:
        case CPSS_PORT_INTERFACE_MODE_CR8_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR8_E:
            return (8 * 256);

        /* USX 1 Lane */
        case CPSS_PORT_INTERFACE_MODE_USX_2_5G_SXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_5G_SXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_10G_SXGMII_E:
            return (1 * 256);

        /* USX 2 Lanes */
        case CPSS_PORT_INTERFACE_MODE_USX_5G_DXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_10G_DXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_20G_DXGMII_E:
            return (256 / 2);

        /* USX 4 Lanes */
        case CPSS_PORT_INTERFACE_MODE_USX_QUSGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_10G_QXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_20G_QXGMII_E:
            return (256 / 4);

        /* USX 8 Lanes */
        case CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E:
            return (256 / 8);

        default: return 0; /* mode not supported or not related to SerDes */
    }
}

/**
* @internal prvCpssCommonPortUsxModeMaxSpeedGet function
* @endinternal
*
* @brief   Get maximal speed supported by USX mode
*
* @param[in] portMode                - port mode in CPSS format
*
* @retval Maximal speed in CPSS enum, CPSS_PORT_SPEED_NA_E for not USX modes.
*/
CPSS_PORT_SPEED_ENT prvCpssCommonPortUsxModeMaxSpeedGet
(
    IN  CPSS_PORT_INTERFACE_MODE_ENT      portMode
)
{
    switch (portMode)
    {
        case CPSS_PORT_INTERFACE_MODE_USX_2_5G_SXGMII_E: return CPSS_PORT_SPEED_2500_E;
        case CPSS_PORT_INTERFACE_MODE_USX_5G_SXGMII_E:   return CPSS_PORT_SPEED_5000_E;
        case CPSS_PORT_INTERFACE_MODE_USX_10G_SXGMII_E:  return CPSS_PORT_SPEED_10000_E;
        case CPSS_PORT_INTERFACE_MODE_USX_5G_DXGMII_E:   return CPSS_PORT_SPEED_2500_E;
        case CPSS_PORT_INTERFACE_MODE_USX_10G_DXGMII_E:  return CPSS_PORT_SPEED_5000_E;
        case CPSS_PORT_INTERFACE_MODE_USX_20G_DXGMII_E:  return CPSS_PORT_SPEED_10000_E;
        case CPSS_PORT_INTERFACE_MODE_USX_QUSGMII_E:     return CPSS_PORT_SPEED_1000_E;
        case CPSS_PORT_INTERFACE_MODE_USX_10G_QXGMII_E:  return CPSS_PORT_SPEED_2500_E;
        case CPSS_PORT_INTERFACE_MODE_USX_20G_QXGMII_E:  return CPSS_PORT_SPEED_5000_E;
        case CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E:     return CPSS_PORT_SPEED_1000_E;
        case CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E:  return CPSS_PORT_SPEED_2500_E;
        default: return CPSS_PORT_SPEED_NA_E;
    }
}


/**
* @internal prvCpssCommonPortSpeedEnumToHwsSpeedConvert function
* @endinternal
*
* @brief   Convert Port speed enum value to hws speed enum
*
* @param[in] cpssSpeed                - port speed in CPSS format
*
* @retval hws Speed enum, 0 - for wrong values
*/
MV_HWS_PORT_SPEED_ENT prvCpssCommonPortSpeedEnumToHwsSpeedConvert
(
    IN  CPSS_PORT_SPEED_ENT             cpssSpeed
)
{
    switch (cpssSpeed)
    {
        case CPSS_PORT_SPEED_10_E:       return MV_HWS_PORT_SPEED_10M_E;
        case CPSS_PORT_SPEED_100_E:      return MV_HWS_PORT_SPEED_100M_E;
        case CPSS_PORT_SPEED_1000_E:     return MV_HWS_PORT_SPEED_1G_E;
        case CPSS_PORT_SPEED_10000_E:    return MV_HWS_PORT_SPEED_10G_E;
        case CPSS_PORT_SPEED_2500_E:     return MV_HWS_PORT_SPEED_2_5G_E;
        case CPSS_PORT_SPEED_5000_E:     return MV_HWS_PORT_SPEED_5G_E;
        case CPSS_PORT_SPEED_20000_E:    return MV_HWS_PORT_SPEED_20G_E;
        case CPSS_PORT_SPEED_40000_E:    return MV_HWS_PORT_SPEED_40G_E;
        case CPSS_PORT_SPEED_100G_E:
        case CPSS_PORT_SPEED_102G_E:
        case CPSS_PORT_SPEED_106G_E:
        case CPSS_PORT_SPEED_107G_E:
            return MV_HWS_PORT_SPEED_100G_E;
        case CPSS_PORT_SPEED_50000_E:
            return MV_HWS_PORT_SPEED_50G_E;
        case CPSS_PORT_SPEED_200G_E:
        case CPSS_PORT_SPEED_212G_E:
            return MV_HWS_PORT_SPEED_200G_E;
        case CPSS_PORT_SPEED_400G_E:
        case CPSS_PORT_SPEED_424G_E:
            return MV_HWS_PORT_SPEED_400G_E;
        default: return MV_HWS_PORT_SPEED_NA_E;
    }
}

#ifdef CHX_FAMILY
extern GT_STATUS prvCpssDxChPortTraceDelay
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_U32      millisec
);
#endif /*CHX_FAMILY*/
#ifdef PX_FAMILY
extern GT_STATUS prvCpssPxPortTraceDelay
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_U32      millisec
);
#endif /*PX_FAMILY*/

/**
* @internal prvCpssCommonPortTraceDelay function
* @endinternal
*
* @brief   common to DX and PX : Make delay and trace it
*
* @param[in] devNum                   - PP device number
* @param[in] portGroupId              - ports group number
* @param[in] millisec                 -  the delay in millisec
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS prvCpssCommonPortTraceDelay
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_U32      millisec
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devInfoPtr;
    PRV_CPSS_DEV_CHECK_MAC(devNum);

    devInfoPtr = PRV_CPSS_PP_MAC(devNum);

    if(CPSS_IS_DXCH_FAMILY_MAC(devInfoPtr->devFamily))
    {
#ifdef CHX_FAMILY
        return prvCpssDxChPortTraceDelay(devNum,portGroupId,millisec);
#else
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, "DX code missing");
#endif /*CHX_FAMILY*/
    }

    if(CPSS_IS_PX_FAMILY_MAC(devInfoPtr->devFamily))
    {
#ifdef PX_FAMILY
        return prvCpssPxPortTraceDelay(devNum,portGroupId,millisec);
#else
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, "PX code missing");
#endif /*PX_FAMILY*/
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
}
