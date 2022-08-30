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
*******************************************************************************
* @file mvHwsD2dPcsIf.c
*
* @brief D2D PCS interface API
*
* @version 1
*
*******************************************************************************/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/d2dPcs/mvHwsD2dPcsIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/gw16/mvGw16If.h>

#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>

#ifdef CHX_FAMILY
extern  GT_U32 hwsFalconNumOfRavens;
extern GT_U32  hwsRavenDevBmp; /* used for falcon Z2 if we want to use only several ravens and not all */
#endif

static char* mvHwsD2dPcsTypeGetFunc(void)
{
  return "D2D_PCS";
}
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};

#ifndef FALCON_DEV_SUPPORT
#define HAWK_SKIP_D2D_CONFIG_MAC    \
    if(hwsFalconNumOfRavens == 0)   \
    {                               \
        /* support Hawk */          \
        return GT_OK;               \
    }
#else
#define HAWK_SKIP_D2D_CONFIG_MAC    \

#endif

GT_STATUS mvHwsD2dPcsReset
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_BOOL enable
)
{
    GT_U32 regAddr, regData, regMask;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 d2dIndex;

    /* support device without D2D */
    HAWK_SKIP_D2D_CONFIG_MAC;

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        /*Eagle: Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[1].D2D_RegFile.PCS_OOR_CFG=0x1*/
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {   /*RAVEN D2D */
        /*Raven: Wrote register via map d2d_reg_block[1].D2D_RegFile.uvm_reg_map: d2d_reg_block[1].D2D_RegFile.PCS_OOR_CFG=0x1*/
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    regData = enable;
    regMask = 0x1;
    regAddr = baseAddr + D2D_PCS_PCS_OOR_CFG;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    return GT_OK;
}

/**
* @internal mvHwsD2dPcsTestGen function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number 0..63
* @param[in] configPtr                - Defines D2D PRBS.
*/
GT_STATUS mvHwsD2dPcsTestGen
(
    IN GT_U8                           devNum,
    IN GT_U32                          d2dNum,
    IN MV_HWS_D2D_PRBS_CONFIG_STC      *configPtr
)
{
    GT_U32 regAddr1,regAddr2, regData, regMask;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 d2dIndex;

    /* support device without D2D */
    HAWK_SKIP_D2D_CONFIG_MAC;

    if(configPtr == NULL)
    {
        return GT_BAD_PTR;
    }
    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        return GT_NOT_SUPPORTED;
    }

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    regAddr1 = baseAddr + D2D_PCS_PCS_CFG_ENA;
    regAddr2 = baseAddr + D2D_PMA_CFG_EN;

    /* Write PCS Global Control register */
    switch(configPtr->direction)
    {
        case HWS_D2D_DIRECTION_MODE_BOTH_E:
            regMask = 0x30003;
            if(configPtr->portPattern == TEST_GEN_Normal)
            {
                regData = 0x10001;
            }
            else
            {
                regData = 0x20002;
            }
            break;
        case HWS_D2D_DIRECTION_MODE_RX_E:
            regMask = 0x30000;
            if(configPtr->portPattern == TEST_GEN_Normal)
            {
                regData = 0x10000;
            }
            else
            {
                regData = 0x20000;
            }
            break;
        case HWS_D2D_DIRECTION_MODE_TX_E:
            regMask = 0x3;
            if(configPtr->portPattern == TEST_GEN_Normal)
            {
                regData = 0x1;
            }
            else
            {
                regData = 0x2;
            }
            break;
        default:
            return GT_BAD_PARAM;
    }

    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr1, regData, regMask));

    if((configPtr->direction == HWS_D2D_DIRECTION_MODE_BOTH_E) || (configPtr->direction == HWS_D2D_DIRECTION_MODE_RX_E))
    {
        if(configPtr->portPattern == TEST_GEN_Normal)
        {
            regData = 0x0;
        }
        else
        {
            regData = 0x1;
        }
        /* Configured PMA gearbox bypass to allow PRBS check in PCS */
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr2, 1, 1, regData));
    }

    return GT_OK;
}



/**
* @internal mvHwsD2dPcsLoopbackSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number 0..63
* @param[in] lbType                   - loopback type
*/
GT_STATUS mvHwsD2dPcsLoopbackSet
(
    IN GT_U8                    devNum,
    IN GT_U32                   d2dNum,
    IN MV_HWS_PORT_LB_TYPE      lbType
)
{
    GT_U32 regAddr, regData;
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 d2dIndex;

    /* support device without D2D */
    HAWK_SKIP_D2D_CONFIG_MAC;
    /*
        PLEASE NOTE:

            - when D2D PCS loopback is configured on Eagle side on d2dIndex 0,2,4...
            THERE IS NO ACCESS via D2D_CP to Raven - in case of such transaction PEX will get stuck.

            - when D2D PCS loopback is configured on Raven side on d2dIndex 0,2,4...
            THERE IS NO ACCESS via D2D_CP to Raven - in case of such transaction PEX will get stuck.
            Moreover in order to disable such loopback in Raven - it should be done via SMI,
            and mvHwsD2dPcsLoopbackSet currently does not support such behavior
    */

    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        return GT_NOT_SUPPORTED;
    }

    switch(lbType)
    {
        case DISABLE_LB:
            regData = 0x0;
            break;

        case TX_2_RX_LB:
            regData = 0x1;
            break;

        case RX_2_TX_LB:
        default:
            return GT_NOT_SUPPORTED;
    }

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    regAddr = baseAddr + D2D_PCS_PCS_CFG_ENA;

    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 31, 1, regData));

    return GT_OK;
}

/**
* @internal mvHwsD2dPcsLoopbackGet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number 0..63
* @param[out] lbType                  - loopback type
*/
GT_STATUS mvHwsD2dPcsLoopbackGet
(
    IN GT_U8                    devNum,
    IN GT_U32                   d2dNum,
    OUT MV_HWS_PORT_LB_TYPE     *lbType
)
{
    GT_U32 regAddr, regData;
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 d2dIndex;

    /* support device without D2D */
    HAWK_SKIP_D2D_CONFIG_MAC;

    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        return GT_NOT_SUPPORTED;
    }

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    regAddr = baseAddr + D2D_PCS_PCS_CFG_ENA;

    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 31, 1, &regData));

    *lbType = (regData == 0) ? DISABLE_LB : TX_2_RX_LB;

    return GT_OK;
}

/**
* @internal mvHwsD2dPcsErrorInjectionSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number 0..63
* @param[in] laneIdx                  - lane 0..3
* @param[in] errorNum                 - number of errors to inject
*/
GT_STATUS mvHwsD2dPcsErrorInjectionSet
(
    IN GT_U8        devNum,
    IN GT_U32       d2dNum,
    IN GT_U32       laneIdx,  /* lane 0..3 */
    IN GT_U32       errorNum    /* 1..0x7FF, 0x7FF enables continuously injection */
)
{
    GT_U32 regAddr1,regAddr2, regData1, regData2;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 d2dIndex;

    /* support device without D2D */
    HAWK_SKIP_D2D_CONFIG_MAC;

    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        return GT_NOT_SUPPORTED;
    }

    switch(errorNum)
    {
        case 0:
            regData1 = 0;
            regData2 = 0;
            break;

        case 1:
            regData1 = (0xF << 8);
            regData2 = 1;
            break;

        case 2:
            return GT_NOT_SUPPORTED;

        case 0x7FF:
            regData1 = (0xF << 8) | (0x7FF << 20);
            regData2 = 1;
            break;

        default:
            regData1 = (0xF << 8) | ((errorNum - 2) << 20);
            regData2 = 1;
            break;
    }

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }

    regAddr1 = baseAddr + D2D_PCS_DEBUG_ERROR_INJECT;
    regAddr2 = baseAddr + D2D_PCS_DBG_LANE_CTRL + laneIdx * 0x4;

    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr1, regData1, 0x7FFFFF00));

    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr2, 6, 2, regData2));

    return GT_OK;
}
#ifndef FALCON_DEV_SUPPORT
/**
* @internal mvHwsD2dPcsFecModeSet function
* @endinternal
 *
* @param[in] driverPtr                - driver to write with smi
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number
* @param[in] direction                - direction
* @param[in] fecMode                  - FEC mode
*/
GT_STATUS mvHwsD2dPcsFecModeSet
(
    IN CPSS_HW_DRIVER_STC               *driverPtr,
    IN  GT_U8                           devNum,
    IN  GT_U32                          d2dNum,
    IN  MV_HWS_D2D_DIRECTION_MODE_ENT   direction,
    IN  MV_HWS_PORT_FEC_MODE            fecMode
)
{
    GT_U32 regAddr1, regAddr2, regData;
    GT_U32 baseAddr = 0xFFFFFFFF, unitNum, unitIndex;
    GT_U32 regMask;     /* register mask */
    GT_U32 d2dIndex;

    /* support device without D2D */
    HAWK_SKIP_D2D_CONFIG_MAC;

    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        return GT_NOT_SUPPORTED;
    }

    switch(fecMode)
    {
        case RS_FEC:
            regData = 1;
            break;
        case FEC_OFF:
            regData = 0;
            break;
        case FC_FEC:
        default:
            return GT_BAD_PARAM;
    }

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    regAddr1 = baseAddr + D2D_PMA_CFG_EN;
    regAddr2 = baseAddr + D2D_PCS_PCS_TX_RATE_CTRL;

    switch(direction)
    {
        case HWS_D2D_DIRECTION_MODE_TX_E:
            regData = regData << 5;
            regMask = 0x20;
            break;

        case HWS_D2D_DIRECTION_MODE_RX_E:
            regData = regData << 4;
            regMask = 0x10;
            break;

        case HWS_D2D_DIRECTION_MODE_BOTH_E:
            regData = (regData << 5) | (regData << 4);
            regMask = 0x30;
            break;

        default:
            return GT_BAD_PARAM;
    }
    HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr1, regData, regMask);

    /* Write to: PCS Transmit Rate Control */
    /* PHY clock period in ps = 1689 (0x699) with FEC, 1583 (0x62F) without FEC */
    regData = (fecMode == RS_FEC) ? (0x699 << 11) : (0x62F << 11);
    regMask = 0x3FF800; /* phy_clk_period_ps {21:11} */
    HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr2, regData, regMask);

#if 0
    relevant only for PRBS traffic


    /* Write to: PAT_PSEL_LANE [4] (FEC) */
    regData = (fecMode == RS_FEC) ? 1 : 0;
    for(serdesNum = 0; serdesNum < 4; serdesNum++)
    {
        for(sublane = 0; sublane < 5; sublane++)
        {
            if(regAddr3 != 0xFFFFFFFF)
            {
                regAddr1 = regAddr3 + 0x80*sublane + 0x1000*serdesNum;
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr1, 4, 1, regData));
            }
            if(regAddr4 != 0xFFFFFFFF)
            {
                regAddr1 = regAddr4 + 0x80*sublane + 0x1000*serdesNum;
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr1, 4, 1, regData));
            }
        }
    }
#endif

    return GT_OK;
}
#endif
/**
* @internal mvHwsD2dPcsErrorCounterGet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number
* @param[in] laneIdx                  - lane index
* @param[out] errorCntPtr             - (pointer to) number of errors counter
*/
GT_STATUS mvHwsD2dPcsErrorCounterGet
(
    IN GT_U8            devNum,
    IN GT_U32           d2dNum,
    IN GT_U32           laneIdx,  /* lane 0..3 */
    OUT GT_U32          *errorCntPtr
)
{
    GT_U32 regAddr, regData;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 d2dIndex;

    /* support device without D2D */
    HAWK_SKIP_D2D_CONFIG_MAC;

    if(errorCntPtr == NULL)
    {
        return GT_BAD_PTR;
    }
    if(laneIdx > 3)
    {
        return GT_BAD_PARAM;
    }
    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        return GT_NOT_SUPPORTED;
    }

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }

    regAddr = baseAddr + D2D_PCS_AM_BER_CNT_LANE  + laneIdx * 0x4;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0));

    *errorCntPtr = regData;

    return GT_OK;
}

/**
* @internal mvHwsD2dPcsFecCounterGet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number
* @param[in] laneIdx                  - lane index
* @param[out] correctedErrorCntPtr    - (pointer to) Count number 160b words corrected by FEC
* @param[out] uncorrectedErrorCntPtr  - (pointer to) Count number 160b words with Uncorrectable error
*/
GT_STATUS mvHwsD2dPcsFecCounterGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  d2dNum,
    IN  GT_U32                  laneIdx,    /* lane 0..3 */
    OUT GT_U32                  *correctedErrorCntPtr,
    OUT GT_U32                  *uncorrectedErrorCntPtr
)
{
    GT_U32 regAddr1, regAddr2, regData;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 d2dIndex;

    /* support device without D2D */
    HAWK_SKIP_D2D_CONFIG_MAC;

    if((correctedErrorCntPtr == NULL) || (uncorrectedErrorCntPtr == NULL))
    {
        return GT_BAD_PTR;
    }
    if(laneIdx > 3)
    {
        return GT_BAD_PARAM;
    }
    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        return GT_NOT_SUPPORTED;
    }

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }

    regAddr1 = baseAddr + D2D_PMA_PMA_FEC_COR_CNT  + laneIdx * 0x4;
    regAddr2 = baseAddr + D2D_PMA_PMA_FEC_UNCOR_CNT  + laneIdx * 0x4;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr1, &regData, 0));
    *correctedErrorCntPtr = regData;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr2, &regData, 0));
    *uncorrectedErrorCntPtr = regData;

    return GT_OK;
}
#ifndef FALCON_DEV_SUPPORT

/**
* @internal mvHwsD2dPcsInit_Z2 function
* @endinternal
 *
* @param[in] driverPtr                - driver to write with smi
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
*/
GT_STATUS mvHwsD2dPcsInit_Z2
(
    IN CPSS_HW_DRIVER_STC  *driverPtr,
    IN GT_U8               devNum,
    IN GT_U32              d2dNum

)
{
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regMask;
    GT_U32 d2dIndex;
    GT_U32 rowIdx;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 d2dNumIter, d2dStart, d2dLast;

    /* support device without D2D */
    HAWK_SKIP_D2D_CONFIG_MAC;

    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        for(d2dIndex = 0; d2dIndex < 2; d2dIndex++)
        {
            for(rowIdx = 0; rowIdx < 32; rowIdx++)
            {
                /* Write to: PCS Transmit Calendar Slot Configuration %n */
                regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, (D2D_PCS_PCS_TX_CAL_BASE + 4*rowIdx));
                regData = 0x0008103E;
                regMask = 0x00FFFFFF;
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

                /* Write to: PCS Receive Calendar Slot Configuration %n */
                regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, (D2D_PCS_PCS_RX_CAL_BASE + 4*rowIdx));
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
            }

            /* Write to: PCS Transmit Calendar Control */
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_TX_CAL_CTRL);
            regData = 0x0040007F;
            regMask = 0xFFFFFFFF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Write to: PCS Receive Calendar Control */
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_RX_CAL_CTRL);
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Write to: PCS Transmit Rate Control */
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_TX_RATE_CTRL);
            regData = (6 << 26) | (0xA << 22) | (0x62F << 11) | (0x5DC);
            regMask = 0xFFFFFFFF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Write to: PCS Global Control */
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_CFG_ENA);
            regData = 0x00050005;
            regMask = 0xFFFFFFFF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
        }
    }
    else
    {   /* falcon */


        if (d2dNum == HWS_D2D_ALL)
        {
            d2dStart = 0;
            d2dLast = hwsFalconNumOfRavens * 4;
        }
        else
        {
            d2dStart = d2dLast = d2dNum;
        }

        for (d2dNumIter = d2dStart; d2dNumIter <= d2dLast; d2dNumIter++)
        {
            d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNumIter);
            if (!(hwsRavenDevBmp & (1<<(d2dIndex/2))))
            {
                continue;
            }

            /*************************************************************************/
            /* Raven/Eagle D2D operation mode is - RS-FEC and it's enabled by default*/
            /* no need to call mvHwsD2dPcsFecModeSet()                               */
            /*************************************************************************/

            if (!PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNumIter))
            {
                /*Raven, done through SMI:
                Wrote register via map d2d_reg_block[1].D2D_RegFile.uvm_reg_map: d2d_reg_block[1].D2D_RegFile.PCS_CFG_ENA=0xd003d*/
                CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum /* localUnitNumPtr */));


                /* Write to: PCS Global Control */
                regData = 0x000d003d;
                regMask = 0xFFFFFFFF;
                regAddr = baseAddr + D2D_PCS_PCS_CFG_ENA;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);

                /* JIRA CPSS-8688 - fix sizes of both calendars to 84 */
                regData = 0x00000000 | (HWS_D2D_PCS_CAL_LEN_CNS - 1);
                regMask = 0x000001FF;

                /* Write to: PCS Transmit Calendar Control */
                regAddr = baseAddr + D2D_PCS_PCS_TX_CAL_CTRL;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);

                /* Write to: PCS Receive Calendar Control */
                regAddr = baseAddr + D2D_PCS_PCS_RX_CAL_CTRL;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
            }
            else
            { /* eagle, done with PEX */
              /*  Wrote register via map d2d_reg_block[0].RXC_RegFile.uvm_reg_map: d2d_reg_block[0].RXC_RegFile.autostart=0xf
                  Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_CFG_ENA=0xd003d
                 mmap.D2D_RegFile.PCS_RX_IFC_REMAP_8.PCS_RX_IFC_REMAP_OFFSET_8.set( 8 );
                 CP Interface
                mmap.D2D_RegFile.PCS_RX_IFC_REMAP_9.PCS_RX_IFC_REMAP_OFFSET_9.set( 8 );
                TX remap
                 CPU port
                mmap.D2D_RegFile.PCS_RX_IFC_REMAP_16.PCS_TX_IFC_REMAP_OFFSET_16.set( 24 );
                 CP Interface
                mmap.D2D_RegFile.PCS_RX_IFC_REMAP_17.PCS_TX_IFC_REMAP_OFFSET_17.set( 24 );*/

                CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum /* localUnitNumPtr */));

                /* Write to: PCS Global Control */
                regData = 0x000d003d;
                regMask = 0xFFFFFFFF;
                regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PCS_PCS_CFG_ENA;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);

                /*Write PCS_RX_IFC_REMAP_8,9=0x8*/
                regData = 0x08;
                regMask = 0x1F;
                regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PCS_RX_IFC_REMAP_8;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
                regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PCS_RX_IFC_REMAP_9;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
                /*Write PCS_TX_IFC_REMAP_16,17=0x1800*/
                regData = 0x1800;
                regMask = 0x1F00;
                regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PCS_RX_IFC_REMAP_16;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
                regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PCS_RX_IFC_REMAP_17;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);

                /* JIRA fix "SWD2D-187" */
                regData = regMask = (1 << 29);
                regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PMA_METAL_FIX;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);

                /* JIRA CPSS-8688 - fix sizes of both calendars to 84 */
                regData = 0x00000000 | (HWS_D2D_PCS_CAL_LEN_CNS - 1);
                regMask = 0x000001FF;

                /* Write to: PCS Transmit Calendar Control */
                regAddr = baseAddr + D2D_PCS_PCS_TX_CAL_CTRL;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);

                /* Write to: PCS Receive Calendar Control */
                regAddr = baseAddr + D2D_PCS_PCS_RX_CAL_CTRL;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
          }
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsD2dPcsInitPart1 function
* @endinternal
 *
* @param[in] driverPtr                - driver to write with smi
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
*/
GT_STATUS mvHwsD2dPcsInitPart1
(
    IN CPSS_HW_DRIVER_STC  *driverPtr,
    IN GT_U8               devNum,
    IN GT_U32              d2dNum

)
{
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regMask;
    GT_U32 d2dIndex;
    GT_U32 rowIdx;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 d2dNumIter, d2dStart, d2dLast;

    /* support device without D2D */
    HAWK_SKIP_D2D_CONFIG_MAC;

    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        for(d2dIndex = 0; d2dIndex < 2; d2dIndex++)
        {
            for(rowIdx = 0; rowIdx < 32; rowIdx++)
            {
                /* Write to: PCS Transmit Calendar Slot Configuration %n */
                regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, (D2D_PCS_PCS_TX_CAL_BASE + 4*rowIdx));
                regData = 0x0008103E;
                regMask = 0x00FFFFFF;
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

                /* Write to: PCS Receive Calendar Slot Configuration %n */
                regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, (D2D_PCS_PCS_RX_CAL_BASE + 4*rowIdx));
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
            }

            /* Write to: PCS Transmit Calendar Control */
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_TX_CAL_CTRL);
            regData = 0x0040007F;
            regMask = 0xFFFFFFFF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Write to: PCS Receive Calendar Control */
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_RX_CAL_CTRL);
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Write to: PCS Transmit Rate Control */
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_TX_RATE_CTRL);
            regData = (6 << 26) | (0xA << 22) | (0x62F << 11) | (0x5DC);
            regMask = 0xFFFFFFFF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Write to: PCS Global Control */
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_CFG_ENA);
            regData = 0x00050005;
            regMask = 0xFFFFFFFF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
        }
    }
    else
    {   /* falcon */


        if (d2dNum == HWS_D2D_ALL)
        {
            d2dStart = 0;
            d2dLast = hwsFalconNumOfRavens * 4;
        }
        else
        {
            d2dStart = d2dLast = d2dNum;
        }

        for (d2dNumIter = d2dStart; d2dNumIter <= d2dLast; d2dNumIter++)
        {
            d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNumIter);
            if (!(hwsRavenDevBmp & (1<<(d2dIndex/2))))
            {
                continue;
            }

            /*************************************************************************/
            /* Raven/Eagle D2D operation mode is - RS-FEC and it's enabled by default*/
            /* no need to call mvHwsD2dPcsFecModeSet()                               */
            /*************************************************************************/

            if (!PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNumIter))
            {
                /*Raven, done through SMI:
                Wrote register via map d2d_reg_block[1].D2D_RegFile.uvm_reg_map: d2d_reg_block[1].D2D_RegFile.PCS_CFG_ENA=0xd003d*/
                CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum /* localUnitNumPtr */));

                /* JIRA CPSS-8688 - fix sizes of both calendars to 84 */
                regData = 0x00000000 | (HWS_D2D_PCS_CAL_LEN_CNS - 1);
                regMask = 0x000001FF;

                /* Write to: PCS Transmit Calendar Control */
                regAddr = baseAddr + D2D_PCS_PCS_TX_CAL_CTRL;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);

                /* Write to: PCS Receive Calendar Control */
                regAddr = baseAddr + D2D_PCS_PCS_RX_CAL_CTRL;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
            }
            else
            { /* eagle, done with PEX */
              /*  Wrote register via map d2d_reg_block[0].RXC_RegFile.uvm_reg_map: d2d_reg_block[0].RXC_RegFile.autostart=0xf
                  Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_CFG_ENA=0xd003d
                 mmap.D2D_RegFile.PCS_RX_IFC_REMAP_8.PCS_RX_IFC_REMAP_OFFSET_8.set( 8 );
                 CP Interface
                mmap.D2D_RegFile.PCS_RX_IFC_REMAP_9.PCS_RX_IFC_REMAP_OFFSET_9.set( 8 );
                TX remap
                 CPU port
                mmap.D2D_RegFile.PCS_RX_IFC_REMAP_16.PCS_TX_IFC_REMAP_OFFSET_16.set( 24 );
                 CP Interface
                mmap.D2D_RegFile.PCS_RX_IFC_REMAP_17.PCS_TX_IFC_REMAP_OFFSET_17.set( 24 );*/

                CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum /* localUnitNumPtr */));

                /*Write PCS_RX_IFC_REMAP_8,9=0x8*/
                regData = 0x08;
                regMask = 0x1F;
                regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PCS_RX_IFC_REMAP_8;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
                regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PCS_RX_IFC_REMAP_9;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
                /*Write PCS_TX_IFC_REMAP_16,17=0x1800*/
                regData = 0x1800;
                regMask = 0x1F00;
                regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PCS_RX_IFC_REMAP_16;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
                regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PCS_RX_IFC_REMAP_17;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);

                /* JIRA fix "SWD2D-187" */
                regData = regMask = (1 << 29);
                regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PMA_METAL_FIX;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);

                /* JIRA CPSS-8688 - fix sizes of both calendars to 84 */
                regData = 0x00000000 | (HWS_D2D_PCS_CAL_LEN_CNS - 1);
                regMask = 0x000001FF;

                /* Write to: PCS Transmit Calendar Control */
                regAddr = baseAddr + D2D_PCS_PCS_TX_CAL_CTRL;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);

                /* Write to: PCS Receive Calendar Control */
                regAddr = baseAddr + D2D_PCS_PCS_RX_CAL_CTRL;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
          }
        }
    }
    return GT_OK;
}

GT_STATUS mvHwsD2dPcsInitPart2
(
    IN CPSS_HW_DRIVER_STC  *driverPtr,
    IN GT_U8               devNum,
    IN GT_U32              d2dNum

)
{
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regMask;
    GT_U32 d2dIndex;
    GT_U32 rowIdx;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 d2dNumIter, d2dStart, d2dLast;

    /* support device without D2D */
    HAWK_SKIP_D2D_CONFIG_MAC;

    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        for(d2dIndex = 0; d2dIndex < 2; d2dIndex++)
        {
            for(rowIdx = 0; rowIdx < 32; rowIdx++)
            {
                /* Write to: PCS Transmit Calendar Slot Configuration %n */
                regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, (D2D_PCS_PCS_TX_CAL_BASE + 4*rowIdx));
                regData = 0x0008103E;
                regMask = 0x00FFFFFF;
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

                /* Write to: PCS Receive Calendar Slot Configuration %n */
                regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, (D2D_PCS_PCS_RX_CAL_BASE + 4*rowIdx));
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
            }

            /* Write to: PCS Transmit Calendar Control */
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_TX_CAL_CTRL);
            regData = 0x0040007F;
            regMask = 0xFFFFFFFF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Write to: PCS Receive Calendar Control */
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_RX_CAL_CTRL);
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Write to: PCS Transmit Rate Control */
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_TX_RATE_CTRL);
            regData = (6 << 26) | (0xA << 22) | (0x62F << 11) | (0x5DC);
            regMask = 0xFFFFFFFF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Write to: PCS Global Control */
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_CFG_ENA);
            regData = 0x00050005;
            regMask = 0xFFFFFFFF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
        }
    }
    else
    {   /* falcon */


        if (d2dNum == HWS_D2D_ALL)
        {
            d2dStart = 0;
            d2dLast = hwsFalconNumOfRavens * 4;
        }
        else
        {
            d2dStart = d2dLast = d2dNum;
        }

        for (d2dNumIter = d2dStart; d2dNumIter <= d2dLast; d2dNumIter++)
        {
            d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNumIter);
            if (!(hwsRavenDevBmp & (1<<(d2dIndex/2))))
            {
                continue;
            }

            /*************************************************************************/
            /* Raven/Eagle D2D operation mode is - RS-FEC and it's enabled by default*/
            /* no need to call mvHwsD2dPcsFecModeSet()                               */
            /*************************************************************************/

            if (!PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNumIter))
            {
                /*Raven, done through SMI:
                Wrote register via map d2d_reg_block[1].D2D_RegFile.uvm_reg_map: d2d_reg_block[1].D2D_RegFile.PCS_CFG_ENA=0xd003d*/
                CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum /* localUnitNumPtr */));


                /* Write to: PCS Global Control */
                regData = 0x000d003d;
                regMask = 0xFFFFFFFF;
                regAddr = baseAddr + D2D_PCS_PCS_CFG_ENA;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
            }
            else
            { /* eagle, done with PEX */
              /*  Wrote register via map d2d_reg_block[0].RXC_RegFile.uvm_reg_map: d2d_reg_block[0].RXC_RegFile.autostart=0xf
                  Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_CFG_ENA=0xd003d
                 mmap.D2D_RegFile.PCS_RX_IFC_REMAP_8.PCS_RX_IFC_REMAP_OFFSET_8.set( 8 );
                 CP Interface
                mmap.D2D_RegFile.PCS_RX_IFC_REMAP_9.PCS_RX_IFC_REMAP_OFFSET_9.set( 8 );
                TX remap
                 CPU port
                mmap.D2D_RegFile.PCS_RX_IFC_REMAP_16.PCS_TX_IFC_REMAP_OFFSET_16.set( 24 );
                 CP Interface
                mmap.D2D_RegFile.PCS_RX_IFC_REMAP_17.PCS_TX_IFC_REMAP_OFFSET_17.set( 24 );*/

                CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum /* localUnitNumPtr */));

                /* Write to: PCS Global Control */
                regData = 0x000d003d;
                regMask = 0xFFFFFFFF;
                regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PCS_PCS_CFG_ENA;
                HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
          }
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsD2dPcsCalendarLengthSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                - portGroup
* @param[in] pcsNum                   - port number
* @param[in] length                   - calendar length
*                                           (APPLICABLE RANGES: 1..512)
*/
GT_STATUS mvHwsD2dPcsCalendarLengthSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          pcsNum,
    MV_HWS_D2D_DIRECTION_MODE_ENT   direction,
    GT_U32                          length
)
{
    GT_U32 regAddr1 = 0xFFFFFFFF;
    GT_U32 regAddr2 = 0xFFFFFFFF;
    GT_U32 d2dIndex;
    portGroup = portGroup;

    /* support device without D2D */
    HAWK_SKIP_D2D_CONFIG_MAC;

    d2dIndex = PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(devNum,pcsNum);

    if((length == 0) || (length > 0x200))
    {
        return GT_OUT_OF_RANGE;
    }

    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        if((direction == HWS_D2D_DIRECTION_MODE_BOTH_E) || (direction == HWS_D2D_DIRECTION_MODE_TX_E))
        {
            regAddr1 = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_TX_CAL_CTRL);
        }
        if((direction == HWS_D2D_DIRECTION_MODE_BOTH_E) || (direction == HWS_D2D_DIRECTION_MODE_RX_E))
        {
            regAddr2 = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_RX_CAL_CTRL);
        }
    }
    else /*falcon*/
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    if(regAddr1 != 0xFFFFFFFF)
    {
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr1, 0, 9, (length - 1)));
    }
    if(regAddr2 != 0xFFFFFFFF)
    {
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr2, 0, 9, (length - 1)));
    }

    return GT_OK;
}

/**
* @internal mvHwsD2dPcsCalendarLengthGet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                - portGroup
* @param[in] pcsNum                   - port number
* @param[out] length                  - (pointer to) calendar length
*/
GT_STATUS mvHwsD2dPcsCalendarLengthGet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          pcsNum,
    MV_HWS_D2D_DIRECTION_MODE_ENT   direction,
    GT_U32                          *lengthPtr
)
{
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 d2dIndex;
    portGroup = portGroup;

    /* support device without D2D */
    HAWK_SKIP_D2D_CONFIG_MAC;

    if(lengthPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    d2dIndex = PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(devNum,pcsNum);

    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        if(direction == HWS_D2D_DIRECTION_MODE_TX_E)
        {
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_TX_CAL_CTRL);
        }
        else if(direction == HWS_D2D_DIRECTION_MODE_RX_E)
        {
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_RX_CAL_CTRL);
        }
        else
        {
            return GT_BAD_PARAM;
        }

    }
    else /*falcon*/
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 9, &regData));

    *lengthPtr = regData + 1;

    return GT_OK;
}

/**
* @internal mvHwsD2dPcsCalendarSlotConfigSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                - portGroup
* @param[in] pcsNum                   - port number
* @param[in] slotNum                  - slot index
*                                           (APPLICABLE RANGES: 0..127)
* @param[in] chNum                    - channel number
*                                           (APPLICABLE RANGES: 0..63)
*/
GT_STATUS mvHwsD2dPcsCalendarSlotConfigSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          pcsNum,
    MV_HWS_D2D_DIRECTION_MODE_ENT   direction,
    GT_U32                          slotNum,
    GT_U32                          chNum
)
{
    GT_U32 regAddr1 = 0xFFFFFFFF;
    GT_U32 regAddr2 = 0xFFFFFFFF;
    GT_U32 rowIdx, fieldOffset;
    GT_U32 d2dIndex;
    portGroup = portGroup;

    /* support device without D2D */
    HAWK_SKIP_D2D_CONFIG_MAC;

    d2dIndex = PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(devNum,pcsNum);
    rowIdx = slotNum / 4;
    fieldOffset = 6 * (slotNum % 4);

    if((slotNum >= 0x80) || (chNum >= 0x40))
    {
        return GT_OUT_OF_RANGE;
    }

    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        /* Write to: PCS Transmit Calendar Slot Configuration %n */
        if((direction == HWS_D2D_DIRECTION_MODE_BOTH_E) || (direction == HWS_D2D_DIRECTION_MODE_TX_E))
        {
            regAddr1 = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, (D2D_PCS_PCS_TX_CAL_BASE + 4*rowIdx));
        }
        if((direction == HWS_D2D_DIRECTION_MODE_BOTH_E) || (direction == HWS_D2D_DIRECTION_MODE_RX_E))
        {
            regAddr2 = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, (D2D_PCS_PCS_RX_CAL_BASE + 4*rowIdx));
        }
    }
    else
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    if(regAddr1 != 0xFFFFFFFF)
    {
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr1, fieldOffset, 6, chNum));
    }
    if(regAddr2 != 0xFFFFFFFF)
    {
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr2, fieldOffset, 6, chNum));
    }

    return GT_OK;
}


/**
* @internal mvHwsD2dPcsCalendarSlotConfigGet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                - portGroup
* @param[in] pcsNum                   - port number
* @param[in] slotNum                  - slot index
*                                           (APPLICABLE RANGES: 0..127)
* @param[out] chNumPtr                - (pointer to) channel number
*/
GT_STATUS mvHwsD2dPcsCalendarSlotConfigGet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          pcsNum,
    MV_HWS_D2D_DIRECTION_MODE_ENT   direction,
    GT_U32                          slotNum,
    GT_U32                          *chNumPtr
)
{
    GT_U32 regAddr, rowIdx, fieldOffset;
    GT_U32 d2dIndex;
    portGroup = portGroup;

    /* support device without D2D */
    HAWK_SKIP_D2D_CONFIG_MAC;

    d2dIndex = PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(devNum,pcsNum);
    rowIdx = slotNum / 4;
    fieldOffset = 6 * (slotNum % 4);

    if(slotNum >= 0x80)
    {
        return GT_OUT_OF_RANGE;
    }
    if(chNumPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        if(direction == HWS_D2D_DIRECTION_MODE_TX_E)
        {
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, (D2D_PCS_PCS_TX_CAL_BASE + 4*rowIdx));
        }
        else if(direction == HWS_D2D_DIRECTION_MODE_RX_E)
        {
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, (D2D_PCS_PCS_RX_CAL_BASE + 4*rowIdx));
        }
        else
        {
            return GT_BAD_PARAM;
        }
    }
    else
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, fieldOffset, 6, chNumPtr));

    return GT_OK;
}
#endif

/**
* @internal mvHwsD2dPcsIfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
*/
GT_STATUS mvHwsD2dPcsIfInit(GT_U8 devNum, MV_HWS_PCS_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[D2D_PCS])
    {
        funcPtrArray[D2D_PCS] = (MV_HWS_PCS_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_PCS_FUNC_PTRS));
        if(!funcPtrArray[D2D_PCS])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[D2D_PCS], 0, sizeof(MV_HWS_PCS_FUNC_PTRS));
    }
    funcPtrArray[D2D_PCS]->pcsTypeGetFunc   = mvHwsD2dPcsTypeGetFunc;

    hwsInitDone[devNum] = GT_TRUE;

    return GT_OK;
}

/**
* @internal hwsD2dPcsIfClose function
* @endinternal
 *
* @param[in] devNum                   - system device number
*/
void hwsD2dPcsIfClose
(
    IN GT_U8      devNum
)
{
    hwsInitDone[devNum] = GT_FALSE;
}


