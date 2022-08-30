/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* mvHwsFalconDevInit.c
*
* DESCRIPTION:
*     Falcon specific HW Services init
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>

#include <cpss/generic/labservices/port/gop/silicon/falcon/mvHwsFalconPortIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>

#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralPortIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralCpll.h>

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mti100Mac/mvHwsMti100MacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mti400Mac/mvHwsMti400MacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mtiCpuMac/mvHwsMtiCpuMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/d2dMac/mvHwsD2dMacIf.h>

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs400If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs200If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs100If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs50If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiLowSpeedPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiCpuPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/d2dPcs/mvHwsD2dPcsIf.h>

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mv_hws_avago_if.h>
#include <cpss/common/labServices/port/gop/port/serdes/gw16/mvGw16If.h>

#include <cpssCommon/private/prvCpssEmulatorMode.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#ifndef MICRO_INIT
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#endif
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#endif

#if (! defined MICRO_INIT) && (! defined MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif

#if (defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #define AVAGO_NOT_EXISTS
#endif

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

#define RAVEN_SHIFT_NIBBLE 0xF000000

GT_STATUS mvHwsFalconExtPowerReductionInit
(
    IN GT_U8    devNum,
    IN GT_U32   portNum,
    IN CPSS_HW_DRIVER_STC  *ssmiPtr
);
#ifndef MICRO_INIT
static GT_STATUS hwsFalconInterruptsD2dClear
(
    IN  GT_U8                   devNum
);
#endif
GT_BOOL hwsFalconZ2Mode = GT_FALSE;
#ifndef MICRO_INIT
MV_OS_GET_DEVICE_DRIVER_FUNC        hwsGetDeviceDriverFuncPtr = NULL;
#endif

#ifdef MV_HWS_REDUCED_BUILD
HWS_UNIT_BASE_ADDR_CALC_BIND    falconBaseAddrCalcFunc = NULL;
#define PRV_NON_SHARED_LAB_SERVICES_DIR_FALCON_DEV_INIT_SRC_GLOBAL_VAR(_var) _var
#else
#define PRV_NON_SHARED_LAB_SERVICES_DIR_FALCON_DEV_INIT_SRC_GLOBAL_VAR(_var) \
    PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.labServicesDir.falconDevinitSrc._var)
#endif

#ifdef MICRO_INIT
#define PRV_CPSS_SW_PTR_ENTRY_UNUSED     0xFFFFFFFB
#endif

/************************* definition *****************************************************/

#define RAVEN_SERDES_MAP_ARR_SIZE           21
static GT_U32 falconSerdesToAvagoMap[RAVEN_SERDES_MAP_ARR_SIZE] =
{
/* laneNum   SBC_Addr */
 /* [0]  */      1,
 /* [1]  */      2,
 /* [2]  */      3,
 /* [3]  */      4,
 /* [4]  */      5,
 /* [5]  */      6,
 /* [6]  */      7,
 /* [7]  */      8,
 /* [8]  */      9,
 /* [9]  */      10,
 /* [10] */      11,
 /* [11] */      12,
 /* [12] */      13,
 /* [13] */      14,
 /* [14] */      15,
 /* [15] */      16,
 /* [16] */      17,
 /* [17] */      18,    /* TSEN */
 /* [18] */      19,    /* PMRO */
 /* [19] */      0xFD   /* SBM */

};

GT_U32 falconAvagoToSerdesMap[RAVEN_SERDES_MAP_ARR_SIZE] =
{
/* SBC_Serdes  laneNum */
/* [ 0]  */       0, /*NOT USED*/
/* [ 1]  */       0,
/* [ 2]  */       1,
/* [ 3]  */       2,
/* [ 4]  */       3,
/* [ 5]  */       4,
/* [ 6]  */       5,
/* [ 7]  */       6,
/* [ 8]  */       7,
/* [ 9]  */       8,
/* [10]  */       9,
/* [11] */        10,
/* [12] */        11,
/* [13] */        12,
/* [14] */        13,
/* [15] */        14,
/* [16] */        15,
/* [17] */        16,
/* TSEN=18 */     0,
/* PMRO=19 */     0,
/* SBM=0xFD */    19

};

/************************* Globals *******************************************************/
/* trace code */
#ifndef MICRO_INIT
typedef GT_STATUS (*TRACE_FUNC)(IN GT_BOOL enable);
static TRACE_FUNC falcon_dev_init_trace_func = NULL;
/* function called from the appDemo */
GT_STATUS   falcon_dev_init_trace_bind( IN TRACE_FUNC trace_func)
{
    falcon_dev_init_trace_func = trace_func;

    return GT_OK;
}
#endif
extern MV_HWS_PORT_INIT_PARAMS *hwsPortsFalconParams[];

extern const MV_HWS_PORT_INIT_PARAMS **hwsPortsFalconParamsSupModesMap;

/* init per device */
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL falconDbInitDone = GT_FALSE;
static GT_U32 hwsRavenCm3UartDev = 0xFFFF;/* non of the raven device cm3 is connected to uart*/
GT_U32  hwsFalconNumOfRavens = 0;
GT_U32  hwsRavenDevBmp = 0xFFFF; /* used for falcon Z2 if we want to use only several ravens and not all */
GT_STATUS hwsFalconInitStaticSerdesMuxingValidation
(
    IN GT_U8 devNum
);

/************************* pre-declaration ***********************************************/


/*****************************************************************************************/

/*******************************************************************************
* hwsPcsIfInit
*
* DESCRIPTION:
*       Init all supported PCS types relevant for devices.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static GT_STATUS pcsIfInit(GT_U8 devNum)
{
    MV_HWS_PCS_FUNC_PTRS **hwsPcsFuncsPtr;

    CHECK_STATUS(hwsPcsGetFuncPtr(devNum, &hwsPcsFuncsPtr));

    CHECK_STATUS(mvHwsD2dPcsIfInit(devNum, hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMtiPcs50IfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMtiPcs100IfInit(hwsPcsFuncsPtr));
#ifndef MICRO_INIT
    CHECK_STATUS(mvHwsMtiPcs200IfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMtiPcs400IfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMtiCpuPcsIfInit(hwsPcsFuncsPtr));
#endif
    CHECK_STATUS(mvHwsMtiLowSpeedPcsIfInit(hwsPcsFuncsPtr));

    return GT_OK;
}

/*******************************************************************************
* hwsMacIfInit
*
* DESCRIPTION:
*       Init all supported MAC types.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static GT_STATUS macIfInit(GT_U8 devNum)
{
    MV_HWS_MAC_FUNC_PTRS **hwsMacFuncsPtr;

    CHECK_STATUS(hwsMacGetFuncPtr(devNum, &hwsMacFuncsPtr));

    CHECK_STATUS(hwsD2dMacIfInit(devNum, hwsMacFuncsPtr));
    CHECK_STATUS(mvHwsMti100MacIfInit(hwsMacFuncsPtr));
#ifndef MICRO_INIT
    CHECK_STATUS(mvHwsMti400MacIfInit(hwsMacFuncsPtr));
    CHECK_STATUS(mvHwsMtiCpuMacIfInit(hwsMacFuncsPtr));
#endif
    return GT_OK;
}

/*******************************************************************************
* hwsFalconSerdesIfInit
*
* DESCRIPTION:
*       Init all supported Serdes types.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static GT_STATUS hwsFalconSerdesIfInit(GT_U8 devNum, GT_U32 ravenDevBmp)
{
    MV_HWS_SERDES_FUNC_PTRS **hwsSerdesFuncsPtr;
#if !defined (AVAGO_NOT_EXISTS) && !defined (MV_HWS_REDUCED_BUILD_EXT_CM3)
    CHECK_STATUS(mvHwsAaplSerdesDbInit(devNum, ravenDevBmp));
#endif
    CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));

    CHECK_STATUS(mvHwsAvago16nmIfInit(devNum, ravenDevBmp, hwsSerdesFuncsPtr));

    CHECK_STATUS(mvHwsD2dPhyIfInit(devNum, hwsSerdesFuncsPtr));

    return GT_OK;
}

/**
* @internal hwsFalconNumOfTilesUpdate function
* @endinternal
 *
*/
#ifndef MV_HWS_REDUCED_BUILD
GT_STATUS hwsFalconNumOfTilesUpdate
(
    GT_U8 devNum,
    GT_U32 numOfTiles
)
{
    hwsDeviceSpecInfo[devNum].numOfTiles = numOfTiles;
    return GT_OK;
}
#endif
/**
* @internal hwsFalconIfPreInit function
* @endinternal
 *
*/
GT_STATUS hwsFalconIfPreInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
    GT_U32 i;

    if (funcPtr == NULL)
        return GT_FAIL;

    if ((funcPtr->osTimerWkPtr == NULL) || (funcPtr->osMemSetPtr == NULL) ||
        (funcPtr->osFreePtr == NULL) || (funcPtr->osMallocPtr == NULL) ||
        (funcPtr->osExactDelayPtr == NULL) || (funcPtr->sysDeviceInfo == NULL) ||
        (funcPtr->osMemCopyPtr == NULL) ||
        (funcPtr->serdesRegSetAccess == NULL) ||(funcPtr->serdesRegGetAccess == NULL) ||
        (funcPtr->serverRegSetAccess == NULL) || (funcPtr->serverRegGetAccess == NULL) ||
        (funcPtr->registerSetAccess == NULL) || (funcPtr->registerGetAccess == NULL)
#ifndef MV_HWS_FREE_RTOS
        || (funcPtr->timerGet == NULL)
#endif
        )
    {
        return GT_BAD_PTR;
    }

    hwsOsExactDelayPtr = funcPtr->osExactDelayPtr;
    hwsOsTimerWkFuncPtr = funcPtr->osTimerWkPtr;
    hwsOsMemSetFuncPtr = funcPtr->osMemSetPtr;
    hwsOsFreeFuncPtr = funcPtr->osFreePtr;
    hwsOsMallocFuncPtr = funcPtr->osMallocPtr;
    hwsSerdesRegSetFuncPtr = funcPtr->serdesRegSetAccess;
    hwsSerdesRegGetFuncPtr = funcPtr->serdesRegGetAccess;
    hwsServerRegSetFuncPtr = funcPtr->serverRegSetAccess;
    hwsServerRegGetFuncPtr = funcPtr->serverRegGetAccess;
    hwsOsMemCopyFuncPtr = funcPtr->osMemCopyPtr;
    hwsServerRegFieldSetFuncPtr = funcPtr->serverRegFieldSetAccess;
    hwsServerRegFieldGetFuncPtr = funcPtr->serverRegFieldGetAccess;
#ifndef MV_HWS_FREE_RTOS
    hwsTimerGetFuncPtr = funcPtr->timerGet;
#endif
    hwsRegisterSetFuncPtr = funcPtr->registerSetAccess;
    hwsRegisterGetFuncPtr = funcPtr->registerGetAccess;

    hwsDeviceSpecInfo[devNum].devType = Falcon;
#ifndef MICRO_INIT
    hwsGetDeviceDriverFuncPtr = ((HWS_EXT_FUNC_STC_PTR*)funcPtr->extFunctionStcPtr)->getDeviceDriver;
    hwsPpHwTraceEnablePtr = ((HWS_EXT_FUNC_STC_PTR*)funcPtr->extFunctionStcPtr)->ppHwTraceEnable;
#ifndef ASIC_SIMULATION
    if (hwsGetDeviceDriverFuncPtr == NULL) {
        return GT_BAD_PTR;
    }
#endif
#endif

    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }

    hwsDeviceSpecInfo[devNum].devType = Falcon;

    return GT_OK;
}

#ifdef SHARED_MEMORY

GT_STATUS hwsFalconIfReIoad
(
    GT_U8 devNum
)
{
    CHECK_STATUS(macIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));

    return GT_OK;
}

#endif

#ifndef MICRO_INIT


GT_VOID hwsFalconPortsParamsSupModesMapSet
(
    GT_U8 devNum
)
{
    PRV_PORTS_PARAMS_SUP_MODE_MAP(devNum) = hwsPortsFalconParamsSupModesMap;
}

#endif

GT_VOID hwsFalconIfInitHwsDevFunc
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc == NULL)
    {
        hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc = funcPtr->sysDeviceInfo;
    }

#ifndef MV_HWS_FREE_RTOS
    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].coreClockGetFunc == NULL)
    {
        hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].coreClockGetFunc = funcPtr->coreClockGetPtr;
    }
#endif


    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc = NULL;

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc = NULL;

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc = NULL;

    /* Configures port init / reset functions */
#if (!defined MV_HWS_REDUCED_BUILD) || defined (MICRO_INIT)
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc = mvHwsFalconPortInit;
#endif
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devApPortinitFunc = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortResetFunc = mvHwsFalconPortReset ;

    /* Configures  device handler functions */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetExtFunc = mvHwsPortAvagoAutoTuneSetExt;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgFunc     = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgGetFunc  = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsIndexGetFunc = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portLbSetWaFunc        = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneStopFunc   = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetWaFunc  = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portFixAlign90Func     = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsSetFunc      = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsGetLanesFunc = hwsFalconPortParamsGetLanes;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortAccessCheckFunc = mvHwsMtipExtMacAccessCheck;
}

/*******************************************************************************
* hwsFalconIfInit
*
* DESCRIPTION:
*       Init all supported units needed for port initialization.
*       Must be called per device.
*
* INPUTS:
*       funcPtr - pointer to structure that hold the "os"
*                 functions needed be bound to HWS.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS hwsFalconIfInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
#ifndef MICRO_INIT
    GT_U32  data, version;
    GT_U32 portNum;
#endif
    MV_HWS_DEV_TYPE devType;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;


#ifdef MICRO_INIT
    GT_U8 cpuIdIndex;
#endif

#ifndef MV_HWS_REDUCED_BUILD
    GT_U8 cpuIdIndex;
    GT_U32 regAddr;
    GT_U32 baseAddr, unitNum, unitIndex;

    MV_HWS_SERDES_CONFIG_STC serdesConfigPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
#endif

    if (hwsInitDone[devNum])
    {
        return GT_ALREADY_EXIST;
    }

#ifdef MV_HWS_REDUCED_BUILD
    if(falconDbInitDone == GT_FALSE)
    {
        CHECK_STATUS(hwsFalconIfPreInit(devNum, funcPtr));
    }
#endif

    hwsDeviceSpecInfo[devNum].devType = Falcon;
    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = LAST_PORT_MODE;
    hwsDeviceSpecInfo[devNum].serdesType = AVAGO_16NM;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesNumber = hwsFalconNumOfRavens * 17;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.spicoNumber = RAVEN_SERDES_MAP_ARR_SIZE;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesToAvagoMapPtr = &falconSerdesToAvagoMap[0];
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.avagoToSerdesMapPtr = &falconAvagoToSerdesMap[0];
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesVosOverride = GT_FALSE;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.sbusAddrToSerdesFuncPtr = NULL;
#ifndef MICRO_INIT
    CHECK_STATUS(hwsFalconPortElementsDbInit(devNum));
    hwsFalconPortsParamsSupModesMapSet(devNum);
#endif

#ifdef MICRO_INIT
    /* MI does not support CPU ports */
    hwsDeviceSpecInfo[devNum].portsNum = hwsFalconNumOfRavens * 16;
    hwsDeviceSpecInfo[devNum].numOfTiles = hwsFalconNumOfRavens / 4;
#else
    /* each Raven hold 16 ports + CPU port */
    hwsDeviceSpecInfo[devNum].portsNum = hwsFalconNumOfRavens * 17;
#endif
    hwsRavenDevBmp                    &= (1<<hwsFalconNumOfRavens)-1;
    devType = hwsDeviceSpecInfo[devNum].devType;

    hwsFalconIfInitHwsDevFunc(devNum, funcPtr);

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

#ifndef MICRO_INIT
    /* get device id and version */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc(devNum, &data, &version);
#endif
    hwsDeviceSpecInfo[devNum].gopRev = GOP_16NM_REV1;


#ifndef MICRO_INIT
#ifndef MV_HWS_REDUCED_BUILD
    /* define IPC connection Type Host side */
    if (mvHwsServiceCpuEnableGet(devNum))
    {
        /* AP FW is loaded to Service CPU, use IPC mode - API execution at the Service CPU */
        /* !!! Replace HOST2HWS_LEGACY_MODE with HOST2SERVICE_CPU_IPC_CONNECTION once approved by CPSS !!! */
        /*hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE*//*HOST2SERVICE_CPU_IPC_CONNECTION*/;
        /* Init HWS Firmware */
        for (cpuIdIndex = 0; cpuIdIndex < hwsFalconNumOfRavens; cpuIdIndex ++)
        {
            if (hwsRavenDevBmp & (1<<cpuIdIndex))
            {
                /* open uart connection to cm3 only for the relavant device  - used for debug*/
                if (hwsRavenCm3UartDev == cpuIdIndex)
                {
                    hwsOsPrintf("hwsFalconIfInit: CM3 in Raven %d is connected to UART\n",cpuIdIndex);
                    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_BASE_UNIT, cpuIdIndex, &baseAddr, &unitIndex, &unitNum));
                    regAddr = baseAddr + MPP_CONTROL_8_15;
                    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0x22000, 0xFF000));
/*                    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, baseAddr+0x118210, 0x4, 0x0));*/
                }
#ifndef ASIC_SIMULATION
                CHECK_STATUS(mvHwsServiceCpuFwInit(devNum, cpuIdIndex, "AP_Raven"));
#endif
            }
        }
        hwsOsPrintf("finish loading AP fw\n");
    }
#endif /* MV_HWS_REDUCED_BUILD */
#endif

    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** Falcon ifInit start ******\n");
    }
#endif

    hwsInitPortsModesParam(devNum,NULL);
    devType = devType; /* avoid warning */
    if(hwsInitDone[devNum] == GT_FALSE)
    {
#ifndef MV_HWS_REDUCED_BUILD
        CHECK_STATUS(hwsDevicePortsElementsCfg(devNum));
#endif
    }

    /* Init SERDES unit relevant for current device */
    CHECK_STATUS(hwsFalconSerdesIfInit(devNum, hwsRavenDevBmp));

    /* Init all MAC units relevant for current device */
    CHECK_STATUS(macIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));
    /* Update HWS After Firmware load and init */
#ifndef MICRO_INIT
    if(!cpssDeviceRunCheck_onEmulator())
#endif
    {
        if (mvHwsServiceCpuEnableGet(devNum))
        {
            for (cpuIdIndex = 0; cpuIdIndex < hwsFalconNumOfRavens; cpuIdIndex++)
            {
                if (hwsRavenDevBmp & (1<<cpuIdIndex))
                {
                    CHECK_STATUS(mvHwsServiceCpuFwPostInit(devNum, cpuIdIndex));
                }
            }
        }
    }

    /* Init configuration sequence executer */
    mvCfgSeqExecInit();
    falconDbInitDone = GT_TRUE;
    hwsInitDone[devNum] = GT_TRUE;
#ifndef MICRO_INIT
    /************************************************************/
    /* Power reduction feature - Raven MTI EXT units reset      */
    /************************************************************/
    CHECK_STATUS(cpssSystemRecoveryStateGet(&tempSystemRecovery_Info));
    if (!((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
          (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E)) )
    {
        /* in Falcon typed 2T/3.2T/4T we turn off unused ravens in order to reduce power consumption. */
        for (portNum = 0; portNum < (hwsDeviceSpecInfo[devNum].portsNum - 2); portNum += 8)
        {
            /* bypass ports of excluded ravens */
            if (   ((portNum / 16) < hwsFalconNumOfRavens)
                && ((hwsRavenDevBmp & (1 << (portNum / 16))) != 0)  )
            {
                CHECK_STATUS(mvHwsFalconExtPowerReductionInit(devNum, portNum, NULL));

                /* disable PCI WD from global metal fix register*/
                CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
                regAddr = baseAddr + MTIP_EXT_GLOBAL_METAL_FIX;
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0, 0x80000000));
            }

        }

    /* in Falcon typed 2T/3.2T/4T we turn off unused ravens in order to refuce power consumption.
       Note : this is a 6.4T Falcon reduced to 2T/3.2T/4T */
        if ( CPSS_FALCON_REDUCED_2T_4T_3_2T_DEVICES_CHECK_MAC(data) ||
             /* 8T device based on 12.8T */(data == CPSS_98CX8550_CNS))
        {
            for ( portNum = 0; portNum < (hwsDeviceSpecInfo[devNum].portsNum - 2); portNum += 8 )
            {
                /* bypass ports of included ravens - turn off excluded ravens */
                if (   ((portNum / 16) < hwsFalconNumOfRavens)
                    && ((hwsRavenDevBmp & (1 << (portNum / 16))) == 0)  )
                {
                    CHECK_STATUS(mvHwsFalconExtPowerReductionInit(devNum, portNum, PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->chipletDrivers[portNum/16]));
                }
             }
        }
    }
#endif
    /*************************************************************************/
    /* CPU port power reduction feature - Raven CPU MTI EXT units reset      */
    /*************************************************************************/
    CPSS_TBD_BOOKMARK_FALCON
#if 0
    for (portNum = 0; portNum < 2; portNum++)
    {
        CHECK_STATUS(mvHwsFalconCpuExtPowerReductionInit(devNum, portNum));
    }
#endif

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("******** Falcon ifInit end ********\n");
    }
#endif

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    CHECK_STATUS(cpssSystemRecoveryStateGet(&tempSystemRecovery_Info));
    if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E){
#endif
        /*******************************************/
        /* SerDes power-down optimization flow     */
        /*******************************************/
#ifndef MICRO_INIT
        hwsOsMemSetFuncPtr(&serdesConfigPtr, 0, sizeof(MV_HWS_SERDES_CONFIG_STC));
        for (portNum = 0; portNum < (hwsDeviceSpecInfo[devNum].portsNum); portNum++)
        {
            if (   ((portNum / 16) < hwsFalconNumOfRavens)
                && ((hwsRavenDevBmp & (1 << (portNum / 16))) != 0 )  )
            {
                MV_HWS_PORT_INIT_PARAMS curPortParams;
                CHECK_STATUS(hwsPortModeParamsGetToBuffer(devNum, 0, portNum, _10GBase_KR, &curPortParams));
                mvHwsAvagoSerdesPowerCtrl(devNum, 0, curPortParams.firstLaneNum, GT_FALSE,  &serdesConfigPtr);
            }
        }
#endif
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    }
#endif
    return GT_OK;
}


#ifndef MV_HWS_REDUCED_BUILD
/*******************************************************************************
* hwsSerdesIfClose
*
* DESCRIPTION:
*       Free all resources allocated by supported serdes types.
*
* INPUTS:
*       GT_U8 devNum
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static void serdesIfClose(GT_U8 devNum)
{
    mvHwsAvagoIfClose(devNum);
    mvHwsGw16IfClose(devNum);
}

/*******************************************************************************
* hwsPcsIfClose
*
* DESCRIPTION:
*       Free all resources allocated by supported PCS types.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static void pcsIfClose
(
    GT_U8 devNum
)
{
    hwsD2dPcsIfClose(devNum);
}

/*******************************************************************************
* hwsMacIfClose
*
* DESCRIPTION:
*       Free all resources allocated by supported MAC types.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static void macIfClose
(
    GT_U8 devNum
)
{
    hwsD2dMacIfClose(devNum);
}

/*******************************************************************************
* hwsFalconIfClose
*
* DESCRIPTION:
*       Free all resource allocated for ports initialization.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void hwsFalconIfClose(GT_U8 devNum)
{
    GT_U32  i;

    if (hwsInitDone[devNum])
    {
        hwsInitDone[devNum] = GT_FALSE;
    }

    /* if there is no active device remove DB */
    for (i = 0; i < HWS_MAX_DEVICE_NUM; i++)
    {
        if (hwsInitDone[i])
        {
            break;
        }
    }
    if (i == HWS_MAX_DEVICE_NUM)
    {
        falconDbInitDone = GT_FALSE;
    }

    serdesIfClose(devNum);
    pcsIfClose(devNum);
    macIfClose(devNum);

    hwsSerdesIfClose(devNum);
    hwsPcsIfClose(devNum);
    hwsMacIfClose(devNum);

    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }
    hwsPortsElementsClose(devNum);
}

#endif

#if 0
GT_STATUS hwsFalconRavenCpllConfig
(
    IN GT_U8 devNum,
    IN GT_U8 ravenIdx,
    IN CPSS_HW_DRIVER_STC *driverPtr
)
{
   /* GT_U32  serdesNum; */ /* serdes iterator */
   /* GT_U32  data, bit;*/
    /******************************************************************************
     * CPLL initialization:
     *  CPLL_1 - used for D2D GW SERDESes -> configuration 156.25Mhz -> 156.25Mhz
     ******************************************************************************/
    /*hwsServerRegFieldSetFuncPtr(devNum, 0xF8250, 16, 1, 1));*/
   /* data = 0x10000;
    driverPtr->writeMask(driverPtr, 0, 0xF8250, &data, 1, 0x10000);*/
    /* D2D cpllNum == CPLL1 */
    /*bit = 25;*/
    /*CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, addr, &data));*/
   /* driverPtr->read(driverPtr, 0, DEVICE_GENERAL_CONTROL_18, &data, 1);
    data =  (data & ~(1 << bit));*/
    /*CHECK_STATUS(hwsServerRegSetFuncPtr(devNum, addr, data));*/
    /*driverPtr->writeMask(driverPtr, 0, DEVICE_GENERAL_CONTROL_18, &data, 1, 0);*/
    ravenIdx = ravenIdx;
    return mvHwsCpllControl(driverPtr, devNum, CPLL0, GT_FALSE, MV_HWS_156MHz_IN, MV_HWS_156MHz_OUT);
}
#endif
#ifndef MICRO_INIT

GT_STATUS hwsFalconRavenGW16SerdesAutoStartInit
(
    IN GT_U8 devNum,
    IN GT_U8 ravenIdx,
    IN CPSS_HW_DRIVER_STC *driverPtr
)
{
    GT_U8       d2dIdx, d2dNum;

    /**********************************************************************/
    /*     D2D PCS + GW16 auto start (autonegotiation) initialization     */
    /**********************************************************************/
    if (driverPtr != NULL)
    {
        /* for raven smi we write all raven in the same address */
        ravenIdx = 0;
    }

    for (d2dIdx = 0; d2dIdx < HWS_D2D_NUMBER_PER_RAVEN_CNS; d2dIdx++)
    {
        d2dNum = ravenIdx * HWS_D2D_STEP_PER_RAVEN_CNS + d2dIdx*2 + 1;

        CHECK_STATUS(mvHwsD2dPcsInitPart1(driverPtr, devNum, d2dNum));

        if(!cpssDeviceRunCheck_onEmulator())
            CHECK_STATUS(mvHwsGW16SerdesAutoStartInit(driverPtr, devNum, d2dNum));

        CHECK_STATUS(mvHwsD2dPcsInitPart2(driverPtr, devNum, d2dNum));
    }

    return GT_OK;
}

GT_STATUS hwsFalconZ2RavenGW16SerdesAutoStartInit
(
    IN GT_U8 devNum,
    IN GT_U32 ravenBmp
)
{

    CHECK_STATUS(mvHwsD2dPcsInit_Z2(NULL, devNum, HWS_D2D_ALL));

    if(!cpssDeviceRunCheck_onEmulator())
        CHECK_STATUS(mvHwsGW16_Z2_SerdesAutoStartInit(devNum, ravenBmp));

    return GT_OK;
}

/*macro to do masked write  register to the chiplet (Raven) using SMI */
/* WARNING : note the order of 'data' and 'mask' as it is like hwsRegisterSetFuncPtr
    and not like prvCpssDrvHwPpPortGroupWriteRegBitMask */
#define SMI_MASK    prvCpssDrvHwPpSmiChipletRegisterWriteRegBitMask

/* macro to get the chiplet (Raven) SMI driver */
#define chiplet_smi_drv(devNum,RavenId) PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->chipletDrivers[RavenId]

/*function to read register from the chiplet (Raven) using SMI */
GT_STATUS prvCpssDrvHwPpSmiChipletRegisterRead
(
    IN GT_U8    devNum,
    IN GT_U32   chipletId,
    IN GT_U32   regAddr,
    OUT GT_U32  *data
)
{
#ifdef ASIC_SIMULATION
    GT_U32  ravenIndex = chipletId;
    GT_U32  tileId = ravenIndex / 4;
    GT_U32  falconAddr;
    if((1 << tileId) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp)
    {
        ravenIndex = 3 - ravenIndex;
    }

    falconAddr = tileId * 0x20000000 +  (ravenIndex % 4) * 0x01000000;

    prvCpssDrvHwPpPortGroupReadRegister(
        devNum, 0/*portGroupId*/, falconAddr + (regAddr & 0x00FFFFFF), data);

    return GT_OK;
#else /*!ASIC_SIMULATION*/
    if ( chipletId >= 16 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG) ;
    }
    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] == NULL ||
       PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->chipletDrivers[chipletId] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG) ;
    }

    regAddr &= 0x00FFFFFF;/* otherwise will not work on all Ravens !!! */

    return chiplet_smi_drv(devNum,chipletId)->read(chiplet_smi_drv(devNum,chipletId), SSMI_FALCON_ADDRESS_SPACE, regAddr, data, 1);

#endif /*!ASIC_SIMULATION*/
}

/*function to write register to the chiplet (Raven) using SMI */
GT_STATUS prvCpssDrvHwPpSmiChipletRegisterWrite
(
    IN GT_U8    devNum,
    IN GT_U32   chipletId,
    IN GT_U32   regAddr,
    IN GT_U32   value
)
{
#ifdef ASIC_SIMULATION
    GT_U32  ravenIndex = chipletId;
    GT_U32  tileId = ravenIndex / 4;
    GT_U32  falconAddr;
    if((1 << tileId) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp)
    {
        ravenIndex = 3 - ravenIndex;
    }

    falconAddr = tileId * 0x20000000 +  (ravenIndex % 4) * 0x01000000;

    return prvCpssDrvHwPpPortGroupWriteRegister(
        devNum, 0/*portGroupId*/, falconAddr + (regAddr & 0x00FFFFFF), value);
#else /*!ASIC_SIMULATION*/
    if ( chipletId >= 16 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG) ;
    }
    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] == NULL ||
       PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->chipletDrivers[chipletId] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG) ;
    }

    regAddr &= 0x00FFFFFF;/* otherwise will not work on all Ravens !!! */

    return chiplet_smi_drv(devNum,chipletId)->writeMask(chiplet_smi_drv(devNum,chipletId), SSMI_FALCON_ADDRESS_SPACE, regAddr, &value, 1, 0xFFFFFFFF);

#endif /*!ASIC_SIMULATION*/
}

/*function to do masked write  register to the chiplet (Raven) using SMI */
/* WARNING : note the order of 'data' and 'mask' as it is like hwsRegisterSetFuncPtr
    and not like prvCpssDrvHwPpPortGroupWriteRegBitMask */
GT_STATUS prvCpssDrvHwPpSmiChipletRegisterWriteRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   chipletId,
    IN GT_U32   regAddr,
    IN GT_U32   value,
    IN GT_U32   mask
)
{
    GT_U32 tmpValue;

    if(chiplet_smi_drv(devNum,chipletId))
    {
        regAddr &= 0x00FFFFFF;/* otherwise will not work on all Ravens !!! */
        return chiplet_smi_drv(devNum,chipletId)->writeMask(chiplet_smi_drv(devNum,chipletId), SSMI_FALCON_ADDRESS_SPACE, regAddr, &value, 1, mask);
    }

    CHECK_STATUS(prvCpssDrvHwPpSmiChipletRegisterRead(devNum,chipletId,regAddr,&tmpValue));
    tmpValue &= ~mask;
    tmpValue |= (mask & value);
    CHECK_STATUS(prvCpssDrvHwPpSmiChipletRegisterWrite(devNum,chipletId,regAddr,tmpValue));

    return GT_OK;
}

#ifdef ASIC_SIMULATION
GT_STATUS debugSmiRead
(
    GT_U32 ravenIndex,
    GT_U32 regAddr
)
{
    GT_U32  tileId = ravenIndex / 4;
    GT_U32  flaconAddr =  tileId * 0x20000000 +  (ravenIndex % 4) * 0x01000000;
    GT_U32 regData;

    prvCpssDrvHwPpPortGroupReadRegister(
        0/*deviceId*/, 0/*portGroupId*/, flaconAddr + (regAddr & 0x00FFFFFF), &regData);
    cpssOsPrintf("regAddr = 0x%x, regData = 0x%x\n", regAddr, regData);

    return GT_OK;
}

GT_STATUS debugSmiWrite
(
    GT_U32 ravenIndex,
    GT_U32 regAddr,
    GT_U32 regData
)
{
    GT_U32  tileId = ravenIndex / 4;
    GT_U32  flaconAddr =  tileId * 0x20000000 +  (ravenIndex % 4) * 0x01000000;

    return prvCpssDrvHwPpPortGroupWriteRegister(
        0/*deviceId*/, 0/*portGroupId*/, flaconAddr + (regAddr & 0x00FFFFFF), regData);
}
#else /*!ASIC_SIMULATION*/
GT_STATUS debugSmiRead
(
    GT_U32 ravenIndex,
    GT_U32 regAddr
)
{
    GT_U32 regData;
    GT_U32  devNum = 0;

    if ( ravenIndex >= 16 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] == NULL ||
       PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->chipletDrivers[ravenIndex] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG) ;
    }
    regAddr &= 0x00FFFFFF;/* otherwise will not work on all Ravens !!! */
    chiplet_smi_drv(devNum,ravenIndex)->read(chiplet_smi_drv(devNum,ravenIndex), SSMI_FALCON_ADDRESS_SPACE, regAddr, &regData, 1);
    cpssOsPrintf("regAddr = 0x%x, regData = 0x%x\n", regAddr, regData);
    return GT_OK;
}

GT_STATUS debugSmiWrite
(
    GT_U32 ravenIndex,
    GT_U32 regAddr,
    GT_U32 regData
)
{
    GT_U32  devNum = 0;

    if ( ravenIndex >= 16 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG) ;
    }
    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] == NULL ||
       PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->chipletDrivers[ravenIndex] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG) ;
    }
    regAddr &= 0x00FFFFFF;/* otherwise will not work on all Ravens !!! */
    chiplet_smi_drv(devNum,ravenIndex)->writeMask(chiplet_smi_drv(devNum,ravenIndex), SSMI_FALCON_ADDRESS_SPACE, regAddr, &regData, 1, 0xFFFFFFFF);
    return GT_OK;
}
#endif /*!ASIC_SIMULATION*/

extern GT_U32 falcon_force_early_check_for_device_not_reset_get(void);
extern GT_BOOL cpssDxChCheckIsHwDidHwReset(IN GT_U8 devNum);

/*assumption PEX device and SMI master decise were crated before */
/* PEX - during init phase 1; SMI - before calling the function (cpssDxChPhyPortSmiInit)*/
GT_STATUS hwsFalconRavenD2dInit
(
    IN GT_U8 devNum
)
{
    GT_STATUS           rc;
    GT_U32              ravenSmiAddr;
    GT_U32              ravenSmiBus;
    GT_U8               ravenIdx;
    /* init dual tile */
    CPSS_HW_DRIVER_STC  *ssmiPtr = NULL;
    GT_U32               deviceId = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devType;
#ifndef ASIC_SIMULATION
    GT_U32              failedRavensBmp = 0;
#endif /*!ASIC_SIMULATION*/

    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType == CPSS_CHANNEL_PEX_FALCON_Z_E)/* Falcon Z2 */
    {
        hwsFalconZ2Mode = GT_TRUE;

        rc = hwsFalconZ2RavenGW16SerdesAutoStartInit(devNum, hwsRavenDevBmp);
        if(rc != GT_OK)
        {
            hwsOsPrintf("hwsFalconZ2RavenGW16SerdesAutoStartInit failed: ravenBmp = %d, rc = %d\n", hwsRavenDevBmp, rc);
            return rc;
        }
    }
    else
    {
        /* loop on all valid Ravens and create SMI connection */
        for (ravenIdx = 0; ravenIdx < hwsFalconNumOfRavens; ravenIdx++)
        {
            if ((!(hwsRavenDevBmp & (1<<ravenIdx))) && !(CPSS_FALCON_REDUCED_2T_4T_3_2T_DEVICES_CHECK_MAC(deviceId) || (deviceId == CPSS_98CX8550_CNS)))
            {
                continue;
            }

            /* get ravenSmiID from ravenIdx - probably fix array*/
            ravenSmiAddr= ravenIdx % HWS_FALCON_3_2_RAVEN_NUM_CNS;
            ravenSmiBus = ravenIdx / HWS_FALCON_3_2_RAVEN_NUM_CNS;

            if(ravenSmiBus % 2)
            {
                ravenSmiAddr = 3 - ravenSmiAddr;
            }

            rc = hwsGetDeviceDriverFuncPtr(devNum, 0, ravenSmiBus /*smi 0/1/2/3*/, ravenSmiAddr, &ssmiPtr);
            if ((rc != GT_OK) || (ssmiPtr == NULL))
            {
#ifndef ASIC_SIMULATION
                hwsOsPrintf("hwsGetDeviceDriverFuncPtr returned with NULL pointer for ravenIdx = %d\n", ravenIdx);
                failedRavensBmp |= 1 << ravenIdx;
#endif /*!ASIC_SIMULATION*/
                /* On WM we expect to not get SSMI driver */
                continue;
            }
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->chipletDrivers[ravenIdx] = ssmiPtr;
        }

         CHECK_STATUS(cpssSystemRecoveryStateGet(&tempSystemRecovery_Info));
         if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
         {
            return GT_OK;
         }

#ifndef ASIC_SIMULATION
        if(failedRavensBmp)
        {
            /* at least one of the Ravens failed on the SMI connection */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "The SSMI connection failed on next Ravens BMP [0x%8.8x] out of[0x%8.8x]",
                failedRavensBmp,
                hwsRavenDevBmp);
        }
#endif /*!ASIC_SIMULATION*/

        if(falcon_force_early_check_for_device_not_reset_get())
        {
            GT_BOOL didHwReset    = cpssDxChCheckIsHwDidHwReset(devNum);

            if(didHwReset != GT_TRUE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR_NEED_RESET,
                    "The Falcon must go through soft/hard reset before called for 'phase1 init' again \n");
            }
        }

        for (ravenIdx = 0; ravenIdx < hwsFalconNumOfRavens; ravenIdx++)
        {
            if (!(hwsRavenDevBmp & (1<<ravenIdx)))
            {
                continue;
            }

            ssmiPtr = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->chipletDrivers[ravenIdx];

#ifdef ASIC_SIMULATION
            if (ssmiPtr == NULL)
            {
                continue;
            }
#endif
            rc = hwsFalconRavenGW16SerdesAutoStartInit(devNum, ravenIdx, ssmiPtr);
            if(rc != GT_OK)
            {
                hwsOsPrintf("hwsFalconRavenGW16SerdesAutoStartInit failed: ravenIdx = %d, rc = %d\n", ravenIdx, rc);
                return rc;
            }
        }
    }
    return GT_OK;
}

GT_STATUS hwsFalconEagleD2dInit
(
    IN GT_U8 devNum
)
{
    GT_U8       d2dIdx, d2dNum;

    /***********************************************************/
    /*     D2D auto start (autonegotiation) initialization     */
    /***********************************************************/
    /* In order to perform D2D/PHY configurations it is needed to setup links
       on all 6 devices and only then start to run basic init sequence
        Wrote register via map d2d_reg_block[0].RXC_RegFile.uvm_reg_map: d2d_reg_block[0].RXC_RegFile.autostart=0xf
        Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_CFG_ENA=0xd003d
        Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_IFC_REMAP_8=0x100808
        Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_IFC_REMAP_9=0x100808*/

    for (d2dIdx = 0; d2dIdx < hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS; d2dIdx++) {
        if (hwsRavenDevBmp & (1<<(d2dIdx/HWS_D2D_NUM_IN_RAVEN_CNS)))
        {
            d2dNum = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0);

            CHECK_STATUS(mvHwsD2dPcsInitPart1(NULL, devNum, d2dNum));

            if(!cpssDeviceRunCheck_onEmulator())
                CHECK_STATUS(mvHwsGW16SerdesAutoStartInit(NULL, devNum, d2dNum));

            CHECK_STATUS(mvHwsD2dPcsInitPart2(NULL, devNum, d2dNum));
        }
    }
    return GT_OK;
}

static GT_STATUS  mvHwsFalconD2dLinkCheckViaD2dCp(
    IN GT_U8    devNum,
    IN GT_U32   ravenIndex,
    IN GT_U32   tapIndex
);
static GT_STATUS  mvHwsFalconD2dUseSmiToPrepareD2dCpAccess(
    IN GT_U8    devNum,
    IN GT_U32   ravenIndex
);


GT_STATUS hwsFalconCheckD2dStatus
(
    IN GT_U8 devNum,
    IN GT_U32 d2dNum

)
{
    GT_STATUS   rc;
    GT_U8       d2dIdx;
    GT_U32      ravenIndex,localD2dId;

    if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType == CPSS_CHANNEL_PEX_FALCON_Z_E)/* Falcon Z2 */
    {
        /* already done as part of hwsFalconZ2RavenGW16SerdesAutoStartInit() */
        return GT_OK;
    }

    if (d2dNum == HWS_D2D_ALL) {
        for (d2dIdx = 0; d2dIdx < hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS; d2dIdx++) {
            if (hwsRavenDevBmp & (1<<(d2dIdx/HWS_D2D_NUM_IN_RAVEN_CNS)))
            {
                d2dNum = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 1);

                rc = mvHwsGW16AutoStartStatusGet(chiplet_smi_drv(devNum,d2dIdx/HWS_D2D_NUM_IN_RAVEN_CNS), devNum, d2dNum);
                if(rc != GT_OK)
                {
                    hwsOsPrintf("RAVEN: mvHwsGW16AutoStartStatusGet failed: d2dNum = %d, rc = %d\n", d2dNum, rc);
                    return rc;
                }
                /***********************************************************/
                /*     D2D PHY wait for auto start init                    */
                /***********************************************************/

                d2dNum = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0);
                rc = mvHwsGW16SerdesAutoStartInitStatusGet(devNum, d2dNum);
                if(rc != GT_OK)
                {
                    hwsOsPrintf("mvHwsGW16SerdesAutoStartInitStatusGet failed: d2dNum = %d, rc = %d\n", d2dNum, rc);
                    return rc;
                }

                ravenIndex = d2dIdx / 2;
                localD2dId = d2dIdx % 2;

                if(localD2dId == 0)
                {
                    /*******************************************************/
                    /* Do SMI access on Raven to allow D2D_CP transactions */
                    /* (to allow PEX transactions)                         */
                    /*******************************************************/
                    CHECK_STATUS(mvHwsFalconD2dUseSmiToPrepareD2dCpAccess(devNum,ravenIndex));

                    /**************************************************/
                    /* D2D_cp is ready - we can use PEX to this Raven */
                    /**************************************************/
                }

                /**************************************************************************************/
                /*  Falcon : check on Eagle Side and on Raven Side that the access to Raven MG is OK  */
                /*  and not generate error interrupts on any side of the D2D_CP                       */
                /**************************************************************************************/
                CHECK_STATUS(mvHwsFalconD2dLinkCheckViaD2dCp(devNum,ravenIndex,localD2dId));
            }
        }
    }
    else
    {
        CHECK_STATUS(mvHwsGW16SerdesAutoStartInitStatusGet(devNum, d2dNum));

    }
    return GT_OK;
}
#endif /*!MICRO_INIT*/
GT_STATUS hwsFalconD2dPcsResetEnable
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_BOOL enable
)
{
    GT_U32 d2dIdx;

    if (d2dNum != HWS_D2D_ALL)
    {
        CHECK_STATUS(mvHwsD2dPcsReset(devNum, d2dNum, enable));
    }
    else
    {
        for (d2dIdx = 0; d2dIdx < hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS; d2dIdx++) {
            if (hwsRavenDevBmp & (1<<(d2dIdx/HWS_D2D_NUM_IN_RAVEN_CNS)))
            {
                d2dNum = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0); /*eagle */
                CHECK_STATUS(mvHwsD2dPcsReset(devNum,d2dNum,enable));
                d2dNum = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 1); /*raven */
                CHECK_STATUS(mvHwsD2dPcsReset(devNum,d2dNum,enable));
            }
        }
    }
    return GT_OK;
}

GT_STATUS hwsFalconD2dMacChannelEnable
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_U32  channel,
    GT_BOOL enable
)
{
    GT_U32 d2dIdx;
    if (d2dNum != HWS_D2D_ALL)
    {
        CHECK_STATUS(hwsD2dMacChannelEnable(devNum, d2dNum, channel, enable));
    }
    else
    {
        for (d2dIdx = 0; d2dIdx < hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS; d2dIdx++) {
            if (hwsRavenDevBmp & (1<<(d2dIdx/HWS_D2D_NUM_IN_RAVEN_CNS)))
            {
                d2dNum = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0); /*eagle */
                CHECK_STATUS(hwsD2dMacChannelEnable(devNum,d2dNum,channel,enable));
                d2dNum = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 1); /*raven */
                CHECK_STATUS(hwsD2dMacChannelEnable(devNum,d2dNum,channel,enable));
            }
        }
    }
    return GT_OK;
}

GT_STATUS mvHwsFalconExtPowerReductionInit
(
    IN GT_U8               devNum,
    IN GT_U32              portNum,
    IN CPSS_HW_DRIVER_STC  *ssmiPtr
)
{
    GT_U32 regAddr;
    GT_U32 regData = 0x0;
    GT_U32 regMask;
    GT_U32 baseAddr, unitNum, unitIndex;

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, portNum, &baseAddr, &unitIndex, &unitNum));
    /*
        Reset assert for all regular 8 MACs
        m_RAL.mtip_ext_units_RegFile.Global_Reset_Control.gc_port_reset_.set(0);
    */
    regAddr = baseAddr + MTIP_EXT_GLOBAL_RESET_CONTROL;
    regMask = 0xFF0000;

    HWS_REGISTER_WRITE_REG(ssmiPtr, devNum, regAddr, regData, regMask);
    /*CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));*/

    /*
        Reset assert for 200/400 MACs
        m_RAL.mtip_ext_units_RegFile.Global_Reset_Control.gc_seg_port_reset_.set(0);
    */
    regMask = 0x3000000;
    HWS_REGISTER_WRITE_REG(ssmiPtr, devNum, regAddr, regData, regMask);
    /*CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));*/

    /*
        Disable Gater for ALL 8 MACs
        m_RAL.mtip_ext_units_RegFile.Global_Clock_Enable.mac_clk_en.set(0);
    */
    regAddr = baseAddr + MTIP_EXT_GLOBAL_CLOCK_ENABLE;
    regMask = 0xFF;
    /*CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));*/
    HWS_REGISTER_WRITE_REG(ssmiPtr, devNum, regAddr, regData, regMask);

    /*
        Disable Gater for 200/400 MACs.
        m_RAL.mtip_ext_units_RegFile.Global_Clock_Enable.seg_mac_clk_en.set(0);
    */
    regMask = 0x300;
    HWS_REGISTER_WRITE_REG(ssmiPtr, devNum, regAddr, regData, regMask);
    /*CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));*/

    /*
        Assert shared PCS Reset for "Quad" (4 adjacent PCS100/50)
        m_RAL.mtip_ext_units_RegFile.Global_Reset_Control.gc_xpcs_reset_.set(0);
    */
    regAddr = baseAddr + MTIP_EXT_GLOBAL_RESET_CONTROL;
    regMask = 0x18000000;

    HWS_REGISTER_WRITE_REG(ssmiPtr, devNum, regAddr, regData, regMask);
    /*CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));*/

    /*
        Assert shared PCS Reset for PCS400 and PCS200
        m_RAL.mtip_ext_units_RegFile.Global_Reset_Control.gc_pcs000_reset_.set(0);
    */
    regMask = 0x4000000;
    HWS_REGISTER_WRITE_REG(ssmiPtr, devNum, regAddr, regData, regMask);
    /*CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));*/


    /*
        Assert shared FEC-91 (Reed Solomon FEC) Reset for all 8 ports.
        m_RAL.mtip_ext_units_RegFile.Global_Reset_Control.gc_f91_reset_.set(0);
    */
    regMask = 0x20000000;
    HWS_REGISTER_WRITE_REG(ssmiPtr, devNum, regAddr, regData, regMask);
    /*CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));*/

    /*
        Assert shared Gigabit Reset for all 8 ports.
        m_RAL.mtip_ext_units_RegFile.Global_Reset_Control.gc_sg_reset_.set(0);
    */
    regMask = 0x40000000;
    HWS_REGISTER_WRITE_REG(ssmiPtr, devNum, regAddr, regData, regMask);
    /*CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));*/

    /*
        Disable shared PCS Gater for "Quad" (4 adjacent PCS100/50)
        m_RAL.mtip_ext_units_RegFile.Global_Clock_Enable.xpcs_clk_en.set(0);
    */
    regAddr = baseAddr + MTIP_EXT_GLOBAL_CLOCK_ENABLE;
    regMask = 0x3000;
    HWS_REGISTER_WRITE_REG(ssmiPtr, devNum, regAddr, regData, regMask);
    /*CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));*/

    /*
        Disable shared PCS Gater for PCS400 and PCS200
        m_RAL.mtip_ext_units_RegFile.Global_Clock_Enable.pcs000_clk_en.set(0);
    */
    regMask = 0x4000;
    HWS_REGISTER_WRITE_REG(ssmiPtr, devNum, regAddr, regData, regMask);
    /*CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));*/

    /*
        Disable shared FEC-91 (Reed Solomon FEC) Gater for all 8 ports.
        m_RAL.mtip_ext_units_RegFile.Global_Clock_Enable.fec91_clk_en.set(0);
    */
    regMask = 0x800;
    HWS_REGISTER_WRITE_REG(ssmiPtr, devNum, regAddr, regData, regMask);
    /*CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));*/

    /*
        Disable shared Gigabit Reset for all 8 ports.
        m_RAL.mtip_ext_units_RegFile.Global_Clock_Enable.sgref_clk_en.set(0);
    */
    regMask = 0x8000;
    HWS_REGISTER_WRITE_REG(ssmiPtr, devNum, regAddr, regData, regMask);
    /*CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));*/

    /*
        m_RAL.mtip_ext_units_RegFile.MTIP_Global_Channel_Control.gc_fec91_1lane_in0.set(1);
        m_RAL.mtip_ext_units_RegFile.MTIP_Global_Channel_Control.gc_fec91_1lane_in2.set(1);
        m_RAL.mtip_ext_units_RegFile.MTIP_Global_Channel_Control.gc_fec91_1lane_in4.set(1);
        m_RAL.mtip_ext_units_RegFile.MTIP_Global_Channel_Control.gc_fec91_1lane_in6.set(1);
    */
    regAddr = baseAddr + MTIP_EXT_GLOBAL_CHANNEL_CONTROL;
    regData = regMask = 0x3C00;
    HWS_REGISTER_WRITE_REG(ssmiPtr, devNum, regAddr, regData, regMask);
    /*CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));*/

    return GT_OK;
}
#ifndef MICRO_INIT
/* read on Raven : the MG register of 'vendorId' and make sure we read 0x11AB (10 times) */
/* this is done via TAP0(on D2D_CP_0) of TAP1(on D2D_CP_1)                               */
static GT_STATUS mvHwsFalconRavenMgVendorIdCheck
(
    IN GT_U8    devNum,
    IN GT_U32   ravenIndex,
    IN GT_U32   tapIndex
)
{
    GT_U32  i,regAddr , baseAddr, unitIndex, unitNum;
    GT_U32  data;
    static GT_U32  maxIter = 10;
    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_BASE_UNIT, ravenIndex, &baseAddr, &unitIndex, &unitNum ));

    /*register 0x00000050 in MG of Raven that must hold 0x11AB */
    regAddr = baseAddr + 0x00300050;
#ifndef ASIC_SIMULATION /* no need WM support for this 'trick' , so ignore this option for it */
    if(tapIndex)
    {
        /* the Cider not show those addresses with bit 23 set , but this is option
           to use tap1 (D2D_CP number 1 instead of the usual number 0) */
        regAddr |= BIT_23;
    }
#endif /*!ASIC_SIMULATION*/

    for (i=0;i<maxIter;i++)
    {
        cpssOsTimerWkAfter(1);
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &data, 0xFFFFFFFF));
        if (data != 0x11AB)
        {
            /* one of reads is not 11ab ... we must kill the run to allow debug */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,
                "ravenIndex[%d] TAP[%d] : Read register regAddr=0x%X,regData=0x%X Fail iterator = %i \n",
                ravenIndex,tapIndex,
                regAddr,data,i);
        }
    }

    return GT_OK;
}

static GT_U32 d2dErrorBits = 0x0000013E;/* bits 1-5,8 */

/* read on Raven : the D2D_CP interrupt cause registers .
    if readOnly == GT_FALSE and if the value != 0 --> cause error
*/
static GT_STATUS mvHwsFalconRavenD2dCpInterruptCheck
(
    IN GT_U8    devNum,
    IN GT_U32   ravenIndex,
    IN GT_U32   tapIndex, /* just for debug print */
    IN GT_BOOL  readOnly
)
{
    GT_U32  regAddr , baseAddr, unitIndex, unitNum;
    GT_U32  data;

#ifndef CPSS_LOG_ENABLE
    GT_UNUSED_PARAM(tapIndex);
#endif
    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_BASE_UNIT, ravenIndex, &baseAddr, &unitIndex, &unitNum ));

    /* Raven : interrupt cause register in D2D_CP */
    regAddr = baseAddr + 0x002F0050;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &data, 0xFFFFFFFF));

    if((data & d2dErrorBits) && readOnly != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,
            "ERROR : ravenIndex[%d] tapIndexp[%d]: Read Raven D2D_CP interrupt cause register regAddr=0x%X,regData=0x%X (Error bit is set) \n",
            ravenIndex,
            tapIndex,
            regAddr,data);
    }

    return GT_OK;
}

/* read on Eagle : the D2D_CP interrupt cause registers .
    if readOnly == GT_FALSE and if the value != 0 --> cause error
*/
static GT_STATUS mvHwsFalconEagleD2dCpInterruptCheck
(
    IN GT_U8    devNum,
    IN GT_U32   ravenIndex,
    IN GT_U32   tapIndex, /* just for debug print */
    IN GT_BOOL  readOnly
)
{
    GT_U32  regAddr , baseAddr, unitIndex, unitNum;
    GT_U32  data;

#ifndef CPSS_LOG_ENABLE
    GT_UNUSED_PARAM(tapIndex);
#endif
    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_BASE_UNIT, ravenIndex, &baseAddr, &unitIndex, &unitNum ));

    /* the D2D_CP of eagle in only on the address of the 'raven 0' although
        collect errors for all accessing to all Ravens of the tile */
    baseAddr &= 0xF0FFFFFF;

    /* Raven : interrupt cause register in D2D_CP */
    regAddr = baseAddr + 0x002E0050;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &data, 0xFFFFFFFF));

    if((data & d2dErrorBits) && readOnly != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,
            "ERROR : Tile[%d] ravenIndex[%d] tapIndex[%d] : Read Eagle D2D_CP interrupt cause register regAddr=0x%X,regData=0x%X (Error bit is set) \n",
                ravenIndex/4,
                ravenIndex,
                tapIndex,
                regAddr,data);
    }

    return GT_OK;
}

/**************************************************************************************/
/*  Falcon : check on Eagle Side and on Raven Side that the access to Raven MG is OK  */
/*  and not generate error interrupts on any side of the D2D_CP                       */
/**************************************************************************************/
static GT_STATUS  mvHwsFalconD2dLinkCheckViaD2dCp(
    IN GT_U8    devNum,
    IN GT_U32   ravenIndex,
    IN GT_U32   tapIndex
)
{
    /* 1. read the interrupts to make them clear (on TAP x) */
    /* eagle side */
    CHECK_STATUS(mvHwsFalconEagleD2dCpInterruptCheck(devNum,ravenIndex,tapIndex,GT_TRUE/*only read -- no check*/));
    /* Raven side */
    CHECK_STATUS(mvHwsFalconRavenD2dCpInterruptCheck(devNum,ravenIndex,tapIndex,GT_TRUE/*only read -- no check*/));

    /* 2. make sure that D2D_x is ok (read properly from Raven MG via D2D_CP_x) */
    CHECK_STATUS(mvHwsFalconRavenMgVendorIdCheck(devNum,ravenIndex,tapIndex));

    /* 3. check no interrupts due to access on TAP x */
    /* Raven side */
    CHECK_STATUS(mvHwsFalconRavenD2dCpInterruptCheck(devNum,ravenIndex,tapIndex,GT_FALSE/*check no interrupts*/));
    /* eagle side */
    CHECK_STATUS(mvHwsFalconEagleD2dCpInterruptCheck(devNum,ravenIndex,tapIndex,GT_FALSE/*check no interrupts*/));

    return GT_OK;
}

/* for the 'external' SMI D2D_CP accessing need first to set the SMI XBAR properly */
static GT_STATUS hwsFalconRavenOpenSmiToD2dCp(
    IN GT_U8    devNum,
    IN GT_U32   ravenIndex
)
{
    GT_U32  regAddr , baseAddr, unitIndex, unitNum;
    GT_U32  xbar_d2d_cp_port_id = 0x2;/* D2D_CP uses port of the XBAR */

    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_BASE_UNIT, ravenIndex, &baseAddr, &unitIndex, &unitNum ));

    /*/Cider/EBU/Falcon/Falcon {Current}/Raven_Full/<Raven>Raven_A0/<Raven> Device Units/RUnit/Units/
    <SSMI> SSMI Units/XBAR remap/SSMI XBAR port %q remap*/
    regAddr = baseAddr + 0x00113100 + 0x4 * xbar_d2d_cp_port_id;

    /* clear bit 16 that was set by cpssHwDriverSip5SlaveSMICreateDrv ? */
    /* clear also bits 0..3 : that cause problems in Ravens : 7,8,15 !!! */
    CHECK_STATUS(SMI_MASK(devNum,ravenIndex,regAddr,0x0,0x1000F));

    return GT_OK;
}

/*******************************************************/
/* Do SMI access on Raven to allow D2D_CP transactions */
/* (to allow PEX transactions)                         */
/*******************************************************/
static GT_STATUS  mvHwsFalconD2dUseSmiToPrepareD2dCpAccess(
    IN GT_U8    devNum,
    IN GT_U32   ravenIndex
)
{
    GT_U32  regAddr , baseAddr, unitIndex, unitNum;
    GT_U32  data,mask;

    /**************/
    /* Raven side */
    /**************/

    /* for the 'internal' SMI D2D_CP accessing need first to set the SMI XBAR properly */
    CHECK_STATUS(hwsFalconRavenOpenSmiToD2dCp(devNum,ravenIndex));

    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_BASE_UNIT, ravenIndex, &baseAddr, &unitIndex, &unitNum ));

    data = mask = 0xFC;
    /* CFG INT CBE DISABLE register */
    regAddr = baseAddr + D2D_CP_CFG_INT_CBE_DISABLE;

    CHECK_STATUS(SMI_MASK(devNum, ravenIndex, regAddr, data, mask));

    data = 0x10;
    mask = 0xf0;
    /* CFG INT CBE register */
    regAddr = baseAddr + D2D_CP_CFG_INT_CBE;
    CHECK_STATUS(SMI_MASK(devNum, ravenIndex, regAddr, data, mask));

    /*
      Raven D2D CP:
    /Cider/EBU/Falcon/Falcon {Current}/Raven_Full/<Raven>Raven_A0/<Raven> Device Units/<D2D_CP> D2D_CP/D2D_CP_RF/CFG D2X REQ TIMEOUT
    Register default is: 0x7F (=127)

    It means that Raven/D2D_CP access set a request towards the XBAR, and after 127 clock cycles (reg_clk 166.67MHz, 6ns cycle time, 762ns) it de-assert the request (=���regret���).
    This time is not enough, because if in parallel there is an access to AVAGO S/D through the AVAGO SBC, it takes ~8usec!

    The needed configuration is: 0x1FFF
    Such a configuration means a regret after ~50us, or x6 the access time to Avago SD through the SBC.
    It will provide enough time for the prev access (Avago SBC) to complete, and then D2D-CP will gain the XBAR arbitration towards e.g. the D2D.
    */
    data = 0xD05;
    mask = 0xffff;
    /* CFG INT D2X req timeout register */
    regAddr = baseAddr + D2D_CP_CFG_D2X_REQ_TIMEOUT;
    CHECK_STATUS(SMI_MASK(devNum, ravenIndex, regAddr, data, mask));

    data = 0xC;
    mask = 0xffff;
    /* CFG INT X2D RDREQ req timeout register */
    regAddr = baseAddr + D2D_CP_CFG_X2D_RDREQ_TIMEOUT;
    CHECK_STATUS(SMI_MASK(devNum, ravenIndex, regAddr, data, mask));

#if 0  /* was needed only in start of debug session */
    regAddr = baseAddr + D2D_CP_INTERRUPT_MASK;
    data = 0x1fe;
    mask = 0x1fe;
    CHECK_STATUS(SMI_MASK(devNum, ravenIndex, regAddr, data, mask));
#endif /*0*/


    /**************/
    /* Eagle side */
    /**************/

    /*
       Eagle D2D-CPs (total 4)
    /Cider/EBU/Falcon/Falcon {Current}/<Eagle> Eagle/MNG/D2D_CP/<D2D_CP> D2D_CP/D2D_CP_RF/CFG X2D RDREQ TIMEOUT
    Register default value is: 0x1F
    Need to be set to: 0xE

    Eagle D2D_CP unit uses the same address space like raven d2d_cp
    */
    if (ravenIndex % 4 == 0) /* done via PEX as it is not on the Raven (althugh 'address' is) */
    {
        data = 0xD;
        mask = 0xffff;
        /* CFG INT X2D req timeout register */
        regAddr = (baseAddr&0xf0ffffff) + EAGLE_D2D_CP_CFG_X2D_RDREQ_TIMEOUT;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, data, mask));

        data = 0x186A;
        mask = 0xffff;
        /* CFG INT D2X req timeout register */
        regAddr = (baseAddr&0xf0ffffff) + EAGLE_D2D_CP_CFG_D2X_REQ_TIMEOUT;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, data, mask));


#if 0  /* was needed only in start of debug session */
        regAddr = (baseAddr&0xf0ffffff) + EAGLE_D2D_CP_INTERRUPT_MASK;
        data = 0x1fe;
        mask = 0x1fe;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, data, mask));
#endif /*0*/
   }

   return GT_OK;
}

/* flag to allow to disable the EmbededCpu config during hwsFalconStep0 */
static GT_U32   hwsFalconStep0_disableServiceCpu = 0;
GT_STATUS   hwsFalconStep0_disableServiceCpu_set(IN GT_U32 disable)
{
    hwsFalconStep0_disableServiceCpu = disable;
    return GT_OK;
}

/*INITIATE D2D MAC+PCS AND ACHIEVE LINK*/
GT_STATUS hwsFalconStep0
(
    IN GT_U8 devNum,
    IN GT_U32 numOfTiles
)
{
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    if(falcon_dev_init_trace_func)
    {
        hwsOsPrintf("****** open trace ******\n");
        falcon_dev_init_trace_func(GT_TRUE);
    }

    hwsFalconNumOfRavens = numOfTiles * HWS_FALCON_3_2_RAVEN_NUM_CNS;
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    /* write to HW services db tiles number */
    CHECK_STATUS(hwsFalconNumOfTilesUpdate(devNum, numOfTiles));
#endif

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** Falcon step 0 start ******\n");
    }
#endif

    /*
        Eagle:
        Wrote register via map d2d_reg_block[0].RXC_RegFile.uvm_reg_map: d2d_reg_block[0].RXC_RegFile.autostart=0xf
        Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_CFG_ENA=0xd003d
        Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_IFC_REMAP_8=0x100008
        Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_IFC_REMAP_9=0x100008
        Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_IFC_REMAP_16=0x101800
        Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_IFC_REMAP_17=0x101800
    */

    CHECK_STATUS(cpssSystemRecoveryStateGet(&tempSystemRecovery_Info));
    if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        /*still need to init SMI driver for Raven related configurations */
        CHECK_STATUS(hwsFalconRavenD2dInit(devNum));
        return GT_OK;
    }

#ifndef MICRO_INIT
    if(hwsFalconStep0_disableServiceCpu)
    {
        mvHwsServiceCpuEnable(devNum,0);
    }

    CHECK_STATUS(hwsFalconEagleD2dInit(devNum));
    /*
    Raven, done through SMI:
    Wrote register via map d2d_reg_block[1].RXC_RegFile.uvm_reg_map: d2d_reg_block[1].RXC_RegFile.autostart=0xf
    Wrote register via map d2d_reg_block[1].D2D_RegFile.uvm_reg_map: d2d_reg_block[1].D2D_RegFile.PCS_CFG_ENA=0xd003d
    */
    CHECK_STATUS(hwsFalconRavenD2dInit(devNum));
    /*
        Eagle: (pull indication is for Raven & Eagle)
        pull for bit [18:17] = 2'b11
        Read  register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_STATUS
    */
    CHECK_STATUS(hwsFalconCheckD2dStatus(devNum, HWS_D2D_ALL));
#else  /* MICRO_INIT */
    {
        GT_U32  ravenIndex;
        for (ravenIndex = 0; ravenIndex < hwsFalconNumOfRavens; ravenIndex++)
        {
            if (hwsRavenDevBmp & (1<<ravenIndex))
            {
                /*******************************************************/
                /* Do SMI access on Raven to allow D2D_CP transactions */
                /* (to allow PEX transactions)                         */
                /*******************************************************/
                CHECK_STATUS(mvHwsFalconD2dUseSmiToPrepareD2dCpAccess(devNum,ravenIndex));
            }
        }
    }
#endif

    /**************************************************/
    /* D2D_cp is ready - we can use PEX to ALL Ravens */
    /**************************************************/

    /* set d2d mac channel in reset */
    CHECK_STATUS(hwsFalconD2dMacChannelEnable(devNum, HWS_D2D_ALL, HWS_D2D_MAC_CHANNEL_ALL, GT_FALSE));

    /*set d2d pcs out of reset */
    CHECK_STATUS(hwsFalconD2dPcsResetEnable(devNum, HWS_D2D_ALL, GT_TRUE));
    /* enable fcu in msdb */
    CHECK_STATUS(mvHwsMsdbFcuEnable(devNum, HWS_D2D_ALL, GT_TRUE));
    /* msdb register misconfiguration fix */
    CHECK_STATUS(mvHwsMsdbRegWaInit(devNum, HWS_D2D_ALL));
    /* enable tsu in mpf */
    CHECK_STATUS(mvHwsMpfTsuEnable(devNum, HWS_D2D_ALL, NON_SUP_MODE, GT_TRUE));

    /*if(!cpssDeviceRunCheck_onEmulator())*/
    {
         GT_U32 d2dIdx, d2dNum;
         for (d2dIdx = 0; d2dIdx < (hwsFalconNumOfRavens * HWS_D2D_NUM_IN_RAVEN_CNS); d2dIdx++)
         {
             if (hwsRavenDevBmp & (1<<(d2dIdx/HWS_D2D_NUM_IN_RAVEN_CNS)))
             {
                 d2dNum = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 0); /*eagle */
                 CHECK_STATUS(mvHwsD2dDisableAll50GChannel(devNum,d2dNum));
                 d2dNum = PRV_HWS_D2D_SET_D2D_NUM_MAC(d2dIdx, 1); /*raven */
                 CHECK_STATUS(mvHwsD2dDisableAll50GChannel(devNum,d2dNum));
             }
         }
    }

    /* clear interrupts errors on Raven/Eagle D2D PMA interrupt cause registers */
    CHECK_STATUS(hwsFalconInterruptsD2dClear(devNum));


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("****** Falcon step 0 end ******\n");
    }
#endif


#ifndef MICRO_INIT
    if(hwsFalconStep0_disableServiceCpu)
    {
        mvHwsServiceCpuEnable(devNum,1);
    }
#endif /*MICRO_INIT*/

    if(falcon_dev_init_trace_func)
    {
        falcon_dev_init_trace_func(GT_FALSE);
        hwsOsPrintf("****** close trace ******\n");
    }

    return GT_OK;
}
#endif /*MICRO_INIT*/

#ifndef FALCON_DEV_SUPPORT
GT_VOID hwsFalconAddrCalc
(
    GT_U8           devNum,
    HWS_UNIT_BASE_ADDR_TYPE_ENT addressType,
    GT_UOPT         portNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
)
{
    GT_U32 tempRegAddr = 0;

    if(PRV_NON_SHARED_LAB_SERVICES_DIR_FALCON_DEV_INIT_SRC_GLOBAL_VAR(falconBaseAddrCalcFunc) == NULL)
    {
        /* not implemented */
        *addressPtr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        return;
    }

    tempRegAddr = PRV_NON_SHARED_LAB_SERVICES_DIR_FALCON_DEV_INIT_SRC_GLOBAL_VAR(falconBaseAddrCalcFunc)(devNum,addressType,portNum);
    if(tempRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        *addressPtr = regAddr + tempRegAddr;
    }
    else
    {
        *addressPtr = tempRegAddr;
    }
    return;
}
#endif
/**
* @internal hwsFalconUnitBaseAddrCalcBind function
* @endinternal
 *
* @param[in] cbFunc                   - the callback function
*/
GT_VOID hwsFalconUnitBaseAddrCalcBind
(
    HWS_UNIT_BASE_ADDR_CALC_BIND    cbFunc
)
{
    /* save the DB function */
    PRV_NON_SHARED_LAB_SERVICES_DIR_FALCON_DEV_INIT_SRC_GLOBAL_VAR(falconBaseAddrCalcFunc) = cbFunc;
}

/**
* @internal hwsFalconSerdesAddrCalc function
* @endinternal
*
* @brief   Calculate serdes register address for Falcon
*
* @param[in] serdesNum                - global serdes lane number
* @param[in] regAddr                  - offset of required register in SD Unit (0x13000000)
*
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
GT_VOID hwsFalconSerdesAddrCalc
(
    GT_U8           devNum,
    GT_UOPT         serdesNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
)
{
#ifndef MICRO_INIT
    GT_U32 tempRegAddr = 0;

    if(PRV_NON_SHARED_LAB_SERVICES_DIR_FALCON_DEV_INIT_SRC_GLOBAL_VAR(falconBaseAddrCalcFunc) == NULL)
    {
        /* not implemented */
        *addressPtr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        return;
    }

    tempRegAddr = PRV_NON_SHARED_LAB_SERVICES_DIR_FALCON_DEV_INIT_SRC_GLOBAL_VAR(falconBaseAddrCalcFunc)(devNum,HWS_UNIT_BASE_ADDR_TYPE_SERDES_E,serdesNum);
    if(tempRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        *addressPtr = regAddr + tempRegAddr;
    }
    else
    {
        *addressPtr = tempRegAddr;
    }
    return;
#else
    GT_U8 raven,tile;
    raven = serdesNum / 16 ;
    tile = raven/4;

    *addressPtr = 0x00340000 + regAddr + ((serdesNum  % 16) * 0x1000) + tile * 0x20000000;
    if ( tile%2 == 0 ) /*tile 0 */
        *addressPtr = *addressPtr + ((raven%4) * 0x1000000);
    else
        *addressPtr = *addressPtr + (0x3000000 - ((raven%4) * 0x1000000));

#endif
}
/**
* @internal hwsFalconGopAddrCalc function
* @endinternal
*
* @brief   Calculate GOP register address for Falcon
*
* @param[in] portNum                  - global port number
* @param[in] regAddr                  - offset of required register in GOP Unit (0x10000000)
*
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
GT_VOID hwsFalconGopAddrCalc
(
    GT_U8           devNum,
    GT_UOPT         portNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
)
{
    GT_U32 tempRegAddr = 0;

    if(PRV_NON_SHARED_LAB_SERVICES_DIR_FALCON_DEV_INIT_SRC_GLOBAL_VAR(falconBaseAddrCalcFunc) == NULL)
    {
        /* not implemented */
        *addressPtr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        return;
    }

    tempRegAddr = PRV_NON_SHARED_LAB_SERVICES_DIR_FALCON_DEV_INIT_SRC_GLOBAL_VAR(falconBaseAddrCalcFunc)(devNum,HWS_UNIT_BASE_ADDR_TYPE_GOP_E,portNum);
    if(tempRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        *addressPtr = regAddr + tempRegAddr;
    }
    else
    {
        *addressPtr = tempRegAddr;
    }

    return;
}

/**
* @internal hwsFalconSetRavenDevBmp function
* @endinternal
*
* @brief   for Falcon Z2 we can init only several raven and not
*          all.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsFalconSetRavenDevBmp
(
    IN GT_U32 ravenDevBmp
)
{
    hwsRavenDevBmp = ravenDevBmp;



    return GT_OK;
}

/**
* @internal hwsFalconSetRavenCm3Uart function
* @endinternal
*
* @brief   for Falcon enable cm3 uart for debug.
*
* @param[in] ravenCm3UartDev       - which raven cm3 device is
*       connected to uart
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsFalconSetRavenCm3Uart
(
    IN GT_U32 ravenCm3UartDev
)
{
    hwsRavenCm3UartDev = ravenCm3UartDev;
    return GT_OK;
}

#ifndef MICRO_INIT
/**
* @internal hwsFalconInterruptsD2dClear function
* @endinternal
*
* @brief   This routine clear interrupts errors on Raven/Eagle D2D PMA interrupt cause registers .
* @brief
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*/
static GT_STATUS hwsFalconInterruptsD2dClear
(
    IN  GT_U8                   devNum
)
{
    GT_STATUS   rc;
    static GT_U32 clear_per_pipe_Arr[] = {
        /* Tile 0 - pipe 0 addresses */
         0x0D7F8060  /* eagle D2D 0 PMA interrupt cause */
        ,0x0D808060  /* eagle D2D 1 PMA interrupt cause */
        ,0x0D818060  /* eagle D2D 2 PMA interrupt cause */
        ,0x0D828060  /* eagle D2D 3 PMA interrupt cause */

        ,0x0D7F7040  /* eagle D2D 0 PCS interrupt cause */
        ,0x0D807040  /* eagle D2D 1 PCS interrupt cause */
        ,0x0D817040  /* eagle D2D 2 PCS interrupt cause */
        ,0x0D827040  /* eagle D2D 3 PCS interrupt cause */

        ,GT_NA
    };
    static GT_U32 clear_per_raven_Arr[] = {
        /* Tile 0 - Raven 0 addresses */
         0x00688060  /* Raven D2D 0 PMA interrupt cause */
        ,0x00698060  /* Raven D2D 1 PMA interrupt cause */

        ,0x00687040  /* Raven D2D 0 PCS interrupt cause */
        ,0x00697040  /* Raven D2D 1 PCS interrupt cause */

        ,GT_NA
    };
    GT_U32 *currRegAddrPtr = &clear_per_pipe_Arr[0];
    GT_U32 regArrSize =  sizeof(clear_per_pipe_Arr)/sizeof(GT_U32);
    GT_U32 i;
    GT_U32  currRegAddr,value;
    GT_U32  tileId, pipeId, ravenId, index, globalRavenIndex;

    for(i=0; i< regArrSize ;i++)
    {
        if ( currRegAddrPtr[i] == GT_NA )
        {
            break;
        }
        currRegAddr = currRegAddrPtr[i];
        /* duplicate to all tiles */
        for(tileId = 0 ; tileId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles; tileId++)
        {
            /* duplicate to all pipes */
            for(pipeId = 0 ; pipeId < 2 ; pipeId++)
            {
                rc = prvCpssHwPpReadRegister(devNum,
                    currRegAddr + tileId * 0x20000000 + pipeId * 0x08000000,
                    &value);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    currRegAddrPtr = &clear_per_raven_Arr[0];
    regArrSize =  sizeof(clear_per_raven_Arr)/sizeof(GT_U32);
    for(i=0; i< regArrSize ;i++)
    {
        if ( currRegAddrPtr[i] == GT_NA )
        {
            break;
        }
        currRegAddr = currRegAddrPtr[i];
        /* duplicate to all tiles */
        for(tileId = 0 ; tileId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles; tileId++)
        {
            /* duplicate to all ravens */
            for(index = 0 ; index < 4 ; index++)
            {
                if((1 << tileId) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp)
                {
                    ravenId = (3 - index);
                }
                else
                {
                    ravenId = index;
                }

                globalRavenIndex = ravenId + (4*tileId);

                if (0 == (hwsRavenDevBmp & (1<<globalRavenIndex)))
                {
                    continue;
                }

                rc = prvCpssHwPpReadRegister(devNum,
                    currRegAddr + tileId * 0x20000000 + index * 0x01000000,
                    &value);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }


    return GT_OK;
}
#endif

