/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvSiliconIf.h
*
* @brief Application IF definition.
*
* @version   46
********************************************************************************
*/

#ifndef __siliconIf_H
#define __siliconIf_H

#ifdef __cplusplus
extern "C" {
#endif
#include <cpss/common/labServices/port/gop/port/mvHwsPortApInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>

#if defined(BOBK_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(RAVEN_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApDefs.h>
#endif

#ifndef MV_HWS_FREE_RTOS
#include <cpss/generic/cpssHwInfo.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#endif

#ifdef CO_CPU_RUN
typedef GT_U8       GT_UOPT;
typedef GT_U16      GT_UREG_DATA;
typedef GT_16       GT_REG_DATA;
#else
typedef GT_U32      GT_UOPT;
typedef GT_U32      GT_UREG_DATA;
typedef GT_32       GT_REG_DATA;
#endif

#define MV_HWS_SW_PTR_ENTRY_UNUSED      0xFFFFFFFB
#define NA_NUM                          0xfffe

#if defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#define hwsOsPrintf osPrintf
#else
#define hwsOsPrintf cpssOsPrintf
#endif

/* printing includes and definitions */
#if defined(CHX_FAMILY) || defined(PX_FAMILY)
#elif defined(MV_HWS_REDUCED_BUILD) && !defined(VIPS_BUILD) && !defined(WIN32)
#include <printf.h>
#define osPrintf        printf
#endif

#ifndef U32_GET_FIELD
/* Returns the info located at the specified offset & length in data.   */
#define U32_GET_FIELD(data,offset,length)           \
        (((data) >> (offset)) & ((1 << (length)) - 1))
#endif

#ifdef HWS_DEBUG
#include <gtOs/gtOsIo.h>
extern GT_FILEP fdHws;
#endif

extern GT_VOID gtBreakOnFail
(
    GT_VOID
);

#define CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum) \
    if (HWS_DEV_SILICON_TYPE(0) == Falcon){ \
        serdesNum = devNum*16 + serdesNum; \
        devNum = 0;\
    }

#define HWS_SERVER_WRITE_REG(driverPtr,devNum,regAddr,data) \
    if (driverPtr == NULL){ \
        CHECK_STATUS(hwsServerRegSetFuncPtr(devNum, regAddr, data)); \
    }else {\
        GT_U32 dPtr = data;\
        if (driverPtr->writeMask != NULL){\
            CHECK_STATUS(driverPtr->writeMask(driverPtr, SSMI_FALCON_ADDRESS_SPACE, regAddr, &dPtr, 1, 0xFFFFFFFF));\
        }else{\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);}\
    }

#define HWS_SERVER_READ_REG(driverPtr,devNum,regAddr,data) \
    if (driverPtr == NULL){ \
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, regAddr, data)); \
    }else {\
        if (driverPtr->read != NULL){\
            CHECK_STATUS(driverPtr->read(driverPtr, SSMI_FALCON_ADDRESS_SPACE, regAddr, data, 1));\
        }else{\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);}\
    }

#define HWS_REGISTER_WRITE_REG(driverPtr,devNum,regAddr,data, mask) \
    if (driverPtr == NULL){ \
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, data, mask)); \
    }else {\
        GT_U32 dPtr = data;\
        if (driverPtr->writeMask != NULL){\
            CHECK_STATUS(driverPtr->writeMask(driverPtr, SSMI_FALCON_ADDRESS_SPACE, regAddr, &dPtr, 1, mask));\
        }else{\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);}\
    }

#define HWS_REGISTER_READ_REG(driverPtr,devNum,regAddr,data, mask) \
    if (driverPtr == NULL){ \
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, data, mask)); \
    }else {\
        if (driverPtr->read != NULL){\
            CHECK_STATUS(driverPtr->read(driverPtr, SSMI_FALCON_ADDRESS_SPACE, regAddr, data, 1));\
            *data = (*data & (mask));\
        }else{\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);}\
    }

/* Global return code status for hws check status usage */
extern GT_STATUS gtStatus;

/*****************************************************/
/*************** Hws Log MACRO APIs ******************/
/*****************************************************/
/*
    Legacy MACRO
*/
#ifdef CHECK_STATUS
#undef CHECK_STATUS
#endif

#if !defined(CM3)
#define CHECK_STATUS(origFunc) \
    CHECK_STATUS_EXT(origFunc, LOG_ARG_NONE_MAC)
#else
#ifdef CPSS_LOG_ERROR_AND_RETURN_MAC
 #undef CPSS_LOG_ERROR_AND_RETURN_MAC
#endif
#define CPSS_LOG_ERROR_AND_RETURN_MAC(_rc, ...) \
    return _rc;
#ifdef LOG_ERROR_NO_MSG
 #undef LOG_ERROR_NO_MSG
#endif
#define LOG_ERROR_NO_MSG ""
#define CHECK_STATUS(origFunc) do {if(GT_OK!=origFunc)CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);} while(0);
#endif /*CM3 && PIPE_DEV_SUPPORT*/

/*
   MACROs for logging Hws APIs. Due to the fact that hws runs in both host CPU and
   in Service CPU, logging should be performed mainly by both. CM3 memory constraint
   force the log handing to be performed in a reduced manner then in host CPU,
   the log extra arguments can be used by the next argument macros ( they cannot
   be called with unlimited format). Use 'CHECK_STATUS_EXT' for function calls
   and 'HWS_LOG_ERROR_AND_RETURN_MAC' for cpss style log of return code and a message.
   Note: library parsing functions like strcpy with max size, and vsnprints that
   parse format and argument list are too heavy for reduced memory architectures.

   Usage examples:

       CHECK_STATUS_EXT(func(devNum, portGroup, serdesNum, regAddr), LOG_ARG_SERDES_MAC(serdesNum), LOG_ARG_REG_ADDR_MAC(regAddr) )
       CHECK_STATUS_EXT(func(), LOG_ARG_NONE_MAC )
       HWS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ARG_STRING_MAC("operation not supported"))

*/

/* Argument macro flags */
#define LOG_ARG_SERDES_FLAG            0
#define LOG_ARG_MAC_FLAG               1
#define LOG_ARG_PCS_FLAG               2
#define LOG_ARG_PORT_FLAG              3
#define LOG_ARG_REG_ADDR_FLAG          4
#define LOG_ARG_STRING_FLAG            5
#define LOG_ARG_GEN_PARAM_FLAG         6
#define LOG_ARG_NONE_FLAG              0xFF

/* Argument macros to be used by user */
#define LOG_ARG_SERDES_IDX_MAC(sdNum)  LOG_ARG_SERDES_FLAG, sdNum
#define LOG_ARG_MAC_IDX_MAC(macNum)    LOG_ARG_MAC_FLAG, macNum
#define LOG_ARG_PCS_IDX_MAC(pcsNum)    LOG_ARG_PCS_FLAG, pcsNum
#define LOG_ARG_PORT_MAC(portNum)      LOG_ARG_PORT_FLAG, portNum
#define LOG_ARG_REG_ADDR_MAC(regAddr)  LOG_ARG_REG_ADDR_FLAG, regAddr
/* This macro require string literal as it uses string literal concatination,
   as we must contain null terminited string because the builder function cannot
   use string copy with size limitition due to memory constraint */
#define LOG_ARG_STRING_MAC(str)        LOG_ARG_STRING_FLAG, str "\0"
/* General parameter macro */
#define LOG_ARG_GEN_PARAM_MAC(param)   LOG_ARG_GEN_PARAM_FLAG, param
/* No args macro */
#define LOG_ARG_NONE_MAC               LOG_ARG_NONE_FLAG

/* Log and return */
#define HWS_LOG_ERROR_AND_RETURN_MAC(rc, ...)   \
        CHECK_STATUS_EXT(rc, __VA_ARGS__)

/* Null pointer check */
#define HWS_NULL_PTR_CHECK_MAC(ptr)         \
do {                                        \
    if (ptr == NULL)                        \
    {                                       \
        CHECK_STATUS_EXT(GT_BAD_PTR,        \
       LOG_ARG_STRING_MAC("param is NULL"));\
    }                                       \
} while (0)


/** TODO - complete information logger. Currently on
 *  information logging is done only in host side */
#ifndef  MV_HWS_FREE_RTOS
#define HWS_INFORMATION_MAC(GT_OK, ...)   \
        CHECK_STATUS_EXT(GT_OK, __VA_ARGS__)
#else
#define HWS_INFORMATION_MAC(GT_OK, ...)
#endif

#ifdef CHECK_STATUS_EXT
#undef CHECK_STATUS_EXT
#endif

/* Host CPU (cpss) without logging enabled */
#ifdef HWS_NO_LOG
#define CHECK_STATUS_EXT(origFunc, ...)                  \
do {                                                     \
    gtStatus = origFunc;                                 \
    if (GT_OK != gtStatus)                               \
    {                                                    \
        gtBreakOnFail();                                 \
        return gtStatus;                                 \
    }                                                    \
} while (0);
#else
/* Host CPU with logging enabled, or Service CPU.
   in either case we need to generate hws message and log it */
#ifdef  MV_HWS_FREE_RTOS
/* Service CPU */
/* This macro should be as minimal as possible in order to
   support the memory constraint of the CM3 architecture */
#define CHECK_STATUS_EXT(origFunc, ...)                         \
do {                                                            \
        if (origFunc!=GT_OK)                                    \
        {                                                       \
            /* Service CPU and Host CPU log  */                 \
            return (mvHwsGeneralLog(__FUNCTION__, __VA_ARGS__,  \
                                           LOG_ARG_NONE_MAC));  \
        }                                                       \
} while(0);
#else
/*  CPSS_LOG_ERROR_AND_RETURN_MAC(rc,...)  */
/* Host CPU with logging enabled */
#define CHECK_STATUS_EXT(origFunc, ...)                       \
do {                                                          \
    gtStatus = origFunc;                                      \
    if (GT_OK != gtStatus)                                    \
    {                                                         \
        gtBreakOnFail();                                      \
        /* Service CPU and Host CPU log  */                   \
        return (mvHwsGeneralLog(__FUNCNAME__, __FILENAME__,   \
         (unsigned int)__LINE__, (int)gtStatus, __VA_ARGS__, LOG_ARG_NONE_MAC)); \
    }                                                         \
} while (0);
#endif /* MV_HWS_FREE_RTOS */
#endif /* HWS_NO_LOG */
/*****************************************************/
/*****************************************************/
/*****************************************************/

            /* ABS definition */
#define ABS(val) (((val) < 0) ? -(val) : (val))

/* max number of devices supported by driver */
#ifdef MV_HWS_REDUCED_BUILD
#define HWS_MAX_DEVICE_NUM (1)
#else
#define HWS_MAX_DEVICE_NUM (128)
#endif

#define MV_HWS_MAX_HOST2HWS_REQ_MSG_NUM      6

#define MV_HWS_MAX_PORT_NUM 72
#ifdef ALDRIN_DEV_SUPPORT
#define ALDRIN_PORTS_NUM 32
#endif

#ifdef PIPE_DEV_SUPPORT
#define PIPE_PORTS_NUM 16
#endif

#define RAVEN_PORTS_NUM 16 /* 16 ports in raven, 16 ravens in Falcon 4 tiles. 16x16 = 256*/

#ifdef AC5_DEV_SUPPORT
#define AC5_PORTS_NUM 30
#endif

#define MV_HWS_MAX_CTLE_BIT_MAPS_NUM 3
#if defined(BC3_DEV_SUPPORT) || defined(BOBK_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)|| defined(ALDRIN2_DEV_SUPPORT)
#define HWS_MAX_CTLE_DB_SIZE 32
#endif

#define HWS_MAX_ADAPT_CTLE_DB_SIZE 8


/*disable Adaptive CTLE , until approval of the feature*/
/*#define RUN_ADAPTIVE_CTLE*/

enum
{
    EXTERNAL_REG,
    INTERNAL_REG,
};
enum{ REG_READ_OP, REG_WRITE_OP};

#if defined (CM3) && (defined (BOBK_DEV_SUPPORT) || defined (ALDRIN_DEV_SUPPORT) || defined (AC5_DEV_SUPPORT)) /*AC5-TBD*/
/**
* @enum MV_HWS_DEV_TYPE
* @endinternal
*
* @brief   Device types list
*
*/

typedef enum
{
    BobK,
    Aldrin,
    AC5,
    LAST_SIL_TYPE,
    Lion2A0,
    Lion2B0,
    HooperA0,
    BobcatA0,
    Alleycat3A0,
    Bobcat3,
    Pipe,
    Aldrin2,
    Raven,
    Falcon,
    AC5P,       /* Hawk */
    AC5X,       /* Phoenix */
    Harrier

}MV_HWS_DEV_TYPE;
#else
typedef enum
{
    Lion2A0,
    Lion2B0,
    HooperA0,
    BobcatA0,
    Alleycat3A0,
    BobK,
    Aldrin,
    Bobcat3,
    Pipe,
    Aldrin2,
    Raven,
    Falcon,
    AC5P,
    AC5,
    AC5X,
    Harrier,
    LAST_SIL_TYPE

}MV_HWS_DEV_TYPE;
#endif


/**
* @enum MV_HWS_UNIT_NUM
* @endinternal
*
* @brief   Number of Raven/Eagle.
*
*/

typedef enum
{
    Raven_0,
    Raven_1,
    Raven_2,
    Raven_3,
    Raven_4,
    Raven_5,
    Raven_6,
    Raven_7,
    Raven_8,
    Raven_9,
    Raven_10,
    Raven_11,
    Raven_12,
    Raven_13,
    Raven_14,
    Raven_15,
    Eagle_0,
    Eagle_1,
    Eagle_2,
    Eagle_3,
    All

}MV_HWS_UNIT_NUM;

/*extern char *devTypeNamesArray[LAST_SIL_TYPE];*/
/**
* @enum MV_HWS_UNITS_ID
* @endinternal
*
* @brief   HWS units
*
*/

typedef enum
{
    GEMAC_UNIT,                     /* 0 */
    XLGMAC_UNIT,                    /* 1 */
    HGLMAC_UNIT,                    /* 2 */
    XPCS_UNIT,                      /* 3 */
    MMPCS_UNIT,                     /* 4 */

    CG_UNIT,                        /* 5 */
    INTLKN_UNIT,                    /* 6 */
    INTLKN_RF_UNIT,                 /* 7 */
    SERDES_UNIT,                    /* 8 */
    SERDES_PHY_UNIT,                /* 9 */
    ETI_UNIT,                       /* 10 */
    ETI_ILKN_RF_UNIT,               /* 11 */

    D_UNIT, /* DDR3 */              /* 12 */

    CGPCS_UNIT,                     /* 13 */
    CG_RS_FEC_UNIT,                 /* 14 */

    PTP_UNIT,                       /* 15 */
#if !defined(BOBK_DEV_SUPPORT) && !defined(AC5_DEV_SUPPORT)

    D2D_EAGLE_UNIT,                 /* 16 */
    D2D_RAVEN_UNIT,                 /* 17 */
    RAVEN_BASE_UNIT,                /* 18 */
    MTI_MAC400_UNIT,                /* 19 */
    MTI_MAC100_UNIT,                /* 20 */
    MTI_PCS25_UNIT,                 /* 21 */
    MTI_PCS50_UNIT,                 /* 22 */
    MTI_PCS100_UNIT,                /* 23 */
    MTI_PCS200_UNIT,                /* 24 */
    MTI_PCS400_UNIT,                /* 25 */
    MTI_LOW_SP_PCS_UNIT,            /* 26 */ /* low speed 8channel PCS */
    MTI_EXT_UNIT,                   /* 27 */
    MTI_RSFEC_UNIT,                 /* 28 */
    RAVEN_MSDB_UNIT,                /* 29 */
    RAVEN_MPFS_UNIT,                /* 30 */
    RAVEN_TSU_UNIT,                 /* 31 */
    RAVEN_STATISTICS_UNIT,          /* 32 */
    MTI_CPU_MAC_UNIT,               /* 33 */
    MTI_CPU_PCS_UNIT,               /* 34 */
    MTI_CPU_EXT_UNIT,               /* 35 */
    RAVEN_MTI_CPU_MPFS_UNIT,        /* 36 */
    RAVEN_DFX_UNIT,                 /* 37 */
    MIF_400_UNIT,                   /* 38 */
    MIF_400_BR_UNIT,                /* 39 */ /* fake unit for preemption MIF_32 registers access  */
    MIF_400_SEG_UNIT,               /* 40 */
    MIF_USX_UNIT,                   /* 41 */
    MIF_CPU_UNIT,                   /* 42 */
    ANP_400_UNIT,                   /* 43 */
    AN_400_UNIT,                    /* 44 */
    /*each USX has 2 ANP units*/
    ANP_USX_UNIT,                   /* 45 */ /* each anp has 4 ports*/
    AN_USX_UNIT,                    /* 46 */ /* each anp has 4 ports*/
    ANP_USX_O_UNIT,                 /* 47 */ /* each anp has 8 ports and onlt the first anp is used*/
    ANP_CPU_UNIT,                   /* 48 */
    AN_CPU_UNIT,                    /* 49 */

    MTI_USX_MAC_UNIT,               /* 50 */
    MTI_USX_EXT_UNIT,               /* 51 */
    MTI_USX_PCS_UNIT,               /* 52 */
    MTI_USX_LPCS_UNIT,              /* 53 */
    MTI_USX_RSFEC_UNIT,             /* 54 */
    MTI_USX_MULTIPLEXER_UNIT,       /* 55 */
    MTI_CPU_RSFEC_UNIT,             /* 56 */
    MTI_CPU_SGPCS_UNIT,             /* 57 */
    MTI_RSFEC_STATISTICS_UNIT,      /* 58 */
    MTI_USX_RSFEC_STATISTICS_UNIT,  /* 59 */
#endif
    LAST_UNIT

}MV_HWS_UNITS_ID;

/**
* @enum MV_HWS_CONNECTION_TYPE
* @endinternal
*
* @brief   Connection Type
*
*/

typedef enum
{
    HOST2HWS_LEGACY_MODE,
    HOST2SERVICE_CPU_IPC_CONNECTION,
    LAST_HOST2HWS_CONNECTION_TYPE

}MV_HWS_CONNECTION_TYPE;

/**
* @enum HWS_UNIT_INFO
* @endinternal
*
* @brief   UNIT INFO
*
*/
typedef struct
{
    GT_U32 baseAddr;
    GT_U32 regOffset;
}HWS_UNIT_INFO;

typedef int (*MV_HWS_SBUS_ADDR_TO_SERDES_NUM_FUNC_PTR)
(
    unsigned char devNum,
    GT_U32 *serdesNum,
    unsigned int  sbusAddr
);

/**
* @struct MV_HWS_AVAGO_SERDES_INFO_STC
* @endinternal
*
* @brief   SerDes Information
*
*/
typedef struct
{
    GT_U32                          serdesNumber;
    GT_U32                          *serdesToAvagoMapPtr;
    GT_U32                          spicoNumber;
    GT_U32                          *avagoToSerdesMapPtr;
    GT_BOOL                         cpllInitDoneStatusArr[MV_HWS_MAX_CPLL_NUMBER];
    MV_HWS_CPLL_OUTPUT_FREQUENCY    cpllCurrentOutFreqArr[MV_HWS_MAX_CPLL_NUMBER];
    GT_BOOL                         serdesVosOverride;
    MV_HWS_SBUS_ADDR_TO_SERDES_NUM_FUNC_PTR sbusAddrToSerdesFuncPtr;
    GT_U32                          ctleBiasOverride[(MV_HWS_MAX_PORT_NUM+31)/32];
    GT_U32                          ctleBiasValue[(MV_HWS_MAX_PORT_NUM+31)/32];
    GT_U8                           ctleBiasLanesNum;
    GT_U16                          ctleBiasMacNum;
}MV_HWS_AVAGO_SERDES_INFO_STC;

/**
* @struct MV_HWS_SERDES_PER_SERDES_INFO_STC
* @endinternal
*
* @brief   SerDes Information
*
*/
typedef struct
{
    struct _HWS_DEVICE_INFO         *hostDevInfo; /* pointer to HWS_DEVICE_INFO */
    GT_U32                          serdesNum;
    GT_U32                          internalLane;
    MV_HWS_SERDES_TYPE              serdesType;
    union
    {
        struct _MV_HWS_COMPHY_SERDES_DATA  *comphySerdesData;
    } serdesData;
}MV_HWS_PER_SERDES_INFO_STC;

typedef MV_HWS_PER_SERDES_INFO_STC* MV_HWS_PER_SERDES_INFO_PTR;

/**
* @struct MV_HWS_SERDES_INFO_STC
* @endinternal
*
* @brief   SerDes Information
*
*/
typedef struct
{
    GT_U32                       numOfSerdeses;
    MV_HWS_PER_SERDES_INFO_STC   *serdesesDb;
}MV_HWS_SERDES_INFO_STC;

#ifndef MV_HWS_FREE_RTOS
/**
* @struct MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC
* @endinternal
*
* @brief   PM Edge Detect info
*
*/
typedef struct{
    GT_U32       phase_tvalBefore_tvSec[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U32       phase_tvalBefore_tvUsec[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U8        phase_contextSwitchedBack;
    GT_U32       phase_edgeDetectin_Sec;
    GT_U32       phase_edgeDetectin_Usec;
    GT_U8        phase_edgeDetectin_Flag;
    GT_32        phase_tmpTimeout;
    GT_32        phase_firstEO[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U32       phase_pCalTimeNano[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U32       phase_pCalTimeSec[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_BOOL      phase_enable[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_BOOL      phase_breakLoop[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_BOOL      phase_breakLoopTotal;
    GT_BOOL      phase_breakLoopInd;
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT phase_tuneRes[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_BOOL      initFlag;

} MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC;

/**
* @struct MV_HWS_AVAGO_PORT_MANAGER_ENHANCE_TUNE_STC
* @endinternal
*
* @brief   PM Enhanced TUne phases
*
*/
typedef struct{
    GT_U32  phase_CurrentDelaySize;
    GT_U32  *phase_CurrentDelayPtr;
    GT_U32  phase_InOutI;
    GT_U8   phase_InOutKk;
    GT_BOOL phase_continueExecute;
    GT_U32  phase_Best_LF[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U32  phase_Best_eye[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U32  phase_Best_dly[MV_HWS_MAX_LANES_NUM_PER_PORT];
    int     phase_LF1_Arr[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U8   subPhase;
    GT_BOOL parametersInitialized;
    GT_BOOL waitForCoreReady;

} MV_HWS_AVAGO_PORT_MANAGER_ENHANCE_TUNE_STC;


/**
* @struct MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC
* @endinternal
*
* @brief   PM One Shot Tuning
*
*/
typedef struct
{
    GT_BOOL coreStatusReady[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_BOOL parametersInitialized;
}MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC;

/**
* @struct MV_HWS_AVAGO_PORT_MANAGER_STC
* @endinternal
*
* @brief   PM Avago data
*
*/
typedef struct{
    MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC   *edgeDetectStcPtr;
    MV_HWS_AVAGO_PORT_MANAGER_ENHANCE_TUNE_STC  *enhanceTuneStcPtr;
    MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC *oneShotTuneStcPtr;

} MV_HWS_AVAGO_PORT_MANAGER_STC;

typedef MV_HWS_AVAGO_PORT_MANAGER_STC* MV_HWS_AVAGO_PORT_MANAGER_STC_PTR;

#endif /* MV_HWS_FREE_RTOS */



#if ((!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) ||(defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)))
#ifdef RUN_ADAPTIVE_CTLE
/**
* @struct MV_HWS_ADAPTIVE_CTLE_SERDES_PRAMS_STC
* @endinternal
*
* @brief   CTLE SerDes parameters
*
*/
typedef struct
{
 GT_U8 trainLf;            /* const in steady state,  The result of the SerDes training */
 GT_U8 enhTrainDelay;      /* const in steady state,  The result of enhance tune, otherwise 0*/
 GT_U8 currSerdesDelay;    /* can change every adaptive ctle cycle - read in phase 1*/
 GT_U8 oldLf;              /* can change every adaptive ctle cycle - read in phase 1*/
 GT_U8 oldDelay;           /* can change every adaptive ctle cycle - read in phase 1*/
 GT_U16 oldEO;              /* can change every adaptive ctle cycle - read in phase 1*/

}MV_HWS_ADAPTIVE_CTLE_SERDES_PRAMS_STC;

/**
* @struct MV_HWS_ADAPTIVE_CTLE_PORTS_PRAMS_STC
* @endinternal
*
* @brief   CTLE Port parameters
*
*/
typedef struct
{
 GT_U8 serdesList[MV_HWS_MAX_LANES_NUM_PER_PORT];   /* ports Serdeses*/
 GT_U8  numOfSerdeses;                              /* serdeses number*/

}MV_HWS_ADAPTIVE_CTLE_PORTS_PRAMS_STC;


/**
* @struct MV_HWS_ADAPTIVE_CTLE_PARAMS_STC
* @endinternal
*
* @brief   CTLE parameters
*
*/
typedef struct
{
   /* global adaptive ctle params:
    max delay 15, min delay 0
    max LF 15, min LF 0
    the time reqiured to wait until the EO update according to delay and LF 250 const
    the adaptive ctle Algorithm period 5 seconds
    */
   /* per ports params: ports bitmap
      in free rtos each array according to the device ports number
      in host all array have size of 72 ports
      */
#if ( defined(MV_HWS_REDUCED_BUILD_EXT_CM3) &&(defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)))
    /* ap or non ap - cm3 only */
    GT_U8 bitMapadaptCtleMode[(MV_PORT_CTRL_MAX_AP_PORT_NUM+(HWS_MAX_ADAPT_CTLE_DB_SIZE -1))/HWS_MAX_ADAPT_CTLE_DB_SIZE];
    /* Enable disable adaptive ctle - host and cm3*/
    GT_U8  bitMapEnableCtleCalibrationBasedTemperture[(MV_PORT_CTRL_MAX_AP_PORT_NUM+(HWS_MAX_ADAPT_CTLE_DB_SIZE-1))/HWS_MAX_ADAPT_CTLE_DB_SIZE]; /* enable/disable per port adaptive ctle*/
    GT_U8  bitMapOldValuesValid[(MV_PORT_CTRL_MAX_AP_PORT_NUM+(HWS_MAX_ADAPT_CTLE_DB_SIZE -1))/HWS_MAX_ADAPT_CTLE_DB_SIZE];                       /* bit map to know if the values ( old lf, old delay, old EO) are valid per port. */

    /* serdes list and number per port.*/
    MV_HWS_ADAPTIVE_CTLE_PORTS_PRAMS_STC portsSerdes[MV_PORT_CTRL_MAX_AP_PORT_NUM];
#if defined(PIPE_DEV_SUPPORT)
    /* per serdes params*/
    MV_HWS_ADAPTIVE_CTLE_SERDES_PRAMS_STC adaptCtleParams[16];
#else
   /* per serdes params*/
    MV_HWS_ADAPTIVE_CTLE_SERDES_PRAMS_STC adaptCtleParams[MV_HWS_MAX_PORT_NUM];
#endif
#else /* not bobcat3 or aldrin2 or pipe in CM3*/

    GT_U8  bitMapEnableCtleCalibrationBasedTemperture[(MV_HWS_MAX_PORT_NUM+(HWS_MAX_ADAPT_CTLE_DB_SIZE-1))/HWS_MAX_ADAPT_CTLE_DB_SIZE]; /* enable/disable per port adaptive ctle*/
    GT_U8  bitMapUpdateTrainning[(MV_HWS_MAX_PORT_NUM+(HWS_MAX_ADAPT_CTLE_DB_SIZE-1))/HWS_MAX_ADAPT_CTLE_DB_SIZE];                      /* bit map for new tranning results per port.*/
    GT_U8  bitMapOldValuesValid[(MV_HWS_MAX_PORT_NUM+(HWS_MAX_ADAPT_CTLE_DB_SIZE -1))/HWS_MAX_ADAPT_CTLE_DB_SIZE];                      /* serdes list and number per port.*/
    MV_HWS_ADAPTIVE_CTLE_PORTS_PRAMS_STC portsSerdes[MV_HWS_MAX_PORT_NUM];
     /* per serdes params*/
    MV_HWS_ADAPTIVE_CTLE_SERDES_PRAMS_STC adaptCtleParams[75];
#endif /*( defined(MV_HWS_REDUCED_BUILD_EXT_CM3) &&(defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)))*/

}MV_HWS_ADAPTIVE_CTLE_PARAMS_STC;
#endif /*RUN_ADAPTIVE_CTLE*/
#endif /*((!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) ||(defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)))*/


#ifndef MV_HWS_FREE_RTOS
/**
* @internal MV_CORE_CLOCK_GET function
* @endinternal
*
* @brief   Get Core Clock
*
* @param[in] devNum                   - Device Number
*
* @param[out] coreClkDbPtr            - DB Core Clock
* @param[out] coreClkHwPtr            - HW Core Clock
*/
typedef GT_STATUS (*MV_CORE_CLOCK_GET)
(
    IN  GT_U8   devNum,
    OUT GT_U32  *coreClkDbPtr,
    OUT GT_U32  *coreClkHwPtr
);
#endif /* MV_HWS_FREE_RTOS */


/**
 * @struct MV_HWS_ENHANCE_TUNE_LITE_PHASE_PARAMS
 *
 * @brief ETL phase parameters
*/

typedef struct{
    GT_U8   phase;
    GT_U8   phaseFinished;

} MV_HWS_ENHANCE_TUNE_LITE_PHASE_PARAMS;

/**
 * @struct MV_HWS_PORT_ENHANCE_TUNE_LITE_STC
 *
 * @brief ETL per phase parameters
*/
typedef struct{
    GT_U8       phase_CurrentDelaySize;
    GT_U8       *phase_CurrentDelayPtr;
    GT_U8       phase_InOutI;
    GT_U16      phase_Best_eye[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U8       phase_Best_dly[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U8       subPhase;
    GT_U8       parametersInitialized;
    MV_HWS_ENHANCE_TUNE_LITE_PHASE_PARAMS   phaseEnhanceTuneLiteParams;

} MV_HWS_PORT_ENHANCE_TUNE_LITE_STC;

/**
 * @struct MV_HWS_PORT_INIT_PARAMS
 *
 * @brief Port Initparameters
*/
typedef struct MV_HWS_PORT_INIT_PARAMS_STC
{
    MV_HWS_PORT_STANDARD        portStandard;
    MV_HWS_PORT_MAC_TYPE        portMacType;
    GT_U16                      portMacNumber;
    MV_HWS_PORT_PCS_TYPE        portPcsType;
    GT_U16                      portPcsNumber;
    MV_HWS_PORT_FEC_MODE        portFecMode;
    MV_HWS_SERDES_SPEED         serdesSpeed;
    GT_U16                      firstLaneNum;
    GT_U16                      activeLanesList[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U8                       numOfActLanes;
    MV_HWS_SERDES_MEDIA         serdesMediaType;
    MV_HWS_SERDES_BUS_WIDTH_ENT serdes10BitStatus;
    GT_U8                       fecSupportedModesBmp;

}MV_HWS_PORT_INIT_PARAMS;

/**
 * @struct HWS_DEVICE_INFO
 *
 * @brief HWS device Info
*/
typedef struct _HWS_DEVICE_INFO
{
    GT_U8                           devNum;
    GT_U32                          portsNum;
    MV_HWS_DEV_TYPE                 devType;
    GT_U32                          serdesType;
    GT_U32                          lastSupPortMode;
    GT_U32                          gopRev;
    MV_HWS_CONNECTION_TYPE          ipcConnType;
    MV_HWS_AVAGO_SERDES_INFO_STC    avagoSerdesInfo; /* For devices with Avago serdeses ONLY. */ /* TODO: change to ptr and dynamically allocate mem or change design to use serdesInfo */
    MV_HWS_SERDES_INFO_STC          serdesInfo;

#if defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
    MV_HWS_PORT_ENHANCE_TUNE_LITE_STC   portEnhanceTuneLiteDbPtr[MV_PORT_CTRL_MAX_AP_PORT_NUM];
#endif
#ifndef MV_HWS_FREE_RTOS
    MV_HWS_AVAGO_PORT_MANAGER_STC_PTR   *portManagerPerPortDbPtrPtr;
    MV_CORE_CLOCK_GET                   coreClockGetPtr;
#endif
#if  ((!defined(MV_HWS_REDUCED_BUILD)) ||(defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)))
#ifdef RUN_ADAPTIVE_CTLE
    MV_HWS_ADAPTIVE_CTLE_PARAMS_STC    adaptiveCtleDb;
#endif /*RUN_ADAPTIVE_CTLE*/
#endif
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(FALCON_DEV_SUPPORT)
    GT_U32 numOfTiles;
#endif
}HWS_DEVICE_INFO;

/**
 * @struct MV_INTLK_REG_DEF
 *
 * @brief HWS INTLK Reg
*/
typedef struct intlknIpRegsDef
{
    GT_U32 regOffset;
    GT_U32 defValue;
}MV_INTLK_REG_DEF;

/**
 * @struct MV_HWS_ELEMENTS_ARRAY
 *
 * @brief HWS Elements
*/
typedef struct
{
    GT_U8  devNum;
    GT_U32 portGroup;
    GT_U32 elementNum;

}MV_HWS_ELEMENTS_ARRAY;

/**
 * @struct MV_INTLK_REG
 *
 * @brief HWS INTLK Reg
*/

#if defined(BC2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
typedef struct
{
    GT_U32 ilkn0MacCfg0;
    GT_U32 ilkn0ChFcCfg0;
    GT_U32 ilkn0MacCfg2;
    GT_U32 ilkn0MacCfg3;
    GT_U32 ilkn0MacCfg4;
    GT_U32 ilkn0MacCfg6;
    GT_U32 ilkn0ChFcCfg1;
    GT_U32 ilkn0PcsCfg0;
    GT_U32 ilkn0PcsCfg1;
    GT_U32 ilkn0En;
    GT_U32 ilkn0StatEn;

}MV_INTLKN_REG;
#endif

/**
 * @enum HWS_DEV_GOP_REV
 *
 * @brief GOP revision
*/

typedef enum
{
    GOP_40NM_REV1, /* Lion2-B0/Hooper */
    GOP_40NM_REV2,
    GOP_28NM_REV1, /* AC3, BC2-A0 */
    GOP_28NM_REV2, /* BC2-B0 */
    GOP_28NM_REV3, /* BobK, Aldrin, AC3X, Bobcat3 */
    GOP_16NM_REV1  /* Raven */

}HWS_DEV_GOP_REV;

#ifdef MV_HWS_REDUCED_BUILD
extern HWS_DEVICE_INFO hwsDeviceSpecInfo[HWS_MAX_DEVICE_NUM];
#endif

#define HWS_CORE_PORTS_NUM(devNum)      hwsDeviceSpecInfo[devNum].portsNum

#if defined(RAVEN_DEV_SUPPORT)||defined(FALCON_DEV_SUPPORT)||defined(ALDRIN_DEV_SUPPORT)||defined(ALDRIN2_DEV_SUPPORT)||defined(BOBK_DEV_SUPPORT)||defined(BC3_DEV_SUPPORT)||defined(PIPE_DEV_SUPPORT)
#define HWS_DEV_SERDES_TYPE(devNum,_serdesNum) (hwsDeviceSpecInfo[devNum].serdesType)
#else
#define HWS_DEV_SERDES_TYPE(devNum,_serdesNum)                 \
((hwsDeviceSpecInfo[devNum].serdesType > COM_PHY_H && hwsDeviceSpecInfo[devNum].serdesType < COM_PHY_C12GP41P2V) ? \
(hwsDeviceSpecInfo[devNum].serdesType) : ((hwsDeviceSpecInfo[devNum].serdesType >= COM_PHY_C12GP41P2V && hwsDeviceSpecInfo[devNum].serdesType < SERDES_LAST) ? \
(hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[_serdesNum].serdesType) : SERDES_LAST))
#endif

#define HWS_DEV_SERDES_INFO(devNum)     hwsDeviceSpecInfo[devNum].serdesInfo
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
 #define HWS_DEV_SILICON_TYPE(devNum)    hwsDeviceSpecInfo[devNum].devType
#else
 #if defined(ALDRIN_DEV_SUPPORT)
  #define HWS_DEV_SILICON_TYPE(devNum) Aldrin
 #elif defined(BOBK_DEV_SUPPORT)
   #define HWS_DEV_SILICON_TYPE(devNum) BobK
 #else
  #define HWS_DEV_SILICON_TYPE(devNum)    hwsDeviceSpecInfo[devNum].devType
 #endif
#endif
#ifdef  MV_HWS_FREE_RTOS
#define HWS_DEV_SILICON_INDEX(devNum)   0
#else
#define HWS_DEV_SILICON_INDEX(devNum)   HWS_DEV_SILICON_TYPE(devNum)
#endif
#define HWS_DEV_PORT_MODES(devNum)      hwsDeviceSpecInfo[devNum].lastSupPortMode
#define HWS_DEV_GOP_REV(devNum)         hwsDeviceSpecInfo[devNum].gopRev

#define HWS_DEV_PORTS_ARRAY(devNum)     (hwsDevicesPortsElementsArray[devNum])

#define HWS_DEV_IS_BOBCAT2_A0(devNum)   ((HWS_DEV_GOP_REV(devNum) == GOP_28NM_REV1) && (HWS_DEV_SILICON_TYPE(devNum) == BobcatA0))
#define HWS_DEV_IS_BOBCAT2_B0(devNum)   ((HWS_DEV_GOP_REV(devNum) == GOP_28NM_REV2) && (HWS_DEV_SILICON_TYPE(devNum) == BobcatA0))

#define HWS_DEV_IS_AC5P_BASED_DEV_MAC(_devNum)  ((HWS_DEV_SILICON_TYPE(_devNum) == AC5P) || ((HWS_DEV_SILICON_TYPE(_devNum) == Harrier)))

/* check SIP_6_20 */
#define HWS_DEV_SIP_6_20_CHECK_MAC(_devNum)     (HWS_DEV_SILICON_TYPE(_devNum) == Harrier)

/* check SIP_6_15 */
#define HWS_DEV_SIP_6_15_CHECK_MAC(_devNum)    ((HWS_DEV_SILICON_TYPE(_devNum) == AC5X) || \
                                                 HWS_DEV_SIP_6_20_CHECK_MAC(_devNum))

/* check SIP_6_10 */
#define HWS_DEV_SIP_6_10_CHECK_MAC(_devNum)    ((HWS_DEV_SILICON_TYPE(_devNum) == AC5P) || \
                                                 HWS_DEV_SIP_6_15_CHECK_MAC(_devNum))

/* check SIP_6 */
#define HWS_DEV_SIP_6_CHECK_MAC(_devNum)       ((HWS_DEV_SILICON_TYPE(_devNum) == Falcon) || \
                                                (HWS_DEV_SILICON_TYPE(_devNum) == Raven) || \
                                                 HWS_DEV_SIP_6_10_CHECK_MAC(_devNum))

/* check SIP_5_25 */
#define HWS_DEV_SIP_5_25_CHECK_MAC(_devNum)    ((HWS_DEV_SILICON_TYPE(_devNum) == Aldrin2) || \
                                                 HWS_DEV_SIP_6_CHECK_MAC(_devNum))

/* check SIP_5_20 */
#define HWS_DEV_SIP_5_20_CHECK_MAC(_devNum)    ((HWS_DEV_SILICON_TYPE(_devNum) == Bobcat3) || \
                                                (HWS_DEV_SILICON_TYPE(_devNum) == Pipe) || \
                                                 HWS_DEV_SIP_5_25_CHECK_MAC(_devNum))

/* check SIP_5_16 */
#define HWS_DEV_SIP_5_16_CHECK_MAC(_devNum)    ((HWS_DEV_SILICON_TYPE(_devNum) == Aldrin) || \
                                                 HWS_DEV_SIP_5_20_CHECK_MAC(_devNum))

/* check SIP_5_15 */
#define HWS_DEV_SIP_5_15_CHECK_MAC(_devNum)    ((HWS_DEV_SILICON_TYPE(_devNum) == BobK) || \
                                                 HWS_DEV_SIP_5_16_CHECK_MAC(_devNum))

/* check SIP_5_10 */
#define HWS_DEV_SIP_5_10_CHECK_MAC(_devNum)    ( HWS_DEV_IS_BOBCAT2_B0(_devNum) || \
                                                 HWS_DEV_SIP_5_15_CHECK_MAC(_devNum))

/* check SIP_5 */
#define HWS_DEV_SIP_5_CHECK_MAC(_devNum)       ( HWS_DEV_IS_BOBCAT2_A0(_devNum) || \
                                                 HWS_DEV_SIP_5_10_CHECK_MAC(_devNum))

/**
* @internal MV_HWS_REDUNDANCY_VECTOR_GET_FUNC_PTR function
* @endinternal
*
* @brief   Get Vector
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
*
* @param[out] sdVector                - Vector
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_HWS_REDUNDANCY_VECTOR_GET_FUNC_PTR)
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    OUT GT_U32                  *sdVector
);

/**
* @internal MV_HWS_CLOCK_SELECT_CFG_FUNC_PTR function
* @endinternal
*
* @brief   Set Clock Select
*
* @param[in] devNum                   - Device Number
* @param[in] portGroupNum             - Port Group
* @param[in] phyPortNum               - Port Num
* @param[in] portMode                 - Port Mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_HWS_CLOCK_SELECT_CFG_FUNC_PTR)
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroupNum,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
);


/**
* @internal MV_HWS_PORT_INIT_FUNC_PTR function
* @endinternal
*
* @brief   Port Init function
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] phyPortNum               - Port Num
* @param[in] portMode                 - Port Mode
* @param[in] portParamsPtr            - Port parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_HWS_PORT_INIT_FUNC_PTR)
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS   *portParamsPtr
);

/* @internal MV_HWS_PORT_ANP_START_FUNC_PTR function
* @endinternal
*
* @brief   anp start machine function
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] phyPortNum               - Port Num
* @param[in] apCfgPtr                 - Port Ap parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_HWS_PORT_ANP_START_FUNC_PTR)
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_AP_CFG                  *apCfgPtr
);

/**
* @internal MV_HWS_PORT_RESET_FUNC_PTR function
* @endinternal
*
* @brief   Port Reset function
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] phyPortNum               - Port Num
* @param[in] portMode                 - Port Mode
* @param[in] action                   - Reset/port down
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_HWS_PORT_RESET_FUNC_PTR)
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_ACTION      action
);

/**
* @internal MV_HWS_PORT_PSYNC_BYPASS_FUNC_PTR function
* @endinternal
*
* @brief   Port PSYNC function
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] phyPortNum               - Port Num
* @param[in] portMode                 - Port Mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_HWS_PORT_PSYNC_BYPASS_FUNC_PTR)
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal MV_HWS_PORT_FIX_ALIGN90_FUNC_PTR function
* @endinternal
*
* @brief   Port Align 90 function
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] phyPortNum               - Port Num
* @param[in] portMode                 - Port Mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_HWS_PORT_FIX_ALIGN90_FUNC_PTR)
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal MV_HWS_PORT_AUTO_TUNE_SET_EXT_FUNC_PTR function
* @endinternal
*
* @brief   Port Auto Tune EXT function
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] phyPortNum               - Port Num
* @param[in] portMode                 - Port Mode
* @param[in] portTuningMode           - Tuning mode
* @param[in] optAlgoMask              - Opt algorithm
* @param[in] results                  - Results
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_HWS_PORT_AUTO_TUNE_SET_EXT_FUNC_PTR)
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_AUTO_TUNE_MODE      portTuningMode,
    IN GT_U32                  optAlgoMask,
    IN void *                  results
);

/**
* @internal MV_HWS_PORT_AUTO_TUNE_SET_WA_FUNC_PTR function
* @endinternal
*
* @brief   Port Auto Tune WA function
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] phyPortNum               - Port Num
* @param[in] portMode                 - Port Mode
* @param[in] curLanesList             - Lanes list
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_HWS_PORT_AUTO_TUNE_SET_WA_FUNC_PTR)
(
       IN GT_U8                devNum,
       IN GT_U32               portGroup,
       IN GT_U32               phyPortNum,
       IN MV_HWS_PORT_STANDARD portMode,
       IN GT_U32               *curLanesList
);

/**
* @internal MV_HWS_PORT_AUTO_TUNE_STOP_FUNC_PTR function
* @endinternal
*
* @brief   Port Auto Tune Stop function
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] phyPortNum               - Port Num
* @param[in] portMode                 - Port Mode
* @param[in] stopRx                   - Stop Rx
* @param[in] stopTx                   - Stop Tx
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
typedef GT_STATUS (*MV_HWS_PORT_AUTO_TUNE_STOP_FUNC_PTR)
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 stopRx,
    IN GT_BOOL                 stopTx
);

/**
* @internal MV_HWS_PORT_GEAR_BOX_FUNC_PTR function
* @endinternal
*
* @brief   Gear Box function
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
typedef GT_STATUS (*MV_HWS_PORT_GEAR_BOX_FUNC_PTR)
(
    void
);

/**
* @internal MV_HWS_PORT_EXT_MODE_CFG_FUNC_PTR function
* @endinternal
*
* @brief   Port Extended mode configuration function
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] phyPortNum               - Port Num
* @param[in] portMode                 - Port Mode
* @param[in] extendedMode             - Extended mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_HWS_PORT_EXT_MODE_CFG_FUNC_PTR)
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 extendedMode
);

/**
* @internal MV_HWS_PORT_EXT_MODE_CFG_GET_FUNC_PTR function
* @endinternal
*
* @brief   Port Extended mode configuration Get function
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] phyPortNum               - Port Num
* @param[in] portMode                 - Port Mode
* @param[out] extendedMode             - Extended mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS  (*MV_HWS_PORT_EXT_MODE_CFG_GET_FUNC_PTR)
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *extendedMode
);

#if 0
typedef GT_STATUS(*MV_HWS_PORT_BETTER_ADAPTATION_SET_FUNC_PTR)
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_BOOL     *betterAlign90
);
#endif

/**
* @internal MV_HWS_PORT_FINE_TUNE_FUNC_PTR function
* @endinternal
*
* @brief   Port Fine Tune function
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] phyPortNum               - Port Num
* @param[in] portMode                 - Port Mode
* @param[in] configPpm                - PPM T/F
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS  (*MV_HWS_PORT_FINE_TUNE_FUNC_PTR)
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 configPpm
);


/**
* @internal MV_HWS_PORT_LB_SET_WA_FUNC_PTR function
* @endinternal
*
* @brief   Port LB WA set function
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] phyPortNum               - Port Num
* @param[in] portMode                 - Port Mode
* @param[in] lbType                   - LB type
* @param[in] curLanesList             - Lanes list
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_HWS_PORT_LB_SET_WA_FUNC_PTR)
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_U32                  lbType,
    IN GT_U32                  *curLanesList
);

/**
* @internal MV_HWS_PORT_PARAMS_INDEX_GET_FUNC_PTR function
* @endinternal
*
* @brief   Port Param index get function
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] phyPortNum               - Port Num
* @param[in] portMode                 - Port Mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_U32 (*MV_HWS_PORT_PARAMS_INDEX_GET_FUNC_PTR)
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal MV_HWS_PCS_MARK_MODE_FUNC_PTR function
* @endinternal
*
* @brief   PCS Mark mode function
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] phyPortNum               - Port Num
* @param[in] portMode                 - Port Mode
* @param[in] enable                   - Enable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS  (*MV_HWS_PCS_MARK_MODE_FUNC_PTR)
(
    IN GT_U8                   devNum,
    IN GT_UOPT                 portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 enable
);

/**
* @internal MV_HWS_XPCS_CONNECT_WA_FUNC_PTR function
* @endinternal
*
* @brief   Port XPCS Connect WA function
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] phyPortNum               - Port Num
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_HWS_XPCS_CONNECT_WA_FUNC_PTR)
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portNum
);

/**
* @internal MV_HWS_PORT_PARAMS_SET_FUNC_PTR function
* @endinternal
*
* @brief   Port Port Param set function
*
* @param[in] devNum                   - Device Number
* @param[in] portGroupNum             - Port Group
* @param[in] phyPortNum               - Port Num
* @param[in] portMode                 - Port Mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_HWS_PORT_PARAMS_SET_FUNC_PTR)
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal MV_HWS_PORT_PARAMS_GET_LANES_FUNC_PTR function
* @endinternal
*
* @brief   Port Lanes Get function
*
* @param[in] devNum                   - Device Number
* @param[in] portGroupNum             - Port Group
* @param[in] phyPortNum               - Port Num
* @param[out] portParams               - Port Parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_HWS_PORT_PARAMS_GET_LANES_FUNC_PTR)
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      portNum,
    OUT MV_HWS_PORT_INIT_PARAMS*    portParams
);

/**
* @internal MV_HWS_CORE_CLOCK_GET_FUNC_PTR function
* @endinternal
*
* @brief   Port Core Clock Get function
*
* @param[in] devNum                   - Device Number
* @param[out] coreClkDbPtr             - DB Core Clock
* @param[out] coreClkHwPtr             - HW Core Clock
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_U32 (*MV_HWS_CORE_CLOCK_GET_FUNC_PTR)
(
    IN GT_U8                       devNum,
    OUT GT_U32                      *coreClkDbPtr,
    OUT GT_U32                      *coreClkHwPtr
);

/**
* @internal MV_HWS_SERDES_REF_CLOCK_GET_FUNC_PTR function
* @endinternal
*
* @brief   Serdes Reference Clock Get function
*
* @param[in] devNum                   - Device Number
* @param[out] refClockPtr             - Serdes Reference Clock
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/

typedef GT_U32 (*MV_HWS_SERDES_REF_CLOCK_GET_FUNC_PTR)
(
 IN  GT_U8                      devNum,
 OUT GT_U32                     *refClockPtr
);

/* @internal MV_HWS_PORT_ACCESS_CHECK_FUNC_PTR function
* @endinternal
*
* @brief   check if port is accessible
*
* @param[in] devNum                   - Device Number
* @param[in] phyPortNum               - Port Num
* @param[in] portMode                 - port standard metric
* @param[out] accessEnablePtr         - (pointer to) access enabled/disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_HWS_PORT_ACCESS_CHECK_FUNC_PTR)
(
    IN GT_U8                    devNum,
    IN GT_U32                   phyPortNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT GT_BOOL                 *accessEnablePtr
);

/**
* @internal MV_SYS_DEVICE_INFO function
* @endinternal
*
* @brief   Return silicon ID and revision ID for current device number.
*
* @param[in] devNum                   - system device number
*
* @param[out] devId               - silicon ID
* @param[out] revNum              - revision number
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_SYS_DEVICE_INFO)
(
    IN  GT_U8  devNum,
    OUT GT_U32 *devId,
    OUT GT_U32 *revNum
);

/**
 * @struct MV_HWS_DEV_FUNC_PTRS
 *
 * @brief Device Function pointers
*/
typedef struct
{
    MV_HWS_PORT_RESET_FUNC_PTR              devPortResetFunc;
    MV_HWS_PORT_AUTO_TUNE_SET_EXT_FUNC_PTR  portAutoTuneSetExtFunc;
    MV_SYS_DEVICE_INFO                      deviceInfoFunc;
    MV_HWS_PORT_INIT_FUNC_PTR               devApPortinitFunc;
    MV_HWS_CORE_CLOCK_GET_FUNC_PTR          coreClockGetFunc;
    MV_HWS_PORT_PARAMS_GET_LANES_FUNC_PTR   portParamsGetLanesFunc;
#ifndef RAVEN_DEV_SUPPORT
    MV_HWS_SERDES_REF_CLOCK_GET_FUNC_PTR    serdesRefClockGetFunc;
    MV_HWS_PORT_INIT_FUNC_PTR               devPortinitFunc;
    MV_HWS_CLOCK_SELECT_CFG_FUNC_PTR        clkSelCfgGetFunc;
    MV_HWS_PORT_AUTO_TUNE_SET_WA_FUNC_PTR   portAutoTuneSetWaFunc;
    MV_HWS_PORT_EXT_MODE_CFG_FUNC_PTR       portExtModeCfgFunc;
    MV_HWS_PORT_FIX_ALIGN90_FUNC_PTR        portFixAlign90Func;
    MV_HWS_PORT_LB_SET_WA_FUNC_PTR          portLbSetWaFunc;
    MV_HWS_PORT_AUTO_TUNE_STOP_FUNC_PTR     portAutoTuneStopFunc;
    MV_HWS_PORT_EXT_MODE_CFG_GET_FUNC_PTR   portExtModeCfgGetFunc;
    MV_HWS_PORT_PARAMS_SET_FUNC_PTR         portParamsSetFunc;
    MV_HWS_PORT_PARAMS_INDEX_GET_FUNC_PTR   portParamsIndexGetFunc;
    MV_HWS_REDUNDANCY_VECTOR_GET_FUNC_PTR   redundVectorGetFunc;
    MV_HWS_PCS_MARK_MODE_FUNC_PTR           pcsMarkModeFunc;
    MV_HWS_PORT_ANP_START_FUNC_PTR          devPortAnpStartFunc;
    MV_HWS_PORT_ACCESS_CHECK_FUNC_PTR       devPortAccessCheckFunc;

#endif
}MV_HWS_DEV_FUNC_PTRS;


/**
* @internal MV_SIL_ILKN_REG_DB_GET function
* @endinternal
*
* @brief   return a ILKN register shadow Db.
*
* @param[in] devNum                  - system device number
* @param[in] portGroup               - Port Group
* @param[in] address                 - Address
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
#if defined(BC2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
typedef MV_INTLKN_REG* (*MV_SIL_ILKN_REG_DB_GET)
(
    GT_U8 devNum,
    GT_U32 portGroup,
    GT_U32 address
);
#endif

#if defined(BC2_DEV_SUPPORT)
extern MV_SIL_ILKN_REG_DB_GET hwsIlknRegDbGetFuncPtr;
#endif
/* os wrapper function prototypes */
/**
* @internal MV_OS_MALLOC_FUNC function
* @endinternal
*
* @brief   Allocates memory block of specified size.
*
* @param[in] size                  - Bytes to allocate
*
*
* @retval Void pointer to the allocated space, or NULL if there is
*       insufficient memory available
*/
typedef void * (*MV_OS_MALLOC_FUNC)
(
    IN GT_U32 size
);
/**
* @internal MV_OS_FREE_FUNC function
* @endinternal
*
* @brief   De-allocates or frees a specified memory block.
*
* @param[in] memblock                  - previously allocated
*       memory block to be freed
*
*
* @retval none
*/
typedef void (*MV_OS_FREE_FUNC)
(
    IN void* const memblock
);

/**
* @internal MV_OS_MEM_SET_FUNC function
* @endinternal
*
* @brief   Stores 'symbol' converted to an unsigned char in each of the elements
*       of the array of unsigned char beginning at 'start', with size 'size'.
*
* @param[in] start                  - start address of memory
*       block for setting
* @param[in] symbol                 - character to store, converted to an unsigned char
* @param[in] size                   - size of block to be set
*
*
*
* @retval none
*/
typedef void * (*MV_OS_MEM_SET_FUNC)
(
    IN void * start,
    IN int    symbol,
    IN GT_U32 size
);

/**
* @internal MV_OS_TIME_WK_AFTER_FUNC function
* @endinternal
*
* @brief   Puts current task to sleep for specified number of millisecond.
*
* @param[in] mils                  - time to sleep in milliseconds
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_OS_TIME_WK_AFTER_FUNC)
(
    IN GT_U32 mils
);

/**
* @internal MV_OS_EXACT_DELAY_FUNC function
* @endinternal
*
* @brief   Implement exact time delay for specified number of
*          millisecond.
*
* @param[in] devNum                  - system device number
* @param[in] portGroup               - Port Group
* @param[in] mils                  - time to delay in
*       milliseconds
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_OS_EXACT_DELAY_FUNC)
(
    IN GT_U8  devNum,
    IN GT_U32 portGroup,
    IN GT_U32 mils
);

/**
* @internal MV_OS_MICRO_DELAY_FUNC function
* @endinternal
*
* @brief   Implement delay for specified number of micro seconds.
*
* @param[in] devNum                  - system device number
* @param[in] portGroup               - Port Group
* @param[in] microSec                - time to delay in micro
*       seconds
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_OS_MICRO_DELAY_FUNC)
(
    IN GT_U8  devNum,
    IN GT_U32 portGroup,
    IN GT_U32 microSec
);

/**
* @internal MV_SERDES_REG_ACCESS_SET function
* @endinternal
*
* @brief   Implement write access to SERDES external/internal
*          registers.
*
* @param[in] devNum         - system device number
* @param[in] portGroup      - Port Group
* @param[in] serdesNum      - seredes number to access
* @param[in] regAddr        - serdes register address (offset)
*       to access
* @param[in] data           - data to write
* @param[in] mask           - mask write
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_SERDES_REG_ACCESS_SET)
(
    IN  GT_U8        devNum,
    IN  GT_UOPT      portGroup,
    IN  GT_U8        regType,
    IN  GT_UOPT      serdesNum,
    IN  GT_UREG_DATA regAddr,
    IN  GT_UREG_DATA data,
    IN  GT_UREG_DATA mask
);

/**
* @internal MV_REG_ACCESS_SET function
* @endinternal
*
* @brief   Implement write access to external/internal registers.
*
* @param[in] devNum         - system device number
* @param[in] portGroup      - Port Group
* @param[in] address        - register address (offset) to
*       access
* @param[in] data           - data to write
* @param[in] mask           - mask write
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_REG_ACCESS_SET)
(
    IN GT_U8 devNum,
    IN GT_U32 portGroup,
    IN GT_U32 address,
    IN GT_U32 data,
    IN GT_U32 mask
);

/**
* @internal MV_SERDES_REF_CLOCK_GET function
* @endinternal
*
* @brief   Serdes Reference Clock Get function
*
* @param[in] devNum                   - Device Number
* @param[out] refClockPtr             - Serdes Reference Clock
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_U32 (*MV_SERDES_REF_CLOCK_GET)
(
 IN  GT_U8                      devNum,
 OUT MV_HWS_REF_CLOCK_SUP_VAL   *refClockPtr
);

/**
* @internal MV_SERDES_REG_ACCESS_GET function
* @endinternal
*
* @brief   Implement read access from SERDES external/internal registers.
*
* @param[in] devNum         - system device number
* @param[in] portGroup      - Port Group
* @param[in] serdesNum      - seredes number to access
* @param[in] regAddr        - serdes register address (offset)
*       to access
* @param[out] data           - read data
* @param[in] mask           - mask write
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_SERDES_REG_ACCESS_GET)
(
    IN  GT_U8        devNum,
    IN  GT_UOPT      portGroup,
    IN  GT_U8        regType,
    IN  GT_UOPT      serdesNum,
    IN  GT_UREG_DATA regAddr,
    OUT  GT_UREG_DATA *data,
    IN  GT_UREG_DATA mask
);

/**
* @internal MV_REG_ACCESS_GET function
* @endinternal
*
* @brief   Implement read access to external/internal registers.
*
* @param[in] devNum         - system device number
* @param[in] portGroup      - Port Group
* @param[in] address        - register address (offset) to
*       access
* @param[out] data           - read data
* @param[in] mask           - mask write
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_REG_ACCESS_GET)
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroup,
    IN  GT_U32  address,
    OUT GT_U32  *data,
    IN  GT_U32  mask
);


/**
* @internal MV_SERVER_REG_ACCESS_SET function
* @endinternal
*
* @brief   definition of server write prototype.
*
* @param[in] devNum         - system device number
* @param[in] addr           - register address (offset) to
*       access
* @param[in] data           - read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_SERVER_REG_ACCESS_SET)
(
    IN GT_U8 devNum,
    IN GT_U32 addr,
    IN GT_U32 data
);

/**
* @internal MV_SERVER_REG_ACCESS_GET function
* @endinternal
*
* @brief   definition of server read prototype.
*
* @param[in] devNum         - system device number
* @param[in] addr           - register address (offset) to
*       access
* @param[out] data           - read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_SERVER_REG_ACCESS_GET)
(
    IN GT_U8 devNum,
    IN GT_U32 addr,
    OUT GT_U32 *data
);

/**
* @internal MV_SERVER_REG_FIELD_ACCESS_SET function
* @endinternal
*
* @brief   definition of server reg field write prototype.
*
* @param[in] devNum         - system device number
* @param[in] regAddr        - register address (offset) to
*       access
* @param[in] fieldOffset   - Offset within thereg to write to
* @param[in] fieldLength   - Field length
* @param[in] fieldData     = Field data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_SERVER_REG_FIELD_ACCESS_SET)
(
    IN GT_U8   devNum,
    IN GT_U32  regAddr,
    IN GT_U32  fieldOffset,
    IN GT_U32  fieldLength,
    IN GT_U32  fieldData
);

/**
* @internal MV_SERVER_REG_FIELD_ACCESS_GET function
* @endinternal
*
* @brief   definition of server reg field read prototype.
*
* @param[in] devNum         - system device number
* @param[in] regAddr        - register address (offset) to
*       access
* @param[in] fieldOffset   - Offset within thereg to write to
* @param[in] fieldLength   - Field length
* @param[out] fieldData     = Field data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_SERVER_REG_FIELD_ACCESS_GET)
(
    IN GT_U8   devNum,
    IN GT_U32  regAddr,
    IN GT_U32  fieldOffset,
    IN GT_U32  fieldLength,
    OUT GT_U32  *fieldData
);

/**
* @internal MV_TIMER_GET function
* @endinternal
*
* @brief   definition of get timer prototype
*
* @param[out] seconds         - Seconds
* @param[out] nanoSeconds     - NanoSeconds
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_TIMER_GET)
(
    OUT GT_U32  *seconds,
    OUT GT_U32  *nanoSeconds
);

/**
* @internal MV_OS_MEM_COPY_FUNC function
* @endinternal
*
* @brief   Copies 'size' characters from the object pointed to by 'source' into
*       the object pointed to by 'destination'. If copying takes place between
*       objects that overlap, the behavior is undefined.
*
* @param[in] destination - destination of copy
* @param[in] source      - source of copy
* @param[in] size        - size of memory to copy
*
* @retval Pointer to destination
*/
typedef void (*MV_OS_MEM_COPY_FUNC)
(
    IN void *       destination,
    IN const void * source,
    IN GT_U32       size
);

/**
* @internal MV_OS_STR_CAT_FUNC function
* @endinternal
*
* @brief   Appends a copy of string 'str2' to the end of string 'str1'.
*
* @param[in] str1   - destination string
* @param[in] str2   - string to add the destination string
*
* @retval Pointer to destination
*/
typedef GT_CHAR* (*MV_OS_STR_CAT_FUNC)
(
    IN GT_CHAR         *str1,
    IN const GT_CHAR   *str2
);

typedef GT_STATUS (*MV_XSMI_ACCESS_GET)
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U16 phyAddress,
    GT_U16 devAddress,
    GT_U16 regAddr,
    GT_U16 *dataPtr
);

typedef GT_STATUS (*MV_XSMI_ACCESS_SET)
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U16 phyAddress,
    GT_U16 devAddress,
    GT_U16 regAddr,
    GT_U16 data
);

/**
* @struct HWS_OS_FUNC_PTR
* @endinternal
*
* @brief   Structure that hold the "os" functions needed be
*          bound to HWS
*
*/
typedef struct {
  MV_OS_EXACT_DELAY_FUNC   osExactDelayPtr;
  MV_OS_MICRO_DELAY_FUNC   osMicroDelayPtr;
  MV_OS_TIME_WK_AFTER_FUNC osTimerWkPtr;
  MV_OS_MEM_SET_FUNC       osMemSetPtr;
  MV_OS_FREE_FUNC          osFreePtr;
  MV_OS_MALLOC_FUNC        osMallocPtr;
  MV_SYS_DEVICE_INFO       sysDeviceInfo;
  MV_SERDES_REF_CLOCK_GET  serdesRefClockGet;
  MV_SERDES_REG_ACCESS_GET serdesRegGetAccess;
  MV_SERDES_REG_ACCESS_SET serdesRegSetAccess;
  MV_SERVER_REG_ACCESS_GET serverRegGetAccess;
  MV_SERVER_REG_ACCESS_SET serverRegSetAccess;
  MV_OS_MEM_COPY_FUNC      osMemCopyPtr;
  MV_OS_STR_CAT_FUNC       osStrCatPtr;
  MV_SERVER_REG_FIELD_ACCESS_SET serverRegFieldSetAccess;
  MV_SERVER_REG_FIELD_ACCESS_GET serverRegFieldGetAccess;
  MV_TIMER_GET             timerGet;
  MV_REG_ACCESS_GET        registerGetAccess;
  MV_REG_ACCESS_SET        registerSetAccess;
  void                     *extFunctionStcPtr;
#ifndef  MV_HWS_FREE_RTOS
  MV_CORE_CLOCK_GET        coreClockGetPtr;
#endif

}HWS_OS_FUNC_PTR;

#ifndef  MV_HWS_FREE_RTOS
/**
* @internal MV_OS_GET_DEVICE_DRIVER_FUNC function
* @endinternal
*
* @brief   Device Driver functions
*
* @param[in] devNum         - system device number
* @param[in] portGroupId    - Port Group
* @param[in] smiIntr        - SMI interface
* @param[in] slaveSmiPhyId  - SMI slave ID
* @param[out] ssmiPtr       - SMI PTR
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_OS_GET_DEVICE_DRIVER_FUNC)
(
 IN  GT_U8                       devNum,
 IN  GT_U32                      portGroupId,
 IN  GT_U32                      smiIntr,
 IN  GT_U32                      slaveSmiPhyId,
 /*OUT CPSS_HW_INFO_STC          *hwInfoPtr*/
 OUT CPSS_HW_DRIVER_STC          **ssmiPtr
);

/**
* @internal MV_OS_HW_TRACE_ENABLE_FUNC function
* @endinternal
*
* @brief   Enable Trace
*
* @param[in] devNum       - system device number
* @param[in] traceType    - 0=read, 1=write, 2=both,
*       3=write_delay, 4=all
* @param[in] enable        - True/False
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
typedef GT_STATUS (*MV_OS_HW_TRACE_ENABLE_FUNC)
(
 IN  GT_U8                       devNum,
 IN  GT_U32                      traceType,/*0=read, 1=write, 2=both, 3=write_delay, 4=all*/
 IN  GT_BOOL                     enable
);

/**
* @struct HWS_EXT_FUNC_STC_PTR
* @endinternal
*
* @brief   Exteranal Functions
*
*/
typedef struct {
    MV_OS_GET_DEVICE_DRIVER_FUNC getDeviceDriver;
    MV_OS_HW_TRACE_ENABLE_FUNC   ppHwTraceEnable;
}HWS_EXT_FUNC_STC_PTR;

#endif

#ifdef MV_HWS_REDUCED_BUILD
extern MV_OS_EXACT_DELAY_FUNC   hwsOsExactDelayPtr;
extern MV_OS_MICRO_DELAY_FUNC   hwsOsMicroDelayPtr;
extern MV_OS_TIME_WK_AFTER_FUNC hwsOsTimerWkFuncPtr;
extern MV_OS_MEM_SET_FUNC       hwsOsMemSetFuncPtr;
extern MV_OS_FREE_FUNC          hwsOsFreeFuncPtr;
extern MV_OS_MALLOC_FUNC        hwsOsMallocFuncPtr;
extern MV_SERDES_REG_ACCESS_SET hwsSerdesRegSetFuncPtr;
extern MV_SERDES_REG_ACCESS_GET hwsSerdesRegGetFuncPtr;
extern MV_OS_MEM_COPY_FUNC      hwsOsMemCopyFuncPtr;
extern MV_OS_STR_CAT_FUNC       hwsOsStrCatFuncPtr;
extern MV_SERVER_REG_ACCESS_SET hwsServerRegSetFuncPtr;
extern MV_SERVER_REG_ACCESS_GET hwsServerRegGetFuncPtr;
extern MV_SERVER_REG_FIELD_ACCESS_SET hwsServerRegFieldSetFuncPtr;
extern MV_SERVER_REG_FIELD_ACCESS_GET hwsServerRegFieldGetFuncPtr;
extern MV_TIMER_GET             hwsTimerGetFuncPtr;
extern MV_REG_ACCESS_SET        hwsRegisterSetFuncPtr;
extern MV_REG_ACCESS_GET        hwsRegisterGetFuncPtr;
#endif

extern GT_STATUS hwsRegisterSetFieldFunc
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  regAddr,
    GT_U32  fieldOffset,
    GT_U32  fieldLength,
    GT_U32  fieldData
);
extern GT_STATUS hwsRegisterGetFieldFunc
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  regAddr,
    GT_U32  fieldOffset,
    GT_U32  fieldLength,
    GT_U32  *fieldDataPtr
);

/**
* @internal hwsOsLocalMicroDelay function
* @endinternal
*
* @brief   This API is used in case micro sec counter is not supported
*         It will convert micro to msec and round up in case needed
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] microSec                 - Delay in micro sec
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsOsLocalMicroDelay
(
    IN GT_U8  devNum,
    IN GT_U32 portGroup,
    IN GT_U32 microSec
);

/**
* @internal mvUnitExtInfoGet function
* @endinternal
*
* @brief   Return silicon specific base address and index for specified unit based on
*         unit index.
* @param[in] devNum                   - Device Number
* @param[in] unitId                   - unit ID (MAC, PCS, SERDES)
* @param[in] unitIndex                - unit index
*
* @param[out] baseAddr                 - unit base address in device
* @param[out] unitIndex                - unit index in device
* @param[out] localUnitNumPtr          - (pointer to) local unit number (support for Bobcat3 multi-pipe)
*                                       None
*/
GT_STATUS  mvUnitExtInfoGet
(
    IN GT_U8           devNum,
    IN MV_HWS_UNITS_ID unitId,
    IN GT_U32           unitNum,
    OUT GT_U32          *baseAddr,
    OUT GT_U32          *unitIndex,
    OUT GT_U32          *localUnitNumPtr
);

/**
* @internal mvUnitInfoGet function
* @endinternal
*
* @brief   Return silicon specific base address and index for specified unit
*
* @param[in] devNum                   - Device Number
* @param[in] unitId                   - unit ID (MAC, PCS, SERDES)
*
* @param[out] baseAddr                 - unit base address in device
* @param[out] unitIndex                - unit index in device
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS  mvUnitInfoGet
(
    IN GT_U8           devNum,
    IN MV_HWS_UNITS_ID unitId,
    OUT GT_U32          *baseAddr,
    OUT GT_U32          *unitIndex
);

/**
* @internal mvUnitInfoSet function
* @endinternal
*
* @brief   Init silicon specific base address and index for specified unit
*
* @param[in] devType                  - Device type
* @param[in] unitId                   - unit ID (MAC, PCS, SERDES)
* @param[in] baseAddr                 - unit base address in device
* @param[in] unitIndex                - unit index in device
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS  mvUnitInfoSet
(
    IN MV_HWS_DEV_TYPE devType,
    IN MV_HWS_UNITS_ID unitId,
    IN GT_U32          baseAddr,
    IN GT_U32          unitIndex
);

/**
* @internal mvUnitInfoGetByAddr function
* @endinternal
*
* @brief   Return unit ID by unit address in device
*
* @param[in] devNum                   - Device Number
* @param[in] baseAddr                 - unit base address in device
*
* @param[out] unitId                   - unit ID (MAC, PCS, SERDES)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS  mvUnitInfoGetByAddr
(
    IN GT_U8           devNum,
    IN GT_U32          baseAddr,
    OUT MV_HWS_UNITS_ID *unitId
);

/**
* @internal genUnitRegisterSet function
* @endinternal
*
* @brief   Implement write access to device registers.
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - port group (core) number
* @param[in] unitId                   -  unit ID
* @param[in] unitNum                  -  unit Num
* @param[in] regOffset                -  Reg Offset
* @param[in] data                     -  to write
* @param[in] mask                     -  Mask
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genUnitRegisterSet
(
    IN GT_U8           devNum,
    IN GT_UOPT         portGroup,
    IN MV_HWS_UNITS_ID unitId,
    IN GT_UOPT         unitNum,
    IN GT_UREG_DATA    regOffset,
    IN GT_UREG_DATA    data,
    IN GT_UREG_DATA    mask
);

/**
* @internal genUnitRegisterGet function
* @endinternal
*
* @brief   Read access to device registers.
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - port group (core) number
* @param[in] unitId                   -  unit ID
* @param[in] unitNum                  -  unit Num
* @param[in] regOffset                -  Reg Offset
* @param[OUT] data                    -  to read
* @param[in] mask                     -  Mask
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genUnitRegisterGet
(
    IN GT_U8           devNum,
    IN GT_UOPT         portGroup,
    IN MV_HWS_UNITS_ID unitId,
    IN GT_UOPT         unitNum,
    IN GT_UREG_DATA    regOffset,
    OUT GT_UREG_DATA    *data,
    IN GT_UREG_DATA    mask
);

/**
* @internal genInterlakenRegSet function
* @endinternal
*
* @brief   Implement write access to INERLAKEN IP registers.
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - port group (core) number
* @param[in] address                  -  to access
* @param[in] data                     -  to write
* @param[in] mask                     -  mask
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genInterlakenRegSet
(
    IN GT_U8 devNum,
    IN GT_U32 portGroup,
    IN GT_U32 address,
    IN GT_U32 data,
    IN GT_U32 mask
);

/**
* @internal genInterlakenRegGet function
* @endinternal
*
* @brief   Implement read access to INERLAKEN IP registers.
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - port group (core) number
* @param[in] address                  -  to access
*
* @param[out] data                     - read data
* @param[in] mask                     -  mask
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genInterlakenRegGet
(
    IN GT_U8 devNum,
    IN GT_U32 portGroup,
    IN GT_U32 address,
    OUT GT_U32 *data,
    IN GT_U32 mask
);

#if defined(CHX_FAMILY) || !defined(PX_FAMILY)
/**
* @internal hwsLion2IfInit function
* @endinternal
*
* @brief   Init all supported units needed for ports initialization
*
* @param[in] devNum                   - Device Number
* @param[in] funcPtr                   - structure that hold the
*       "os" functions needed be bound to HWS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsLion2IfInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsLion2IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @param[in] devNum                   - Device Number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsLion2IfClose
(
    IN GT_U8 devNum
);

/**
* @internal hwsHooperIfInit function
* @endinternal
*
* @brief   Init all supported units needed for ports initialization
*
* @param[in] devNum                   - Device Number
* @param[in] funcPtr                   - structure that hold the
*       "os" functions needed be bound to HWS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsHooperIfInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsHooperIfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @param[in] devNum                   - Device Number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsHooperIfClose
(
    IN GT_U8 devNum
);

/**
* @internal hwsNp5IfInit function
* @endinternal
*
* @brief   Init all supported units needed for ports initialization
*
* @param[in] devNum                   - Device Number
* @param[in] funcPtr                   - structure that hold the
*       "os" functions needed be bound to HWS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsNp5IfInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsNp5IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @param[in] devNum                   - Device Number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsNp5IfClose
(
    IN GT_U8 devNum
);

/**
* @internal hwsBobcat2IfInit function
* @endinternal
*
* @brief   Init all supported units needed for ports initialization
*
* @param[in] devNum                   - Device Number
* @param[in] funcPtr                   - structure that hold the
*       "os" functions needed be bound to HWS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobcat2IfInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsBobcat2IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @param[in] devNum                   - Device Number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsBobcat2IfClose
(
    IN GT_U8 devNum
);

GT_VOID hwsAlleycat3IfInitHwsDevFunc
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
);

#ifdef SHARED_MEMORY

GT_STATUS hwsAc3IfReIoad
(
    GT_U8 devNum
);

#endif

/**
* @internal mvHwsAlleycat3IfInit function
* @endinternal
*
* @brief   Init all software related DB: DevInfo, Port (Port modes, MAC, PCS and SERDES)
*         and address mapping.
*         Must be called per device.
*
* @param[in] devNum                   - Device Number
* @param[in] funcPtr                   - structure that hold the
*       "os" functions needed be bound to HWS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAlleycat3IfInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

GT_VOID hwsAlleycat5IfInitHwsDevFunc
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
);


GT_VOID hwsAlleycat5PortsParamsSupModesMapSet
(
    GT_U8 devNum
);


#ifdef SHARED_MEMORY

GT_STATUS hwsAlleycat5IfReIoad
(
    GT_U8 devNum
);

#endif

/**
* @internal mvHwsAlleycat5IfInit function
* @endinternal
*
* @brief   Init all software related DB: DevInfo, Port (Port modes, MAC, PCS and SERDES)
*         and address mapping.
*         Must be called per device.
*
* @param[in] devNum                   - Device Number
* @param[in] funcPtr                   - structure that hold the
*       "os" functions needed be bound to HWS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAlleycat5IfInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsAlleycat3IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @param[in] devNum                   - Device Number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsAlleycat3IfClose
(
    IN GT_U8 devNum
);

/**
* @internal hwsAlleycat5IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsAlleycat5IfClose
(
    IN GT_U8 devNum
);

/**
* @internal hwsBobKIfPreInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @param[in] devNum                   - Device Number
* @param[in] funcPtr                   - structure that hold the
*       "os" functions needed be bound to HWS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobKIfPreInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsBobKConvertSbusAddrToSerdes function
* @endinternal
*
* @brief   Convert sbus address to serdes number.
*
* @param[in] devNum                   - Device Number
* @param[in] sbusAddr                - sbus address
*
* @param[out] serdesNum               - serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int hwsBobKConvertSbusAddrToSerdes
(
    IN unsigned char devNum,
    OUT GT_U32 *serdesNum,
    IN unsigned int  sbusAddr
);

/**
* @internal hwsBobKIfInit function
* @endinternal
*
* @brief   Init all software related DB: DevInfo, Port (Port modes, MAC, PCS and SERDES)
*         and address mapping.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobKIfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

#ifndef MV_HWS_REDUCED_BUILD

GT_VOID hwsBobKPortsParamsSupModesMapSet
(
    GT_U8 devNum
);

#endif

GT_VOID hwsBobKIfInitHwsDevFunc
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsAldrinSerdesIfInit function
* @endinternal
*
* @brief   Init all supported Serdes types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsAldrinSerdesIfInit(GT_U8 devNum);

/**
* @internal hwsAldrinIfPreInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @param[in] devNum                   - Device Number
* @param[in] funcPtr                   - structure that hold the
*       "os" functions needed be bound to HWS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsAldrinIfPreInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsBobcat3IfPreInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @param[in] devNum                   - Device Number
* @param[in] funcPtr                   - structure that hold the
*       "os" functions needed be bound to HWS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobcat3IfPreInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsAldrin2IfPreInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @param[in] devNum                   - Device Number
* @param[in] funcPtr                   - structure that hold the
*       "os" functions needed be bound to HWS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsAldrin2IfPreInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsFalconIfPreInit function
* @endinternal
*
* @brief   Falcon pre init
*
* @param[in] devNum                   - Device Number
* @param[in] funcPtr                   - structure that hold the
*       "os" functions needed be bound to HWS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsFalconIfPreInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);


/**
* @internal hwsFalconNumOfTilesUpdate function
* @endinternal
*
* @brief   Falcon number of tiles update
*
* @param[in] devNum                   - Device Number
* @param[in] numOfTiles              - number of tiles
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsFalconNumOfTilesUpdate
(
    IN GT_U8 devNum,
    IN GT_U32 numOfTiles
);

GT_VOID hwsAldrinPortsParamsSupModesMapSet
(
    GT_U8 devNum
);

GT_VOID hwsAldrinIfInitHwsDevFunc
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsAldrinIfInit function
* @endinternal
*
* @brief   Init all software related DB: DevInfo, Port (Port modes, MAC, PCS and SERDES)
*         and address mapping.
*         Must be called per device.
*
* @param[in] devNum                   - Device Number
* @param[in] funcPtr                   - structure that hold the
*       "os" functions needed be bound to HWS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsAldrinIfInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);


GT_VOID hwsBobcat3IfInitHwsDevFunc
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
);


GT_VOID hwsBobcat3PortsParamsSupModesMapSet
(
    GT_U8 devNum
);

/**
* @internal hwsBobcat3IfInit function
* @endinternal
*
* @brief   Init all software related DB: DevInfo, Port (Port modes, MAC, PCS and SERDES)
*         and address mapping.
*         Must be called per device.
*
* @param[in] devNum                   - Device Number
* @param[in] funcPtr                   - structure that hold the
*       "os" functions needed be bound to HWS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobcat3IfInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsRavenIfPreInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization Must be called per device..
*
* @param[in] devNum                   - system device number
* @param[in] funcPtr                  - pointer to structure that hold the "os" functions needed be bound to HWS.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsRavenIfPreInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsRavenIfGopRevPreInit function
* @endinternal
*
* @brief   .
*
* @param[in] devNum                   - system device number
* @param[in] gopRev                  - GOP revision.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsRavenIfGopRevPreInit
(
    IN GT_U8           devNum,
    IN HWS_DEV_GOP_REV gopRev
);

GT_VOID hwsRavenPortsParamsSupModesMapSet
(
    GT_U8 devNum
);

/**
* @internal hwsRavenIfInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*          Must be called per device.
*
* @param[in] devNum                   - system device number
* @param[in] funcPtr                  - pointer to structure that hold the "os" functions needed be bound to HWS.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsRavenIfInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsRavenCpllConfig function
* @endinternal
*
* @brief   Cpll config.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsRavenCpllConfig
(
    IN GT_U8 devNum
);

GT_VOID hwsAldrin2PortsParamsSupModesMapSet
(
    GT_U8 devNum
);


GT_VOID hwsAldrin2IfInitHwsDevFunc
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
);


/**
* @internal hwsAldrin2IfInit function
* @endinternal
*
* @brief   Init all software related DB: DevInfo, Port (Port modes, MAC, PCS and SERDES)
*         and address mapping.
*         Must be called per device.
*
* @param[in] devNum                   - system device number
* @param[in] funcPtr                  - pointer to structure that hold the "os" functions needed be bound to HWS.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsAldrin2IfInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

#ifdef SHARED_MEMORY

GT_STATUS hwsHawkIfReIoad
(
    GT_U8 devNum
);

#endif

GT_VOID hwsHawkPortsParamsSupModesMapSet
(
    GT_U8 devNum
);

GT_VOID hwsHawkIfInitHwsDevFunc
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
);


/**
* @internal hwsHawkIfInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @param[in] devNum                   - system device number
* @param[in] funcPtr                  - pointer to structure that hold the "os" functions needed be bound to HWS.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsHawkIfInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

#ifdef SHARED_MEMORY

GT_STATUS hwsPhoenixIfReIoad
(
    GT_U8 devNum
);

#endif

GT_VOID hwsPhoenixPortsParamsSupModesMapSet
(
    GT_U8 devNum
);

GT_STATUS hwsPhoenixSerdesGetClockAndInit
(
    GT_U8 devNum,
    GT_BOOL callbackInitOnly
);


GT_VOID hwsPhoenixIfInitHwsDevFunc
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsPhoenixIfInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @param[in] devNum                   - system device number
* @param[in] funcPtr                  - pointer to structure that hold the "os" functions needed be bound to HWS.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPhoenixIfInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

#ifdef SHARED_MEMORY

GT_STATUS hwsHarrierIfReIoad
(
    GT_U8 devNum
);

#endif
GT_VOID hwsHarrierPortsParamsSupModesMapSet
(
    GT_U8 devNum
);

GT_VOID hwsHarrierIfInitHwsDevFunc
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsHarrierIfInit function
* @endinternal
 *
*/
GT_STATUS hwsHarrierIfInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsHawkIfPreInit function
* @endinternal
 *
*/
GT_STATUS hwsHawkIfPreInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsPhoenixRegDbInit function
* @endinternal
 *
*/
GT_STATUS hwsPhoenixRegDbInit
(
    GT_U8 devNum
);


/**
* @internal hwsPhoenixIfPreInit function
* @endinternal
 *
*/
GT_STATUS hwsPhoenixIfPreInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsIronmanAsPhoenix function
* @endinternal
* function to allow ironman temporary supported in the HWS as 'phoenix' (AC5X)
*/
GT_STATUS hwsIronmanAsPhoenix(void);

/**
* @internal hwsIsIronmanAsPhoenix function
* @endinternal
* function to check if ironman temporary supported in the HWS as 'phoenix' (AC5X)
*/
GT_U32  hwsIsIronmanAsPhoenix(void);

/**
* @internal hwsHarrierRegDbInit function
* @endinternal
 *
*/
GT_STATUS hwsHarrierRegDbInit
(
    GT_U8 devNum
);


/**
* @internal hwsHarrierIfPreInit function
* @endinternal
 *
*/
GT_STATUS hwsHarrierIfPreInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsFalconIfInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @param[in] devNum                   - system device number
* @param[in] funcPtr                  - pointer to structure that hold the "os" functions needed be bound to HWS.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsFalconIfInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsFalconStep0 function
* @endinternal
*
* @brief   Falcon step0.
*
* @param[in] devNum                   - Device Number
* @param[in] numOfTiles              - number of tiles
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsFalconStep0
(
    IN GT_U8 devNum,
    IN GT_U32 numOfTiles
);


/**
* @internal hwsRavenCpllConfig function
* @endinternal
*
* @brief   Cpll config.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsRavenCpllConfig
(
    IN GT_U8 devNum
);

/**
* @internal hwsBobKSerdesIfInit function
* @endinternal
*
* @brief   Init all supported Serdes types.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobKSerdesIfInit
(
    IN GT_U8 devNum
);

/**
* @internal hwsBobKIfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsBobKIfClose
(
    IN GT_U8 devNum
);

/**
* @internal hwsAldrinIfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsAldrinIfClose
(
    IN GT_U8 devNum
);

/**
* @internal hwsBobcat3IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsBobcat3IfClose
(
    IN GT_U8 devNum
);

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
);

/**
* @internal hwsFalconIfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsFalconIfClose
(
    IN GT_U8 devNum
);

/**
* @internal hwsHawkIfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsHawkIfClose
(
    IN GT_U8 devNum
);

/**
* @internal hwsHarrierIfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsHarrierIfClose
(
    IN GT_U8 devNum
);

/**
* @internal hwsPhoenixIfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsPhoenixIfClose
(
    IN GT_U8 devNum
);

/**
* @internal hwsFalconSetRavenDevBmp function
* @endinternal
*
* @brief   for Falcon Z2 we can init only several raven and not
*          all.
*
* @param[in] ravenDevBmp       - raven device bitmap
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsFalconSetRavenDevBmp
(
    IN GT_U32 ravenDevBmp
);

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
);

#endif /* #if defined(CHX_FAMILY) || !defined(PX_FAMILY) */

#if defined(PX_FAMILY) || !defined(CHX_FAMILY)
/**
* @internal hwsPipeIfPreInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @param[in] devNum                   - system device number
* @param[in] funcPtr                  - pointer to structure that hold the "os" functions needed be bound to HWS.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPipeIfPreInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsPipeIfInit function
* @endinternal
*
* @brief   Init all software related DB: DevInfo, Port (Port modes, MAC, PCS and SERDES)
*         and address mapping.
*         Must be called per device.
*
* @param[in] devNum                   - system device number
* @param[in] funcPtr                  - pointer to structure that hold the "os" functions needed be bound to HWS.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPipeIfInit
(
    IN GT_U8 devNum,
    IN HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsPipeIfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsPipeIfClose
(
    IN GT_U8 devNum
);

#endif

/**
* @internal mvHwsRedundancyVectorGet function
* @endinternal
*
* @brief   Get SD vector.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @param[out] sdVector                - SD vector
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsRedundancyVectorGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    OUT GT_U32                  *sdVector
);

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsClockSelectorConfig function
* @endinternal
*
* @brief   Configures the DP/Core Clock Selector on port
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsClockSelectorConfig
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
);
#endif

/**
* @internal hwsDeviceSpecGetFuncPtr function
* @endinternal
*
* @brief   Get function structure pointer.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsDeviceSpecGetFuncPtr(MV_HWS_DEV_FUNC_PTRS **hwsFuncsPtr);

/**
* @internal mvHwsReNumberDevNum function
* @endinternal
*
* @brief   Replace the ID of a device from old device number
*         to a new device number
* @param[in] oldDevNum                - old device num
* @param[in] newDevNum                - new device num
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsReNumberDevNum
(
    IN GT_U8   oldDevNum,
    IN GT_U8   newDevNum
);

/**
* @internal mvHwsDeviceInit function
* @endinternal
*
* @brief   Init device,
*         according to device type call function to init all software related DB:
*         DevInfo, Port (Port modes, MAC, PCS and SERDES) and address mapping.
* @param[in] devNum                  - Device Number
* @param[in] devType                  - enum of the device type
* @param[in] funcPtr                  - pointer to structure that hold the "os"
*                                      functions needed be bound to HWS.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsDeviceInit
(
    IN GT_U8             devNum,
    IN MV_HWS_DEV_TYPE   devType,
    IN HWS_OS_FUNC_PTR   *funcPtr
);

/**
* @internal mvHwsDeviceClose function
* @endinternal
*
* @brief   Close device,
*         according to device type call function to free all resource allocated for ports initialization.
*
* @param[in] devNum                  - Device Number
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsDeviceClose
(
    IN GT_U8   devNum
);

/**
* @internal mvHwsGeneralLog function
* @endinternal
*
* @brief   Hws log message builder and logger functionn for HOST/FW
*
* @param[in] funcName                 - function name
* @param[in] fileName                 - file name
* @param[in] lineNum                  - line number
* @param[in] returnCode               - function return code
*                                      ...         - argument list
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsGeneralLog
(
    IN const char      *funcName,
#ifndef  MV_HWS_FREE_RTOS
    IN char            *fileName,
    IN GT_U32          lineNum,
    IN GT_STATUS       returnCode,
#endif
    ...
);

/**
* @internal mvHwsGeneralLogStrMsgWrapper function
* @endinternal
*
* @brief   Hws log message builder wrapper function for a log
*          message that is already evaluated by the caller.
*
* @param[in] ffuncNamePtr    - function name pointer
* @param[in] fileNamePtr    - file name pointer
* @param[in] lineNum        - line number
* @param[in] returnCode     - function return code
* @param[in] strMsgPtr      - string message to log
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
unsigned int mvHwsGeneralLogStrMsgWrapper
(
    IN const char      *funcNamePtr,
#ifndef  MV_HWS_FREE_RTOS
    IN char            *fileNamePtr,
    IN unsigned int    lineNum,
    IN int             returnCode,
#endif
    IN char            *strMsgPtr
);

#ifdef __cplusplus
}
#endif

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
);
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
    OUT GT_U32          *addressPtr
);
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
    OUT GT_U32          *addressPtr
);
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
);

/**
* @internal hwsBobcat3SerdesAddrCalc function
* @endinternal
*
* @brief   Calculate serdes register address for BC3
*
* @param[in] serdesNum                - serdes lane number
* @param[in] regAddr                  - offset of required register in SD Unit
*
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
GT_VOID hwsBobcat3SerdesAddrCalc
(
    IN GT_UOPT         serdesNum,
    IN GT_UREG_DATA    regAddr,
    OUT GT_U32          *addressPtr
);

/**
* @internal hwsPhoenixSerdesAddrCalc function
* @endinternal
*
* @brief   Calculate serdes register address for AC5x
*
* @param[in] devNum                   - device number
* @param[in] serdesNum                - serdes lane number
* @param[in] regAddr                  - offset of required register in SD Unit
* @param[in] regType                  - type of required reg
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
void hwsSip6_10SerdesAddrCalc
(
    GT_U8           devNum,
    GT_UOPT         serdesNum,
    GT_UREG_DATA    regAddr,
    GT_U32          regType,
    GT_U32          *addressPtr
);

/**
* @internal hwsFalconAsBobcat3Set function
* @endinternal
*
* @brief   indicate the HWS that the Bobcat3 device is actually Falcon + number of tiles
*          this function is called by CPSS (for Falcon) prior to bobcat3 initialization
*
* @param[in] numOfTiles               - number of tiles.
*                                       None
*/
void hwsFalconAsBobcat3Set
(
    IN GT_U32 numOfTiles
);

/* get indication if the Bobcat3 device is actually Falcon */
/* function actually return the number of Tiles (1,2,4)*/
GT_U32  hwsFalconAsBobcat3Check(void);

/* Falcon type pass CPSS to HWS */

/**
* @enum HWS_FALCON_DEV_TYPE_ENT
* @endinternal
*
* @brief   indicate the HWS what Falcon device currently initialized
*
*/
typedef enum
{
    /**@brief - not Falcon device */
    HWS_FALCON_DEV_NONE_E,

    /**@brief - Falcon 128X25G device */
    HWS_FALCON_DEV_98CX8522_E,

    /**@brief - Falcon 256X25G device */
    HWS_FALCON_DEV_98CX8542_E,

    /**@brief - Armstrong2 80X25G device - all Ravens are present but not all are used (used Ravens 0,2,5,6,7)*/
    HWS_ARMSTRONG2_DEV_98EX5610_E,

    /**@brief - Armstrong2 80X25G device - all Ravens are present but not all are used (used Ravens 0,2,5,6,7)*/
    HWS_ARMSTRONG2_DEV_98CX8512_E,

    /**@brief - Falcon 6.4T based device - all Ravens are present but not all are used (used Ravens 0,2,5,6,7)*/
    HWS_FALCON_DEV_98CX8530_E,

    /**@brief - Falcon 4T package - not all Raven are used (used Ravens 0,2,5,7)*/
    HWS_FALCON_DEV_98CX8525_E,

    /**@brief - Falcon 6.4T package - not all Raven are used (used Ravens 0,2,5,7)*/
    HWS_FALCON_DEV_98CX8520_E,

    /**@brief - Falcon 4T package - all Raven are used */
    HWS_FALCON_DEV_98CX8535_E,

    /**@brief - Falcon 4T package 80X25G device - not all Ravens are present */
    HWS_FALCON_DEV_98CX8514_E,

    /**@brief - Falcon 4T package 80X25G device - not all Ravens are present */
    HWS_FALCON_DEV_98EX5614_E,

    /**@brief - Falcon 8T package 160X25G device - not all Ravens are present */
    HWS_FALCON_DEV_98CX8550_E

} HWS_FALCON_DEV_TYPE_ENT;

/**
* @internal hwsFalconTypeSet function
* @endinternal
*
* @brief   indicate the HWS that the Falcon device is actually device
*          tof the specified type.
*
* @param[in] devNum             - device number.
* @param[in] type               - type of Falcon device.
*
*
* @return None
*/
void hwsFalconTypeSet
(
    IN GT_U8 devNum,
    IN HWS_FALCON_DEV_TYPE_ENT type
);

/**
* @internal hwsFalconTypeGet function
* @endinternal
*
* @brief   used by HWS to determinate what is a type of Falcon device.
*
* @param[in] devNum             - device number.
*
* @return Falcon device type
*/
HWS_FALCON_DEV_TYPE_ENT hwsFalconTypeGet
(
    IN GT_U8 devNum
);

/**
* @internal hwsFalconSerdesAddrCalc function
* @endinternal
*
* @brief   Calculate serdes register address for Falcon
*
* @param[in] devNum                   - Device Number
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
);


/**
* @internal hwsRavenSerdesAddrCalc function
* @endinternal
*
* @brief   Calculate serdes register address for Raven
*
* @param[in] serdesNum                - global serdes lane number
* @param[in] regAddr                  - offset of required
*       register in SD Unit (0x34000000)
*
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
GT_VOID hwsRavenSerdesAddrCalc
(
    IN GT_UOPT         serdesNum,
    IN GT_UREG_DATA    regAddr,
    OUT GT_U32          *addressPtr
);

/**
* @internal hwsFalconGopAddrCalc function
* @endinternal
*
* @brief   Calculate GOP register address for Falcon
*
* @param[in] devNum                  - Device Number
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
);

/**
* @internal hwsFalconInitSerdesMuxing function
* @endinternal
*
* @brief   init static serdes muxing
*
* @param[in] devNum                  - Device Number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsFalconInitStaticSerdesMuxing
(
    IN GT_U8 devNum
);

/**
* @internal mvHwsAvagoGetAaplChipIndex function
* @endinternal
*
* @brief   return chip index
*
* @param[in] devNum                  - Device Number
* @param[in] serdesNum               - serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
unsigned int mvHwsAvagoGetAaplChipIndex
(
    unsigned char devNum,
    unsigned int serdesNum
);

/**
* @internal HWS_UNIT_BASE_ADDR_TYPE_ENT function
* @endinternal
*
*/
typedef enum {
    HWS_UNIT_BASE_ADDR_TYPE_GOP_E,
    HWS_UNIT_BASE_ADDR_TYPE_MIB_E,
    HWS_UNIT_BASE_ADDR_TYPE_SERDES_E,
    HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
    HWS_UNIT_BASE_ADDR_TYPE_D2D_EAGLE_E,
    HWS_UNIT_BASE_ADDR_TYPE_D2D_RAVEN_E,

    HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC100_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC400_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC_STATISTICS_E,
    HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MSDB_E,
    HWS_UNIT_BASE_ADDR_TYPE_RAVEN_TSU_E,  /* AKA : PTP */
    HWS_UNIT_BASE_ADDR_TYPE_MTI_EXT_E,
    HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_MPFS_E,

    HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_MAC_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_PCS_E,
    HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_CPU_MPFS_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_EXT_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_STATISTICS_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_RSFEC_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_GSPCS_E,

    HWS_UNIT_BASE_ADDR_TYPE_RAVEN_DFX_E,

    HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS25_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS100_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS200_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS400_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_LSPCS_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_RSFEC_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_RSFEC_STATISTICS_E,

    HWS_UNIT_BASE_ADDR_TYPE_MIF_CPU_E,
    HWS_UNIT_BASE_ADDR_TYPE_MIF_400_E,
    HWS_UNIT_BASE_ADDR_TYPE_MIF_USX_E,

    HWS_UNIT_BASE_ADDR_TYPE_ANP_CPU_E,
    HWS_UNIT_BASE_ADDR_TYPE_AN_CPU_E,
    HWS_UNIT_BASE_ADDR_TYPE_ANP_400_E,
    HWS_UNIT_BASE_ADDR_TYPE_AN_400_E,
    HWS_UNIT_BASE_ADDR_TYPE_ANP_USX_E,
    HWS_UNIT_BASE_ADDR_TYPE_AN_USX_E,
    HWS_UNIT_BASE_ADDR_TYPE_ANP_USX_O_E,

    HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MAC_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_EXT_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_PCS_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_PCS_LSPCS_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_RSFEC_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MULTIPLEXER_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MAC_STATISTICS_E,
    HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_RSFEC_STATISTICS_E,

    HWS_UNIT_BASE_ADDR_TYPE__MUST_BE_LAST__E
}HWS_UNIT_BASE_ADDR_TYPE_ENT;


GT_VOID hwsFalconAddrCalc
(
    GT_U8           devNum,
    HWS_UNIT_BASE_ADDR_TYPE_ENT addressType,
    GT_UOPT         portNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
);

/*******************************************************************************
* HWS_UNIT_BASE_ADDR_CALC_BIND
*
* DESCRIPTION:
*       function type for Bind function that Calculate the base address
*                    of a global port, for next units :
*                    MIB/SERDES/GOP
*
* INPUTS:
*       unitId     - the unit : MIB/SERDES/GOP
*       portNum    - the global port num
*
* OUTPUTS:
*       None
*
* RETURNS:
*       The address of the port in the unit
*
*******************************************************************************/
typedef GT_U32 /*GT_UREG_DATA*/ (*HWS_UNIT_BASE_ADDR_CALC_BIND)
(
    GT_U8                                   devNum,
    GT_U32/*HWS_UNIT_BASE_ADDR_TYPE_ENT*/   hwsUnitId,
    GT_U32/*GT_UOPT*/                       portNum
);

/**
* @internal hwsFalconUnitBaseAddrCalcBind function
* @endinternal
*
* @brief   for FALCON : Bind the HWS with function that Calculate the base address
*         of a global port, for next units :
*         MIB/SERDES/GOP
* @param[in] cbFunc                   - the callback function
*                                       None
*/
GT_VOID hwsFalconUnitBaseAddrCalcBind
(
    HWS_UNIT_BASE_ADDR_CALC_BIND    cbFunc
);

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
    IN MV_HWS_UNIT_NUM unitNumber ,
    OUT GT_U32 *avsVol
);

/**
* @internal mvHwsAvsVoltageSet function
* @endinternal
*
* @brief   Set the Avs voltage in mv.
*
* @param[in] devNum                  - system device number
* @param[in] unitNumber              - chip unit index number.
* @param[in] vcore                   - Avs voltage requsted in
*       mv.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvsVoltageSet
(
    IN GT_U8 devNum,
    IN MV_HWS_UNIT_NUM unitNumber ,
    IN GT_U32 vcore
);

/**
* @internal mvHwsDroStatisticsGet function
* @endinternal
*
* @brief   get  DRO statistics.
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
    IN MV_HWS_UNIT_NUM unitNumber ,
    OUT GT_U32 *droAvgPtr,
    OUT GT_U32 *maxValuePtr,
    OUT GT_U32 *minValuePtr
);

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
);

/**
* @internal hwsHawkRegDbInit function
* @endinternal
 *
*/
GT_STATUS hwsHawkRegDbInit
(
    GT_U8 devNum
);

GT_STATUS hwsHawkStep0
(
    IN GT_U8 devNum
);

/**
* @internal genUnitRegisterFieldSet function
* @endinternal
*
* @brief   set field in register function.
*
* @param[in] devNum                  - system device number
* @param[in] portGroup               - port group
* @param[in] phyPortNum              - physical port number
* @param[in] unitId                  - unit to be configured
* @param[in] fieldName               - register field to be configured
* @param[in] fieldData               - register field data to be written
* @param[out] fieldRegOutPtr         - (pointer to) field structure that was configured
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genUnitRegisterFieldSet
(
    IN GT_U8           devNum,
    IN GT_U32          portGroup,
    IN GT_UOPT         phyPortNum,
    IN MV_HWS_UNITS_ID unitId,
    IN GT_U32          fieldName,
    IN GT_UREG_DATA    fieldData,
    OUT MV_HWS_REG_ADDR_FIELD_STC *fieldRegOutPtr
);

/**
* @internal genUnitFindAddressPrv function
* @endinternal
*
* @brief   get register address.
*
* @param[in] devNum                  - system device number
* @param[in] phyPortNum              - physical port number
* @param[in] unitId                  - unit to be configured
* @param[in] fieldName               - register field to be configured
* @param[out] fieldRegOutPtr         - (pointer to) field structure that was configured
*
* @retval address                        - on success
* @retval MV_HWS_SW_PTR_ENTRY_UNUSED     - on error
*/
GT_U32 genUnitFindAddressPrv
(
    IN GT_U8           devNum,
    IN GT_UOPT         phyPortNum,
    IN MV_HWS_UNITS_ID unitId,
    IN MV_HWS_PORT_STANDARD portMode,
    IN GT_U32          fieldName,
    OUT MV_HWS_REG_ADDR_FIELD_STC *fieldRegOutPtr
);

/**
* @internal genUnitPortModeRegisterFieldSet function
* @endinternal
*
* @brief   set field in register function.
*
* @param[in] devNum                  - system device number
* @param[in] portGroup               - port group
* @param[in] phyPortNum              - physical port number
* @param[in] unitId                  - unit to be configured
* @param[in] portMode                - port mode used
* @param[in] fieldName               - register field to be configured
* @param[in] fieldData               - register field data to be written
* @param[out] fieldRegOutPtr         - (pointer to) field structure that was configured
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genUnitPortModeRegisterFieldSet
(
    IN GT_U8           devNum,
    IN GT_U32          portGroup,
    IN GT_UOPT         phyPortNum,
    IN MV_HWS_UNITS_ID unitId,
    IN MV_HWS_PORT_STANDARD portMode,
    IN GT_U32          fieldName,
    IN GT_UREG_DATA    fieldData,
    OUT MV_HWS_REG_ADDR_FIELD_STC *fieldRegOutPtr
);

/**
* @internal genUnitRegisterFieldGet function
* @endinternal
*
* @brief   get field in register function.
*
* @param[in] devNum                  - system device number
* @param[in] portGroup               - port group
* @param[in] phyPortNum              - physical port number
* @param[in] unitId                  - unit to be configured
* @param[in] fieldName               - register field to be configured
* @param[out] fieldDataPtr           - (pointer to) register field data
* @param[out] fieldRegOutPtr         - (pointer to) field structure that was configured
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genUnitRegisterFieldGet
(
    IN GT_U8           devNum,
    IN GT_U32          portGroup,
    IN GT_UOPT         phyPortNum,
    IN MV_HWS_UNITS_ID unitId,
    IN GT_U32          fieldName,
    OUT GT_UREG_DATA   *fieldDataPtr,
    OUT MV_HWS_REG_ADDR_FIELD_STC *fieldRegOutPtr
);

/**
* @internal genUnitPortModeRegisterFieldGet function
* @endinternal
*
* @brief   get field in register function.
*
* @param[in] devNum                  - system device number
* @param[in] portGroup               - port group
* @param[in] phyPortNum              - physical port number
* @param[in] unitId                  - unit to be configured
* @param[in] portMode                - port mode used
* @param[in] fieldName               - register field to be configured
* @param[out] fieldDataPtr           - (pointer to) register field data
* @param[out] fieldRegOutPtr         - (pointer to) field structure that was configured
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genUnitPortModeRegisterFieldGet
(
    IN GT_U8           devNum,
    IN GT_U32          portGroup,
    IN GT_UOPT         phyPortNum,
    IN MV_HWS_UNITS_ID unitId,
    IN MV_HWS_PORT_STANDARD portMode,
    IN GT_U32          fieldName,
    OUT GT_UREG_DATA   *fieldDataPtr,
    OUT MV_HWS_REG_ADDR_FIELD_STC *fieldRegOutPtr

);

typedef struct
{
    GT_U32  ciderUnit;
    GT_U32  ciderIndexInUnit;
    GT_U32  expressChannelId;       /* default configuration of channel ID if port works:
                                        - EMAC in preemption mode
                                        - in relular mode(preemption disabled)
                                        - if preemption feature is not supported(for examlple AC5X) */
    GT_U32  preemptionChannelId;    /* configuration of second channel ID (PMAC) when port works in preemption mode */

}MV_HWS_HAWK_CONVERT_STC;

GT_STATUS mvHwsGlobalMacToLocalIndexConvert
(
    IN GT_U8                    devNum,
    IN GT_U32                   portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT MV_HWS_HAWK_CONVERT_STC *ciderIndexPtr
);

/**
* @internal mvHwsRegDbInit function
* @endinternal
*
* @brief   DB init.
*
* @param[in] devNum                  - system device number
* @param[in] unitId                  - unit to be configured
* @param[in] entryPtr                - entry
*
* @retval GT_OK                      - on success
* @retval not GT_OK                  - on error
*/
GT_STATUS mvHwsRegDbInit
(
    IN GT_U8 devNum,
    IN MV_HWS_UNITS_ID unitId,
    IN const MV_HWS_REG_ADDR_FIELD_STC *entryPtr
);

/**
* @internal genUnitRegDbEntryGet function
* @endinternal
*
* @brief   get DB entry.
*
* @param[in] devNum                  - system device number
* @param[in] phyPortNum              - physical port number
* @param[in] unitId                  - unit to be configured
* @param[in] fieldName               - register field to be configured
* @param[out] fieldRegOutPtr         - (pointer to) field structure that was configured
*
* @retval address                        - on success
* @retval MV_HWS_SW_PTR_ENTRY_UNUSED     - on error
*/
GT_STATUS genUnitRegDbEntryGet
(
    IN GT_U8           devNum,
    IN GT_UOPT         phyPortNum,
    IN MV_HWS_UNITS_ID unitId,
    IN MV_HWS_PORT_STANDARD portMode,
    IN GT_U32          fieldName,
    OUT MV_HWS_REG_ADDR_FIELD_STC *fieldRegOutPtr,
    OUT MV_HWS_HAWK_CONVERT_STC *convertIdxPtr
);

/**
* @internal mvHwsHWAccessLock function
* @endinternal
*
* @brief   Protection Definition
*         =====================
*         some registers cannot be accessed by more than one
*         client concurrently Concurrent access might result in
*         invalid data read/write.
*         Multi-Processor Environment This case is protected by
*         HW Semaphore HW Semaphore is defined based in MSYS /
*         CM3 resources In case customer does not use MSYS / CM3
*         resources, the customer will need to implement its own
*         HW Semaphore This protection is relevant ONLY in case
*         Service CPU Firmware is loaded to CM3
*
* @param[in] devNum                   - system device number
*
*          */
void mvHwsHWAccessLock
(
    IN GT_U8 devNum,
#if !defined MV_HWS_REDUCED_BUILD_EXT_CM3 || defined (FALCON_DEV_SUPPORT)
    IN GT_U8 chipIndex,
#endif
    IN GT_U8 hwSem
);

/**
* @internal mvHwsHWAccessUnlock function
* @endinternal
*
* @brief   See description in mvHwsAvagoAccessLock API
* @param[in] devNum                   - system device number
*/
void mvHwsHWAccessUnlock
(
    IN GT_U8 devNum,
#if !defined MV_HWS_REDUCED_BUILD_EXT_CM3 || defined (FALCON_DEV_SUPPORT)
    IN GT_U8 chipIndex,
#endif
    IN GT_U8 hwSem
);

#ifdef SHARED_MEMORY

GT_STATUS hwsFalconIfReIoad
(
    GT_U8 devNum
);

#endif
GT_VOID hwsFalconPortsParamsSupModesMapSet
(
    GT_U8 devNum
);

GT_VOID hwsFalconIfInitHwsDevFunc
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
);


const MV_HWS_PORT_INIT_PARAMS   * hwsDevicePortsElementsSupModesCatalogGet
(
    IN GT_U8       devNum,
    GT_U32         portNum
);


#endif /* __siliconIf_H */



