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
* @file mvComphySerdesIf.c \
*
* @brief Comphy interface
*
* @version   1
********************************************************************************
*/

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#endif

#ifdef MV_HWS_REDUCED_BUILD_EXT_CM3
#define hwsOsPrintf osPrintf
#endif
#define HWS_COMPHY_GET_SERDES_INFO_FROM_SERDES_DEV(_serdesDevPtr)  ((MV_HWS_PER_SERDES_INFO_PTR)(_serdesDevPtr->appData))
#define HWS_COMPHY_GET_DEV_INFO_FROM_SERDES_DEV(_serdesDevPtr)     ((HWS_DEVICE_INFO*)((MV_HWS_PER_SERDES_INFO_PTR)(_serdesDevPtr->appData))->hostDevInfo)

#define HWS_COMPHY_GET_SERDES_DEV_FROM_SERDES_INFO(_serdesInfoPtr) ((MCESD_DEV_PTR)((_serdesInfoPtr->serdesData.comphySerdesData)->sDev))

#define HWS_COMPHY_CALC_MASK(fieldLen, fieldOffset, mask)    \
            if(((fieldLen) + (fieldOffset)) >= 32)           \
             (mask) = (GT_U32)(0 - (1<< (fieldOffset)));     \
            else                                             \
            (mask) = (((1<<((fieldLen) + (fieldOffset)))) - (1 << (fieldOffset)))

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
#define HWS_COMPHY_CHECK_SYSTEM_RECOVERY_STATUS(systemRecoveryInProgress)                                           \
            do{                                                                                                     \
                CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;                                              \
                systemRecoveryInProgress = GT_FALSE ;                                                               \
                cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);                                               \
                if ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&         \
                    (tempSystemRecovery_Info.systemRecoveryState != CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E))       \
                {                                                                                                   \
                    systemRecoveryInProgress = GT_TRUE;                                                             \
                }                                                                                                   \
            }while (0);
#else
#define HWS_COMPHY_CHECK_SYSTEM_RECOVERY_STATUS(systemRecoveryInProgress) systemRecoveryInProgress = GT_FALSE;
#endif

MCESD_STATUS mvHwsComphyRegisterWriteCallback(MCESD_DEV_PTR sDev, MCESD_U32 reg, MCESD_U32 value);
MCESD_STATUS mvHwsComphyRegisterReadCallback(MCESD_DEV_PTR sDev, MCESD_U32 reg, MCESD_U32 *value);
MCESD_STATUS mvHwsComphySetPinCallback(MCESD_DEV_PTR sDev, MCESD_U16 pin, MCESD_U16 pinValue);
MCESD_STATUS mvHwsComphyGetPinCallback(MCESD_DEV_PTR sDev, MCESD_U16 pin, MCESD_U16 *pinValue);
MCESD_STATUS mvHwsComphyWait(MCESD_DEV_PTR dev, MCESD_U32 ms);

#if defined  SHARED_MEMORY && !defined MV_HWS_REDUCED_BUILD

extern const MV_HWS_COMPHY_REG_FIELD_STC mvHwsComphyC28GP4X4PinToRegMapSdw[];
extern const MV_HWS_COMPHY_REG_FIELD_STC mvHwsComphyC28GP4X1PinToRegMapSdw[];
extern const MV_HWS_SERDES_TXRX_TUNE_PARAMS ac5xTxRxTuneParams[];
extern const MV_HWS_COMPHY_REG_FIELD_STC mvHwsComphyC28GP4X1PinToRegMapSdw[];
extern const MV_HWS_COMPHY_REG_FIELD_STC mvHwsComphyC112GX4PinToRegMapSdw[];
extern const MV_HWS_SERDES_TXRX_TUNE_PARAMS ac5pTxRxTuneParams[];


const MV_HWS_COMPHY_REG_FIELD_STC * mvHwsComphyPinToRegMapGet
(
    MV_HWS_DEV_TYPE                 devType,
    GT_U32                          serdesNum
)
{
    switch(devType)
    {
       case AC5X:
         if((serdesNum<=3)||
            (serdesNum>=8&&serdesNum<=11))
        {
            return mvHwsComphyC28GP4X4PinToRegMapSdw;
        }
        else
        {
            return mvHwsComphyC28GP4X1PinToRegMapSdw;
        }
        break;
      case AC5P:
        if(serdesNum == 32)
        {
            return mvHwsComphyC28GP4X1PinToRegMapSdw;
        }
        else
        {
            return mvHwsComphyC112GX4PinToRegMapSdw;
        }
       default:
        break;
    }

    return NULL;

}


const MV_HWS_SERDES_TXRX_TUNE_PARAMS * mvHwsComphyTxRxTuneParamsGet
(
    GT_U8   devNum,
    GT_UOPT serdesNum
)
{
    if((PRV_CPSS_PP_MAC(devNum)->devFamily)==CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        return ac5xTxRxTuneParams;
    }
    else if((PRV_CPSS_PP_MAC(devNum)->devFamily)==CPSS_PP_FAMILY_DXCH_AC5P_E)
    {
        if(serdesNum==32)
        {
            return ac5xTxRxTuneParams;
        }
        else
        {
          return ac5pTxRxTuneParams;
        }
    }

    return NULL;

}

#endif

/**
* @internal mvHwsComphyC12GP41P2VSerdesCheckAccess function
* @endinternal
*
* @brief   Check SerDes is accessible
*
* @param[in]  devNum                  - system device number
* @param[in]  portGroup               - port group (core) number
* @param[in]  serdesNum               - serdesNum
*
* @retval 0                           - on success
* @retval 1                           - on error
*/
GT_STATUS mvHwsComphyC12GP41P2VSerdesCheckAccess
(
    IN  GT_U8      devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum
)
{
    GT_UREG_DATA data;

    /* check analog reset */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                        SERDES_EXTERNAL_CONFIGURATION_1, &data, 0));

    if (((data >> 3) & 1) == 0)
        return GT_NOT_INITIALIZED;

    return GT_OK;
}

GT_STATUS mvHwsComphySerdesIndexInfoSDevGet
(
    IN  GT_U8                      devNum,
    IN  GT_UOPT                    serdesNum,
    OUT MV_HWS_PER_SERDES_INFO_PTR *serdesInfoPtr,
    OUT GT_U32                     *serdesIndexPtr,
    OUT MCESD_DEV_PTR              *sDevPtr

)
{
    MV_HWS_PER_SERDES_INFO_PTR serdesInfo;
    GT_U32                     serdesIndex;

    CHECK_STATUS(mvHwsSerdesGetInfoBySerdesNum(devNum,serdesNum,&serdesIndex,&serdesInfo));

    if(!HWS_COMPHY_SERDES_TYPE_CHECK(serdesInfo->serdesType))
    {
        return GT_NOT_SUPPORTED;
    }

    /* assign real value only if parameter is needed (parameter sent is not a NULL pointer) */
    if ( NULL !=  serdesInfoPtr )
    {
        *serdesInfoPtr = serdesInfo;
    }
    if ( NULL !=  serdesIndexPtr )
    {
        *serdesIndexPtr = serdesIndex;
    }

    if ( NULL != sDevPtr )
    {
        *sDevPtr = HWS_COMPHY_GET_SERDES_DEV_FROM_SERDES_INFO(serdesInfo);
        (*sDevPtr)->appData = (MCESD_PVOID)serdesInfo; /* Update the sDev's internal pointer appData to the correct serdesInfo */
    }

    return GT_OK;
}


MCESD_STATUS mvHwsComphyWait(MCESD_DEV_PTR dev, MCESD_U32 ms)
{
    GT_UNUSED_PARAM(dev);
    hwsOsTimerWkFuncPtr(ms);
    return MCESD_OK;
}


MCESD_STATUS mvHwsComphyRegisterWriteCallback(MCESD_DEV_PTR sDev, MCESD_U32 reg, MCESD_U32 value)
{
    GT_STATUS                    rc;
    MV_HWS_PER_SERDES_INFO_PTR   serdesInfo = HWS_COMPHY_GET_SERDES_INFO_FROM_SERDES_DEV(sDev);
    HWS_DEVICE_INFO*             devInfo    = HWS_COMPHY_GET_DEV_INFO_FROM_SERDES_DEV(sDev);
    GT_BOOL                      systemRecoveryInProgress;

    HWS_COMPHY_CHECK_SYSTEM_RECOVERY_STATUS(systemRecoveryInProgress)

    if(serdesInfo->serdesType == COM_PHY_C12GP41P2V)
    {
        rc = mvHwsComphyC12GP41P2VSerdesCheckAccess(devInfo->devNum, 0, serdesInfo->serdesNum);
        if (rc != GT_OK)
        {
            hwsOsPrintf("RegisterWriteCallback: access to serdes[%d] under reset, rc = %d \n",serdesInfo->serdesNum, rc);
            return MCESD_FAIL;
        }
        reg = 4 * reg; /* MCESD 12G driver uses jumps of 1 instead of 4 */
    }

    /*skip in case of HA */
    if (systemRecoveryInProgress == GT_TRUE)
    {
        return MCESD_OK;
    }

    comphyPrintf("[%02d]reg set-reg[0x%08x]-",serdesInfo->serdesNum,reg);

    rc = hwsSerdesRegSetFuncPtr(devInfo->devNum, 0, INTERNAL_REG,
                                serdesInfo->serdesNum,
                                (GT_UREG_DATA)reg, (GT_UREG_DATA)value, 0xFFFFFFFF);
    comphyPrintf("value[0x%08x]\n",value);

    return rc ? MCESD_FAIL : MCESD_OK;
}

MCESD_STATUS mvHwsComphyRegisterReadCallback(MCESD_DEV_PTR sDev, MCESD_U32 reg, MCESD_U32 *value)
{
    GT_UREG_DATA                 tempValue;
    GT_STATUS                    rc;
    MV_HWS_PER_SERDES_INFO_PTR   serdesInfo = HWS_COMPHY_GET_SERDES_INFO_FROM_SERDES_DEV(sDev);
    HWS_DEVICE_INFO*             devInfo    = HWS_COMPHY_GET_DEV_INFO_FROM_SERDES_DEV(sDev);

    if(serdesInfo->serdesType == COM_PHY_C12GP41P2V)
    {
        rc = mvHwsComphyC12GP41P2VSerdesCheckAccess(devInfo->devNum, 0, serdesInfo->serdesNum);
        if (rc != GT_OK)
        {
            hwsOsPrintf("RegisterReadCallback: access to serdes[%d] under reset, rc = %d \n",serdesInfo->serdesNum, rc);
            return MCESD_FAIL;
        }
        reg = 4 * reg; /* MCESD 12G driver uses jumps of 1 instead of 4 */
    }
    comphyPrintf("[%02d]reg get-reg[0x%08x]-",serdesInfo->serdesNum,reg);

    rc = hwsSerdesRegGetFuncPtr(devInfo->devNum, 0, INTERNAL_REG,
                                serdesInfo->serdesNum,
                                (GT_UREG_DATA)reg, &tempValue, 0xFFFFFFFF);

    comphyPrintf("value[0x%08x]\n",tempValue);

    *value = tempValue;
    return rc ? MCESD_FAIL : MCESD_OK;
}

static GT_STATUS mvHwsComphySelectLane
(
    IN  MCESD_DEV_PTR               sDev,
    IN  MV_HWS_PER_SERDES_INFO_PTR  serdesInfo,
    IN  GT_U32                      regAddr
)
{

#if defined(C28GP4X4) || defined(C112GX4) ||  defined(N5XC56GP5X4)
    MCESD_U32 tempRegVal=0, laneSelAddr=0, laneSelMask=0, maskOffset=0;

    if ((regAddr & 0x1FFFF) < 0x8000)
    {
        switch (serdesInfo->serdesType )
        {
            case COM_PHY_C12GP41P2V:
            case COM_PHY_C28GP4X1:
                return GT_OK;

            case COM_PHY_N5XC56GP5X4:
                laneSelAddr = 0xA318;
                maskOffset = 28;
                laneSelMask = 0xF<<maskOffset;
                break;

            case COM_PHY_C28GP4X4:
            case COM_PHY_C112GX4:
                laneSelAddr = 0xA314;
                maskOffset = 29;
                laneSelMask = 0x7<<maskOffset;
                break;

            default:
                return MCESD_FAIL;

        }
        MCESD_CHECK_STATUS(mvHwsComphyRegisterReadCallback(sDev, laneSelAddr, &tempRegVal));
        /* Modify */
        tempRegVal = (tempRegVal & ~laneSelMask) | (((MCESD_U32)(serdesInfo->internalLane << maskOffset)) & laneSelMask);
        /* Write */
        MCESD_CHECK_STATUS(mvHwsComphyRegisterWriteCallback(sDev, laneSelAddr, tempRegVal));

    }

#else
    GT_UNUSED_PARAM(sDev);
    GT_UNUSED_PARAM(serdesInfo);
    GT_UNUSED_PARAM(regAddr);
#endif
    return GT_OK;
}

/*#include <cpss/common/labServices/port/gop/port/serdes/comPhy/C112GX4/mcesdC112GX4_Defs.h>  TODO: Delete when impleneting SDW PinToReg table (the current implementation refers to x7121m) */
MCESD_STATUS mvHwsComphySetPinCallback(MCESD_DEV_PTR sDev, MCESD_U16 pin, MCESD_U16 pinValue)
{
    GT_STATUS                          rc;
    MV_HWS_PER_SERDES_INFO_PTR         serdesInfo = HWS_COMPHY_GET_SERDES_INFO_FROM_SERDES_DEV(sDev);
    HWS_DEVICE_INFO                    *devInfo   = HWS_COMPHY_GET_DEV_INFO_FROM_SERDES_DEV(sDev);
    MV_HWS_COMPHY_SERDES_DATA          *currSerdesData = serdesInfo->serdesData.comphySerdesData;
    GT_UREG_DATA                       reg, value, mask;
    GT_BOOL                            systemRecoveryInProgress;
    const MV_HWS_COMPHY_REG_FIELD_STC  *pinToRegMap;


    /* TODO: verify pin number */
    if(!currSerdesData)
    {
        comphyPrintf("[%02d]pin get-pin[  %8d]-value[0x%08x] failed\n",serdesInfo->serdesNum,pin,0);
        return MCESD_FAIL;
    }

#if defined  SHARED_MEMORY && !defined MV_HWS_REDUCED_BUILD

    /*dynamicly get the map*/
    pinToRegMap = mvHwsComphyPinToRegMapGet(devInfo->devType,serdesInfo->serdesNum);

     /*if not supported yet then use shared*/
    if(NULL==pinToRegMap)
    {
        pinToRegMap = currSerdesData->pinToRegMap;
    }
#else
    pinToRegMap = currSerdesData->pinToRegMap;
#endif
    /* TODO: verify pin number */
    if(MV_HWS_SERDES_REG_NA == pinToRegMap[pin].regOffset)
    {
        comphyPrintf("[%02d]pin set-pin[  %8d]-value[0x%08x] failed\n",serdesInfo->serdesNum,pin,pinValue);
        return MCESD_FAIL;
    }

    if(MV_HWS_SERDES_TIED_PIN == pinToRegMap[pin].regOffset)
    {
        if(pinValue == pinToRegMap[pin].fieldStart)
        {
            comphyPrintf("[%02d]pin set-pin[  %8d]-value[0x%08x] (tied pin)\n",serdesInfo->serdesNum, pin, pinValue);
            return MCESD_OK;
        }
        else
        { /* Tried to set value different from the default */
            comphyPrintf("[%02d]pin set-pin[  %8d]-value[0x%08x] (tied pin) failed\n",serdesInfo->serdesNum, pin, pinValue);
            return MCESD_FAIL;
        }
    }

    HWS_COMPHY_CHECK_SYSTEM_RECOVERY_STATUS(systemRecoveryInProgress)
    /*skip in case of HA */
    if (systemRecoveryInProgress == GT_TRUE )
    {
        return GT_OK;
    }
    reg   = pinToRegMap[pin].regOffset;
    value = ((GT_UREG_DATA)pinValue) <<pinToRegMap[pin].fieldStart;
    HWS_COMPHY_CALC_MASK(pinToRegMap[pin].fieldLen, pinToRegMap[pin].fieldStart, mask);

    comphyPrintf("[%02d]pin set-pin[  %8d]-", serdesInfo->serdesNum, pin);
    rc = hwsSerdesRegSetFuncPtr(devInfo->devNum, 0, EXTERNAL_REG,
                                                serdesInfo->serdesNum,
                                                reg, value, mask);
    comphyPrintf("value[0x%08x]\n",value);

    return rc ? MCESD_FAIL : MCESD_OK;
}

MCESD_STATUS mvHwsComphyGetPinCallback(MCESD_DEV_PTR sDev, MCESD_U16 pin, MCESD_U16 *pinValue)
{
    GT_STATUS                          rc;
    MV_HWS_PER_SERDES_INFO_PTR         serdesInfo  = HWS_COMPHY_GET_SERDES_INFO_FROM_SERDES_DEV(sDev);
    HWS_DEVICE_INFO                    *devInfo    = HWS_COMPHY_GET_DEV_INFO_FROM_SERDES_DEV(sDev);
    MV_HWS_COMPHY_SERDES_DATA          *currSerdesData = serdesInfo->serdesData.comphySerdesData;
    GT_UREG_DATA                       reg, value, mask;
    const MV_HWS_COMPHY_REG_FIELD_STC  *pinToRegMap;

        /* TODO: verify pin number */
    if(!currSerdesData)
    {
        comphyPrintf("[%02d]pin get-pin[  %8d]-value[0x%08x] failed\n",serdesInfo->serdesNum,pin,0);
        return MCESD_FAIL;
    }

#if defined  SHARED_MEMORY && !defined MV_HWS_REDUCED_BUILD

    /*dynamicly get the map*/
    pinToRegMap = mvHwsComphyPinToRegMapGet(devInfo->devType,serdesInfo->serdesNum);

     /*if not supported yet then use shared*/
    if(NULL==pinToRegMap)
    {
        pinToRegMap = currSerdesData->pinToRegMap;
    }
#else
    pinToRegMap = currSerdesData->pinToRegMap;
#endif

    /* TODO: verify pin number */
    if(MV_HWS_SERDES_REG_NA == pinToRegMap[pin].regOffset)
    {
        comphyPrintf("[%02d]pin get-pin[  %8d]-value[0x%08x] failed\n",serdesInfo->serdesNum,pin,0);
        return MCESD_FAIL;
    }

    if(MV_HWS_SERDES_TIED_PIN == pinToRegMap[pin].regOffset)
    {
        *pinValue = pinToRegMap[pin].fieldStart;
        comphyPrintf("[%02d]pin get-pin[  %8d]-value[0x%08x] (tied pin)\n",serdesInfo->serdesNum,pin,*pinValue);
        return MCESD_OK;
    }

    reg   = pinToRegMap[pin].regOffset;
    HWS_COMPHY_CALC_MASK(pinToRegMap[pin].fieldLen,pinToRegMap[pin].fieldStart, mask);

    /* in COM_PHY_N5XC56GP5X4 serdes we have a WA that write to internal registers and not to the pins*/
    if ((serdesInfo->serdesType == COM_PHY_N5XC56GP5X4) && (currSerdesData->pinToRegMap[pin].regOffset > 0x20000))
    {
        CHECK_STATUS(mvHwsComphySelectLane(sDev, serdesInfo, currSerdesData->pinToRegMap[pin].regOffset));
    }

    comphyPrintf("[%02d]pin get-pin[  %8d]-",serdesInfo->serdesNum,pin);
    rc = hwsSerdesRegGetFuncPtr(devInfo->devNum, 0, EXTERNAL_REG,
                                                serdesInfo->serdesNum,
                                                reg, &value, mask);
    *pinValue = (MCESD_U16)(value >> pinToRegMap[pin].fieldStart);
    comphyPrintf("value[0x%08x]\n",*pinValue);

    return rc ? MCESD_FAIL : MCESD_OK;
}
#define INVALID_LANE 0xffffffff
/**
* @internal mvHwsComphySerdesInit function
* @endinternal
*
* @brief   Init Comphy Serdes driver.
*
* @param[in] devNum           - system device number
* @param[in] serdesType       - serdesType (Comphy only)
*                           use SERDES_LAST to init all Comphy types
* @param[in] firmwareSelector   - FW version to download
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComphySerdesInit
(
    IN GT_U8                   devNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    IN GT_U8                   firmwareSelector,
    IN GT_BOOL                 callbackInitOnly
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      serdesIterator, masterLane = INVALID_LANE;
    MV_HWS_PER_SERDES_INFO_PTR  currSerdes;
    MV_HWS_COMPHY_SERDES_DATA   *currSerdesData;
    GT_BOOL                     systemRecoveryInProgress;
    MCESD_U8                    ipMajorRev = 0;
    MCESD_U8                    ipMinorRev = 0;
    if (NULL == HWS_DEV_SERDES_INFO(devNum).serdesesDb)
    {
        return GT_BAD_STATE;
    }
    HWS_COMPHY_CHECK_SYSTEM_RECOVERY_STATUS(systemRecoveryInProgress)

    for(serdesIterator = 0 ; serdesIterator < HWS_DEV_SERDES_INFO(devNum).numOfSerdeses ; serdesIterator++)
    {
        currSerdes = &HWS_DEV_SERDES_INFO(devNum).serdesesDb[serdesIterator];
        currSerdes->hostDevInfo = &hwsDeviceSpecInfo[devNum];

        if((serdesType == currSerdes->serdesType) || (serdesType == SERDES_LAST))
        {
            if(currSerdes->internalLane == 0)/* First lane of group of lanes - master lane */
            {
                IF_C12GP41P2V (currSerdes->serdesType, ipMajorRev = IP_MAJOR_C12GP41P2V;    ipMinorRev = IP_MINOR_C12GP41P2V;);
                IF_C28GP4X1   (currSerdes->serdesType, ipMajorRev = IP_MAJOR_C28GP4X1;      ipMinorRev = IP_MINOR_C28GP4X1;);
                IF_C28GP4X4   (currSerdes->serdesType, ipMajorRev = IP_MAJOR_C28GP4X4;      ipMinorRev = IP_MINOR_C28GP4X4;);
                IF_C112GX4    (currSerdes->serdesType, ipMajorRev = IP_MAJOR_C112GX4;       ipMinorRev = IP_MINOR_C112GX4;);
                IF_N5XC56GP5X4(currSerdes->serdesType, ipMajorRev = IP_MAJOR_N5XC56GP5X4;   ipMinorRev = IP_MINOR_N5XC56GP5X4;);

                currSerdesData = HWS_DEV_SERDES_INFO(devNum).serdesesDb[serdesIterator].serdesData.comphySerdesData;
                currSerdesData->sDev = (MCESD_DEV_PTR)hwsOsMallocFuncPtr(sizeof(MCESD_DEV));
                rc = (GT_STATUS)mcesdLoadDriver(
                            ipMajorRev,                      ipMinorRev,
                            mvHwsComphyRegisterReadCallback, mvHwsComphyRegisterWriteCallback,
                            mvHwsComphySetPinCallback,       mvHwsComphyGetPinCallback,
                            mvHwsComphyWait,
                            (MCESD_PVOID)  (currSerdes),
                            (MCESD_DEV_PTR)(currSerdesData->sDev));
                masterLane = serdesIterator;

                /*skip in case of HA init and catch up */
                if (systemRecoveryInProgress == GT_FALSE && callbackInitOnly == GT_FALSE )
                {
                    if(MV_HWS_COMPHY_FIRMWARE_NR != firmwareSelector)
                    {
                        /* TODO: Broadcast firmware */
                        rc = mvHwsSerdesFirmwareDownload(devNum, 0, currSerdes->serdesType, &currSerdes->serdesNum, 1, firmwareSelector);
                        if((rc != GT_OK) && (rc != GT_NOT_IMPLEMENTED))
                        {
                            return rc;
                        }
                    }
                }
            }
            else
            {
                if( masterLane != INVALID_LANE )
                {
                    /* Set the last internalSerdesNum==0 as the master lane of this current lane */
                    HWS_DEV_SERDES_INFO(devNum).serdesesDb[serdesIterator].serdesData.comphySerdesData->sDev =
                         HWS_DEV_SERDES_INFO(devNum).serdesesDb[masterLane].serdesData.comphySerdesData->sDev;
                }
                else
                {
                    return GT_BAD_STATE;
                }
            }
        }
    }
    return rc;
}

/**
* @internal mvHwsComphySerdesRegisterWrite function
* @endinternal
*
* @brief   Write to SerDes's internal register
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  regAddr   - register address
*                           for 12G :
*                           regAddr = (cider_base_address - 0x1000) / 4
* @param[in]  mask      - mask
* @param[in]  data      - value to set
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphySerdesRegisterWrite
(
    IN  GT_U8   devNum,
    IN  GT_UOPT portGroup,
    IN  GT_UOPT serdesNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  mask,
    IN  GT_U32  data
)
{
    MCESD_DEV_PTR               sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_U32                   tempRegVal;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    CHECK_STATUS(mvHwsComphySelectLane(sDev, serdesInfo, regAddr));

    /* Read */
    MCESD_CHECK_STATUS(mvHwsComphyRegisterReadCallback(sDev, (MCESD_U32)regAddr, &tempRegVal));

    /* Modify */
    tempRegVal = (tempRegVal & ~mask) | (data & mask);

    /* Write */
    return mvHwsComphyRegisterWriteCallback(sDev, (MCESD_U32)regAddr, (MCESD_U32)tempRegVal);
}

/**
* @internal mvHwsComphySerdesRegisterRead function
* @endinternal
*
* @brief   Read from SerDes's internal register
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  regAddr   - register address 
*                           for 12G :
*                           regAddr = (cider_base_address - 0x1000) / 4
* @param[in]  mask      - mask
* @param[out] data      - read value - can be NULL
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphySerdesRegisterRead
(
    IN  GT_U8   devNum,
    IN  GT_UOPT portGroup,
    IN  GT_UOPT serdesNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  mask,
    OUT GT_U32  *data
)
{
    MCESD_DEV_PTR               sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_U32                   tempRegVal;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    CHECK_STATUS(mvHwsComphySelectLane(sDev, serdesInfo, regAddr));

    MCESD_CHECK_STATUS(mvHwsComphyRegisterReadCallback(sDev, (MCESD_U32)regAddr, &tempRegVal));
    if(NULL != data)
    {
        *data = (tempRegVal & mask);
    }
    else
    {
        hwsOsPrintf("reg[0x%08x]=0x%08x\n",regAddr,(tempRegVal & mask));
    }
    return GT_OK;
}

/**
* @internal mvHwsComphySerdesRegisterWriteField function
* @endinternal
*
* @brief   Write to SerDes's internal register
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  regAddr   - register address 
*                           for 12G :
*                           regAddr = (cider_base_address - 0x1000) / 4
* @param[in]  fieldOffset - field's start bit number
* @param[in]  fieldLength - field's length
* @param[in]  data      - value to set
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphySerdesRegisterWriteField
(
    IN  GT_U8   devNum,
    IN  GT_UOPT portGroup,
    IN  GT_UOPT serdesNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  fieldOffset,
    IN  GT_U32  fieldLength,
    IN  GT_U32  data
)
{
    GT_U32 mask;

    GT_UNUSED_PARAM(portGroup);
    HWS_COMPHY_CALC_MASK(fieldLength, fieldOffset, mask);
    data = data << fieldOffset;
    return mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, regAddr, mask, data);
}

/**
* @internal mvHwsComphySerdesRegisterReadField function
* @endinternal
*
* @brief   Reads from SerDes's internal register
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  regAddr   - register address 
*                           for 12G :
*                           regAddr = (cider_base_address - 0x1000) / 4
* @param[in]  fieldOffset - field's start bit number
* @param[in]  fieldLength - field's length
* @param[out] data      - read value - can be NULL
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphySerdesRegisterReadField
(
    IN  GT_U8   devNum,
    IN  GT_UOPT portGroup,
    IN  GT_UOPT serdesNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  fieldOffset,
    IN  GT_U32  fieldLength,
    OUT GT_U32  *data
)
{
    GT_U32 mask;

    GT_UNUSED_PARAM(portGroup);
    HWS_COMPHY_CALC_MASK(fieldLength, fieldOffset, mask);
    CHECK_STATUS(mvHwsComphySerdesRegisterRead(devNum, portGroup, serdesNum, regAddr, mask, data));
    if(NULL != data)
    {
        *data = *data >> fieldOffset;
    }
    return GT_OK;
}

/**
* @internal mvHwsComphySerdesPinSet function
* @endinternal
*
* @brief   Set SerDes pin
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  pinNum    - pin number
* @param[in]  pinVal    - value to set
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphySerdesPinSet
(
    IN  GT_U8   devNum,
    IN  GT_UOPT portGroup,
    IN  GT_UOPT serdesNum,
    IN  GT_U16  pinNum,
    IN  GT_U16  pinVal
)
{
    MCESD_DEV_PTR sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    return mvHwsComphySetPinCallback(sDev, (MCESD_U16)pinNum, (MCESD_U16)pinVal);
}

/**
* @internal mvHwsComphySerdesPinSet function
* @endinternal
*
* @brief   Set SerDes pin
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  pinNum    - pin number
* @param[out]  pinVal   - pin value
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphySerdesPinGet
(
    IN  GT_U8   devNum,
    IN  GT_UOPT portGroup,
    IN  GT_UOPT serdesNum,
    IN  GT_U16  pinNum,
    OUT GT_U16  *pinVal
)
{
    MCESD_DEV_PTR              sDev;
    MCESD_U16                  tempPinVal;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    MCESD_CHECK_STATUS(mvHwsComphyGetPinCallback(sDev, (MCESD_U16)pinNum, &tempPinVal));
    if(NULL != pinVal)
    {
        *pinVal = tempPinVal;
    }
    else
    {
        hwsOsPrintf("pin[%d]=%d\n",pinNum,tempPinVal);
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyIfInit function
* @endinternal
*
* @brief   Init Comphy Serdes IF functions.
*
* @param[in] devNum                   - system device number
* @param[in] serdesType               - serdesType (Comphy only)
*                           use SERDES_LAST to init all Comphy types
* @param[in] funcPtrArray             - array for function registration
* @param[in] firmwareVersion   - FW version to download
* @param[in] callbackInitOnly - if GT_TRUE only initialize function pointers.

*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyIfInit
(
    IN GT_U8                   devNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray,
    IN GT_U8                   firmwareVersion,
    IN GT_BOOL                 callbackInitOnly
)
{
    GT_STATUS rc;

    if((!HWS_COMPHY_SERDES_TYPE_CHECK(serdesType) && (serdesType!=SERDES_LAST)) || (funcPtrArray == NULL))
    {
        return GT_BAD_PARAM;
    }

    IF_C12GP41P2V (serdesType, CHECK_STATUS(mvHwsComphyC12GP41P2VIfInit(funcPtrArray)););
#ifndef AC5_DEV_SUPPORT
    IF_C28GP4X1   (serdesType, CHECK_STATUS(mvHwsComphyC28GP4X1IfInit   (funcPtrArray)););
    IF_C28GP4X4   (serdesType, CHECK_STATUS(mvHwsComphyC28GP4X4IfInit   (funcPtrArray)););
    IF_C112GX4    (serdesType, CHECK_STATUS(mvHwsComphyC112GX4IfInit    (funcPtrArray)););
    IF_N5XC56GP5X4(serdesType, CHECK_STATUS(mvHwsComphyN5XC56GP5X4IfInit(funcPtrArray)););
#endif

    rc = mvHwsComphySerdesInit(devNum,serdesType,firmwareVersion,callbackInitOnly);
    if(rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

GT_STATUS mvHwsComphySerdesManualTxDBSet
(
    IN GT_U8                                       devNum,
    IN GT_UOPT                                     portGroup,
    IN GT_U32                                      phyPortNum,
    IN MV_HWS_SERDES_SPEED                         serdesSpeed,
    IN GT_UOPT                                     serdesNum,
    IN MV_HWS_SERDES_TX_CONFIG_DATA_UNT           *txComphyConfigPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MV_HWS_COMPHY_SERDES_DATA   *serdesData;
    GT_U8                       baudRateIterator0/*default param iterartor*/, baudRateIterator1/*override param iterartor*/, overrideSize /*override param size*/;

    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(phyPortNum);

    if (txComphyConfigPtr == NULL) {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,NULL));
    serdesData = HWS_COMPHY_GET_SERDES_DATA_FROM_SERDES_INFO(serdesInfo);
    overrideSize = MV_HWS_SERDES_TXRX_OVERRIDE_TUNE_PARAMS_ARR_SIZE;

    if (serdesData->tuneOverrideParams == NULL)
    {
        serdesData->tuneOverrideParams = (MV_HWS_SERDES_TXRX_TUNE_PARAMS*)hwsOsMallocFuncPtr(overrideSize*sizeof(MV_HWS_SERDES_TXRX_TUNE_PARAMS));
        if (NULL == serdesData->tuneOverrideParams)
        {
            return GT_NO_RESOURCE;
        }
        for (baudRateIterator1 = 0; baudRateIterator1 < overrideSize; baudRateIterator1++)
        {
            serdesData->tuneOverrideParams[baudRateIterator1].serdesSpeed = LAST_MV_HWS_SERDES_SPEED;
        }
    }

    /*Find default param for the given serded speed*/
    for(baudRateIterator0 = 0 ; serdesData->tuneParams[baudRateIterator0].serdesSpeed != LAST_MV_HWS_SERDES_SPEED ; baudRateIterator0++)
    {
        if(serdesData->tuneParams[baudRateIterator0].serdesSpeed == serdesSpeed) break;
    }
    if(serdesData->tuneParams[baudRateIterator0].serdesSpeed == LAST_MV_HWS_SERDES_SPEED)
    {
        return GT_NOT_FOUND;
    }

    for(baudRateIterator1 = 0 ; baudRateIterator1 < overrideSize; baudRateIterator1++)
    {
        if(serdesData->tuneOverrideParams[baudRateIterator1].serdesSpeed == serdesSpeed)
        {
            break;
        }
        else if (serdesData->tuneOverrideParams[baudRateIterator1].serdesSpeed == LAST_MV_HWS_SERDES_SPEED)
        {
            serdesData->tuneOverrideParams[baudRateIterator1].serdesSpeed = serdesSpeed;
            /*Set default values for the RX; TX will be set with override values*/
            hwsOsMemCopyFuncPtr(&(serdesData->tuneOverrideParams[baudRateIterator1].rxParams),
                                &(serdesData->tuneParams[baudRateIterator0].rxParams),
                                sizeof(MV_HWS_SERDES_RX_CONFIG_DATA_UNT));
            break;
        }
    }
    if (baudRateIterator1 == overrideSize)
    {
        return GT_NOT_FOUND;
    }
    hwsOsMemCopyFuncPtr(&(serdesData->tuneOverrideParams[baudRateIterator1].txParams),
                        txComphyConfigPtr,
                        sizeof(MV_HWS_SERDES_TX_CONFIG_DATA_UNT));

    return GT_OK;
}

GT_STATUS mvHwsComphySerdesManualRxDBSet
(
    IN GT_U8                                               devNum,
    IN GT_UOPT                                             portGroup,
    IN GT_U32                                              phyPortNum,
    IN MV_HWS_SERDES_SPEED                                 serdesSpeed,
    IN GT_UOPT                                             serdesNum,
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT                   *rxComphyConfigPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MV_HWS_COMPHY_SERDES_DATA   *serdesData;
    GT_U8                       baudRateIterator0/*default param iterartor*/, baudRateIterator1/*override param iterartor*/, overrideSize /*override param size*/;
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(phyPortNum);

    if (rxComphyConfigPtr == NULL) {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,NULL));
    serdesData = HWS_COMPHY_GET_SERDES_DATA_FROM_SERDES_INFO(serdesInfo);
    overrideSize = MV_HWS_SERDES_TXRX_OVERRIDE_TUNE_PARAMS_ARR_SIZE;

    if (serdesData->tuneOverrideParams == NULL)
    {
        serdesData->tuneOverrideParams = (MV_HWS_SERDES_TXRX_TUNE_PARAMS*)hwsOsMallocFuncPtr(overrideSize*sizeof(MV_HWS_SERDES_TXRX_TUNE_PARAMS));
        if (NULL == serdesData->tuneOverrideParams)
        {
            return GT_NO_RESOURCE;
        }
        for (baudRateIterator1 = 0; baudRateIterator1 < overrideSize; baudRateIterator1++)
        {
            serdesData->tuneOverrideParams[baudRateIterator1].serdesSpeed = LAST_MV_HWS_SERDES_SPEED;
        }
    }

    /*Find default param for the given serded speed*/
    for(baudRateIterator0 = 0 ; serdesData->tuneParams[baudRateIterator0].serdesSpeed != LAST_MV_HWS_SERDES_SPEED ; baudRateIterator0++)
    {
        if(serdesData->tuneParams[baudRateIterator0].serdesSpeed == serdesSpeed) break;
    }
    if(serdesData->tuneParams[baudRateIterator0].serdesSpeed == LAST_MV_HWS_SERDES_SPEED)
    {
        return GT_NOT_FOUND;
    }

    for(baudRateIterator1 = 0 ; baudRateIterator1 < overrideSize; baudRateIterator1++)
    {
        if(serdesData->tuneOverrideParams[baudRateIterator1].serdesSpeed == serdesSpeed)
        {
            break;
        }
        else if (serdesData->tuneOverrideParams[baudRateIterator1].serdesSpeed == LAST_MV_HWS_SERDES_SPEED)
        {
            serdesData->tuneOverrideParams[baudRateIterator1].serdesSpeed = serdesSpeed;
            /*Set default values for the TX; RX will be set with override values*/
            hwsOsMemCopyFuncPtr(&(serdesData->tuneOverrideParams[baudRateIterator1].txParams),
                                &(serdesData->tuneParams[baudRateIterator0].txParams),
                                sizeof(MV_HWS_SERDES_TX_CONFIG_DATA_UNT));
            break;
        }
    }
    if (baudRateIterator1 == overrideSize)
    {
        return GT_NOT_FOUND;
    }

    hwsOsMemCopyFuncPtr(&(serdesData->tuneOverrideParams[baudRateIterator1].rxParams),
                        rxComphyConfigPtr,
                        sizeof(MV_HWS_SERDES_RX_CONFIG_DATA_UNT));
    return GT_OK;
}

GT_STATUS mvHwsComphySerdesManualDBClear
(
    IN GT_U8                                devNum,
    IN GT_U32                               phyPortNum,
    IN GT_UOPT                              serdesNum
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MV_HWS_COMPHY_SERDES_DATA   *serdesData;
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(phyPortNum);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,NULL));
    serdesData = HWS_COMPHY_GET_SERDES_DATA_FROM_SERDES_INFO(serdesInfo);

    if (serdesData->tuneOverrideParams != NULL)
    {
        hwsOsFreeFuncPtr(serdesData->tuneOverrideParams);
        serdesData->tuneOverrideParams = NULL;
    }

    return GT_OK;
}

#ifndef MV_HWS_REDUCED_BUILD
/**
* @internal mvHwsComphySerdesAddToDatabase function
* @endinternal
*
* @brief   Add SerDes to the main SerDeses database. Allocate
*          the database if needed (in the first call) Note:
*          before the firs call, the hwsDeviceSpecInfo must be
*          initialized with the correct numOfSerdeses and
*          serdesesDb=NULL
*
* @param[in] devNum                   - system device number
* @param[in] serdesIndex              - index in the table where
*                                       to add the new SerDes
* @param[in] serdesInfoPrototype      - a prototype of the
*                                       SerDes record to add
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphySerdesAddToDatabase
(
    IN GT_U8                                devNum,
    IN GT_U32                               serdesIndex,
    IN const MV_HWS_PER_SERDES_INFO_STC     *serdesInfoPrototype
)
{
#ifndef MV_HWS_REDUCED_BUILD
    if (hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses == 0)
    {
        return GT_BAD_SIZE;
    }
    if (serdesIndex >= hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses)
    {
        return GT_OUT_OF_RANGE;
    }

    /* Allocate SerDeses database inside the device info database if needed */
    if(hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb == NULL)
    {
        hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb =
             (MV_HWS_PER_SERDES_INFO_STC*)hwsOsMallocFuncPtr(hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses * sizeof(MV_HWS_PER_SERDES_INFO_STC));
        if (NULL == hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb)
        {
            return GT_NO_RESOURCE;
        }
    }
    hwsOsMemCopyFuncPtr(&hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[serdesIndex], serdesInfoPrototype, sizeof(MV_HWS_PER_SERDES_INFO_STC));

    /* Malloc and copy the SerDes data*/
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[serdesIndex].serdesData.comphySerdesData =
             (MV_HWS_COMPHY_SERDES_DATA*)hwsOsMallocFuncPtr(sizeof(MV_HWS_COMPHY_SERDES_DATA));
    if (NULL == hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[serdesIndex].serdesData.comphySerdesData)
    {
        return GT_NO_RESOURCE;
    }
    hwsOsMemCopyFuncPtr(hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[serdesIndex].serdesData.comphySerdesData, serdesInfoPrototype->serdesData.comphySerdesData, sizeof(MV_HWS_COMPHY_SERDES_DATA));
#else
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesIndex);
    GT_UNUSED_PARAM(serdesInfoPrototype);
#endif
    return GT_OK;
}

/**
* @internal mvHwsComphySerdesDestroyDatabase function
* @endinternal
*
* @brief   Free allocated memory
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphySerdesDestroyDatabase
(
    IN GT_U8  devNum
)
{
#ifndef MV_HWS_REDUCED_BUILD
    GT_U32 serdesIndex;

    if ((hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses == 0)||(hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb == NULL))
    {
        return GT_BAD_STATE;
    }
    for (serdesIndex = 0; serdesIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses ; serdesIndex++)
    {
        hwsOsFreeFuncPtr(hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[serdesIndex].serdesData.comphySerdesData);
    }
    hwsOsFreeFuncPtr(hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb);
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb = NULL;
#else
    GT_UNUSED_PARAM(devNum);
#endif
    return GT_OK;
}
#endif


