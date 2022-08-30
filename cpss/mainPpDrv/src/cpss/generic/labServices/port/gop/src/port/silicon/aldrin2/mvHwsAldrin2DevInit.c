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
* @file mvHwsAldrin2DevInit.c
*
* @brief Aldrin2 specific HW Services init
*
* @version   1
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/generic/labservices/port/gop/silicon/bobk/mvHwsBobKPortIf.h>
#include <cpss/generic/labservices/port/gop/silicon/aldrin2/mvHwsAldrin2PortIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralPortIf.h>

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacSgIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xgMac/mvHwsXgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMac28nmIf.h>

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/gPcs/mvHwsGPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/xPcs/mvHwsXPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcs28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/cgPcs/mvHwsCgPcs28nmIf.h>

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mv_hws_avago_if.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>

#ifndef MV_HWS_REDUCED_BUILD
#include <mvDdr3TrainingIpPrvIf.h>
#include <mvHwsDdr3BobK.h>
#endif

/************************* definition *****************************************************/

#define ALDRIN2_MPCS_BASE           (0x10180000)
#define ALDRIN2_CGPCS_BASE          (0x10300000)
#define ALDRIN2_GIG_BASE            (0x10000000)
#define ALDRIN2_XLG_BASE            (0x100C0000)
#define ALDRIN2_SERDES_BASE         (0x13000000)
#define ALDRIN2_CG_BASE             (0x10340000)
#define ALDRIN2_CG_RS_FEC_BASE      (0x10340800)
#define ALDRIN2_PTP_BASE            (0x10180800)

#define IND_OFFSET                  (0x1000)
#define CG_IND_OFFSET               (0x1000)

/* total sbus clients in device */
#define ALDRIN2_SERDES_MAP_ARR_SIZE         83

/* MACRO to add pipe offset to serdes sbus_address to match avago implementation */
#define HWS_AVAGO_SERDES_PIPE_MAC(sd,pipe)   \
        ( sd | pipe << 8 )

#define HWS_AVAGO_SERDES_PIPE_1_MAC(sd)   \
        HWS_AVAGO_SERDES_PIPE_MAC(sd,1)

/* laneNum for SBC_Unit_0: #0 to #31. laneNum for SBC_Unit_1: #32 to #72 */
static GT_U32 aldrin2SerdesToAvagoMap[ALDRIN2_MAX_AVAGO_SERDES_NUMBER] =
{
/* laneNum   SBC_Serdes */
 /* [0]  */      3,
 /* [1]  */      36,
 /* [2]  */      4,
 /* [3]  */      35,
 /* [4]  */      5,
 /* [5]  */      34,
 /* [6]  */      6,
 /* [7]  */      33,
 /* [8]  */      7,
 /* [9]  */      32,
 /* [10] */      8,
 /* [11] */      31,
 /* [12] */      9,
 /* [13] */      30,
 /* [14] */      10,
 /* [15] */      29,
 /* [16] */      11,
 /* [17] */      28,
 /* [18] */      12,
 /* [19] */      27,
 /* [20] */      13,
 /* [21] */      26,
 /* [22] */      14,
 /* [23] */      25,
 /* [24] */      22,
 /* [25] */      17,
 /* [26] */      23,
 /* [27] */      16,
 /* [28] */      20,
 /* [29] */      19,
 /* [30] */      21,
 /* [31] */      18,    /* end of SBC_Unit_0 */
 /* [32] */      HWS_AVAGO_SERDES_PIPE_1_MAC(26),
 /* [33] */      HWS_AVAGO_SERDES_PIPE_1_MAC(23),
 /* [34] */      HWS_AVAGO_SERDES_PIPE_1_MAC(27),
 /* [35] */      HWS_AVAGO_SERDES_PIPE_1_MAC(22),
 /* [36] */      HWS_AVAGO_SERDES_PIPE_1_MAC(28),
 /* [37]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(21),
 /* [38]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(29),
 /* [39]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(20),
 /* [40]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(30),
 /* [41]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(19),
 /* [42]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(31),
 /* [43]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(18),
 /* [44]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(32),
 /* [45]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(17),
 /* [46]  */     HWS_AVAGO_SERDES_PIPE_1_MAC(33),
 /* [47] */      HWS_AVAGO_SERDES_PIPE_1_MAC(16),
 /* [48] */      HWS_AVAGO_SERDES_PIPE_1_MAC(34),
 /* [49] */      HWS_AVAGO_SERDES_PIPE_1_MAC(15),
 /* [50] */      HWS_AVAGO_SERDES_PIPE_1_MAC(35),
 /* [51] */      HWS_AVAGO_SERDES_PIPE_1_MAC(14),
 /* [52] */      HWS_AVAGO_SERDES_PIPE_1_MAC(36),
 /* [53] */      HWS_AVAGO_SERDES_PIPE_1_MAC(13),
 /* [54] */      HWS_AVAGO_SERDES_PIPE_1_MAC(37),
 /* [55] */      HWS_AVAGO_SERDES_PIPE_1_MAC(12),
 /* [56] */      HWS_AVAGO_SERDES_PIPE_1_MAC(38),
 /* [57] */      HWS_AVAGO_SERDES_PIPE_1_MAC(11),
 /* [58] */      HWS_AVAGO_SERDES_PIPE_1_MAC(39),
 /* [59] */      HWS_AVAGO_SERDES_PIPE_1_MAC(10),
 /* [60] */      HWS_AVAGO_SERDES_PIPE_1_MAC(40),
 /* [61] */      HWS_AVAGO_SERDES_PIPE_1_MAC(9),
 /* [62] */      HWS_AVAGO_SERDES_PIPE_1_MAC(41),
 /* [63] */      HWS_AVAGO_SERDES_PIPE_1_MAC(8),
 /* [64] */      HWS_AVAGO_SERDES_PIPE_1_MAC(42),
 /* [65] */      HWS_AVAGO_SERDES_PIPE_1_MAC(7),
 /* [66] */      HWS_AVAGO_SERDES_PIPE_1_MAC(43),
 /* [67] */      HWS_AVAGO_SERDES_PIPE_1_MAC(6),
 /* [68] */      HWS_AVAGO_SERDES_PIPE_1_MAC(44),
 /* [69] */      HWS_AVAGO_SERDES_PIPE_1_MAC(5),
 /* [70] */      HWS_AVAGO_SERDES_PIPE_1_MAC(45),
 /* [71] */      HWS_AVAGO_SERDES_PIPE_1_MAC(4),
 /* [72] */      HWS_AVAGO_SERDES_PIPE_1_MAC(1)  /* CPU port */ /* end of SBC_Unit_1 */
};

/* Pipe0- SBC_Serdes for a single SBC_Unit: #0 to #36.
   TSEN for a single SBC_Unit is #1 and #24
   Pipe1 - SBC_Serdes for a single SBC_Unit: #0 to #45.
   TSEN for a single SBC_Unit is #2 and #24  */
GT_U32 aldrin2AvagoToSerdesMap[ALDRIN2_SERDES_MAP_ARR_SIZE] =
{
/* SBC_Serdes  laneNum */

    /* Pipe0 */

/* [0]  */       0, /* NOT USED*/
/* [1]  */       0, /* TSEN0 */
/* [2]  */       0, /* pmro0 */
/* [3]  */       0,
/* [4]  */       2,
/* [5]  */       4,
/* [6]  */       6,
/* [7]  */       8,
/* [8]  */      10,
/* [9]  */      12,
/* [10] */      14,
/* [11] */      16,
/* [12] */      18,
/* [13] */      20,
/* [14] */      22,
/* [15] */       0, /* pmro1 */
/* [16] */      27,
/* [17] */      25,
/* [18] */      31,
/* [19] */      29,
/* [20] */      28,
/* [21] */      30,
/* [22] */      24,
/* [23] */      26,
/* [24] */       0, /* TSEN1 */
/* [25] */      23,
/* [26] */      21,
/* [27] */      19,
/* [28] */      17,
/* [29] */      15,
/* [30] */      13,
/* [31] */      11,
/* [32] */       9,
/* [33] */       7,
/* [34] */       5,
/* [35] */       3,
/* [36] */       1,

    /* Pipe1 - keeping pipe1 mapping in
       the high indexes of this array */

/* SBC_Serdes  laneNum */
/* [0]  */       0, /* NOT USED*/
/* [1]  */       72, /* CPU */
/* [2]  */       0, /* TSEN3 */
/* [3]  */       0, /* pmro3 */
/* [4]  */       71,
/* [5]  */       69,
/* [6]  */       67,
/* [7]  */       65,
/* [8]  */       63,
/* [9]  */       61,
/* [10] */       59,
/* [11] */       57,
/* [12] */       55,
/* [13] */       53,
/* [14] */       51,
/* [15] */       49, /* pmro1 */
/* [16] */       47,
/* [17] */       45,
/* [18] */       43,
/* [19] */       41,
/* [20] */       39,
/* [21] */       37,
/* [22] */       35,
/* [23] */       33,
/* [24] */        0, /* TSEN2 */
/* [25] */        0, /* pmro2 */
/* [26] */       32,
/* [27] */       34,
/* [28] */       36,
/* [29] */       38,
/* [30] */       40,
/* [31] */       42,
/* [32] */       44,
/* [33] */       46,
/* [34] */       48,
/* [35] */       50,
/* [36] */       52,
/* [37] */       54,
/* [38] */       56,
/* [39] */       58,
/* [40] */       60,
/* [41] */       62,
/* [42] */       64,
/* [43] */       66,
/* [44] */       68,
/* [45] */       70
};

/************************* Globals *******************************************************/

extern MV_HWS_PORT_INIT_PARAMS *hwsPortsAldrin2Params[];

extern const MV_HWS_PORT_INIT_PARAMS hwsAldrin2Port0SupModes[];
extern const MV_HWS_PORT_INIT_PARAMS hwsAldrin2Port24SupModes[];
extern const MV_HWS_PORT_INIT_PARAMS hwsAldrin2Port72SupModes[];
extern const MV_HWS_PORT_INIT_PARAMS *hwsPortsAldrin2ParamsSupModesMap[];

/* init per device */
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL aldrin2DbInitDone = GT_FALSE;

/************************* pre-declaration ***********************************************/
#if (!defined MV_HWS_REDUCED_BUILD)
extern GT_STATUS mvHwsGeneralClkSelCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);
#endif

/*****************************************************************************************/

/**
* @internal hwsAldrin2ConvertSbusAddrToSerdes function
* @endinternal
*
* @brief   Convert sbus address to serdes number.
*
* @param[in] devNum                   - system device number
* @param[in] sbusAddr                 - sbus address
*
* @param[out] serdesNum               - serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static int hwsAldrin2ConvertSbusAddrToSerdes
(
    IN unsigned char    devNum,
    OUT GT_U32          *serdesNum,
    IN unsigned int     sbusAddr
)
{
    *serdesNum = 0;

    /* ring1 check */
    if (sbusAddr & (1 << 8))
    {
        /* remove the ring1 tag if exist */
        sbusAddr &= 0xFF;
        /* add ring1 offset to the mapping array */
        sbusAddr += 36;
    }

    /* array range check */
    if(sbusAddr > hwsDeviceSpecInfo[devNum].avagoSerdesInfo.spicoNumber)
    {
        return GT_BAD_PARAM;
    }

    *serdesNum = hwsDeviceSpecInfo[devNum].avagoSerdesInfo.avagoToSerdesMapPtr[sbusAddr];

    return GT_OK;
}

/**
* @internal pcsIfInit function
* @endinternal
*
* @brief   Init all supported PCS types relevant for devices.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS pcsIfInit
(
    IN GT_U8 devNum
)
{
    MV_HWS_PCS_FUNC_PTRS **hwsPcsFuncsPtr;

    CHECK_STATUS(hwsPcsGetFuncPtr(devNum, &hwsPcsFuncsPtr));

    CHECK_STATUS(mvHwsGPcs28nmIfInit(hwsPcsFuncsPtr));  /* GPCS */
    CHECK_STATUS(mvHwsMMPcs28nmIfInit(hwsPcsFuncsPtr)); /* MMPCS */
    CHECK_STATUS(mvHwsCgPcs28nmIfInit(devNum, hwsPcsFuncsPtr)); /* CGPCS */

    return GT_OK;
}

/**
* @internal macIfInit function
* @endinternal
*
* @brief   Init all supported MAC types.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS macIfInit
(
    IN GT_U8 devNum
)
{
    MV_HWS_MAC_FUNC_PTRS **hwsMacFuncsPtr;

    CHECK_STATUS(hwsMacGetFuncPtr(devNum, &hwsMacFuncsPtr));

    CHECK_STATUS(hwsGeMac28nmIfInit (devNum, hwsMacFuncsPtr));  /* GE MAC  */
    CHECK_STATUS(hwsXgMac28nmIfInit (hwsMacFuncsPtr));  /* XG MAC  */
    CHECK_STATUS(hwsXlgMac28nmIfInit(devNum, hwsMacFuncsPtr));  /* XLG MAC */
    CHECK_STATUS(hwsCgMac28nmIfInit (devNum, hwsMacFuncsPtr));  /* CG MAC  */
#if !defined (MV_HWS_REDUCED_BUILD) || defined (MICRO_INIT)
    /* ILKN/SG configuration is not done in Internal CPU */
    CHECK_STATUS(hwsGeMacSgIfInit(devNum, hwsMacFuncsPtr));   /* GE_SG MAC */
#endif
    return GT_OK;
}

/**
* @internal hwsAldrin2SerdesIfInit function
* @endinternal
*
* @brief   Init all supported Serdes types.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS hwsAldrin2SerdesIfInit
(
    IN GT_U8 devNum
)
{
  MV_HWS_SERDES_FUNC_PTRS **hwsSerdesFuncsPtr;
#if !defined (ASIC_SIMULATION) && !defined (MV_HWS_REDUCED_BUILD_EXT_CM3)
    CHECK_STATUS(mvHwsAaplSerdesDbInit(devNum, 1));
#endif
  CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));

  CHECK_STATUS(mvHwsAvagoIfInit(devNum, hwsSerdesFuncsPtr));

  return GT_OK;
}

/**
* @internal hwsAldrin2IfPreInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @param[in] devNum                   - system device number
* @param[in] funcPtr                  - pointer to structure that hold the "os"
*                                      functions needed be bound to HWS.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsAldrin2IfPreInit
(
    IN GT_U8             devNum,
    IN HWS_OS_FUNC_PTR   *funcPtr
)
{
    GT_U32 i;

    if (funcPtr == NULL)
        return GT_FAIL;

    if ((funcPtr->osTimerWkPtr == NULL) || (funcPtr->osMemSetPtr == NULL) ||
        (funcPtr->osFreePtr == NULL) || (funcPtr->osMallocPtr == NULL) ||
        (funcPtr->osExactDelayPtr == NULL) || (funcPtr->sysDeviceInfo == NULL) ||
        (funcPtr->osMemCopyPtr == NULL) || (funcPtr->serdesRegSetAccess == NULL) ||
        (funcPtr->serdesRegGetAccess == NULL) || (funcPtr->serverRegSetAccess == NULL)
        || (funcPtr->serverRegGetAccess == NULL) ||
        (funcPtr->registerSetAccess == NULL) || (funcPtr->registerGetAccess == NULL)
#ifndef MV_HWS_FREE_RTOS
        || (funcPtr->timerGet == NULL)
        || (funcPtr->osStrCatPtr == NULL)
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
    hwsOsStrCatFuncPtr = funcPtr->osStrCatPtr;
    hwsServerRegFieldSetFuncPtr = funcPtr->serverRegFieldSetAccess;
    hwsServerRegFieldGetFuncPtr = funcPtr->serverRegFieldGetAccess;
#ifndef MV_HWS_FREE_RTOS
    hwsTimerGetFuncPtr = funcPtr->timerGet;
#endif

    hwsRegisterSetFuncPtr = funcPtr->registerSetAccess;
    hwsRegisterGetFuncPtr = funcPtr->registerGetAccess;

    /* needs to be initialized before configuring the CPLL */
    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }

    hwsDeviceSpecInfo[devNum].devType = Aldrin2;

    return GT_OK;
}
#ifndef MV_HWS_REDUCED_BUILD

GT_VOID hwsAldrin2PortsParamsSupModesMapSet
(
    GT_U8 devNum
)
{
    PRV_PORTS_PARAMS_SUP_MODE_MAP(devNum) = hwsPortsAldrin2ParamsSupModesMap;
}

#endif

GT_VOID hwsAldrin2IfInitHwsDevFunc
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
#ifndef MV_HWS_REDUCED_BUILD

   hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsSetFunc         = NULL;
   hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsGetLanesFunc    = NULL;

   hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc          = mvHwsGeneralClkSelCfg;
#endif
   hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc = NULL;

   hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc = NULL;

   /* Configures port init / reset functions */
   #if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)
   hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc = mvHwsGeneralPortInit;
   #endif
   hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devApPortinitFunc = mvHwsGeneralApPortInit;
   hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortResetFunc = mvHwsGeneralPortReset;

   /* Configures  device handler functions */
   hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetExtFunc = mvHwsPortAvagoAutoTuneSetExt;
   hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgFunc     = NULL;                         /* TBD - Need to be implemented */
   hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgGetFunc  = NULL;                         /* TBD - Need to be implemented */
   hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsIndexGetFunc = NULL;
   hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portLbSetWaFunc        = NULL;
   hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneStopFunc   = NULL;
   hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetWaFunc  = NULL;
   hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portFixAlign90Func     = NULL;
}

/**
* @internal hwsAldrin2IfInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @param[in] devNum                   - system device number
* @param[in] funcPtr                  - pointer to structure that hold the "os"
*                                      functions needed be bound to HWS.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsAldrin2IfInit
(
    IN GT_U8             devNum,
    IN HWS_OS_FUNC_PTR   *funcPtr
)
{
    GT_U32  portGroup = devNum;
    GT_U32  data, version;
    MV_HWS_DEV_TYPE devType;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
    GT_U32  i;
#ifndef MV_HWS_REDUCED_BUILD
    GT_STATUS rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
#endif

    if (hwsInitDone[devNum])
    {
        return GT_ALREADY_EXIST;
    }
#ifndef MV_HWS_REDUCED_BUILD
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
#endif


#ifdef MV_HWS_REDUCED_BUILD
    if(aldrin2DbInitDone == GT_FALSE)
    {
        CHECK_STATUS(hwsAldrin2IfPreInit(devNum, funcPtr));
    }
#endif

    hwsDeviceSpecInfo[devNum].devType = Aldrin2;
    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].portsNum = HWS_ALDRIN2_PORTS_NUM_CNS;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = LAST_PORT_MODE;
    hwsDeviceSpecInfo[devNum].serdesType = AVAGO;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesNumber = MAX_AVAGO_SERDES_NUMBER;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.spicoNumber = ALDRIN2_SERDES_MAP_ARR_SIZE;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesToAvagoMapPtr = &aldrin2SerdesToAvagoMap[0];
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.avagoToSerdesMapPtr = &aldrin2AvagoToSerdesMap[0];
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesVosOverride = GT_TRUE;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.sbusAddrToSerdesFuncPtr = hwsAldrin2ConvertSbusAddrToSerdes;
    for (i = 0; i < MV_HWS_MAX_CTLE_BIT_MAPS_NUM; i++)
    {
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasOverride[i] = 0;
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasValue[i] = 0;
    }
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasLanesNum = 0;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasLanesNum = 0;

    devType = hwsDeviceSpecInfo[devNum].devType;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    hwsAldrin2IfInitHwsDevFunc(devNum, funcPtr);

#ifndef MV_HWS_REDUCED_BUILD
    /* define IPC connection Type Host side */
    if (mvHwsServiceCpuEnableGet(devNum))
    {
        /* AP FW is loaded to Service CPU, use IPC mode - API execution at the Service CPU */

        /* !!! Replace HOST2HWS_LEGACY_MODE with HOST2SERVICE_CPU_IPC_CONNECTION once approved by CPSS !!! */
        hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE/*HOST2SERVICE_CPU_IPC_CONNECTION*/;

        if (!((tempSystemRecovery_Info.systemRecoveryMode.ha2phasesInitPhase == CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_PHASE1_E) &&
              ( tempSystemRecovery_Info.systemRecoveryMode.haReadWriteState != CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E)))
        {
            /* Init HWS Firmware */
            CHECK_STATUS(mvHwsServiceCpuFwInit(devNum, 0, "AP_Aldrin2"));
        }
    }
    else
    {
        /* AP FW is NOT loaded to Service CPU, use Legacy mode - API execution at the Host */
        hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;
    }

    if (mvHwsServiceCpuDbaEnableGet(devNum))
    {
        /* DBA FW is loaded to Second Service CPU */
        if (!((tempSystemRecovery_Info.systemRecoveryMode.ha2phasesInitPhase == CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_PHASE1_E) &&
              ( tempSystemRecovery_Info.systemRecoveryMode.haReadWriteState != CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E)))
        {
            CHECK_STATUS(mvHwsServiceCpuDbaFwInit(devNum));
        }
    }
    hwsAldrin2PortsParamsSupModesMapSet(devNum);

#else
    /* define IPC connection Type Service CPU side */
    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;
#endif /* MV_HWS_REDUCED_BUILD */

    /* get device id and version */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc(devNum, &data, &version);

    hwsDeviceSpecInfo[devNum].gopRev = GOP_28NM_REV3;


    hwsInitPortsModesParam(devNum,NULL);
    portGroup = portGroup;
    if(hwsInitDone[devNum] == GT_FALSE)
    {
#ifndef MV_HWS_REDUCED_BUILD
        CHECK_STATUS(hwsDevicePortsElementsCfg(devNum));
#else
        CHECK_STATUS(hwsAldrin2PortsElementsCfg(devNum, portGroup));
#endif
    }

    /* Init all MAC units relevant for current device */
    CHECK_STATUS(macIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));

    /* Init SERDES unit relevant for current device */
    CHECK_STATUS(hwsAldrin2SerdesIfInit(devNum));

    /* Init port manager db */
#ifndef MV_HWS_FREE_RTOS
    CHECK_STATUS_EXT(mvHwsPortManagerInit(devNum), LOG_ARG_STRING_MAC("port manager init failed"));
#endif /* MV_HWS_FREE_RTOS */

#ifdef MV_HWS_FREE_RTOS
    CHECK_STATUS_EXT(mvHwsPortEnhanceTuneLitePhaseDeviceInit(devNum),
                     LOG_ARG_STRING_MAC("EnhanceTuneLite device init failed"));
#endif

    /* Update HWS After Firmware load and init */
    if (mvHwsServiceCpuEnableGet(devNum))
    {
#ifndef MV_HWS_REDUCED_BUILD
        /* DBA FW is loaded to Second Service CPU */
        if (!((tempSystemRecovery_Info.systemRecoveryMode.ha2phasesInitPhase == CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_PHASE1_E) &&
              ( tempSystemRecovery_Info.systemRecoveryMode.haReadWriteState != CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E)))
        {
#endif
            CHECK_STATUS(mvHwsServiceCpuFwPostInit(devNum, devNum));
#ifndef MV_HWS_REDUCED_BUILD
        }
#endif
    }

    /* Init configuration sequence executer */
    mvCfgSeqExecInit();

    mvUnitInfoSet(devType,  SERDES_UNIT,     ALDRIN2_SERDES_BASE,     IND_OFFSET);
    mvUnitInfoSet(devType,  MMPCS_UNIT,      ALDRIN2_MPCS_BASE,       IND_OFFSET);
    mvUnitInfoSet(devType,  CGPCS_UNIT,      ALDRIN2_CGPCS_BASE,      CG_IND_OFFSET);
    mvUnitInfoSet(devType,  GEMAC_UNIT,      ALDRIN2_GIG_BASE,        IND_OFFSET);
    mvUnitInfoSet(devType,  XLGMAC_UNIT,     ALDRIN2_XLG_BASE,        IND_OFFSET);
    mvUnitInfoSet(devType,  CG_UNIT,         ALDRIN2_CG_BASE,         CG_IND_OFFSET);
    mvUnitInfoSet(devType,  CG_RS_FEC_UNIT,  ALDRIN2_CG_RS_FEC_BASE,  CG_IND_OFFSET);
    mvUnitInfoSet(devType,  PTP_UNIT,        ALDRIN2_PTP_BASE,        IND_OFFSET);

    aldrin2DbInitDone = GT_TRUE;
    hwsInitDone[devNum] = GT_TRUE;

    return GT_OK;
}
#if 0
/**
* @internal hwsAldrin2PortCGCheck function
* @endinternal
*
* @brief   Check if the port is CG port in Aldrin2 device
*
* @param[in] portNum                  - global MAC port number
*
* @retval 0                        - not CG port
* @retval 1                        - CG port
*/
GT_U32 hwsAldrin2PortCGCheck
(
    GT_UOPT         portNum
)
{
    /* port :
        0,4,8    (DP[0]  local ports 0,4,8)
        12,16,20 (DP[1] local ports 0,4,8)
    */
    return ((portNum % 4) == 0 && portNum <= 20);
}
#endif
/**
* @internal hwsAldrin2SerdesAddrCalc function
* @endinternal
*
* @brief   Calculate serdes register address for Aldrin2
*
* @param[in] serdesNum                - global serdes lane number
* @param[in] regAddr                  - offset of required register in SD Unit (0x13000000)
*
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
GT_VOID hwsAldrin2SerdesAddrCalc
(
    IN GT_UOPT         serdesNum,
    IN GT_UREG_DATA    regAddr,
    OUT GT_U32         *addressPtr
)
{
    GT_U32 mgChain;
    GT_U32 localSerdes;

    if (serdesNum < 24)/*0..23*/
    {
        localSerdes = serdesNum;
        mgChain = 0;
    }
    else if(serdesNum < 72)/*24..71*/
    {
        localSerdes = serdesNum - 24;
        mgChain = 1;
    }
    else if(serdesNum == 72)
    {
        localSerdes = 63;
        mgChain = 1;
    }
    else /* >= 73 */
    {
        /* not valid address */
        *addressPtr = 0x11111111;
        return;
    }

    *addressPtr = 0x13000000 + regAddr + (0x1000 * localSerdes) + (mgChain * 0x00080000);


    return;
}
/**
* @internal hwsAldrin2GopAddrCalc function
* @endinternal
*
* @brief   Calculate GOP register address for Aldrin2
*
* @param[in] portNum                  - global port number
* @param[in] regAddr                  - offset of required register in GOP Unit (0x10000000)
*
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
GT_VOID hwsAldrin2GopAddrCalc
(
    IN GT_UOPT         portNum,
    IN GT_UREG_DATA    regAddr,
    OUT GT_U32         *addressPtr
)
{
    GT_U32 mgChain;
    GT_U32 localPort;

    if (portNum < 24)/*0..23*/
    {
        localPort = portNum;
        mgChain = 0;
    }
    else if(portNum < 72)/*24..71*/
    {
        localPort = portNum - 24;
        mgChain = 1;
    }
    else if(portNum == 72)
    {
        localPort = 63;
        mgChain = 1;
    }
    else /* >= 73 */
    {
        /* not valid address */
        *addressPtr = 0x11111111;
        return;
    }

    *addressPtr = 0x10000000 + regAddr + (0x1000 * localPort) + (mgChain * 0x00400000);


    return;
}
/**
* @internal hwsAldrin2MibAddrCalc function
* @endinternal
*
* @brief   Calculate MIB register address for Aldrin2
*
* @param[in] portNum                  - global port number
* @param[in] regAddr                  - offset of required register in MIB Unit (0x12000000)
*
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
GT_VOID hwsAldrin2MibAddrCalc
(
    IN GT_UOPT         portNum,
    IN GT_UREG_DATA    regAddr,
    OUT GT_U32          *addressPtr
)
{
    GT_U32 mgChain;
    GT_U32 localPort;

    if (portNum < 24)/*0..23*/
    {
        localPort = portNum;
        mgChain = 0;
    }
    else if(portNum < 72)/*24..71*/
    {
        localPort = portNum - 24;
        mgChain = 1;
    }
    else if(portNum == 72)
    {
        localPort = 63;
        mgChain = 1;
    }
    else /* >= 73 */
    {
        /* not valid address */
        *addressPtr = 0x11111111;
        return;
    }

    *addressPtr = 0x12000000 + regAddr + (0x400 * localPort) + (mgChain * 0x00800000);


    return;
}

#ifndef MV_HWS_REDUCED_BUILD
/**
* @internal serdesIfClose function
* @endinternal
*
* @brief   Free all resources allocated by supported serdes types.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static void serdesIfClose
(
    IN GT_U8 devNum
)
{
    mvHwsAvagoIfClose(devNum);
}

/**
* @internal hwsAldrin2IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsAldrin2IfClose
(
    IN GT_U8 devNum
)
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
            break;;
        }
    }
    if (i == HWS_MAX_DEVICE_NUM)
    {
        aldrin2DbInitDone = GT_FALSE;
    }

    serdesIfClose(devNum);

    hwsSerdesIfClose(devNum);
    hwsPcsIfClose(devNum);
    hwsMacIfClose(devNum);

#ifndef MV_HWS_FREE_RTOS
    /* we can use this API also in simulation, to be consistent with port manager hws APIs,
       they just return GT_OK */
    mvHwsPortManagerClose(devNum);
#endif /* MV_HWS_FREE_RTOS */

    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }
    hwsPortsElementsClose(devNum);
}

#endif
