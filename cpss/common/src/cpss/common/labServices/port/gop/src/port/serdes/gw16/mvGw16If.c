/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* mvGw16If.c
*
* DESCRIPTION:
*       GW16nm SERDES configuration
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef MV_HWS_REDUCED_BUILD
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/gw16/mvGw16If.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralCpll.h>
#ifndef MV_HWS_REDUCED_BUILD
#include <cpssCommon/private/prvCpssEmulatorMode.h>
#endif

#ifdef CHX_FAMILY
extern  GT_U32 hwsFalconNumOfRavens;
extern  GT_BOOL hwsPpHwTraceFlag;
#endif


#ifndef CO_CPU_RUN
static char* mvHwsGw16SerdesTypeGetFunc(void)
{
  return "GW16";
}
#endif

#define RAVEN_LANE_OFFSET_CNS           0x400
/************************* Globals *******************************************************/

static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};

/************************* pre-declaration ***********************************************/

/*****************************************************************************************/
#ifndef FALCON_DEV_SUPPORT
static GT_STATUS mvHwsGW16SerdesOverridesApply
(
    CPSS_HW_DRIVER_STC *driverPtr,
    GT_U8 devNum,
    GT_U32 d2dNum
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    d2dNum = d2dNum;
    driverPtr = driverPtr;
#else
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regMask;
    GT_U32 lane;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 d2dIndex;
    GT_U32 ctleValue;

    if (hwsDeviceSpecInfo[devNum].devType == Falcon)
    {
        d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
        if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum) == GT_FALSE)
        {
            /* Raven, done through SMI */
            CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum));
            ctleValue = 4;
        }
        else
        {
            /* Eagle, done with PEX */
            CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum));
            ctleValue = 1;
        }
    }
    else
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    /* The PMA layer is released fom TX/RX reset with the following sequence */
    regAddr = baseAddr + D2D_PMA_CFG_RESET;
    regData = 0x0;
    regMask = 0xFF;
    HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
    regData = 0xFF;
    HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);

    /* trim RX regulator = 875mV */
    regAddr = baseAddr + D2D_PHY_RXC_PLL_REG_LEVEL;
    HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, 1, 0x7);

    /* ("txc_pllcpctrl", 0x12)  # pcp=2, pcphalf=1 TX PLL setting for BW~=5MHz */
    regAddr = baseAddr + D2D_PHY_TXC_PLL_CP_CTRL;
    HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, 0x12, 0x1F);

    /* ("txc_pllloopctrl", 14)  # Set the optimal TX PLL damping resitor value for frequency */
    regAddr = baseAddr + D2D_PHY_TXC_PLL_LOOP_CTRL;
    HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, 14, 0x3F);

    regAddr = baseAddr + D2D_PHY_TXC_SLCTHRES0;
    HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, 0x1F, 0x1F);

    for(lane = 0; lane < 4; lane++)
    {
        /* CTLE 'ib' setting is being reduced from 12 to 9 */
        regAddr = baseAddr + D2D_PHY_RX_CTLE_CTRL + lane * RAVEN_LANE_OFFSET_CNS;
        HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, (12<<1), 0x1E);

        regAddr = baseAddr + D2D_PHY_RX_CENTERLEVEL + lane * RAVEN_LANE_OFFSET_CNS;
        HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, 0x10, 0xFF);

        /* RX CTLE settings per SERDES */
        regAddr = baseAddr + D2D_PHY_RX_CTLE_EQ + lane * RAVEN_LANE_OFFSET_CNS;
        HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, ctleValue, 0xF);

        regAddr = baseAddr + D2D_PHY_RX_CDA_PI_KP + lane * RAVEN_LANE_OFFSET_CNS;
        HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, 0xF, 0xF);
    }

    regAddr = baseAddr + 0x3fc0;
    HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, 0x8e, 0xFF);

#endif
    return GT_OK;
}

/**
* @internal mvHwsGW16SerdesAutoStartInit function
* @endinternal
*
* @param[in] devNum                   - The Pp's device number
*/
GT_STATUS mvHwsGW16SerdesAutoStartInit
(
    CPSS_HW_DRIVER_STC *driverPtr,
    GT_U8 devNum,
    GT_U32 d2dNum
)
{
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regMask;
    GT_U32 d2dIndex;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_STATUS rc;

    if (hwsDeviceSpecInfo[devNum].devType == Falcon)
    {
        rc = mvHwsGW16SerdesOverridesApply(driverPtr,devNum,d2dNum);
        if(rc != GT_OK)
        {
            hwsOsPrintf("mvHwsGW16SerdesOverridesApply failed: devNum = %d, d2dNumm = %d, rc = %d\n", devNum, d2dNum, rc);
            return rc;
        }

        d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);

        if (!PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
        {
            if(d2dIndex %2 == 0) /* init CPLL only once for current Raven (with two D2Ds) */
            {
                /* Raven CPLL initialization */
                rc = mvHwsCpllControl(driverPtr, devNum, CPLL0, GT_FALSE, MV_HWS_156MHz_IN, MV_HWS_156MHz_OUT);
                if(rc != GT_OK)
                {
                    hwsOsPrintf("mvHwsCpllControl for Raven failed: devNum = %d, rc = %d\n", devNum, rc);
                    return rc;
                }
            }

            /*Raven, done through SMI:
            Wrote register via map d2d_reg_block[1].RXC_RegFile.uvm_reg_map: d2d_reg_block[1].RXC_RegFile.autostart=0xf
            Wrote register via map d2d_reg_block[1].D2D_RegFile.uvm_reg_map: d2d_reg_block[1].D2D_RegFile.PCS_CFG_ENA=0xd003d*/
            CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum /* localUnitNumPtr */));
            regData = 0xF;
            regMask = 0xF;
            regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PHY_RXC_AUTOSTART;
            HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
        }
        else
        { /* eagle, done with PEX */
            /*  Wrote register via map d2d_reg_block[0].RXC_RegFile.uvm_reg_map: d2d_reg_block[0].RXC_RegFile.autostart=0xf
              Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_CFG_ENA=0xd003d
              Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_IFC_REMAP_8=0x100808
              Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_IFC_REMAP_9=0x100808*/
            CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum /* localUnitNumPtr */));
            regData = 0xF;
            regMask = 0xF;
            regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PHY_RXC_AUTOSTART;
            HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
        }
    }
    else
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    return GT_OK;
}

GT_STATUS mvHwsGW16AutoStartStatusGet
(
    CPSS_HW_DRIVER_STC *driverPtr,
    GT_U8 devNum,
    GT_U32 d2dNum
)
{
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regMask;
    GT_U32 cnt;
    GT_U32 d2dIndex, laneIdx;
    GT_U32 baseAddr, unitNum, unitIndex;


    if (hwsDeviceSpecInfo[devNum].devType == Falcon)
    {
        d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
        /*hwsOsPrintf("\n\rRaven d2dIndex=%d,d2dNum=%d",d2dIndex,d2dNum);*/
        if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum) == GT_FALSE)
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum /* localUnitNumPtr */));

            cnt = 0;
            regMask = 0x60000;
            regAddr = baseAddr + D2D_PCS_PCS_RX_STATUS;
            HWS_REGISTER_READ_REG(driverPtr, devNum, regAddr, &regData, regMask);
    #ifdef ASIC_SIMULATION
                regData = 0x60000;
    #endif /*ASIC_SIMULATION*/
            while (regData != 0x60000)
            {
                if(cnt == 100)
                {
                    hwsOsPrintf("RAVEN_SMI D2D[%d] PHY init was failed, regAddr = 0x%x, link_status=0x%x\n", d2dNum, regAddr, regData);
                    return GT_FAIL;
                }
    #ifndef MV_HWS_REDUCED_BUILD
                if(cpssDeviceRunCheck_onEmulator())
                {
                    hwsOsTimerWkFuncPtr(100);
                }
                else
    #endif /*!MV_HWS_REDUCED_BUILD*/
                {
                    hwsOsTimerWkFuncPtr(100);
                }
                HWS_REGISTER_READ_REG(driverPtr, devNum, regAddr, &regData, regMask);
                cnt++;
            }

            /* reset RS_FEC Corrected and Uncorrected Error Counters */
            for(laneIdx = 0; laneIdx < 4; laneIdx++)
            {
                regAddr = baseAddr + D2D_PMA_PMA_FEC_COR_CNT  + laneIdx * 0x4;
                HWS_REGISTER_READ_REG(driverPtr, devNum, regAddr, &regData, 0);
                /*hwsOsPrintf("\n\rRaven Cofec regAddr:%x read,",regAddr);*/
                regAddr = baseAddr + D2D_PMA_PMA_FEC_UNCOR_CNT  + laneIdx * 0x4;
                HWS_REGISTER_READ_REG(driverPtr, devNum, regAddr, &regData, 0);
                /*hwsOsPrintf("Unfec regAddr:%x read.",regAddr);*/
            }
        }
    }

    return GT_OK;
}


#endif

/**
* @internal mvHwsGW16SerdesAutoStartInitStatusGet function
* @endinternal
*
* @param[in] devNum                   - The Pp's device number
*/
GT_STATUS mvHwsGW16SerdesAutoStartInitStatusGet
(
    GT_U8 devNum,
    GT_U32 d2dNum
)
{
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regMask;
    GT_U32 cnt;
    GT_U32 d2dIndex,laneIdx;
    GT_U32 baseAddr, unitIndex, unitNum;

    if (hwsDeviceSpecInfo[devNum].devType == Falcon)
    {
        /*Eagle: (pull indication is for Raven & Eagle)
        pull for bit [18:17] = 2'b11
        Read  register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_STATUS*/
        d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
        if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum /* localUnitNumPtr */));
            cnt = 0;
            regMask = 0x60000;
            regAddr = baseAddr + D2D_PCS_PCS_RX_STATUS;
            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
#ifdef ASIC_SIMULATION
            regData = 0x60000;
#endif /*ASIC_SIMULATION*/
            while (regData != 0x60000) {
                if(cnt == 20)
                {
                    hwsOsPrintf("D2D[%d][%d] PHY init was failed, link_status=0x%x\n", devNum, d2dNum, regData);
                    return GT_FAIL;
                }
#ifndef MV_HWS_REDUCED_BUILD
                if(cpssDeviceRunCheck_onEmulator())
                {
                    hwsOsTimerWkFuncPtr(100);
                }
                else
#endif /*!MV_HWS_REDUCED_BUILD*/
                {
                    hwsOsTimerWkFuncPtr(10);
                }
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
                cnt++;
            }

            /* reset RS_FEC Corrected and Uncorrected Error Counters */
            for(laneIdx = 0; laneIdx < 4; laneIdx++)
            {
                regAddr = baseAddr + D2D_PMA_PMA_FEC_COR_CNT  + laneIdx * 0x4;
                hwsRegisterGetFuncPtr(devNum, 0 ,regAddr, &regData ,0xFFFFFFFF);
                /*hwsOsPrintf("\n\rEagle Cofec regAddr:%x read,",regAddr);*/
                regAddr = baseAddr + D2D_PMA_PMA_FEC_UNCOR_CNT  + laneIdx * 0x4;
                hwsRegisterGetFuncPtr(devNum, 0 ,regAddr, &regData ,0xFFFFFFFF);
                /*hwsOsPrintf("UnFec regAddr:%x read.",regAddr);*/
            }
        }
    }
#ifndef FALCON_DEV_SUPPORT
    else
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }
#endif
    return GT_OK;
}


/**
* @internal mvHwsD2dPmaLaneTestGenStatus function
* @endinternal
*
* @brief   Read the tested pattern receive error counters and status.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] d2dNum                   - D2D number (0..63)
* @param[in] d2dLaneNum               - D2D lane number (0..3)
* @param[in] txPattern                - pattern to transmit
* @param[in] counterAccumulateMode    - counter Accumulate Mode.
*
* @param[out] status                  - Defines serdes test
*       generator results
*/
GT_STATUS mvHwsD2dPmaLaneTestGenStatus
(
    IN GT_U8                     devNum,
    IN GT_U32                    d2dNum,
    IN GT_U32                    d2dLaneNum,
    IN MV_HWS_SERDES_TX_PATTERN  txPattern,
    IN GT_BOOL                   counterAccumulateMode,
    OUT MV_HWS_SERDES_TEST_GEN_STATUS *status
)
{
    GT_U32          regAddr;
    GT_U32          regValue;
    GT_U32          d2dSubLaneNum;
    GT_U32          byteCount;
    GT_U32 d2dIndex;        /* D2D index */
    GT_U32 baseAddr, unitNum, unitIndex;

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }

    counterAccumulateMode = counterAccumulateMode;
    txPattern = txPattern;

    status->lockStatus = 1;
    status->txFramesCntr.l[0] = 0;
    status->errorsCntr = 0;

    /* Read  from: PAT_STS (Sticky pattern pass flag )  -  0x2 PASS else FAIL*/
    for(d2dSubLaneNum = 0; d2dSubLaneNum < 5; d2dSubLaneNum++)
    {
        regAddr = baseAddr + D2D_KDU_PAT_RX_STS + 0x1000*d2dLaneNum + 0x80*d2dSubLaneNum;
        CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 2, &regValue));

        if((regValue & 0x2) != 0x2)
        {
            status->lockStatus = 0;
        }

        if((regValue & 0x1) == 0x1)
        {
            /* Take error counter snapshot */
            regAddr = baseAddr + D2D_KDU_PAT_RX_CTRL_LANE + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 2, 1, 1));

            for(byteCount = 0; byteCount < 3; byteCount++)
            {
                regAddr = baseAddr + D2D_KDU_PAT_RX_PERR_CNT_LANE + 0x1000*d2dLaneNum + 0x80*d2dSubLaneNum + byteCount*4;
                CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 8, &regValue));

                status->errorsCntr += regValue;
            }

            /* Take error counter snapshot - reset to default value */
            regAddr = baseAddr + D2D_KDU_PAT_RX_CTRL_LANE + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 2, 1, 0));

            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 1));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 0));
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsD2dPmaLaneTestGenInjectErrorEnable function
* @endinternal
*
* @brief   Activates the D2D PMA error injection.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] d2dNum                   - D2D number (0..63)
* @param[in] d2dLaneNum               - D2D lane number (0..3)
* @param[in] serdesDirection          - SERDES direction
*/
GT_STATUS mvHwsD2dPmaLaneTestGenInjectErrorEnable
(
    IN GT_U8                       devNum,
    IN GT_U32                      d2dNum,
    IN GT_UOPT                     d2dLaneNum,  /* Lane index 0..3*/
    IN MV_HWS_SERDES_DIRECTION     serdesDirection
)
{
    GT_U32  regAddr;            /* register address */
    GT_U32  d2dSubLaneNum;      /* D2D sublane loop iterator */
    GT_U32  d2dIndex;           /* D2D index */
    GT_U32  baseAddr, unitNum, unitIndex;

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }

    for(d2dSubLaneNum = 0; d2dSubLaneNum < 5; d2dSubLaneNum++)
    {
        if(serdesDirection == RX_DIRECTION)
        {
            regAddr = baseAddr + D2D_KDU_PAT_RX_CTRL_LANE + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
        }
        else
        {
            regAddr = baseAddr + D2D_KDU_PAT_TX_CTRL_LANE + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
        }
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 4, 1, 1));
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 4, 1, 0));
    }

    return GT_OK;
}

/**
* @internal mvHwsD2dPmaLaneTestGeneneratorConfig function
* @endinternal
*
* @brief   Activates the D2D PMA Lane test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] d2dNum                   - D2D number (0..63)
* @param[in] d2dLaneNum               - D2D lane number (0..3)
* @param[in] configPtr                - (pointer to) PMA test generator configuration structure
*/
GT_STATUS mvHwsD2dPmaLaneTestGeneneratorConfig
(
    IN GT_U8                           devNum,
    IN GT_U32                          d2dNum,
    IN GT_U32                          d2dLaneNum,  /* Lane index 0..3*/
    IN MV_HWS_D2D_PRBS_CONFIG_STC      *configPtr
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 regData;         /* register data */
    GT_U32 regMask;         /* register mask */
    GT_U32 prbsRegData;     /* PRBS HW value */
    GT_U32 d2dSubLaneNum;   /* D2D sublane loop iterator */
    GT_U32 d2dIndex;        /* D2D index */
    GT_U32 baseAddr, unitNum, unitIndex;

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }

    if(configPtr->mode == SERDES_NORMAL)
    {
        regAddr = baseAddr + D2D_PMA_CFG_EN;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 2, 2, 0));

        /* Write to: PAT_CTRL_LANE (Clear errors =1) */
        for(d2dSubLaneNum = 0; d2dSubLaneNum < 5; d2dSubLaneNum++)
        {
            regAddr = baseAddr + D2D_KDU_PAT_TX_CTRL_LANE + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 1));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 0));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 1, 0));
        }

        return GT_OK;
    }


    switch(configPtr->serdesPattern)
    {
        case  PRBS9:
            prbsRegData = 0;
            break;
        case  PRBS15:
            prbsRegData = 1;
            break;
        case  PRBS23:
            prbsRegData = 2;
            break;
        case  PRBS31:
            prbsRegData = 3;
            break;
        case Other:
            prbsRegData = 4;
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    /* The lane tester requires - write to PMA Operation Control - PMA_TX_SEL set to PMA lane generator */
    regAddr = baseAddr + D2D_PMA_CFG_EN;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 2, 2, 2));


    /* Write to KDU - reset tx sub-lane pattern checkers simultaneously */
    regAddr = baseAddr + D2D_KDU_FIFO_LANE_CTRL + 0x1000*d2dLaneNum;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 2, 3));

    hwsOsTimerWkFuncPtr(1);

    /* Write to KDU - reset tx sub-lane pattern checkers simultaneously */
    regAddr = baseAddr + D2D_KDU_FIFO_LANE_CTRL + 0x1000*d2dLaneNum;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 2, 0));

    /* RX+TX RS-FEC enable/disable configuration */
    regData = (configPtr->fecMode == RS_FEC) ? 1 : 0;
    regAddr = baseAddr + D2D_PMA_CFG_EN;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 5, 1, regData));

    /* Write to: PAT_CTRL_LANE (Clear errors =1) */
    for(d2dSubLaneNum = 0; d2dSubLaneNum < 5; d2dSubLaneNum++)
    {
        regAddr = baseAddr + D2D_KDU_PAT_TX_CTRL_LANE + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 1));
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 0));
    }

    /******************************************************/
    /* User pattern configuration                         */
    /******************************************************/
    if(configPtr->userDefPatternEnabled == GT_TRUE)
    {
        for(d2dSubLaneNum = 0; d2dSubLaneNum < 5; d2dSubLaneNum++)
        {
            regAddr = baseAddr + D2D_KDU_PAT_TX_PUSER_PAT_LANE + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 8, configPtr->userDefPattern[0]));
            regAddr = baseAddr + D2D_KDU_PAT_TX_PUSER_PAT_LANE + 4 + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 8, configPtr->userDefPattern[1]));
            regAddr = baseAddr + D2D_KDU_PAT_TX_PUSER_PAT_LANE + 8 + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 8, configPtr->userDefPattern[2]));
            regAddr = baseAddr + D2D_KDU_PAT_TX_PUSER_PAT_LANE + 0xC + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 8, configPtr->userDefPattern[3]));
        }
    }

    /* Write to: PAT_PSEL_LANE [2:0] (select pattern) */
    for(d2dSubLaneNum = 0; d2dSubLaneNum < 5; d2dSubLaneNum++)
    {
        regAddr = baseAddr + D2D_KDU_PAT_TX_PSEL_LANE + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 3, prbsRegData));
    }

    /* Write to: PAT_PSEL_LANE [4] (FEC) */
    regData = (configPtr->fecMode == RS_FEC) ? 1 : 0;
    for(d2dSubLaneNum = 0; d2dSubLaneNum < 5; d2dSubLaneNum++)
    {
        regAddr = baseAddr + D2D_KDU_PAT_TX_PSEL_LANE + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 4, 1, regData));
    }

    switch(configPtr->direction)
    {
        case HWS_D2D_DIRECTION_MODE_BOTH_E:
            regData = 0x3;
            regMask = 0x3;
            break;
        case HWS_D2D_DIRECTION_MODE_RX_E:
            regData = 0x2;
            regMask = 0x2;
            break;
        case HWS_D2D_DIRECTION_MODE_TX_E:
            regData = 0x1;
            regMask = 0x1;
            break;
        default:
            return GT_BAD_PARAM;
    }
    /* Write to: PAT_CTRL_LANE(Enable Pattern Gen/Checker) */
    for(d2dSubLaneNum = 0; d2dSubLaneNum < 5; d2dSubLaneNum++)
    {
        regAddr = baseAddr + D2D_KDU_PAT_TX_CTRL_LANE + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    }

    /* Write to KDU - reset tx sub-lane pattern checkers simultaneously */
    regAddr = baseAddr + D2D_KDU_FIFO_LANE_CTRL + 0x1000*d2dLaneNum;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 2, 0));

    return GT_OK;
}

/**
* @internal mvHwsD2dPmaLaneTestCheckerConfig function
* @endinternal
*
* @brief   Activates the D2D PMA Lane test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] d2dNum                   - D2D number (0..63)
* @param[in] d2dLaneNum               - D2D lane number (0..3)
* @param[in] configPtr                - (pointer to) PMA test checker configuration structure
*/
GT_STATUS mvHwsD2dPmaLaneTestCheckerConfig
(
    IN GT_U8                           devNum,
    IN GT_U32                          d2dNum,
    IN GT_U32                          d2dLaneNum,  /* Lane index 0..3*/
    IN MV_HWS_D2D_PRBS_CONFIG_STC      *configPtr
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 regData;         /* register data */
    GT_U32 regMask;         /* register mask */
    GT_U32 prbsRegData;     /* PRBS HW value */
    GT_U32 d2dSubLaneNum;   /* D2D sublane loop iterator */
    GT_U32 d2dIndex;        /* D2D index */
    GT_U32 baseAddr, unitNum, unitIndex;

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }

    if(configPtr->mode == SERDES_NORMAL)
    {
        regAddr = baseAddr + D2D_PMA_CFG_EN;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 1, 0));

        /* Write to: PAT_CTRL_LANE (Clear errors =1) */
        for(d2dSubLaneNum = 0; d2dSubLaneNum < 5; d2dSubLaneNum++)
        {
            regAddr = baseAddr + D2D_KDU_PAT_RX_CTRL_LANE + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 1));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 0));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 1, 1, 0));
        }

        return GT_OK;
    }


    switch(configPtr->serdesPattern)
    {
        case  PRBS9:
            prbsRegData = 0;
            break;
        case  PRBS15:
            prbsRegData = 1;
            break;
        case  PRBS23:
            prbsRegData = 2;
            break;
        case  PRBS31:
            prbsRegData = 3;
            break;
        case Other:
            prbsRegData = 4;
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    /* The lane tester requires - write to PMA Operation Control - PMA_TX_SEL set to PMA lane generator */
    regAddr = baseAddr + D2D_PMA_CFG_EN;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 1, 1));


    /* Write to KDU - reset tx sub-lane pattern checkers simultaneously */
    regAddr = baseAddr + D2D_KDU_FIFO_LANE_CTRL + 0x1000*d2dLaneNum;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 2, 3));

    hwsOsTimerWkFuncPtr(1);

    /* Write to KDU - reset tx sub-lane pattern checkers simultaneously */
    regAddr = baseAddr + D2D_KDU_FIFO_LANE_CTRL + 0x1000*d2dLaneNum;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 2, 0));

    /* RX+TX RS-FEC enable/disable configuration */
    regData = (configPtr->fecMode == RS_FEC) ? 1 : 0;
    regAddr = baseAddr + D2D_PMA_CFG_EN;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 4, 1, regData));

    /* Write to: PAT_CTRL_LANE (Clear errors =1) */
    for(d2dSubLaneNum = 0; d2dSubLaneNum < 5; d2dSubLaneNum++)
    {
        regAddr = baseAddr + D2D_KDU_PAT_RX_CTRL_LANE + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 1));
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 0));
    }

    /******************************************************/
    /* User pattern configuration                         */
    /******************************************************/
    if(configPtr->userDefPatternEnabled == GT_TRUE)
    {
        for(d2dSubLaneNum = 0; d2dSubLaneNum < 5; d2dSubLaneNum++)
        {
            regAddr = baseAddr + D2D_KDU_PAT_RX_PUSER_PAT_LANE + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 8, configPtr->userDefPattern[0]));
            regAddr = baseAddr + D2D_KDU_PAT_RX_PUSER_PAT_LANE + 4 + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 8, configPtr->userDefPattern[1]));
            regAddr = baseAddr + D2D_KDU_PAT_RX_PUSER_PAT_LANE + 8 + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 8, configPtr->userDefPattern[2]));
            regAddr = baseAddr + D2D_KDU_PAT_RX_PUSER_PAT_LANE + 0xC + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 8, configPtr->userDefPattern[3]));
        }
    }

    /* Write to: PAT_PSEL_LANE [2:0] (select pattern) */
    for(d2dSubLaneNum = 0; d2dSubLaneNum < 5; d2dSubLaneNum++)
    {
        regAddr = baseAddr + D2D_KDU_PAT_RX_PSEL_LANE + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 3, prbsRegData));
    }

    /* Write to: PAT_PSEL_LANE [4] (FEC) */
    regData = (configPtr->fecMode == RS_FEC) ? 1 : 0;
    for(d2dSubLaneNum = 0; d2dSubLaneNum < 5; d2dSubLaneNum++)
    {
        regAddr = baseAddr + D2D_KDU_PAT_RX_PSEL_LANE + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 4, 1, regData));
    }

    switch(configPtr->direction)
    {
        case HWS_D2D_DIRECTION_MODE_BOTH_E:
            regData = 0x3;
            regMask = 0x3;
            break;
        case HWS_D2D_DIRECTION_MODE_RX_E:
            regData = 0x2;
            regMask = 0x2;
            break;
        case HWS_D2D_DIRECTION_MODE_TX_E:
            regData = 0x1;
            regMask = 0x1;
            break;
        default:
            return GT_BAD_PARAM;
    }
    /* Write to: PAT_CTRL_LANE(Enable Pattern Gen/Checker) */
    for(d2dSubLaneNum = 0; d2dSubLaneNum < 5; d2dSubLaneNum++)
    {
        regAddr = baseAddr + D2D_KDU_PAT_RX_CTRL_LANE + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    }

    /* Write to KDU - reset tx sub-lane pattern checkers simultaneously */
    regAddr = baseAddr + D2D_KDU_FIFO_LANE_CTRL + 0x1000*d2dLaneNum;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 2, 0));


    if((configPtr->serdesPattern == Other/* user defined pattern */) ||
       (configPtr->fecMode == RS_FEC))
    {
        regData = 3;
    }
    else
    {
        regData = 0;
    }
    for(d2dSubLaneNum = 0; d2dSubLaneNum < 5; d2dSubLaneNum++)
    {
        regAddr = baseAddr + ((D2D_KDU_PAT_RX_PUSER_MASK_LANE + 0xC/* byte num 3 */) + 0x80*d2dSubLaneNum + 0x1000*d2dLaneNum);
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 6, 2, regData));
    }

    return GT_OK;
}

/**
* @internal mvHwsD2dPmaLaneTestLoopbackSet function
* @endinternal
*
* @brief   Activates loopback between lane checker and lane generator.
*          To enable loopback XBAR configuration should be done + FIFO reset/unreset
*
* @param[in] devNum                   - The Pp's device number
* @param[in] d2dNum                   - D2D number (0..63)
* @param[in] lbType                   - loopback type
*/
GT_STATUS mvHwsD2dPmaLaneTestLoopbackSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  d2dNum,
    IN MV_HWS_PORT_LB_TYPE     lbType
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 regValue;        /* register value */
    GT_U32 regMask;         /* register mask */
    GT_U32 d2dSubLaneNum;   /* D2D sublane loop iterator */
    GT_U32 d2dIndex;        /* D2D index */
    GT_U32 d2dLaneNum;
    GT_U32 baseAddr, unitNum, unitIndex;

    /*
        PLEASE NOTE:

            - when D2D PMA loopback is configured on Eagle side on d2dIndex 0,2,4...
            THERE IS NO ACCESS via D2D_CP to Raven - in case of such transaction PEX will get stuck.

            - when D2D PMA loopback is configured on Raven side on d2dIndex 1,3,5...
            THERE IS NO ACCESS via D2D_CP to Raven - in case of such transaction PEX will get stuck.
            Moreover in order to disable such loopback in Raven - it should be done via SMI,
            and mvHwsD2dPcsLoopbackSet currently does not support such behavior
    */

    /* can't be loopback on cp d2dnum */
    if ((d2dNum % 4 == 1) && (lbType == RX_2_TX_LB))
    {
        return GT_NOT_SUPPORTED;
    }

    if (!((lbType == RX_2_TX_LB) || (lbType == DISABLE_LB)))
    {
        return GT_NOT_SUPPORTED;
    }

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
        /* PCS_RX_SEL - Set to 0 */
        regAddr = baseAddr + D2D_PCS_PCS_CFG_ENA;
        regMask = 0x30000;
        regValue = (lbType == RX_2_TX_LB) ? 0 : 0x10000;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regValue, regMask));

        /* PMA configuration to enable loopback */
        regMask = 0x13F;
        regAddr = baseAddr + D2D_PMA_CFG_EN;
        if(lbType == RX_2_TX_LB)
        {
            regValue = 0x38;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regValue, regMask));
            regValue = 0x13c;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regValue, regMask));
            regValue = 0x134;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regValue, regMask));
        }
        else
        {
            regValue = 0x30;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regValue, regMask));
        }
    }
    else
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum));

        /*1.PCS_RX_SEL             – Set to 0*/
        regAddr = baseAddr + D2D_PCS_PCS_CFG_ENA;
        regMask = 0x30000;
        regValue = (lbType == RX_2_TX_LB) ? 0 : 0x10000;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regValue, regMask));


        regMask = 0xD;
        regValue = (lbType == RX_2_TX_LB) ? 9 : 0;
        /* PMA configuration to enable loopback */
        regAddr = baseAddr + D2D_PMA_CFG_EN;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regValue, regMask));

        if(lbType == RX_2_TX_LB)
        {
            for(d2dLaneNum = 0; d2dLaneNum < 4; d2dLaneNum++)
            {
                /* FIFO reset/unreset configuration */
                regAddr = baseAddr + D2D_KDU_FIFO_CTRL + 0x1000 * d2dLaneNum;
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 1, 1));
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 1, 0));
            }
        }

        for(d2dLaneNum = 0; d2dLaneNum < 4; d2dLaneNum++)
        {
            for(d2dSubLaneNum = 0; d2dSubLaneNum < 5; d2dSubLaneNum++)
            {
                regAddr = baseAddr + D2D_KDU_FIFO_XBARCFG_LANE + 0x4 * d2dSubLaneNum + 0x1000 * d2dLaneNum;
                regValue = (lbType == RX_2_TX_LB) ? (d2dSubLaneNum + 1) : 0;
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 3, regValue));
            }
        }

        if(lbType == RX_2_TX_LB)
        {
            for(d2dLaneNum = 0; d2dLaneNum < 4; d2dLaneNum++)
            {
                /* FIFO reset/unreset configuration */
                regAddr = baseAddr + D2D_KDU_FIFO_CTRL + 0x1000 * d2dLaneNum;
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 1, 1));
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 1, 0));
            }
        }
    }
    return GT_OK;
}




static MV_HWS_D2D_DIRECTION_MODE_ENT hwsGw16TileTestGenMode[HWS_MAX_DEVICE_NUM] = {HWS_D2D_DIRECTION_MODE_BOTH_E};
#ifndef FALCON_DEV_SUPPORT
GT_STATUS mvHwsGw16TileTestGenModeSet
(
    GT_U8                               devNum,
    MV_HWS_D2D_DIRECTION_MODE_ENT  mode
)
{
    hwsGw16TileTestGenMode[devNum] = mode;

    return GT_OK;
}
#endif

/**
* @internal mvHwsGw16TileTestGen function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - GW16 channel index (APPLICABLE RANGES: 0..3)
* @param[in] txPattern                - pattern to transmit
* @param[in] mode                     - test mode or normal
*/
GT_STATUS mvHwsGw16TileTestGen
(
    IN GT_U8                       devNum,
    IN GT_UOPT                     portGroup,  /* D2D index */
    IN GT_UOPT                     serdesNum,  /* Channel index */
    IN MV_HWS_SERDES_TX_PATTERN    txPattern,
    IN MV_HWS_SERDES_TEST_GEN_MODE mode
)
{
    GT_U32      regAddr;
    GT_U32      regData;
    GT_U32      regMask;

    if((portGroup > 1) || (serdesNum > 3))
    {
        hwsOsPrintf("mvHwsGw16TestGen: bad param for devNum=%d, portGroup=%d, serdesNum=%d\n", devNum, portGroup, serdesNum);
        return GT_BAD_PARAM;
    }

    if(hwsRegisterSetFuncPtr == NULL)
    {
        hwsOsPrintf("mvHwsGw16TestGen: bad pointer hwsRegisterSetFuncPtr for devNum=%d\n", devNum);
        return GT_BAD_PTR;
    }

    switch(txPattern)
    {
        case  PRBS31:
            regData = (mode == SERDES_TEST) ? 0x3F : 0;
            regMask = 0x7F;
            break;

        case  PRBS15:
            regData = (mode == SERDES_TEST) ? 0x1F : 0;
            regMask = 0x7F;
            break;

        default:
            return GT_NOT_SUPPORTED;
    }

    /* RX debug enigines should be enabled */
    regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_RX_ANALYSIS_MODE + serdesNum * RAVEN_LANE_OFFSET_CNS));
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 1, 1));

    if((hwsGw16TileTestGenMode[devNum] == HWS_D2D_DIRECTION_MODE_BOTH_E) ||
       (hwsGw16TileTestGenMode[devNum] == HWS_D2D_DIRECTION_MODE_TX_E))
    {
        /* 1. Write to Tx_pattern_address  data 0x1F */
        regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_TX_PATSEL1 + serdesNum * RAVEN_LANE_OFFSET_CNS));
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    }

    if((hwsGw16TileTestGenMode[devNum] == HWS_D2D_DIRECTION_MODE_BOTH_E) ||
       (hwsGw16TileTestGenMode[devNum] == HWS_D2D_DIRECTION_MODE_RX_E))
    {
        /* 2. Write to Rx_pattern_address  data 0x1F */
        regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_RX_PATPRBS + serdesNum * RAVEN_LANE_OFFSET_CNS));
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    }

    /* 3. Enable/Disable PRBS */
    regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_RX_PATRSTSNAP + serdesNum * RAVEN_LANE_OFFSET_CNS));
    regData = (mode == SERDES_TEST) ? 0x1F : 0;
    regMask = 0x1F;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

    regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_RX_PATRSTSNAP + serdesNum * RAVEN_LANE_OFFSET_CNS));
    regData = 0;
    regMask = 0x1F;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));


    return GT_OK;
}

/**
* @internal mvHwsGw16TileTestGenGet function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - GW16 channel index (APPLICABLE RANGES: 0..3)
* @param[out] txPattern               - (pointer to) pattern to transmit
* @param[out] mode                    - (pointer to) test mode or normal
*/
GT_STATUS mvHwsGw16TileTestGenGet
(
    IN GT_U8                       devNum,
    IN GT_UOPT                     portGroup,
    IN GT_UOPT                     serdesNum,
    OUT MV_HWS_SERDES_TX_PATTERN    *txPatternPtr,
    OUT MV_HWS_SERDES_TEST_GEN_MODE *modePtr
)
{
    GT_U32      regAddr;
    GT_U32      regData;
    GT_U32      regMask;

    if((portGroup > 1) || (serdesNum > 3))
    {
        hwsOsPrintf("mvHwsGw16TestGen: bad param for devNum=%d, portGroup=%d, serdesNum=%d\n", devNum, portGroup, serdesNum);
        return GT_BAD_PARAM;
    }

    if(hwsRegisterGetFuncPtr == NULL)
    {
        hwsOsPrintf("mvHwsGw16TestGen: bad pointer hwsRegisterGetFuncPtr for devNum=%d\n", devNum);
        return GT_BAD_PTR;
    }

    *txPatternPtr = PRBS31;

    regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_RX_PATRSTSNAP + serdesNum * RAVEN_LANE_OFFSET_CNS));
    regMask = 0x1F;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    *modePtr = (regData == 0x1F) ? SERDES_TEST : SERDES_NORMAL;

    return GT_OK;
}

/**
* @internal mvHwsGw16TileTestGenStatus function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - GW16 channel index (APPLICABLE RANGES: 0..3)
* @param[in] txPattern                - pattern to transmit
* @param[in] counterAccumulateMode    - counter Accumulate Mode.
*
* @param[out] status                  - Defines serdes test
*       generator results
*/
GT_STATUS mvHwsGw16TileTestGenStatus
(
    IN GT_U8                     devNum,
    IN GT_U32                    portGroup,
    IN GT_U32                    serdesNum,
    IN MV_HWS_SERDES_TX_PATTERN  txPattern,
    IN GT_BOOL                   counterAccumulateMode,
    OUT MV_HWS_SERDES_TEST_GEN_STATUS *status
)
{
    GT_U32      regAddr;
    GT_U32      regData;
    GT_U32      regData1;
    GT_U32      tmpData;
    GT_U32      regMask;
    GT_U32      d2dSubLaneNum;

    counterAccumulateMode = counterAccumulateMode;
    txPattern = txPattern;

    if((portGroup > 1) || (serdesNum > 3))
    {
        hwsOsPrintf("mvHwsGw16TestGen: bad param for devNum=%d, portGroup=%d, serdesNum=%d\n", devNum, portGroup, serdesNum);
        return GT_BAD_PARAM;
    }

    if(hwsRegisterGetFuncPtr == NULL)
    {
        hwsOsPrintf("mvHwsGw16TestGen: bad pointer hwsRegisterGetFuncPtr for devNum=%d\n", devNum);
        return GT_BAD_PTR;
    }

    regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_RX_PATSTAT + serdesNum * RAVEN_LANE_OFFSET_CNS));
    regMask = 0x1F;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData1, regMask));

    status->errorsCntr = 0;
    status->txFramesCntr.l[0] = 0;
    status->lockStatus = 1;

    /* take snapshot */
    regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_RX_PATRSTSNAP + serdesNum * RAVEN_LANE_OFFSET_CNS));
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0x20, 0x20));
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0, 0x20));

    for(d2dSubLaneNum = 0; d2dSubLaneNum < 5; d2dSubLaneNum++)
    {
        regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_RX_PAT_ERR_COUNT_BASE + d2dSubLaneNum * 0x10 + serdesNum * RAVEN_LANE_OFFSET_CNS));
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
        tmpData = regData;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr + 4, &regData, regMask));
        tmpData = tmpData | (regData << 8);
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr + 8, &regData, regMask));
        tmpData = tmpData | (regData << 16);
        status->errorsCntr = tmpData;
    }

    if(regData1 != 0)
    {
        status->lockStatus = 0;

    }

    return GT_OK;
}


/**
* @internal mvHwsGw16IfClose function
* @endinternal
*
* @brief   Release all system resources allocated by Serdes IF functions.
*
* @param[in] devNum                   - The Pp's device number
*/
void mvHwsGw16IfClose
(
    IN GT_U8      devNum
)
{
    hwsInitDone[devNum] = GT_FALSE;
}

/**
* @internal mvHwsD2dPhyIfInit function
* @endinternal
*
* @brief   Init GW16 Serdes IF functions.
*
* @param[in] devNum                   - The Pp's device number
*/
GT_STATUS mvHwsD2dPhyIfInit
(
    GT_U8  devNum,
    MV_HWS_SERDES_FUNC_PTRS **funcPtrArray
)
{
    hwsInitDone[devNum] = GT_TRUE;

    /* DB init */
    if(!funcPtrArray[D2D_PHY_GW16])
    {
        funcPtrArray[D2D_PHY_GW16] = (MV_HWS_SERDES_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_SERDES_FUNC_PTRS));
        if(!funcPtrArray[D2D_PHY_GW16])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[D2D_PHY_GW16], 0, sizeof(MV_HWS_SERDES_FUNC_PTRS));
    }
#ifndef CO_CPU_RUN
    funcPtrArray[D2D_PHY_GW16]->serdesTestGenFunc            = mvHwsGw16TileTestGen;
    funcPtrArray[D2D_PHY_GW16]->serdesTestGenGetFunc         = mvHwsGw16TileTestGenGet;
    funcPtrArray[D2D_PHY_GW16]->serdesTestGenStatusFunc      = mvHwsGw16TileTestGenStatus;
    funcPtrArray[D2D_PHY_GW16]->serdesTypeGetFunc            = mvHwsGw16SerdesTypeGetFunc;
#endif /* CO_CPU_RUN */

    return GT_OK;
}

#ifdef CHX_FAMILY


static GT_STATUS mvHwsFalconZ2GW16SerdesOverridesApply
(
    GT_U8 devNum,
    GT_U32 devIndex,
    GT_U32 d2dNum
)
{
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regMask;
    GT_U32 lane;
    GT_U32 baseAddr, unitNum, unitIndex;

    CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, (devIndex*2 + d2dNum), &baseAddr, &unitIndex, &unitNum));

    /* The PMA layer is released fom TX/RX reset with the following sequence */
    regAddr = baseAddr + D2D_PMA_CFG_RESET;
    regData = 0x0;
    regMask = 0xFF;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    regData = 0xFF;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

    /* trim RX regulator = 875mV */
    regAddr = baseAddr + D2D_PHY_RXC_PLL_REG_LEVEL;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 3, 1));

    /* ("txc_pllcpctrl", 0x12)  # pcp=2, pcphalf=1 TX PLL setting for BW~=5MHz */
    regAddr = baseAddr + D2D_PHY_TXC_PLL_CP_CTRL;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0x12, 0x1F));

    /* ("txc_pllloopctrl", 14)  # Set the optimal TX PLL damping resitor value for frequency */
    regAddr = baseAddr + D2D_PHY_TXC_PLL_LOOP_CTRL;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 14, 0x1F));

    regAddr = baseAddr + D2D_PHY_TXC_SLCTHRES0;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 5, 0x1F));

    for(lane = 0; lane < 4; lane++)
    {
        /* CTLE 'ib' setting is being reduced from 12 to 9 */
        regAddr = baseAddr + D2D_PHY_RX_CTLE_CTRL + lane * RAVEN_LANE_OFFSET_CNS;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, (12<<1), 0x1E));

        regAddr = baseAddr + D2D_PHY_RX_CENTERLEVEL + lane * RAVEN_LANE_OFFSET_CNS;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0x10, 0xFF));

        /* RX CTLE settings per SERDES */
        regAddr = baseAddr + D2D_PHY_RX_CTLE_EQ + lane * RAVEN_LANE_OFFSET_CNS;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0x5, 0xF));

        regAddr = baseAddr + D2D_PHY_RX_CDA_PI_KP + lane * RAVEN_LANE_OFFSET_CNS;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0xF, 0xF));
    }

    regAddr = baseAddr + 0x3fc0;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0x8e, 0xFF));

    return GT_OK;
}

GT_STATUS mvHwsGW16SerdesSimplePairAutoStartInit
(
    GT_U8 devNum,
    GT_U32 devIndex1,
    GT_U32 d2dNum1,
    GT_U32 devIndex2,
    GT_U32 d2dNum2
)
{
    GT_U32 regAddr1;
    GT_U32 regAddr2;
    GT_U32 regData;
    GT_U32 regMask;
    GT_U32 baseAddr1, baseAddr2, unitNum, unitIndex;

    CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, (devIndex1*2 + d2dNum1), &baseAddr1, &unitIndex, &unitNum));
    CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, (devIndex2*2 + d2dNum2), &baseAddr2, &unitIndex, &unitNum));

    /* Set autostart field */
    regAddr1 = baseAddr1 + D2D_PHY_RXC_AUTOSTART;
    regAddr2 = baseAddr2 + D2D_PHY_RXC_AUTOSTART;
    regData = 0x0;
    regMask = 0xF;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr1, regData, regMask));
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr2, regData, regMask));

    regData = 0xF;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr1, regData, regMask));
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr2, regData, regMask));

    return GT_OK;
}

GT_STATUS mvHwsGW16SerdesGradualAutoStartInitStatusGet
(
    GT_U8 devNum,
    GT_U32 devIndex,
    GT_U32 d2dNum
)
{
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regMask;
    GT_U32 cnt;
    GT_U32 baseAddr, unitNum, unitIndex;

    CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, (devIndex*2 + d2dNum), &baseAddr, &unitIndex, &unitNum));

    cnt = 0;
    regAddr = baseAddr + D2D_PHY_RXC_AUTOSTART;

    /* Poll the 4 autostart_done bits */
    regMask = 0xF0;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
    while(regData != regMask)
    {
        hwsOsTimerWkFuncPtr(10);
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
        cnt++;
        if(cnt == 50)
        {
            hwsOsPrintf("D2D[%d][%d] PHY init was failed, link_status=0x%x\n", devIndex, d2dNum, regData);
            return GT_OK;
        }
    }

    return GT_OK;
}


/**
* @internal mvHwsGW16_Z2_SerdesAutoStartInit function
* @endinternal
*
* @param[in] devNum                   - The Pp's device number
* @param[in] ravenBmp                 - bitmap of requested Ravens
*/
GT_STATUS mvHwsGW16_Z2_SerdesAutoStartInit
(
    GT_U8 devNum,
    GT_U32 ravenBmp
)
{
    GT_STATUS   rc;
    GT_U32      devIndex;
    GT_U32      pairIndex;
    GT_U32      d2dIndex;
    GT_U32 d2dPairsArrayZ2[16][4] = { {/*gp0m0*/0,   0, /*gp3m1*/3,   1},
                                      {/*gp0m1*/0,   1, /*gp3m0*/3,   0},
                                      {/*gp1m0*/1,   0, /*gp2m1*/2,   1},
                                      {/*gp1m1*/1,   1, /*gp2m0*/2,   0},

                                      {/*gp4m0*/4,   0, /*gp7m1*/7,   1},
                                      {/*gp4m1*/4,   1, /*gp7m0*/7,   0},
                                      {/*gp5m0*/5,   0, /*gp6m1*/6,   1},
                                      {/*gp5m1*/5,   1, /*gp6m0*/6,   0},

                                      {/*gp8m0*/8,   0, /*gp11m1*/11, 1},
                                      {/*gp8m1*/8,   1, /*gp11m0*/11, 0},
                                      {/*gp9m0*/9,   0, /*gp10m1*/10, 1},
                                      {/*gp9m1*/9,   1, /*gp10m0*/10, 0},

                                      {/*gp10m0*/12, 0, /*gp15m1*/15, 1},
                                      {/*gp10m1*/12, 1, /*gp15m0*/15, 0},
                                      {/*gp13m0*/13, 0, /*gp14m1*/14, 1},
                                      {/*gp13m1*/13, 1, /*gp14m0*/14, 0}};

#if !defined MV_HWS_FREE_RTOS && !defined PX_FAMILY
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsGW16_Z2_SerdesAutoStartInit ******\n");
    }
#endif

    rc = mvHwsCpllControl(NULL, devNum, CPLL0, GT_FALSE, MV_HWS_156MHz_IN, MV_HWS_156MHz_OUT);
    if(rc != GT_OK)
    {
        hwsOsPrintf("mvHwsCpllControl failed: devNum = %d, rc = %d\n", devNum, rc);
        return rc;
    }

    /* Registers value override only */
    for(devIndex = 0; devIndex < hwsFalconNumOfRavens; devIndex++)
    {
        if(((1<<devIndex) & ravenBmp) == 0)
        {
            continue;
        }

        for(d2dIndex = 0; d2dIndex < 2; d2dIndex++)
        {
            rc = mvHwsFalconZ2GW16SerdesOverridesApply(devNum, devIndex, d2dIndex);
            if(rc != GT_OK)
            {
                cpssOsPrintf("mvHwsGW16SerdesOverridesApply failed: devNum = %d, d2dNum = %d, rc = %d\n", devIndex,d2dIndex,rc);
                return rc;
            }
        }
    }

    /* Start the D2D pattern generators so that traffic is running before pair-autostart.
       On Raven this must be done after the dummy autostart (before the staggered autostarts)
       so that the D2D clocks are active.*/
/*
    def pat_init_qfn(self, bom, tc, m, q, mask, verbose=True):
        psel = (mask >> 16) & 0x1F
        for ch in bom.loops.just('subch'):
            cmt = "gp%dm%dt%dd%d" % (tc, m, q, ch)
            cmr = "gp%dm%dr%dd%d" % (tc, m, q, ch)
            bom.set_reg(cmt+"patctl", 1)
            bom.set_reg(cmt+"psel", psel)
            bom.set_reg(cmr+"patctl", 2)
            bom.set_reg(cmr+"psel", psel)
*/

    for(pairIndex = 0; pairIndex < (sizeof(d2dPairsArrayZ2) / sizeof(d2dPairsArrayZ2[0])); pairIndex++)
    {
        rc = mvHwsGW16SerdesSimplePairAutoStartInit(devNum, d2dPairsArrayZ2[pairIndex][0], d2dPairsArrayZ2[pairIndex][1], d2dPairsArrayZ2[pairIndex][2], d2dPairsArrayZ2[pairIndex][3]);
        if(rc != GT_OK)
        {
            cpssOsPrintf("mvHwsGW16SerdesSimplePairAutoStartInit failed: devNum = %d, rc = %d\n", d2dPairsArrayZ2[pairIndex][0], rc);
            return rc;
        }

        rc = mvHwsGW16SerdesGradualAutoStartInitStatusGet(devNum, d2dPairsArrayZ2[pairIndex][0], d2dPairsArrayZ2[pairIndex][1]);
        if(rc != GT_OK)
        {
            cpssOsPrintf("mvHwsGW16SerdesGradualAutoStartInitStatusGet failed: devNum = %d, rc = %d\n", d2dPairsArrayZ2[pairIndex][0], rc);
            return rc;
        }
        rc = mvHwsGW16SerdesGradualAutoStartInitStatusGet(devNum, d2dPairsArrayZ2[pairIndex][2], d2dPairsArrayZ2[pairIndex][3]);
        if(rc != GT_OK)
        {
            cpssOsPrintf("mvHwsGW16SerdesGradualAutoStartInitStatusGet failed: devNum = %d, rc = %d\n", d2dPairsArrayZ2[pairIndex][2], rc);
            return rc;
        }

        hwsOsTimerWkFuncPtr(50);
    }

#if !defined MV_HWS_FREE_RTOS && !defined PX_FAMILY
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsGW16_Z2_SerdesAutoStartInit ******\n");
    }
#endif

    return GT_OK;
}
#endif


