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
* mvHwsDiagnostic.h
*
* DESCRIPTION:
*
* DEPENDENCIES:
*
******************************************************************************/
#ifndef MV_HWS_REDUCED_BUILD
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/port/private/mvHwsDiagnostic.h>

#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacSgIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacSgIf.h>
#include <cpss/common/labServices/port/gop/port/mac/hglMac/mvHwsHglMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xgMac/mvHwsXgMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/ilknMac/mvHwsIlknMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xgMac/mvHwsXgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMac28nmIf.h>

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/hglPcs/mvHwsHglPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcsV2If.h>
#include <cpss/common/labServices/port/gop/port/pcs/xPcs/mvHwsXPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/ilknPcs/mvHwsIlknPcsIf.h>

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH/mvComPhyHIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyHRev2/mvComPhyHRev2If.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdes28nmPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nmRev3/mvComPhyH28nmRev3If.h>

#define BASE_DFX_SERVER_AC5_UNIT        0x84000000
#define BASE_DFX_SERVER_AC5X_UNIT       0x94400000
#define BASE_DFX_SERVER_AC5P_UNIT       0x0B800000
#define BASE_DFX_SERVER_HARRIER_UNIT    0x0

GT_STATUS mvHwsDiagCheckValidInputParams(GT_U8 devNum, MV_HWS_DEV_TYPE devType);
GT_STATUS mvHwsDiagPCSInitDBStatusGet(GT_U8 devNum);
GT_STATUS mvHwsDiagMACInitDBStatusGet(GT_U8 devNum);
GT_STATUS mvHwsDiagSerdesInitDBStatusGet(GT_U8 devNum);
GT_STATUS mvHwsDiagDdr3TipInitDBStatusGet(GT_U8 devNum);

#ifndef MV_HWS_REDUCED_BUILD
static const char *DEV_TYPE[LAST_SIL_TYPE] =
{
    "Lion2A0",
    "Lion2B0",
    "HooperA0",
    "BobcatA0",
    "Alleycat3A0",
    "BobK",
    "Aldrin",
    "Bobcat3",
    "Aldrin2"
};

#define MV_HWS_RETURN_PCS_BAD_PRT(devNum, sType, pcsMode) \
{ \
    hwsOsPrintf("Init DB for: devNum=%d, siliconType=%s, pcsMode=%s is wrong\n", devNum, DEV_TYPE[sType], pcsMode); \
    return GT_BAD_PARAM; \
}

#define MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, macMode) \
{ \
    hwsOsPrintf("Init DB for: devNum=%d, siliconType=%s, macMode=%s is wrong\n", devNum, DEV_TYPE[sType], macMode); \
    return GT_BAD_PARAM; \
}

#define MV_HWS_RETURN_SERDES_BAD_PRT(devNum, sType, serdesMode) \
{ \
    hwsOsPrintf("Init DB for: devNum=%d, siliconType=%s, serdesMode=%s is wrong\n", devNum, DEV_TYPE[sType], serdesMode); \
    return GT_BAD_PARAM; \
}

/**
* @internal mvHwsAvsVoltageGet function
* @endinternal
*
* @brief   Get the Avs voltage in mv.
*
* @param[in] devNum                   - system device number
* @param[in] unitNumber              - chip unit index number.
* @param[out] avsVol                - Avs voltage in mv
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvsVoltageGet
(
    IN GT_U8 devNum,
    IN MV_HWS_UNIT_NUM unitNumber,
    OUT GT_U32 *avsVol
)
{
    GT_U32 baseAddr ,unitIndex,unitNum;
    GT_U32 controlAddr, statusAddr;

    if (avsVol == NULL) {
        return GT_BAD_PTR;
    }

    switch (HWS_DEV_SILICON_TYPE(devNum))
    {
        case Aldrin2:
            CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, DFX_SERVER_AVS_ENABLE_CONTROL, avsVol));
            *avsVol = (*avsVol) >> 3;
            *avsVol = (*avsVol & 0x3ff);
            *avsVol = ((*avsVol) * 10) + 640;
            break;
        case AC5:
            CHECK_STATUS(hwsServerRegGetFuncPtr(devNum,BASE_DFX_SERVER_AC5_UNIT + DFX_SERVER_AVS_ENABLE_CONTROL, avsVol));
            *avsVol = (*avsVol) >> 3;
            *avsVol = (*avsVol & 0x3ff);
            *avsVol = ((*avsVol) * 1198) /1024;
            break;
        case AC5X:
            CHECK_STATUS(hwsServerRegGetFuncPtr(devNum,BASE_DFX_SERVER_AC5X_UNIT + DFX_SERVER_AVS_ENABLE_CONTROL, avsVol));
            *avsVol = (*avsVol) >> 3;
            *avsVol = (*avsVol & 0x3ff);
            *avsVol = ((*avsVol) * 1198) /1024;
            break;
        case AC5P:
            CHECK_STATUS(hwsServerRegGetFuncPtr(devNum,BASE_DFX_SERVER_AC5P_UNIT + DFX_SERVER_AVS_ENABLE_CONTROL, avsVol));
            *avsVol = (*avsVol) >> 3;
            *avsVol = (*avsVol & 0x3ff);
            *avsVol = ((*avsVol) * 1198) /1024;
            break;
        case Harrier:
            CHECK_STATUS(hwsServerRegGetFuncPtr(devNum,BASE_DFX_SERVER_HARRIER_UNIT + DFX_SERVER_AVS_5NM_ENABLED_CONTROL, avsVol));
            *avsVol = (*avsVol) >> 3;
            *avsVol = (*avsVol & 0x3ff);
            *avsVol = ((*avsVol) * 1222) /1024;
            break;
        case Falcon:
            if (unitNumber <= Raven_15)
            {
                CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_DFX_UNIT, unitNumber * 16, &baseAddr, &unitIndex, &unitNum));
                controlAddr = baseAddr + DFX_SERVER_AVS_CONTROL;
                statusAddr = baseAddr + DFX_SERVER_AVS_STATUS;
                /* set page 2 in control register in order to get in status register debug bus: avs_cfg[31:0] */
                CHECK_STATUS(hwsServerRegFieldSetFuncPtr(devNum, controlAddr, 29, 3, 2));
                CHECK_STATUS(hwsServerRegFieldGetFuncPtr(devNum, statusAddr, 8, 10, avsVol));

                *avsVol = ((*avsVol) * 1198) /1024;
            }
            else if (unitNumber >= Eagle_0 && unitNumber <=Eagle_3)
            {
                controlAddr = (unitNumber%4)*FALCON_TILE_OFFSET + FALCON_DFX_BASE_ADDRESS_CNS + DFX_SERVER_AVS_CONTROL;
                statusAddr = (unitNumber%4)*FALCON_TILE_OFFSET + FALCON_DFX_BASE_ADDRESS_CNS + DFX_SERVER_AVS_STATUS;
                /* set page 2 in control register in order to get in status register debug bus: avs_cfg[31:0] */
                CHECK_STATUS(hwsServerRegFieldSetFuncPtr(devNum, controlAddr, 29, 3, 2));
                CHECK_STATUS(hwsServerRegFieldGetFuncPtr(devNum, statusAddr, 8, 10, avsVol));

                *avsVol = ((*avsVol) * 1198) /1024;
            }
            else
            {
                return GT_NOT_SUPPORTED;
            }
        break;
        default:
            return GT_NOT_APPLICABLE_DEVICE;
    }
    return GT_OK;
}

/**
* @internal mvHwsAvsVoltageSet function
* @endinternal
*
* @brief   Set the Avs voltage in mv.
*
* @param[in] devNum                  - system device number
* @param[in] unitNumber              - chip unit index number.
* @param[in] vcore                   - Avs voltage requsted in mv.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvsVoltageSet
(
    IN GT_U8 devNum,
    IN MV_HWS_UNIT_NUM unitNumber,
    IN GT_U32 vcore
)
{
    GT_U32 avsTargetVal = 0;
    GT_U32 i;
    GT_U32 regAddr = 0x0;
    GT_U32 avsAvgAddr;
    GT_U32 baseAddr ,unitIndex,unitNum;

    switch (HWS_DEV_SILICON_TYPE(devNum))
    {
        case Falcon:
            if (unitNumber == All)
            {
                for (i = 0; i < 16; i++)
                {
                    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_DFX_UNIT, i * 16, &baseAddr, &unitIndex, &unitNum));
                    regAddr = baseAddr + DFX_SERVER_AVS_ENABLE_CONTROL;
                    avsAvgAddr = baseAddr + DFX_SERVER_AVS_DISABLE_CONTROL1;

                    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, avsAvgAddr, 16, 3, 0x4));
                    avsTargetVal = (vcore * 1024) / 1198; /* coverting from - VALUE = (VDD / 1.198 ) * 1024 */

                    CHECK_STATUS(mvHwsAvsRegUnitSet(devNum, unitNumber, regAddr, avsTargetVal));
                }
            }
            else if (unitNumber <= Raven_15)
            {
                CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_DFX_UNIT, unitNumber * 16, &baseAddr, &unitIndex, &unitNum));
                regAddr = baseAddr + DFX_SERVER_AVS_ENABLE_CONTROL;
                avsAvgAddr = baseAddr + DFX_SERVER_AVS_DISABLE_CONTROL1;

                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, avsAvgAddr, 16, 3, 0x4));
                avsTargetVal = (vcore * 1024) / 1198; /* coverting from - VALUE = (VDD / 1.198 ) * 1024 */
            }
            else if (unitNumber >= Eagle_0 && unitNumber <=Eagle_3 )
            {
                regAddr = (unitNumber%4)*FALCON_TILE_OFFSET + FALCON_DFX_BASE_ADDRESS_CNS + DFX_SERVER_AVS_ENABLE_CONTROL;
                avsAvgAddr = (unitNumber%4)*FALCON_TILE_OFFSET + FALCON_DFX_BASE_ADDRESS_CNS + DFX_SERVER_AVS_DISABLE_CONTROL1;

                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, avsAvgAddr, 16, 3, 0x4));
                avsTargetVal = (vcore * 1024) / 1198; /* coverting from - VALUE = (VDD / 1.198 ) * 1024 */
            }
            else
            {
                /* wrong unitNumber */
                return GT_NOT_SUPPORTED;
            }
        break;
        case Aldrin2:
            regAddr = DFX_SERVER_AVS_ENABLE_CONTROL;
            avsTargetVal = (vcore - 640) * 10;
            break;
        case AC5:
            if ((vcore < 720) || (vcore > 980)) /* range is between 0.72V-0.98V */
            {
                return GT_OUT_OF_RANGE;
            }
            regAddr = DFX_SERVER_AVS_ENABLE_CONTROL + BASE_DFX_SERVER_AC5_UNIT;
            avsTargetVal = (vcore * 1024) / 1198; /* coverting from - VALUE = (VDD / 1.198 ) * 1024 */
            break;
        case AC5X:
            if ((vcore < 720) || (vcore > 880)) /* range is between 0.72V-0.88V */
            {
                return GT_OUT_OF_RANGE;
            }
            regAddr = DFX_SERVER_AVS_ENABLE_CONTROL + BASE_DFX_SERVER_AC5X_UNIT;
            avsTargetVal = (vcore * 1024) / 1198; /* coverting from - VALUE = (VDD / 1.198 ) * 1024 */
            break;
        case AC5P:
            if ((vcore < 800) || (vcore > 930)) /* range is between 0.8V-0.93V */
            {
                return GT_OUT_OF_RANGE;
            }
            regAddr = DFX_SERVER_AVS_ENABLE_CONTROL + BASE_DFX_SERVER_AC5P_UNIT;
            avsTargetVal = (vcore * 1024) / 1198; /* coverting from - VALUE = (VDD / 1.198 ) * 1024 */
            break;
        case Harrier:
            if ((vcore < 700) || (vcore > 830)) /* range is between 0.7V-0.8V */
            {
                return GT_OUT_OF_RANGE;
            }
            regAddr = DFX_SERVER_AVS_5NM_ENABLED_CONTROL + BASE_DFX_SERVER_HARRIER_UNIT;
            avsTargetVal = (vcore * 1024) / 1222; /* coverting from - VALUE = (VDD / 1.222 ) * 1024 */
            break;
        default:
            return GT_NOT_APPLICABLE_DEVICE;
    }

    if (unitNumber != All)
    {
        CHECK_STATUS(mvHwsAvsRegUnitSet(devNum,unitNumber, regAddr, avsTargetVal));
    }
     return GT_OK;

}

/**
* @internal mvHwsAvsRegUnitSet function
* @endinternal
*
* @brief   set  avs register.
*
* @param[in] devNum                  - system device number
* @param[in] regAddr                 - avs register address.
* @param[in] avsTargetVal           - avs target value.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvsRegUnitSet
(
    IN GT_U8 devNum,
    IN MV_HWS_UNIT_NUM unitNumber,
    IN GT_U32 regAddr,
    IN GT_U32 avsTargetVal
)
{
    GT_U32 avsVal;
    GT_U32 readData,regValue;
    GT_U32 avsRegVal;
    GT_U32 statusAddr;
    GT_U32 controlAddr;
    GT_U32 baseAddr ,unitIndex,unitNum;

    if(HWS_DEV_SILICON_TYPE(devNum) == Falcon)
    {
        /* In order to get current Voltage it is needed to read
           AVS debug bus. Control register (0xF8130) set to default value
           during soft reset, while AVS voltage stays with configured value.
           Only with power down/power up AVS and control register are synced. */

        if (unitNumber <= Raven_15)
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_DFX_UNIT, unitNumber * 16, &baseAddr, &unitIndex, &unitNum));
            controlAddr = baseAddr + DFX_SERVER_AVS_CONTROL;
            statusAddr = baseAddr + DFX_SERVER_AVS_STATUS;
        }
        else if (unitNumber >= Eagle_0 && unitNumber <=Eagle_3 )
        {
            controlAddr = (unitNumber%4)*FALCON_TILE_OFFSET + FALCON_DFX_BASE_ADDRESS_CNS + DFX_SERVER_AVS_CONTROL;
            statusAddr = (unitNumber%4)*FALCON_TILE_OFFSET + FALCON_DFX_BASE_ADDRESS_CNS + DFX_SERVER_AVS_STATUS;
        }
        else
        {
            return GT_BAD_PARAM;
        }

        /* set page 2 in control register in order to get in status register debug bus: avs_cfg[31:0] */
        CHECK_STATUS(hwsServerRegFieldSetFuncPtr(devNum, controlAddr, 29, 3, 2));
        CHECK_STATUS(hwsServerRegFieldGetFuncPtr(devNum, statusAddr, 8, 10, &regValue));
    }
    else if(HWS_DEV_SILICON_TYPE(devNum) == AC5)
    {
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum,regAddr, &regValue));
        regValue = (regValue) >> 3;
        regValue = (regValue & 0x3ff);
    }
    else
    {
        CHECK_STATUS(hwsServerRegFieldGetFuncPtr(devNum, regAddr, 3, 10, &regValue));
    }
    avsVal = regValue;

    while (avsVal != avsTargetVal)
    {
        if (avsVal < avsTargetVal)
        {
            if(avsTargetVal - avsVal < 8)
            {
                avsVal++;
            }
            else
            {
                avsVal+=8;
            }
        }
        else
        {
            if(avsVal - avsTargetVal < 8)
            {
                avsVal--;
            }
            else
            {
                avsVal-=8;
            }
        }

        avsRegVal = (avsVal << 3) | (avsVal << 13);
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, regAddr, &readData));

        readData = readData & (~0x7ffff8);
        avsRegVal = avsRegVal & (0x7ffff8);
        avsRegVal = avsRegVal | readData;
        /*set val*/
        if(HWS_DEV_SILICON_TYPE(devNum) == Falcon)
        {
            /*
                For Falcon "hwsServerRegSetFuncPtr" function performs
                broadcast write operation to all tiles - so it's needed to
                use another function.
            */
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, avsRegVal, 0xFFFFFFFF));
            hwsOsTimerWkFuncPtr(300);
        }
        else
        {
            CHECK_STATUS(hwsServerRegSetFuncPtr(devNum, regAddr, avsRegVal));
            hwsOsTimerWkFuncPtr(1000);
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsDiagDeviceDbCheck function
* @endinternal
*
* @brief   This API checks the DB initialization of PCS, MAC, Serdes
*         and DDR units
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS mvHwsDiagDeviceDbCheck(GT_U8 devNum, MV_HWS_DEV_TYPE devType)
{
    GT_STATUS rc;

    /* If device not valid for hws, we cannot continue to other hws APIs.
       We also cannot use hws check status macro.*/
    rc = (mvHwsDiagCheckValidInputParams(devNum, devType));
    if (rc!=GT_OK)
    {
        return rc;
    }

    CHECK_STATUS(mvHwsDiagPCSInitDBStatusGet(devNum));
    CHECK_STATUS(mvHwsDiagMACInitDBStatusGet(devNum));
    CHECK_STATUS(mvHwsDiagSerdesInitDBStatusGet(devNum));

    /* TBD - Need to add checking to DDR initialization DB */
    /* CHECK_STATUS(mvHwsDiagDdr3TipInitDBStatusGet(devNum)); */

    return GT_OK;
}

/**
* @internal mvHwsDiagCheckValidInputParams function
* @endinternal
*
* @brief   This function checks the validity of input devNum and devType
*         parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS mvHwsDiagCheckValidInputParams(GT_U8 devNum, MV_HWS_DEV_TYPE devType)
{
    if(devNum >= HWS_MAX_DEVICE_NUM)
    {
        hwsOsPrintf("mvHwsDiagDeviceInitDBStatusGet: Device number=%d is bigger than HWS_MAX_DEVICE_NUM=%d value\n", devNum, HWS_MAX_DEVICE_NUM-1);
        return GT_BAD_PARAM;
    }

    if(hwsDeviceSpecInfo[devNum].devNum != devNum)
    {
        hwsOsPrintf("mvHwsDiagDeviceInitDBStatusGet: Device number=%d is not initialized in the system\n", devNum);
        return GT_BAD_PARAM;
    }

    if(devType >= sizeof(DEV_TYPE)/sizeof(char*))
    {
        hwsOsPrintf("mvHwsDiagDeviceInitDBStatusGet: Device number=[%d] devType[%d] out of the size of DEV_TYPE array \n" , devNum , devType);
        return GT_BAD_PARAM;
    }

    if(HWS_DEV_SILICON_TYPE(devNum) != devType)
    {
        hwsOsPrintf("mvHwsDiagDeviceInitDBStatusGet: Device number=%d is different than Device type %s\n", devNum, DEV_TYPE[devType]);
        return GT_BAD_PARAM;
    }

    if(hwsDeviceSpecInfo[devNum].devType >= sizeof(DEV_TYPE)/sizeof(char*))
    {
        hwsOsPrintf("mvHwsDiagDeviceInitDBStatusGet: the size of DEV_TYPE array is wrong\n");
        return GT_BAD_VALUE;
    }

    return GT_OK;
}

/**
* @internal mvHwsDiagPCSInitDBStatusGet function
* @endinternal
*
* @brief   This function checks the DB initialization of PCS unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS mvHwsDiagPCSInitDBStatusGet(GT_U8 devNum)
{
    MV_HWS_DEV_TYPE sType = HWS_DEV_SILICON_TYPE(devNum);

    MV_HWS_PCS_FUNC_PTRS **hwsPcsFuncsPtr;
    CHECK_STATUS(hwsPcsGetFuncPtr(devNum, &hwsPcsFuncsPtr));

    switch (sType)
    {
    case Lion2B0:
    case HooperA0:
        if(hwsPcsFuncsPtr[MMPCS]->pcsModeCfgFunc != mvHwsMMPcsV2Mode)
        {
            MV_HWS_RETURN_PCS_BAD_PRT(devNum, sType, "MMPCS");
        }

        if(hwsPcsFuncsPtr[HGLPCS]->pcsModeCfgFunc != mvHwsHglPcsMode)
        {
            MV_HWS_RETURN_PCS_BAD_PRT(devNum, sType, "HGLPCS");
        }

        if(hwsPcsFuncsPtr[XPCS]->pcsModeCfgFunc != mvHwsXPcsMode)
        {
            MV_HWS_RETURN_PCS_BAD_PRT(devNum, sType, "XPCS");
        }
        break;

    case Alleycat3A0:
    case AC5:
    case BobcatA0:
        if(hwsPcsFuncsPtr[XPCS]->pcsModeCfgFunc != mvHwsXPcsMode)
        {
            MV_HWS_RETURN_PCS_BAD_PRT(devNum, sType, "XPCS");
        }

        if(hwsPcsFuncsPtr[MMPCS]->pcsModeCfgFunc != mvHwsMMPcs28nmMode)
        {
            MV_HWS_RETURN_PCS_BAD_PRT(devNum, sType, "MMPCS");
        }
        break;
    default:
        hwsOsPrintf("mvHwsDiagPCSInitDBStatusGet: Unsupported device type=%d\n", sType);
        return GT_FAIL;
    }

    if((sType == BobcatA0) && (hwsPcsFuncsPtr[INTLKN_PCS]->pcsModeCfgFunc != mvHwsIlknPcsMode))
    {
        MV_HWS_RETURN_PCS_BAD_PRT(devNum, sType, "INTLKN_PCS");
    }

    hwsOsPrintf("PCS Initialization DB for: devNum=%d, siliconType=%s is corrected\n", devNum, DEV_TYPE[sType]);

    return GT_OK;
}

/**
* @internal mvHwsDiagMACInitDBStatusGet function
* @endinternal
*
* @brief   This function checks the DB initialization of MAC unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS mvHwsDiagMACInitDBStatusGet(GT_U8 devNum)
{
    MV_HWS_DEV_TYPE sType = HWS_DEV_SILICON_TYPE(devNum);

    MV_HWS_MAC_FUNC_PTRS **hwsMacFuncsPtr;
    CHECK_STATUS(hwsMacGetFuncPtr(devNum, &hwsMacFuncsPtr));

    switch (sType)
    {
    case Lion2B0:
    case HooperA0:
        if(hwsMacFuncsPtr[GEMAC_X]->macModeCfgFunc != mvHwsGeMacModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "GEMAC_X");
        }

        if(hwsMacFuncsPtr[GEMAC_SG]->macModeCfgFunc != mvHwsGeMacSgModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "GEMAC_SG");
        }

        if(hwsMacFuncsPtr[HGLMAC]->macModeCfgFunc != mvHwsHglMacModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "HGLMAC");
        }

        if(hwsMacFuncsPtr[XGMAC]->macModeCfgFunc != mvHwsXgMacModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "XGMAC");
        }

        if(hwsMacFuncsPtr[XLGMAC]->macModeCfgFunc != mvHwsXlgMacModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "XLGMAC");
        }
        break;

    case Alleycat3A0:
    case AC5:
    case BobcatA0:
        if(hwsMacFuncsPtr[GEMAC_X]->macModeCfgFunc != mvHwsGeMac28nmModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "GEMAC_X");
        }

        if(hwsMacFuncsPtr[GEMAC_NET_X]->macModeCfgFunc != mvHwsGeNetMac28nmModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "GEMAC_NET_X");
        }

        if(hwsMacFuncsPtr[QSGMII_MAC]->macModeCfgFunc != mvHwsQsgmiiMac28nmModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "QSGMII_MAC");
        }

        if(hwsMacFuncsPtr[GEMAC_SG]->macModeCfgFunc != mvHwsGeMacSgModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "GEMAC_SG");
        }

        if(hwsMacFuncsPtr[GEMAC_NET_SG]->macModeCfgFunc != mvHwsGeNetMacSgModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "GEMAC_NET_SG");
        }

        if(hwsMacFuncsPtr[XGMAC]->macModeCfgFunc != mvHwsXgMac28nmModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "XGMAC");
        }

        if(hwsMacFuncsPtr[XLGMAC]->macModeCfgFunc != mvHwsXlgMac28nmModeCfg)
        {
            MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "XLGMAC");
        }
        break;
    default:
        hwsOsPrintf("mvHwsDiagMACInitDBStatusGet: Unsupported device type=%d\n", sType);
        return GT_FAIL;
    }

    if((sType == BobcatA0) && (hwsMacFuncsPtr[INTLKN_MAC]->macModeCfgFunc != mvHwsIlknMacModeCfg))
    {
        MV_HWS_RETURN_MAC_BAD_PRT(devNum, sType, "INTLKN_MAC");
    }

    hwsOsPrintf("MAC Initialization DB for: devNum=%d, siliconType=%s is corrected\n", devNum, DEV_TYPE[sType]);

    return GT_OK;
}

/**
* @internal mvHwsDiagSerdesInitDBStatusGet function
* @endinternal
*
* @brief   This function checks the DB initialization of Serdes unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS mvHwsDiagSerdesInitDBStatusGet(GT_U8 devNum)
{
    MV_HWS_DEV_TYPE sType  = HWS_DEV_SILICON_TYPE(devNum);
#ifndef PX_FAMILY
    HWS_DEV_GOP_REV gopRev = HWS_DEV_GOP_REV(devNum);
#endif
    MV_HWS_SERDES_FUNC_PTRS **hwsSerdesFuncsPtr;
    CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));

    switch (sType)
    {
#ifndef PX_FAMILY
    case Lion2B0:
    case HooperA0:
        if(hwsSerdesFuncsPtr[COM_PHY_H_REV2]->serdesPowerCntrlFunc != mvHwsComHRev2SerdesPowerCtrl)
        {
            MV_HWS_RETURN_SERDES_BAD_PRT(devNum, sType, "COM_PHY_H_REV2");
        }
        break;
    case Alleycat3A0:
    case AC5:
    case BobcatA0:
        switch (gopRev)
        {
        case GOP_28NM_REV1:
            if(hwsSerdesFuncsPtr[COM_PHY_28NM]->serdesPowerCntrlFunc != mvHwsComH28nmSerdesPowerCtrl)
            {
                MV_HWS_RETURN_SERDES_BAD_PRT(devNum, sType, "COM_PHY_28NM");
            }
            break;
        case GOP_28NM_REV2:
            if(hwsSerdesFuncsPtr[COM_PHY_28NM]->serdesManualRxCfgFunc != mvHwsComH28nmRev3SerdesManualRxConfig)
            {
                MV_HWS_RETURN_SERDES_BAD_PRT(devNum, sType, "COM_PHY_28NM");
            }
            break;

        default:
            hwsOsPrintf("mvHwsDiagSerdesInitDBStatusGet: Unsupported GOP revision\n");
            return GT_FAIL;
        }
        break;
#endif
    default:
        hwsOsPrintf("mvHwsDiagSerdesInitDBStatusGet: Unsupported device type=%d\n", sType);
        return GT_FAIL;
    }

#ifndef PX_FAMILY
    if((sType == Alleycat3A0 || sType == AC5) && (hwsSerdesFuncsPtr[COM_PHY_28NM]->serdesManualRxCfgFunc != mvHwsComH28nmRev3SerdesManualRxConfig))
    {
        MV_HWS_RETURN_SERDES_BAD_PRT(devNum, sType, "COM_PHY_28NM");
    }
    hwsOsPrintf("Serdes Initialization DB for: devNum=%d, siliconType=%s is corrected\n", devNum, DEV_TYPE[sType]);
    return GT_OK;
#endif

}

#if 0
/**
* @internal mvHwsDiagDdr3TipInitDBStatusGet function
* @endinternal
*
* @brief   This function checks the DB initialization of DDR unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS mvHwsDiagDdr3TipInitDBStatusGet(GT_U8 devNum)
{
    MV_HWS_DEV_TYPE sType = HWS_DEV_SILICON_TYPE(devNum);

    MV_HWS_TRAINING_IP_FUNC_PTRS *hwsDdr3TipFuncsPtr;

    /* need to fix warning here, the hwsTrainingIpFuncsPtr declared
       as static function in mvHwsDdr3TrainingIpIf.c */
    CHECK_STATUS(hwsTrainingIpFuncsPtr(devNum, &hwsDdr3TipFuncsPtr));

    switch (sType)
    {
    case BobcatA0:
        if(hwsDdr3TipFuncsPtr->trainingIpInitController != mvHwsDdr3TipInitController)
        {
            hwsOsPrintf("Init DB for: devNum=%d, siliconType=%s, portMode=%s is wrong\n", devNum, DEV_TYPE[sType], "Ddr3Tip");
            return GT_BAD_PARAM;
        }
        break;
    default:
        hwsOsPrintf("mvHwsDiagDdr3TipInitDBStatusGet: Unsupported device type=%d\n", sType);
        return GT_FAIL;
    }

    hwsOsPrintf("Ddr3Tip Initialization DB for: devNum=%d, siliconType=%s is corrected\n", devNum, DEV_TYPE[sType]);

    return GT_OK;
}
#endif

/**
* @internal mvHwsDroStatisticsGet function
* @endinternal
*
* @brief   get DRO statistics.
*
* @param[in] devNum                  - system device number
* @param[in] unitNumber              - chip unit index number.
* @param[out] droAvgPtr              - DRO average.
* @param[out] maxValuePtr            - DRO Maximum value.
* @param[out] minValuePtr            - DRO Minimum value.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsDroStatisticsGet
(
    IN GT_U8 devNum,
    IN MV_HWS_UNIT_NUM unitNumber,
    OUT GT_U32 *droAvgPtr,
    OUT GT_U32 *maxValuePtr,
    OUT GT_U32 *minValuePtr
)
{
    GT_U32 regValue,droReadControlAddr,droReadStatusAddr;
    GT_U32 baseAddr ,unitIndex,unitNum;
    GT_U32 i;
    GT_U32 counter = 0;
    GT_U32 minValue = 0,maxValue = 0;

    if(NULL == droAvgPtr || NULL == maxValuePtr || NULL == minValuePtr)
        return GT_BAD_PTR;

    if (HWS_DEV_SILICON_TYPE(devNum) == Falcon)
    {

        if (unitNumber <= Raven_15 )
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_DFX_UNIT, unitNumber * 16, &baseAddr, &unitIndex, &unitNum));

            droReadControlAddr = baseAddr + DFX_SERVER_DRO_READ_CONTROL;
            droReadStatusAddr = baseAddr + DFX_SERVER_DRO_READ_STATUS;

            /*ENABLE AUTOMATIC MODE AND STRAT COUNT*/
            CHECK_STATUS(hwsServerRegSetFuncPtr(devNum, droReadControlAddr, 0x009320fb));
            hwsOsTimerWkFuncPtr(1000);

            for (i=0; i<251; i++)
            {
                CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, droReadStatusAddr, &regValue));
                regValue= regValue& 0xFFFF;
                if (i==0)
                {
                    minValue=regValue;
                    maxValue=regValue;
                }
                if (regValue > maxValue)
                {
                    maxValue = regValue;
                }
                else if (regValue < minValue)
                {
                    minValue= regValue;
                }

                counter = counter + regValue;
                hwsOsTimerWkFuncPtr(1000);
            }

            /*disable DRO read*/
            CHECK_STATUS(hwsServerRegSetFuncPtr(devNum, droReadControlAddr, 0x00400000));

            *droAvgPtr = counter / 251;
            *minValuePtr = minValue;
            *maxValuePtr = maxValue;
        }
        else if (unitNumber >= Eagle_0 && unitNumber <=Eagle_3 )
        {
            /* currntly not supported TBD*/
            return GT_NOT_SUPPORTED;
        }
        else
        {
            /* wrong unitNumber */
            return GT_NOT_SUPPORTED;
        }
    }
    else
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }
    return GT_OK;
}
#endif /* MV_HWS_REDUCED_BUILD */

/**
* @internal mvHwsDeviceTemperatureGet function
* @endinternal
*
* @brief   Get device's temperature.
*
* @param[in] devNum                   - system device number
* @param[out] temperaturePtr           - (pointer to) temperature in Celsius degrees
*                                      (can be negative)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsDeviceTemperatureGet
(
    IN GT_U8     devNum,
    OUT GT_32    *temperaturePtr
)
{

    if (NULL == temperaturePtr)
    {
        return GT_BAD_PTR;
    }

    switch (HWS_DEV_SILICON_TYPE(devNum))
    {
        case AC5:
            CHECK_STATUS(mvHwsTseneAuxadc12nmTempGet(devNum, temperaturePtr));
            break;
        default:
            return GT_NOT_IMPLEMENTED;
    }
    return GT_OK;
}

/**
* @internal mvHwsTseneAuxadc12nmTempGet function
* @endinternal
*
* @brief   Gets the PP temperature for devices with 12 nm sensor (a.k.a TSENE_AUXADC).
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon AC5P;
*         AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
*
* @param[out] temperaturePtr       - (pointer to) temperature in Celsius degrees
*                                      (can be negative)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_READY             - the temperature reading is not valid
*/
GT_STATUS mvHwsTseneAuxadc12nmTempGet
(
    IN GT_U8   devNum ,
    OUT GT_32  *tempInCelsiusPtr
)
{
    GT_U32 tmpData = 0;
    GT_32 hwValue_step1;

#ifdef MV_HWS_REDUCED_BUILD
    GT_U32 data = 0, mask = 0;
#endif
    /*
     * Write to External Temperature Sensor 12 nm Control 0 register
     * bit[0] : 0 (stop conversion/measurement active)
     */
#ifdef MV_HWS_REDUCED_BUILD
    /* External_Temperature_Sensor_12_nm_Control_0 = 0x840F80D0
       CM3 needs to read/write to DFX register through a window (remapping in sysmap.c) */
    tmpData = ((*((volatile unsigned int*)(0xb01F80D0)))); /* read */
    data = 0x0;
    mask = 0x1;
    tmpData = (tmpData & ~mask) | (data & mask);    /* modify*/
    ((*((volatile unsigned int*)(0xb01F80D0)))) = tmpData; /* write */
#else
    CHECK_STATUS(hwsServerRegFieldSetFuncPtr(devNum, BASE_DFX_SERVER_AC5_UNIT + DFX_SERVER_EXTERNAL_TEMPERATUE_SENSOR_12NM_CONTROL_0, 0 , 1, 0x0));
#endif
    /* wait 1 msec */
    hwsOsTimerWkFuncPtr(1);
    /*
     * Read External Temperature Sensor Readout field
     * bit[15:0] : temperature readout
     */
#ifdef MV_HWS_REDUCED_BUILD
    /* External_Temperature_Sensor_12_nm_Status = 0x840F80DC
       CM3 needs to read/write to DFX register through a window (remapping in sysmap.c) */
    tmpData = ((*((volatile unsigned int*)(0xb01F80DC))));
#else
    CHECK_STATUS(hwsServerRegFieldGetFuncPtr(devNum, BASE_DFX_SERVER_AC5_UNIT + DFX_SERVER_EXTERNAL_TEMPERATUE_SENSOR_12NM_STATUS, 0 , 16, &tmpData));
#endif

    /* convert the HW value to Celsius */
    /* Calculate:
        T(Celsius) = T(code) * TSENE_GAIN + TSENE_OFFSET
        where T(code) is bits [15:6] of data
        TSENE_OFFSET = -272.5 (default)
        TSENE_GAIN = 0.42 (default)
        T(Celsius) = T(code) * 0.42 - 272.5
    */

    /* get bits o[15:6] of data */
    hwValue_step1 =  (tmpData >> 6) & 0x3ff;/* actual 10 bits field ! */

    /* multiply on 100 to avoid integer rounding */
    *tempInCelsiusPtr = (hwValue_step1 * 42 - 27250) / 100;

    /*
     * Write below bits to External Temperature Sensor 12 nm Control 0 register
     * bit[0] : 1 (start conversion/measurement active)
     */
#ifdef MV_HWS_REDUCED_BUILD
    /* External_Temperature_Sensor_12_nm_Control_0 = 0x840F80D0
       CM3 needs to read/write to DFX register through a window (remapping in sysmap.c) */
    tmpData = ((*((volatile unsigned int*)(0xb01F80D0)))); /* read */
    data = 0x1;
    mask = 0x1;
    tmpData = (tmpData & ~mask) | (data & mask);    /* modify*/
    ((*((volatile unsigned int*)(0xb01F80D0)))) = tmpData; /* write */
#else
    CHECK_STATUS(hwsServerRegFieldSetFuncPtr(devNum, BASE_DFX_SERVER_AC5_UNIT + DFX_SERVER_EXTERNAL_TEMPERATUE_SENSOR_12NM_CONTROL_0, 0 , 1, 0x1));
#endif

    return GT_OK;
}
