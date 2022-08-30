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
* @file mvAvagoSerdesIf.c \
*
* @brief Avago interface
*
* @version   1
********************************************************************************
*/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>

#ifdef MV_HWS_REDUCED_BUILD_EXT_CM3
#include <mvHwsPortCtrlInc.h>
#endif

#define MV_HWS_AVAGO_SBUS_RESET_TIMEOUT 500
#define MV_HWS_AVAGO_LOW_POWER_MODE_ENABLE_SKIP 0

#define ADAPTIVE_CTLE_MAX_DELAY 15
#define ADAPTIVE_CTLE_MIN_DELAY 0
#define ADAPTIVE_CTLE_MAX_LF 15
#define ADAPTIVE_CTLE_MIN_LF 0

/*#define ADAPT_CTLE_DEBUG 1*/


#ifdef _VISUALC
#pragma warning(disable : 4100)
/*relate to :FORCE_FUNC_CAST */
#pragma warning(disable : 4054) /* warning C4054: from function pointer 'GT_STATUS (__cdecl *)(void)' to data pointer 'void *' */
/*relate to :FORCE_FUNC_CAST */
#pragma warning(disable : 4152) /* warning C4152: nonstandard extension, function/data pointer conversion in expression */
#endif

#ifndef ASIC_SIMULATION
/* Avago include */
#include <cpss/common/labServices/port/gop/port/serdes/avago/mv_hws_avago_if.h>
#include <include/aapl.h>
#include <include/pmro.h>
extern void serdes_dfe_update_ctle(Aapl_t *aapl, uint addr, Avago_serdes_dfe_state_t *dfe_state, int int_code);
extern void serdes_dfe_update_gain(Aapl_t *aapl, uint addr, Avago_serdes_dfe_state_t *dfe_state, BOOL enable_write);

#ifdef CHX_FAMILY
extern GT_U32  hwsFalconNumOfRavens;
#endif

#else
GT_STATUS mvHwsAvagoSerdesOperation
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      serdesNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_SERDES_OPERATION_MODE_ENT  operation,
    GT_U32                     *data,
    GT_U32                     *result
)
{
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(portMode);
    GT_UNUSED_PARAM(operation);
    GT_UNUSED_PARAM(data);
    GT_UNUSED_PARAM(result);
    return GT_OK;
}
#endif /* ASIC_SIMULATION */

#ifdef MV_HWS_REDUCED_BUILD_EXT_CM3
#define MARVELL_AVAGO_NO_DEBUG_FLAG
#endif

#ifndef MARVELL_AVAGO_NO_DEBUG_FLAG
#define HWS_AVAGO_DBG(s) hwsOsPrintf s
#else
#define HWS_AVAGO_DBG(s)
#endif

#ifdef MARVELL_AVAGO_DEBUG_FLAG
#define AVAGO_DBG(s) hwsOsPrintf s
#else
#define AVAGO_DBG(s)
#endif /* MARVELL_AVAGO_DEBUG_FLAG */

#define ARR_SIZE_CALC_MAC(_arr) sizeof(_arr)/sizeof(_arr[0])

/* define a macro to calculate the execution time */
#if !defined(CPSS_BLOB) && !defined(MV_HWS_FREE_RTOS)
    #define EXEC_TIME_START() \
        GT_U32 start_Seconds, start_NanoSeconds; \
        hwsTimeRT(&start_Seconds, &start_NanoSeconds);
    #define EXEC_TIME_END_PRINT(_msg) \
        { \
            GT_U32 end_Seconds, end_NanoSeconds; \
            hwsTimeRT(&end_Seconds, &end_NanoSeconds); \
            if (end_NanoSeconds < start_NanoSeconds) \
            { \
                end_NanoSeconds += 1000000000; \
                end_Seconds--; \
            } \
            HWS_AVAGO_DBG((" Time is: %d.%09d seconds\n", (end_Seconds-start_Seconds), (end_NanoSeconds-start_NanoSeconds))); \
        }
#else
    #define EXEC_TIME_START()
    #define EXEC_TIME_END_PRINT(_msg)
#endif

/* #define TUNE_BY_PHASE_DBG */
#ifdef TUNE_BY_PHASE_DBG
#define AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(...)  \
    hwsOsPrintf(__VA_ARGS__)
#else
#define AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(...)
#endif

#if defined MV_HWS_FREE_RTOS
    uint  (*aapl_user_supplied_log_func_ptr)  (const char  *funcNamePtr, char  *strMsgPtr);
#elif !defined ASIC_SIMULATION /* !MV_HWS_FREE_RTOS */
    uint  (*aapl_user_supplied_log_func_ptr)  (const char  *funcNamePtr, char  *fileNamePtr, unsigned int  lineNum, int  returnCode, char  *strMsgPtr);
#endif


#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
extern GT_BOOL  prvCpssSystemRecoveryInProgress
(
    GT_VOID
);

#endif


#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
#include <cpssCommon/private/prvCpssEmulatorMode.h>
/* return 0 when Avago     supported
   return 1 when Avago NOT supported
*/
GT_U32  hwsAvagoNotSupportedCheck(void)
{
    /* the emulator not supports the memories of <SERDES And SBC Controller> */
    return cpssDeviceRunCheck_onEmulator();
}
/* function that do nothing and return GT_OK */
/* it should allow to avoid any access to AVAGO driver code */
static  GT_STATUS hwsAvagoNotSupported_dummySet(void)
{
    return GT_OK;
}
/* macro to force casting between 2 functions prototypes */
#define FORCE_FUNC_CAST (void*)
/* macro to bind CB function (for 'Set' functions ... without 'OUT' parameters)*/
#define BIND_SET_FUNC(func)                                 \
    hwsAvagoNotSupportedCheck() ?                           \
        FORCE_FUNC_CAST hwsAvagoNotSupported_dummySet :     \
        func


static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesLoopbackGet
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    MV_HWS_SERDES_LB  *lbModePtr
)
{
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    *lbModePtr = SERDES_LP_DISABLE;
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesTestGenGet
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     serdesNum,
    MV_HWS_SERDES_TX_PATTERN    *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE *modePtr
)
{
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(txPatternPtr);
    GT_UNUSED_PARAM(modePtr);
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesTestGenStatus
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          serdesNum,
    MV_HWS_SERDES_TX_PATTERN        txPattern,
    GT_BOOL                         counterAccumulateMode,
    MV_HWS_SERDES_TEST_GEN_STATUS   *status
)
{
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(txPattern);
    GT_UNUSED_PARAM(counterAccumulateMode);
    hwsOsMemSetFuncPtr(status,0,sizeof(*status));
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesPolarityConfigGet
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    GT_BOOL           *invertTx,
    GT_BOOL           *invertRx
)
{
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(invertTx);
    GT_UNUSED_PARAM(invertRx);
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesTxEnableGet
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     *enablePtr
)
{
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(enablePtr);

    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesTxIfSelectGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_U32      *serdesTxIfNum
)
{
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    *serdesTxIfNum = 0;
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdeSpeedGet
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                serdesNum,
    MV_HWS_SERDES_SPEED   *rate
)
{
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(rate);
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesManualTxConfigGet
(
    GT_U8                           devNum,
    GT_UOPT                         portGroup,
    GT_UOPT                         serdesNum,
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT  *configParams
)
{
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    hwsOsMemSetFuncPtr(configParams,0,sizeof(*configParams));
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesCdrLockStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_BOOL                 *enable
)
{
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    *enable = GT_TRUE;
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesEncodingTypeGet
(
    GT_U8                            devNum,
    GT_U32                           serdesNum,
    MV_HWS_SERDES_ENCODING_TYPE     *txEncodingPtr,
    MV_HWS_SERDES_ENCODING_TYPE     *rxEncodingPtr
)
{
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    *txEncodingPtr = SERDES_ENCODING_NA;
    *rxEncodingPtr = SERDES_ENCODING_NA;
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesRxDatapathConfigGet
(
    GT_U8                                  devNum,
    GT_U32                                 serdesNum,
    MV_HWS_SERDES_RX_DATAPATH_CONFIG_STC   *rxDatapathConfigPtr
)
{
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    hwsOsMemSetFuncPtr(rxDatapathConfigPtr,0,sizeof(*rxDatapathConfigPtr));
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesRxPllLockGet
(
    GT_U8                                  devNum,
    GT_U32                                 serdesNum,
    GT_BOOL                                *lockPtr
)
{
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    *lockPtr = GT_FALSE;
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesSignalDetectGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_BOOL     *signalDet
)
{
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(signalDet);
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesErrorInject
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_UOPT     numOfBits,
    IN MV_HWS_SERDES_DIRECTION    serdesDirection
)
{
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(numOfBits);
    GT_UNUSED_PARAM(serdesDirection);
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvago16nmSerdesPmroMetricGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       serdesNum,
    OUT GT_U16       *metricPtr
)
{
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(metricPtr);
    return GT_OK;
}

/* macro to bind CB function (for 'Get' functions ... with 'OUT' parameters)*/
/* the macro requires function with same name as 'orig' but with prefix 'hwsAvagoNotSupported_' */
#define BIND_GET_FUNC(func)               \
    hwsAvagoNotSupportedCheck() ?         \
        hwsAvagoNotSupported_##func :     \
        func

#define RETURN_GT_OK_ON_EMULATOR       \
    if(hwsAvagoNotSupportedCheck())    \
    {                                  \
        /* no SERDESes on emulator */  \
        return GT_OK;                  \
    }


#else /*defined MV_HWS_REDUCED_BUILD_EXT_CM3 */
#define BIND_SET_FUNC(func)               \
        func

#define BIND_GET_FUNC(func)               \
        func

#define RETURN_GT_OK_ON_EMULATOR       /* empty */

#endif /*defined MV_HWS_REDUCED_BUILD_EXT_CM3 */

/************************* definition *****************************************************/
#define MV_AVAGO_MIN_EYE_SIZE           10
#define MV_AVAGO_EYE_TAP                4

#define ICAL_TRAINING_MODE              0
#define PCAL_TRAINING_MODE              1

#define SKIP_SERDES                     0xFFFF

#define MV_AVAGO_TX_EQUALIZE_PASS       (0x54)

/************************* Globals *******************************************************/
#ifndef ASIC_SIMULATION

/* For non-AP port: delay values for EnhanceTune and EnhanceTuneLite configuration */
GT_U32  static_delay_10G[] = {25, 26, 28, 30}; /* best delay values for 10G speed */
#ifndef ALDRIN_DEV_SUPPORT
GT_U32  static_delay_25G[] = {15, 17, 19, 21}; /* best delay values for 25G speed */
#endif
/* Dividers DB for ref clock 156.25Mhz */
GT_U8 avagoBaudRate2DividerValue_156M[LAST_MV_HWS_SERDES_SPEED]=
{
    NA_VALUE,   /*  SPEED_NA  */
    8,          /*  _1_25G    */
    20,         /*  _3_125G   */
    NA_VALUE,   /*  _3_33G    */
    24,         /*  _3_75G    */
    NA_VALUE,   /*  _4_25G    */
    32,         /*  _5G       */
    40,         /*  _6_25G    */
    48,         /*  _7_5G     */
    66,         /*  _10_3125G */
    72,         /*  _11_25G   */
    NA_VALUE,   /*  _11_5625G */
    80,         /*  _12_5G    */
    70,         /*  _10_9375G */
    78,         /*  _12_1875G */
    NA_VALUE,   /*  _5_625G   */
    33,         /*  _5_15625G */
    NA_VALUE,   /*  _12_8906G */
    132,        /*  _20_625G  */
    165,        /*  _25_78125G*/
    176,        /*  _27_5G    */
    NA_VALUE,   /*  _28_05G   */
    170,        /*  _26_5625G */
    181,        /*  _28_28125G*/
    170,        /*  _26_5625G_PAM4 */
    180,        /*  _28_125G_PAM4*/
    168,        /* _26_25G         */
    NA_VALUE,   /* _27_1G          */
    66,         /* _10_3125G_SR_LR */
    165,        /* _25_78125G_SR_LR */
    NA_VALUE,   /*_26_5625G_PAM4_SR_LR */
    175,        /* _27_34375G */
    NA_VALUE,   /* _10G */
    NA_VALUE,   /* _2_578125 */
    NA_VALUE,   /* _1_25G_SR_LR */
    NA_VALUE,   /* _26_5625G_SR_LR */
    174,        /* _27_1875_PAM4 */
};

/* Dividers DB for ref clock 78Mhz */
GT_U8 avagoBaudRate2DividerValue_78M[LAST_MV_HWS_SERDES_SPEED]=
{
    NA_VALUE,   /*  SPEED_NA  */
    16,         /*  _1_25G    */
    40,         /*  _3_125G   */
    NA_VALUE,   /*  _3_33G    */
    48,         /*  _3_75G    */
    NA_VALUE,   /*  _4_25G    */
    64,         /*  _5G       */
    80,         /*  _6_25G    */
    96,         /*  _7_5G     */
    132,        /*  _10_3125G */
    NA_VALUE,   /*  _11_25G   */
    NA_VALUE,   /*  _11_5625G */
    160,        /*  _12_5G    */
    NA_VALUE,   /*  _10_9375G */
    156,        /*  _12_1875G */
    NA_VALUE,   /*  _5_625G   */
    66,         /*  _5_15625G */
    165,        /*  _12_8906G */
    NA_VALUE,   /*  _20_625G  */
    NA_VALUE,   /*  _25_78125G*/
    NA_VALUE,   /*  _27_5G    */
    NA_VALUE,   /*  _28_05G   */
    NA_VALUE,   /*  _26_5625G */
    NA_VALUE,   /*  _28_28125G*/
    NA_VALUE,   /* _26_5625G_PAM4  */
    NA_VALUE,   /* _28_125G_PAM4 */
    NA_VALUE,   /* _26_25G         */
    NA_VALUE,   /* _27_1G          */
    132,        /*  _10_3125G_SR_LR */
    NA_VALUE,   /* _25_78125G_SR_LR */
    NA_VALUE,   /*_26_5625G_PAM4_SR_LR */
    NA_VALUE,   /* _27_34375G */
    NA_VALUE,   /* _10G */
    NA_VALUE,   /* _2_578125 */
    NA_VALUE,   /* _1_25G_SR_LR */
    NA_VALUE,   /* _26_5625G_SR_LR */
    NA_VALUE    /* _27_1875_PAM4 */
};

/* Dividers DB for ref clock 312.5Mhz */
GT_U8 avagoBaudRate2DividerValue_312_5M[LAST_MV_HWS_SERDES_SPEED]=
{
    NA_VALUE,   /*  SPEED_NA  */
    NA_VALUE,   /*  _1_25G    */
    NA_VALUE,   /*  _3_125G   */
    NA_VALUE,   /*  _3_33G    */
    NA_VALUE,   /*  _3_75G    */
    NA_VALUE,   /*  _4_25G    */
    NA_VALUE,   /*  _5G       */
    NA_VALUE,   /*  _6_25G    */
    NA_VALUE,   /*  _7_5G     */
    NA_VALUE,   /*  _10_3125G */
    NA_VALUE,   /*  _11_25G   */
    NA_VALUE,   /*  _11_5625G */
    NA_VALUE,   /*  _12_5G    */
    NA_VALUE,   /*  _10_9375G */
    NA_VALUE,   /*  _12_1875G */
    NA_VALUE,   /*  _5_625G   */
    NA_VALUE,   /*  _5_15625G */
    NA_VALUE,   /*  _12_8906G */
    NA_VALUE,   /*  _20_625G  */
    NA_VALUE,   /*  _25_78125G*/
    88,         /*  _27_5G    */
    NA_VALUE,   /*  _28_05G   */
    NA_VALUE,   /*  _26_5625G */
    NA_VALUE,   /*  _28_28125G*/
    NA_VALUE,   /* _26_5625G_PAM4  */
    NA_VALUE,   /* _28_125G_PAM4 */
    NA_VALUE,   /* _26_25G         */
    NA_VALUE,   /* _27_1G          */
    NA_VALUE,   /*  _10_3125G_SR_LR */
    NA_VALUE,   /*  _25_78125G_SR_LR*/
    NA_VALUE,   /*_26_5625G_PAM4_SR_LR */
    NA_VALUE,   /* _27_34375G */
    NA_VALUE,   /* _10G */
    NA_VALUE,   /* _2_578125 */
    NA_VALUE,   /* _1_25G_SR_LR */
    NA_VALUE,   /* _26_5625G_SR_LR */
    NA_VALUE    /* _27_1875_PAM4 */
};

/* Dividers DB for ref clock 312.5Mhz */
GT_U8 avagoBaudRate2DividerValue_164_24M[LAST_MV_HWS_SERDES_SPEED]=
{
    NA_VALUE,   /*  SPEED_NA  */
    NA_VALUE,   /*  _1_25G    */
    NA_VALUE,   /*  _3_125G   */
    NA_VALUE,   /*  _3_33G    */
    NA_VALUE,   /*  _3_75G    */
    NA_VALUE,   /*  _4_25G    */
    NA_VALUE,   /*  _5G       */
    NA_VALUE,   /*  _6_25G    */
    NA_VALUE,   /*  _7_5G     */
    NA_VALUE,   /*  _10_3125G */
    NA_VALUE,   /*  _11_25G   */
    NA_VALUE,   /*  _11_5625G */
    NA_VALUE,   /*  _12_5G    */
    NA_VALUE,   /*  _10_9375G */
    NA_VALUE,   /*  _12_1875G */
    NA_VALUE,   /*  _5_625G   */
    NA_VALUE,   /*  _5_15625G */
    NA_VALUE,   /*  _12_8906G */
    NA_VALUE,   /*  _20_625G  */
    NA_VALUE,   /*  _25_78125G*/
    NA_VALUE,   /*  _27_5G    */
    NA_VALUE,   /*  _28_05G   */
    NA_VALUE,   /*  _26_5625G */
    NA_VALUE,   /*  _28_28125G*/
    NA_VALUE,   /* _26_5625G_PAM4  */
    NA_VALUE,   /* _28_125G_PAM4 */
    NA_VALUE,   /* _26_25G         */
    165,        /* _27_1G          */
    NA_VALUE,   /* _10_3125G_SR_LR */
    NA_VALUE,   /* _25_78125G_SR_LR */
    NA_VALUE,   /*_26_5625G_PAM4_SR_LR */
    NA_VALUE,   /* _27_34375G */
    NA_VALUE,   /* _10G */
    NA_VALUE,   /* _2_578125 */
    NA_VALUE,   /* _1_25G_SR_LR */
    NA_VALUE,   /* _26_5625G_SR_LR */
    NA_VALUE    /* _27_1875_PAM4 */
};

GT_U8 avagoBaudRate2DwellTimeValue[LAST_MV_HWS_SERDES_SPEED]=
{
    NA_VALUE,   /*  SPEED_NA  */
    8,          /*  _1_25G    */
    8,          /*  _3_125G   */
    NA_VALUE,   /*  _3_33G    */
    8,          /*  _3_75G    */
    NA_VALUE,   /*  _4_25G    */
    8,          /*  _5G       */
    9,          /*  _6_25G    */
    10,         /*  _7_5G     */
    11,         /*  _10_3125G */
    11,         /*  _11_25G   */
    NA_VALUE,   /*  _11_5625G */
    12,         /*  _12_5G    */
    12,         /*  _10_9375G */
    12,         /*  _12_1875G */
    NA_VALUE,   /*  _5_625G   */
    12,         /*  _5_15625G */
    12,         /*  _12_8906G */
    NA_VALUE,   /*  _20_625G  */
    13,         /*  _25_78125G*/
    13,         /*  _27_5G    */
    NA_VALUE,   /*  _28_05G   */
    13,         /*  _26_5625G */
    13,         /*  _28_28125G*/
    13,         /*  _26_5625G_PAM4 */
    13,         /*  _28_125G_PAM4*/
    13,         /*  _26_25G   */
    13,         /*  _27_1G    */
    11,         /*  _10_3125G_SR_LR */
    13,         /*  _25_78125G_SR_LR */
    NA_VALUE,   /* _26_5625G_PAM4_SR_LR */
    13,         /* _27_34375G */
    NA_VALUE,   /* _10G */
    NA_VALUE,   /* _2_578125 */
    NA_VALUE,   /* _1_25G_SR_LR   */
    NA_VALUE,   /* _26_5625G_SR_LR */
    13          /* _27_1875_PAM4 */
};
extern Aapl_t* aaplSerdesDb[HWS_MAX_DEVICE_NUM];

#if defined(CHX_FAMILY) || defined(PX_FAMILY)
extern HWS_MUTEX avagoAccessMutex;
#endif /* defined(CHX_FAMILY) || defined(PX_FAMILY) */

extern GT_STATUS mvCfgSeqRunPolOp
(
    GT_U8            devNum,
    GT_U32           portGroup,
    GT_U32           index,
    MV_OP_PARAMS    *params
);
#endif /* ASIC_SIMULATION */

#ifndef ASIC_SIMULATION
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT)
#define AVAGO_TX(pre3,pre2,pre,atten,post) \
    (atten),(post),(pre),(pre2),(pre3)

#define AVAGO_RX_BASIC(lf,hf,bw,dc,gs1,gs2,ffeFix,shortChannel,bflf,bfhf) \
    (lf),(hf),(bw),(dc),(gs1),(gs2),(ffeFix),(shortChannel),(bflf),(bfhf)

#define AVAGO_RX_ADVANCED(minLf,maxLf,minHf,maxHf,minPre1,maxPre1,minPre2,maxPre2,minPost,maxPost,squelch) \
    (minLf),(maxLf),(minHf),(maxHf),(minPre1),(maxPre1),(minPre2),(maxPre2),(minPost),(maxPost),(squelch)

#define AVAGO_RX_MAIN(iCalEffort,pCalEffort,dfeDataRate,dfeCommon,int11d,int17d) \
    (iCalEffort),(pCalEffort),(dfeDataRate),(dfeCommon),(int11d),(int17d)

#define AVAGO_RX_MISC(termination,pre1PosGradient,pre1NegGradient,pre2PosGradient,pre2NegGradient,hfPosGradient,hfNegGradient,agcTargetLow,agcTargetHigh,coldEnvelope,hotEnvelop) \
    (termination),(pre1PosGradient),(pre1NegGradient),(pre2PosGradient),(pre2NegGradient),(hfPosGradient),(hfNegGradient),(agcTargetLow),(agcTargetHigh),(coldEnvelope),(hotEnvelop)

#define NA      NA_8BIT
#define NA_S    NA_8BIT_SIGNED
#define L_NA    NA_16BIT
#define L_NA_S  NA_16BIT_SIGNED


MV_HWS_AVAGO_16NM_TXRX_TUNE_PARAMS hwsAvagoSerdesTxRx16nmTuneParamsArray[] =
{
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3)  /* Non AN */ || defined (FALCON_DEV_SUPPORT)  /* Falcon Micro-Init */
{_1_25G,                   {AVAGO_TX(   0,   0,   0,   0,   0)},{AVAGO_RX_BASIC(   7,  15,  15,   0,   0,   1,  NA,  NA,   5,   1),AVAGO_RX_ADVANCED(  NA,  15,  NA,  NA,   0,   0,   0,   0,   8,   8, 148),AVAGO_RX_MAIN(     NA,     NA,      0,      0,   L_NA,   L_NA),AVAGO_RX_MISC(    NA,L_NA_S,L_NA_S,L_NA_S,L_NA_S,L_NA_S,L_NA_S,    NA,    NA,    NA,    NA)}},
{_1_25G_SR_LR,             {AVAGO_TX(   0,   0,   0,   0,   0)},{AVAGO_RX_BASIC(   8,   0,   3,   0,   0,   0,  NA,   0,   2,   8),AVAGO_RX_ADVANCED(   2,  12,   0,   6,   0,  15,   0,   0,  -3,  10, 100),AVAGO_RX_MAIN(     NA,     NA,     NA,     NA,   L_NA,   L_NA),AVAGO_RX_MISC(    NA,L_NA_S,L_NA_S,L_NA_S,L_NA_S,L_NA_S,L_NA_S,    NA,    NA,    NA,    NA)}},
{_3_125G,                  {AVAGO_TX(   0,   0,   0,   0,   0)},{AVAGO_RX_BASIC(   7,  15,  15,   0,   0,   1,  NA,  NA,   5,   1),AVAGO_RX_ADVANCED(  NA,  15,  NA,  NA,   0,   0,   0,   0,   8,   8, 148),AVAGO_RX_MAIN(     NA,     NA,      0,      0,   L_NA,   L_NA),AVAGO_RX_MISC(    NA,L_NA_S,L_NA_S,L_NA_S,L_NA_S,L_NA_S,L_NA_S,    NA,    NA,    NA,    NA)}},
{_5_15625G,                {AVAGO_TX(   0,   0,   4,   0,   0)},{AVAGO_RX_BASIC(  NA,  NA,  NA,   0,   3,   2, 0x0,   0,   5,   1),AVAGO_RX_ADVANCED(   0,  15,  10,  15,   0,   8,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,      0, 0x2F5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   120,    70)}},
{_10_3125G,                {AVAGO_TX(   0,   0,   0,   4,   0)},{AVAGO_RX_BASIC(  NA,  NA,  NA,   0,   3,   2,0x1F,   0,   5,   1),AVAGO_RX_ADVANCED(   0,  15,  10,  15,   0,   8,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   120,    70)}},
{_10_3125G_SR_LR,          {AVAGO_TX(   0,   0,   2,   2,  10)},{AVAGO_RX_BASIC(   8,   0,   3,   0,   0,   0,0x1F,   0,   2,   8),AVAGO_RX_ADVANCED(   2,  12,   0,   6,   0,  15,   0,   0,  -3,  10, 100),AVAGO_RX_MAIN(    0x0,      1,      0,      0,      0, 0x2F5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,    NA,    NA)}},
{_10_9375G,                {AVAGO_TX(   0,   0,   0,   4,   0)},{AVAGO_RX_BASIC(  NA,  NA,  NA,   0,   3,   2,0x1F,   0,   5,   1),AVAGO_RX_ADVANCED(   0,  15,  10,  15,   0,   8,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   120,    70)}},
{_12_1875G,                {AVAGO_TX(   0,   0,   0,   4,   0)},{AVAGO_RX_BASIC(  NA,  NA,  NA,   0,   3,   2,0x1F,   0,   5,   1),AVAGO_RX_ADVANCED(   0,  15,  10,  15,   0,   8,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   120,    70)}},
{_20_625G,                 {AVAGO_TX(   0,   0,   0,   0,   0)},{AVAGO_RX_BASIC(   3,  15,  15,   0,   1,   2,0x19,   0,   2,   8),AVAGO_RX_ADVANCED(   3,   7,   0,  15,   0,   5,   0,   0,  -3,  10,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,    125, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   160,    70)}},
{_25_78125G,               {AVAGO_TX(   0,   0,   0,   0,   0)},{AVAGO_RX_BASIC(   3,  15,  15,   0,   1,   2,0x19,   0,   2,   8),AVAGO_RX_ADVANCED(   3,   7,   0,  15,   0,   5,   0,   0,  -3,  10,  68),AVAGO_RX_MAIN(    0x0,      0,      0,      0,    125, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   160,    70)}},
{_25_78125G_SR_LR,         {AVAGO_TX(   0,   0,   2,   0,  10)},{AVAGO_RX_BASIC(   8,   2,   2,   0,   0,   0,0x19,   0,   2,   8),AVAGO_RX_ADVANCED(   2,  12,   0,   6,   0,  15,   0,   0,  -3,  10, 100),AVAGO_RX_MAIN(    0x0,      0,      0,      0,    125, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,    NA,    NA)}},
{_27_34375G,               {AVAGO_TX(   0,   0,   0,   0,   0)},{AVAGO_RX_BASIC(   3,  15,  15,   0,   1,   2,0x19,   0,   2,   8),AVAGO_RX_ADVANCED(   3,   7,   0,  15,   0,   5,   0,   0,  -3,  10,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,    125, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   160,    70)}},
{_26_5625G_PAM4,           {AVAGO_TX(   0,   0,   0,   0,   0)},{AVAGO_RX_BASIC(   8,   6,   6,   0,   2,   3,0x10,   0,   2,   8),AVAGO_RX_ADVANCED(   4,   8,   0,  15,   0,  15,  -8,   5,  -3,  10,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      3,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   500,  -500,   160,   160,   180,   100)}},
{_27_1875_PAM4,            {AVAGO_TX(   0,   0,   0,   0,   0)},{AVAGO_RX_BASIC(   8,   6,   6,   0,   2,   3,0x10,   0,   2,   8),AVAGO_RX_ADVANCED(   4,   8,   0,  15,   0,  15,  -8,   5,  -3,  10,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      3,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   500,  -500,   160,   160,   180,   100)}},
{_26_5625G_PAM4_SR_LR,     {AVAGO_TX(   0,   0,   4,   0,   4)},{AVAGO_RX_BASIC(  NA,  NA,  NA,   0,   0,   0,0x18,   1,   2,   8),AVAGO_RX_ADVANCED(   0,   8,   0,  15,   0,  15,  -8,  10,  -3,   8,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   800,  -800,   160,   160,    NA,    NA)}},
{_28_125G_PAM4,            {AVAGO_TX(   0,   0,   8,   0,   2)},{AVAGO_RX_BASIC(   8,   6,   6,   0,   2,   3,0x10,   0,   2,   8),AVAGO_RX_ADVANCED(   4,   8,   0,  15,   0,  15,  -8,   5,  -3,  10,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      3,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   500,  -500,   160,   160,   180,   100)}},
#else /* AN */
{_1_25G,                   {AVAGO_TX(   0,   0,   0,   0,   0)},{AVAGO_RX_BASIC(   7,  15,  15,   0,   0,   1,  NA,  NA,   5,   1),AVAGO_RX_ADVANCED(  NA,  15,  NA,  NA,   0,   0,   0,   0,   8,   8, 148),AVAGO_RX_MAIN(     NA,     NA,      0,      0,   L_NA,   L_NA),AVAGO_RX_MISC(    NA,L_NA_S,L_NA_S,L_NA_S,L_NA_S,L_NA_S,L_NA_S,    NA,    NA,    NA,    NA)}},
{_10_3125G,                {AVAGO_TX(   0,   0,   2,   4,  12)},{AVAGO_RX_BASIC(  NA,  NA,  NA,   0,   3,   2,0x1F,   0,   5,   1),AVAGO_RX_ADVANCED(   0,  15,  10,  15,   0,   8,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   120,    70)}},
{_10_9375G,                {AVAGO_TX(   0,   0,   2,   4,  12)},{AVAGO_RX_BASIC(  NA,  NA,  NA,   0,   3,   2,0x1F,   0,   5,   1),AVAGO_RX_ADVANCED(   0,  15,  10,  15,   0,   8,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   120,    70)}},
{_12_1875G,                {AVAGO_TX(   0,   0,   2,   4,  12)},{AVAGO_RX_BASIC(  NA,  NA,  NA,   0,   3,   2,0x1F,   0,   5,   1),AVAGO_RX_ADVANCED(   0,  15,  10,  15,   0,   8,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   120,    70)}},
{_20_625G,                 {AVAGO_TX(   0,   0,   0,   0,   0)},{AVAGO_RX_BASIC(   3,  15,  15,   0,   1,   2,0x19,   0,   2,   8),AVAGO_RX_ADVANCED(   3,   7,   0,  15,   0,   5,   0,   0,  -3,  10,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,    125, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   160,    70)}},
{_25_78125G,               {AVAGO_TX(   0,   0,   2,   0,  14)},{AVAGO_RX_BASIC(   3,   6,   6,   0,   1,   2,0x19,   0,   2,   8),AVAGO_RX_ADVANCED(   3,   7,   0,  15,   0,   5,   0,   0,  -3,  10,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,    125, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   160,    70)}},
{_27_34375G,               {AVAGO_TX(   0,   0,   2,   0,  14)},{AVAGO_RX_BASIC(   3,  15,  15,   0,   1,   2,0x19,   0,   2,   8),AVAGO_RX_ADVANCED(   3,   7,   0,  15,   0,   5,   0,   0,  -3,  10,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,    125, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   140,   140,   160,    70)}},
{_26_5625G_PAM4,           {AVAGO_TX(   0,   0,   6,   0,   6)},{AVAGO_RX_BASIC(   8,   6,   6,   0,   2,   3,0x10,   0,   2,   8),AVAGO_RX_ADVANCED(   4,   8,   0,  15,   0,  15,  -8,   5,  -3,  10,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      3,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   500,  -500,   160,   160,   180,   100)}},
{_27_1875_PAM4,            {AVAGO_TX(   0,   0,   6,   0,   6)},{AVAGO_RX_BASIC(   8,   6,   6,   0,   2,   3,0x10,   0,   2,   8),AVAGO_RX_ADVANCED(   4,   8,   0,  15,   0,  15,  -8,   5,  -3,  10,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      3,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   500,  -500,   160,   160,   180,   100)}},
{_28_125G_PAM4,            {AVAGO_TX(   0,   0,   6,   0,   6)},{AVAGO_RX_BASIC(   8,   6,   6,   0,   2,   3,0x10,   0,   2,   8),AVAGO_RX_ADVANCED(   4,   8,   0,  15,   0,  15,  -8,   5,  -3,  10,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      3,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   500,  -500,   160,   160,   180,   100)}},
#ifdef RAVEN_DEV_SUPPORT
{_10_3125G_SR_LR,          {AVAGO_TX(   0,   0,   2,   2,  10)},{AVAGO_RX_BASIC(   8,   0,   3,   0,   0,   0,0x1F,   0,   2,   8),AVAGO_RX_ADVANCED(   2,  12,   0,   6,   0,  15,   0,   0,  -3,  10, 100),AVAGO_RX_MAIN(    0x0,      1,      0,      0,      0, 0x2F5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,    NA,    NA)}},
{_25_78125G_SR_LR,         {AVAGO_TX(   0,   0,   2,   0,  10)},{AVAGO_RX_BASIC(   8,   2,   2,   0,   0,   0,0x19,   0,   2,   8),AVAGO_RX_ADVANCED(   2,  12,   0,   6,   0,  15,   0,   0,  -3,  10, 100),AVAGO_RX_MAIN(    0x0,      0,      0,      0,    125, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,    NA,    NA)}},
{_26_5625G_PAM4_SR_LR,     {AVAGO_TX(   0,   0,   4,   0,   4)},{AVAGO_RX_BASIC(  NA,  NA,  NA,   0,   0,   0,0x18,   0,   2,   8),AVAGO_RX_ADVANCED(   0,   8,   0,  15,   0,  15,  -8,  10,  -3,   8,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   800,  -800,   160,   160,    NA,    NA)}},
#endif
#endif
};

MV_HWS_AVAGO_16NM_TXRX_TUNE_PARAMS hwsAvagoSerdesTxRx16nmTuneParamsArrayProfile1[] =
{
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3)  /* Non AN */ || defined (FALCON_DEV_SUPPORT)  /* Falcon Micro-Init */
{_10_3125G,                {AVAGO_TX(   0,   0,   0,   4,   0)},{AVAGO_RX_BASIC(   4,  13,  13,   0,   3,   2,0x1F,   0,   5,   1),AVAGO_RX_ADVANCED(   2,   7,  10,  15,   0,   8,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   120,    70)}},
{_10_9375G,                {AVAGO_TX(   0,   0,   0,   4,   0)},{AVAGO_RX_BASIC(   4,  13,  13,   0,   3,   2,0x1F,   0,   5,   1),AVAGO_RX_ADVANCED(   2,   7,  10,  15,   0,   8,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   120,    70)}},
{_12_1875G,                {AVAGO_TX(   0,   0,   0,   4,   0)},{AVAGO_RX_BASIC(   4,  13,  13,   0,   3,   2,0x1F,   0,   5,   1),AVAGO_RX_ADVANCED(   2,   7,  10,  15,   0,   8,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   120,    70)}},
{_20_625G,                 {AVAGO_TX(   0,   0,   0,   0,   0)},{AVAGO_RX_BASIC(   8,   2,   2,   0,   1,   2,0x19,   0,   2,   8),AVAGO_RX_ADVANCED(   2,   9,   2,  12,   0,  15,   0,   0,  -3,  10,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,    100, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   160,    70)}},
{_25_78125G,               {AVAGO_TX(   0,   0,   0,   0,   0)},{AVAGO_RX_BASIC(   8,   2,   2,   0,   1,   2,0x19,   0,   2,   8),AVAGO_RX_ADVANCED(   2,   9,   2,  12,   0,  15,   0,   0,  -3,  10,  68),AVAGO_RX_MAIN(    0x0,      0,      0,      0,    100, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   160,    70)}},
{_27_34375G,               {AVAGO_TX(   0,   0,   0,   0,   0)},{AVAGO_RX_BASIC(   8,   2,   2,   0,   1,   2,0x19,   0,   2,   8),AVAGO_RX_ADVANCED(   2,   9,   2,  12,   0,  15,   0,   0,  -3,  10,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,    100, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   120,    70)}},
{_26_5625G_PAM4,           {AVAGO_TX(   0,   0,   2,   0,   0)},{AVAGO_RX_BASIC(   8,   0,   0,   0,   2,   3,0x18,   0,   2,   8),AVAGO_RX_ADVANCED(   4,  12,   0,  11,   4,  15,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   800,  -800,   160,   160,   120,    40)}},
{_27_1875_PAM4,            {AVAGO_TX(   0,   0,   2,   0,   0)},{AVAGO_RX_BASIC(   8,   0,   0,   0,   2,   3,0x18,   0,   2,   8),AVAGO_RX_ADVANCED(   4,  12,   0,  11,   4,  15,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   800,  -800,   160,   160,   120,    40)}},
{_28_125G_PAM4,            {AVAGO_TX(   0,   0,   2,   0,   0)},{AVAGO_RX_BASIC(   8,   0,   0,   0,   2,   3,0x18,   0,   2,   8),AVAGO_RX_ADVANCED(   4,  12,   0,  11,   4,  15,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   800,  -800,   160,   160,   120,    40)}},
#else /* AN */
{_10_3125G,                {AVAGO_TX(   0,   0,   2,   4,  12)},{AVAGO_RX_BASIC(   4,  13,  13,   0,   3,   2,0x1F,   0,   5,   1),AVAGO_RX_ADVANCED(   2,   7,  10,  15,   0,   8,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   120,    70)}},
{_10_9375G,                {AVAGO_TX(   0,   0,   2,   4,  12)},{AVAGO_RX_BASIC(   4,  13,  13,   0,   3,   2,0x1F,   0,   5,   1),AVAGO_RX_ADVANCED(   2,   7,  10,  15,   0,   8,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   120,    70)}},
{_12_1875G,                {AVAGO_TX(   0,   0,   2,   4,  12)},{AVAGO_RX_BASIC(   4,  13,  13,   0,   3,   2,0x1F,   0,   5,   1),AVAGO_RX_ADVANCED(   2,   7,  10,  15,   0,   8,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   120,    70)}},
{_20_625G,                 {AVAGO_TX(   0,   0,   0,   0,   0)},{AVAGO_RX_BASIC(   8,   2,   2,   0,   1,   2,0x19,   0,   2,   8),AVAGO_RX_ADVANCED(   2,   9,   2,  12,   0,  15,   0,   0,  -3,  10,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,    100, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   160,    70)}},
{_25_78125G,               {AVAGO_TX(   0,   0,   2,   0,  14)},{AVAGO_RX_BASIC(   8,   2,   2,   0,   1,   2,0x19,   0,   2,   8),AVAGO_RX_ADVANCED(   2,   9,   2,  12,   0,  15,   0,   0,  -3,  10,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,    100, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   160,    70)}},
{_27_34375G,               {AVAGO_TX(   0,   0,   2,   0,  14)},{AVAGO_RX_BASIC(   8,   2,   2,   0,   1,   2,0x19,   0,   2,   8),AVAGO_RX_ADVANCED(   2,   9,   2,  12,   0,  15,   0,   0,  -3,  10,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,    100, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   130,   130,   120,    70)}},
{_26_5625G_PAM4,           {AVAGO_TX(   0,   0,   6,   0,   6)},{AVAGO_RX_BASIC(   8,   0,   0,   0,   2,   3,0x18,   0,   2,   8),AVAGO_RX_ADVANCED(   4,  12,   0,  11,   4,  15,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   800,  -800,   160,   160,   120,    40)}},
{_27_1875_PAM4,            {AVAGO_TX(   0,   0,   6,   0,   6)},{AVAGO_RX_BASIC(   8,   0,   0,   0,   2,   3,0x18,   0,   2,   8),AVAGO_RX_ADVANCED(   4,  12,   0,  11,   4,  15,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   800,  -800,   160,   160,   120,    40)}},
{_28_125G_PAM4,            {AVAGO_TX(   0,   0,   6,   0,   6)},{AVAGO_RX_BASIC(   8,   0,   0,   0,   2,   3,0x18,   0,   2,   8),AVAGO_RX_ADVANCED(   4,  12,   0,  11,   4,  15,  -5,   5,  -3,   8,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      0,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   800,  -800,   160,   160,   120,    40)}},
#endif
};

MV_HWS_AVAGO_16NM_TXRX_TUNE_PARAMS hwsAvagoSerdesTxRx16nmTuneParamsArrayProfile2[] =
{
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3)  /* Non AN */ || defined (FALCON_DEV_SUPPORT)  /* Falcon Micro-Init */
{_27_34375G,               {AVAGO_TX(   0,   0,   0,   0,   0)},{AVAGO_RX_BASIC(   6,  15,  15,   0,   1,   2,0x19,   0,   2,   8),AVAGO_RX_ADVANCED(   0,   7,   0,  15,   0,  15,   0,   0,  -3,  10,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,    150, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   150,    50)}},
{_26_5625G_PAM4,           {AVAGO_TX(   0,   0,   6,   0,   2)},{AVAGO_RX_BASIC(   8,   6,   6,   0,   2,   3,0x10,   0,   2,   8),AVAGO_RX_ADVANCED(   4,   8,   0,  15,   0,  15,  -5,   5,  -3,  10,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      3,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   500,  -500,   160,   160,   180,   100)}},
{_27_1875_PAM4,            {AVAGO_TX(   0,   0,   6,   0,   2)},{AVAGO_RX_BASIC(   8,   6,   6,   0,   2,   3,0x10,   0,   2,   8),AVAGO_RX_ADVANCED(   4,   8,   0,  15,   0,  15,  -5,   5,  -3,  10,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      3,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   500,  -500,   160,   160,   180,   100)}},
#else /* AN */
{_25_78125G,               {AVAGO_TX(   0,   0,   2,   0,  14)},{AVAGO_RX_BASIC(   6,  15,  15,   0,   1,   2,0x19,   0,   2,   8),AVAGO_RX_ADVANCED(   0,   7,   0,  15,   0,  15,   0,   0,  -3,  10,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,    125, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   150,    50)}},
{_27_34375G,               {AVAGO_TX(   0,   0,   2,   0,  14)},{AVAGO_RX_BASIC(   6,  15,  15,   0,   1,   2,0x19,   0,   2,   8),AVAGO_RX_ADVANCED(   0,   7,   0,  15,   0,  15,   0,   0,  -3,  10,  68),AVAGO_RX_MAIN(    0x0,      1,      0,      0,    125, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,L_NA_S,L_NA_S,  1200, -1200,   160,   160,   150,    50)}},
{_26_5625G_PAM4,           {AVAGO_TX(   0,   0,   6,   0,   6)},{AVAGO_RX_BASIC(   8,   6,   6,   0,   2,   3,0x10,   0,   2,   8),AVAGO_RX_ADVANCED(   4,   8,   0,  15,   0,  15,  -5,   5,  -3,  10,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      3,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   500,  -500,   160,   160,   180,   100)}},
{_27_1875_PAM4,            {AVAGO_TX(   0,   0,   6,   0,   6)},{AVAGO_RX_BASIC(   8,   6,   6,   0,   2,   3,0x10,   0,   2,   8),AVAGO_RX_ADVANCED(   4,   8,   0,  15,   0,  15,  -5,   5,  -3,  10,  68),AVAGO_RX_MAIN(   0x10,      1,      0,      3,      0, 0x2B5C),AVAGO_RX_MISC(     2,  1000, -1000,  1000, -1000,   500,  -500,   160,   160,   180,   100)}},
#endif
};

MV_HWS_AVAGO_16NM_TXRX_TUNE_PARAMS *hwsAvagoSerdesTxRx16nmTuneParamsArrayPtrs[HWS_AVAGO_16NM_TXRX_PARAMS_NUM_OF_PROFILES] =
{
    hwsAvagoSerdesTxRx16nmTuneParamsArray,
    hwsAvagoSerdesTxRx16nmTuneParamsArrayProfile1,
    hwsAvagoSerdesTxRx16nmTuneParamsArrayProfile2
};

GT_U8 hwsAvagoSerdesTxRx16nmTuneParamsArraySizes[HWS_AVAGO_16NM_TXRX_PARAMS_NUM_OF_PROFILES] =
{
    ARR_SIZE_CALC_MAC(hwsAvagoSerdesTxRx16nmTuneParamsArray),
    ARR_SIZE_CALC_MAC(hwsAvagoSerdesTxRx16nmTuneParamsArrayProfile1),
    ARR_SIZE_CALC_MAC(hwsAvagoSerdesTxRx16nmTuneParamsArrayProfile2)
};
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT)*/

#ifndef RAVEN_DEV_SUPPORT
MV_HWS_AVAGO_TXRX_TUNE_PARAMS hwsAvagoSerdesTxRxTuneParamsArray[] =
{   /* SD speed         atten  post  pre  pre2 pre3  DC     LF  HF  BW   SQ   GS1  GS2  shChEn dfeGain dfeGain2*/
    {_1_25G,            {19,    0,    0,   0,   0}, {100,   15, 15, 13,   150, 0,   0, GT_FALSE, 0,      0}}
   ,{_3_125G,           {16,    0,    0,   0,   0}, {100,   15, 15, 13,   150, 0,   0, GT_FALSE, 0,      0}}
   ,{_5G,               {12,    0,    0,   0,   0}, {80 ,   6 , 15, 13,   150, 0,   0, GT_FALSE, 0,      0}}
   ,{_6_25G,            {12,    0,    0,   0,   0}, {110,   4 , 15, 13,   150, 0,   0, GT_FALSE, 0,      0}}
   ,{_7_5G,             {12,    0,    0,   0,   0}, {110,   4 , 15, 13,   150, 0,   0, GT_FALSE, 0,      0}}
#ifndef MV_HWS_FREE_RTOS /* non-AP/Host CPU*/
   ,{_10_3125G,         {4,     0,    0,   0,   0}, {100,   3 , 15, 13,   68 , 0,   0, GT_FALSE, 0,      0}} /* use copper/non-AP values (10G-KR/CR) */
#else
   ,{_10_3125G,         {2,    18,    4,   0,   0}, {100,   3 , 15, 13,   68 , 0,   0, GT_FALSE, 0,      0}} /* for AP TX values must be same as init */
#endif
   ,{_12_1875G,         {4,     0,    0,   0,   0}, {100,   3 , 15, 13,   68 , 0,   0, GT_FALSE, 0,      0}}
   ,{_5_15625G,         {12,    0,    0,   0,   0}, {80 ,   6 , 15, 13,   150, 0,   0, GT_FALSE, 0,      0}}
   ,{_12_8906G,         {4,     0,    0,   0,   0}, {100,   3 , 15, 13,   68 , 0,   0, GT_FALSE, 0,      0}}
   ,{_20_625G,          {4,     0,    0,   0,   0}, {110,   6 , 9 , 9 ,   68 , 0,   0, GT_FALSE, 0,      0}}
   ,{_25_78125G,        {1,     0,    6,   0,   0}, {110,   6 , 9 , 9 ,   68 , 0,   0, GT_FALSE, 0,      0}} /* for AP TX values must be same as init */
   ,{_27_1G,            {1,     0,    6,   0,   0}, {110,   6 , 9 , 9 ,   84 , 0,   0, GT_FALSE, 0,      0}}
   ,{_27_5G,            {1,     0,    6,   0,   0}, {110,   6 , 9 , 9 ,   84 , 0,   0, GT_FALSE, 0,      0}}
   ,{_26_25G,           {1,     0,    6,   0,   0}, {110,   6 , 9 , 9 ,   84 , 0,   0, GT_FALSE, 0,      0}}
   ,{_10_3125G_SR_LR,   {9,     10,   0,   0,   0}, {100,   7 , 15, 13,   100, 0,   0, GT_FALSE, 0,      0}} /* 10G optics values */
   ,{_25_78125G_SR_LR,  {4,     9,    2,   0,   0}, {110,   6,  9 , 9 ,   84 , 0,   0, GT_FALSE, 0,      0}} /* 25G optics values */
   ,{_26_5625G,         {0,     4,    0,   0,   0}, {0,     8,  0 , 0 ,   68,  0,   0, GT_FALSE, 0,      0}}
};
GT_U8 hwsAvagoSerdesTxRxTuneParamsArrayGetSize(void)
{
    return sizeof(hwsAvagoSerdesTxRxTuneParamsArray)/sizeof(hwsAvagoSerdesTxRxTuneParamsArray[0]);
}
#endif /* RAVEN_DEV_SUPPORT */
#endif /* ASIC_SIMULATION */
/* To configure regular port in Micro-init on Service CPU */
#ifdef MICRO_INIT
MV_HWS_AVAGO_TXRX_TUNE_PARAMS miAvagoSerdesTxRxTuneParamsArray[] =
{
   {_3_125G,   {16,    0,      0}, {100,   15, 15, 13, 150}}
  ,{_10_3125G, {4,     0,      0}, {100,   3 , 15, 13, 68 }} /* use copper/non-AP values (10G-KR/CR), for 10G-SR/10G-LR mode values use Tx/Rx manual API's */
};
#endif

/*
    For PMD Training in AP: TX values must be same as init values that written
    in hwsAvagoSerdesTxRxTuneParamsArray (for Int 0x15)
*/
#ifndef RAVEN_DEV_SUPPORT
                                                                       /* txAmp  emph0  emph1 */
MV_HWS_TX_TUNE_PARAMS hwsAvagoSerdesTxTuneInitParams[2] = {/* _10_3125G  */{2,    18,     4},
                                                           /* _25_78125G */{1,     0,     6}};

MV_HWS_TX_TUNE_PARAMS hwsAvagoSerdesTxTunePresetParams[2] = {/* _10_3125G  */{0,    0,     0},
                                                             /* _25_78125G */{0,    0,     0}};
#else

MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA hwsAvagoSerdesTxTune16nmPresetParams[3][AN_PRESETS_NUM] = /* Spico int 0x3D */
{                      /* Preset1[CL136] / Preset[CL72]          Preset2[CL136] / Initialize[CL72]     Preset3[CL136] */
                       /*  atten  post   pre    pre2   pre3      atten  post   pre    pre2   pre3      atten  post   pre    pre2   pre3 */
/*_10_3125G            */{{4     ,0     ,0     ,TX_NA ,TX_NA }, {4     ,12    ,2     ,TX_NA ,TX_NA }, {TX_NA ,TX_NA ,TX_NA ,TX_NA ,TX_NA ,}},
/*_25_78125G           */{{0     ,0     ,0     ,TX_NA ,TX_NA }, {0     ,14    ,2     ,TX_NA ,TX_NA }, {TX_NA ,TX_NA ,TX_NA ,TX_NA ,TX_NA ,}},
/*_26_5625G_PAM4       */{{0     ,0     ,0     ,0     ,TX_NA }, {0     ,6     ,6     ,0     ,TX_NA }, {0     ,0     ,10    ,0     ,TX_NA ,}}
};
#endif

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(FALCON_DEV_SUPPORT) /* MI needs it */
MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA hwsAvagoSerdesTxTune16nmPreset1ParamsProfiles[4][HWS_AVAGO_16NM_TXRX_PARAMS_NUM_OF_PROFILES] =
{                      /* Profile 0                              Profile1                             Profile 2                          */
                       /*  atten  post   pre    pre2   pre3      atten  post   pre    pre2   pre3      atten  post   pre    pre2   pre3  */
/*_10_3125G            */{{4     ,0     ,0     ,TX_NA ,TX_NA }, {4     ,0     ,0     ,TX_NA ,TX_NA }, {4     ,0     ,0     ,TX_NA ,TX_NA }},
/*_25_78125G           */{{0     ,0     ,0     ,TX_NA ,TX_NA }, {0     ,0     ,0     ,TX_NA ,TX_NA }, {0     ,0     ,0     ,TX_NA ,TX_NA }},
/*_26_5625G_PAM4       */{{0     ,0     ,0     ,0     ,TX_NA }, {0     ,0     ,0     ,0     ,TX_NA }, {0     ,2     ,6     ,0     ,TX_NA }},
/*_28_125G_PAM4        */{{0     ,0     ,8     ,0     ,TX_NA }, {0     ,0     ,2     ,0     ,TX_NA }, {0     ,0     ,0     ,0     ,TX_NA }}
};
#endif

/* ------------ end of generic Avago serdes init values ------------- */

/************************* Declaration ***************************************************/
#ifndef CO_CPU_RUN
/*******************************************************************************
* mvHwsAvagoSerdesTypeGetFunc
*
* DESCRIPTION:
*       Return "AVAGO" string
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Return "AVAGO" string
*
*******************************************************************************/
#ifndef RAVEN_DEV_SUPPORT
static char* mvHwsAvagoSerdesTypeGetFunc(void)
{
  return "AVAGO";
}
#endif

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT)
static char* mvHwsAvago16nmSerdesTypeGetFunc(void)
{
  return "AVAGO_16NM";
}
#endif
#endif /* CO_CPU_RUN */

/**********************************************************/
/**** Our private log handling declarations glue START ****/
/**********************************************************/
/* Message bufer size to pass to hws. This size can be evaluated independatly
   from HWS log handling, as in HWS log handle the buffer will cut if needed. */
#ifndef  MV_HWS_FREE_RTOS
#define HWS_STRING_BUF_SIZE     AAPL_LOG_BUF_SIZE
#else
#define HWS_STRING_BUF_SIZE     40
#endif

GT_BOOL aaplEnableLog = GT_FALSE;

/**
* @internal mvHwsAvagoApplLogUser function
* @endinternal
*
* @brief   AVAGO log glud function. This function will build a
*          perliminary log message and send it to HWS log
*          handler.
*
* @param[in] caller - function name, or 0, depends on Avago code
* @param[in] line   - line number, or 0, depends on Avago code
* @param[in] fmt    - format string
* @param[in] ap     - arg list
* @param[in]log_sel - log level
*
* @retval 0 - on success
* @retval 1 - on error
*
* @note  1. When running as FW, message strings will not
*       be evaluated with the additional arguments,
*       i.e, ("msg %d", msgIntegerParam) will be evaluated to
*       "msg" %d" and not "msg <msgIntegerParamas>" the lack of
*       the memory in FW arcitecture, and the fact that Avago
*       code build it's own log messages.
*/
#ifndef ASIC_SIMULATION
#ifndef  MV_HWS_FREE_RTOS
/* In host side we have enought space to log all log arguments */
static void mvHwsAvagoApplLogUser
(
    const char *caller,         /**< [in] Caller's __func__ value, or 0. */
    char *fileName,             /**< [in] Caller's file-name. */
    int line,                   /**< [in] Caller's __LINE__ value, or 0. */
    const char *fmt,            /**< [in] printf format string. */
    va_list ap,                 /** arg list */
    Aapl_log_type_t log_sel     /**< /[in] Type of message logging. */
)
#else
#if (defined(MICRO_INIT) && defined(CM3))
/* In MICRO-INIT CM3 FW side we also do not contain the message argument, only function name */
static void mvHwsAvagoApplLogUser
(
    const char *caller         /**< [in] Caller's __func__ value, or 0. */
)
#else
/* In FW side we do not contain the extra arguments in the log message */
static void mvHwsAvagoApplLogUser
(
    const char *caller,         /**< [in] Caller's __func__ value, or 0. */
    const char *fmt,            /**< [in] printf format string. */
    Aapl_log_type_t log_sel     /**< [in] Type of message logging. */
)
#endif /* (defined(MICRO_INIT) && defined(CM3)) */
#endif /* MV_HWS_FREE_RTOS */
{
    if (aapl_user_supplied_log_func_ptr != NULL)
    {
#ifndef  MV_HWS_FREE_RTOS
        char buf[HWS_STRING_BUF_SIZE]={'\0'};
#else
        char buf[HWS_STRING_BUF_SIZE]={'\0'};
        int fmtSize=0;
#endif

        /* Initializing buffer */
        AAPL_MEMSET(buf, '\0', sizeof(buf));

/* In non-FreeRTOS environment we have enougth memory to perform parameters format parsing including size limit */
#ifndef  MV_HWS_FREE_RTOS
        hwsVsnprintf(buf, (HWS_STRING_BUF_SIZE-1), fmt, ap);
        aapl_user_supplied_log_func_ptr(
                caller /* function name */,
                fileName,
                line /* line number */,
                (log_sel==AVAGO_ERR) ? 1 : 0, /* return code */
                buf /* string argument */);

/* In FreeRTOS environment we have memory constraint so only the most important information will be logged,
   which are function name and the format string */
#else
/* In FW side we log only error messages, no space for unimportant messages */
#if !(defined(MICRO_INIT) && defined(CM3))
        if (log_sel == AVAGO_ERR)
#endif /* MICRO_INIT */
        {
#if !(defined(MICRO_INIT) && defined(CM3))
            fmtSize = osStrlen(fmt);
            hwsMemCpy(&buf[0], &fmt[0], (fmtSize > HWS_STRING_BUF_SIZE) ? HWS_STRING_BUF_SIZE : fmtSize);
#else
            /* Copying the format message as much as we can */
            fmtSize = 8;
            hwsMemCpy(&buf[0], "fail msg", (fmtSize > HWS_STRING_BUF_SIZE) ? HWS_STRING_BUF_SIZE : fmtSize);
#endif /* !(defined(MICRO_INIT) && defined(CM3)) */

            aapl_user_supplied_log_func_ptr(
                        caller /* function name */,
                        buf    /* string argument */);
        }
#endif /* MV_HWS_FREE_RTOS */
    }
}

/**
* @internal mvHws_aapl_fail function
* @endinternal
*
* @brief   Prints the specified message and marks an error
*
* @param[in] aapl   - AAPL structure pointer
* @param[in] caller - function name, or 0, depends on Avago code
* @param[in] fileName - Caller's file-name
* @param[in] line     - line number, or 0, depends on Avago code
* @param[in] fmt      - format string
*
* @return mvHws_aapl_fail always returns -1 to indicate an
*         error.
*/
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
int mvHws_aapl_fail
(
    Aapl_t *aapl,           /**< AAPL structure pointer. */
    const char *caller,     /**< Caller function, usually __func__ */
    char *fileName,         /**< [in] Caller's file-name. */
    int line,               /**< Caller line number, usually __LINE__ */
    const char *fmt,        /**< Format control string (printf format) */
    ...                     /**< Format string parameters. */
)
#else
#if (defined(MICRO_INIT) && defined(CM3))
int mvHws_aapl_fail
(
    Aapl_t *aapl,           /**< AAPL structure pointer. */
    const char *caller      /**< Caller function, usually __func__ */
)
#else
int mvHws_aapl_fail
(
    Aapl_t *aapl,           /**< AAPL structure pointer. */
    const char *caller,     /**< Caller function, usually __func__ */
    const char *fmt         /**< Format control string (printf format) */
)
#endif /* MICRO_INIT */
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */
{

#ifndef  MV_HWS_FREE_RTOS
    va_list ap;
    va_start(ap, fmt);
#endif

/* Calling our user defined log */
#ifndef  MV_HWS_FREE_RTOS
    mvHwsAvagoApplLogUser(caller, fileName, line, fmt, ap, AVAGO_ERR);
#else
    #if (defined(MICRO_INIT) && defined(CM3))
        mvHwsAvagoApplLogUser(caller);
    #else
        mvHwsAvagoApplLogUser(caller, fmt, AVAGO_ERR);
    #endif /* (defined(MICRO_INIT) && defined(CM3)) */
#endif /* MV_HWS_FREE_RTOS */

#ifndef  MV_HWS_FREE_RTOS
    va_end(ap);
#endif

    if( aapl->return_code > 0 )
        aapl->return_code = 0;

    aapl->return_code -= 1;

    return -1;
}

#if !(defined(MICRO_INIT) && defined(CM3))
static int mvHwsCheckDebugLevel(
    Aapl_t *aapl,
    Aapl_log_type_t log_sel)
{
    if( (int) log_sel > (int) (aapl->debug & 0xf) && log_sel < AVAGO_MEM_LOG )
        return 1;
    return 0;
}

/**
* @internal mvHws_aapl_log_printf function
* @endinternal
*
* @brief  Prints message to the specified log.
*
* @details If log_sel is one of the AVAGO_DEBUGn values,
*  logging only occurs if the aapl->debug field is greater than
*  or equal to the log_sel value.
*
* @return void
*/
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
void mvHws_aapl_log_printf
(
    Aapl_t *aapl,               /**< [in] Pointer to Aapl_t structure. */
    Aapl_log_type_t log_sel,    /**< [in] Type of message logging. */
    const char *caller,         /**< [in] Caller's __func__ value, or 0. */
    char *fileName,             /**< [in] Caller's file-name. */
    int line,                   /**< [in] Caller's __LINE__ value, or 0. */
    const char *fmt,            /**< [in] printf format string. */
    ...                         /**< [in] printf arguments. */
)
#else
void mvHws_aapl_log_printf
(
    Aapl_t *aapl,               /**< [in] Pointer to Aapl_t structure. */
    Aapl_log_type_t log_sel,    /**< [in] Type of message logging. */
    const char *caller,         /**< [in] Caller's __func__ value, or 0. */
    const char *fmt             /**< [in] printf format string. */
)
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */
{
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    va_list ap;
#endif
    if(mvHwsCheckDebugLevel(aapl, log_sel))
        return;

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    va_start(ap, fmt);
#endif

    /* Calling our user defined log */
    #ifndef  MV_HWS_FREE_RTOS
        mvHwsAvagoApplLogUser(caller, fileName, line, fmt, ap, log_sel);
    #else
        mvHwsAvagoApplLogUser(caller, fmt, log_sel);
    #endif /* MV_HWS_FREE_RTOS */

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    va_end(ap);
#endif
}
#endif /* !(defined(MICRO_INIT) && defined(CM3)) */
#endif /* ASIC_SIMULATION */
#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsAvagoIfInit function
* @endinternal
*
* @brief   Init Avago Serdes IF functions.
*
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - array for function registration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoIfInit
(
    IN GT_U8  devNum,
    IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray
)
{
#ifdef ASIC_SIMULATION
    /* avoid warning */
    GT_UNUSED_PARAM(devNum);
#else
    GT_STATUS res;

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if(hwsAvagoNotSupportedCheck())
    {
        /* the Avago driver MUST not be accessed */
        res = GT_OK;
    }
    else
#endif /*! MV_HWS_REDUCED_BUILD_EXT_CM3*/
    {
        /* for 28nm chipIndex equals to devNum */
        res = mvHwsAvagoSerdesInit(devNum, devNum);
    }

    if(res == GT_ALREADY_EXIST)
    {
        /* Avago Serdes for this device was already initialized */
        return GT_OK;
    }
    else if(res != GT_OK)
    {
        return res;
    }
#endif

    if(hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesNumber > MAX_AVAGO_SERDES_NUMBER)
    {
        HWS_AVAGO_DBG(("Size of serdesArr_copy(%d) is not enough for serdes number = %d\n", MAX_AVAGO_SERDES_NUMBER, hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesNumber));
        return GT_BAD_STATE;
    }
    if(!funcPtrArray[AVAGO])
    {
        funcPtrArray[AVAGO] = (MV_HWS_SERDES_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_SERDES_FUNC_PTRS));
        if(!funcPtrArray[AVAGO])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[AVAGO], 0, sizeof(MV_HWS_SERDES_FUNC_PTRS));
    }
    /* Avago implementation */
    funcPtrArray[AVAGO]->serdesArrayPowerCntrlFunc    = BIND_SET_FUNC(mvHwsAvagoSerdesArrayPowerCtrl      );
    funcPtrArray[AVAGO]->serdesPowerCntrlFunc         = BIND_SET_FUNC(mvHwsAvagoSerdesPowerCtrl           );
    funcPtrArray[AVAGO]->serdesManualTxCfgFunc        = BIND_SET_FUNC(mvHwsAvagoSerdesManualTxConfig      );
    funcPtrArray[AVAGO]->serdesManualRxCfgFunc        = BIND_SET_FUNC(mvHwsAvagoSerdesManualCtleConfig    );
    funcPtrArray[AVAGO]->serdesRxAutoTuneStartFunc    = BIND_SET_FUNC(mvHwsAvagoSerdesRxAutoTuneStart     );
    funcPtrArray[AVAGO]->serdesTxAutoTuneStartFunc    = BIND_SET_FUNC(mvHwsAvagoSerdesTxAutoTuneStart     );
    funcPtrArray[AVAGO]->serdesAutoTuneStartFunc      = BIND_SET_FUNC(mvHwsAvagoSerdesAutoTuneStart       );
    funcPtrArray[AVAGO]->serdesAutoTuneResultFunc     = BIND_SET_FUNC(mvHwsAvagoSerdesAutoTuneResult      );
    funcPtrArray[AVAGO]->serdesAutoTuneStatusFunc     = BIND_SET_FUNC(mvHwsAvagoSerdesAutoTuneStatus      );
    funcPtrArray[AVAGO]->serdesResetFunc              = BIND_SET_FUNC(mvHwsAvagoSerdesReset               );
#ifndef CO_CPU_RUN
    funcPtrArray[AVAGO]->serdesDfeCfgFunc             = BIND_SET_FUNC(mvHwsAvagoSerdesDfeConfig           );
#ifndef RAVEN_DEV_SUPPORT
    funcPtrArray[AVAGO]->serdesDigitalReset           = BIND_SET_FUNC(mvHwsAvagoSerdesDigitalReset        );
    funcPtrArray[AVAGO]->serdesAutoTuneStatusShortFunc =BIND_SET_FUNC(mvHwsAvagoSerdesAutoTuneStatusShort);
    funcPtrArray[AVAGO]->serdesArrayHaltDfeTuneFunc   = BIND_SET_FUNC(mvHwsAvagoSerdesArrayHaltDfeTune    );
    funcPtrArray[AVAGO]->serdesLoopbackCfgFunc        = BIND_SET_FUNC(mvHwsAvagoSerdesLoopback            );
    funcPtrArray[AVAGO]->serdesLoopbackGetFunc        = BIND_GET_FUNC(mvHwsAvagoSerdesLoopbackGet         );
#endif
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
    funcPtrArray[AVAGO]->serdesTestGenFunc            = BIND_SET_FUNC(mvHwsAvagoSerdesTestGen             );
    funcPtrArray[AVAGO]->serdesTestGenGetFunc         = BIND_GET_FUNC(mvHwsAvagoSerdesTestGenGet          );
    funcPtrArray[AVAGO]->serdesTestGenStatusFunc      = BIND_GET_FUNC(mvHwsAvagoSerdesTestGenStatus       );
#endif
    funcPtrArray[AVAGO]->serdesPolarityCfgFunc        = BIND_SET_FUNC(mvHwsAvagoSerdesPolarityConfig      );
    funcPtrArray[AVAGO]->serdesPolarityCfgGetFunc     = BIND_GET_FUNC(mvHwsAvagoSerdesPolarityConfigGet   );
    funcPtrArray[AVAGO]->serdesTxEnableFunc           = BIND_SET_FUNC(mvHwsAvagoSerdesTxEnable            );
    funcPtrArray[AVAGO]->serdesTxEnableGetFunc        = BIND_GET_FUNC(mvHwsAvagoSerdesTxEnableGet         );
    funcPtrArray[AVAGO]->serdesTxIfSelectFunc         = BIND_SET_FUNC(mvHwsAvagoSerdesTxIfSelect          );
    funcPtrArray[AVAGO]->serdesTxIfSelectGetFunc      = BIND_GET_FUNC(mvHwsAvagoSerdesTxIfSelectGet       );
    funcPtrArray[AVAGO]->serdesAutoTuneStopFunc       = BIND_SET_FUNC(mvHwsAvagoSerdesTxAutoTuneStop      );
    funcPtrArray[AVAGO]->serdesSpeedGetFunc           = BIND_GET_FUNC(mvHwsAvagoSerdeSpeedGet             );
    funcPtrArray[AVAGO]->serdesManualTxCfgGetFunc     = BIND_GET_FUNC(mvHwsAvagoSerdesManualTxConfigGet   );
    funcPtrArray[AVAGO]->serdesSignalDetectGetFunc    = BIND_GET_FUNC(mvHwsAvagoSerdesSignalDetectGet     );
    funcPtrArray[AVAGO]->serdesCdrLockStatusGetFunc   = BIND_GET_FUNC(mvHwsAvagoSerdesCdrLockStatusGet    );
    funcPtrArray[AVAGO]->serdesEncodingTypeGetFunc    = BIND_GET_FUNC(mvHwsAvagoSerdesEncodingTypeGet     );
    funcPtrArray[AVAGO]->serdesRxDatapathConfigGetFunc= BIND_GET_FUNC(mvHwsAvagoSerdesRxDatapathConfigGet);
    funcPtrArray[AVAGO]->serdesErrorInjectFunc        = BIND_GET_FUNC(mvHwsAvagoSerdesErrorInject);
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    funcPtrArray[AVAGO]->serdesRxPllLockGetFunc       = BIND_GET_FUNC(mvHwsAvagoSerdesRxPllLockGet);
#endif
    funcPtrArray[AVAGO]->serdesTypeGetFunc            = mvHwsAvagoSerdesTypeGetFunc;

#endif /* CO_CPU_RUN */

    return GT_OK;
}
#endif

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT)
/**
* @internal mvHwsAvago16nmIfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] ravenDevBmp              - raven supported devices
* @param[in] funcPtrArray             - array for function registration
*/
GT_STATUS mvHwsAvago16nmIfInit
(
    IN GT_U8  devNum,
    IN GT_U32 ravenDevBmp,
    IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray
)
{
#ifdef ASIC_SIMULATION
    /* avoid warning */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(ravenDevBmp);
#else
    GT_STATUS res;

#ifndef RAVEN_DEV_SUPPORT
    unsigned int chipIndex;
#endif
    GT_UNUSED_PARAM(ravenDevBmp);

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3

    /* set init value for the delay for non CM3 builds.*/
    aapl_global_delay(2000);

    if(hwsAvagoNotSupportedCheck())
    {
        /* the Avago driver MUST not be accessed */
        res = GT_OK;
    }
    else
#endif /*! MV_HWS_REDUCED_BUILD_EXT_CM3*/
    {
#ifndef RAVEN_DEV_SUPPORT
        for (chipIndex = 0; chipIndex < RAVEN_MAX_AVAGO_CHIP_NUMBER ; chipIndex++)
        {
            if ((ravenDevBmp >> chipIndex) & 0x1)
            {
                res = mvHwsAvagoSerdesInit(devNum, chipIndex + (devNum * RAVEN_MAX_AVAGO_CHIP_NUMBER));
                if (res != GT_OK)
                {
                    return res;
                }
            }
        }
#else
        res = mvHwsAvagoSerdesInit(devNum, 0);
        if (res != GT_OK)
        {
            return res;
        }
#endif
        aapl_global_delay(0);
    }

#endif

    if(hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesNumber > MAX_AVAGO_SERDES_NUMBER)
    {
        HWS_AVAGO_DBG(("Size of serdesArr_copy(%d) is not enough for serdes number = %d\n", MAX_AVAGO_SERDES_NUMBER, hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesNumber));
        return GT_BAD_STATE;
    }

    /* Avago implementation */
    if(!funcPtrArray[AVAGO_16NM])
    {
        funcPtrArray[AVAGO_16NM] = (MV_HWS_SERDES_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_SERDES_FUNC_PTRS));
        if(!funcPtrArray[AVAGO_16NM])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[AVAGO_16NM], 0, sizeof(MV_HWS_SERDES_FUNC_PTRS));
    }
    funcPtrArray[AVAGO_16NM]->serdesArrayPowerCntrlFunc    = BIND_SET_FUNC(mvHwsAvagoSerdesArrayPowerCtrl      );
    funcPtrArray[AVAGO_16NM]->serdesPowerCntrlFunc         = BIND_SET_FUNC(mvHwsAvagoSerdesPowerCtrl           );
    funcPtrArray[AVAGO_16NM]->serdesManualTxCfgFunc        = BIND_SET_FUNC(mvHwsAvagoSerdesManualTxConfig      );
    funcPtrArray[AVAGO_16NM]->serdesManualRxCfgFunc        = BIND_SET_FUNC(mvHwsAvago16nmSerdesManualRxConfig  );
    funcPtrArray[AVAGO_16NM]->serdesRxAutoTuneStartFunc    = BIND_SET_FUNC(mvHwsAvagoSerdesRxAutoTuneStart);
    funcPtrArray[AVAGO_16NM]->serdesTxAutoTuneStartFunc    = BIND_SET_FUNC(mvHwsAvagoSerdesTxAutoTuneStart);
    funcPtrArray[AVAGO_16NM]->serdesAutoTuneStartFunc      = BIND_SET_FUNC(mvHwsAvagoSerdesAutoTuneStart);
    funcPtrArray[AVAGO_16NM]->serdesAutoTuneResultFunc     = BIND_SET_FUNC(mvHwsAvago16nmSerdesAutoTuneResult);
    funcPtrArray[AVAGO_16NM]->serdesAutoTuneStatusFunc     = BIND_SET_FUNC(mvHwsAvagoSerdesAutoTuneStatus);
    funcPtrArray[AVAGO_16NM]->serdesResetFunc              = BIND_SET_FUNC(mvHwsAvagoSerdesReset);
#ifndef CO_CPU_RUN
    funcPtrArray[AVAGO_16NM]->serdesDfeCfgFunc             = BIND_SET_FUNC(mvHwsAvago16nmSerdesDfeConfig );
#ifndef RAVEN_DEV_SUPPORT
    funcPtrArray[AVAGO_16NM]->serdesDigitalReset           = BIND_SET_FUNC(mvHwsAvagoSerdesDigitalReset);
    funcPtrArray[AVAGO_16NM]->serdesAutoTuneStatusShortFunc =BIND_SET_FUNC(mvHwsAvagoSerdesAutoTuneStatusShort);
    funcPtrArray[AVAGO_16NM]->serdesLoopbackCfgFunc        = BIND_SET_FUNC(mvHwsAvagoSerdesLoopback);
    funcPtrArray[AVAGO_16NM]->serdesLoopbackGetFunc        = BIND_GET_FUNC(mvHwsAvagoSerdesLoopbackGet);
    funcPtrArray[AVAGO_16NM]->serdesTxIfSelectFunc         = BIND_SET_FUNC(mvHwsAvagoSerdesTxIfSelect          );
    funcPtrArray[AVAGO_16NM]->serdesTxIfSelectGetFunc      = BIND_GET_FUNC(mvHwsAvagoSerdesTxIfSelectGet       );
    funcPtrArray[AVAGO_16NM]->serdesTxEnableGetFunc        = BIND_GET_FUNC(mvHwsAvagoSerdesTxEnableGet         );
    funcPtrArray[AVAGO_16NM]->serdesErrorInjectFunc        = BIND_GET_FUNC(mvHwsAvagoSerdesErrorInject);
#endif
#if ( !defined(MV_HWS_FREE_RTOS) )
    funcPtrArray[AVAGO_16NM]->serdesAuxCounterStartFunc    = BIND_SET_FUNC(mvHwsAvagoSerdesAuxCounterStart);
    funcPtrArray[AVAGO_16NM]->serdesTestGenFunc            = BIND_SET_FUNC(mvHwsAvagoSerdesTestGen);
    funcPtrArray[AVAGO_16NM]->serdesTestGenGetFunc         = BIND_GET_FUNC(mvHwsAvagoSerdesTestGenGet);
    funcPtrArray[AVAGO_16NM]->serdesTestGenStatusFunc      = BIND_GET_FUNC(mvHwsAvagoSerdesTestGenStatus);
#endif
    funcPtrArray[AVAGO_16NM]->serdesPolarityCfgFunc        = BIND_SET_FUNC(mvHwsAvagoSerdesPolarityConfig);
    funcPtrArray[AVAGO_16NM]->serdesPolarityCfgGetFunc     = BIND_GET_FUNC(mvHwsAvagoSerdesPolarityConfigGet);
    funcPtrArray[AVAGO_16NM]->serdesTxEnableFunc           = BIND_SET_FUNC(mvHwsAvagoSerdesTxEnable);
    funcPtrArray[AVAGO_16NM]->serdesAutoTuneStopFunc       = BIND_SET_FUNC(mvHwsAvagoSerdesTxAutoTuneStop      );
    funcPtrArray[AVAGO_16NM]->serdesSpeedGetFunc           = BIND_GET_FUNC(mvHwsAvagoSerdeSpeedGet             );
    funcPtrArray[AVAGO_16NM]->serdesManualTxCfgGetFunc     = BIND_GET_FUNC(mvHwsAvagoSerdesManualTxConfigGet   );
    funcPtrArray[AVAGO_16NM]->serdesSignalDetectGetFunc    = BIND_GET_FUNC(mvHwsAvagoSerdesSignalDetectGet     );
    funcPtrArray[AVAGO_16NM]->serdesTypeGetFunc            = mvHwsAvago16nmSerdesTypeGetFunc;
    funcPtrArray[AVAGO_16NM]->serdesLowPowerModeEnableFunc = BIND_SET_FUNC(mvHwsAvagoSerdesLowPowerModeEnable);
    funcPtrArray[AVAGO_16NM]->serdesCdrLockStatusGetFunc   = BIND_GET_FUNC(mvHwsAvagoSerdesCdrLockStatusGet);
    funcPtrArray[AVAGO_16NM]->serdesEncodingTypeGetFunc    = BIND_GET_FUNC(mvHwsAvagoSerdesEncodingTypeGet);
    funcPtrArray[AVAGO_16NM]->serdesRxDatapathConfigGetFunc= BIND_GET_FUNC(mvHwsAvagoSerdesRxDatapathConfigGet);
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    funcPtrArray[AVAGO_16NM]->serdesRxPllLockGetFunc       = BIND_GET_FUNC(mvHwsAvagoSerdesRxPllLockGet);
    funcPtrArray[AVAGO_16NM]->serdesDroGetFunc             = BIND_GET_FUNC(mvHwsAvago16nmSerdesPmroMetricGet);
#endif

    funcPtrArray[AVAGO_16NM]->serdesOperFunc               = BIND_SET_FUNC(mvHwsAvagoSerdesOperation);

#endif /* CO_CPU_RUN */

    return GT_OK;
}
#endif /* #if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT) */

/**
* @internal mvHwsAvagoSetFwDownloadFileNames function
* @endinternal
* @brief   Set Avago FW Download files names
*         hwsOsPrintf("Chip %d (%s %s), ring %d, SBus devices:
*         %d\n", chip, aapl->chip_name[chip],
*         aapl->chip_rev[chip], ring,
*         aapl->max_sbus_addr[chip][ring]);
* @retval 0                        - on success
* @retval 1                        - on error
*/
#ifndef ASIC_SIMULATION
/**
* @internal mvHwsAvagoSerdesRxAutoTuneStatusShort function
* @endinternal
*
* @brief   Per SERDES check the Rx training status
*         This function is necessary for 802.3ap functionality
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] rxStatus                 - status of Rx-Training
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesRxAutoTuneStatusShort
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    OUT MV_HWS_AUTO_TUNE_STATUS *rxStatus
)
{
    GT_32 data;

    *rxStatus = TUNE_NOT_COMPLITED;

    /* get DFE status */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x126, ((0 << 12) | (0xB << 8)), &data));

    /* 0xC0: Traning PASS with VOS, 0x80: Traning PASS without VOS */
    if(data == 0x80 || data == 0xC0)
    {
        *rxStatus = TUNE_PASS;

        if (HWS_DEV_SERDES_TYPE(devNum, serdesNum) != AVAGO_16NM)
        {
            /* Enable TAP1 after Rx training */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, TAP1_AVG_ENABLE, NULL));
        }
    }
    /* TUNE_FAIL is relevant only for DFE_ICAL mode */
    else if (data == 0x0)
    {
        *rxStatus = TUNE_RESET;
    }
    else if ((data >> 0x9) & 0x1) /*bit[9]*/
    {
        *rxStatus = TUNE_FAIL;
    }


    return GT_OK;
}
#endif /* ASIC_SIMULATION */


/**
* @internal mvHwsAvagoSerdesTxAutoTuneStatusShort function
* @endinternal
*
* @brief   Per SERDES check the Tx training status
*         This function is necessary for 802.3ap functionality
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxAutoTuneStatusShort
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    OUT MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(txStatus);
#else

    GT_U32 data;
    GT_U32 sbus_addr;
    unsigned int chipIndex;
    GT_32 result = 0, sumResults = 0;
    RETURN_GT_OK_ON_EMULATOR;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    *txStatus = TUNE_NOT_COMPLITED;

    /* check PMD training status */
    AVAGO_LOCK(devNum, chipIndex);
    data = avago_serdes_pmd_status(aaplSerdesDb[chipIndex], sbus_addr);
    AVAGO_UNLOCK(devNum, chipIndex);
    if (data == 0)
        *txStatus = TUNE_FAIL;
    else if (data == 0x1)
    {
        *txStatus = TUNE_PASS;

        if (HWS_DEV_SERDES_TYPE(devNum, serdesNum) != AVAGO_16NM)
        {
            /* Enable TAP1 after pmd_train is completed */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, TAP1_AVG_ENABLE, NULL));
        }
        else /*Read all options of preset 1,2,3, init and only one can be or none*/
        {
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x126, 0x6400, &result));
            sumResults += result;
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x126, 0x6500, &result));
            sumResults += result;
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x126, 0x6600, &result));
            sumResults += result;
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x126, 0x6700, &result));
            sumResults += result;
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x126, 0x6800, &result));
            sumResults += result;

            if(sumResults>1)
            {
                *txStatus = TUNE_FAIL;
            }
        }
    }
    else if (data == 0x2)
        *txStatus = TUNE_NOT_COMPLITED;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesAutoTuneStatusShort function
* @endinternal
*
* @brief   Check the Serdes Rx or Tx training status
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] rxStatus                 - Rx-Training status
* @param[out] txStatus                 - Tx-Training status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneStatusShort
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  serdesNum,
    OUT MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    OUT MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    if(rxStatus)
    {
        *rxStatus = TUNE_PASS;
    }
    if(txStatus)
    {
        *txStatus = TUNE_PASS;
    }
#else
    if ((NULL == rxStatus) && (NULL == txStatus))
    {
        return GT_BAD_PARAM;
    }

    if (rxStatus != NULL)
    {
        CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesNum, rxStatus));
    }

    if (txStatus != NULL)
    {
        CHECK_STATUS(mvHwsAvagoSerdesTxAutoTuneStatusShort(devNum, portGroup, serdesNum, txStatus));
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}



/**
* @internal mvHwsAvagoSerdesArrayHaltDfeTune function
* @endinternal
*
* @brief   Halts DFE tune for safe SerDes power down
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - physical serdes number
*
* @param[out] numOfSer                 - number of serdes's
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesArrayHaltDfeTune
(
    IN GT_U8 devNum,
    IN GT_UOPT portGroup,
    IN GT_UOPT *serdesArr,
    OUT GT_UOPT numOfSer
)
{
#ifdef ASIC_SIMULATION
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    (GT_VOID)serdesArr;
    GT_UNUSED_PARAM(numOfSer);
#else
    MV_HWS_AUTO_TUNE_STATUS rxStatus;
    GT_U32 i;
    GT_BOOL skipSbusReset = GT_TRUE;
    GT_U32 tryCount = 0;


    /* Performing hard reset to the SERDES to ensure no training is running while
    the SERDES will go to disable mode */
    for (i=0; i< numOfSer; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesArr[i], &rxStatus));
        if ((rxStatus != TUNE_PASS) && (rxStatus != TUNE_RESET))
        {
            /* it's enough if even 1 of the lane needs sbus_reset to apply it for all active lanes */
            skipSbusReset = GT_FALSE;
        }
    }
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3

     if (prvCpssSystemRecoveryInProgress())
     {
         return GT_OK;
     }
#endif

    if (skipSbusReset)
    {
        return GT_OK;
    }

    for (i=0; i< numOfSer; i++)
    {
        /* perform sbus reset */
        CHECK_STATUS(mvHwsAvagoSbusReset(devNum,serdesArr[i],GT_TRUE));
    }

    hwsOsExactDelayPtr(devNum, portGroup, 20);

    for (i=0; i< numOfSer; i++)
    {
        /* perform sbus Un-reset */
        CHECK_STATUS(mvHwsAvagoSbusReset(devNum,serdesArr[i],GT_FALSE));
    }

    hwsOsExactDelayPtr(devNum, portGroup, 10);

    for (i=0; i< numOfSer; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesArr[i], &rxStatus));
        while ((rxStatus != TUNE_PASS) && (rxStatus != TUNE_RESET) && (tryCount < MV_HWS_AVAGO_SBUS_RESET_TIMEOUT))
        {
            CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesArr[i], &rxStatus));
            /* since we are waiting for HW to finish it is ok to use delay at this point, there is no need
               for SW parallelization - same effect */
            tryCount++;
            hwsOsExactDelayPtr(devNum, portGroup, 10);
        }
    }

    if (tryCount == MV_HWS_AVAGO_SBUS_RESET_TIMEOUT)
    {
        return GT_TIMEOUT;
    }
#endif
    return GT_OK;
}



/**
* @internal mvHwsAvagoSerdesArrayPowerCtrl function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] numOfSer  - number of SERDESes to configure
* @param[in] serdesArr - collection of SERDESes to configure
* @param[in] powerUp   - True for PowerUP, False for PowerDown
* @param[in] serdesConfigPtr - pointer to Serdes params struct
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvagoSerdesArrayPowerCtrl
(
    IN GT_U8                       devNum,
    IN GT_UOPT                     portGroup,
    IN GT_UOPT                     numOfSer,
    IN GT_UOPT                     *serdesArr,
    IN GT_BOOL                     powerUp,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(numOfSer);
    (GT_VOID)serdesArr;

    /* do some sanity checks (that failed on HW) and can be found by WM ! */
    if(serdesConfigPtr == NULL)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("NULL pointer error\n"));
    }

    /* define divider value */
    if(powerUp == GT_TRUE)
    {
        switch(serdesConfigPtr->refClock)
        {
            case _78Mhz:
            case _156dot25Mhz:
            case _312_5Mhz:
            case _164_24Mhz:
                break;

            default:
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("illegal reference clock\n"));
        }
    }

#else
    GT_UREG_DATA    data;
    GT_UOPT serdesNum;
    GT_U32 divider;
    GT_U8 dwell = NA_VALUE;

    if(serdesConfigPtr == NULL)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("NULL pointer error\n"));
    }

    /* define divider value */
    if(powerUp == GT_TRUE)
    {
        switch(serdesConfigPtr->refClock)
        {
            case _78Mhz:
                divider = avagoBaudRate2DividerValue_78M[serdesConfigPtr->baudRate];
                break;

            case _156dot25Mhz:
                divider = avagoBaudRate2DividerValue_156M[serdesConfigPtr->baudRate];
                break;

            case _312_5Mhz:
                divider = avagoBaudRate2DividerValue_312_5M[serdesConfigPtr->baudRate];
                break;

            case _164_24Mhz:
                divider = avagoBaudRate2DividerValue_164_24M[serdesConfigPtr->baudRate];
                break;

            default:
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("illegal reference clock\n"));
        }

        if(divider == NA_VALUE)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("illegal baudrate divider\n"));
        }


        /* define dwell value */
        dwell = avagoBaudRate2DwellTimeValue[serdesConfigPtr->baudRate];
        if(serdesConfigPtr->baudRate == _1_25G)
        {
            dwell = 11;
        }
    }
    else
    {
        divider = NA_VALUE;
    }

    CHECK_STATUS(mvHwsAvagoSerdesArrayPowerCtrlImpl(devNum, portGroup, numOfSer, serdesArr, powerUp,
                                                    divider,
                                                    serdesConfigPtr));

    if ( powerUp == GT_TRUE && (serdesConfigPtr->serdesType != AVAGO_16NM) )
    {
        if (dwell != NA_VALUE)
        {
            for (serdesNum = 0; serdesNum < numOfSer; serdesNum++)
            {
                /* select DFE tuning dwell time */
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[serdesNum], 0x18, 0x7, NULL));
                /* big register write LSB */
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[serdesNum], 0x19, (1 << dwell), NULL));
                /* big register write MSB */
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[serdesNum], 0x19, 0, NULL));
            }
        }
    }

    /*  RM-6080676
     *      The SERDES Signal Detect polarity in 1 / 2.5Gbps modes
     *      should be inverted. In 1G and 2.5G mode, Bit[0] in the
     *      SERDES Miscellaneous register should = 1.
    */
    if ( (Bobcat3 == HWS_DEV_SILICON_TYPE(devNum)) || (Pipe == HWS_DEV_SILICON_TYPE(devNum))
         || (Aldrin2 == HWS_DEV_SILICON_TYPE(devNum)) )
    {
        if(((serdesConfigPtr->busWidth == _10BIT_ON) && (serdesConfigPtr->baudRate == _1_25G))  ||
           ((serdesConfigPtr->busWidth == _10BIT_ON) && (serdesConfigPtr->baudRate == _3_125G)) ||
           ((serdesConfigPtr->busWidth == _10BIT_ON) && (serdesConfigPtr->baudRate == _5G)))
        {
            data = (powerUp == GT_FALSE) ? 0 : 1;
        }
        else
        {
            data = 0;
        }
        for (serdesNum = 0; serdesNum < numOfSer; serdesNum++)
        {
            CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[serdesNum],
                                                AVAGO_SD_METAL_FIX, data, 1));
        }
    }

    return GT_OK;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

#if !defined (ASIC_SIMULATION) && !defined (MV_HWS_REDUCED_BUILD_EXT_CM3)
/**
* @internal mvHwsAvagoSerdesPMDdebugPrint function
* @endinternal
*
* @brief   Per SERDES configure the TX parameters: amplitude, 3 TAP Tx FIR.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPMDdebugPrint
(
    IN GT_U8    devNum,
    IN GT_U32   serdesNum
)
{
    Avago_serdes_pmd_debug_t *pmd_debug;
    unsigned int    sbus_addr;
    unsigned int    chipIndex;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* Allocates and initializes memory for a PMD debug structure */
    AVAGO_LOCK(devNum, chipIndex);
    pmd_debug = avago_serdes_pmd_debug_construct(aaplSerdesDb[chipIndex]);
    if ( pmd_debug != NULL )
    {
        /* Gathers up the PMD training debug information from Serdes */
        avago_serdes_pmd_debug(aaplSerdesDb[chipIndex], sbus_addr, pmd_debug);

        /* print PMD debug info */
        avago_serdes_pmd_debug_print(aaplSerdesDb[chipIndex], pmd_debug);

        /* Frees the memory pointed */
        avago_serdes_pmd_debug_destruct(aaplSerdesDb[chipIndex], pmd_debug);
    }
    AVAGO_UNLOCK(devNum, chipIndex);

    return GT_OK;
}
#endif /* !defined (ASIC_SIMULATION) && !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) */

#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
/**
* @internal mvHwsAvagoSerdesPowerCtrl function
* @endinternal
*
* @brief   Power up Serdes number
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - Serdes number
* @param[in] powerUp   - True for PowerUP, False for PowerDown
* @param[in] serdesConfig - pointer to Serdes paramters:
*      refClock  - ref clock value
*      refClockSource - ref clock source (primary line or
*      secondary)
*      baudRate - Serdes speed
*      busWidth - Serdes bus modes: 10Bits/20Bits/40Bits
*      media - RXAUI or XAUI
*      encoding - Rx & Tx data encoding NRZ/PAM4
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvagoSerdesPowerCtrl
(
    IN GT_U8                       devNum,
    IN GT_UOPT                     portGroup,
    IN GT_UOPT                     serdesNum,
    IN GT_BOOL                     powerUp,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    GT_UOPT serdesArr[1];

    serdesArr[0] = serdesNum;
    return mvHwsAvagoSerdesArrayPowerCtrl(devNum, portGroup, 1, serdesArr, powerUp, serdesConfigPtr);
}
#else

/**
* @internal mvHwsAvagoSerdesPowerCtrl function
* @endinternal
*
* @brief   Power up Serdes number
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - Serdes number
* @param[in] powerUp   - True for PowerUP, False for PowerDown
* @param[in] serdesConfig - pointer to Serdes paramters:
*      refClock  - ref clock value
*      refClockSource - ref clock source (primary line or
*      secondary)
*      baudRate - Serdes speed
*      busWidth - Serdes bus modes: 10Bits/20Bits/40Bits
*      media - RXAUI or XAUI
*      encoding - Rx & Tx data encoding NRZ/PAM4
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvagoSerdesPowerCtrl
(
    IN GT_U8                       devNum,
    IN GT_UOPT                     portGroup,
    IN GT_UOPT                     serdesNum,
    IN GT_BOOL                     powerUp,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(powerUp);
    (GT_VOID)serdesConfigPtr;
#else
    GT_U32 divider;
    GT_U8 dwell = NA_VALUE;

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3

     if (prvCpssSystemRecoveryInProgress())
     {
         return GT_OK;
     }
#endif

    if(serdesConfigPtr == NULL)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("NULL pointer error\n"));
    }

    /* define deivider value */
    if(powerUp == GT_TRUE)
    {
        switch(serdesConfigPtr->refClock)
        {
            case _78Mhz:
                divider = avagoBaudRate2DividerValue_78M[serdesConfigPtr->baudRate];
                break;

            case _156dot25Mhz:
                divider = avagoBaudRate2DividerValue_156M[serdesConfigPtr->baudRate];
                break;

            case _312_5Mhz:
                divider = avagoBaudRate2DividerValue_312_5M[serdesConfigPtr->baudRate];
                break;

            case _164_24Mhz:
                divider = avagoBaudRate2DividerValue_164_24M[serdesConfigPtr->baudRate];
                break;

            default:
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("illegal reference clock\n"));
        }

        if(divider == NA_VALUE)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("illegal baudrate divider\n"));
        }

        /* define dwell value */
        dwell = avagoBaudRate2DwellTimeValue[serdesConfigPtr->baudRate];
        if(serdesConfigPtr->baudRate == _1_25G)
        {
            dwell = 11;
        }
    }
    else
    {
        divider = NA_VALUE;
    }

    CHECK_STATUS(mvHwsAvagoSerdesPowerCtrlImpl(devNum, portGroup, serdesNum, powerUp,divider,serdesConfigPtr));

    if ( powerUp == GT_TRUE && (serdesConfigPtr->serdesType != AVAGO_16NM) )
    {
        if (dwell != NA_VALUE)
        {
            /* select DFE tuning dwell time */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x7, NULL));
            /* big register write LSB */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 0x800, NULL));
            /* big register write MSB */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 0, NULL));
        }
    }

    /*  RM-6080676 for BC3 only */

    return GT_OK;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}
#endif

/**
* @internal mvHwsAvagoSerdesManualTxConfig function
* @endinternal
*
* @brief   Per SERDES configure the TX parameters
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txAmp                    - Tx Attenuator [0...31]
* @param[in] txAmpAdj                 - not used in Avago serdes
* @param[in] emph0                    - Post-Cursor: for BobK/Aldrin (IP_Rev=0xde) [-31...31],
*                                      for Bobcat3 (IP_Rev=0xcd) [0...31]
* @param[in] emph1                    - Pre-Cursor:  for BobK/Aldrin (IP_Rev=0xde) [-31...31],
*                                      for Bobcat3 (IP_Rev=0xcd) [0...31]
* @param[in] slewRate                 - Slew rate  [0...3], 0 is fastest
*                                      (the SlewRate parameter is not supported in Avago Serdes 28nm)
* @param[in] txEmphEn                  -  Tx emph Enable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualTxConfig
(
    IN GT_U8    devNum,
    IN GT_UOPT  portGroup,
    IN GT_UOPT  serdesNum,
    IN MV_HWS_SERDES_TX_CONFIG_DATA_UNT *txConfigPtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(txConfigPtr);
    /*hwsOsMemSetFuncPtr(txConfigPtr, 0, sizeof(txConfigPtr));*/

#else

    GT_U32   sbus_addr;
    unsigned int    ip_rev;
    Avago_serdes_tx_eq_t    txParamValues;
    unsigned int chipIndex;
    if(txConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }
    hwsOsMemSetFuncPtr(&txParamValues, 0, sizeof(Avago_serdes_tx_eq_t));
    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    AVAGO_LOCK(devNum, chipIndex);
    ip_rev = aapl_get_ip_rev(aaplSerdesDb[chipIndex], sbus_addr);
    AVAGO_UNLOCK(devNum, chipIndex);

    txParamValues.atten = txConfigPtr->txAvago.atten;
    txParamValues.post  = SIGNED8_TO_SIGNED16(txConfigPtr->txAvago.post);
    txParamValues.pre   = SIGNED8_TO_SIGNED16(txConfigPtr->txAvago.pre);
    txParamValues.pre2  = SIGNED8_TO_SIGNED16(txConfigPtr->txAvago.pre2);
    txParamValues.pre3  = SIGNED8_TO_SIGNED16(txConfigPtr->txAvago.pre3);

    /* Attenuator [0...31] */
    if ((txParamValues.atten & 0xFFFFFFE0) != 0)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ARG_STRING_MAC("Attenuator must be [0...31]\n"));
    }

    if ((txParamValues.post > 31) || (txParamValues.pre > 31))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ARG_STRING_MAC("Post-Cursor/Pre-Cursor must be [-31...31]\n"));
    }

    /* for BobK/Aldrin/Aldrin2(Serdes 24-71)/Pipe(Serdes 0-11) */
    if (ip_rev == 0xde)
    {
#ifndef RAVEN_DEV_SUPPORT
        /* Post-Cursor [-31...31] */
        if ((txParamValues.post < -31) || (txParamValues.post > 31))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ARG_STRING_MAC("Post-Cursor must be [-31...31]\n"));
        }
        /* Pre-Cursor [-31...31] */
        if ((txParamValues.pre < -31) || (txParamValues.pre > 31))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ARG_STRING_MAC("Pre-Cursor must be [-31...31]\n"));
        }
        txParamValues.pre2 = 0;
        txParamValues.pre3 = 0;
    }
    /* for Bobcat3/Aldrin2(Serdes 0-23)/Pipe(Serdes 12-15) */
    else if (ip_rev == 0xcd)
    {
        /* Post-Cursor [0...31] */
        if ((txParamValues.post < 0) || (txParamValues.post > 31))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ARG_STRING_MAC("Post-Cursor must be [0...31]\n"));
        }
        /* Pre-Cursor [0...31] */
        if ((txParamValues.pre < 0) || (txParamValues.pre > 31))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ARG_STRING_MAC("Pre-Cursor must be [0...31]\n"));
        }
        txParamValues.pre2 = 0;
        txParamValues.pre3 = 0;
#endif
    }

#if !defined BOBK_DEV_SUPPORT && !defined ALDRIN_DEV_SUPPORT
    /* for Raven */
    else if (HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO_16NM)
    {
        /* Post-Cursor [even values: -18...18] */
        if ((txParamValues.post < -18) || (txParamValues.post > 18) || ((txParamValues.post % 2) != 0))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ARG_STRING_MAC("Post-Cursor must be [even values: -18...18]\n"));
        }
        /* Pre-Cursor [even values: -10...10] */
        if ((txParamValues.pre < -10) || ((txParamValues.pre) > 10) || ((txParamValues.pre % 2) != 0))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ARG_STRING_MAC("Pre-Cursor must be [even values: -10...10]\n"));
        }

        /* Pre2-cursor [-15...15] */
        if ((txParamValues.pre2 < -15) || (txParamValues.pre2 > 15))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ARG_STRING_MAC("Pre2-Cursor must be [-15...15]\n"));
        }
        /* Pre3-cursor [-1, 0, 1] */
        if ((txParamValues.pre3 < -1) || (txParamValues.pre3 > 1))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ARG_STRING_MAC("Pre3-Cursor must be [-1, 0, 1]\n"));
        }

        /* (Pre3 + Pre2 + Pre1 + Atten + Post <= 26) */
        if (txParamValues.atten +
            txParamValues.post  +
            txParamValues.pre   +
            txParamValues.pre2  +
            txParamValues.pre3 > 26)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ARG_STRING_MAC("Pre3 + Pre2 + Pre + Atten + Post > 26)\n"));
        }
    }
#endif
    else
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("Unknow ip_rev\n"));
    }

    AVAGO_LOCK(devNum, chipIndex);
    avago_serdes_set_tx_eq(aaplSerdesDb[chipIndex], sbus_addr, &txParamValues);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();
#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesManualTxConfigGet function
* @endinternal
*
* @brief   Per SERDES get the configure TX parameters: amplitude, 3 TAP Tx FIR.
*         Can be run after create port.
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - physical serdes number
*
* @param[out] txConfigPtr - pointer to Serdes Tx params struct
*               MV_HWS_MAN_TUNE_COMPHY_H_TX_CONFIG_DATA
*               MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualTxConfigGet
(
    IN GT_U8                              devNum,
    IN GT_UOPT                            portGroup,
    IN GT_UOPT                            serdesNum,
    OUT MV_HWS_SERDES_TX_CONFIG_DATA_UNT  *txConfigPtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(txConfigPtr);
    hwsOsMemSetFuncPtr(txConfigPtr, 0, sizeof(txConfigPtr));
#else

    GT_U32    sbus_addr;
    unsigned int chipIndex;
    Avago_serdes_tx_eq_t    txParamValues;

    if (txConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    hwsOsMemSetFuncPtr(txConfigPtr, 0, sizeof(txConfigPtr));

    AVAGO_LOCK(devNum, chipIndex);
    avago_serdes_get_tx_eq(aaplSerdesDb[chipIndex], sbus_addr, &txParamValues);
    AVAGO_UNLOCK(devNum, chipIndex);

    txConfigPtr->txAvago.atten = txParamValues.atten;
    txConfigPtr->txAvago.post  = txParamValues.post;
    txConfigPtr->txAvago.pre   = txParamValues.pre;
    txConfigPtr->txAvago.pre2  = txParamValues.pre2;
    txConfigPtr->txAvago.pre3  = txParamValues.pre3;
#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesCdrLockStatusGet function
* @endinternal
*
* @brief   Reads and resets the signal_ok_deasserted signal.
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - physical serdes number
*
* @param[out]  enable   - (pointer to) enable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesCdrLockStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_BOOL                 *enable
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    *enable = GT_TRUE;
#else

    GT_U32    sbus_addr;
    unsigned int chipIndex;

    if (NULL == enable )
    {
        return GT_BAD_PTR;
    }

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    *enable = GT_FALSE;

    AVAGO_LOCK(devNum, chipIndex);
    *enable = (avago_serdes_get_signal_ok(aaplSerdesDb[chipIndex], sbus_addr, FALSE)) ? GT_TRUE : GT_FALSE;
    AVAGO_UNLOCK(devNum, chipIndex);

#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);

    return GT_OK;
}

/**
* @internal mvHwsSerdesEncodingTypeGet function
* @endinternal
*
* @brief   Retrieves the Tx and Rx line encoding values.
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - physical PCS number
*
* @param[out] txEncodingPtr           - NRZ/PAM4
* @param[out] rxEncodingPtr           - NRZ/PAM4
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEncodingTypeGet
(
    GT_U8                            devNum,
    GT_U32                           serdesNum,
    MV_HWS_SERDES_ENCODING_TYPE     *txEncodingPtr,
    MV_HWS_SERDES_ENCODING_TYPE     *rxEncodingPtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    *txEncodingPtr = SERDES_ENCODING_NA;
    *rxEncodingPtr = SERDES_ENCODING_NA;
#else

    GT_U32    sbus_addr;
    unsigned int chipIndex;
    Avago_serdes_line_encoding_t txEncoding;
    Avago_serdes_line_encoding_t rxEncoding;

    if (NULL == txEncodingPtr ||  NULL == rxEncodingPtr)
    {
        return GT_BAD_PTR;
    }

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    *txEncodingPtr = SERDES_ENCODING_NA;
    *rxEncodingPtr = SERDES_ENCODING_NA;

    AVAGO_LOCK(devNum, chipIndex);
    avago_serdes_get_tx_rx_line_encoding(aaplSerdesDb[chipIndex], sbus_addr, &txEncoding, &rxEncoding);
    AVAGO_UNLOCK(devNum, chipIndex);

    *txEncodingPtr = (txEncoding == AVAGO_SERDES_PAM4) ? SERDES_ENCODING_PAM4 : SERDES_ENCODING_NRZ;
    *rxEncodingPtr = (rxEncoding == AVAGO_SERDES_PAM4) ? SERDES_ENCODING_PAM4 : SERDES_ENCODING_NRZ;

#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesRxDatapathConfigGet function
* @endinternal
*
* @brief   Retrieves the Rx data path configuration values
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - physical PCS number
*
* @param[out] rxDatapathConfigPtr     - (pointer to)struct:
*                                        polarityInvert (true/false)
*                                        grayEnable     (true/false)
*                                        precodeEnable  (true/false)
*                                        swizzleEnable  (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesRxDatapathConfigGet
(
    GT_U8                                  devNum,
    GT_U32                                 serdesNum,
    MV_HWS_SERDES_RX_DATAPATH_CONFIG_STC   *rxDatapathConfigPtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    hwsOsMemSetFuncPtr(rxDatapathConfigPtr, 0, sizeof(rxDatapathConfigPtr));
#else

    GT_U32    sbus_addr;
    unsigned int chipIndex;
    Avago_serdes_datapath_t rxDatapath;

    if (NULL == rxDatapathConfigPtr)
    {
        return GT_BAD_PTR;
    }

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    hwsOsMemSetFuncPtr(rxDatapathConfigPtr, 0, sizeof(rxDatapathConfigPtr));

    AVAGO_LOCK(devNum, chipIndex);
    avago_serdes_get_rx_datapath(aaplSerdesDb[chipIndex], sbus_addr, &rxDatapath);
    AVAGO_UNLOCK(devNum, chipIndex);

    rxDatapathConfigPtr->grayEnable     = (rxDatapath.gray_enable)     ? GT_TRUE : GT_FALSE;
    rxDatapathConfigPtr->polarityInvert = (rxDatapath.polarity_invert) ? GT_TRUE : GT_FALSE;
    rxDatapathConfigPtr->precodeEnable  = (rxDatapath.precode_enable)  ? GT_TRUE : GT_FALSE;
    rxDatapathConfigPtr->swizzleEnable  = (rxDatapath.swizzle_enable)  ? GT_TRUE : GT_FALSE;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
/**
* @internal mvHwsAvagoSerdesRxPllLockGet function
* @endinternal
*
* @brief   Returns whether or not the RX PLL is frequency locked
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
*
* @param[out] lockPtr                 - (pointer to) lock state
*                                        GT_TRUE =  locked
*                                        GT_FALSE = not locked
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesRxPllLockGet
(
    GT_U8                 devNum,
    GT_U32                serdesNum,
    GT_BOOL               *lockPtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    *lockPtr = GT_TRUE;
#else

    GT_U32    sbus_addr;
    unsigned int chipIndex;

    if (NULL == lockPtr)
    {
        return GT_BAD_PTR;
    }

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    *lockPtr = GT_FALSE;

    AVAGO_LOCK(devNum, chipIndex);
    *lockPtr = (avago_serdes_get_frequency_lock(aaplSerdesDb[chipIndex], sbus_addr)) ? GT_TRUE : GT_FALSE;
    AVAGO_UNLOCK(devNum, chipIndex);

#endif /* ASIC_SIMULATION */

    return GT_OK;
}
#endif /* #ifndef MV_HWS_REDUCED_BUILD_EXT_CM3 */

/**
* @internal mvHwsAvagoSerdesSignalDetectGet function
* @endinternal
*
* @brief   Per SERDES get indication check CDR lock and Signal Detect.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] signalDet                - TRUE if signal detected and FALSE otherwise.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalDetectGet
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    OUT GT_BOOL     *signalDet
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    *signalDet = GT_TRUE;
#else

    GT_U32 sbus_addr;
    GT_U32 i, data;
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined RAVEN_DEV_SUPPORT || defined FALCON_DEV_SUPPORT
    GT_U8 hitCounter = 0;
    GT_32 sdata;
#endif
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    MV_HWS_SERDES_LB  lbMode;
#endif
    RETURN_GT_OK_ON_EMULATOR;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined RAVEN_DEV_SUPPORT || defined FALCON_DEV_SUPPORT
    if (HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO_16NM)
    {

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
         /* in serdes loopback analog tx2rx, we forced signal  */
        CHECK_STATUS(mvHwsAvagoSerdesLoopbackGet(devNum, portGroup, serdesNum, &lbMode));
        if ( lbMode == SERDES_LP_AN_TX_RX )
        {
            *signalDet = GT_TRUE;
            return GT_OK;
        }
#endif
        /*
            For long cables AVAGO_16nm provides unstable results,
            so it is needed to continue polling more time.
        */
        *signalDet = GT_FALSE;

        for ( i = 0; i < 12 ; i++ )
        {
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x4026, 0, &sdata));
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x8026, (sdata & 0xffef), NULL));
            hwsOsTimerWkFuncPtr(1);
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x4026, 0, &sdata));

            if((sdata & 0x10) == 0)
            {
                hitCounter++;
            }else{
                hitCounter = 0;
            }
            if(hitCounter >= 10)
            {
                *signalDet = GT_TRUE;
                return GT_OK;
            }
            else if ( (hitCounter == 0) && (i>=1) )
            {
                return GT_OK;
            }
        }
#if 0
        for (i = 0; i < 1200; i++)
        {
            /* cable is not connected */
            if ((i>100) && (hitCounter == 0))
            {
                *signalDet = GT_FALSE;
                return GT_OK;
            }
            /*CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_EXTERNAL_STATUS, &data, (1 << 2)));*/
            /* check signal_ok in oCoreStatus instead of idle */
            CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_CORE_STATUS, &data, (1 << 4)));
            if(data/* == 0*/)
            {
                hitCounter++;
                if(hitCounter >= 20)
                {
                    *signalDet = GT_TRUE;
                    return GT_OK;
                }
            }
        }
        *signalDet = GT_FALSE;
#endif
    }
    else
#endif
    {
        /* check Signal Detect on Serdes */
        for(i=0; i < 10; i++)
        {
            CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_EXTERNAL_STATUS, &data, (1 << 2)));
            if(data)
            {
                *signalDet = GT_FALSE;
                return GT_OK;
            }
        }
        *signalDet = GT_TRUE;
    }

#endif /* ASIC_SIMULATION */

    return GT_OK;
}

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT)
/**
* @internal mvHwsAvagoSignalDetectInvert function
* @endinternal
*
* @brief   Per SERDES get indication check CDR lock and Signal
*          Detect change.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] invert                - TRUE if invert signal
*                                        detect
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSignalDetectInvert
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    OUT GT_BOOL     invert
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(invert);
#else

    GT_U32 data;
    data = (invert ? 0x1 : 0x0);

    RETURN_GT_OK_ON_EMULATOR;

    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_SD_METAL_FIX , data, 0x1));

#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSyncOkErrataFix function
* @endinternal
*
* @brief   Per SERDES fix for wrong signal detect indication with 25G RS-FEC mode.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSyncOkErrataFix
(
    IN GT_U8                            devNum,
    IN GT_U32                           portGroup,
    IN GT_U32                           serdesNum
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
#else

    GT_UREG_DATA                            regData;
    GT_BOOL                                 isSignalDetectOverrideSet = GT_FALSE;

    RETURN_GT_OK_ON_EMULATOR;

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_SD_METAL_FIX , &regData, 0xFFFF));
    /* Bit 4 - Signal Ditect override*/
    if(((regData & 0x10) >> 4))
    {
        isSignalDetectOverrideSet = GT_TRUE;
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_SD_METAL_FIX , 0x0, 0x10));
    }
    /* Flip rf_metal_fix[0] and back to the old value */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_SD_METAL_FIX , &regData, 0x1));
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_SD_METAL_FIX , regData^0x1 , 0x1));
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_SD_METAL_FIX , regData , 0x1));
    /* If override is previously set, set it back */
    if(isSignalDetectOverrideSet)
    {
        isSignalDetectOverrideSet = GT_FALSE;
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_SD_METAL_FIX , 0x10, 0x10));
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}
#endif
/**
* @internal mvHwsAvagoPllStableGet function
* @endinternal
*
* @brief   Wait for stable PLL signal
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - SerDes number
*
* @param[out] pllStable                - PLL Stable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoPllStableGet
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    OUT GT_BOOL     *pllStable
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    *pllStable = GT_TRUE;
#else
    GT_UREG_DATA    data_ureg;

    CHECK_STATUS_EXT(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_EXTERNAL_STATUS, &data_ureg, 0x1),
                     LOG_ARG_SERDES_IDX_MAC(serdesNum), LOG_ARG_STRING_MAC("reading o_tx_rdy"));
    if (data_ureg != 0)
    {
        *pllStable = GT_TRUE;
    }
    else
    {
        *pllStable = GT_FALSE;
    }

#endif
    return GT_OK;
}
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
/**
* @internal mvHwsAvagoSerdesSignalOkChange function
* @endinternal
*
* @brief   Per SERDES get indication check CDR lock and Signal
*          Detect change.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] signalOkChange                - TRUE if signal
*       detected and FALSE otherwise.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalOkChange
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    OUT GT_BOOL     *signalOkChange
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(signalOkChange);
#else

    GT_U32 data;

    RETURN_GT_OK_ON_EMULATOR;


    /* check CDR lock change and Signal Detect change on Serdes */

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_INTERRUPT_CAUSE_REG0, &data, (1 << 5)));
    if(data)
    {
        *signalOkChange = GT_TRUE;
    }
    else
    {
        *signalOkChange = GT_FALSE;
    }

#endif /* ASIC_SIMULATION */

    return GT_OK;
}
#endif


/**
* @internal mvHwsAvagoSerdesRxSignalCheck function
* @endinternal
*
* @brief   Per SERDES check there is Rx Signal and indicate if Serdes is ready for Tuning or not
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] rxSignal                 - Serdes is ready for Tuning or not
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesRxSignalCheck
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    OUT MV_HWS_AUTO_TUNE_STATUS     *rxSignal
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(rxSignal);
#else
    unsigned int chipIndex;
    GT_U32 sbus_addr;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* check if there is Rx Signal on Serdes */
    AVAGO_LOCK(devNum, chipIndex);
    *rxSignal = ((avago_serdes_mem_rd(aaplSerdesDb[chipIndex], sbus_addr, AVAGO_LSB, 0x027) & 0x0010) != 0) ? TUNE_READY : TUNE_NOT_READY;
    AVAGO_UNLOCK(devNum, chipIndex);
#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesReset function
* @endinternal
*
* @brief   Per SERDES Clear the serdes registers (back to defaults.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] analogReset              - Analog Reset (On/Off)
* @param[in] digitalReset             - digital Reset (On/Off)
* @param[in] syncEReset               - SyncE Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesReset
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_BOOL     analogReset,
    IN GT_BOOL     digitalReset,
    IN GT_BOOL     syncEReset
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(analogReset);
    GT_UNUSED_PARAM(digitalReset);
    GT_UNUSED_PARAM(syncEReset);
#else

  return(mvHwsAvagoSerdesResetImpl(devNum, portGroup, serdesNum, analogReset, digitalReset, syncEReset));
#endif /* ASIC_SIMULATION */

  return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesDigitalReset function
* @endinternal
*
* @brief   Run digital RESET/UNRESET (RF) on current SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] digitalReset             - digital Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesDigitalReset
(
    IN GT_U8           devNum,
    IN GT_UOPT         portGroup,
    IN GT_UOPT         serdesNum,
    IN MV_HWS_RESET    digitalReset
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(digitalReset);
#else

    GT_U32  data;

    /* SERDES digital RESET/UNRESET (RF) */
    data = (digitalReset == UNRESET) ? 1 : 0;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_1, (data << 3), (1 << 3)));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

extern GT_U32 hwsRavenDevBmp;

/**
* @internal mvHwsAvagoSerdesPolarityConfig function
* @endinternal
*
* @brief   Per serdes invert the Tx or Rx.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] invertTx                 - invert TX polarity (GT_TRUE - invert, GT_FALSE - don't)
* @param[in] invertRx                 - invert RX polarity (GT_TRUE - invert, GT_FALSE - don't)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPolarityConfig
(
    IN GT_U8   devNum,
    IN GT_UOPT portGroup,
    IN GT_UOPT serdesNum,
    IN GT_BOOL invertTx,
    IN GT_BOOL invertRx
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(invertTx);
    GT_UNUSED_PARAM(invertRx);
#else
if ((HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO) || ((HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO_16NM)
#if !defined(MV_HWS_REDUCED_BUILD) && defined(CHX_FAMILY)
    &&  (hwsRavenDevBmp & (1 << (mvHwsAvagoGetAaplChipIndex(devNum,serdesNum))))
#endif
    ))
    {
        return (mvHwsAvagoSerdesPolarityConfigImpl(devNum, portGroup, serdesNum, invertTx, invertRx));
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesPolarityConfigGet function
* @endinternal
*
* @brief   Per SERDES invert the Tx or Rx.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] invertTx                 - invert TX polarity (GT_TRUE - invert, GT_FALSE - don't)
* @param[out] invertRx                 - invert RX polarity (GT_TRUE - invert, GT_FALSE - don't)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPolarityConfigGet
(
    IN GT_U8             devNum,
    IN GT_UOPT           portGroup,
    IN GT_UOPT           serdesNum,
    OUT GT_BOOL           *invertTx,
    OUT GT_BOOL           *invertRx
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(invertTx);
    GT_UNUSED_PARAM(invertRx);
#else

    GT_U32 avagoInvertTx;
    GT_U32 avagoInvertRx;
    int res;

    res = mvHwsAvagoSerdesPolarityConfigGetImpl(devNum, portGroup, serdesNum,
                                                &avagoInvertTx, &avagoInvertRx);
    if(res != GT_OK)
    {
        HWS_AVAGO_DBG(("mvHwsAvagoSerdesPolarityConfigGetImpl failed (%d)\n", res));
        return GT_FAIL;
    }

    *invertTx = avagoInvertTx;
    *invertRx = avagoInvertRx;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesTemperatureTransmitSet function
* @endinternal
*
* @brief   Per SERDES transmit Tj
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] temperature              - temperature
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTemperatureTransmitSet
(
    IN GT_U8               devNum,
    IN GT_UOPT             portGroup,
    IN GT_UOPT             serdesNum,
    IN GT_32               temperature
)
{
#ifdef ASIC_SIMULATION
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(temperature);

#else
    GT_BOOL adaptive = GT_FALSE;
    GT_UNUSED_PARAM(adaptive);
    RETURN_GT_OK_ON_EMULATOR;

    if(temperature < MV_HWS_AVAGO_MIN_TEMEPRATURE)
    {
        temperature = MV_HWS_AVAGO_MIN_TEMEPRATURE;
    }
    else if(temperature > MV_HWS_AVAGO_MAX_TEMPERATURE)
    {
        temperature = MV_HWS_AVAGO_MAX_TEMPERATURE;
    }
#ifndef RAVEN_DEV_SUPPORT
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x2C, 0x2800, NULL));
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x6C, temperature, NULL));
#else
    CHECK_STATUS(mvHwsAvago16nmSerdesHalSet(devNum,serdesNum,HWS_SERDES_TEMPERATURE,0,temperature));
#endif
#endif
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesDfeConfig function
* @endinternal
*
* @brief   Per SERDES configure the DFE parameters.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] dfeMode                  - Enable/disable/freeze/Force
* @param[in] dfeCfg                   - array of dfe configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesDfeConfig
(
    IN GT_U8               devNum,
    IN GT_UOPT             portGroup,
    IN GT_UOPT             serdesNum,
    IN MV_HWS_DFE_MODE     dfeMode,
    IN GT_REG_DATA         *dfeCfg
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(dfeMode);
#else

    Avago_serdes_dfe_tune_t  dfe_state;
    GT_U32 sbus_addr;
    unsigned int chipIndex;

    RETURN_GT_OK_ON_EMULATOR;

    /* dfeCfg is not used in Avago serdes */
    dfeCfg = NULL;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* Initialize the dfe_state struct with default values */
    avago_serdes_tune_init(aaplSerdesDb[chipIndex], &dfe_state);
    CHECK_AVAGO_RET_CODE();

    switch (dfeMode)
    {
        case DFE_STOP_ADAPTIVE:
            dfe_state.tune_mode = AVAGO_DFE_STOP_ADAPTIVE;
            break;
        case DFE_START_ADAPTIVE:
            dfe_state.tune_mode = AVAGO_DFE_START_ADAPTIVE;
            break;
        case DFE_ICAL:
            /* Disable TAP1 before iCal */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, TAP1_AVG_DISABLE, NULL));
            dfe_state.tune_mode = AVAGO_DFE_ICAL;
            break;
        case DFE_PCAL:
            dfe_state.tune_mode = AVAGO_DFE_PCAL;
            break;

        case DFE_ICAL_VSR:
            /* Disable TAP1 before iCal */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, TAP1_AVG_DISABLE, NULL));
            dfe_state.tune_mode = AVAGO_DFE_ICAL;
            dfe_state.dfe_disable = GT_TRUE;
            break;
        case DFE_ICAL_BYPASS_CTLE:
            /* Disable TAP1 before iCal */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, TAP1_AVG_DISABLE, NULL));
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0xA, 0x781, NULL));
            break;
        default:
            return GT_BAD_PARAM;
    }


    AVAGO_LOCK(devNum, chipIndex);
    /* Run/Halt DFE tuning on a serdes based on dfe_tune_mode */
    avago_serdes_tune(aaplSerdesDb[chipIndex], sbus_addr, &dfe_state);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();


#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(dfeCfg);

    return GT_OK;
}

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined RAVEN_DEV_SUPPORT || defined FALCON_DEV_SUPPORT
/**
* @internal mvHwsAvago16nmSerdesDfeConfig function
* @endinternal
*
* @brief   Per SERDES configure the DFE parameters.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] dfeMode                  - Enable/disable/freeze/Force
* @param[in] dfeCfg                   - array of dfe configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvago16nmSerdesDfeConfig
(
    IN GT_U8               devNum,
    IN GT_UOPT             portGroup,
    IN GT_UOPT             serdesNum,
    IN MV_HWS_DFE_MODE     dfeMode,
    IN GT_REG_DATA         *dfeCfg
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(dfeMode);
    GT_UNUSED_PARAM(dfeCfg);
#else

    Avago_serdes_dfe_tune_t  dfe_state;
    GT_U32 sbus_addr;
    GT_U32 halData = 0;
    unsigned int chipIndex;

    RETURN_GT_OK_ON_EMULATOR;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* Initialize the dfe_state struct with default values */
    avago_serdes_tune_init(aaplSerdesDb[chipIndex], &dfe_state);
    CHECK_AVAGO_RET_CODE();

    switch (dfeMode)
    {
        case DFE_STOP_ADAPTIVE:
            dfe_state.tune_mode = AVAGO_DFE_STOP_ADAPTIVE;
            /* turn on fix rxFFE */
            halData = (HWS_PAM4_MODE_CHECK(*dfeCfg)) ? 0x18 : 0x19;
            CHECK_STATUS(mvHwsAvago16nmSerdesHalSet(devNum,serdesNum,HWS_SERDES_GLOBAL_TUNE_PARAMS,0x9,halData));
            break;

        case DFE_START_ADAPTIVE_FIXED_FFE:
            /* turn on fix rxFFE */
            halData = 0x18;
            GT_ATTR_FALLTHROUGH;
        case DFE_START_ADAPTIVE:
            dfe_state.tune_mode = AVAGO_DFE_START_ADAPTIVE;
            /* turn off fix rxFFE */
            halData |= (HWS_PAM4_MODE_CHECK(*dfeCfg)) ? 0x0 : 0x1;
            CHECK_STATUS(mvHwsAvago16nmSerdesHalSet(devNum,serdesNum,HWS_SERDES_GLOBAL_TUNE_PARAMS,0x9,halData));
            break;

        case DFE_PCAL_FIXED_FFE:
            /* turn on fix rxFFE */
            halData = 0x18;
            GT_ATTR_FALLTHROUGH;
        case DFE_PCAL:
            dfe_state.tune_mode = AVAGO_DFE_PCAL;
            /* turn off fix rxFFE */
            halData |= (HWS_PAM4_MODE_CHECK(*dfeCfg)) ? 0x0 : 0x1;
            CHECK_STATUS(mvHwsAvago16nmSerdesHalSet(devNum,serdesNum,HWS_SERDES_GLOBAL_TUNE_PARAMS,0x9,halData));
            break;

        case DFE_ICAL:
            dfe_state.tune_mode = AVAGO_DFE_ICAL;
            break;

        case DFE_ICAL_VSR:
            dfe_state.tune_mode = AVAGO_DFE_ICAL;
            dfe_state.dfe_disable = GT_TRUE;
            break;

        case DFE_ICAL_BYPASS_CTLE:
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0xA, 0x781, NULL));
            break;

        default:
            return GT_BAD_PARAM;
    }

    AVAGO_LOCK(devNum, chipIndex);
    /* Run/Halt DFE tuning on a serdes based on dfe_tune_mode */
    avago_serdes_tune(aaplSerdesDb[chipIndex], sbus_addr, &dfe_state);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();
#endif /* ASIC_SIMULATION */

    return GT_OK;
}
#endif
#if defined(MICRO_INIT) || ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
/**
* @internal mvHwsAvagoSerdesTestGen function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical Serdes number
* @param[in] txPattern                - pattern to transmit ("Other" means HW default - K28.5
*                                      [alternate running disparity])
* @param[in] mode                     - test  or normal
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTestGen
(
    IN GT_U8                       devNum,
    IN GT_UOPT                     portGroup,
    IN GT_UOPT                     serdesNum,
    IN MV_HWS_SERDES_TX_PATTERN    txPattern,
    IN MV_HWS_SERDES_TEST_GEN_MODE mode
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(txPattern);
    GT_UNUSED_PARAM(mode);
#else

    GT_U32 sbus_addr;
    unsigned int chipIndex;
    Avago_serdes_rx_cmp_data_t  rxPatternData = AVAGO_SERDES_RX_CMP_DATA_OFF;
    Avago_serdes_tx_data_sel_t  txPatternData = AVAGO_SERDES_TX_DATA_SEL_CORE;
    long tx_user[4] = {0};
    GT_UREG_DATA widthMode10Bit;

#ifndef ALDRIN_DEV_SUPPORT
    MV_HWS_AUTO_TUNE_STATUS rxStatus, rxStatusPrev=TUNE_PASS;
    GT_U32 rxStatusCnt = 0;
#endif

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* Read the saved value from AVAGO_SD_METAL_FIX register to check the Rx/Tx width (saved in mvHwsAvagoSerdesPowerCtrlImpl).
       PRBS test in GPCS modes can run only with Rx/Tx width=20BIT. */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_SD_METAL_FIX, &widthMode10Bit, (1 << 9)));
    if ((widthMode10Bit >> 9) == 1)
    {
        /* Change Rx/Tx width mode to 20BIT for GPCS modes */
        AVAGO_LOCK(devNum, chipIndex);
        avago_serdes_set_tx_rx_width(aaplSerdesDb[chipIndex], sbus_addr, 20, 20);
        AVAGO_UNLOCK(devNum, chipIndex);
        CHECK_AVAGO_RET_CODE();
    }
#ifndef ALDRIN_DEV_SUPPORT
    /* Stop adaptive before start PRBS */
    if (hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM)
    {   /* Check adaptive status*/
        CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesNum, &rxStatusPrev));
        if (TUNE_NOT_COMPLITED == rxStatusPrev) {
            AVAGO_LOCK(devNum, chipIndex);
            /* If not complete --> stop adaptive by spicoint 0xa data 0x0 */
            avago_spico_int(aaplSerdesDb[chipIndex], sbus_addr, 0xa, 0x0);
            AVAGO_UNLOCK(devNum, chipIndex);
            CHECK_AVAGO_RET_CODE();

            /* Check adaptive status and wait for complete */
            do
            {
                CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesNum, &rxStatus));
                ms_sleep(20);
            }
            while(((rxStatusCnt++) < 100) && (TUNE_NOT_COMPLITED == rxStatus));
        }
    }
#endif
    if (mode == SERDES_TEST)
    {
        switch (txPattern)
        {
            case _1T:
                tx_user[0] = 0xAAAAA;
                tx_user[1] = 0xAAAAA;
                tx_user[2] = 0xAAAAA;
                tx_user[3] = 0xAAAAA;
                break;
            case _2T:
                tx_user[0] = 0xCCCCC;
                tx_user[1] = 0xCCCCC;
                tx_user[2] = 0xCCCCC;
                tx_user[3] = 0xCCCCC;
                break;
            case _5T:
                tx_user[0] = 0x7C1F;
                tx_user[1] = 0x7C1F;
                tx_user[2] = 0x7C1F;
                tx_user[3] = 0x7C1F;
                break;
            case _10T:
                tx_user[0] = 0x3FF;
                tx_user[1] = 0x3FF;
                tx_user[2] = 0x3FF;
                tx_user[3] = 0x3FF;
                break;
            case PRBS7:
                txPatternData = AVAGO_SERDES_TX_DATA_SEL_PRBS7;
                rxPatternData = AVAGO_SERDES_RX_CMP_DATA_PRBS7;
                break;
            case PRBS9:
                txPatternData = AVAGO_SERDES_TX_DATA_SEL_PRBS9;
                rxPatternData = AVAGO_SERDES_RX_CMP_DATA_PRBS9;
                break;
            case PRBS11:
                txPatternData = AVAGO_SERDES_TX_DATA_SEL_PRBS11;
                rxPatternData = AVAGO_SERDES_RX_CMP_DATA_PRBS11;
                break;
            case PRBS15:
                txPatternData = AVAGO_SERDES_TX_DATA_SEL_PRBS15;
                rxPatternData = AVAGO_SERDES_RX_CMP_DATA_PRBS15;
                break;
            case PRBS23:
                txPatternData = AVAGO_SERDES_TX_DATA_SEL_PRBS23;
                rxPatternData = AVAGO_SERDES_RX_CMP_DATA_PRBS23;
                break;
            case PRBS31:
                txPatternData = AVAGO_SERDES_TX_DATA_SEL_PRBS31;
                rxPatternData = AVAGO_SERDES_RX_CMP_DATA_PRBS31;
                break;
            case PRBS13:
                txPatternData = AVAGO_SERDES_TX_DATA_SEL_PRBS13;
                rxPatternData = AVAGO_SERDES_RX_CMP_DATA_PRBS13;
                break;
            default:
                return GT_NOT_SUPPORTED;
        }

        /* USER modes txPattern: 1T, 2T, 5T, 10T */
        if (txPattern <= _10T)
        {
            AVAGO_LOCK(devNum, chipIndex);
            /* Sets the USER TX data source */
            avago_serdes_set_tx_data_sel(aaplSerdesDb[chipIndex], sbus_addr, AVAGO_SERDES_TX_DATA_SEL_USER);
            AVAGO_UNLOCK(devNum, chipIndex);
            CHECK_AVAGO_RET_CODE();

            AVAGO_LOCK(devNum, chipIndex);
            /* Sets the USER pattern to compare incoming data against, Auto-seed to received 40 bit repeating pattern */
            avago_serdes_set_rx_cmp_data(aaplSerdesDb[chipIndex], sbus_addr, AVAGO_SERDES_RX_CMP_DATA_SELF_SEED);
            AVAGO_UNLOCK(devNum, chipIndex);
            CHECK_AVAGO_RET_CODE();

            AVAGO_LOCK(devNum, chipIndex);
            /* Loads the 80-bit value into the TX user data register */
            avago_serdes_set_tx_user_data(aaplSerdesDb[chipIndex], sbus_addr, tx_user);
            AVAGO_UNLOCK(devNum, chipIndex);
            CHECK_AVAGO_RET_CODE();

            AVAGO_LOCK(devNum, chipIndex);
            /* reset counter */
            avago_serdes_error_reset(aaplSerdesDb[chipIndex], sbus_addr);
            AVAGO_UNLOCK(devNum, chipIndex);
            CHECK_AVAGO_RET_CODE();
        }
        else /* for PRBS modes */
        {
            AVAGO_LOCK(devNum, chipIndex);
            /* Sets the PRBS TX data source */
            avago_serdes_set_tx_data_sel(aaplSerdesDb[chipIndex], sbus_addr, txPatternData);
            AVAGO_UNLOCK(devNum, chipIndex);
            CHECK_AVAGO_RET_CODE();

            AVAGO_LOCK(devNum, chipIndex);
            /* Sets the PRBS pattern to compare incoming data against */
            avago_serdes_set_rx_cmp_data(aaplSerdesDb[chipIndex], sbus_addr, rxPatternData);
            AVAGO_UNLOCK(devNum, chipIndex);
            CHECK_AVAGO_RET_CODE();

            /* Sets the data comparisons (0x203): Turn on data comparison with Compare Sum and PRBS-generated data */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x3, 0x203, NULL));

            AVAGO_LOCK(devNum, chipIndex);
            /* reset counter */
            avago_serdes_error_reset(aaplSerdesDb[chipIndex], sbus_addr);
            AVAGO_UNLOCK(devNum, chipIndex);
            CHECK_AVAGO_RET_CODE();
        }
    }
    else /* for SERDES_NORMAL mode */
    {
        if ((widthMode10Bit >> 9) == 1)
        {
            /* Set back the Rx/Tx width mode to 10BIT if it was changed to 20BIT for GPCS modes */
            AVAGO_LOCK(devNum, chipIndex);
            avago_serdes_set_tx_rx_width(aaplSerdesDb[chipIndex], sbus_addr, 10, 10);
            AVAGO_UNLOCK(devNum, chipIndex);
            CHECK_AVAGO_RET_CODE();

            /* perform Serdes Digital Reset/Unreset */
            CHECK_STATUS(mvHwsSerdesDigitalReset(devNum, portGroup, serdesNum, HWS_DEV_SERDES_TYPE(devNum, serdesNum), RESET));

            /* delay 1ms */
            hwsOsExactDelayPtr(devNum, portGroup, 1);

            CHECK_STATUS(mvHwsSerdesDigitalReset(devNum, portGroup, serdesNum, HWS_DEV_SERDES_TYPE(devNum, serdesNum), UNRESET));
        }

        AVAGO_LOCK(devNum, chipIndex);
        /* Sets the TX data source */
        avago_serdes_set_tx_data_sel(aaplSerdesDb[chipIndex], sbus_addr, AVAGO_SERDES_TX_DATA_SEL_CORE);
        AVAGO_UNLOCK(devNum, chipIndex);
        CHECK_AVAGO_RET_CODE();

        AVAGO_LOCK(devNum, chipIndex);
        /* Sets the PRBS pattern to compare incoming data against */
        avago_serdes_set_rx_cmp_data(aaplSerdesDb[chipIndex], sbus_addr, AVAGO_SERDES_RX_CMP_DATA_OFF);
        AVAGO_UNLOCK(devNum, chipIndex);
        CHECK_AVAGO_RET_CODE();

#ifndef ALDRIN_DEV_SUPPORT
        if (hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM)
        {
            AVAGO_LOCK(devNum, chipIndex);
            avago_serdes_aux_counter_disable(aaplSerdesDb[chipIndex], sbus_addr);
            AVAGO_UNLOCK(devNum, chipIndex);
            CHECK_AVAGO_RET_CODE();
        }
#endif
    }

#ifndef ALDRIN_DEV_SUPPORT
    /* If previous ststus = TUNE_NOT_COMPLITED start adaptive by spicoint 0xa data 0x6 */
    if ((TUNE_NOT_COMPLITED == rxStatusPrev) || (hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM)) {
        AVAGO_LOCK(devNum, chipIndex);
        avago_spico_int(aaplSerdesDb[chipIndex], sbus_addr, 0xa, 0x6);
        ms_sleep(1);
        if (hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM)
        {
            /* Only the 16nm have the aux_counter that can be read during adaptive */
            avago_serdes_aux_counter_start(aaplSerdesDb[chipIndex], sbus_addr);
        }
        AVAGO_UNLOCK(devNum, chipIndex);
        CHECK_AVAGO_RET_CODE();

    }
#endif
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesTestGenGet function
* @endinternal
*
* @brief   Get configuration of the Serdes test generator/checker.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] txPatternPtr             - pattern to transmit ("Other" means any mode not
*                                      included explicitly in MV_HWS_SERDES_TX_PATTERN type)
* @param[out] modePtr                  - test mode or normal
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - unexpected pattern
* @retval GT_FAIL                  - HW error
*/
GT_STATUS mvHwsAvagoSerdesTestGenGet
(
    IN GT_U8                       devNum,
    IN GT_UOPT                     portGroup,
    IN GT_UOPT                     serdesNum,
    OUT MV_HWS_SERDES_TX_PATTERN    *txPatternPtr,
    OUT MV_HWS_SERDES_TEST_GEN_MODE *modePtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(txPatternPtr);
    GT_UNUSED_PARAM(modePtr);
#else

    GT_U32 sbus_addr;
    unsigned int chipIndex;
    Avago_serdes_tx_data_sel_t data_sel;
    long tx_user[4] = {0};

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    AVAGO_LOCK(devNum, chipIndex);
    data_sel = avago_serdes_get_tx_data_sel(aaplSerdesDb[chipIndex], sbus_addr);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();

    switch(data_sel)
    {
        case AVAGO_SERDES_TX_DATA_SEL_PRBS7:
            *txPatternPtr = PRBS7;
            break;
        case AVAGO_SERDES_TX_DATA_SEL_PRBS9:
            *txPatternPtr = PRBS9;
            break;
        case AVAGO_SERDES_TX_DATA_SEL_PRBS11:
            *txPatternPtr = PRBS11;
            break;
        case AVAGO_SERDES_TX_DATA_SEL_PRBS15:
            *txPatternPtr = PRBS15;
            break;
        case AVAGO_SERDES_TX_DATA_SEL_PRBS23:
            *txPatternPtr = PRBS23;
            break;
        case AVAGO_SERDES_TX_DATA_SEL_PRBS31:
            *txPatternPtr = PRBS31;
            break;
        case AVAGO_SERDES_TX_DATA_SEL_PRBS13:
            *txPatternPtr = PRBS13;
            break;

        case AVAGO_SERDES_TX_DATA_SEL_USER: /* User pattern generator */
            AVAGO_LOCK(devNum, chipIndex);
            avago_serdes_get_tx_user_data(aaplSerdesDb[chipIndex], sbus_addr, tx_user);
            AVAGO_UNLOCK(devNum, chipIndex);
            tx_user[0] = tx_user[0] & 0x3FF;
            tx_user[1] = (tx_user[1] >> 4) & 0x3FFF;

            if (tx_user[0] == 0x2AA)
            {
                *txPatternPtr = _1T;
            }
            else if (tx_user[0] == 0x333)
            {
                *txPatternPtr = _2T;
            }
            else if (tx_user[0] == 0x1F)
            {
                *txPatternPtr = _5T;
            }
            else if ((tx_user[0] == 0x3FF) && (tx_user[1] == 0x3FF0))
            {
                *txPatternPtr = _10T;
            }
            break;
        default:
            *txPatternPtr = Other;
            break;
    }

    *modePtr = (data_sel == AVAGO_SERDES_TX_DATA_SEL_CORE) ? SERDES_NORMAL : SERDES_TEST;
#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesTestGenStatus function
* @endinternal
*
* @brief   Read the tested pattern receive error counters and status.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txPattern                - pattern to transmit
* @param[in] counterAccumulateMode    - Enable/Disable reset the accumulation of error counters
*
* @param[out] status                   - test generator status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTestGenStatus
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          serdesNum,
    IN MV_HWS_SERDES_TX_PATTERN        txPattern,
    IN GT_BOOL                         counterAccumulateMode,
    OUT MV_HWS_SERDES_TEST_GEN_STATUS   *status
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(txPattern);
    GT_UNUSED_PARAM(counterAccumulateMode);
    hwsOsMemSetFuncPtr(status,0,sizeof(*status));
#else

    GT_U32 sbus_addr;
    GT_U32 data;
    MV_HWS_AUTO_TUNE_STATUS rxStatus;
    unsigned int chipIndex;

    /* avoid warning */
    GT_UNUSED_PARAM(txPattern);

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_EXTERNAL_STATUS, &data, (1 <<2)));

    CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesNum, &rxStatus));

    AVAGO_LOCK(devNum, chipIndex);
#ifndef ALDRIN_DEV_SUPPORT
    if ((hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM) && (rxStatus == TUNE_NOT_COMPLITED))
    {
        /* Retrieves the value of the error counter */
        status->errorsCntr = avago_serdes_aux_counter_read(aaplSerdesDb[chipIndex], sbus_addr);

        if (!counterAccumulateMode)
        {
            /* clear on read */
            avago_serdes_aux_counter_start(aaplSerdesDb[chipIndex], sbus_addr);
        }
    }
    else
#endif
    {
        /* Retrieves the value of the error counter */
        status->errorsCntr = avago_serdes_get_errors(aaplSerdesDb[chipIndex], sbus_addr, AVAGO_LSB, !counterAccumulateMode);
    }
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();

    status->lockStatus = (data) ? 0 : 1; /* if bit[2] is set then there is not signal (and vice versa) */
    status->txFramesCntr.l[0] = 0;
    status->txFramesCntr.l[1] = 0;

#endif /* ASIC_SIMULATION */

    return GT_OK;
}

#ifndef ALDRIN_DEV_SUPPORT
/**
* @internal mvHwsAvagoSerdesAuxCounterStart function
* @endinternal
*
* @brief   Start aux_counters.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAuxCounterStart
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          serdesNum
)
{
#ifndef ASIC_SIMULATION
    GT_U32 sbus_addr;
    unsigned int chipIndex;
    if (hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM)
    {
        chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
        CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

        AVAGO_LOCK(devNum, chipIndex);
        /* Start aux_counter */
        avago_serdes_aux_counter_start(aaplSerdesDb[chipIndex], sbus_addr);

        /* clear on read */
        avago_serdes_mem_wr(aaplSerdesDb[chipIndex], sbus_addr, AVAGO_LSB, 0x41, 0x8541);
        avago_serdes_mem_wr(aaplSerdesDb[chipIndex], sbus_addr, AVAGO_LSB, 0x41, 0x8540);

        AVAGO_UNLOCK(devNum, chipIndex);
        CHECK_AVAGO_RET_CODE();
    }
#else
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);

    return GT_OK;
}
#endif
#endif /* #if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */
/**
* @internal mvHwsAvagoSerdesLoopback function
* @endinternal
*
* @brief   Perform an Internal/External SERDES loopback mode for Debug use
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - physical Serdes number
* @param[in] lbMode    - loopback mode
*
* @retval 0 - on success
* @retval 1 - on error
*
* @note  - In 'AN_TX_RX' mode, the Rx & Tx Serdes Polarity
*          configurations should be disabled, for getting LinkUp
*          and Traffic on port.
*        - After return to 'Normal' mode, the Rx & Tx Serdes
*          Polarity configuration should be restored, for
*          re-synch again the PCS and MAC units and getting
*          LinkUp and Traffic on port.
*        - Switching between 'AN_TX_RX'/'DIG_RX_TX' modes
*          requires returning to the 'Normal' mode before.
*
*/
GT_STATUS mvHwsAvagoSerdesLoopback
(
    IN GT_U8             devNum,
    IN GT_UOPT           portGroup,
    IN GT_UOPT           serdesNum,
    IN MV_HWS_SERDES_LB  lbMode
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(lbMode);
#else

    GT_U32       sbus_addr;
    unsigned int chipIndex;
#ifndef ALDRIN_DEV_SUPPORT
    MV_HWS_SERDES_SPEED rate;
#endif
    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    switch(lbMode)
    {
        case SERDES_LP_DISABLE:
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x20, 0x20, NULL));
#ifndef ALDRIN_DEV_SUPPORT
            if (hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM)
            {
                CHECK_STATUS(mvHwsAvagoSerdesDigitalReset(devNum,portGroup,serdesNum,RESET));
            }
#endif
            /* set loopback_clock=ref_clock*/
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x30, 0, NULL));
#ifndef ALDRIN_DEV_SUPPORT
            if (hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM)
            {
                CHECK_STATUS(mvHwsAvagoSerdesDigitalReset(devNum,portGroup,serdesNum,UNRESET));
            }
#endif

            /* Disable Loopback mode*/
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x8, 0x100, NULL));
            if (hwsDeviceSpecInfo[devNum].serdesType != AVAGO_16NM)
            {
                /* CDR Rx power down */
                AVAGO_LOCK(devNum, chipIndex);
                avago_serdes_set_tx_rx_enable(aaplSerdesDb[chipIndex], sbus_addr, GT_TRUE, GT_FALSE, GT_TRUE);
                AVAGO_UNLOCK(devNum, chipIndex);

                /* CDR Rx power up for recalibration */
                AVAGO_LOCK(devNum, chipIndex);
                avago_serdes_set_tx_rx_enable(aaplSerdesDb[chipIndex], sbus_addr, GT_TRUE, GT_TRUE, GT_TRUE);
                AVAGO_UNLOCK(devNum, chipIndex);
            }
             AVAGO_LOCK(devNum, chipIndex);
             avago_serdes_set_tx_data_sel(aaplSerdesDb[chipIndex], sbus_addr, AVAGO_SERDES_TX_DATA_SEL_CORE);
             AVAGO_UNLOCK(devNum, chipIndex);

#ifndef ALDRIN_DEV_SUPPORT
            if (hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM)
            {
                /* operate SerDes in low power mode */
                CHECK_STATUS(mvHwsAvagoSerdesLowPowerModeEnable(devNum, portGroup, serdesNum, GT_TRUE));
            }
#endif

            break;
        case SERDES_LP_AN_TX_RX:
            /* set loopback_clock=ref_clock */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x30, 0, NULL));
            /* Internal Loopback mode */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x8, 0x101, NULL));
            if (hwsDeviceSpecInfo[devNum].serdesType != AVAGO_16NM)
            {
                /* CDR Rx power down */
                AVAGO_LOCK(devNum, chipIndex);
                avago_serdes_set_tx_rx_enable(aaplSerdesDb[chipIndex], sbus_addr, GT_TRUE, GT_FALSE, GT_TRUE);
                AVAGO_UNLOCK(devNum, chipIndex);

                /* CDR Rx power up for recalibration */
                AVAGO_LOCK(devNum, chipIndex);
                avago_serdes_set_tx_rx_enable(aaplSerdesDb[chipIndex], sbus_addr, GT_TRUE, GT_TRUE, GT_TRUE);
                AVAGO_UNLOCK(devNum, chipIndex);
            }

#ifndef ALDRIN_DEV_SUPPORT
            if (hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM)
            {
                /* operate SerDes in low power mode */
                CHECK_STATUS(mvHwsAvagoSerdesLowPowerModeEnable(devNum, portGroup, serdesNum, GT_FALSE));
            }
#endif
                /* disable EID */
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x20, 0x0, NULL));
            break;
        case SERDES_LP_DIG_RX_TX:
            /* set loopback and loopback_clock=CDR_clock */

            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x30, 0x10, NULL));

            if (HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO)
            {
#ifndef ALDRIN_DEV_SUPPORT
                CHECK_STATUS(mvHwsAvagoSerdeSpeedGet(devNum, portGroup, serdesNum, &rate));
                if ((rate == _25_78125G) || (rate == _27_5G) || (rate == _26_25G))
                {
                    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x8024, NULL));
                    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 0x184c, NULL));
                }
#endif

                /* CDR Rx power down */
                AVAGO_LOCK(devNum, chipIndex);
                avago_serdes_set_tx_rx_enable(aaplSerdesDb[chipIndex], sbus_addr, GT_TRUE, GT_FALSE, GT_TRUE);
                AVAGO_UNLOCK(devNum, chipIndex);

                /* CDR Rx power up for recalibration */
                AVAGO_LOCK(devNum, chipIndex);
                avago_serdes_set_tx_rx_enable(aaplSerdesDb[chipIndex], sbus_addr, GT_TRUE, GT_TRUE, GT_TRUE);
                AVAGO_UNLOCK(devNum, chipIndex);
            }

            AVAGO_LOCK(devNum, chipIndex);
            avago_serdes_set_tx_data_sel(aaplSerdesDb[chipIndex], sbus_addr, AVAGO_SERDES_TX_DATA_SEL_LOOPBACK);
            AVAGO_UNLOCK(devNum, chipIndex);
#ifndef ALDRIN_DEV_SUPPORT
            if (hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM)
            {
                /* add 100ms delay for 16nm */
                hwsOsExactDelayPtr(devNum, portGroup, 100);

                /* operate SerDes in low power mode */
                CHECK_STATUS(mvHwsAvagoSerdesLowPowerModeEnable(devNum, portGroup, serdesNum, GT_FALSE));

            }
#endif
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    CHECK_AVAGO_RET_CODE();
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesLoopbackGet function
* @endinternal
*
* @brief   Gets the status of Internal/External SERDES loopback mode.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical Serdes number
*
* @param[out] lbModePtr                - current loopback mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesLoopbackGet
(
    IN GT_U8             devNum,
    IN GT_UOPT           portGroup,
    IN GT_UOPT           serdesNum,
    OUT MV_HWS_SERDES_LB  *lbModePtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    *lbModePtr = SERDES_LP_DISABLE;
#else

    GT_U32 sbus_addr;
    unsigned int chipIndex;
    GT_BOOL internalLB;
    Avago_serdes_tx_data_sel_t  externalLB;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    AVAGO_LOCK(devNum, chipIndex);
    /* Get TRUE if internal loopback is enabled, FALSE if external signal */
    internalLB = avago_serdes_get_rx_input_loopback(aaplSerdesDb[chipIndex], sbus_addr);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();

    if (internalLB == GT_TRUE)
    {
        /* Internal Loopback mode */
        *lbModePtr = SERDES_LP_AN_TX_RX;
        return GT_OK;
    }

    AVAGO_LOCK(devNum, chipIndex);
    /* Get the selected TX data source */
    externalLB = avago_serdes_get_tx_data_sel(aaplSerdesDb[chipIndex], sbus_addr);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();

    /* Check the External Loopback or Disable mode */
    *lbModePtr = (externalLB == AVAGO_SERDES_TX_DATA_SEL_LOOPBACK) ? SERDES_LP_DIG_RX_TX : SERDES_LP_DISABLE;
#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesTxEnable function
* @endinternal
*
* @brief   Enable/Disable Tx.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] enable                   - whether to  or disable Tx.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxEnable
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_BOOL     enable
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(enable);
#else

    GT_U32 sbus_addr;
    unsigned int data;
    unsigned int chipIndex;

    RETURN_GT_OK_ON_EMULATOR;
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3

     if (prvCpssSystemRecoveryInProgress())
     {
         return GT_OK;
     }
#endif


    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* Semaphore protection should start before training-check function is performed and take place until
       tx-enable function is finised, to avoid situations where training-check function returned one status
       and by the time we will perform tx-enable, the training status will be changed by another CPU.*/
    AVAGO_LOCK(devNum, chipIndex);

    /* If training is in process, not performing tx enable */
    data = avago_serdes_pmd_status(aaplSerdesDb[chipIndex], sbus_addr);
    CHECK_AVAGO_RET_CODE_WITH_ACTION(AVAGO_UNLOCK(devNum, chipIndex));
    if (data == 0x2)
    {
        HWS_AVAGO_DBG(("mvHwsAvagoSerdesTxEnable failed because training is in process\n"));
        AVAGO_UNLOCK(devNum, chipIndex);
        return GT_NOT_READY;
    }

    /* Set the TX output Enabled/Disabled */
    avago_serdes_set_tx_output_enable(aaplSerdesDb[chipIndex], sbus_addr, enable);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();

#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesTxEnableGet function
* @endinternal
*
* @brief   Enable / Disable transmission of packets in SERDES layer of a port.
*         Use this API to disable Tx for loopback ports.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] enablePtr                - GT_TRUE  - Enable transmission of packets in
*                                      SERDES layer of a port
*                                      - GT_FALSE - Disable transmission of packets in
*                                      SERDES layer of a port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, serdesNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
*
* @note Disabling transmission of packets in SERDES layer of a port causes
*       to link down of devices that are connected to the port.
*
*/
GT_STATUS mvHwsAvagoSerdesTxEnableGet
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    OUT GT_BOOL     *enablePtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(enablePtr);
#else

    GT_U32 sbus_addr;
    unsigned int chipIndex;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    AVAGO_LOCK(devNum, chipIndex);
    /* Get the SERDES TX output mode: TRUE in enabled, FALSE otherwise */
    *enablePtr = avago_serdes_get_tx_output_enable(aaplSerdesDb[chipIndex], sbus_addr);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();
#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);

    return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsAvagoSerdesTxIfSelect function
* @endinternal
*
* @brief   tx interface select.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      enable    - whether to enable or disable Tx.
* @param[in] serdesTxIfNum            - number of serdes Tx
*       interface
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxIfSelect
(
    IN GT_U8           devNum,
    IN GT_UOPT         portGroup,
    IN GT_UOPT         serdesNum,
    IN GT_UREG_DATA    serdesTxIfNum
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(serdesTxIfNum);
#else

    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, serdesTxIfNum, (7 << 3)));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}
#endif
/**
* @internal mvHwsAvagoSerdesTxIfSelectGet function
* @endinternal
*
* @brief   Return the SERDES Tx interface select number.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      serdesType  - SERDES types
*
* @param[out] serdesTxIfNum            - interface select number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxIfSelectGet
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    OUT GT_U32      *serdesTxIfNum
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    *serdesTxIfNum = 0;
#else

    GT_UREG_DATA data;

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, &data, (7 << 3)));
    data = data >> 3;
    if (data > 7)
    {
        return GT_BAD_PARAM;
    }

    *serdesTxIfNum = data;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesAutoTuneStartExt function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] rxTraining               - Rx Training modes
* @param[in] txTraining               - Tx Training modes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneStartExt
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN MV_HWS_RX_TRAINING_MODES    rxTraining,
    IN MV_HWS_TX_TRAINING_MODES    txTraining
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(rxTraining);
    GT_UNUSED_PARAM(txTraining);
#else

    GT_U32 sbus_addr;
    Avago_serdes_dfe_tune_t   dfe_state;
    Avago_serdes_pmd_config_t pmd_mode;
    GT_BOOL signalDet = GT_FALSE;
    GT_BOOL isPam4 = GT_FALSE;
    GT_U32 profile = 0;
    unsigned int chipIndex;

    RETURN_GT_OK_ON_EMULATOR;

    if ((txTraining != IGNORE_TX_TRAINING) && (rxTraining != IGNORE_RX_TRAINING))
    {
        return GT_BAD_PARAM;
    }
    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* Initialize the dfe_state struct with default values */
    avago_serdes_tune_init(aaplSerdesDb[chipIndex], &dfe_state);
    CHECK_AVAGO_RET_CODE();

    /* Initialize the pmd_mode */
    avago_serdes_pmd_init(aaplSerdesDb[chipIndex], &pmd_mode);

    switch(rxTraining)
    {
        case IGNORE_RX_TRAINING:
            break;
        case START_CONTINUE_ADAPTIVE_TUNING:
            /* start continues adaptive tuning */
            dfe_state.tune_mode = AVAGO_DFE_START_ADAPTIVE;
            break;
        case STOP_CONTINUE_ADAPTIVE_TUNING:
            /* stop continues adaptive tuning */
            dfe_state.tune_mode = AVAGO_DFE_STOP_ADAPTIVE;
            break;
        case ONE_SHOT_DFE_TUNING:
            if (HWS_DEV_SERDES_TYPE(devNum, serdesNum) != AVAGO_16NM)
            {
                /* Disable TAP1 before iCal */
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, TAP1_AVG_DISABLE, NULL));
            }
            dfe_state.tune_mode = AVAGO_DFE_ICAL;
            break;
        case ONE_SHOT_DFE_VSR_TUNING:
            dfe_state.tune_mode = AVAGO_DFE_ICAL;
            dfe_state.dfe_disable = GT_TRUE;
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    if ((rxTraining != IGNORE_RX_TRAINING))
    {
        /* signal check - if currently no signal could be detected on the serdes, skippin the rx training */
        CHECK_STATUS(mvHwsAvagoSerdesSignalDetectGet(devNum, portGroup, serdesNum, &signalDet));
        if (signalDet == GT_TRUE)
        {
            AVAGO_LOCK(devNum, chipIndex);
            /* Launches and halts DFE tuning procedures */
            avago_serdes_tune(aaplSerdesDb[chipIndex], sbus_addr, &dfe_state);
            AVAGO_UNLOCK(devNum, chipIndex);
            CHECK_AVAGO_RET_CODE();
        }
        else
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ARG_STRING_MAC("Training can not be made when no signal is detected\n"));
        }
    }

    if (txTraining != IGNORE_TX_TRAINING)
    {
        pmd_mode.sbus_addr = sbus_addr;
        pmd_mode.disable_timeout = TRUE;
        pmd_mode.train_mode = (txTraining == START_TRAINING) ? AVAGO_PMD_TRAIN : AVAGO_PMD_BYPASS;
#if 0
        if (HWS_DEV_SERDES_TYPE(devNum) == AVAGO_16NM)
        {
            GT_BOOL      isPam4;
            /* Get PAM4 Eyes heights */
            AVAGO_LOCK(devNum, chipIndex);
            isPam4 = avago_serdes_get_rx_line_encoding(aaplSerdesDb[chipIndex], sbus_addr);
            AVAGO_UNLOCK(devNum, chipIndex);
            if ( isPam4) {
                pmd_mode.disable_timeout = FALSE ;
            }
        }
#endif
        if (HWS_DEV_SERDES_TYPE(devNum, serdesNum) != AVAGO_16NM)
        {
            /* Disable TAP1 before PMD training */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, TAP1_AVG_DISABLE, NULL));
            /* change IF_Dwell_Shift to 0x1 */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, 0x5C00, NULL));
        }

/* Read the divider value: */

#ifndef ALDRIN_DEV_SUPPORT
        if (HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO_16NM)
        {
            AVAGO_LOCK(devNum, chipIndex);
            isPam4 = avago_serdes_get_rx_line_encoding(aaplSerdesDb[chipIndex], sbus_addr);
            AVAGO_UNLOCK(devNum, chipIndex);
        }
#endif
        /* check profile */
        mvHwsAvagoSerdesManualInterconnectDBGet(devNum, portGroup, serdesNum, serdesNum, &profile);
        if (isPam4 )  /* only for falcon Pam4 speeds */
        {
            if ( 0 == profile )
        {
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x4, 0x312, NULL));
            }
            else
            {
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x4, 0x112, NULL));
            }
        }
        else
        {
            AVAGO_LOCK(devNum, chipIndex);
            avago_serdes_pmd_train(aaplSerdesDb[chipIndex], &pmd_mode);
            AVAGO_UNLOCK(devNum, chipIndex);
            CHECK_AVAGO_RET_CODE();
        }
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting.
*         This function calls to the mvHwsAvagoSerdesAutoTuneStartExt, which includes
*         all the functional options.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] rxTraining               - Rx Training (true (AVAGO_DFE_ICAL) /false)
* @param[in] txTraining               - Tx Training (true (AVAGO_PMD_TRAIN) /false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneStart
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_BOOL     rxTraining,
    IN GT_BOOL     txTraining
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(rxTraining);
    GT_UNUSED_PARAM(txTraining);
#else

    MV_HWS_RX_TRAINING_MODES    rxMode;
    MV_HWS_TX_TRAINING_MODES    txMode;

    if (txTraining && rxTraining)
    {
        return GT_BAD_PARAM;
    }

    /* for TRUE: set Single time iCal */
    rxMode = (rxTraining == GT_TRUE) ? ONE_SHOT_DFE_TUNING : IGNORE_RX_TRAINING;

    txMode = (txTraining == GT_TRUE) ? START_TRAINING : IGNORE_TX_TRAINING;

    CHECK_STATUS(mvHwsAvagoSerdesAutoTuneStartExt(devNum, portGroup, serdesNum, rxMode, txMode));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesRxAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] rxTraining               - Rx Training (true/false)
*                                      txTraining - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesRxAutoTuneStart
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_BOOL     rxTraining
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(rxTraining);
#else

    MV_HWS_RX_TRAINING_MODES    rxMode;

    /* for TRUE: set Single time iCal */
    rxMode = (rxTraining == GT_TRUE) ? ONE_SHOT_DFE_TUNING : IGNORE_RX_TRAINING;

    CHECK_STATUS(mvHwsAvagoSerdesAutoTuneStartExt(devNum, portGroup, serdesNum, rxMode, IGNORE_TX_TRAINING));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesTxAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*                                      rxTraining - Rx Training (true/false)
* @param[in] txTraining               - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxAutoTuneStart
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_BOOL     txTraining
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(txTraining);
#else

    MV_HWS_TX_TRAINING_MODES    txMode;

    /* for TRUE: set the AVAGO_PMD_TRAIN mode, for FALSE: set the AVAGO_PMD_BYPASS mode */
    txMode = (txTraining == GT_TRUE) ? START_TRAINING : STOP_TRAINING;

    CHECK_STATUS(mvHwsAvagoSerdesAutoTuneStartExt(devNum, portGroup, serdesNum, IGNORE_RX_TRAINING, txMode));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesAutoTuneStatus function
* @endinternal
*
* @brief   Per SERDES check the Rx & Tx training status
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] rxStatus                - Rx tune status
* @param[out] txStatus                - Tx tune status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneStatus
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    OUT MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    OUT MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(rxStatus);
    GT_UNUSED_PARAM(txStatus);
#else
    GT_U32 i;

    if ((NULL == rxStatus) && (NULL == txStatus))
    {
        return GT_BAD_PARAM;
    }

    if (rxStatus != NULL)
    {
        *rxStatus = TUNE_NOT_COMPLITED;

        for (i = 0; i < MV_AVAGO_TRAINING_TIMEOUT; i++)
        {
            /* Delay in 1ms */
            hwsOsExactDelayPtr(devNum, portGroup, 1);

            CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesNum, rxStatus));
            if (*rxStatus == TUNE_PASS)
                break;
        }

        /* in case training failed or took too long/short */
        if ((*rxStatus != TUNE_PASS) || (i == MV_AVAGO_TRAINING_TIMEOUT))
        {
            *rxStatus = TUNE_FAIL; /* else *rxStatus = TUNE_PASS */
        }
    }

    if (txStatus != NULL)
    {
        *txStatus = TUNE_NOT_COMPLITED;

        for (i = 0; i < MV_AVAGO_TRAINING_TIMEOUT; i++)
        {
            /* Delay in 1ms */
            hwsOsExactDelayPtr(devNum, portGroup, 1);

            CHECK_STATUS(mvHwsAvagoSerdesTxAutoTuneStatusShort(devNum, portGroup, serdesNum, txStatus));
            if (*txStatus == TUNE_PASS)
                break;
        }

        /* in case training failed or took too long/short */
        if ((*txStatus != TUNE_PASS) || (i == MV_AVAGO_TRAINING_TIMEOUT))
        {
            *txStatus = TUNE_FAIL; /* else *txStatus = TUNE_PASS */
        }
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesTxAutoTuneStop function
* @endinternal
*
* @brief   Per SERDES stop the TX training
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxAutoTuneStop
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  serdesNum
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
#else

    GT_U32 sbus_addr;
    unsigned int chipIndex;
    Avago_serdes_pmd_config_t pmd_mode;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* Initialize the pmd_mode */
    avago_serdes_pmd_init(aaplSerdesDb[chipIndex], &pmd_mode);

    pmd_mode.sbus_addr = sbus_addr;
    pmd_mode.disable_timeout = TRUE;
    pmd_mode.train_mode = AVAGO_PMD_RESTART;

    AVAGO_LOCK(devNum, chipIndex);
    /* stop the PMD link training procedure */
    avago_serdes_pmd_train(aaplSerdesDb[chipIndex], &pmd_mode);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();
#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesManualCtleConfig function
* @endinternal
*
* @brief   Set the Serdes Manual CTLE config for DFE
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] ctleConfigPtr            - CTLE Params
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualCtleConfig
(
    IN GT_U8                             devNum,
    IN GT_UOPT                           portGroup,
    IN GT_UOPT                           serdesNum,
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT  *rxConfigPtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(rxConfigPtr);
#else
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA *ctleConfigPtr = &rxConfigPtr->rxAvago;
    GT_U32 sbus_addr;
    unsigned int chipIndex;
    GT_U32  squelch;
    Avago_serdes_dfe_state_t    dfe;

    if(ctleConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }
    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);

    /* [0...308] */
    if ((ctleConfigPtr->squelch > 308) && (ctleConfigPtr->squelch != NA_16BIT))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ARG_STRING_MAC("Serdes squelch is Out of Range: [0...308]\n"));
    }

    /* [0...15] or NA_8BIT */
    if ((((ctleConfigPtr->bandWidth     & 0xFFF0) != 0) && (ctleConfigPtr->bandWidth     != NA_8BIT))||
        (((ctleConfigPtr->highFrequency & 0xFFF0) != 0) && (ctleConfigPtr->highFrequency != NA_8BIT))||
        (((ctleConfigPtr->lowFrequency  & 0xFFF0) != 0) && (ctleConfigPtr->lowFrequency  != NA_8BIT)))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ARG_STRING_MAC("Serdes CTLE BW/HF/LF is Out of Range: [0...15]\n"));
    }

    /* [0...255] or NA_8BIT */
    if (((ctleConfigPtr->dcGain & 0xFF00) != 0) && (ctleConfigPtr->dcGain != NA_8BIT))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ARG_STRING_MAC("Serdes CTLE dcGain is Out of Range: [0...255]\n"));
    }

#if !defined BOBK_DEV_SUPPORT && !defined ALDRIN_DEV_SUPPORT
    if (HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO_16NM)
    {
        /* [0...3] or NA_8BIT */
        if ((((ctleConfigPtr->gainshape1 & 0xFC) != 0) && (ctleConfigPtr->gainshape1 != NA_8BIT)) ||
            (((ctleConfigPtr->gainshape2 & 0xFC) != 0) && (ctleConfigPtr->gainshape2 != NA_8BIT)))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ARG_STRING_MAC("Serdes CTLE gainshape1/gainshape2 is Out of Range: [0...3]\n"));
        }

        /* [0...255] or NA_8BIT */
        if (((ctleConfigPtr->dfeGAIN != 0) && ctleConfigPtr->dfeGAIN  != NA_8BIT) ||
            ((ctleConfigPtr->dfeGAIN2 != 0) && ctleConfigPtr->dfeGAIN2 != NA_8BIT))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ARG_STRING_MAC("Serdes CTLE dfeGAIN/dfeGAIN2 is Out of Range: [0...255]\n"));
        }

        /* [0, 1] or NA_8BIT */
        if (((ctleConfigPtr->shortChannelEn & 0xFFFFFFFE) != 0) && (ctleConfigPtr->shortChannelEn != NA_8BIT))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ARG_STRING_MAC("Serdes CTLE shortChannelEn is Out of Range: [0...1]\n"));
        }
    }
    else
#endif
    if (HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO)
    {
        if ((ctleConfigPtr->gainshape1 != 0) ||
            (ctleConfigPtr->gainshape2 != 0))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("gainshape1/gainshape2 params are not supported for AVAGO 28nm\n"));
        }
        if ((ctleConfigPtr->dfeGAIN != 0) ||
            (ctleConfigPtr->dfeGAIN2 != 0))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("dfeGAIN/dfeGAIN2 params are not supported for AVAGO 28nm\n"));
        }
        if (ctleConfigPtr->shortChannelEn != 0)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("shortChannelEn param is not supported for AVAGO 28nm\n"));
        }
    }

    RETURN_GT_OK_ON_EMULATOR;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    dfe.bw               = ctleConfigPtr->bandWidth;
    dfe.dc               = ctleConfigPtr->dcGain;
    dfe.hf               = ctleConfigPtr->highFrequency;
    dfe.lf               = ctleConfigPtr->lowFrequency;
    dfe.gainshape1       = ctleConfigPtr->gainshape1;
    dfe.gainshape2       = ctleConfigPtr->gainshape2;
    dfe.dfeGAIN          = ctleConfigPtr->dfeGAIN;
    dfe.dfeGAIN2         = ctleConfigPtr->dfeGAIN2;
    dfe.short_channel_en = ctleConfigPtr->shortChannelEn;

    AVAGO_LOCK(devNum, chipIndex);
    serdes_dfe_update_ctle(aaplSerdesDb[chipIndex], sbus_addr, &dfe, 0x26);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();

    AVAGO_LOCK(devNum, chipIndex);
    serdes_dfe_update_gain(aaplSerdesDb[chipIndex], sbus_addr, &dfe, GT_TRUE);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();

    if (ctleConfigPtr->squelch != NA_16BIT)
    {
        /* formula to convert the milli-volt to fix value */
        squelch = (ctleConfigPtr->squelch < 68) ? 68 : ctleConfigPtr->squelch;
        squelch = (squelch - 68) / 16;

        /* Set the signal OK threshold on Serdes */
        CHECK_STATUS(mvHwsAvagoSerdesSignalOkCfg(devNum, portGroup, serdesNum, squelch));
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/*******************************************************************************
*
* @brief   Set the Low Power mode from Avago Serdes
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - physical serdes number*
* @param[in] enable    - enable/disable low power mode
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvagoSerdesLowPowerModeEnable
(
     IN GT_U8                   devNum,
     IN GT_UOPT                 portGroup,
     IN GT_UOPT                 serdesNum,
     IN GT_BOOL                 enableLowPower
)
{
#if MV_HWS_AVAGO_LOW_POWER_MODE_ENABLE_SKIP || defined(ASIC_SIMULATION)
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(enableLowPower);
#else
    GT_U32  sbus_addr;
    unsigned int chipIndex;
    GT_BOOL      isPam4;
    GT_32        data;

    /* avoid warning */
    GT_UNUSED_PARAM(portGroup);

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);

    if (hwsDeviceSpecInfo[devNum].serdesType != AVAGO_16NM)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("Serdes type is not supported"));
    }

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    AVAGO_LOCK(devNum, chipIndex);
    avago_serdes_enable_low_power_mode(aaplSerdesDb[chipIndex], sbus_addr, enableLowPower);
    isPam4 = avago_serdes_get_rx_line_encoding(aaplSerdesDb[chipIndex], sbus_addr);
    AVAGO_UNLOCK(devNum, chipIndex);

    if (( !isPam4 ) &&  (enableLowPower)){
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x4093,0, &data));
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x8093,(data|0x40),NULL));
    }
    CHECK_AVAGO_RET_CODE();
#endif /* MV_HWS_AVAGO_LOW_POWER_MODE_ENABLE_SKIP || defined(ASIC_SIMULATION) */
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesManualCtleConfigGet function
* @endinternal
*
* @brief   Get the Serdes CTLE (RX) configurations
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - physical lane number
*
* @param[out] configParams - pointer to Ctle params struct:
*               dcGain - DC-Gain value (rang: 0-255)
*               lowFrequency - CTLE Low-Frequency (rang: 0-15)
*               highFrequency - CTLE High-Frequency (rang: 0-15)
*               bandWidth - CTLE Band-width (rang: 0-15)
*               squelch - Signal OK threshold (rang: 0-308)
*               gainshape1 - CTLE gainshape1
*               gainshape2 - CTLE gainshape2
*               shortChannelEn - Enable/Disable Short channel
*               dfeGAIN - DFE Gain Tap strength (rang: 0-255)
*               dfeGAIN2 - DFE Gain Tap2 strength (rang: 0-255)
*
* @retval 0                        - on success
* @retval 1                        - on error
*
* @note The squelch transforms from units 0 - 15 to
*       units 0 - 308 by formula (x 16) + 68. So it's value can be not the same as it was set.
*       For example, was set the squelch = 110. It's transforms and safes inside as
*       integer part of (squelch - 68) / 16 = (integer part)(110 - 68) / 16 = 2
*       And we obtain after getting (2 16) + 68 = 100
*
*/
GT_STATUS mvHwsAvagoSerdesManualCtleConfigGet
(
    IN GT_U8                               devNum,
    IN GT_UOPT                             portGroup,
    IN GT_UOPT                             serdesNum,
    OUT MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    *configParams
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    hwsOsMemSetFuncPtr(configParams,0,sizeof(*configParams));
#else

    GT_U32 sbus_addr;
    unsigned int chipIndex;
    Avago_serdes_dfe_state_t    dfe;
    GT_UOPT     signalThreshold;

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if(hwsAvagoNotSupportedCheck())
    {
        hwsOsMemSetFuncPtr(configParams,0,sizeof(*configParams));
        /* no SERDESes on emulator */
        return GT_OK;
    }
#endif /* not MV_HWS_REDUCED_BUILD_EXT_CM3*/

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    hwsOsMemSetFuncPtr(&dfe,0,sizeof(dfe));

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    AVAGO_LOCK(devNum, chipIndex);
    avago_serdes_get_dfe_state(aaplSerdesDb[chipIndex], sbus_addr, &dfe);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();

    configParams->bandWidth      = dfe.bw;
    configParams->dcGain         = dfe.dc;
    configParams->highFrequency  = dfe.hf;
    configParams->lowFrequency   = dfe.lf;
    configParams->gainshape1     = dfe.gainshape1;
    configParams->gainshape2     = dfe.gainshape2;
    configParams->dfeGAIN        = dfe.dfeGAIN;
    configParams->dfeGAIN2       = dfe.dfeGAIN2;
    configParams->shortChannelEn = dfe.short_channel_en;

    /* Get the signal OK threshold on Serdes */
    CHECK_STATUS(mvHwsAvagoSerdesSignalOkThresholdGet(devNum, portGroup, serdesNum, &signalThreshold));
    configParams->squelch = (signalThreshold * 16) + 68;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/*******************************************************************************
* mvHwsAvagoSerdesEyeMetricGet
*
* DESCRIPTION:
*       Get the simple eye metric (mV) in range [0..1000]
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - serdes number
*
* OUTPUTS:
*       eoMatricValue - eye metric (mV) [0..1000]
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsAvagoSerdesEyeMetricGet
(
    IN GT_U8       devNum,
    IN GT_U8       portGroup,
    IN GT_U8       serdesNum,
    OUT GT_U32      *eoMatricValue
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(eoMatricValue);
#else

    GT_U32 sbus_addr;
    unsigned int chipIndex;

    RETURN_GT_OK_ON_EMULATOR;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    AVAGO_LOCK(devNum, chipIndex);
    *eoMatricValue = avago_serdes_eye_get_simple_metric(aaplSerdesDb[chipIndex], sbus_addr);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();
#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesSignalOkCfg function
* @endinternal
*
* @brief   Set the signal OK threshold on Serdes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] signalThreshold          - Signal OK threshold (0-15)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalOkCfg
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_UOPT     signalThreshold
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(signalThreshold);
#else

    GT_U32 sbus_addr;
    unsigned int chipIndex;

    RETURN_GT_OK_ON_EMULATOR;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    AVAGO_LOCK(devNum, chipIndex);
    avago_serdes_initialize_signal_ok(aaplSerdesDb[chipIndex], sbus_addr, signalThreshold);
    AVAGO_UNLOCK(devNum, chipIndex);

    CHECK_AVAGO_RET_CODE();
#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesSignalLiveDetectGet function
* @endinternal
*
* @brief   Per SERDES get live indication check CDR lock and Signal Detect.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] signalDet                - TRUE if signal detected and FALSE otherwise.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalLiveDetectGet
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    OUT GT_BOOL     *signalDet
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(signalDet);
#else

    GT_U32 sbus_addr;
    unsigned int chipIndex;
    GT_U32 i;

    RETURN_GT_OK_ON_EMULATOR;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* check CDR lock and Signal Detect on Serdes */
    AVAGO_LOCK(devNum, chipIndex);
    for(i=0; i < 10; i++)
    {
        if(FALSE == avago_serdes_get_signal_ok_live(aaplSerdesDb[chipIndex], sbus_addr))
        {
            AVAGO_UNLOCK(devNum, chipIndex);
            *signalDet = GT_FALSE;
            return GT_OK;
        }
    }
    AVAGO_UNLOCK(devNum, chipIndex);

    *signalDet = GT_TRUE;
#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesSignalOkThresholdGet function
* @endinternal
*
* @brief   Get the signal OK threshold on Serdes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] signalThreshold          - Signal OK threshold (0-15)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalOkThresholdGet
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    OUT GT_UOPT     *signalThreshold
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    *signalThreshold = 0;
#else

    GT_U32 sbus_addr;
    unsigned int chipIndex;

    RETURN_GT_OK_ON_EMULATOR;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    AVAGO_LOCK(devNum, chipIndex);
    *signalThreshold = avago_serdes_get_signal_ok_threshold(aaplSerdesDb[chipIndex], sbus_addr);
    AVAGO_UNLOCK(devNum, chipIndex);

    CHECK_AVAGO_RET_CODE();
#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesErrorInject function
* @endinternal
*
* @brief   Injects errors into the RX or TX data
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] numOfBits                - Number of error bits to inject (max=65535)
* @param[in] serdesDirection          - Rx or Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesErrorInject
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_UOPT     numOfBits,
    IN MV_HWS_SERDES_DIRECTION    serdesDirection
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(numOfBits);
    GT_UNUSED_PARAM(serdesDirection);
#else

    GT_U32 sbus_addr;
    unsigned int chipIndex;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    AVAGO_LOCK(devNum, chipIndex);
    (serdesDirection == RX_DIRECTION) ? avago_serdes_rx_inject_error(aaplSerdesDb[chipIndex], sbus_addr, numOfBits)
                                      : avago_serdes_tx_inject_error(aaplSerdesDb[chipIndex], sbus_addr, numOfBits);
    AVAGO_UNLOCK(devNum, chipIndex);

    CHECK_AVAGO_RET_CODE();
#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesShiftSamplePoint function
* @endinternal
*
* @brief   Shift Serdes sampling point earlier in time
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] delay                    - set the  (0-0xF)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesShiftSamplePoint
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_U32      delay
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(delay);
#else
    int y;
    int intValue;

    RETURN_GT_OK_ON_EMULATOR;

    y = (delay - 15);
    intValue = (y |(y << 4));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x402e, NULL));
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, intValue, NULL));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

#ifndef ASIC_SIMULATION
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT))
/**
* @internal mvHwsAvagoSerdesArrayAutoTuneSet function
* @endinternal
*
* @brief   Set iCAL(CTLE and DFE) or pCAL(DFE) Auto Tuning on multiple Serdeses
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] trainingMode             - for value 0: set iCAL mode,
*                                      for value 1: set pCAL mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAvagoSerdesArrayAutoTuneSet
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_BOOL     trainingMode
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum       = devNum;
    portGroup    = portGroup;
    serdesArr    = serdesArr;
    numOfSerdes  = numOfSerdes;
    GT_UNUSED_PARAM(trainingMode);
#else

    GT_U32  i, k, j;
    GT_U32  serdesArr_copy[MAX_AVAGO_SERDES_NUMBER] = {0};
    GT_U32  passCounter = 0;
    GT_U32  numOfActiveSerdes = 0;
    GT_BOOL signalDet = GT_FALSE;
    MV_HWS_AUTO_TUNE_STATUS     rxStatus;
    MV_HWS_DFE_MODE             dfeMode;

    /* copy the original Serdes array for saving the data */
    hwsOsMemCopyFuncPtr(serdesArr_copy, serdesArr, sizeof(GT_U32) * numOfSerdes);

    dfeMode = (trainingMode) ? DFE_PCAL : DFE_ICAL;

    for (j=0; j < numOfSerdes; j++)
    {
        if (serdesArr_copy[j] == SKIP_SERDES) /* skip Serdes[j] if No Signal-Detect */
            continue;

        /* signal check - if currently no signal could be detected on the serdes, skippin the training */
        CHECK_STATUS(mvHwsAvagoSerdesSignalDetectGet(devNum, portGroup, serdesArr_copy[j], &signalDet));
        if (signalDet == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Training can not be made when no signal is detected\n");
        }

        numOfActiveSerdes++;

        /* run iCAL(CTLE and DFE) or pCAL(DFE) */
        CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesArr_copy[j], dfeMode, NULL));
    }

    if (numOfActiveSerdes == 0)
    {
#ifndef MICRO_INIT
        HWS_AVAGO_DBG(("No need to run Training on Serdes\n"));
#endif
        return GT_OK;
    }

    for (k = 0; k < MV_AVAGO_TRAINING_TIMEOUT; k++)
    {
        for (i=0; i < numOfSerdes; i++)
        {
            if (serdesArr_copy[i] == SKIP_SERDES) /* if Serdes pass then no need to check status on it */
                continue;

            /* check that training completed */
            CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesArr_copy[i], &rxStatus));
            if (rxStatus == TUNE_PASS)
            {
                passCounter++;

                /* Stop traning if Timeout reached or if traning finished too fast */
                if ((k >= MV_AVAGO_TRAINING_TIMEOUT-1) || (k < 10))
                {
                    /* stop the training */
                    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr_copy[i], 0xa, 0x0, NULL));
                    HWS_AVAGO_DBG(("Error: Training failed. Stopped training k=%d\n", k));
                    return GT_TIMEOUT;
                }

                serdesArr_copy[i] = SKIP_SERDES;
            }
            else if ((dfeMode == DFE_ICAL) && (rxStatus == TUNE_FAIL))
            {
                serdesArr_copy[i] = SKIP_SERDES;
                return GT_FAIL;
            }
        }

        /* when all Serdeses finish, then stop the Timeout loop */
        if (passCounter == numOfActiveSerdes)
            break;
        else
            hwsOsExactDelayPtr(devNum, portGroup, 1); /* Delay of 1ms */
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}
#endif
#endif

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3

GT_STATUS mvHwsAvagoSerdesArrayAutoTuneSetByPhase
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_BOOL     trainingMode
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum       = devNum;
    portGroup    = portGroup;
    serdesArr    = serdesArr;
    numOfSerdes  = numOfSerdes;
    GT_UNUSED_PARAM(trainingMode);
#else

    GT_U32  j;
    GT_U32  serdesArr_copy[MAX_AVAGO_SERDES_NUMBER] = {0};
    GT_U32  numOfActiveSerdes = 0;
    GT_BOOL signalDet = GT_FALSE;


    /* copy the original Serdes array for saving the data */
    hwsOsMemCopyFuncPtr(serdesArr_copy, serdesArr, sizeof(GT_U32) * numOfSerdes);

    trainingMode = (trainingMode) ? DFE_PCAL : DFE_ICAL;

    for (j=0; j < numOfSerdes; j++)
    {
        if (serdesArr_copy[j] == SKIP_SERDES) /* skip Serdes[j] if No Signal-Detect */
            continue;

        /* signal check - if currently no signal could be detected on the serdes, skippin the training */
        CHECK_STATUS(mvHwsAvagoSerdesSignalDetectGet(devNum, portGroup, serdesArr_copy[j], &signalDet));
        if (signalDet == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Training can not be made when no signal is detected\n");
        }

        numOfActiveSerdes++;

        /* run iCAL(CTLE and DFE) or pCAL(DFE) */
        CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesArr_copy[j], trainingMode, NULL));
    }

    if (numOfActiveSerdes == 0)
    {
#ifndef MICRO_INIT
        HWS_AVAGO_DBG(("No need to run Training on Serdes\n"));
#endif
        return GT_OK;
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEnhaceTuneByPhaseInitDb function
* @endinternal
*
* @brief   Init Hws DB of enhance-tune-by-phase algorithm used by port manager.
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhaceTuneByPhaseInitDb
(
    IN GT_U8   devNum,
    IN GT_U32  phyPortNum
)
{
#ifdef ASIC_SIMULATION
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(phyPortNum);
#else
    GT_U32 i;

    hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->phase_CurrentDelaySize = 0;
    hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->phase_CurrentDelayPtr = 0;
    hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->phase_InOutI = 0;
    hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->phase_InOutKk = 0;
    hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->subPhase = 0;
    hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->waitForCoreReady = GT_FALSE;

    for (i=0; i<MV_HWS_MAX_LANES_NUM_PER_PORT; i++)
    {
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->phase_Best_LF[i] = 0;
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->phase_Best_eye[i] = 0;
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->phase_Best_dly[i] = 0;
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->phase_LF1_Arr[i]  = 0;
    }
#endif

    return GT_OK;
}
/**
* @internal mvHwsAvagoSerdesOneShotTuneByPhaseInitDb function
* @endinternal
*
* @brief   Init Hws DB of one shot -tune-by-phase algorithm for
* KR2/CR2 mode, used by port manager.
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesOneShotTuneByPhaseInitDb
(
    IN GT_U8   devNum,
    IN GT_U32  phyPortNum
)
{
#ifdef ASIC_SIMULATION
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(phyPortNum);
#else
    GT_U32 i;
    for (i=0; i<MV_HWS_MAX_LANES_NUM_PER_PORT; i++)
    {
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->oneShotTuneStcPtr->coreStatusReady[i] = GT_FALSE;
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesPrePostTuningByPhase function
* @endinternal
*
* @brief   reset/unreset CG_UNIT Rx GearBox Register, and clock
*          from SerDes to GearBox before running iCal.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] resetEnable              - reset True/False.
* @param[in] fecCorrect               - FEC type.
* @param[in] port                     - Port Mode.
* @param[out] coreStatusReady                 -  array that
*       indicates if the core status of all lanes is ready
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPrePostTuningByPhase
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      phyPortNum,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_BOOL     resetEnable,
    OUT GT_BOOL     *coreStatusReady,
    IN MV_HWS_PORT_FEC_MODE  fecCorrect,
    IN MV_HWS_PORT_STANDARD  portMode
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(phyPortNum);
    GT_UNUSED_PARAM(serdesArr);
    GT_UNUSED_PARAM(numOfSerdes);
    GT_UNUSED_PARAM(resetEnable);
    GT_UNUSED_PARAM(coreStatusReady);
    GT_UNUSED_PARAM(fecCorrect);
    GT_UNUSED_PARAM(portMode);
#else
    GT_U32                      i, regData;
    GT_BOOL  gearBoxRegAccess = GT_TRUE;

    /* 50/40G/52_5G KR2 NO FEC workaround */
    if((HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) || (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
    {
        if((fecCorrect == FEC_OFF) &&
          ((portMode == _50GBase_KR2) || (portMode == _40GBase_KR2) || (portMode == _50GBase_SR2) || (portMode == _50GBase_CR2) || (portMode == _52_5GBase_KR2)))
        {
            if(resetEnable == GT_FALSE)
            {
                for (i = 0; i < numOfSerdes; i++)
                {
                    if (coreStatusReady[i])
                    {
                        continue;
                    }
                    else
                    {
                        /* Poll for o_core_status[4] == 1 */
                        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, SERDES_UNIT, (serdesArr[i] & 0xFFFF), AVAGO_CORE_STATUS, &regData, 0));
                        if(((regData >> 4) & 1) != 1)
                        {
                            {
                                gearBoxRegAccess = GT_FALSE;
                                coreStatusReady[i] = GT_FALSE;
                                continue;
                            }
                        }
                        else
                        {
                            coreStatusReady[i] = GT_TRUE;
                        }
                    }
                }
            }

            if (gearBoxRegAccess == GT_TRUE)
            {
                regData = (resetEnable == GT_TRUE) ? 0 : 1;
                /* Disable rx_tlat - stop clock from SERDES to GearBox */
                for (i = 0; i < numOfSerdes; i++)
                {
                    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, SERDES_UNIT, (serdesArr[i] & 0xFFFF), SERDES_EXTERNAL_CONFIGURATION_0, (regData << 13), (1 << 13)));
                }

                /* Reset sd_rx_reset_ - GearBox RX reset*/
                if(phyPortNum % 4 == 0)
                {
                    regData = (resetEnable == GT_TRUE) ? 0x0 : 0x3;
                    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, CG_UNIT, (phyPortNum & HWS_2_LSB_MASK_CNS), CG_RESETS, regData, 0x3));
                }
                else if(phyPortNum % 2 == 0)
                {
                    regData = (resetEnable == GT_TRUE) ? 0x0 : 0xC;
                    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, CG_UNIT, (phyPortNum & HWS_2_LSB_MASK_CNS), CG_RESETS, regData, 0xC));
                }
                else
                {
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("illegal port num for 50G/40G WA"));
                }
            }
        }
    }
#endif
    return GT_OK;
}


/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase1 function
* @endinternal
*
* @brief   Move sample point and launch iCal. this is phase1 of the phased enhance-tune
*         algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] currentDelayPtr          - delays attay to set initial values with
* @param[in] currentDelaySize         - delay array size
* @param[in] best_eye                 -  array to set initial values with
*
* @param[out] currentDelayPtr          - delays attay to set initial values with
* @param[out] currentDelaySize         - delay array size
* @param[out] best_eye                 -  array to set initial values with
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase1
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    INOUT GT_U32      **currentDelayPtr,
    INOUT GT_U32      *currentDelaySize,
    INOUT GT_U32      *best_eye
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesArr);
    GT_UNUSED_PARAM(numOfSerdes);
    GT_UNUSED_PARAM(currentDelayPtr);
    GT_UNUSED_PARAM(currentDelaySize);
    GT_UNUSED_PARAM(best_eye);
#else

    unsigned int sbus_addr;
    unsigned int dwell;
    GT_U32  sdDetect;
    GT_U32  i;
    GT_U32  serdesArrValid[MAX_AVAGO_SERDES_NUMBER] = {0};
    MV_HWS_SERDES_SPEED rate;


    for (i=0; i < MV_HWS_MAX_LANES_NUM_PER_PORT; i++)
    {
        best_eye[i] = 4;
    }

    if (numOfSerdes > MV_HWS_MAX_LANES_NUM_PER_PORT)
    {
        HWS_AVAGO_DBG(("numOfSerdes %d is greater than MV_HWS_MAX_LANES_NUM_PER_PORT %d\n", numOfSerdes, MV_HWS_MAX_LANES_NUM_PER_PORT));
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsAvagoSerdeSpeedGet(devNum, portGroup, serdesArr[0], &rate));
    if(HWS_DEV_SILICON_TYPE(devNum) >= Bobcat3)
    {
        CHECK_STATUS(mvHwsAvagoSerdeSpeedGet(devNum, portGroup, serdesArr[0], &rate));
        if(rate >= _20_625G)
        {
            *currentDelayPtr = static_delay_25G;
            *currentDelaySize = sizeof(static_delay_25G)/sizeof(GT_U32);
        }
        else
        {
            *currentDelayPtr = static_delay_10G;
            *currentDelaySize = sizeof(static_delay_10G)/sizeof(GT_U32);
        }
    }
    else
    {
        *currentDelayPtr = static_delay_10G;
        *currentDelaySize = sizeof(static_delay_10G)/sizeof(GT_U32);
    }

    /* enlarge Dwell time */
    dwell = avagoBaudRate2DwellTimeValue[rate];
    for (i=0; i< numOfSerdes; i++)
    {
            /* select DFE tuning dwell time */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x18, 0x7, NULL));
            /* big register write LSB */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, 1 << dwell, NULL));
            /* big register write MSB */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, 0, NULL));
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 1 ## \n");
    /* check CDR lock and Signal Detect on all Serdeses */
    for (i=0; i < numOfSerdes; i++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 2 ## serdesNum=%d\n", MV_HWS_SERDES_NUM(serdesArr[i]));
        CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, MV_HWS_SERDES_NUM(serdesArr[i]), &sbus_addr));

        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, MV_HWS_SERDES_NUM(serdesArr[i]),
                                            AVAGO_EXTERNAL_STATUS, &sdDetect, (1 << 2)));
        if (sdDetect != 0)
        {
            serdesArr[i] = SKIP_SERDES; /* change the Serdes number to mark this array's element as not relevant */
            continue;
        }

        /* set SamplePoint to default value (delay=0) */
        CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[i], (*currentDelayPtr)[0]) );
    }

    /* copy the original Serdes array for saving the data */
    hwsOsMemCopyFuncPtr(serdesArrValid, serdesArr, sizeof(GT_U32) * numOfSerdes);

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 3 ## \n");
    /* run iCAL(CTLE and DFE) */
    CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSetByPhase(devNum, portGroup, serdesArr, numOfSerdes, ICAL_TRAINING_MODE));
#endif
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase2 function
* @endinternal
*
* @brief   Getting LF values from serdeses. this is phase2 of the phased enhance-tune
*         algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] best_LF                  - array to be filled with LF values from serdes
*
* @param[out] best_LF                  - array to be filled with LF values from serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase2
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_U32      *best_LF
)
{

#ifdef ASIC_SIMULATION
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesArr);
    GT_UNUSED_PARAM(numOfSerdes);
    GT_UNUSED_PARAM(best_LF);
#else
    int LF;
    GT_U8 k;
    if ( numOfSerdes >= MV_HWS_MAX_LANES_NUM_PER_PORT )
    {
        return GT_FAIL;
    }
    /* save the LF value of all Serdeses */
    for (k=0; k < numOfSerdes; k++)
    {
        if (serdesArr[k] == SKIP_SERDES) /* No Signal-Detect on this Serdes */
            continue;

        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 4 ## serdesNum=%d\n", serdesArr[k]);
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[k], 0x126, ((2 << 12) | (1 << 8)), &LF));
        best_LF[k] = LF;
    }
#endif
    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 5 ## \n");
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase3 function
* @endinternal
*
* @brief   Move sample point, then launch pCal than calculate eye. this is phase3
*         of the phased enhance-tune algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] currentDelaySize         - delays array size
* @param[in] currentDelayPtr          - delays array
* @param[in] inOutI                   - iteration index between different phases
* @param[in] best_LF                  - best LF array to update
* @param[in] best_eye                 - best eyes array to update
* @param[in] best_dly                 - best delays array to update
* @param[in] subPhase                 - assist flag to know which code to execute in this sub-phase
*
* @param[out] inOutI                   - iteration index between different phases
* @param[out] best_LF                  - best LF array to update
* @param[out] best_eye                 - best eyes array to update
* @param[out] best_dly                 - best delays array to update
* @param[out] subPhase                 - assist flag to know which code to execute in this sub-phase
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase3
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_U32      currentDelaySize,
    IN GT_U32      *currentDelayPtr,
    INOUT GT_U32      *inOutI,
    INOUT GT_U32      *best_LF,
    INOUT GT_U32      *best_eye,
    INOUT GT_U32      *best_dly,
    INOUT GT_U8       *subPhase
)
{

#ifdef ASIC_SIMULATION
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesArr);
    GT_UNUSED_PARAM(numOfSerdes);
    GT_UNUSED_PARAM(currentDelaySize);
    GT_UNUSED_PARAM(currentDelayPtr);
    GT_UNUSED_PARAM(inOutI);
    GT_UNUSED_PARAM(best_LF);
    GT_UNUSED_PARAM(best_eye);
    GT_UNUSED_PARAM(best_dly);
    GT_UNUSED_PARAM(subPhase);
#else
    GT_U32  eye;
    int LF;
    GT_U32 i,k, j;
    unsigned int sbus_addr;
    unsigned int chipIndex;
    GT_U32      dly;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesArr[0]);
    /* find the peak of the eye accoding to delay */
    for (i=(*inOutI); i < currentDelaySize; i++)
    {
        if (*subPhase == 0)
        {
            AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 6 ## \n");
            dly = currentDelayPtr[i];

            for (k=0; k < numOfSerdes; k++)
            {
                AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 7 ## serdesNum=%d\n", serdesArr[k]);
                if (serdesArr[k] == SKIP_SERDES) /* No Signal-Detect on this Serdes */
                    continue;

                CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[k], dly));
            }

            /* trigger pCAL(DFE) on all relevant Serdeses */
            CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSetByPhase(devNum, portGroup, serdesArr, numOfSerdes, PCAL_TRAINING_MODE));
            *subPhase = 1;
            break;
        }
        else
        {
            /**********
              ************
              ***********
              ****************/

            for (j=0; j < numOfSerdes; j++)
            {
                dly = currentDelayPtr[i];

                if (serdesArr[j] == SKIP_SERDES)
                    continue;

                AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 8 ## serdesNum=%d\n", serdesArr[j]);
                CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesArr[j], &sbus_addr));

                AVAGO_LOCK(devNum, chipIndex);

                eye = avago_serdes_eye_get_simple_metric(aaplSerdesDb[chipIndex], sbus_addr);
                AVAGO_UNLOCK(devNum, chipIndex);
                CHECK_AVAGO_RET_CODE();

                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[j], 0x126, ((2 << 12) | (1 << 8)), &LF));

                if (eye > (best_eye[j] + MV_AVAGO_EYE_TAP))
                {
                    best_eye[j] = eye;
                    best_dly[j] = dly;

                    if (LF >= (int)best_LF[j])
                        best_LF[j] = LF;
                }
    #ifndef MICRO_INIT
                AVAGO_DBG(("current_eye[%d]=%d dly[%d]=%d\n", j, eye, j, dly));
    #endif
                if (best_eye[j] < MV_AVAGO_MIN_EYE_SIZE)
                {
                    HWS_AVAGO_DBG(("Error on Serdes %d: Eye is too small (size is %d). Training Failed\n", serdesArr[j], best_eye[j]));
                    return GT_FAIL;
                }
            }
            /**********
            ************
            ***********
            ****************/
            *subPhase = 0;
            /*
            i++;
            break;*/
        }
    }

    if (i<currentDelaySize)
    {
        (*inOutI) = i;
    }
    else
    {
        (*inOutI) = 0xFFFFFFFF;
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase4 function
* @endinternal
*
* @brief   Move sample point to the best delay value that was found in previous phase
*         then launch iCal. this is phase4 of the phased enhance-tune algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] best_dly                 - best delay to set the serdes with
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase4
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_U32      *best_dly
)
{
#ifdef ASIC_SIMULATION

    devNum      =devNum     ;
    portGroup   =portGroup  ;
    serdesArr   =serdesArr  ;
    numOfSerdes =numOfSerdes;
    best_dly    =best_dly   ;
#else
    GT_U32 k;

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 9 ## \n");
    for (k=0; k < numOfSerdes; k++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 10 ## serdesNum=%d\n", serdesArr[k]);
        if (serdesArr[k] == SKIP_SERDES) /* skip Serdes if the dly is the same as the best_dly */
            continue;

        CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[k], best_dly[k]));
    }

    /* run iCAL(CTLE and DFE) with best delay */
    CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSetByPhase(devNum, portGroup, serdesArr, numOfSerdes, ICAL_TRAINING_MODE));
#endif
    return GT_OK;

}

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase5 function
* @endinternal
*
* @brief   Get LF values from serdes then launch iCal. this is phase5 of the phased
*         enhance-tune algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] LF1_Arr                  - LF array to fill with values
*
* @param[out] LF1_Arr                  - LF array to fill with values
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase5
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    INOUT int         *LF1_Arr
)
{
#ifdef ASIC_SIMULATION
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesArr);
    GT_UNUSED_PARAM(numOfSerdes);
    GT_UNUSED_PARAM(LF1_Arr);
#else
    GT_U32 k;
    GT_U32  serdesArrValid[MAX_AVAGO_SERDES_NUMBER] = {0};

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 11 ## \n");
    /* copy the original Serdes array for saving the data */
    hwsOsMemCopyFuncPtr(serdesArrValid, serdesArr, sizeof(GT_U32) * numOfSerdes);

    for (k=0; k < numOfSerdes; k++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 12 ## serdesNum=%d\n", serdesArrValid[k]);
        /* read the LF value */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x126, ((2 << 12) | (1 << 8)), &LF1_Arr[k]));
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 13 ## \n");
    /* run iCAL(CTLE and DFE) with best delay */
    CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSetByPhase(devNum, portGroup, serdesArr, numOfSerdes, ICAL_TRAINING_MODE));
#endif
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase6 function
* @endinternal
*
* @brief   Read LF and Gain from serdes after iCal was launched in previous phase,
*         than calculate best LF and Gain, write to serdes and launch pCal.
*         this is phase6 of the phased enhance-tune algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] LF1_Arr                  - LF array to fill with values
* @param[in] min_LF                   - minimum LF for reference
* @param[in] max_LF                   - maimum LF for reference
* @param[in] LF1_Arr                  - previous LF values for reference
* @param[in] best_eye                 - besy eye for printing purpose
* @param[in] best_dly                 - besy delay for printing purpose
* @param[in] inOutKk                  - iterator index
* @param[in] continueExecute          - whether or not to continue algorithm execution
*
* @param[out] inOutKk                  - iterator index
* @param[out] continueExecute          - whether or not to continue algorithm execution
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase6
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_U8       min_LF,
    IN GT_U8       max_LF,
    IN int         *LF1_Arr,
    IN GT_U32      *best_eye,
    IN GT_U32      *best_dly,
    INOUT GT_U8       *inOutKk,
    INOUT GT_BOOL     *continueExecute
)
{
#ifdef ASIC_SIMULATION
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesArr);
    GT_UNUSED_PARAM(numOfSerdes);
    GT_UNUSED_PARAM(min_LF);
    GT_UNUSED_PARAM(max_LF);
    GT_UNUSED_PARAM(LF1_Arr);
    GT_UNUSED_PARAM(inOutKk);
    GT_UNUSED_PARAM(continueExecute);
#else
    GT_U32  serdesArrValid[MAX_AVAGO_SERDES_NUMBER] = {0};
    int LF2_Arr[MAX_AVAGO_SERDES_NUMBER]  = {0};
    int DFEgain_Arr[MAX_AVAGO_SERDES_NUMBER]    = {0};
    unsigned int LF, GAIN;
    GT_U32 k;

    /* copy the original Serdes array for saving the data */
    hwsOsMemCopyFuncPtr(serdesArrValid, serdesArr, sizeof(GT_U32) * numOfSerdes);

    for (k=*inOutKk; k < numOfSerdes; k++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 14 ## serdesNum=%d\n", serdesArrValid[k]);
        /* read again the LF value after run the iCAL */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x126, ((2 << 12) | (1 << 8)), &LF2_Arr[k]));

        /* read the DFE gain */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x126, ((3 << 12) | (0 << 8)), &DFEgain_Arr[k]));

        LF = (LF1_Arr[k] + LF2_Arr[k])/2; /* take the avarge from two tunning values */
        LF = (LF < min_LF) ? min_LF : LF;
        LF = (LF > max_LF) ? max_LF : LF;

        /* write the selected LF value back to Serdes */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x26, ((2 << 12) | (1 << 8) | LF), NULL));

        GAIN = (DFEgain_Arr[k] > 1) ? (DFEgain_Arr[k]-1) : DFEgain_Arr[k];
        /* write the selected GAIN value back to Serdes */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x26, ((3 << 12) | (0 << 8) | GAIN), NULL));
#ifndef MICRO_INIT
        AVAGO_DBG(("Serdes %d: setting best_dly=%d best_eye=%d LF=%d GAIN=%d\n", serdesArrValid[k], best_dly[k], best_eye[k], LF, GAIN));
#endif

        /* run  pCAL(DFE) */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup,  serdesArrValid[k], 0xa, 2, NULL));

        /*k++;
        break;*/
    }

    if (k<numOfSerdes)
    {
        *continueExecute = GT_TRUE;
    }
    else
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 15 ## \n");
        *continueExecute = GT_FALSE;
    }
#endif
    GT_UNUSED_PARAM(best_eye);
    GT_UNUSED_PARAM(best_dly);
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesScanLowFrequency function
* @endinternal
*
* @brief   Scan Low frequency algorithm
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
 *@param[in] phyPortNum               - physical port number
 *@param[in] portMode                 - HWS port mode
* @param[in] serdesArr                - array of serdes's
* @param[in] numOfSerdes              - number of serdes's
* @param[in] minlF                    - minimum LF for reference
* @param[in] maxLf                    - maximum LF for reference
* @param[in] hf                       - HF for reference
 *@param[in] bw                       - BW for reference
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesScanLowFrequency
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_U32       minLf,
    IN GT_U32       maxLf,
    IN GT_U32       hf,
    IN GT_U32       bw
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(phyPortNum);
    GT_UNUSED_PARAM(portMode);
    GT_UNUSED_PARAM(serdesArr);
    GT_UNUSED_PARAM(numOfSerdes);
    GT_UNUSED_PARAM(minLf);
    GT_UNUSED_PARAM(maxLf);
    GT_UNUSED_PARAM(hf);
    GT_UNUSED_PARAM(bw);
#else
    GT_U32 i, j;
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    ctleData[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_BOOL  lfCalibrated[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_BOOL  hfCalibrated[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_BOOL  bwCalibrated[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U32   bestEye[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U32   bestLf[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_UOPT  numOfCalLanes = 0;
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT results;
    hwsOsMemSetFuncPtr(ctleData,0,sizeof(ctleData));
    hwsOsMemSetFuncPtr(lfCalibrated,0,sizeof(lfCalibrated));
    hwsOsMemSetFuncPtr(hfCalibrated,0,sizeof(hfCalibrated));
    hwsOsMemSetFuncPtr(bwCalibrated,0,sizeof(bwCalibrated));
    hwsOsMemSetFuncPtr(bestEye,0,sizeof(bestEye));
    hwsOsMemSetFuncPtr(bestLf,0,sizeof(bestLf));
    hwsOsMemSetFuncPtr(&results,0,sizeof(results));

    /* Get ctle data from HW */
    CHECK_STATUS(mvHwsPortManualCtleConfigGet(devNum, portGroup, phyPortNum, portMode, 0xFF, ctleData));

    /* reach HighFrequency and BandWidth values to the wanted point, by fine tuning */
    while (numOfCalLanes != numOfSerdes)
    {
        for (i = 0; i < numOfSerdes; i++)
        {
            if ((hfCalibrated[i]) && (bwCalibrated[i]))
            {
                continue;
            }

            if (ctleData[i].highFrequency != hf)
            {
                if (ctleData[i].highFrequency < hf)
                {
                    ctleData[i].highFrequency++;
                }
                else
                {
                    ctleData[i].highFrequency--;
                }
            }
            else
            {
                hfCalibrated[i] = GT_TRUE;
            }
            if (ctleData[i].bandWidth != bw)
            {
                if (ctleData[i].bandWidth < bw)
                {
                    ctleData[i].bandWidth++;
                }
                else
                {
                    ctleData[i].bandWidth--;
                }
            }
            else
            {
                bwCalibrated[i] = GT_TRUE;
            }

            if ((bwCalibrated[i]) && (hfCalibrated[i]))
            {
                numOfCalLanes++;
            }
        }
        /* Write ctle data to HW */
        CHECK_STATUS(mvHwsPortManualCtleConfig(devNum, portGroup, phyPortNum, portMode, 0xFF, ctleData));
        /* 5ms delay */
        hwsOsExactDelayPtr(devNum, portGroup, 5);
        i = 0;
    }

    /* Get ctle data from HW */
    CHECK_STATUS(mvHwsPortManualCtleConfigGet(devNum, portGroup, phyPortNum, portMode, 0xFF, ctleData));
    numOfCalLanes = 0;

    /* reach LowFrequency value to the wanted point, by fine tuning */
    while (numOfCalLanes != numOfSerdes)
    {
        for (i = 0; i < numOfSerdes; i++)
        {
            if (lfCalibrated[i])
            {
                continue;
            }

            if (minLf != ctleData[i].lowFrequency)
            {
                if (ctleData[i].lowFrequency < minLf)
                {
                    ctleData[i].lowFrequency++;
                }
                else
                {
                    ctleData[i].lowFrequency--;
                }
            }
            else
            {
                lfCalibrated[i] = GT_TRUE;
                numOfCalLanes++;
            }
        }
        /* Write ctle data to HW */
        CHECK_STATUS(mvHwsPortManualCtleConfig(devNum, portGroup, phyPortNum, portMode, 0xFF, ctleData));
        /* 5ms delay */
        hwsOsExactDelayPtr(devNum, portGroup, 5);
        i = 0;
    }
    /* Get ctle data from HW */
    CHECK_STATUS(mvHwsPortManualCtleConfigGet(devNum, portGroup, phyPortNum, portMode, 0xFF, ctleData));
    /* Get EO value from HW for each Low Frequency value to find the best eye */
    for (j = minLf; j <= maxLf; j++)
    {
        /* 250ms Delay - time takes to EO to update after writing to ctle register */
        hwsOsExactDelayPtr(devNum, portGroup, 250);
        for (i = 0; i < numOfSerdes; i++)
        {
            CHECK_STATUS(mvHwsAvagoSerdesAutoTuneResult(devNum,portGroup,serdesArr[i], &results));
            if (results.avagoResults.EO > (bestEye[i] + 4))
            {
                bestEye[i] = results.avagoResults.EO;
                bestLf[i] =  results.avagoResults.LF;
            }
            if (ctleData[i].lowFrequency == maxLf)
            {
                continue;
            }
            else
            {
                ctleData[i].lowFrequency++;
            }
        }
        CHECK_STATUS(mvHwsPortManualCtleConfig(devNum, portGroup, phyPortNum, portMode, 0xFF, ctleData));
    }

    /* 5ms delay to ensure last call is updated */
    hwsOsExactDelayPtr(devNum, portGroup, 5);
    /* Get ctle values from HW */
    CHECK_STATUS(mvHwsPortManualCtleConfigGet(devNum, portGroup, phyPortNum, portMode, 0xFF, ctleData));
    numOfCalLanes = 0;
    hwsOsMemSetFuncPtr(lfCalibrated,0,sizeof(lfCalibrated));

    /* Bring the Low Frequency value to the best one the found prior by fine tuning */
    while (numOfCalLanes != numOfSerdes)
    {
        for (i = 0; i < numOfSerdes; i++)
        {
            if (lfCalibrated[i])
            {
                continue;
            }

            if (bestLf[i] != ctleData[i].lowFrequency)
            {
                if (ctleData[i].lowFrequency < bestLf[i])
                {
                    ctleData[i].lowFrequency++;
                }
                else
                {
                    ctleData[i].lowFrequency--;
                }
            }
            else
            {
                lfCalibrated[i] = GT_TRUE;
                numOfCalLanes++;
            }
        }
        /* write ctle data to HW */
        CHECK_STATUS(mvHwsPortManualCtleConfig(devNum, portGroup, phyPortNum, portMode, 0xFF, ctleData));
        /* 5ms delay */
        hwsOsExactDelayPtr(devNum, portGroup, 5);
    }

#endif /* ASIC_SIMULATION */

    return GT_OK;
}

#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT))
/**
* @internal mvHwsAvagoSerdesEnhanceTune function
* @endinternal
*
* @brief   Set the ICAL with shifted sampling point to find best sampling point
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] min_LF                   - Minimum LF value that can be set on Serdes (0...15)
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes (0...15)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTune
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_U8       min_LF,
    IN GT_U8       max_LF
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesArr);
    GT_UNUSED_PARAM(numOfSerdes);
    GT_UNUSED_PARAM(min_LF);
    GT_UNUSED_PARAM(max_LF);
#else

    GT_U32 sbus_addr;
    unsigned int dwell;
    GT_U32  dly, eye;
    GT_U32  sdDetect;
    GT_U32  i, j, k;
    GT_32    LF, GAIN;
    GT_U32  best_dly[MAX_AVAGO_SERDES_NUMBER] = {0};
    GT_U32  best_eye[MAX_AVAGO_SERDES_NUMBER] = {0};
    GT_32   best_LF[MAX_AVAGO_SERDES_NUMBER]  = {0};
    GT_32   LF1_Arr[MAX_AVAGO_SERDES_NUMBER]  = {0};
    GT_32   LF2_Arr[MAX_AVAGO_SERDES_NUMBER]  = {0};
    GT_32     DFEgain_Arr[MAX_AVAGO_SERDES_NUMBER]    = {0};
    GT_U32  serdesArrValid[MAX_AVAGO_SERDES_NUMBER] = {0};
    MV_HWS_SERDES_SPEED rate;
    GT_U32 *currentDelayPtr;
    GT_U32 currentDelaySize;
    unsigned int chipIndex;

    EXEC_TIME_START();

    RETURN_GT_OK_ON_EMULATOR;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesArr[0]);

    for (i=0; i < MAX_AVAGO_SERDES_NUMBER; i++)
    {
        best_eye[i] = 4;
    }

    if (numOfSerdes > MAX_AVAGO_SERDES_NUMBER)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT("numOfSerdes %d is greater than MAX_AVAGO_SERDES_NUMBER %d\n", numOfSerdes, MAX_AVAGO_SERDES_NUMBER);
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsAvagoSerdeSpeedGet(devNum, portGroup, serdesArr[0], &rate));
#ifndef ALDRIN_DEV_SUPPORT
    if(HWS_DEV_SILICON_TYPE(devNum) >= Bobcat3 && rate >= _20_625G)
    {
        currentDelayPtr = static_delay_25G;
        currentDelaySize = sizeof(static_delay_25G)/sizeof(GT_U32);
    }
    else
#endif
    {
        currentDelayPtr = static_delay_10G;
        currentDelaySize = sizeof(static_delay_10G)/sizeof(GT_U32);
    }

    /* enlarge Dwell time */
    dwell = avagoBaudRate2DwellTimeValue[rate];
    for (i=0; i< numOfSerdes; i++)
    {
            /* select DFE tuning dwell time */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x18, 0x7, NULL));
            /* big register write LSB */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, 1 << dwell, NULL));
            /* big register write MSB */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, 0, NULL));
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 1 ## \n");

    /* check CDR lock and Signal Detect on all Serdeses */
    for (i=0; i < numOfSerdes; i++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 2 ## serdesNum=%d\n", MV_HWS_SERDES_NUM(serdesArr[i]));

        CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, MV_HWS_SERDES_NUM(serdesArr[i]), &sbus_addr));

        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, MV_HWS_SERDES_NUM(serdesArr[i]),
                                            AVAGO_EXTERNAL_STATUS, &sdDetect, (1 << 2)));
        if (sdDetect != 0)
        {
            serdesArr[i] = SKIP_SERDES; /* change the Serdes number to mark this array's element as not relevant */
            continue;
        }

        /* set SamplePoint to default value (delay=0) */
        CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[i], currentDelayPtr[0]));
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 3 ## \n");
    /* copy the original Serdes array for saving the data */
    hwsOsMemCopyFuncPtr(serdesArrValid, serdesArr, sizeof(GT_U32) * numOfSerdes);

    /* run iCAL(CTLE and DFE) */
    CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSet(devNum, portGroup, serdesArr, numOfSerdes, ICAL_TRAINING_MODE));

    /* save the LF value of all Serdeses */
    for (k=0; k < numOfSerdes; k++)
    {
        if (serdesArr[k] == SKIP_SERDES) /* No Signal-Detect on this Serdes */
            continue;

        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 4 ## serdesNum=%d\n", serdesArr[k]);
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[k], 0x126, ((2 << 12) | (1 << 8)), &LF));
        best_LF[k]=LF;
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 5 ## \n");
    /* find the peak of the eye accoding to delay */
    for (i=0; i < currentDelaySize; i++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 6 ## \n");
        dly = currentDelayPtr[i];

        for (k=0; k < numOfSerdes; k++)
        {
            AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 7 ## serdesNum=%d\n", serdesArr[k]);
            if (serdesArr[k] == SKIP_SERDES) /* No Signal-Detect on this Serdes */
                continue;

            CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[k], dly));
        }

        /* trigger pCAL(DFE) on all relevant Serdeses */
        CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSet(devNum, portGroup, serdesArr, numOfSerdes, PCAL_TRAINING_MODE));

        for (j=0; j < numOfSerdes; j++)
        {
            if (serdesArr[j] == SKIP_SERDES) /* No Signal-Detect on this Serdes */
                continue;

            AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 8 ## serdesNum=%d\n", serdesArr[j]);
            CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesArr[j], &sbus_addr));

            AVAGO_LOCK(devNum, chipIndex);
            /* get a simple eye metric in range [0..1000] */
            eye = avago_serdes_eye_get_simple_metric(aaplSerdesDb[chipIndex], sbus_addr);
            AVAGO_UNLOCK(devNum, chipIndex);
            CHECK_AVAGO_RET_CODE();

            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[j], 0x126, ((2 << 12) | (1 << 8)), &LF));

            if (eye > (best_eye[j] + MV_AVAGO_EYE_TAP))
            {
                best_eye[j] = eye;
                best_dly[j] = dly;

                if (LF >= best_LF[j])
                    best_LF[j] = LF;
            }
#ifndef MICRO_INIT
            AVAGO_TUNE_BY_PHASE_DEBUG_PRINT("current_eye[%d]=%d dly[%d]=%d\n", j, eye, j, dly);
#endif
            if (best_eye[j] < MV_AVAGO_MIN_EYE_SIZE)
            {
                AVAGO_TUNE_BY_PHASE_DEBUG_PRINT("Error on Serdes %d: Eye is too small (size is %d). Training Failed\n", serdesArr[j], best_eye[j]);
                return GT_FAIL;
            }
        }
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 9 ## \n");
    for (k=0; k < numOfSerdes; k++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 10 ## serdesNum=%d\n", serdesArr[k]);
        if (serdesArr[k] == SKIP_SERDES) /* skip Serdes if the dly is the same as the best_dly */
            continue;

        CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[k], best_dly[k]));
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 11 ## \n");
    /* run iCAL(CTLE and DFE) with best delay */
    CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSet(devNum, portGroup, serdesArr, numOfSerdes, ICAL_TRAINING_MODE));

    for (k=0; k < numOfSerdes; k++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 12 ## serdesNum=%d\n", serdesArrValid[k]);
        /* read the LF value */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x126, ((2 << 12) | (1 << 8)), &LF1_Arr[k]));
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 13 ## \n");
    /* run iCAL(CTLE and DFE) with best delay */
    CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSet(devNum, portGroup, serdesArr, numOfSerdes, ICAL_TRAINING_MODE));

    for (k=0; k < numOfSerdes; k++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 14 ## serdesNum=%d\n", serdesArrValid[k]);
        /* read again the LF value after run the iCAL */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x126, ((2 << 12) | (1 << 8)), &LF2_Arr[k]));

        /* read the DFE gain */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x126, ((3 << 12) | (0 << 8)), &DFEgain_Arr[k]));

        LF = (LF1_Arr[k] + LF2_Arr[k])/2; /* take the avarge from two tunning values */
        LF = (LF < min_LF) ? min_LF : LF;
        LF = (LF > max_LF) ? max_LF : LF;

        /* write the selected LF value back to Serdes */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x26, ((2 << 12) | (1 << 8) | LF), NULL));

        GAIN = (DFEgain_Arr[k] > 1) ? (DFEgain_Arr[k]-1) : DFEgain_Arr[k];
        /* write the selected GAIN value back to Serdes */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x26, ((3 << 12) | (0 << 8) | GAIN), NULL));
#if (defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
#ifdef RUN_ADAPTIVE_CTLE
        hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesArrValid[k]].enhTrainDelay = (GT_U8)(best_dly[k] - 15);
        hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesArrValid[k]].currSerdesDelay = (GT_U8)(best_dly[k] - 15);
        hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesArrValid[k]].trainLf = (GT_U8)LF;
#endif /*RUN_ADAPTIVE_CTLE*/
#endif
#ifndef MICRO_INIT
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT("Serdes %d: setting best_dly=%d best_eye=%d LF=%d GAIN=%d\n", serdesArrValid[k], best_dly[k], best_eye[k], LF, GAIN);
#endif
        /* run pCAL(DFE) */
        CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesArrValid[k], DFE_PCAL, NULL));
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 15 ## \n");

    EXEC_TIME_END_PRINT("mvHwsAvagoSerdesEnhanceTune");

#endif /* ASIC_SIMULATION */

    return GT_OK;
}
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT)) */

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
/**
* @internal mvHwsAvagoSerdesEnhanceTuneLite function
* @endinternal
*
* @brief   Set the PCAL with shifted sampling point to find best sampling point
*         This API runs only for AP port after linkUp indication and before running
*         the Rx-Training Adative pCal
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] serdesSpeed              - speed of serdes
* @param[in] min_dly                  - Minimum delay_cal value:
*                                      - for Serdes speed 10G (25...28)
*                                      - for Serdes speed 25G (15...19)
* @param[in] max_dly                  - Maximum delay_cal value:
*                                      - for Serdes speed 10G (30)
*                                      - for Serdes speed 25G (21)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTuneLite
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN MV_HWS_SERDES_SPEED     serdesSpeed,
    IN GT_U8                   min_dly,
    IN GT_U8                   max_dly
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesArr);
    GT_UNUSED_PARAM(numOfSerdes);
    GT_UNUSED_PARAM(serdesSpeed);
    GT_UNUSED_PARAM(min_dly);
    GT_UNUSED_PARAM(max_dly);
    GT_UNUSED_PARAM(serdesSpeed);

#else

    unsigned int sbus_addr;
    unsigned int chipIndex;
    GT_U32  dly, eye;
    MV_HWS_AUTO_TUNE_STATUS  sdDetect;
    GT_U32  i, j, k;
    GT_U32  best_dly[HWS_MAX_PORT_AVAGO_SERDES_NUM] = {0};
    GT_U32  best_eye[HWS_MAX_PORT_AVAGO_SERDES_NUM] = {0};
    GT_U32  serdesArrValid[HWS_MAX_PORT_AVAGO_SERDES_NUM] = {0};
    GT_U32 *currentDelayPtr;
    GT_U32 currentDelaySize;
    GT_U32 dwell;

    RETURN_GT_OK_ON_EMULATOR;

    if (numOfSerdes > HWS_MAX_PORT_AVAGO_SERDES_NUM)
    {
        HWS_AVAGO_DBG(("numOfSerdes %d is greater than MAX_AVAGO_SERDES_NUMBER %d\n", numOfSerdes, 4));
        return GT_BAD_PARAM;
    }
    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesArr[0]);

    for (i=0; i < numOfSerdes; i++)
    {
        best_eye[i] = 4;
    }

    /* define dwell value */
    if (avagoBaudRate2DwellTimeValue[serdesSpeed] != NA_VALUE)
    {
        dwell = avagoBaudRate2DwellTimeValue[serdesSpeed];
    }
    else
    {
        dwell = 11;
    }

    if(HWS_DEV_SILICON_TYPE(devNum) >= Bobcat3)
    {
        if(serdesSpeed >= _20_625G)
        {
            currentDelayPtr = static_delay_25G;
            currentDelaySize = sizeof(static_delay_25G)/sizeof(GT_U32);
        }
        else
        {
            currentDelayPtr = static_delay_10G;
            currentDelaySize = sizeof(static_delay_10G)/sizeof(GT_U32);
        }
    }
    else
    {
        currentDelayPtr = static_delay_10G;
        currentDelaySize = sizeof(static_delay_10G)/sizeof(GT_U32);
    }

    for (i=0; i < numOfSerdes; i++)
    {
        CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, MV_HWS_SERDES_NUM(serdesArr[i]), &sbus_addr));

        /* check CDR lock and Signal Detect on Serdes */
        CHECK_STATUS(mvHwsAvagoSerdesRxSignalCheck(devNum, portGroup, MV_HWS_SERDES_NUM(serdesArr[i]), &sdDetect));
        if (sdDetect == TUNE_NOT_READY)
        {
            serdesArr[i] = SKIP_SERDES; /* change the Serdes number to mark this array's element as not relevant */
            continue;
        }

        /* select DFE tuning dwell time */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x18, 0x7, NULL));
        /* big register write LSB */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, 1 << dwell, NULL));
        /* big register write MSB */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, 0, NULL));
    }

    /* copy the original Serdes array for saving the data */
    hwsOsMemCopyFuncPtr(serdesArrValid, serdesArr, sizeof(GT_U32) * numOfSerdes);

    /* find the peak of the eye according to delay */
    for (i=0; i < currentDelaySize; i++)
    {
        dly = currentDelayPtr[i];

        if(dly >= min_dly && dly <= max_dly)
        {

            for (k=0; k < numOfSerdes; k++)
            {
                if (serdesArr[k] == SKIP_SERDES) /* No Signal-Detect on this Serdes */
                    continue;

                CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[k], dly));
            }

            /* trigger pCAL(DFE) on all relevant Serdeses */
            CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSet(devNum, portGroup, serdesArr, numOfSerdes, PCAL_TRAINING_MODE));

            for (j=0; j < numOfSerdes; j++)
            {
                if (serdesArr[j] == SKIP_SERDES) /* No Signal-Detect on this Serdes */
                    continue;

                CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesArr[j], &sbus_addr));

                AVAGO_LOCK(devNum, chipIndex);
                /* get a simple eye metric in range [0..1000] */
                eye = avago_serdes_eye_get_simple_metric(aaplSerdesDb[chipIndex], sbus_addr);
                AVAGO_UNLOCK(devNum, chipIndex);
                CHECK_AVAGO_RET_CODE();

                if (eye > (best_eye[j] + MV_AVAGO_EYE_TAP))
                {
                    best_eye[j] = eye;
                    best_dly[j] = dly;
                }
                if (best_eye[j] < MV_AVAGO_MIN_EYE_SIZE)
                {
                    HWS_AVAGO_DBG(("Error on Serdes %d: Eye is too small (size is %d). Training Failed\n", serdesArr[j], best_eye[j]));
                    return GT_FAIL;
                }
            }
        }
    }

    for (k=0; k < numOfSerdes; k++)
    {
        if (serdesArr[k] == SKIP_SERDES) /* skip Serdes if the dly is the same as the best_dly */
            continue;

        CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[k], best_dly[k]));
        /* for adaptive ctle*/
        #ifdef RUN_ADAPTIVE_CTLE
        hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesArr[k]].enhTrainDelay = (GT_U8)(best_dly[k] -15);
        hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesArr[k]].currSerdesDelay = (GT_U8)(best_dly[k] -15);
        #endif /*RUN_ADAPTIVE_CTLE*/
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

/**
* @internal mvHwsAvagoSerdesEnhanceTuneLitePhase1 function
* @endinternal
*
* @brief   Set the PCAL with shifted sampling point to find best sampling point.
*         This API runs only for AP port after linkUp indication and before running
*         the Rx-Training Adative pCal.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] serdesSpeed              - speed of serdes
* @param[in] currentDelaySize         - delays array size
* @param[in] currentDelayPtr          - delays array
* @param[in] inOutI                   - iteration index between different phases
* @param[in] best_eye                 - best eyes array to update
* @param[in] best_dly                 - best delays array to update
* @param[in] subPhase                 - assist flag to know which code to execute in this sub-phase
* @param[in] min_dly                  - Minimum delay_cal value: (rang: 0-31)
* @param[in] max_dly                  - Maximum delay_cal value: (rang: 0-31)
*
* @param[out] inOutI                   - iteration index between different phases
* @param[out] best_eye                 - best eyes array to update
* @param[out] best_dly                 - best delays array to update
* @param[out] subPhase                 - assist flag to know which code to execute in this sub-phase
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTuneLitePhase1
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN MV_HWS_SERDES_SPEED     serdesSpeed,
    IN GT_U8       currentDelaySize,
    IN GT_U8       *currentDelayPtr,
    INOUT GT_U8       *inOutI,
    INOUT GT_U16      *best_eye,
    INOUT GT_U8       *best_dly,
    INOUT GT_U8       *subPhase,
    IN GT_U8       min_dly,
    IN GT_U8       max_dly
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesArr);
    GT_UNUSED_PARAM(numOfSerdes);
    GT_UNUSED_PARAM(serdesSpeed);
    GT_UNUSED_PARAM(currentDelaySize);
    GT_UNUSED_PARAM(currentDelayPtr);
    GT_UNUSED_PARAM(inOutI);
    GT_UNUSED_PARAM(best_eye);
    GT_UNUSED_PARAM(best_dly);
    GT_UNUSED_PARAM(subPhase);
    GT_UNUSED_PARAM(serdesSpeed);
    GT_UNUSED_PARAM(min_dly);
    GT_UNUSED_PARAM(max_dly);
#else

    GT_U32 sbus_addr;
    unsigned int chipIndex;
    GT_U8   dly;
    GT_U16  eye;
    GT_U8   i, k, j;
    GT_U8   dwell;

    if (numOfSerdes > HWS_MAX_PORT_AVAGO_SERDES_NUM)
    {
        HWS_AVAGO_DBG(("numOfSerdes %d is greater than MAX_AVAGO_SERDES_NUMBER %d\n", numOfSerdes, 4));
        return GT_BAD_PARAM;
    }
    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesArr[0]);
    /* define dwell value */
    if (avagoBaudRate2DwellTimeValue[serdesSpeed] != NA_VALUE)
    {
        dwell = avagoBaudRate2DwellTimeValue[serdesSpeed];
    }
    else
    {
        dwell = 11;
    }

    /* find the peak of the eye according to delay */
    for (i=(*inOutI); i < currentDelaySize; i++)
    {
        if (*subPhase == 0)
        {
            dly = currentDelayPtr[i];

            /* filter undesired delays */
            if ( dly >= min_dly && dly <= max_dly )
            {
                for (k = 0; k < numOfSerdes; k++)
                {
                    /* select DFE tuning dwell time */
                    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[k], 0x18, 0x7, NULL));
                    /* big register write LSB */
                    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[k], 0x19, 1 << dwell, NULL));
                    /* big register write MSB */
                    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[k], 0x19, 0, NULL));

                    CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[k], dly));

                    CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesArr[k], DFE_PCAL, NULL));
                }
            }

            *subPhase = 1;
            break;
        }
        else
        {
            for (j=0; j < numOfSerdes; j++)
            {
                dly = currentDelayPtr[i];

                CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesArr[j], &sbus_addr));

                AVAGO_LOCK(devNum, chipIndex);
                /* get a simple eye metric in range [0..1000] */
                eye = avago_serdes_eye_get_simple_metric(aaplSerdesDb[chipIndex], sbus_addr);
                AVAGO_UNLOCK(devNum, chipIndex);
                CHECK_AVAGO_RET_CODE();
                if (eye > (best_eye[j] + MV_AVAGO_EYE_TAP))
                {
                    best_eye[j] = eye;
                    best_dly[j] = dly;
                }
                if (best_eye[j] < MV_AVAGO_MIN_EYE_SIZE)
                {
                    HWS_AVAGO_DBG(("Error on Serdes %d: Eye is too small (size is %d). Training Failed\n", serdesArr[j], best_eye[j]));
                    return GT_FAIL;
                }
            }

            *subPhase = 0;
        }
    }

    if (i < currentDelaySize)
    {
        (*inOutI) = i;
    }
    else
    {
        if ((*subPhase)==0)
        {
            (*inOutI) = 0xFF;
        }
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEnhanceTuneLitePhase2 function
* @endinternal
*
* @brief   Set shift sample point to with the best delay_cal value.
*         This API runs only for AP port after linkUp indication and before running
*         the Rx-Training Adative pCal
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] best_dly                 - best delay to set on serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTuneLitePhase2
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_U8       *best_dly
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum      = devNum;
    portGroup   = portGroup;
    serdesArr   = serdesArr;
    GT_UNUSED_PARAM(numOfSerdes);
    best_dly    = best_dly;
#else
    GT_U8 k;

    for (k=0; k < numOfSerdes; k++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[k], best_dly[k]));
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesVoltageGet function
* @endinternal
*
* @brief   Get the voltage (in mV) from Avago Serdes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] chipIndex                - chip index
*
* @param[out] voltage                  - Serdes  value (in mV)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesVoltageGet
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     chipIndex,
    OUT GT_UOPT    *voltage
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(chipIndex);
    *voltage = 0;
#else

    unsigned int    sensor_addr;

#ifdef CHX_FAMILY
    if ((HWS_DEV_SILICON_TYPE(devNum) == Raven) || (HWS_DEV_SILICON_TYPE(devNum) == Falcon))
    {
        if (chipIndex > hwsFalconNumOfRavens - 1)
        {
            return GT_NOT_SUPPORTED;
        }
    }
#endif

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if(hwsAvagoNotSupportedCheck())
    {
        *voltage = 0;
        /* no SERDESes on emulator */
        return GT_OK;
    }
#endif /* not MV_HWS_REDUCED_BUILD_EXT_CM3*/

    if ((HWS_DEV_SILICON_TYPE(devNum) == BobK) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin))
    {
        sensor_addr = 9;
    }
    else if ((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3))
    {
        sensor_addr = 26;
    }
    else if ((HWS_DEV_SILICON_TYPE(devNum) == Aldrin2))
    {
        sensor_addr = 1; /* sbus_address 1 of TSEN0 in ring0 */
    }
    else if ((HWS_DEV_SILICON_TYPE(devNum) == Pipe) || (HWS_DEV_SILICON_TYPE(devNum) == Raven) || (HWS_DEV_SILICON_TYPE(devNum) == Falcon))
    {
        sensor_addr = 18;
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }

    AVAGO_LOCK(devNum, chipIndex);
    /* returns the voltage in mV */
    if ((HWS_DEV_SILICON_TYPE(devNum) != Raven) && (HWS_DEV_SILICON_TYPE(devNum) != Falcon))
    {
        chipIndex = devNum;
    }
    *voltage = avago_sensor_get_voltage(aaplSerdesDb[chipIndex], sensor_addr, 0, 0);
    AVAGO_UNLOCK(devNum, chipIndex);

#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesAutoTuneResult function
* @endinternal
*
* @brief   Per SERDES return the adapted tuning results
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] results                  - the adapted tuning results.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneResult
(
    IN  GT_U8                               devNum,
    IN  GT_UOPT                             portGroup,
    IN  GT_UOPT                             serdesNum,
    OUT MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *tuneResults
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    hwsOsMemSetFuncPtr(tuneResults,0,sizeof(*tuneResults));
#else

    GT_U32                              sbus_addr;
    unsigned int                        chipIndex;
    GT_U32                              data, column, i;
    GT_32                               dfe[13];
    Avago_serdes_tx_eq_t                tx_eq;
    GT_UOPT                             signalThreshold;
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    configParams;
    MV_HWS_AVAGO_AUTO_TUNE_RESULTS      *results;

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if(hwsAvagoNotSupportedCheck())
    {
        hwsOsMemSetFuncPtr(tuneResults,0,sizeof(*tuneResults));
        /* no SERDESes on emulator */
        return GT_OK;
    }
#endif /* not MV_HWS_REDUCED_BUILD_EXT_CM3*/

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    AVAGO_LOCK(devNum, chipIndex);
    /* Gets the TX equalization values */
    avago_serdes_get_tx_eq(aaplSerdesDb[chipIndex], sbus_addr, &tx_eq);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();
    results = &tuneResults->avagoResults;
    results->avagoStc.atten = tx_eq.atten;
    results->avagoStc.post  = tx_eq.post;
    results->avagoStc.pre   = tx_eq.pre;

    results->avagoStc.atten   = tx_eq.atten;
    results->avagoStc.post = tx_eq.post;
    results->avagoStc.pre = tx_eq.pre;

    for (i=0; i <= 12; i++)
    {
        if (i==1)
        {
            column=1;
            data = (column << 12) | (8 << 8);
        }
        else
        {
            column=3;
            data = (column << 12) | (i << 8);
        }
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x126, data, &dfe[i]));
        (dfe[i] > 0x8000) ? (dfe[i] = dfe[i] - 0x10000) : dfe[i];
        results->DFE[i] = dfe[i];
        AVAGO_DBG(("   DFE[%d] = %d \n", i, results->DFE[i]));
    }

    CHECK_STATUS(mvHwsAvagoSerdesManualCtleConfigGet(devNum, portGroup, serdesNum, &configParams));

    results->HF = configParams.highFrequency;
    results->LF = configParams.lowFrequency;
    results->DC = configParams.dcGain;
    results->BW = configParams.bandWidth;
    results->gainshape1 = configParams.gainshape1;
    results->gainshape2 = configParams.gainshape2;
    results->dfeGAIN = configParams.dfeGAIN;
    results->dfeGAIN2 = configParams.dfeGAIN2;

    AVAGO_LOCK(devNum, chipIndex);
    results->EO = avago_serdes_eye_get_simple_metric(aaplSerdesDb[chipIndex], sbus_addr);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();

    /* Get the signal OK threshold on Serdes */
    CHECK_STATUS(mvHwsAvagoSerdesSignalOkThresholdGet(devNum, portGroup, serdesNum, &signalThreshold));
    results->sqleuch = (signalThreshold * 16) + 68;

    AVAGO_DBG(("   HF = %d \n", results->HF));
    AVAGO_DBG(("   LF = %d \n", results->LF));
    AVAGO_DBG(("   DC = %d \n", results->DC));
    AVAGO_DBG(("   BW = %d \n", results->BW));
    AVAGO_DBG(("   EO = %d \n", results->EO));

#endif /* ASIC_SIMULATION */

    return GT_OK;
}
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined RAVEN_DEV_SUPPORT || defined FALCON_DEV_SUPPORT
/*******************************************************************************
* mvHwsAvago16nmSerdesAutoTuneResult
*
* DESCRIPTION:
*       Per SERDES return the adapted tuning results
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       results - the adapted tuning results.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsAvago16nmSerdesAutoTuneResult
(
    IN  GT_U8                               devNum,
    IN  GT_UOPT                             portGroup,
    IN  GT_UOPT                             serdesNum,
    OUT MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *tuneResults
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    hwsOsMemSetFuncPtr(tuneResults, 0, sizeof(*tuneResults));
#else
    GT_U32                          sbus_addr;
    unsigned int                    chipIndex;
    GT_U32                          data, member, i;
    GT_32                           ctle[11];
    GT_32                           rxffe[14];
    GT_32                           dfe[13];
    Avago_serdes_tx_eq_t            tx_eq;
    GT_UOPT                         signalThreshold;
    GT_BOOL                         isPam4;
    MV_HWS_AVAGO_AUTO_TUNE_RESULTS  *results;
    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if(hwsAvagoNotSupportedCheck())
    {
        hwsOsMemSetFuncPtr(tuneResults, 0, sizeof(*tuneResults));
        /* no SERDESes on emulator */
        return GT_OK;
    }
#endif /* not MV_HWS_REDUCED_BUILD_EXT_CM3*/

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    AVAGO_LOCK(devNum, chipIndex);
    /* Gets the TX equalization values */
    avago_serdes_get_tx_eq(aaplSerdesDb[chipIndex], sbus_addr, &tx_eq);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();
    results = &tuneResults->avagoResults;
    results->avagoStc.atten = tx_eq.atten;
    results->avagoStc.post  = tx_eq.post;
    results->avagoStc.pre   = tx_eq.pre;
    results->avagoStc.pre2  = tx_eq.pre2;
    results->avagoStc.pre3  = tx_eq.pre3;

    for (i=0; i <= 12; i++)
    {
        member=0xB;
        data = (member << 8) | i;

        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x2C, data, &dfe[i]));
        /*(dfe[i] > 0x8000) ? (dfe[i] = dfe[i] - 0x10000) : dfe[i];*/
        results->DFE[i] = dfe[i];
        AVAGO_DBG(("   DFE[%d] = %d \n", i, results->DFE[i]));
    }

    member=9;
    for (i = 0; i <= 10; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x2C, ((member << 8) | i), &ctle[i]));
    }
    results->HF             = ctle[0];
    results->LF             = ctle[1];
    results->DC             = ctle[2];
    results->BW             = ctle[3];
    results->gainshape1     = ctle[4];
    results->gainshape2     = ctle[5];
    results->shortChannelEn = ctle[6];
    results->minHf          = ctle[7];
    results->maxHf          = ctle[8];
    results->minLf          = ctle[9];
    results->maxLf          = ctle[10];

    member=0xD;
    for (i = 0; i <= 13; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x2C, ((member << 8) | i ), &rxffe[i]));
    }

    results->rxffe_pre2 = rxffe[0];
    results->rxffe_pre1 = rxffe[1];
    results->rxffe_post1 = rxffe[2];
    results->rxffe_bflf = rxffe[3];
    results->rxffe_bfhf = rxffe[4];
    results->rxffe_datarate = rxffe[6];

#if !defined MV_HWS_REDUCED_BUILD_EXT_CM3 || defined (FALCON_DEV_SUPPORT)
    results->rxffe_minPre1 = rxffe[7];
    results->rxffe_maxPre1 = rxffe[8];
    results->rxffe_minPre2 = rxffe[9];
    results->rxffe_maxPre2 = rxffe[10];
    results->rxffe_minPost = rxffe[12];
    results->rxffe_maxPost = rxffe[13];

    results->vernier_upper_odd_dly = 0;
    results->vernier_upper_even_dly = 0;
    results->vernier_middle_odd_dly = 0;
    results->vernier_middle_even_dly = 0;
    results->vernier_lower_odd_dly = 0;
    results->vernier_lower_even_dly = 0;
    results->vernier_test_odd_dly = 0;
    results->vernier_test_even_dly = 0;
    results->vernier_edge_odd_dly = 0;
    results->vernier_edge_even_dly = 0;
    results->vernier_tap_dly = 0;
#endif

    AVAGO_LOCK(devNum, chipIndex);
    results->termination = avago_serdes_get_rx_term(aaplSerdesDb[chipIndex], sbus_addr);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();

    /* Get PAM4 Eyes heights */
    AVAGO_LOCK(devNum, chipIndex);
    isPam4 = avago_serdes_get_rx_line_encoding(aaplSerdesDb[chipIndex], sbus_addr);
    AVAGO_UNLOCK(devNum, chipIndex);

    if (isPam4 == GT_FALSE)      /* NRZ */
    {
        CHECK_STATUS(mvHwsAvago16nmSerdesHalGet(devNum, serdesNum, HWS_SERDES_PAM4_EYE, 0, (GT_32*)&results->EO));
        for (i = 0; i < CPSS_PAM4_EYES_ARRAY_SIZE_CNS; i++)
        {
            results->pam4EyesArr[i] = 0;
        }
    }
    else                         /* PAM4 */
    {
        results->EO = 0;
        for (i = 0; i < CPSS_PAM4_EYES_ARRAY_SIZE_CNS; i++)
        {
            CHECK_STATUS(mvHwsAvago16nmSerdesHalGet(devNum, serdesNum, HWS_SERDES_PAM4_EYE, i, (GT_32*)&results->pam4EyesArr[i]));
        }
    }

    /* Get the signal OK threshold on Serdes */
    CHECK_STATUS(mvHwsAvagoSerdesSignalOkThresholdGet(devNum, portGroup, serdesNum, &signalThreshold));
    results->sqleuch = (signalThreshold * 16) + 68;

    AVAGO_DBG(("   HF = %d \n"          , results->HF));
    AVAGO_DBG(("   LF = %d \n"          , results->LF));
    AVAGO_DBG(("   DC = %d \n"          , results->DC));
    AVAGO_DBG(("   BW = %d \n"          , results->BW));
    AVAGO_DBG(("   EO = %d \n"          , results->EO));
    AVAGO_DBG(("   EOLowerEven = %d \n" , results->pam4EyesArr[0]));
    AVAGO_DBG(("   EOLowerOdd = %d \n"  , results->pam4EyesArr[1]));
    AVAGO_DBG(("   EOMiddleEven = %d \n", results->pam4EyesArr[2]));
    AVAGO_DBG(("   EOMiddleOdd = %d \n" , results->pam4EyesArr[3]));
    AVAGO_DBG(("   EOUpperEven = %d \n" , results->pam4EyesArr[4]));
    AVAGO_DBG(("   EOUpperOdd = %d \n"  , results->pam4EyesArr[5]));

    CHECK_STATUS(mvHwsAvago16nmSerdesHalGet(devNum, serdesNum, HWS_SERDES_ENVELOPE_CONFIG, 0x1, (GT_32*)&results->coldEnvelope));
    CHECK_STATUS(mvHwsAvago16nmSerdesHalGet(devNum, serdesNum, HWS_SERDES_ENVELOPE_CONFIG, 0x2, (GT_32*)&results->hotEnvelope));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}
#endif
/**
* @internal mvHwsAvagoSerdeCpllOutputRefClkGet function
* @endinternal
*
* @brief   Return the output frequency of CPLL reference clock source of SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] refClk                   - CPLL reference clock frequency
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdeCpllOutputRefClkGet
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    OUT MV_HWS_CPLL_OUTPUT_FREQUENCY    *refClk
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(refClk);
#else
    GT_U32  readDataAddressReg, readDataOutReg;
    GT_U32  data1, data2, data3, shift = 0 , addrShift = 16;

    if(HWS_DEV_SILICON_TYPE(devNum) == Bobcat3)
    {
        if((serdesNum < 36) || (serdesNum == 72))
        {
            readDataOutReg = DEVICE_GENERAL_STATUS_3;
            readDataAddressReg = DEVICE_GENERAL_CONTROL_17; /* CPLL_0 */
        }
        else
        {
            readDataOutReg = DEVICE_GENERAL_STATUS_4;
            readDataAddressReg = DEVICE_GENERAL_CONTROL_18; /* CPLL_1 */
        }
    }
    else if(HWS_DEV_SILICON_TYPE(devNum) == Aldrin2)
    {
        if(serdesNum < 24)
        {
            readDataOutReg = DEVICE_GENERAL_STATUS_3;
            readDataAddressReg = DEVICE_GENERAL_CONTROL_17; /* CPLL_0 */
        }
        else
        {
            readDataOutReg = DEVICE_GENERAL_STATUS_4;
            readDataAddressReg = DEVICE_GENERAL_CONTROL_18; /* CPLL_1 */
        }
    }
    else if(HWS_DEV_SILICON_TYPE(devNum) == Aldrin)
    {
        readDataAddressReg = DEVICE_GENERAL_CONTROL_13;
        readDataOutReg = DEVICE_GENERAL_STATUS_4;
        shift = 16;
    }
    else if(HWS_DEV_SILICON_TYPE(devNum) == BobK)
    {
        readDataAddressReg = DEVICE_GENERAL_CONTROL_21;
        readDataOutReg = DEVICE_GENERAL_STATUS_4;
    }
    else if(HWS_DEV_SILICON_TYPE(devNum) == Pipe)
    {
        readDataAddressReg = DEVICE_GENERAL_CONTROL_16;
        addrShift = 22;
        readDataOutReg = DEVICE_GENERAL_STATUS_1;
        shift = 14;
    }
    else
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ARG_STRING_MAC("illegal device type\n"));
    }

    CHECK_STATUS(hwsServerRegFieldSetFuncPtr(devNum, readDataAddressReg, addrShift, 8, 0xA));
    CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, readDataOutReg, &data1));
    data1 = (data1 >> shift) & 0xFFFF; /* Get cpll_ndiv_Msb: Bits #0_15 */

    /* set 1ms delay */
    hwsOsExactDelayPtr(devNum, devNum, 1);

    CHECK_STATUS(hwsServerRegFieldSetFuncPtr(devNum, readDataAddressReg, addrShift, 8, 0xB));
    CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, readDataOutReg, &data2));
    data2 = (data2 >> shift) & 0xFFFF; /* Get cpll_ndiv_Lsb: Bits #0_15 */

    /* set 1ms delay */
    hwsOsExactDelayPtr(devNum, devNum, 1);

    CHECK_STATUS(hwsServerRegFieldSetFuncPtr(devNum, readDataAddressReg, addrShift, 8, 0xD));
    CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, readDataOutReg, &data3));
    data3 = (data3 >> shift) & 0xFFFF; /* Get cpll_NHF_NLF: Bits #0_15 */

    /*
        For divider values per output frequency check CPLL init sequences:
            cpllDataArray
        or
            bc3CpllDataArray
    */

    if (( ((data1==0x8555) && ((data2==0x5555)||(data2==0xA938/*+9.6PPM*/))) ||
          ((data1==0x8556) && (data2==0x2FC9) /*+25 PPM*/)||
          ((data1==0x8557) && (data2==0x0A3D) /*+50 PPM*/)||
          ((data1==0x8000) && (data2==0xD1B7) /*+25PPM*/)) && (data3==0x4400))
    {
        *refClk = MV_HWS_156MHz_OUT; /* 156.25Mhz */
    }
    else if (( ((data1==0x8555) && ((data2==0x5555)||(data2==0xA938/*+9.6PPM*/))) ||
          ((data1==0x8556) && (data2==0x2FC9) /*+25PPM*/) ||
          ((data1==0x8557) && (data2==0x0A3D) /*+50 PPM*/)||
          ((data1==0x8000) && (data2==0xD1B7) /*+25PPM*/)) && (data3==0x4800))
    {
        *refClk = MV_HWS_78MHz_OUT; /* 78.125Mhz */
    }
    else if (( ((data1==0x8555) && ((data2==0x5555)||(data2==0xA938/*+9.6PPM*/))) ||
          ((data1==0x8556) && (data2==0x2FC9) /*+25PPM*/)||
          ((data1==0x8557) && (data2==0x0A3D) /*+50 PPM*/)||
          ((data1==0x8000) && (data2==0xD1B7) /*+25PPM*/)) && (data3==0x4000))
    {
        *refClk = MV_HWS_312MHz_OUT; /* 312.5Mhz */
    }
    else if ((data1==0x8C28) && (data2==0x325D) /*+25PPM*/ )
    {
        *refClk = MV_HWS_164MHz_OUT; /* 164.24Mhz */
    }
    else
    {
        *refClk = 0;
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ARG_STRING_MAC("unknown CPLL divider value\n"));
    }

#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(portGroup);

    return GT_OK;
}

/**
* @internal mvHwsAvago16nmDividerGet function
* @endinternal
*
* @brief   Return SERDES clock divider value.
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - Serdes Number
*
* @param[out] dividerValue           - sereds clock divider value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvago16nmDividerGet
(
    IN GT_U8                 devNum,
    IN GT_U32                serdesNum,
    OUT GT_U32               *dividerValue
)
{
#ifdef ASIC_SIMULATION
     /* avoid warnings */
     GT_UNUSED_PARAM(devNum);
     GT_UNUSED_PARAM(serdesNum);
     GT_UNUSED_PARAM(dividerValue);
#else
     GT_U32 sbusAddr;
     GT_U32 divxCtl1Reg = 0x0D8;
     GT_U32 divxCtl2Reg = 0x0F5;
     GT_U32 data1, data2;
     GT_U32 divider;
     GT_U32 divxPre, divxLsHi, divxLsLo, divxLsHalf, divxInCntl, divxLsDiv1;
     GT_U32 divxHsSel, divxHsHalf, divxHsHi, divxHsLo;
     GT_U32 chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
     CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbusAddr));

     AVAGO_LOCK(devNum, chipIndex);
     data1 = avago_serdes_mem_rd(aaplSerdesDb[chipIndex], sbusAddr, AVAGO_ESB, divxCtl1Reg);
     data2 = avago_serdes_mem_rd(aaplSerdesDb[chipIndex], sbusAddr, AVAGO_ESB, divxCtl2Reg);
     AVAGO_UNLOCK(devNum, chipIndex);
     CHECK_AVAGO_RET_CODE();

     /* parse divx_cntl */
     divxHsSel  = (data1 & 0x8000) >> 15;
     divxHsHalf = (data1 & 0x4000) >> 14;
     divxHsHi   = (data1 & 0x3f00) >> 8;
     divxHsLo   = (data1 & 0x003f);

     /* parse divx2_cntl */
     divxInCntl = (data2 & 0x0300) >> 8;
     divxLsHalf = (data2 & 0x0080) >> 7;
     divxLsHi   = (data2 & 0x0070) >> 4;
     divxLsDiv1 = (data2 & 0x0008) >> 3;
     divxLsLo   = (data2 & 0x0007);

     /* get divx_pre from div_in_cntl */
     switch (divxInCntl)
     {
     case 0: divxPre = 2; break;
     case 1: divxPre = 2; break;
     case 2: divxPre = 4; break;
     case 3: divxPre = 5; break;
     default: divxPre = 0; break;
     }

     /* original equation: */
     /*    tb_divx_divider = (!divx_hs_sel & divx_ls_div1) ? (2 * divx_pre) : (divx_pre * (divx_half + (2 * (divx_hi + divx_lo)))); */
     if (divxHsSel == 0)
     {
         if (divxLsDiv1 == 1)
         {
             divider = 2 * divxPre;
         }
         else
         {
             divider = divxPre * (divxLsHalf + (2 * (divxLsHi + divxLsLo)));
         }
     }
     else
     {
         divider = divxPre * (divxHsHalf + (2 * (divxHsHi + divxHsLo)));
     }
     *dividerValue = divider/2;
#endif
     return GT_OK;
}

/**
* @internal mvHwsAvagoSerdeSpeedGet function
* @endinternal
*
* @brief   Return SERDES baud rate.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] rate                   - sereds speed
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdeSpeedGet
(
    IN GT_U8                 devNum,
    IN GT_U32                portGroup,
    IN GT_U32                serdesNum,
    OUT MV_HWS_SERDES_SPEED   *rate
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(rate);
#else

    GT_U32  sbus_addr;
    unsigned int chipIndex = devNum;
    GT_U32  divider, serdesSpeed;
    MV_HWS_CPLL_OUTPUT_FREQUENCY  outRefClk;
    GT_U32  refClk;
    GT_U32  data;
    GT_BOOL isCpllUsed;
    MV_HWS_DEV_TYPE devType;
    Avago_serdes_line_encoding_t txLineEncoding = AVAGO_SERDES_NRZ, rxLineEncoding = AVAGO_SERDES_NRZ;

    if (rate == NULL)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    devType = HWS_DEV_SILICON_TYPE(devNum);
    if ((devType == Raven) || (devType == Falcon))
    {
        CHECK_STATUS(mvHwsAvago16nmDividerGet(devNum,serdesNum,&divider));
        AVAGO_LOCK(devNum, chipIndex);
        avago_serdes_get_tx_rx_line_encoding(aaplSerdesDb[chipIndex], sbus_addr, &txLineEncoding, &rxLineEncoding);
        AVAGO_UNLOCK(devNum, chipIndex);
        CHECK_AVAGO_RET_CODE();
    }
    else
    {
        AVAGO_LOCK(devNum, chipIndex);
        divider = divider_lookup28nm(avago_serdes_mem_rd(aaplSerdesDb[chipIndex], sbus_addr, AVAGO_ESB, 0x220));
        AVAGO_UNLOCK(devNum, chipIndex);
        CHECK_AVAGO_RET_CODE();
    }

    /* Get the reference clock source */
    if((devType == Raven) || (devType == Falcon))
    {
        isCpllUsed = GT_FALSE;
    }
    else
    {
        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, &data, 0));

        if((devType == Bobcat3) || (devType == Pipe) || (devType == Aldrin2))
        {
            isCpllUsed = (((data >> 8) & 1) == 0) ? GT_TRUE : GT_FALSE;
        }
        else
        {
            /* BobK, Aldrin */
            isCpllUsed = (((data >> 8) & 1) == 1) ? GT_TRUE : GT_FALSE;
        }
    }
    if (isCpllUsed == GT_TRUE) /* CPLL */
    {
        CHECK_STATUS(mvHwsAvagoSerdeCpllOutputRefClkGet(devNum, portGroup, serdesNum, &outRefClk));

        switch (outRefClk)
        {
            case MV_HWS_156MHz_OUT:
                refClk = 156250;  /* 156.25Mhz */
                if (devType == Pipe)
                {
                    /* need to read reference clock ratio divider */
                    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, &data, (3 << 11)));
                    /* get bits 12:11 */
                    data = (data >> 11) & 3;
                    switch(data)
                    {
                        case 1:
                            refClk = 156250;  /* 156.25Mhz */
                            break;
                        case 2:
                            refClk = 78125;   /* 78.125Mhz */
                            break;
                        case 3:
                        default:
                            HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ARG_STRING_MAC("illegal clock ratio value\n"));
                    }
                }
                break;
            case MV_HWS_78MHz_OUT:
                refClk = 78125;   /* 78.125Mhz */
                break;
            case MV_HWS_312MHz_OUT:
                refClk = 312500;  /* 312.5Mhz */
                if ((devType == Bobcat3) || (devType == Pipe) || (devType == Raven) || (devType == Aldrin2))
                {
                    /* need to read reference clock ratio divider */
                    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, &data, (3 << 11)));
                    /* get bits 12:11 */
                    data = (data >> 11) & 3;
                    switch(data)
                    {
                        case 1:
                            refClk = 312500;  /* 312.5Mhz */
                            break;
                        case 2:
                            refClk = 156250;  /* 156.25Mhz */
                            break;
                        case 3:
                            refClk = 78125;   /* 78.125Mhz */
                            break;
                        default:
                            HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ARG_STRING_MAC("illegal clock ratio value\n"));
                    }
                }
                break;

            case MV_HWS_164MHz_OUT:
                refClk = 164242;  /* 164.24Mhz */
                if ((devType == Bobcat3) || (devType == Aldrin2))
                {
                    /* need to read reference clock ratio divider */
                    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, &data, (3 << 11)));
                    /* get bits 12:11 */
                    data = (data >> 11) & 3;
                    switch(data)
                    {
                        case 1:
                            refClk = 164242;  /* 164.24Mhz */
                            break;
                        default:
                            HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ARG_STRING_MAC("illegal clock ratio value\n"));
                    }
                }
                break;

            default:
                refClk = 0;
                return GT_NOT_INITIALIZED; /* this return value used in mvHwsPortInterfaceGet if port was not initialized */
        }
    }
    else /* External Ref Clock */
    {
        if ((devType == Bobcat3) || (devType == Pipe) || (devType == Aldrin2))
        {
            /* need to read reference clock ratio divider */
            CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, &data, (3 << 11)));
            /* get bits 12:11 */
            data = (data >> 11) & 3;
            switch (data)
            {
                case 1:
                    refClk = 156250; /* 156.25Mhz */
                    break;
                case 2:
                    refClk = 78125;  /* 78.125Mhz */
                    break;
                default:
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ARG_STRING_MAC("illegal clock ratio value\n"));
            }
        }
        else
        { /* BobK and Aldrin */
            refClk = 156250; /* 156.25Mhz */
        }
    }

    serdesSpeed = divider * refClk;
    switch (serdesSpeed)
    {
        case 1250000:
            *rate = _1_25G;
            break;
        case 3125000:
            *rate = _3_125G;
            break;
        case 5000000:
            *rate = _5G;
            break;
        case 5156250:
            *rate = _5_15625G;
            break;
        case 6250000:
            *rate = _6_25G;
            break;
        case 7500000:
            *rate = _7_5G;
            break;
        case 10312500:
            *rate = _10_3125G;
            break;
        case 12500000:
            *rate = _12_5G;
            break;
        case 10937500:
            *rate = _10_9375G;
            break;
        case 12187500:
            *rate = _12_1875G;
            break;
        case 12890625:
            *rate = _12_8906G;
            break;
        case 25781250:
            *rate = _25_78125G;
            break;
        case 27500000:
            *rate = _27_5G;
            break;
        case 20625000:
            *rate = _20_625G;
            break;
        case 26562500:
            switch (rxLineEncoding)
            {
            case AVAGO_SERDES_PAM4:
                *rate = _26_5625G_PAM4;
                break;
            case AVAGO_SERDES_NRZ:
                *rate = _26_5625G;
                break;
            }
            break;
        case 28281250:
            *rate = _28_28125G; /*TODO need to read NRZ*/
            break;
        case 26250000:
            *rate = _26_25G;
            break;
        case 27099930:
            *rate = _27_1G;
            break;
        default:
            *rate = SPEED_NA;
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ARG_STRING_MAC("illegal serdesSpeed\n"), LOG_ARG_GEN_PARAM_MAC(serdesSpeed));
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEomUiGet function
* @endinternal
*
* @brief   Return SERDES baud rate in Ui.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] baudRatePtr              - current system baud rate in pico-seconds.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEomUiGet
(
    IN GT_U8             devNum,
    IN GT_U32            portGroup,
    IN GT_U32            serdesNum,
    OUT GT_U32            *baudRatePtr
)
{

#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(baudRatePtr);
#else
    MV_HWS_SERDES_SPEED   rate;

    RETURN_GT_OK_ON_EMULATOR;

    CHECK_STATUS(mvHwsAvagoSerdeSpeedGet(devNum,portGroup,serdesNum,&rate));
    switch (rate)
    {
        case _1_25G:
            *baudRatePtr = 800;
            break;
        case _3_125G:
            *baudRatePtr = 320;
            break;
        case _5G:
            *baudRatePtr = 200;
            break;
        case _5_15625G:
            *baudRatePtr = 194;
            break;
        case _6_25G:
            *baudRatePtr = 160;
            break;
        case _10_3125G:
            *baudRatePtr = 97;
            break;
        case _10_9375G:
            *baudRatePtr = 91;
            break;
        case _12_5G:
            *baudRatePtr = 80;
            break;
        case _12_1875G:
            *baudRatePtr = 82;
            break;
        case _12_8906G:
            *baudRatePtr = 77;
            break;
        case _25_78125G:
            *baudRatePtr = 39;
            break;
        case _27_5G:
            *baudRatePtr = 36;
            break;
        case _26_25G:
            *baudRatePtr = 38;
            break;
        default:
            *baudRatePtr = 0;
            return GT_NOT_INITIALIZED;
    }
#endif /* ASIC_SIMULATION */
    return GT_OK;
}

#ifndef MV_HWS_AVAGO_NO_VOS_WA
/**
* @internal mvHwsAvagoSerdesVosOverrideModeSet function
* @endinternal
*
* @brief   Set the override mode of the serdeses VOS parameters. If the override mode
*         is set to true, the VOS parameters will be overriden, if set to false, they will
*         not be overriden.
* @param[in] devNum                   - device number
* @param[in] vosOverride              - GT_TRUE means override, GT_FALSE means not.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesVosOverrideModeSet
(
    IN GT_U8   devNum,
    IN GT_BOOL vosOverride
)
{
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesVosOverride = vosOverride;

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesVosOverrideModeGet function
* @endinternal
*
* @brief   Get the override mode of the serdeses VOS parameters. If the override mode
*         is set to true, the VOS parameters will be overriden, if set to false, they will
*         not be overriden.
* @param[in] devNum                   - device number
*
* @param[out] vosOverridePtr           - (pointer to) current VOS override mode.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesVosOverrideModeGet
(
    IN GT_U8   devNum,
    OUT GT_BOOL *vosOverridePtr
)
{
    if (vosOverridePtr == NULL)
    {
        return GT_BAD_PTR;
    }
    *vosOverridePtr = hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesVosOverride;

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesVosParamsGet function
* @endinternal
*
* @brief   Get the VOS Override parameters from the local DB.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesVosParamsGet
(
    IN unsigned char  devNum,
    OUT unsigned long  *vosParamsPtr
)
{
#ifndef ASIC_SIMULATION
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if(hwsAvagoNotSupportedCheck())
    {
        *vosParamsPtr = 0;
        /* no SERDESes on emulator */
        return GT_OK;
    }
#endif /* not MV_HWS_REDUCED_BUILD_EXT_CM3*/

    CHECK_STATUS(mvHwsAvagoSerdesManualVosParamsGet(devNum, vosParamsPtr));
#else
    /* avoid warnings */
    *vosParamsPtr = 0;
    GT_UNUSED_PARAM(devNum);
#endif

    return GT_OK;
}
#endif /*#ifndef MV_HWS_AVAGO_NO_VOS_WA*/

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
#ifndef CO_CPU_RUN
/**
* @internal mvHwsAvagoIfClose function
* @endinternal
*
* @brief   Release all system resources allocated by Serdes IF functions.

* @param[in] devNum                   - system device number
*/
void mvHwsAvagoIfClose
(
    IN GT_U8 devNum
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
#else
#if defined(CHX_FAMILY) || defined(PX_FAMILY)
    GT_U32  i;
#endif /* defined(CHX_FAMILY) || defined(PX_FAMILY) */
    GT_U32 chipIndex = devNum;

    CPSS_LOG_INFORMATION_MAC("Close AAPL for devNum %d\n", devNum);
    if (HWS_DEV_SILICON_TYPE(devNum) == Falcon)
    {
        for (chipIndex = 0; chipIndex < RAVEN_MAX_AVAGO_CHIP_NUMBER; chipIndex++)
        {
            if (aaplSerdesDb[chipIndex + (RAVEN_MAX_AVAGO_CHIP_NUMBER * devNum)] != NULL)
            {
                break;
            }
        }
        if (chipIndex == (GT_U32)(RAVEN_MAX_AVAGO_CHIP_NUMBER))
        {
            CPSS_LOG_INFORMATION_MAC("AAPL is already closed for devNum %d\n", devNum);
            return;
        }
    }
    else if(aaplSerdesDb[chipIndex] == NULL)
    {
        CPSS_LOG_INFORMATION_MAC("AAPL is already closed for devNum %d\n", devNum);
        return;
    }

    /* close AAPL */
    if (HWS_DEV_SILICON_TYPE(devNum) == Falcon)
    {
        for (chipIndex = 0; chipIndex < RAVEN_MAX_AVAGO_CHIP_NUMBER; chipIndex++)
        {
            if (aaplSerdesDb[chipIndex + (RAVEN_MAX_AVAGO_CHIP_NUMBER * devNum)] != NULL)
            {
                aapl_destruct(aaplSerdesDb[chipIndex + (RAVEN_MAX_AVAGO_CHIP_NUMBER * devNum)]);
                cpssOsFree(aaplSerdesDb[chipIndex + (RAVEN_MAX_AVAGO_CHIP_NUMBER * devNum)]);
                aaplSerdesDb[chipIndex + (RAVEN_MAX_AVAGO_CHIP_NUMBER * devNum)] = NULL;
            }
        }
    }
    else
    {
        aapl_destruct(aaplSerdesDb[chipIndex]);
        aaplSerdesDb[chipIndex] = NULL;
    }

#if defined(CHX_FAMILY) || defined(PX_FAMILY)
    /* if there is no active device delete Mutex */
    for (i = 0; i < HWS_MAX_DEVICE_NUM; i++)
    {
        if (aaplSerdesDb[i] != NULL)
        {
            return;
        }
    }

    /* Init Avago Access Protection in multi-process environment */
    /* This protection is implemented at the scope of the Host!! */
    /* ========================================================= */
    hwsOsMutexDelete(avagoAccessMutex);
    avagoAccessMutex = 0;
#endif /* defined(CHX_FAMILY) || defined(PX_FAMILY) */

#endif /* ASIC_SIMULATION */
}
#endif



/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetectByPhaseInitDb function
* @endinternal
*
* @brief   Init Hws DB of edge-detect-by-phase algorithm used by port manager.
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] timeOut                  - time Out
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetectByPhaseInitDb
(
    IN GT_U8   devNum,
    IN GT_U32  phyPortNum,
    IN GT_U32  timeOut
)
{
#ifdef ASIC_SIMULATION
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(phyPortNum);
    GT_UNUSED_PARAM(timeOut);
#else
    GT_U32 i;
    MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC   *edgeDetectStcPtr;

    edgeDetectStcPtr = hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->edgeDetectStcPtr;

    edgeDetectStcPtr->phase_breakLoopTotal = 0;
    edgeDetectStcPtr->phase_breakLoopInd = 0;

    edgeDetectStcPtr->phase_edgeDetectin_Flag = 0;
    edgeDetectStcPtr->phase_edgeDetectin_Sec = 0;
    edgeDetectStcPtr->phase_edgeDetectin_Usec = 0;

    edgeDetectStcPtr->phase_tmpTimeout = timeOut;

    edgeDetectStcPtr->phase_contextSwitchedBack = 0;

    for (i=0; i<MV_HWS_MAX_LANES_NUM_PER_PORT; i++)
    {
        edgeDetectStcPtr->phase_tvalBefore_tvSec[i] = 0;
        edgeDetectStcPtr->phase_tvalBefore_tvUsec[i] = 0;

        edgeDetectStcPtr->phase_firstEO[i] = 0;
        edgeDetectStcPtr->phase_enable[i] = 0;
        edgeDetectStcPtr->phase_breakLoop[i] = 0;

        edgeDetectStcPtr->phase_pCalTimeSec[i] = 0;
        edgeDetectStcPtr->phase_pCalTimeNano[i] = 0;
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetectPhase1 function
* @endinternal
*
* @brief   Set DFE dwell_time parameter. This is phase1 of the by-phase edge detect
*         algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] numOfSerdeses            - num of serdeses in the serdeses array
* @param[in] serdesArr                - serdeses array
* @param[in] threshEO                 - Eye monitor threshold
* @param[in] dwellTime                - define dwell value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetectPhase1
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      numOfSerdeses,
    IN GT_U32      *serdesArr,
    IN GT_32       threshEO,
    IN GT_U32      dwellTime
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(numOfSerdeses);
    GT_UNUSED_PARAM(serdesArr);
    GT_UNUSED_PARAM(dwellTime);
#else
    /*GT_U32   i;*/

    GT_U32 serdesNum;

    for (serdesNum=0; serdesNum<numOfSerdeses; serdesNum++)
    {
        /* select DFE tuning dwell time */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[serdesNum], 0x18, 0x7, NULL));

        /* big register write LSB */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[serdesNum], 0x19, 1<<dwellTime, NULL));

        /* big register write MSB */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[serdesNum], 0x19, 0, NULL));
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 1 ## serdesNum=%d \n", serdesArr[serdesNum]);
    }
#endif
    GT_UNUSED_PARAM(threshEO);

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetectPhase2_1 function
* @endinternal
*
* @brief   Launching pCal and take starting time for the pCal process. This is
*         phase2_1 of the by-phase edge detect algorithm.
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
* @param[in] portGroup                - port group (core) number
* @param[in] numOfSerdeses            - num of serdeses in the serdeses array
* @param[in] serdesArr                - serdeses array
* @param[in] threshEO                 - Eye monitor threshold
* @param[in] dwellTime                - define dwell value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetectPhase2_1
(
    IN GT_U8       devNum,
    IN GT_U32      portNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      numOfSerdeses,
    IN GT_U32      *serdesArr,
    IN GT_32       threshEO,
    IN GT_U32      dwellTime
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(numOfSerdeses);
    GT_UNUSED_PARAM(serdesArr);
    GT_UNUSED_PARAM(portNum);
#else
    GT_STATUS rc;
    GT_U32 serdesNum, tmpData;
    MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC   *edgeDetectStcPtr;

    edgeDetectStcPtr = hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[portNum]->edgeDetectStcPtr;

    for (serdesNum=0; serdesNum<numOfSerdeses &&
          edgeDetectStcPtr->phase_breakLoopTotal == GT_FALSE; serdesNum++)
    {
        if (edgeDetectStcPtr->phase_breakLoop[serdesNum]==GT_TRUE)
        {
            continue;
        }

        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 2 ## serdesNum=%d \n", serdesArr[serdesNum]);
        /*gettimeofday(&phase_tvalBefore[devNum][portNum][serdesNum], NULL); */
        edgeDetectStcPtr->phase_tvalBefore_tvSec[serdesNum] = 0;
        edgeDetectStcPtr->phase_tvalBefore_tvUsec[serdesNum] = 0;
        rc = hwsTimerGetFuncPtr(
            &edgeDetectStcPtr->phase_tvalBefore_tvSec[serdesNum],
            &edgeDetectStcPtr->phase_tvalBefore_tvUsec[serdesNum]);
        if (rc!=GT_OK)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ARG_STRING_MAC("HWS os timer API returned error"), LOG_ARG_GEN_PARAM_MAC(rc));
        }

        /* check that there is a Signal-detect before running the PCAL */
        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[serdesNum],
                                            AVAGO_EXTERNAL_STATUS,
            &tmpData, (1 << 2)));
        if (tmpData != 0)
        {
            return GT_NOT_READY;
        }

        /* PCAL also stop the START_ADAPTIVE if it was ran before */
        CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesArr[serdesNum], DFE_PCAL, NULL));
    }
#endif
    GT_UNUSED_PARAM(threshEO);
    GT_UNUSED_PARAM(dwellTime);

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetectPhase2_2 function
* @endinternal
*
* @brief   Calculate pCal time duration, check for timeout or for peer TxAmp change
*         (edge-detection). This is phase2_2 of the by-phase edge detect algorithm.
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
* @param[in] portGroup                - port group (core) number
* @param[in] numOfSerdeses            - num of serdeses in the serdeses array
* @param[in] serdesArr                - serdeses array
* @param[in] dwellTime                - define dwell value
* @param[in] continueExecute          - whether or not algorithm execution should be continued
* @param[in] isInNonBlockWaitPtr      - whether or not algorithm is in non-block waiting time
*
* @param[out] continueExecute          - whether or not algorithm execution should be continued
* @param[out] isInNonBlockWaitPtr      - whether or not algorithm is in non-block waiting time
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetectPhase2_2
(
    IN GT_U8       devNum,
    IN GT_U32      portNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      numOfSerdeses,
    IN GT_U32      *serdesArr,
    IN GT_32       threshEO,
    IN GT_U32      dwellTime,
    INOUT GT_BOOL     *continueExecute,
    INOUT GT_BOOL     *isInNonBlockWaitPtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(numOfSerdeses);
    GT_UNUSED_PARAM(serdesArr);
    GT_UNUSED_PARAM(portNum);
    GT_UNUSED_PARAM(continueExecute);
    GT_UNUSED_PARAM(isInNonBlockWaitPtr);
#else
    GT_U32 serdesNum;
    GT_STATUS rc;
    MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC   *edgeDetectStcPtr;

    edgeDetectStcPtr = hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[portNum]->edgeDetectStcPtr;

    /* normal execution */
    if ((edgeDetectStcPtr->phase_edgeDetectin_Flag & 0x1) ==0)
    {
        *isInNonBlockWaitPtr = GT_FALSE;
        for (serdesNum=(edgeDetectStcPtr->phase_edgeDetectin_Flag>>2); serdesNum < numOfSerdeses &&
              edgeDetectStcPtr->phase_breakLoopTotal == GT_FALSE; serdesNum++)
        {
            /* flag for returning to execution from the point where non-blocking-waiting
               was triggered (simlulating "context switch")*/
            if ( ((edgeDetectStcPtr->phase_edgeDetectin_Flag>>1) & 1) == 0)
            {
                if (edgeDetectStcPtr->phase_breakLoop[serdesNum] == GT_FALSE)
                {
                    GT_U32 phase_tvalAfter_tvSec, phase_tvalAfter_tvUsec;
                    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 4 ## serdesNum=%d \n", serdesArr[serdesNum]);
                    /* take the EO value */
                    CHECK_STATUS(mvHwsAvagoSerdesAutoTuneResult(devNum, portGroup, serdesArr[serdesNum],
                            &edgeDetectStcPtr->phase_tuneRes[serdesNum]));

                    /* get time of day */
                    phase_tvalAfter_tvSec = 0;
                    phase_tvalAfter_tvUsec = 0;
                    rc = hwsTimerGetFuncPtr(
                        &phase_tvalAfter_tvSec,
                        &phase_tvalAfter_tvUsec);
                    if (rc!=GT_OK)
                    {
                        HWS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ARG_STRING_MAC("HWS os timer API returned error"), LOG_ARG_GEN_PARAM_MAC(rc));
                    }

                    /* nanoseconds diff time accumulation. It needs to be accumulated
                       as it will be evaludated to 0 when dividing be a orderly big number
                       like 1000 to microsec or 1000000 to milisec */
                    if (phase_tvalAfter_tvSec >
                        edgeDetectStcPtr->phase_tvalBefore_tvSec[serdesNum])
                    {
                        edgeDetectStcPtr->phase_pCalTimeNano[serdesNum] +=
                        1000000000U - edgeDetectStcPtr->phase_tvalBefore_tvUsec[serdesNum] +
                            phase_tvalAfter_tvUsec;
                    }
                    else
                    {
                        edgeDetectStcPtr->phase_pCalTimeNano[serdesNum] +=
                            phase_tvalAfter_tvUsec -
                            edgeDetectStcPtr->phase_tvalBefore_tvUsec[serdesNum];
                    }
                    if (edgeDetectStcPtr->phase_pCalTimeNano[serdesNum]>1000000000U)
                    {
                        edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] +=1;
                        if (phase_tvalAfter_tvSec - edgeDetectStcPtr->phase_tvalBefore_tvSec[serdesNum] > 1)
                        {
                            edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] +=
                                (phase_tvalAfter_tvSec - edgeDetectStcPtr->phase_tvalBefore_tvSec[serdesNum]) - 1;
                        }
                        edgeDetectStcPtr->phase_pCalTimeNano[serdesNum] -= 1000000000U;
                    }

                    /* TIMEOUT CHECK */
                    if (edgeDetectStcPtr->phase_tmpTimeout <=
                        (GT_32)( ( edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] * 1000) +
                          (edgeDetectStcPtr->phase_pCalTimeNano[serdesNum]/1000000) ) )
                    {
                        edgeDetectStcPtr->phase_breakLoop[serdesNum] = GT_TRUE;
                        edgeDetectStcPtr->phase_breakLoopInd++;
                        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 4.5 ## TIMEOUT. serdesNum=%d \n", serdesArr[serdesNum]);
                    }

                    /* take the first EO value as reference */
                    if (edgeDetectStcPtr->phase_enable[serdesNum] == GT_FALSE
                        && edgeDetectStcPtr->phase_breakLoop[serdesNum] == GT_FALSE)
                    {
                        edgeDetectStcPtr->phase_firstEO[serdesNum] =
                            edgeDetectStcPtr->phase_tuneRes[serdesNum].avagoResults.EO;
                        edgeDetectStcPtr->phase_enable[serdesNum]  = GT_TRUE;
                    }

                    if (1 && edgeDetectStcPtr->phase_breakLoop[serdesNum] == GT_FALSE
                        /*((GT_32)(edgeDetectStcPtr->phase_firstEO[serdesNum] -
                                 edgeDetectStcPtr->phase_tuneRes[serdesNum].EO) > threshEO) ||
                        ((GT_32)(edgeDetectStcPtr->phase_firstEO[serdesNum] -
                                 edgeDetectStcPtr->phase_tuneRes[serdesNum].EO) < (-threshEO))*/)
                    {
                        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 5 ## serdesNum=%d \n", serdesArr[serdesNum]);
                        /* eye openning changed between first tune eye value and last tune eye value - delay in 20ms */
#if 0
                        hwsOsExactDelayPtr(devNum, portGroup, 20);
                        edgeDetectStcPtr->phase_breakLoop[serdesNum] = GT_TRUE;
                        edgeDetectStcPtr->phase_breakLoopInd++;
#endif
                        /* marking that the waiting time should begin, but doing it in non blocking way */
                        edgeDetectStcPtr->phase_edgeDetectin_Flag =
                            ( (GT_U8)serdesNum << 0x2)/*bits[2:7] for serdes number*/ | 0x1/* bit[0] for flag1*/ | 0x2/*bit[1] for flag2*/;
                        /* taking time for reference. this time will be the base time in whic 20 msec time will need to pass from
                           in order to continue execution */
                        /* those timers are not per serdes, because the reference edge detect behavior sleeps on a single
                           lane indication, and this non-blocking behavior should execute same seuqnece, no need per lane timers,
                           one set will do for a whole non-blocking-waiting sequence.
                           If one lane edge-detected, whole algorithm halts */
                        /*
                           NOTE: due the implementation of this algorithm, if one lane edge-detected and is going
                           to wait (non-block) than after the wait is finished and algorithm is continued,
                           other lanes can fall in the timeout as the wait took time. But it is fine as the wait already
                           been done due to the first lane, and other lanes waited anway. If use wants to wait to edge change
                           per lane, independant of other lanes, currently the timeout value should be multipled
                           by (edje-detected-wait-time * lanes amount). If requirements are changed, algorithm should be changed.
                           (implementation similar to legacy edge detect). Also the port manager training timeout
                           should bo considered. */
                        rc = hwsTimerGetFuncPtr(
                        &edgeDetectStcPtr->phase_edgeDetectin_Sec,
                        &edgeDetectStcPtr->phase_edgeDetectin_Usec);
                        if (rc!=GT_OK)
                        {
                            HWS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ARG_STRING_MAC("HWS os timer API returned error"), LOG_ARG_GEN_PARAM_MAC(rc));
                        }
                        *continueExecute = GT_TRUE;
                        /* those counters will be used in non-blocking-wait scope and should not be used again afterwards
                           because of the  edgeDetectStcPtr->phase_breakLoop[serdesNum] = GT_TRUE */
                        edgeDetectStcPtr->phase_pCalTimeNano[serdesNum] = 0;
                        edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] = 0;
                        *isInNonBlockWaitPtr = GT_TRUE;
                        return GT_OK;
                    }
                }
            }
            else
            {
                /* when we reached here, we finish non-blocking-waiting on an edge detected lane.
                   By here "context-switch-back" succesfully made as if the blocking wait just ended */
                /* marking flag2 (bit[1]) to zero and setting serdes valued (bits[2:7]) to zero,
                   flag1 (bit[0]) was already cleared in different scope (time non-blocking-waiting scope) */
                edgeDetectStcPtr->phase_edgeDetectin_Flag = 0;
                edgeDetectStcPtr->phase_contextSwitchedBack = 1;
            }
            AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 6 ## serdesNum=%d. breakInd=%d. sdNum=%d \n", serdesArr[serdesNum],
                                            edgeDetectStcPtr->phase_breakLoopInd, numOfSerdeses);

            if (edgeDetectStcPtr->phase_contextSwitchedBack && ((serdesNum +1) ==  numOfSerdeses) )
            {
                /* by here we finished this phase on all serdeses, and we are after non-block-wait
                   on a lane */
                *isInNonBlockWaitPtr = GT_FALSE;
                edgeDetectStcPtr->phase_contextSwitchedBack = 0;
            }

            /* if all serdeses finished */
            if (edgeDetectStcPtr->phase_breakLoopInd==numOfSerdeses)
            {
                edgeDetectStcPtr->phase_breakLoopTotal = GT_TRUE;
            }
        }

        if (edgeDetectStcPtr->phase_breakLoopTotal == GT_TRUE)
        {
            *continueExecute = GT_FALSE;

            for (serdesNum=0; serdesNum<numOfSerdeses; serdesNum++)
            {
                HWS_AVAGO_DBG(("tuneRes.EO=%d, SD=%d\n",
                        edgeDetectStcPtr->phase_tuneRes[serdesNum].avagoResults.EO,serdesArr[serdesNum]));
            }
        }
        else
        {
            *continueExecute = GT_TRUE;
        }
    }
    else /* serdes edge detection waiting execution */
    {
        GT_U32 nonBlockingWaitingScopeUsec,nonBlockingWaitingScopeSec;
        GT_U8 serdesNum = (edgeDetectStcPtr->phase_edgeDetectin_Flag>>2);
        *continueExecute = GT_TRUE;
        *isInNonBlockWaitPtr = GT_TRUE;
        rc = hwsTimerGetFuncPtr(
        &nonBlockingWaitingScopeSec,
        &nonBlockingWaitingScopeUsec);
        if (rc!=GT_OK)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ARG_STRING_MAC("HWS os timer API returned error"), LOG_ARG_GEN_PARAM_MAC(rc));
        }

        /* accumulating the nanoseconds */
        if (nonBlockingWaitingScopeSec >
                edgeDetectStcPtr->phase_edgeDetectin_Sec)
        {
            /* we can use here the phase_pCalTimeNano of the serdes parameter as by here the usage of it is done */
            edgeDetectStcPtr->phase_pCalTimeNano[serdesNum] +=
                1000000000U - edgeDetectStcPtr->phase_edgeDetectin_Usec +
                    nonBlockingWaitingScopeUsec;
        }
        else
        {
            /* we can use here the phase_pCalTimeNano of the serdes parameter as by here the usage of it is done */
            edgeDetectStcPtr->phase_pCalTimeNano[serdesNum] +=
                    nonBlockingWaitingScopeUsec - edgeDetectStcPtr->phase_edgeDetectin_Usec;
        }
        if (edgeDetectStcPtr->phase_pCalTimeNano[serdesNum]>1000000000U)
        {
            edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] +=1;
            if ((nonBlockingWaitingScopeSec - edgeDetectStcPtr->phase_edgeDetectin_Sec)>1)
            {
                edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] +=
                    (nonBlockingWaitingScopeSec - edgeDetectStcPtr->phase_edgeDetectin_Sec) - 1;
            }
            edgeDetectStcPtr->phase_pCalTimeNano[serdesNum] -= 1000000000U;
        }

        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## wait non-block ## serdesNum=%d START-CONTINUE. timestamp=%d\n",
                                        serdesArr[serdesNum],
                  ((edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] * 1000)
             + edgeDetectStcPtr->phase_pCalTimeNano[serdesNum]/1000000)
                );

        /* time passed from first taking time */
        if ( ((edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] * 1000)
             + edgeDetectStcPtr->phase_pCalTimeNano[serdesNum]/1000000) > (20 /* wait time in milisec */))
        {
            /* setting flag1 (bit[0]) to zero to mark that waiting time ended */
            edgeDetectStcPtr->phase_edgeDetectin_Flag &= 0xFE;
            edgeDetectStcPtr->phase_breakLoop[serdesNum] = GT_TRUE;
            edgeDetectStcPtr->phase_breakLoopInd++;
            AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## wait non-block ## serdesNum=%d FINISHED. timestamp=%d\n",
                                            serdesArr[serdesNum],
                        ((edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] * 1000)
                        + edgeDetectStcPtr->phase_pCalTimeNano[serdesNum]/1000000)
                            );
            return GT_OK;
        }
        edgeDetectStcPtr->phase_edgeDetectin_Sec = nonBlockingWaitingScopeSec;
        edgeDetectStcPtr->phase_edgeDetectin_Usec = nonBlockingWaitingScopeUsec;
    }

#endif /* ASIC_SIMULATION */
    GT_UNUSED_PARAM(threshEO);
    GT_UNUSED_PARAM(dwellTime);

    return GT_OK;
}

#ifndef ASIC_SIMULATION
static GT_U32 tvalBefore_tvSec=0;
static GT_U32 tvalBefore_tvUsec=0;
static GT_U32 tvalAfter_tvSec=0;
static GT_U32 tvalAfter_tvUsec=0;
#endif

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetect function
* @endinternal
*
* @brief   Detect TxAmp peak for positive or negative changing, then run iCal
*         and start adaptive pCal.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] timeout                  -  in miliseconds
* @param[in] threshEO                 - Eye monitor threshold
* @param[in] dwellTime                - define dwell value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetect
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_32       timeout,
    IN GT_32       threshEO,
    IN GT_U32      dwellTime
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(timeout);
    GT_UNUSED_PARAM(threshEO);
    GT_UNUSED_PARAM(dwellTime);
#else

    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT tuneRes;
    GT_32    firstEO=0;
    GT_U32   pCalTime;
    GT_BOOL  enable=GT_FALSE;
    GT_BOOL  breakLoop=GT_FALSE;
    GT_U32   i;
    GT_U32   sdDetect;
    MV_HWS_AUTO_TUNE_STATUS  rxStatus = TUNE_NOT_COMPLITED;
    GT_STATUS rc;

    /*struct timeval tvalBefore, tvalAfter;*/

    tvalBefore_tvSec = 0;
    tvalBefore_tvUsec = 0;
    tvalAfter_tvSec = 0;
    tvalAfter_tvUsec = 0;

    /* select DFE tuning dwell time */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x7, NULL));

    /* big register write LSB */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 1<<dwellTime, NULL));

    /* big register write MSB */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 0, NULL));

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 1 ## serdesNum=%d \n", serdesNum);
    do
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 2 ## serdesNum=%d \n", serdesNum);
        /*gettimeofday (&tvalBefore, NULL);*/
        rc = hwsTimerGetFuncPtr(&tvalBefore_tvSec,&tvalBefore_tvUsec);
        if (rc!=GT_OK)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ARG_STRING_MAC("HWS os timer API returned error"), LOG_ARG_GEN_PARAM_MAC(rc));
        }

        /* check that there is a Signal-detect before running the PCAL */
        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_EXTERNAL_STATUS, &sdDetect, (1 << 2)));
        if (sdDetect != 0)
        {
            return GT_NOT_READY;
        }

        /* PCAL also stop the START_ADAPTIVE if it was ran before */
        CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesNum, DFE_PCAL, NULL));

        rxStatus = TUNE_NOT_COMPLITED;

        for (i = 0; i < MV_AVAGO_TRAINING_TIMEOUT; i++)
        {
            /* Delay in 1ms */
            hwsOsExactDelayPtr(devNum, portGroup, 1);

            CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesNum, &rxStatus));
            if (rxStatus == TUNE_PASS)
            {
                break;
            }
            if (rxStatus == TUNE_FAIL)
            {
                HWS_AVAGO_DBG(("rxStatus is in TUNE_FAIL\n"));
                return GT_FAIL;
            }
            if (rxStatus == TUNE_NOT_COMPLITED && i > 300) {
                return GT_FAIL;
            }
        }

        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 4 ## serdesNum=%d \n", serdesNum);
        /* in case training failed or took too long/short */
        if ((rxStatus != TUNE_PASS) || (i == MV_AVAGO_TRAINING_TIMEOUT))
        {
            rxStatus = TUNE_FAIL;
            return GT_FAIL;
        }

        /* take the EO value */
        CHECK_STATUS(mvHwsAvagoSerdesAutoTuneResult(devNum, portGroup, serdesNum, &tuneRes));

        /*gettimeofday (&tvalAfter, NULL);*/
        rc = hwsTimerGetFuncPtr(&tvalAfter_tvSec,&tvalAfter_tvUsec);
        if (rc!=GT_OK)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ARG_STRING_MAC("HWS os timer API returned error"), LOG_ARG_GEN_PARAM_MAC(rc));
        }

        /* diff time in microseconds */
        /*pCalTime = (((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000000L + tvalAfter.tv_usec) - tvalBefore.tv_usec);*/
        pCalTime = (((tvalAfter_tvSec - tvalBefore_tvSec)*1000000L + (tvalAfter_tvUsec*1000)) - (tvalBefore_tvUsec*1000));
        pCalTime = pCalTime / 1000; /* diff time in ms */

        timeout = timeout - pCalTime;
        if (timeout <= 0)
        {
            breakLoop = GT_TRUE;
        }

        /* take the first EO value as reference */
        if (enable == GT_FALSE)
        {
            firstEO = tuneRes.avagoResults.EO;
            enable  = GT_TRUE;
        }

        if (((GT_32)(firstEO - tuneRes.avagoResults.EO) > threshEO) || ((GT_32)(firstEO - tuneRes.avagoResults.EO) < (-threshEO)))
        {
            AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 5 ## serdesNum=%d \n", serdesNum);
            /* delay in 20ms */
            hwsOsExactDelayPtr(devNum, portGroup, 20);
            breakLoop = GT_TRUE;
        }

        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 6 ## serdesNum=%d \n", serdesNum);
    } while (breakLoop == GT_FALSE);

    HWS_AVAGO_DBG(("Run CTLE training\n"));
    CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesNum, DFE_ICAL, NULL));

    for (i = 0; i < MV_AVAGO_TRAINING_TIMEOUT; i++)
    {
        /* Delay in 1ms */
        hwsOsExactDelayPtr(devNum, portGroup, 1);

        CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesNum, &rxStatus));
        if (rxStatus == TUNE_PASS)
        {
            break;
        }
        if (rxStatus == TUNE_FAIL)
        {
            HWS_AVAGO_DBG(("rxStatus is in TUNE_FAIL\n"));
            return GT_FAIL;
        }
        if (rxStatus == TUNE_NOT_COMPLITED && i > 300) {
                return GT_FAIL;
            }
    }

    /* check that there is a Signal-detect before running the PCAL */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_EXTERNAL_STATUS, &sdDetect, (1 << 2)));
    if (sdDetect != 0)
    {
        return GT_NOT_READY;
    }

    CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesNum, DFE_START_ADAPTIVE, NULL));

    /* Delay in 5ms */
    hwsOsExactDelayPtr(devNum, portGroup, 5);

    /* take the EO value */
    CHECK_STATUS(mvHwsAvagoSerdesAutoTuneResult(devNum, portGroup, serdesNum, &tuneRes));
    HWS_AVAGO_DBG(("tuneRes.EO=%d\n", tuneRes.avagoResults.EO));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
/**
* @internal mvHwsAvago16nmSerdesPmroMetricGet function
* @endinternal
*
* @brief   Measures process performance of the chip
*
* @param[in] devNum    - system device number
* @param[in] serdesNum - physical serdes number
*
* @param[out] metricPtr - pointer to a number indicating
*       propagation delay through a set of gates and
*       interconnects. A higher number indicates faster process
*       performance.
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvago16nmSerdesPmroMetricGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       serdesNum,
    OUT GT_U16       *metricPtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(metricPtr);
#else
    unsigned int    chipIndex;

    if(metricPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);

    RETURN_GT_OK_ON_EMULATOR;
    AVAGO_LOCK(devNum, chipIndex);
    *metricPtr = avago_pmro_get_metric(aaplSerdesDb[chipIndex], 19);
    AVAGO_UNLOCK(devNum, chipIndex);
#endif

    return GT_OK;
}
#endif
/*******************************************************************************
*
* @brief   Get the rx_signal_ok indication from o_core_status serdes macro.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
*
* @param[out] signalOk                 - (pointer to) whether or not rx_signal_ok is up
*
* @retval 0                        - on success
* @retval 1                        - on error
*
* @note rx_signal_ok is a consilidation of multiple conditions in the serdes.
*       It can be expresses as the following AND operation:
*       _________
*       CDR_LOCK --|     |
*       |  AND  |
*       RX_RDY  --| (logic |--> rx_signal_ok
*       | gate) |
*       RX_IDLE --|_________|
*
*/
GT_STATUS mvHwsAvagoSerdesRxSignalOkGet
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    OUT GT_BOOL     *signalOk
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    *signalOk = GT_TRUE;
#else
    GT_U32       data;

    RETURN_GT_OK_ON_EMULATOR;

    HWS_NULL_PTR_CHECK_MAC(signalOk);

    *signalOk = GT_FALSE;

    /* read rx_signal_ok indication from o_core_status[4] */
    CHECK_STATUS_EXT(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_CORE_STATUS, &data, (1 << 4)),
                    LOG_ARG_SERDES_IDX_MAC(serdesNum), LOG_ARG_STRING_MAC("reading o_core_status[4]"));

    *signalOk = (data ? GT_TRUE : GT_FALSE);
#endif

    return GT_OK;
}

/**
* @internal mvHwsAvago16nmSerdesHalFunc function
* @endinternal
*
* @brief   Invokes the indicated HAL function
*
* @param[in] devNum      - system device number
* @param[in] serdesNum   - serdes number
* @param[in] halFuncType - Type of HAL function
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvago16nmSerdesHalFunc
(
    IN GT_U8       devNum,
    IN GT_U32      serdesNum,
    IN MV_HWS_AVAGO_SERDES_HAL_FUNC_TYPES  halFuncType
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum      = devNum;
    serdesNum   = serdesNum;
    GT_UNUSED_PARAM(halFuncType);
#else

    unsigned int    sbus_addr;
    unsigned int    chipIndex = devNum;
    Avago_serdes_hal_func_type_t    halFunc;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum,serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    if (HWS_DEV_SILICON_TYPE(devNum) != Raven && HWS_DEV_SILICON_TYPE(devNum) != Falcon)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ARG_STRING_MAC("mvHwsAvago16nmSerdesHalFunc is NOT_IMPLEMENTED for this device\n"));
    }

    RETURN_GT_OK_ON_EMULATOR;

    switch (halFuncType)
    {
        case HWS_SERDES_HAL_RXCLK_VERNIER_APPLY:
            halFunc = AVAGO_HAL_RXCLK_VERNIER_APPLY;
            break;
        case HWS_SERDES_HAL_TXEQ_FFE_APPLY:
            halFunc = AVAGO_HAL_TXEQ_FFE_APPLY;
            break;
        case HWS_SERDES_HAL_DATA_CHANNEL_APPLY:
            halFunc = AVAGO_HAL_DATA_CHANNEL_APPLY;
            break;
        case HWS_SERDES_HAL_TEST_CHANNEL_APPLY:
            halFunc = AVAGO_HAL_TEST_CHANNEL_APPLY;
            break;
        case HWS_SERDES_HAL_RXEQ_FFE_APPLY:
            halFunc = AVAGO_HAL_RXEQ_FFE_APPLY;
            break;
        case HWS_SERDES_HAL_RXEQ_CTLE_APPLY:
            halFunc = AVAGO_HAL_RXEQ_CTLE_APPLY;
            break;
        case HWS_SERDES_HAL_RXEQ_DFE_APPLY:
            halFunc = AVAGO_HAL_RXEQ_DFE_APPLY;
            break;

        default:
            return GT_BAD_PARAM;
    }
    AVAGO_LOCK(devNum, chipIndex);
    avago_serdes_hal_func(aaplSerdesDb[chipIndex], sbus_addr, halFunc);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();
#endif

    return GT_OK;
}

#endif /*MV_HWS_REDUCED_BUILD_EXT_CM3*/

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT)
/**
* @internal mvHwsAvago16nmSerdesHalGet function
* @endinternal
*
* @brief   Retrieves the indicated HAL register value
*
* @param[in] devNum    - system device number
* @param[in] serdesNum - serdes number
* @param[in] halType   - HAL types
* @param[in] regSelect - HAL register selector
*
* @param[out] halRegValue - HAL register value
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvago16nmSerdesHalGet
(
    IN GT_U8                           devNum,
    IN GT_U32                          serdesNum,
    IN MV_HWS_AVAGO_SERDES_HAL_TYPES   halType,
    IN GT_U32                          regSelect,
    OUT GT_32                           *halRegValue
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(halType);
    GT_UNUSED_PARAM(regSelect);
    GT_UNUSED_PARAM(halRegValue);
#else

    GT_U32 sbus_addr;
    unsigned int            chipIndex = devNum;
    Avago_serdes_hal_type_t serdesHalType;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum,serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    if (HWS_DEV_SILICON_TYPE(devNum) != Raven && HWS_DEV_SILICON_TYPE(devNum) != Falcon)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ARG_STRING_MAC("mvHwsAvago16nmSerdesHalGet is NOT_IMPLEMENTED for this device\n"));
    }

    HWS_NULL_PTR_CHECK_MAC(halRegValue);

    switch (halType)
    {
        case HWS_SERDES_GLOBAL_TUNE_PARAMS:
            serdesHalType = AVAGO_HAL_GLOBAL_TUNE_PARAMS;
            break;
        case HWS_SERDES_M4_TUNE_PARAMS:
            serdesHalType = AVAGO_HAL_M4_TUNE_PARAMS;
            break;
        case HWS_SERDES_RXCLK_SELECT:
            serdesHalType = AVAGO_HAL_RXCLK_SELECT;
            break;
        case HWS_SERDES_TXEQ_FFE:
            serdesHalType = AVAGO_HAL_TXEQ_FFE;
            break;
        case HWS_SERDES_RXEQ_CTLE:
            serdesHalType = AVAGO_HAL_RXEQ_CTLE;
            break;
        case HWS_SERDES_RXEQ_DFE:
            serdesHalType = AVAGO_HAL_RXEQ_DFE;
            break;
        case HWS_SERDES_RXEQ_FFE:
            serdesHalType = AVAGO_HAL_RXEQ_FFE;
            break;
        case HWS_SERDES_DATA_CHANNEL_INPUTS:
            serdesHalType = AVAGO_HAL_DATA_CHANNEL_INPUTS;
            break;
        case HWS_SERDES_DATA_CHANNEL_OFFSET:
            serdesHalType = AVAGO_HAL_DATA_CHANNEL_OFFSET;
            break;
        case HWS_SERDES_DATA_CHANNEL_CAL:
            serdesHalType = AVAGO_HAL_DATA_CHANNEL_CAL;
            break;
        case HWS_SERDES_TEST_CHANNEL_INPUTS:
            serdesHalType = AVAGO_HAL_TEST_CHANNEL_INPUTS;
            break;
        case HWS_SERDES_TEST_CHANNEL_OFFSET:
            serdesHalType = AVAGO_HAL_TEST_CHANNEL_OFFSET;
            break;
        case HWS_SERDES_TEST_CHANNEL_CAL:
            serdesHalType = AVAGO_HAL_TEST_CHANNEL_CAL;
            break;
        case HWS_SERDES_PAM4_EYE:
            serdesHalType = AVAGO_HAL_PAM4_EYE;
            break;
        case HWS_SERDES_PAM4_LEVELS:
            serdesHalType = AVAGO_HAL_PAM4_LEVELS;
            break;
        case HWS_SERDES_GRADIENT_INPUTS:
            serdesHalType = AVAGO_HAL_GRADIENT_INPUTS;
            break;
        case HWS_SERDES_DATA_CHANNEL_CAL_DELTA:
            serdesHalType = AVAGO_HAL_DATA_CHANNEL_CAL_DELTA;
            break;
        case HWS_SERDES_CLK_VERNIER_CAL:
            serdesHalType = AVAGO_HAL_CLK_VERNIER_CAL;
            break;
        case HWS_SERDES_TEMPERATURE:
            serdesHalType = 0x28; /* not defined in Avago_serdes_hal_type_t */
            break;
        case HWS_SERDES_ENVELOPE_CONFIG:
            serdesHalType = 0x43; /* not defined in Avago_serdes_hal_type_t */
            break;
        case HWS_SERDES_KR_TRAINING_CONFIG:
            serdesHalType = 0x3A; /* kr training */
            break;
        case HWS_SERDES_PLL_GAIN:
            serdesHalType = 0x41; /* pll gain */
            break;

        default:
            return GT_BAD_PARAM;
    }
    AVAGO_LOCK(devNum, chipIndex);
    *halRegValue = avago_serdes_hal_get(aaplSerdesDb[chipIndex], sbus_addr, serdesHalType, regSelect);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();
#endif

    return GT_OK;
}

/**
* @internal mvHwsAvago16nmSerdesHalSetIfValid function
* @endinternal
*
* @brief   Sets the indicated HAL register value
*
* @param[in] devNum    -     system device number
* @param[in] serdesNum -     serdes number
* @param[in] halType   -     HAL types
* @param[in] regSelect -     HAL register selector
* @param[in] registerValue - Value to write into the register
* @param[in] invalidDef -    Skip writing if
*                             registerValue == invalidDef
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvago16nmSerdesHalSetIfValid
(
    IN GT_U8                           devNum,
    IN GT_U32                          serdesNum,
    IN MV_HWS_AVAGO_SERDES_HAL_TYPES   halType,
    IN GT_U32                          regSelect,
    IN GT_32                           registerValue,
    IN GT_U32                          invalidDef
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(halType);
    GT_UNUSED_PARAM(regSelect);
    GT_UNUSED_PARAM(registerValue);
    GT_UNUSED_PARAM(invalidDef);
#else
    if (((GT_U32)(registerValue)) != invalidDef)
    {
        if(invalidDef == NA_8BIT_SIGNED)
        {
            CHECK_STATUS(mvHwsAvago16nmSerdesHalSet(devNum,serdesNum,halType,regSelect,SIGNED8_TO_SIGNED32(registerValue)));
        }
        else if(invalidDef == NA_16BIT_SIGNED)
        {
            CHECK_STATUS(mvHwsAvago16nmSerdesHalSet(devNum,serdesNum,halType,regSelect,SIGNED16_TO_SIGNED32(registerValue)));
        }
        else
        {
            CHECK_STATUS(mvHwsAvago16nmSerdesHalSet(devNum,serdesNum,halType,regSelect,registerValue));
        }
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsAvago16nmSerdesHalSet function
* @endinternal
*
* @brief   Sets the indicated HAL register value
*
* @param[in] devNum    - system device number
* @param[in] serdesNum - serdes number
* @param[in] halType   - HAL types
* @param[in] regSelect - HAL register selector
* @param[in] registerValue - Value to write into the register
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvago16nmSerdesHalSet
(
    IN GT_U8                           devNum,
    IN GT_U32                          serdesNum,
    IN MV_HWS_AVAGO_SERDES_HAL_TYPES   halType,
    IN GT_U32                          regSelect,
    IN GT_32                           registerValue
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(halType);
    GT_UNUSED_PARAM(regSelect);
    GT_UNUSED_PARAM(registerValue);
#else

    GT_U32 sbus_addr;
    unsigned int            chipIndex = devNum;
    Avago_serdes_hal_type_t serdesHalType=0;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum,serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    if ((HWS_DEV_SILICON_TYPE(devNum) != Raven) && (HWS_DEV_SILICON_TYPE(devNum) != Falcon))
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ARG_STRING_MAC("mvHwsAvago16nmSerdesHalSet is NOT_IMPLEMENTED for this device\n"));
    }

    switch (halType)
    {
        case HWS_SERDES_GLOBAL_TUNE_PARAMS:
            serdesHalType = AVAGO_HAL_GLOBAL_TUNE_PARAMS;
            break;
        case HWS_SERDES_M4_TUNE_PARAMS:
            serdesHalType = AVAGO_HAL_M4_TUNE_PARAMS;
            break;
        case HWS_SERDES_RXCLK_SELECT:
            serdesHalType = AVAGO_HAL_RXCLK_SELECT;
            break;
        case HWS_SERDES_TXEQ_FFE:
            serdesHalType = AVAGO_HAL_TXEQ_FFE;
            break;
        case HWS_SERDES_RXEQ_CTLE:
            serdesHalType = AVAGO_HAL_RXEQ_CTLE;
            break;
        case HWS_SERDES_RXEQ_DFE:
            serdesHalType = AVAGO_HAL_RXEQ_DFE;
            break;
        case HWS_SERDES_RXEQ_FFE:
            serdesHalType = AVAGO_HAL_RXEQ_FFE;
            break;
        case HWS_SERDES_DATA_CHANNEL_INPUTS:
            serdesHalType = AVAGO_HAL_DATA_CHANNEL_INPUTS;
            break;
        case HWS_SERDES_DATA_CHANNEL_OFFSET:
            serdesHalType = AVAGO_HAL_DATA_CHANNEL_OFFSET;
            break;
        case HWS_SERDES_DATA_CHANNEL_CAL:
            serdesHalType = AVAGO_HAL_DATA_CHANNEL_CAL;
            break;
        case HWS_SERDES_TEST_CHANNEL_INPUTS:
            serdesHalType = AVAGO_HAL_TEST_CHANNEL_INPUTS;
            break;
        case HWS_SERDES_TEST_CHANNEL_OFFSET:
            serdesHalType = AVAGO_HAL_TEST_CHANNEL_OFFSET;
            break;
        case HWS_SERDES_TEST_CHANNEL_CAL:
            serdesHalType = AVAGO_HAL_TEST_CHANNEL_CAL;
            break;
        case HWS_SERDES_PAM4_EYE:
            serdesHalType = AVAGO_HAL_PAM4_EYE;
            break;
        case HWS_SERDES_PAM4_LEVELS:
            serdesHalType = AVAGO_HAL_PAM4_LEVELS;
            break;
        case HWS_SERDES_GRADIENT_INPUTS:
            serdesHalType = AVAGO_HAL_GRADIENT_INPUTS;
            break;
        case HWS_SERDES_DATA_CHANNEL_CAL_DELTA:
            serdesHalType = AVAGO_HAL_DATA_CHANNEL_CAL_DELTA;
            break;
        case HWS_SERDES_CLK_VERNIER_CAL:
            serdesHalType = AVAGO_HAL_CLK_VERNIER_CAL;
            break;
        case HWS_SERDES_TEMPERATURE:
            serdesHalType = 0x28; /* not defined in Avago_serdes_hal_type_t */
            break;
        case HWS_SERDES_ENVELOPE_CONFIG:
            serdesHalType = 0x43; /* not defined in Avago_serdes_hal_type_t */
            break;
        case HWS_SERDES_KR_TRAINING_CONFIG:
            serdesHalType = 0x3A; /* kr training */
            break;
        case HWS_SERDES_PLL_GAIN:
            serdesHalType = 0x41; /* pll gain */
            break;
        default:
            return GT_BAD_PARAM;
    }
    AVAGO_LOCK(devNum, chipIndex);
    avago_serdes_hal_set(aaplSerdesDb[chipIndex], sbus_addr, serdesHalType, regSelect, registerValue);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();
#endif

    return GT_OK;
}


/**
* @internal mvHwsAvago16nmSerdesManualRxConfig function
* @endinternal
*
* @brief   Set the Serdes Manual Rx config
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] rxConfigPtr              - Rx config data
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvago16nmSerdesManualRxConfig
(
    IN GT_U8                                devNum,
    IN GT_UOPT                              portGroup,
    IN GT_UOPT                              serdesNum,
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT    *rxConfigPtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(rxConfigPtr);
#else
    GT_U32 sbus_addr;
    unsigned int chipIndex;
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT    ctleData;

    hwsOsMemSetFuncPtr(&ctleData, 0, sizeof(ctleData));
    ctleData.rxAvago.dcGain         = (GT_U16)rxConfigPtr->rxAvago16nm.dcGain;
    ctleData.rxAvago.lowFrequency   = (GT_U16)rxConfigPtr->rxAvago16nm.lowFrequency;
    ctleData.rxAvago.highFrequency  = (GT_U16)rxConfigPtr->rxAvago16nm.highFrequency;
    ctleData.rxAvago.bandWidth      = (GT_U16)rxConfigPtr->rxAvago16nm.bandWidth;
    ctleData.rxAvago.squelch        = (GT_U16)rxConfigPtr->rxAvago16nm.squelch;
    ctleData.rxAvago.gainshape1     = (GT_U8) rxConfigPtr->rxAvago16nm.gainShape1;
    ctleData.rxAvago.gainshape2     = (GT_U8) rxConfigPtr->rxAvago16nm.gainShape2;
    ctleData.rxAvago.shortChannelEn = (GT_BOOL)rxConfigPtr->rxAvago16nm.shortChannelEn;

    CHECK_STATUS(mvHwsAvagoSerdesManualCtleConfig(devNum, portGroup, serdesNum, &ctleData));
    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* basic */
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_GLOBAL_TUNE_PARAMS, 0x9, rxConfigPtr->rxAvago16nm.ffeFix,           NA_8BIT );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_RXEQ_FFE,           0x3 ,rxConfigPtr->rxAvago16nm.bfLf,             NA_8BIT_SIGNED );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_RXEQ_FFE,           0x4 ,rxConfigPtr->rxAvago16nm.bfHf,             NA_8BIT_SIGNED );

    /* advanced */
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_RXEQ_CTLE,          0x9,  rxConfigPtr->rxAvago16nm.minLf,           NA_8BIT        );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_RXEQ_CTLE,          0xA,  rxConfigPtr->rxAvago16nm.maxLf,           NA_8BIT        );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_RXEQ_CTLE,          0x7,  rxConfigPtr->rxAvago16nm.minHf,           NA_8BIT        );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_RXEQ_CTLE,          0x8,  rxConfigPtr->rxAvago16nm.maxHf,           NA_8BIT        );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_RXEQ_FFE,           0x7,  rxConfigPtr->rxAvago16nm.minPre1,         NA_8BIT_SIGNED );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_RXEQ_FFE,           0x8,  rxConfigPtr->rxAvago16nm.maxPre1,         NA_8BIT_SIGNED );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_RXEQ_FFE,           0x9,  rxConfigPtr->rxAvago16nm.minPre2,         NA_8BIT_SIGNED );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_RXEQ_FFE,           0xA,  rxConfigPtr->rxAvago16nm.maxPre2,         NA_8BIT_SIGNED );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_RXEQ_FFE,           0xC,  rxConfigPtr->rxAvago16nm.minPost,         NA_8BIT_SIGNED );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_RXEQ_FFE,           0xD,  rxConfigPtr->rxAvago16nm.maxPost,         NA_8BIT_SIGNED );

    /* main */
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_GLOBAL_TUNE_PARAMS, 0x18, rxConfigPtr->rxAvago16nm.iCalEffort,      NA_8BIT        );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_GLOBAL_TUNE_PARAMS, 0x0D, rxConfigPtr->rxAvago16nm.pCalEffort,      NA_8BIT        );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_RXEQ_FFE,           0x5,  rxConfigPtr->rxAvago16nm.dfeDataRate,     NA_8BIT        );
    if (NA_8BIT != rxConfigPtr->rxAvago16nm.dfeCommon) mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x809e,
                                                                                            ((rxConfigPtr->rxAvago16nm.dfeCommon<<2) + 0x220), NULL    );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_GLOBAL_TUNE_PARAMS, 0x1D, rxConfigPtr->rxAvago16nm.int11d,          NA_16BIT       );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_GLOBAL_TUNE_PARAMS, 0x17, rxConfigPtr->rxAvago16nm.int17d,          NA_16BIT       );


    /* misc */
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_GRADIENT_INPUTS,    0x5,  rxConfigPtr->rxAvago16nm.pre1PosGradient, NA_16BIT_SIGNED);
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_GRADIENT_INPUTS,    0x6,  rxConfigPtr->rxAvago16nm.pre1NegGradient, NA_16BIT_SIGNED);
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_GRADIENT_INPUTS,    0xe,  rxConfigPtr->rxAvago16nm.pre2PosGradient, NA_16BIT_SIGNED);
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_GRADIENT_INPUTS,    0xf,  rxConfigPtr->rxAvago16nm.pre2NegGradient, NA_16BIT_SIGNED);
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_GRADIENT_INPUTS,    0xc,  rxConfigPtr->rxAvago16nm.hfPosGradient,   NA_16BIT_SIGNED);
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_GRADIENT_INPUTS,    0xd,  rxConfigPtr->rxAvago16nm.hfNegGradient,   NA_16BIT_SIGNED);
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_GRADIENT_INPUTS,    0x1,  rxConfigPtr->rxAvago16nm.agcTargetLow,    NA_8BIT        );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_GRADIENT_INPUTS,    0x2,  rxConfigPtr->rxAvago16nm.agcTargetHigh,   NA_8BIT        );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_ENVELOPE_CONFIG,    0x1,  rxConfigPtr->rxAvago16nm.coldEnvelope,    NA_8BIT        );
    mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_ENVELOPE_CONFIG,    0x2,  rxConfigPtr->rxAvago16nm.hotEnvelope,     NA_8BIT        );

    /* issue interrupt to set termination */
    if(rxConfigPtr->rxAvago16nm.termination != NA_8BIT)
    {
        AVAGO_LOCK(devNum, chipIndex);
        avago_serdes_set_rx_term(aaplSerdesDb[chipIndex], sbus_addr, rxConfigPtr->rxAvago16nm.termination);
        AVAGO_UNLOCK(devNum, chipIndex);
    }

#endif /* ASIC_SIMULATION */
    return GT_OK;
}

#endif /* #if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT) */

/*******************************************************************************/
/******************************* DEBUG functions *******************************/
/*******************************************************************************/

#if 0 /*Not used*/
#ifndef ASIC_SIMULATION
/**
* @internal mvHwsAvagoSerdesAaplEnableLog function
* @endinternal
*
* @brief   Get the AAPL log information according to the debug
*          level.
*
* @note   Works only after Device initialization
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAvagoSerdesAaplEnableLog(GT_U8 chipIndex, GT_BOOL enableLog, GT_U8 debugLevel)
{
    aaplEnableLog = enableLog;
    aaplSerdesDb[chipIndex]->debug = debugLevel;

    return GT_OK;
}
#endif /* ASIC_SIMULATION */
#endif /*Not used*/

#if ((!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) ||(defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)))
#ifdef RUN_ADAPTIVE_CTLE

/**
* @internal mvHwsAvagoAdaptiveCtlePortEnableSet function
* @endinternal
*
* @brief  enable adaptive ctle on specific port
*
* @param[in] devNum                   - system device number
* @param[in] portNum                   - port number
* @param[in] devNum                   - GT_TRUE - enable
*                                       GT_FALSE - disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoAdaptiveCtlePortEnableSet
(
    IN GT_U8   devNum,
    IN GT_U32  phyPortNum,
    IN GT_BOOL enable
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(phyPortNum);
    GT_UNUSED_PARAM(enable);
#else

    GT_U32 bitMapIndex;
    bitMapIndex = phyPortNum / HWS_MAX_ADAPT_CTLE_DB_SIZE;

    if (enable)
    {
        hwsDeviceSpecInfo[devNum].adaptiveCtleDb.bitMapEnableCtleCalibrationBasedTemperture[bitMapIndex] |= (0x1 << (phyPortNum%HWS_MAX_ADAPT_CTLE_DB_SIZE));
    }
    else
    {
         hwsDeviceSpecInfo[devNum].adaptiveCtleDb.bitMapEnableCtleCalibrationBasedTemperture[bitMapIndex] &= ~(0x1 << (phyPortNum%HWS_MAX_ADAPT_CTLE_DB_SIZE));
    }

#endif /*ASIC_SIMULATION*/
    return GT_OK;

}



/**
* @internal mvHwsAvagoAdaptiveCtleSerdesesInitDb function
* @endinternal
*
* @brief   Init Hws DB of adaptive ctle values.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoAdaptiveCtleSerdesesInitDb
(
    IN GT_U8   devNum
)
{

#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
#else

    GT_U32 ii;
    GT_U32 loop;
#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
    loop = ((MV_PORT_CTRL_MAX_AP_PORT_NUM+(HWS_MAX_ADAPT_CTLE_DB_SIZE-1))/HWS_MAX_ADAPT_CTLE_DB_SIZE);
#else
    loop = ((MV_HWS_MAX_PORT_NUM+(HWS_MAX_ADAPT_CTLE_DB_SIZE -1))/HWS_MAX_ADAPT_CTLE_DB_SIZE);
#endif
    /* TODO: disable Adaptive CTLE , until approval of the feature*/
    loop = 0;
    /* enable adaptive ctle per port*/
    for (ii = 0 ; ii < loop; ii ++)
    {
        /*hwsOsMemSetFuncPtr(&(hwsDeviceSpecInfo[devNum].adaptiveCtleDb.bitMapEnableCtleCalibrationBasedTemperture[ii]),1,sizeof(GT_U8));*/
        hwsDeviceSpecInfo[devNum].adaptiveCtleDb.bitMapEnableCtleCalibrationBasedTemperture[ii] = 0x7f;
        hwsDeviceSpecInfo[devNum].adaptiveCtleDb.bitMapEnableCtleCalibrationBasedTemperture[ii] |= (0x1<<7);
    }

#ifdef ADAPT_CTLE_DEBUG
     hwsOsPrintf("\nport | SD | oldLF | newLF | oldDelay | newDelay | temp | oldEO | newEO |\n");
     hwsOsPrintf("------------------------------------------------------------------------\n");
#endif

#endif /*ASIC_SIMULATION*/
    return GT_OK;
}


/**
* @internal mvHwsAvagoAdaptiveCtleCalcNewLfNewDelayBasedTemperature function
* @endinternal
*
* @brief   Set serdes parametrs LF, Delay and EO.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] serdesArr                - port's serdeses arrray
* @param[in] numOfSerdes              - serdeses number
* @param[in] newLfArr                 -  array of serdes LF
* @param[in] newDelayArr              -  array of serdes delays
*
* @retval 0                        - on success
* @retval 1                        - on error
*/

static GT_STATUS mvHwsAvagoAdaptiveCtleCalcNewLfNewDelayBasedTemperature
(
    IN GT_U8                           devNum,
    IN GT_UOPT                         portGroup,
    IN GT_U16                          *serdesArr,
    IN GT_UOPT                         numOfSerdes,
    IN GT_U8                          *newLfArr,
    IN GT_U8                          *newDelayArr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesArr);
    GT_UNUSED_PARAM(numOfSerdes);
    GT_UNUSED_PARAM(newLfArr);
    GT_UNUSED_PARAM(newDelayArr);
#else
    GT_U8 ii;
    GT_8 deltaDelay;
    GT_8 tempLf;
    GT_8 tempDelay;
    GT_32 temperature;
    GT_STATUS rc;
    GT_U32 chipIndex = devNum;

    /* get temperture. for FREE RTOS the temperture read again every 5 seconds. for non FREE RTOS we read the temperture every time*/
    rc = mvHwsPortTemperatureGet(devNum,portGroup, chipIndex, &temperature);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*deltaDelay = ((angleVar * temperature) + offsetVar)/100)*/
    deltaDelay  = (GT_8)(((4*((GT_16)temperature))-145)/100);
    if ((deltaDelay > 2) || (deltaDelay < -2))
    {
        return GT_FAIL;
    }

    if (numOfSerdes > MAX_AVAGO_SERDES_NUMBER)
    {
        return GT_BAD_PARAM;
    }

    for (ii = 0; ii < numOfSerdes; ii++)
    {
        /* delay*/
        tempDelay = ((hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesArr[ii]].enhTrainDelay + deltaDelay));
        tempDelay = (tempDelay > ADAPTIVE_CTLE_MAX_DELAY ? ADAPTIVE_CTLE_MAX_DELAY : tempDelay);
        tempDelay = (tempDelay < ADAPTIVE_CTLE_MIN_DELAY ? ADAPTIVE_CTLE_MIN_DELAY : tempDelay);
        newDelayArr[serdesArr[ii]] = (GT_U8)tempDelay;

        /* LF */
        tempLf  = (hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesArr[ii]].trainLf - (deltaDelay/2));
        tempLf = (tempLf > ADAPTIVE_CTLE_MAX_LF ? ADAPTIVE_CTLE_MAX_LF : tempLf);
        tempLf = (tempLf < ADAPTIVE_CTLE_MIN_LF ? ADAPTIVE_CTLE_MIN_LF : tempLf);
        newLfArr[serdesArr[ii]] = (GT_U8)tempLf;
    }

#endif /*ASIC_SIMULATION*/

    return GT_OK;
}


/**
* @internal mvHwsAvagoAdaptiveCtleSerdesLfAndDelaySet function
* @endinternal
*
* @brief   Set serdes parametrs LF, Delay and EO for specific
*          serdes index.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] numOfSerdes              - serdeses number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAvagoAdaptiveCtleSerdesLfAndDelaySet
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesIndex
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesIndex);
#else
    GT_U32 intValue;
    GT_U32 initSerdesIndex = (GT_U32)serdesIndex;
    GT_U32 lf;
    GT_U32 delay;

    if (serdesIndex > MAX_AVAGO_SERDES_NUMBER)
    {
        return GT_BAD_PARAM;
    }

    lf = (GT_U32)(hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesIndex].oldLf);
    delay = (GT_U32)(hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesIndex].oldDelay);

    /* LF*/
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, initSerdesIndex, 0x26, ((2 << 12) | (1 << 8) | lf ), NULL));

    /* Delay*/
    intValue = (delay |(delay << 4));
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, initSerdesIndex, 0x18, 0x402e, NULL));
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, initSerdesIndex, 0x19, intValue, NULL));

    /* save serdes delay in DB for next period */
    hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesIndex].currSerdesDelay = (GT_U16)delay;

#endif /*ASIC_SIMULATION*/
    return GT_OK;
}


/**
* @internal mvHwsAvagoAdaptiveCtleSerdesesLfAndDelaySet function
* @endinternal
*
* @brief   Set serdes parametrs LF, Delay and EO.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] serdesArr                - port's serdeses arrray
* @param[in] numOfSerdes              - serdeses number
* @param[in] newDelayArr            -  array of serdes delays
* @param[in] newLfArr               -  array of serdes LF
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAvagoAdaptiveCtleSerdesesLfAndDelaySet
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U16      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    IN GT_U8      *newDelayArr,
    IN GT_U8      *newLfArr
)
{
/* if before before*/
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesArr);
    GT_UNUSED_PARAM(numOfSerdes);
    GT_UNUSED_PARAM(newDelayArr);
    GT_UNUSED_PARAM(newLfArr);
#else
    GT_U8 ii = 0;
    GT_U32 intValue = 0 ;
    GT_U32 intSerdes = 0;
    GT_U32 intLf = 0;

    if (numOfSerdes > MAX_AVAGO_SERDES_NUMBER)
    {
        return GT_BAD_PARAM;
    }

    for (ii = 0; ii < numOfSerdes; ii++)
    {
        intSerdes = (GT_U32)serdesArr[ii];
        intLf = (GT_U32)newLfArr[intSerdes];
        /* LF*/
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, intSerdes, 0x26, ((2 << 12) | (1 << 8) |intLf ), NULL));

        /* Delay*/
        intValue = (newDelayArr[intSerdes] |(newDelayArr[intSerdes] << 4));
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, intSerdes, 0x18, 0x402e, NULL));
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, intSerdes, 0x19, intValue, NULL));
        /* save serdes delay in DB for next period */
        hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[intSerdes].currSerdesDelay = newDelayArr[intSerdes];
    }

#endif /*ASIC_SIMULATION*/
    return GT_OK;
}


/**
* @internal mvHwsAvagoAdaptiveCtleSerdesesEoGet function
* @endinternal
*
* @brief   Get current serdes parametrs, LF, Delay, EO.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] serdesArr                - port's serdeses arrray
* @param[in] numOfSerdes              - serdeses number
* @param[out] newEoArr                - array of serdeses EO
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAvagoAdaptiveCtleSerdesesEoGet
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U16      *serdesArr,
    IN GT_UOPT     numOfSerdes,
    OUT GT_U16      *newEoArr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesArr);
    GT_UNUSED_PARAM(numOfSerdes);
    GT_UNUSED_PARAM(newEoArr);
#else

    GT_U8 ii;
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT results;      /* current serdes parameters ,HWS
                                              format */

    if (numOfSerdes > MAX_AVAGO_SERDES_NUMBER)
    {
        return GT_BAD_PARAM;
    }

    for (ii = 0; ii < numOfSerdes; ii++)
    {
        /* read the EO and LF value from serders*/
        CHECK_STATUS(mvHwsSerdesAutoTuneResult(devNum, portGroup, (GT_U32)(serdesArr[ii]), HWS_DEV_SERDES_TYPE(devNum, (GT_U32)(serdesArr[ii])), &results));
        newEoArr[serdesArr[ii]] = (GT_U16)results.avagoResults.EO;
    }
#endif /*ASIC_SIMULATION*/
    return GT_OK;
}


/**
* @internal mvHwsAvagoAdaptiveCtleSerdesesLfAndDelayAndEoGet
*           function
* @endinternal
*
* @brief   Get current serdes parametrs, LF, Delay, EO.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] portIndex                - port index
* @param[in] serdesArr                - port's serdeses arrray
* @param[in] numOfSerdes              - serdeses number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAvagoAdaptiveCtleSerdesesLfAndDelayAndEoGet
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_U32      portIndex,
    IN GT_U16      *serdesArr,
    IN GT_UOPT     numOfSerdes
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(portIndex);
    GT_UNUSED_PARAM(serdesArr);
    GT_UNUSED_PARAM(numOfSerdes);
#else

    GT_U8 ii;
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT results;      /* current serdes parametrs */
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3))
    /* indicate that tranning done on the port. for debug on host side only.
     After adaptive CTLE starts running (when port status changed to link up), the port does not execute training again. */
    GT_BOOL portUpdateTranning;
    GT_U8 bitMapIndex = portIndex/HWS_MAX_ADAPT_CTLE_DB_SIZE;
#endif

    if (numOfSerdes > MAX_AVAGO_SERDES_NUMBER)
    {
        return GT_BAD_PARAM;
    }

#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3))
    portUpdateTranning =
        ((hwsDeviceSpecInfo[devNum].adaptiveCtleDb.bitMapUpdateTrainning[bitMapIndex] & (0x1 << (portIndex%HWS_MAX_ADAPT_CTLE_DB_SIZE))) > 0 ? GT_TRUE : GT_FALSE);
#endif
    for (ii = 0; ii < numOfSerdes; ii++)
    {
        /* read the EO and LF value from serders*/
        CHECK_STATUS(mvHwsSerdesAutoTuneResult(devNum, portGroup, ((GT_U32)(serdesArr[ii])), HWS_DEV_SERDES_TYPE(devNum, ((GT_U32)(serdesArr[ii]))), &results));

#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3))
        if (portUpdateTranning == GT_TRUE)
        {
            hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesArr[ii]].trainLf = results.avagoResults.LF;
        }
#endif
        hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesArr[ii]].oldEO = (GT_U16)results.avagoResults.EO;
        hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesArr[ii]].oldLf = (GT_U8)results.avagoResults.LF;

        /* get serdes delay from DB*/
        hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesArr[ii]].oldDelay =
            hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesArr[ii]].currSerdesDelay;
    }

#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3))
    /* indicate that tranning done on the port. for debug on host side only.
     After adaptive CTLE starts running (when port status changed to link up), the port does not execute training again. */
    if (portUpdateTranning == GT_TRUE)
    {
        hwsDeviceSpecInfo[devNum].adaptiveCtleDb.bitMapUpdateTrainning[bitMapIndex] &= ~(0x1 << (portIndex%HWS_MAX_ADAPT_CTLE_DB_SIZE));
    }
#endif

#endif /*ASIC_SIMULATION*/
    return GT_OK;
}


/**
* @internal mvHwsAvagoAdaptiveCtleBasedTemperaturePhase1
*           function
* @endinternal
*
* @brief   Run Steady State apdative ctle algorithm. this
*          feature do delay and LF Calibration based Temperature
*          to improve the EO per serdes. in case it does't
*          improved, do rollback.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] portIndex                - port number
* @param[in] serdesArr                - port's serdeses arrray
* @param[in] numOfSerdes              - serdeses number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoAdaptiveCtleBasedTemperaturePhase1
(
    IN GT_U8      devNum,
    IN GT_UOPT    portGroup,
    IN GT_U32     portIndex,
    IN GT_U16     *serdesArr,
    IN GT_UOPT    numOfSerdes
)
{

    GT_U8 ii;
    GT_U8 bitMapIndex;
    GT_U8 newLfArr[MAX_AVAGO_SERDES_NUMBER] = {0};
    GT_U8 newDelayArr[MAX_AVAGO_SERDES_NUMBER] = {0};
    GT_U32 chipIndex = devNum;

#ifdef ADAPT_CTLE_DEBUG

   GT_32 temperature;
   CHECK_STATUS(mvHwsPortTemperatureGet(devNum,portGroup, chipIndex, &temperature));
#endif

   /*1. Get the current serdes parameters: LF, Delay and EO */
    CHECK_STATUS(mvHwsAvagoAdaptiveCtleSerdesesLfAndDelayAndEoGet(devNum, portGroup,portIndex, serdesArr, numOfSerdes));

    /*2. check that the current EO is not below the Bad trheshold of 50*/
    for (ii = 0; ii < numOfSerdes; ii++)
    {
        if (hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesArr[ii]].oldEO < 50)
        {
            /*The EO is in lower range and can't be improved*/
            bitMapIndex = (GT_U8)(portIndex/HWS_MAX_ADAPT_CTLE_DB_SIZE);
            hwsDeviceSpecInfo[devNum].adaptiveCtleDb.bitMapOldValuesValid[bitMapIndex] &= ~(0x1 << (portIndex%HWS_MAX_ADAPT_CTLE_DB_SIZE));
            return GT_OK;
        }
    }

    /*3. Calculate the new Delay and new LF based on the current temperature */
    CHECK_STATUS(mvHwsAvagoAdaptiveCtleCalcNewLfNewDelayBasedTemperature(devNum,portGroup, serdesArr, numOfSerdes, newLfArr, newDelayArr));

     /*4. write new Delay and new LF to the SerDes */
    CHECK_STATUS(mvHwsAvagoAdaptiveCtleSerdesesLfAndDelaySet(devNum, portGroup, serdesArr, numOfSerdes, newDelayArr, newLfArr));

#ifdef ADAPT_CTLE_DEBUG
   for (ii = 0; ii < numOfSerdes; ii++)
    {
        hwsOsPrintf(" %2d  | %2d |  %2d   |  %2d   |    %2d    |    %2d    |  %3d |   -   |   -   |\n",
                 portIndex,
                 serdesArr[ii],
                 hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesArr[ii]].oldLf,
                 newLfArr[serdesArr[ii]],
                 hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesArr[ii]].oldDelay,
                 newDelayArr[serdesArr[ii]],
                 temperature);

        hwsOsPrintf("------------------------------------------------------------------------\n");
    }
#endif
    /* set valid bit = 1 for ports old values*/
    bitMapIndex = portIndex/HWS_MAX_ADAPT_CTLE_DB_SIZE;
    hwsDeviceSpecInfo[devNum].adaptiveCtleDb.bitMapOldValuesValid[bitMapIndex] |= (0x1 << (portIndex%HWS_MAX_ADAPT_CTLE_DB_SIZE));

    return GT_OK;

}


/**
* @internal mvHwsAvagoAdaptiveCtleBasedTemperaturePhase2
*           function
* @endinternal
*
* @brief   Run Steady State apdative ctle algorithm. this
*          feature do delay and LF Calibration based Temperature
*          to improve the EO per serdes. in case it does't
*          improved, do rollback.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] portIndex                - port number
* @param[in] serdesArr                - port's serdeses arrray
* @param[in] numOfSerdes              - serdeses number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoAdaptiveCtleBasedTemperaturePhase2
(
    IN GT_U8      devNum,
    IN GT_UOPT    portGroup,
    IN GT_U32     portIndex,
    IN GT_U16     *serdesArr,
    IN GT_UOPT    numOfSerdes
)
{

    GT_U8 ii;
    GT_U16 newEoArr[MAX_AVAGO_SERDES_NUMBER] = {0};
    GT_16  eoGapInt = 0;

    /*5. read EO and check if improved */
    CHECK_STATUS(mvHwsAvagoAdaptiveCtleSerdesesEoGet(devNum, portGroup, serdesArr, numOfSerdes, newEoArr));

    for (ii = 0; ii < numOfSerdes; ii++)
    {
#ifdef ADAPT_CTLE_DEBUG
            hwsOsPrintf(" %2d  | %2d |   -   |   -   |     -    |    -     |   -  |  %2d  |  %2d  |\n",portIndex, serdesArr[ii], hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesArr[ii]].oldEO, newEoArr[serdesArr[ii]]);
            hwsOsPrintf("------------------------------------------------------------------------\n");
#endif
        eoGapInt = ((GT_16)(newEoArr[serdesArr[ii]]) - (GT_16)(hwsDeviceSpecInfo[devNum].adaptiveCtleDb.adaptCtleParams[serdesArr[ii]].oldEO));
        if ( eoGapInt < 4)
        {
            CHECK_STATUS(mvHwsAvagoAdaptiveCtleSerdesLfAndDelaySet(devNum, portGroup, (GT_U8)serdesArr[ii]));
        }
    }

    return GT_OK;
}
#endif /*RUN_ADAPTIVE_CTLE*/
#endif /*((!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) ||(defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)))*/


#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT)

/**
* @internal mvHwsAvagoSerdesDefaultRxTerminationSet function
* @endinternal
*
* @brief   Initialize Avago envelope related configurations
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] rxTermination            - rxTermination
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesDefaultRxTerminationSet
(
    GT_U8                                devNum,
    GT_U32                               portGroup,
    GT_U8                                rxTermination
)
{

#ifndef ASIC_SIMULATION
    GT_U32 i, j;
#endif

    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);

#ifndef ASIC_SIMULATION

    for (i = 0; i < HWS_AVAGO_16NM_TXRX_PARAMS_NUM_OF_PROFILES; i++)
    {
        for (j = 0; j < hwsAvagoSerdesTxRx16nmTuneParamsArraySizes[i]; j++)
        {
            hwsAvagoSerdesTxRx16nmTuneParamsArrayPtrs[i][j].rxTuneData.termination = rxTermination;
        }
    }
#else
    GT_UNUSED_PARAM(rxTermination);

#endif
    return GT_OK;
}
#endif

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && !defined(ASIC_SIMULATION)
GT_STATUS mvHwsAvagoSerdesTxRxPrint(void)
{
    GT_U32 i, size;
    size = sizeof(hwsAvagoSerdesTxRx16nmTuneParamsArray) / sizeof(MV_HWS_AVAGO_16NM_TXRX_TUNE_PARAMS);
    hwsOsPrintf("Baud Rate {atten ,post ,pre ,pre2 ,pre3 },{DC ,LF  ,HF   ,BW  ,GS0 ,GS1 ,Min_LF,Max_LF,Min_HF,Max_HF,BFLF  ,BFHF  ,Min_pre1,Max_pre1,Min_pre2,Max_pre2,Min_post,Max_post,SQ[mV] ,shortChannelEn ,termination\n");
    for (i = 0; i < size; i++) {
        hwsOsPrintf(" %2d      ,{%2d    ,%2d   ,%2d  ,%2d   ,%2d   },{%2d ,%2d  ,%2d  ,%2d  ,%2d  ,%2d   ,%2d    ,%2d    ,%2d    ,%2d    ,%2d    ,%2d    ,%2d      ,%2d      ,%2d      ,%2d      ,%2d      ,%2d      ,%4d   , %2d            ,%2d        }\n",
                 hwsAvagoSerdesTxRx16nmTuneParamsArray[i].serdesSpeed,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].txTuneData.atten,
                 hwsAvagoSerdesTxRx16nmTuneParamsArray[i].txTuneData.post,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].txTuneData.pre,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].txTuneData.pre2,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].txTuneData.pre3,
                 hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.dcGain,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.lowFrequency,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.highFrequency,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.bandWidth,
                 hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.gainShape1,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.gainShape2,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.minLf,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.maxLf,
                 hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.minHf,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.maxHf,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.bfLf,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.bfHf,
                 hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.minPre1,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.maxPre1,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.minPre2,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.maxPre2,
                 hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.minPost,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.maxPost,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.squelch,hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.shortChannelEn,
                 hwsAvagoSerdesTxRx16nmTuneParamsArray[i].rxTuneData.termination);
    }
    return GT_OK;
}
#endif
/*******************************************************************************/
/******************************* DEBUG functions *******************************/
/*******************************************************************************/
/* Following functions are for use by CLI and therefore not defined as static,
 * and not exported*/

/********************************************************************************
   runMultipleEnhanceTune is a debug function for Testing only for running
   the mvHwsAvagoSerdesEnhanceTune from Terminal
********************************************************************************/
void runMultipleEnhanceTune
(
    GT_U32  serdesNumbers0_31,
    GT_U32  serdesNumbers32_63,
    GT_U8   min_LF,
    GT_U8   max_LF
);

void bobcat3RunMultipleEnhanceTune
(
    GT_U32  serdesNumbers0_31,
    GT_U32  serdesNumbers32_63,
    GT_U32  serdesNumbers64_95,
    GT_U8   min_LF,
    GT_U8   max_LF
);

/**
* @internal mvHwsAvagoSerdesTap1DbgPrint function
* @endinternal
*
* @brief   Get tap1 value from spico of a given serdes.
*
* @note   APPLICABLE DEVICES:      Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin.
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] serdesNum                - serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTap1DbgPrint
(
    IN GT_U8 devNum,
    IN GT_U32 portGroup,
    IN GT_U32 serdesNum
);


/********************************************************************************
   runMultipleEnhanceTune is a debug function for Testing only for running
   the mvHwsAvagoSerdesEnhanceTune from Terminal
********************************************************************************/
void runMultipleEnhanceTune
(
    IN GT_U32  serdesNumbers0_31,
    IN GT_U32  serdesNumbers32_63,
    IN GT_U8   min_LF,
    IN GT_U8   max_LF
)
{
    static GT_U32  serdesArr[64]= {0};
    GT_U32  i, numOfSerdes;

    numOfSerdes=0; /* num of Serdes to run the function */
    for (i=0; i<32; i++)
    {
        if ((((serdesNumbers0_31 >> i) & 1) == 1))
        {
            serdesArr[numOfSerdes] = i;
            numOfSerdes++;
            HWS_AVAGO_DBG(("Serdes %d\n", i));
        }
    }

    for (i=32; i<64; i++)
    {
        if ((((serdesNumbers32_63 >> (i-32)) & 1) == 1))
        {
            serdesArr[numOfSerdes] = i;
            numOfSerdes++;
            HWS_AVAGO_DBG(("Serdes %d\n", i));
        }
    }

    mvHwsAvagoSerdesEnhanceTune(0, 0, serdesArr, numOfSerdes, min_LF, max_LF);
}

void bobcat3RunMultipleEnhanceTune
(
    IN GT_U32  serdesNumbers0_31,
    IN GT_U32  serdesNumbers32_63,
    IN GT_U32  serdesNumbers64_95,
    IN GT_U8   min_LF,
    IN GT_U8   max_LF
)
{
    static GT_U32  serdesArr[96]= {0};
    GT_U32  i, numOfSerdeses = 0;

    for (i=0; i<32; i++)
    {
        if(((serdesNumbers0_31 >> i) & 1) == 1)
        {
            serdesArr[numOfSerdeses] = i;
            numOfSerdeses++;
            HWS_AVAGO_DBG(("Serdes %d\n", i));
        }
    }

    for (i=32; i<64; i++)
    {
        if(((serdesNumbers32_63 >> (i-32)) & 1) == 1)
        {
            serdesArr[numOfSerdeses] = i;
            numOfSerdeses++;
            HWS_AVAGO_DBG(("Serdes %d\n", i));
        }
    }

    for (i=64; i<96; i++)
    {
        if(((serdesNumbers64_95 >> (i-64)) & 1) == 1)
        {
            serdesArr[numOfSerdeses] = i;
            numOfSerdeses++;
            HWS_AVAGO_DBG(("Serdes %d\n", i));
        }
    }

    mvHwsAvagoSerdesEnhanceTune(0, 0, serdesArr, numOfSerdeses, min_LF, max_LF);
}

/**
* @internal mvHwsAvagoSerdesTap1DbgPrint function
C* @endinternal
*
* @brief   Get tap1 value from spico of a given serdes.
*
* @note   APPLICABLE DEVICES:      Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin.
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] serdesNum                - serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTap1DbgPrint
(
    IN GT_U8 devNum,
    IN GT_U32 portGroup,
    IN GT_U32 serdesNum
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesNum);
#else

    GT_32 data = 0;

    /* get HW value */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x126,
                            ((0x5 << 12) | (0xB << 8)), &data));

    HWS_AVAGO_DBG((" TAP1=%s, HW value bits[0:3]=0x%02x \n", (data & 8) ? "ENABLED" : "DISABLED", data));

#endif
    return GT_OK;
}


