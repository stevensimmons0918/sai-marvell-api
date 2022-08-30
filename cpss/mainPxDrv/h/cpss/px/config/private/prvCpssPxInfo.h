/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file prvCpssPxInfo.h
*
* @brief Includes structures and private functions definition for the use of Px Prestera SW.
*
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssPxInfo_h
#define __prvCpssPxInfo_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* BOOKMARK for places that call cpssDriver/common and need cast to GT_U8 on 'sw devNum' */
/*#define CAST_SW_DEVNUM(devNum)    ((GT_U8)devNum)*/

/* get private types */
#include <cpss/common/config/private/prvCpssConfigTypes.h>
/* get registers structure */
#include <cpss/px/cpssHwInit/private/prvCpssPxRegsVer1.h>
/* get generic registers access APIs */
#include <cpss/common/cpssHwInit/private/prvCpssHwRegisters.h>
/* get Errata manager */
#include <cpss/px/config/private/prvCpssPxErrataMng.h>
/* get tables manager */
#include <cpss/px/cpssHwInit/private/prvCpssPxHwTables.h>
/* get HW specific info (and 'fine tuning') */
#include <cpss/px/config/private/prvCpssPxInfoEnhanced.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/private/prvCpssPxPort.h>
#include <cpss/px/diag/cpssPxDiag.h>

/* get ingress processing info */
#include <cpss/px/ingress/cpssPxIngress.h>
/* get egress processing info */
#include <cpss/px/egress/cpssPxEgress.h>

#include <cpss/px/cpssHwInit/cpssPxHwInit.h>
#include <cpss/px/port/cpssPxPortMapping.h>
#include <cpss/px/port/cpssPxPortCtrl.h>

/* get CPSS definitions for private port manager configurations */
#include <cpss/common/port/private/prvCpssPortManagerTypes.h>

/* macro to get a pointer on the Px device
    devNum - the device id of the Px device

    NOTE : the macro do NO validly checks !!!!
           (caller responsible for checking with other means/macro)
*/
#ifndef CPSS_USE_MUTEX_PROFILER
#define PRV_CPSS_PX_PP_MAC(devNum) \
    ((PRV_CPSS_PX_PP_CONFIG_STC*)PRV_CPSS_PP_CONFIG_ARR_MAC[devNum])
#else
#ifdef  WIN32
#define PRV_CPSS_PX_PP_MAC(devNum)    ((PRV_CPSS_PX_PP_CONFIG_STC*)prvCpssGetPrvCpssPpConfig(devNum,__FUNCTION__))
#else
#define PRV_CPSS_PX_PP_MAC(devNum)   ((PRV_CPSS_PX_PP_CONFIG_STC*)prvCpssGetPrvCpssPpConfig(devNum,__func__))
#endif
#endif /*CPSS_USE_MUTEX_PROFILER  */

/* check that the devFamily is one of PX */
#define PRV_CPSS_PX_FAMILY_CHECK_MAC(devNum)                 \
   (PRV_CPSS_PP_MAC(devNum)->functionsSupportedBmp &         \
    PRV_CPSS_PX_FUNCTIONS_SUPPORT_CNS)


/* check that the device is on of EXISTING PX
   return GT_BAD_PARAM or GT_NOT_APPLICABLE_DEVICE on error
*/
#define PRV_CPSS_PX_DEV_CHECK_MAC(devNum)                       \
    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))              {  \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "devNum[%d] not exists",(devNum)); \
    }                                                           \
    if(0 == PRV_CPSS_PX_FAMILY_CHECK_MAC(devNum))          {    \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "devNum[%d] is not PX device",(devNum)); \
    }

/* check that the PHYSICAL port number hold valid number */
#define PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum) \
    CPSS_PARAM_CHECK_MAX_MAC(portNum,PRV_CPSS_PX_PORTS_NUM_CNS)

/* check that the port bitmap is valid */
#define PRV_CPSS_PX_PORT_BMP_CHECK_MAC(devNum, portsBmp) \
    if (portsBmp >= BIT_17)               \
    {                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Port bitmap not valid [0..%x]", \
            (BIT_17-1));                 \
    }

/* get the port type */
#define PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portNum) \
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType

/* get the port interface mode */
#define PRV_CPSS_PX_PORT_IFMODE_MAC(devNum,portNum) \
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portIfMode

/* get the port speed */
#define PRV_CPSS_PX_PORT_SPEED_MAC(devNum,portNum) \
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portSpeed

/* get port's options i.e. which ifModes it supports */
#define PRV_CPSS_PX_PORT_TYPE_OPTIONS_MAC(devNum,portNum) \
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portTypeOptions

/* get isFlexLink */
#define PRV_CPSS_PX_IS_FLEX_LINK_MAC(devNum,portNum) \
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].isFlexLink

/* port serdes optimization algorithms bitmap */
#define PRV_CPSS_PX_PORT_SD_OPT_ALG_BMP_MAC(devNum,portNum) \
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].serdesOptAlgBmp

/* access to the registers addresses of the device */
#define PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)  \
    (&(PRV_CPSS_PX_PP_MAC(devNum)->regsAddrVer1))

/* macro to check if sdma interface used for cpu traffic by current device */
#define  PRV_CPSS_PX_SDMA_USED_CHECK_MAC(devNum)    \
    if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode != CPSS_NET_CPU_PORT_MODE_SDMA_E)   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "devNum[%d] : The <cpuPortMode> is not 'SDMA'",(devNum))

/* check that the networkif of the device was initialized
    return GT_NOT_INITIALIZED if not initialized
*/
#define PRV_CPSS_PX_NETIF_INITIALIZED_CHECK_MAC(devNum)                        \
    if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_NONE_E)  \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "devNum[%d] : The networkif LIB was not initialized",(devNum))

/* Pipe A1 revision check */
#define PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(_devNum)                     \
    ((PRV_CPSS_PP_MAC(_devNum)->revision > 0) ? 1 : 0)

/* check that the device is A1 revision
   return GT_BAD_PARAM or GT_NOT_APPLICABLE_DEVICE on error
*/
#define PRV_CPSS_PX_A1_DEV_CHECK_MAC(devNum)                    \
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);                          \
    if (0 == PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum)) {      \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "devNum[%d] is not PX A1 device",(devNum)); \
    }


/* The size of XLG ports MAC MIB counters entry in words */
/* the device hold 'all 30 counters are 64 bits' */
/* The size of XLG ports MAC MIB counters entry in words */
#define PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS (30*2)

/**
* @struct PRV_CPSS_PX_PORT_XLG_MIB_SHADOW_STC
 *
 * @brief A structure to hold for XLG MIB counters
*/
typedef struct{

    GT_U32 mibShadow[PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];

    GT_U32 captureMibShadow[PRV_CPSS_PX_XLG_MIB_COUNTERS_ENTRY_SIZE_CNS];

    /** clear on read enable */
    GT_BOOL clearOnReadEnable;

} PRV_CPSS_PX_PORT_XLG_MIB_SHADOW_STC;

/* max amount of data paths */
#define PRV_CPSS_PX_MAX_DP_CNS 2

/**
* @struct PRV_CPSS_PX_RESOURCES_STATUS_STC
 *
 * @brief A structure to hold info about the resources : credits/headers/payloads/BW
*/
typedef struct{

    GT_U32 usedDescCredits[PRV_CPSS_PX_MAX_DP_CNS];

    GT_U32 maxDescCredits[PRV_CPSS_PX_MAX_DP_CNS];

    GT_U32 usedPayloadCredits[PRV_CPSS_PX_MAX_DP_CNS];

    GT_U32 maxPayloadCredits[PRV_CPSS_PX_MAX_DP_CNS];

    GT_U32 usedHeaderCredits[PRV_CPSS_PX_MAX_DP_CNS];

    GT_U32 maxHeaderCredits[PRV_CPSS_PX_MAX_DP_CNS];

    GT_U32 coreOverallSpeedSummary[PRV_CPSS_PX_MAX_DP_CNS];

} PRV_CPSS_PX_RESOURCES_STATUS_STC;

/*------------------------------------------------*
 * global ==> [dp , local] DMA  conversion table  *
 *------------------------------------------------*/
typedef struct
{
    GT_U32 dataPathIdx;
    GT_U32 localDmaNum;
}PRV_CPSS_PX_LOCAL_DMA_STC;

typedef struct
{
    GT_U32                    globalDmaNum;
    PRV_CPSS_PX_LOCAL_DMA_STC localDma;
}PRV_CPSS_PX_GLOBAL2LOCAL_DMA_CONV_STC;

/**
* @struct PRV_CPSS_PX_PORT_INFO_STC
 *
 * @brief A structure to hold all PX data needed on port
*/
typedef struct{

    PRV_CPSS_PX_PORT_XLG_MIB_SHADOW_STC *portsMibShadowArr      [PRV_CPSS_PX_GOP_PORTS_NUM_CNS];

    CPSS_PX_DETAILED_PORT_MAP_STC portsMapInfoShadowArr   [PRV_CPSS_PX_PORTS_NUM_CNS];

    GT_PHYSICAL_PORT_NUM portsMac2PhyscalConvArr [PRV_CPSS_PX_GOP_PORTS_NUM_CNS];

    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT egressSourcePortEntryTypeArr [PRV_CPSS_PX_PORTS_NUM_CNS];

    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT egressTargetPortEntryTypeArr [PRV_CPSS_PX_PORTS_NUM_CNS];

    /** list of ports that were force down, use to fix DISMATCH_PORTS_LINK_WA_E */
    CPSS_PORTS_BMP_STC portForceLinkDownBmp;

    /** SERDES reference clock type. */
    CPSS_PX_PP_SERDES_REF_CLOCK_ENT serdesRefClock;

    CPSS_PX_DIAG_TRANSMIT_MODE_ENT prbsMode                [PRV_CPSS_MAX_PP_PORTS_NUM_CNS];

    GT_BOOL prbsSerdesCountersClearOnReadEnable[PRV_CPSS_MAX_MAC_PORTS_NUM_CNS][PRV_CPSS_MAX_PORT_LANES_CNS];

    /** data path resources status. */
    PRV_CPSS_PX_RESOURCES_STATUS_STC resourcesStatus;

    PRV_CPSS_PORT_MNG_DB_STC portManagerDb;

} PRV_CPSS_PX_PORT_INFO_STC;

/**
* @struct PRV_CPSS_PX_HW_TABLES_SHADOW_STC
 *
 * @brief This struct defines HW table shadow DB info (per HW table)
*/
typedef struct{

    GT_U32 *hwTableMemPtr;

    /** type of the shadow */
    CPSS_PX_SHADOW_TYPE_ENT shadowType;

    /** @brief size of the memory in hwTableMemPtr (in bytes)
     *  NOTE: relevant only when shadow type is CPSS_PX_SHADOW_TYPE_CPSS_E
     */
    GT_U32 hwTableMemSize;

} PRV_CPSS_PX_HW_TABLES_SHADOW_STC;

/**
* @struct PRV_CPSS_PX_NET_INFO_STC
 *
 * @brief A structure to hold all PP data needed on networkIf PX layer
*/
typedef struct{

    /** @brief RX buffers allocation method used.
     *  CPSS_RX_BUFF_DYNAMIC_ALLOC_E and CPSS_RX_BUFF_STATIC_ALLOC_E -
     *  CPSS responsible for RX buffers allocation and managment.
     *  CPSS_RX_BUFF_NO_ALLOC_E - application responsible for RX
     *  buffer allocation.
     */
    CPSS_RX_BUFF_ALLOC_METHOD_ENT allocMethod;

    /** @brief GT_TRUE if buffers allocated in cached CPU memory,
     *  GT_FALSE otherwise. Used for DX only - ignored for all other.
     *  When using cached RX buffers on Linux/FreeBSD please take
     *  in mind high impact on system performance due to IOCTL from
     *  user to kernel space during the invalidate action.
     */
    GT_BOOL buffersInCachedMem;

} PRV_CPSS_PX_NET_INFO_STC;

/**
* @struct PRV_CPSS_PX_PORT_SERDES_POLARITY_CONFIG_STC
 *
 * @brief Port SERDES TX/RX Polarity configuration parameters.
*/
typedef struct{

    /** Tx Polarity */
    GT_BOOL txPolarity;

    /** Rx Polarity */
    GT_BOOL rxPolarity;

} PRV_CPSS_PX_PORT_SERDES_POLARITY_CONFIG_STC;



/*----------------------------------------------------*/
/*   Port Data Base                                   */
/*----------------------------------------------------*/
#define PRV_CPSS_PX_DYNAMIC_PA_SLICE_NUM_CNS  340
/**
* @struct PRV_CPSS_PX_PORT_PA_SINGLE_PORT_DB_STC
 *
 * @brief A structure to single port speed used in PA
*/
typedef struct{

    /** port number */
    GT_PHYSICAL_PORT_NUM physicalPortNum;

    /** GT_TRUE if port gets the speed */
    GT_BOOL isInitilized;

    /** speed of port in Mbps */
    GT_U32 portSpeedInMBit;

} PRV_CPSS_PX_PORT_PA_SINGLE_PORT_DB_STC;

/**
* @struct PRV_CPSS_PX_PORT_PA_PORT_DB_STC
 *
 * @brief A structure to single port speed used in PA
*/
typedef struct{

    /** number of initialized ports */
    GT_U32 numInitPorts;

    PRV_CPSS_PX_PORT_PA_SINGLE_PORT_DB_STC prv_portDB[PRV_CPSS_PX_PORTS_NUM_CNS];

} PRV_CPSS_PX_PORT_PA_PORT_DB_STC;

struct PRV_CPSS_PX_PA_WORKSPACE_STCT;
struct PRV_CPSS_PX_PA_UNIT_STCT;

/**
* @struct PRV_CPSS_PX_PIZZA_ARBITER_STC
 *
 * @brief PA related structures
*/
typedef struct
{
    PRV_CPSS_PX_PORT_PA_PORT_DB_STC               paPortDB;
    GT_U32                                        paUnitPipeBWInGBitArr[CPSS_PX_PA_UNIT_MAX_E];
    GT_U32                                        paMinSliceResolutionMbps;
    struct PRV_CPSS_PX_PA_WORKSPACE_STCT         *paWsPtr;
    struct PRV_CPSS_PX_PA_UNIT_STCT              *paUnitDrvList;
}PRV_CPSS_PX_PIZZA_ARBITER_STC;

/**
* @struct PRV_CPSS_PX_CNC_INFO_STC
 *
 * @brief CNC related structures
*/
typedef struct{

    /** pointer to DMA queue host memory */
    GT_UINTPTR block;

    /** size of DMA queue. The size is in DMA messages count, not in bytes */
    GT_U32 blockSize;

    /** current DMA message index */
    GT_U32 curIdx;

    /** number of unread DMA message */
    GT_U32 unreadCount;

} PRV_CPSS_PX_CNC_INFO_STC;


/**
* @struct PRV_CPSS_PX_PP_CONFIG_STC
 *
 * @brief A structure to hold all PX data needed
*/
typedef struct
{
    PRV_CPSS_GEN_PP_CONFIG_STC                      genInfo;

    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC               regsAddrVer1;

    PRV_CPSS_PX_ERRATA_STC                          errata;

    PRV_CPSS_PX_PP_CONFIG_FINE_TUNING_STC           fineTuning;

    PRV_CPSS_PX_TABLES_INFO_STC                     *accessTableInfoPtr;
    GT_U32                                          accessTableInfoSize;

    PRV_CPSS_PX_NET_INFO_STC                        netIf;
    PRV_CPSS_PX_PORT_INFO_STC                       port;

    PRV_CPSS_PX_PP_HW_INFO_STC                      hwInfo;

    CPSS_PORT_SERDES_TUNE_STC_PTR                *serdesCfgDbArrPtr;

    PRV_CPSS_PX_PORT_SERDES_POLARITY_CONFIG_STC        *serdesPolarityPtr;

    PRV_CPSS_PX_PP_TABLE_FORMAT_INFO_STC            tableFormatInfo[PRV_CPSS_PX_TABLE_LAST_FORMAT_E];

    PRV_CPSS_PX_HW_TABLES_SHADOW_STC                shadowInfoArr[CPSS_PX_TABLE_LAST_E];

    PRV_CPSS_PX_PIZZA_ARBITER_STC                   paData;

    PRV_CPSS_PX_CNC_INFO_STC                        cncDmaDesc;

} PRV_CPSS_PX_PP_CONFIG_STC;

#if defined  CPSS_USE_MUTEX_PROFILER
extern void * prvCpssGetPrvCpssPpConfig(    IN GT_U32      devNum,IN const char *               functionName);
#endif

/* Write PTP/Packet type register data and mask registers */
GT_STATUS prvCpssPxIngressPacketTypeRegisterWrite
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           regDataAddr,
    IN GT_U32           regMaskAddr,
    IN GT_U32           regDataValue,
    IN GT_U32           regMaskValue,
    IN GT_U32           regWriteBits
);

/* Read PTP/Packet type register data and mask registers */
GT_STATUS prvCpssPxIngressPacketTypeRegisterRead
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           regDataAddr,
    IN GT_U32           regMaskAddr,
    IN GT_U32           *regDataValuePtr,
    IN GT_U32           *regMaskValuePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxInfo_h */



