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
* mvAvagoSerdesIf.c.c
*
* DESCRIPTION:
*         Avago interface
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
#include "sdAvagoFirmware/serdes.0x10B9_208D_009.h"
#include "sdAvagoFirmware/sbus_master.0x0226_2001.h"
#include "sdAvagoFirmware/serdes.0x2464_0245.h"
#include "sdAvagoFirmware/sbus_master.0x101A_0001.h"
#include "sdAvagoFirmware/serdes.0x2464_0245swap.h"
#include <stdio.h>
#include <stdlib.h>
#endif

#include <cpss/common/labServices/port/gop/port/serdes/avago/mv_hws_avago_if.h>
/* Avago include */
#include "include/aapl.h"
#include "include/aapl_core.h"
#include "include/sbus.h"
#include "include/spico.h"
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include "eye.h"
#endif


#if !defined(ASIC_SIMULATION) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#   if AAPL_ENABLE_AACS_SERVER
#   include <aacs_server.h>
#ifdef CHX_FAMILY
    extern GT_U32 hwsFalconNumOfRavens;
    extern GT_U32 hwsRavenDevBmp;
#endif
#   endif /* AAPL_ENABLE_AACS_SERVER */
#endif /* !defined(ASIC_SIMULATION) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) */
/************************* Globals *******************************************************/
unsigned int avagoConnection = AVAGO_SBUS_CONNECTION;

/* AAPL struct array for each device */
Aapl_t* aaplSerdesDb[HWS_MAX_DEVICE_NUM * RAVEN_MAX_AVAGO_CHIP_NUMBER] = {NULL};
/* AAPL struct array */
#ifdef MV_HWS_REDUCED_BUILD_EXT_CM3
#if defined(FALCON_DEV_SUPPORT)
Aapl_t  aaplSerdesDbDef[RAVEN_MAX_AVAGO_CHIP_NUMBER];
#else
Aapl_t  aaplSerdesDbDef[HWS_MAX_DEVICE_NUM];
#endif
#endif

EXT void  ms_sleep(uint msec);
extern int recovery_process;

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(MICRO_INIT) && !defined ALDRIN_DEV_SUPPORT)
#define MAX_AVG_SIZE 500
MV_HWS_AVG_BLOCK avgBlockData; /* a structure that holds nessaccary parameters for decoding */
GT_UL64 mvHwsAvagoSerdesCalcVosDfxParams( unsigned char devNum, GT_U32 serdesNum, GT_U8 bitsPerSds, GT_U8 dataArr[] );
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

GT_STATUS mvHwsAvagoSerdesVosDfxParamsSet(GT_U32 devNum, GT_U32 serdesNum);
#if AAPL_ENABLE_AACS_SERVER
static unsigned int aacsServerEnable = 0;

/* Avago server Task Id */
static GT_U32 avagoAacsServerTid;
#endif /* AAPL_ENABLE_AACS_SERVER */
#define MV_HWS_AVAGO_STABLE_PLL_TIMEOUT 100

#if defined(CHX_FAMILY) || defined(PX_FAMILY)
HWS_MUTEX avagoAccessMutex = 0;
#endif /* defined(CHX_FAMILY) || defined(PX_FAMILY) */

#if defined MV_HWS_FREE_RTOS
extern  uint  (*aapl_user_supplied_log_func_ptr)  (const char  *funcNamePtr, char  *strMsgPtr);
#else /* !MV_HWS_FREE_RTOS */
extern  uint  (*aapl_user_supplied_log_func_ptr)  (const char  *funcNamePtr, char  *fileNamePtr, unsigned int  lineNum, int  returnCode, char  *strMsgPtr);
#endif



#ifndef MV_HWS_AVAGO_NO_VOS_WA
#define SERVER_HD_EFUSE_VOS_REGION_BOBK   0x200
#define SERVER_HD_EFUSE_VOS_REGION_ALDRIN 0x100

typedef struct{
    GT_U32  serdes;
    GT_U32  serdesVosOffset;

}HWS_AVAGO_SERDES_VOS_OFFSET;

/* Code for Non CM3 CPU (host) and for Bobk CM3  */
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && defined(BOBK_DEV_SUPPORT))
/* Serdeses 24->35 := total 12 */
HWS_AVAGO_SERDES_VOS_OFFSET serdesVosOffsetBobK[]={
    /* Serdes  Vos-offset*/
     {   24,      0xc0    }
    ,{   25,      0xb0    }
    ,{   26,      0xa0    }
    ,{   27,      0x90    }
    ,{   28,      0x40    }
    ,{   29,      0x50    }
    ,{   30,      0x60    }
    ,{   31,      0x70    }
    ,{   32,      0x30    }
    ,{   33,      0x20    }
    ,{   34,      0x10    }
    ,{   35,      0x00    }
};
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && defined(BOBK_DEV_SUPPORT)) */

/* Code for Non CM3 CPU (host) and for Aldrin CM3  */
/* Serdeses 0->31 := total 32 */
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && defined(ALDRIN_DEV_SUPPORT))
HWS_AVAGO_SERDES_VOS_OFFSET serdesVosOffsetAldrin[]={
    /* Serdes  Vos-offset*/
     {   0,      0x100    }
    ,{   1,      0x0f0    }
    ,{   2,      0x110    }
    ,{   3,      0x0e0    }
    ,{   4,      0x120    }
    ,{   5,      0x0d0    }
    ,{   6,      0x130    }
    ,{   7,      0x0c0    }
    ,{   8,      0x0a0    }
    ,{   9,      0x150    }
    ,{   10,     0x0b0    }
    ,{   11,     0x140    }
    ,{   12,     0x160    }
    ,{   13,     0x090    }
    ,{   14,     0x170    }
    ,{   15,     0x080    }
    ,{   16,     0x180    }
    ,{   17,     0x070    }
    ,{   18,     0x190    }
    ,{   19,     0x060    }
    ,{   20,     0x1a0    }
    ,{   21,     0x050    }
    ,{   22,     0x1b0    }
    ,{   23,     0x040    }
    ,{   24,     0x1c0    }
    ,{   25,     0x030    }
    ,{   26,     0x1d0    }
    ,{   27,     0x020    }
    ,{   28,     0x000    }
    ,{   29,     0x1f0    }
    ,{   30,     0x010    }
    ,{   31,     0x1e0    }
};
#endif
/* VOS Override values DB. To store the values read from DFX. The values will be stored as following:

       serdesVosParamsDB[0] = vos0-3 base serdes
       serdesVosParamsDB[0] = vos4-6 base serdes
       serdesVosParamsDB[1] = vos0-3 base serdes + 1
       serdesVosParamsDB[1] = vos4-6 base serdes + 1
       .
       .
       .
       serdesVosParamsDB[n] = vos0-3 base serdes + n
       serdesVosParamsDB[n] = vos4-6 base serdes + n

   This DB is sent to service CPU upon init, so its size should be a multiplition of VOS IPC buffer size.
*/
GT_U32  serdesVosParamsDB[72];
GT_BOOL serdesVosParamsDBInitialized = GT_FALSE;
#ifndef ALDRIN_DEV_SUPPORT
#define VOS_WORDS 72
GT_UL64 serdesVosParams64BitDB[VOS_WORDS];/* array of longs to hold the data of 7 samplers per serdes */
GT_BOOL serdesVos64BitDbInitialized = GT_FALSE;
GT_U32 minVosValue; /* minimum VOS value */
GT_U32 baseVosValue; /* base VOS value */
#endif
#endif /* MV_HWS_AVAGO_NO_VOS_WA */

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3

extern GT_BOOL  prvCpssSystemRecoveryInProgress
(
    GT_VOID
);
#endif

/*
   Tx offset parameters implementation optional compilation flags
*/
/*#define MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_FW_DEBUG*/
/*#define MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG*/
/*#define MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_IMPLEMENTATION_A*/

#if (defined(MV_HWS_FREE_RTOS) || defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG))

#ifdef MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_IMPLEMENTATION_A

/*
    Implementation A:

        Maximum 1 byte memory waste. This implementation wastes less memory then the next implementation
        but consume more in text (code). Currently, this implementation is more memory-consume then next
        implementation, but it can be changed as serdeses numbers and supported speeds demands will change.

        (Note: array second dimention size of
                (MV_HWS_IPC_MAX_PORT_NUM) + (MV_HWS_IPC_MAX_PORT_NUM/2) + (1)
              implements the calculation-
                ((MV_HWS_IPC_MAX_PORT_NUM * 1.5) +1 )
              The plus 1 is because of the integer down-grade [floor] devision)

       Data is organaized as follows (each letter represent serdes):

       sd0[Tx0] sd0[Tx1]  sd0[Tx2] sd1[Tx0]   sd1[Tx1] sd1[Tx2]    sd2[Tx0] sd2[Tx1]    sd2[Tx2]
         ____________       ____________         ____________        ____________        ____________
        |     |      |     |      |     |       |     |      |      |      |     |      |      |     |
        |_X___|__X___|     |__ X__|__Y__|       |__Y__|___Y__|      |___Z__|__Z__|      |__Z___|_____| ...

      This implementation does not support preset/init feature (chose using preset values instead of init)
*/
GT_U8 mvHwsAvagoSerdesManualTxOffsetDb[3] [(MV_HWS_IPC_MAX_PORT_NUM) + (MV_HWS_IPC_MAX_PORT_NUM/2) + (1)];

#define TX_AMP_RAW_VAL_SHIFT_OP(_sd) (4*(_sd%2))
#define TX_AMP_RAW_VAL_IDX(_sd) ((_sd/2)*3 + (_sd%2))
#define TX_AMP_RAW_VAL_PRV(_sd,_speedIdx) \
    ((GT_8)(((mvHwsAvagoSerdesManualTxOffsetDb[ _speedIdx ][TX_AMP_RAW_VAL_IDX(_sd)]) >> (4*(_sd%2))) & 0xF))
#define TX_AMP_RAW_VAL(_sd,_speedIdx) \
    (((TX_AMP_RAW_VAL_PRV(_sd,_speedIdx) & 0x8)) ?                              \
    ( (GT_8)255 - (GT_8)(((TX_AMP_RAW_VAL_PRV(_sd,_speedIdx) & 0x7) ) - 1) ) :  \
        (TX_AMP_RAW_VAL_PRV(_sd,_speedIdx)))


#define TX_EMPH0_RAW_VAL_SHIFT_OP(_sd) (4*(1-(_sd%2)))
#define TX_EMPH0_RAW_VAL_IDX(_sd) ((_sd/2)*3 + (2*(_sd%2)))
#define TX_EMPH0_RAW_VAL_PRV(_sd,_speedIdx) \
    ((GT_8)((mvHwsAvagoSerdesManualTxOffsetDb[ _speedIdx ][TX_EMPH0_RAW_VAL_IDX(_sd)] >> (4*(1-(_sd%2)))) & 0xF))
#define TX_EMPH0_RAW_VAL(_sd,_speedIdx) \
    (((TX_EMPH0_RAW_VAL_PRV(_sd,_speedIdx) & 0x8))?                                 \
    ( (GT_8)255 - (GT_8)(((TX_EMPH0_RAW_VAL_PRV(_sd,_speedIdx) & 0x7) ) - 1) ) : \
    (TX_EMPH0_RAW_VAL_PRV(_sd,_speedIdx)))


#define TX_EMPH1_RAW_VAL_SHIFT_OP(_sd) (4*(_sd%2))
#define TX_EMPH1_RAW_VAL_IDX(_sd) ((_sd/2)*3 + (1+(_sd%2)))
#define TX_EMPH1_RAW_VAL_PRV(_sd,_speedIdx) \
    ((GT_8)((mvHwsAvagoSerdesManualTxOffsetDb[ _speedIdx ][TX_EMPH1_RAW_VAL_IDX(_sd)] >> (4*(_sd%2)))) & 0xF)
#define TX_EMPH1_RAW_VAL(_sd,_speedIdx) \
    (((TX_EMPH1_RAW_VAL_PRV(_sd,_speedIdx) & 0x8))?                                     \
    ( (GT_8)255 - (GT_8)(((TX_EMPH1_RAW_VAL_PRV(_sd,_speedIdx) & 0x7) ) - 1) ) :    \
    (TX_EMPH1_RAW_VAL_PRV(_sd,_speedIdx)))

#else

/*
   Implementation B:

       this array wastes 4bits per port AND per speed. It could be implemented without any waste (see
       implementation A) but then the code to manage it would consume more space and would be hard to
       maintain. (above example)

       Data is organaized as follows:

       sd0[Tx0] sd0[Tx1] sd0[Tx2]       sd1[Tx0] sd1[Tx1] sd1[Tx2]
         ________________________         ________________________
        |     |      |     |     |       |     |      |      |     |
        |_X___|__X___|_ X__|_____|       |__Y__|___Y__|___Y__|_____|    ...

       This implementation supports preset/init feature (chose using preset values instead of init) - bit 15 indicade if to request preset or init from peer side

*/
GT_U16 mvHwsAvagoSerdesManualTxOffsetDb[3][MV_HWS_IPC_MAX_PORT_NUM] = {{0}};
#define TX_PARAMETER_STEP   2
#define TX_AMP_RAW_VAL(_sd, _speedIdx)                                                                      \
    (((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd] & 0x0010)) ?                                      \
            (GT_8) (255 - ((GT_8)((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd]) & 0x000F)*TX_PARAMETER_STEP - 1 )) :               \
            (GT_8) (mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd] & 0x000F)*TX_PARAMETER_STEP)

#define TX_EMPH0_RAW_VAL(_sd, _speedIdx)                                                                    \
    ((((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd] & 0x0200))) ?                                 \
            (GT_8) (255 - ((GT_8)(((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd]) & 0x01E0) >> 5)*TX_PARAMETER_STEP - 1))  :       \
            (GT_8) (((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd]) & 0x01E0) >> 5)*TX_PARAMETER_STEP )

#define TX_EMPH1_RAW_VAL(_sd, _speedIdx)                                                                    \
    ((((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd] & 0x4000))) ?                                 \
            (GT_8) (255 - ((GT_8)(((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd]) & 0x3C00) >> 10)*TX_PARAMETER_STEP - 1) )   :     \
            (GT_8) (((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd]) & 0x3C00) >> 10)*TX_PARAMETER_STEP )

#if 0
#define TX_PRESET_RAW_VAL(_sd, _speedIdx) \
    (((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd] & 0x8000))? 1 : 0 )
#endif

#define RX_INIT_RAW_VAL(_sd, _speedIdx) \
    (((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd] & 0x8000))? 1 : 0 )

#endif /* MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_IMPLEMENTATION_A */
#endif /* (defined(MV_HWS_FREE_RTOS) || defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG)) */

#define MV_HWS_LANE_SERDES_SPEED_NUM 3

#ifndef BOBK_DEV_SUPPORT
MV_HWS_AVAGO_TXRX_OVERRIDE_CONFIG_DATA mvHwsAvagoSerdesManualConfigDb[MV_HWS_LANE_SERDES_SPEED_NUM][MV_HWS_IPC_MAX_PORT_NUM];

#endif
#ifdef RAVEN_DEV_SUPPORT
MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA hwsAvagoSerdesPresetParamsManualConfigDb[MV_HWS_LANE_SERDES_SPEED_NUM][MV_HWS_IPC_MAX_PORT_NUM]= {{{0xFF}}};
#endif

#define APPLY_IF_VALID(val,newVal,invalidDef) \
        if ((newVal) != (invalidDef))         \
        {                                     \
            val = newVal;                     \
        }

/*************************** Pre-Declarations *******************************************************/
#ifdef MV_HWS_FREE_RTOS
extern GT_STATUS mvPortCtrlSerdesLock(void);
extern GT_STATUS mvPortCtrlSerdesUnlock(void);
#endif /* MV_HWS_FREE_RTOS */

#ifndef ASIC_SIMULATION
extern GT_STATUS mvHwsAvagoInitI2cDriver(unsigned char devNum);
#endif /* ASIC_SIMULATION */

void mvHwsAvagoAccessValidate(unsigned int chipIndex, uint sbus_addr);
GT_STATUS mvHwsAvagoCheckSerdesAccess(unsigned int devNum, unsigned char portGroup, unsigned char serdesNum);

#ifdef MV_HWS_FREE_RTOS
int *serdesFwPtr = NULL; /*init with pointer to file in SPI*/
int serdesFwSize = 0;
/*replace with file size from file system*/
int *sbusMasterFwPtr = NULL; /*init with pointer to file in SPI*/
int sbusMasterFwSize = 0;
int *serdesFwDataSwapPtr = NULL;
int swapFwSize = 0;
#endif /* MV_HWS_FREE_RTOS */

extern MV_HWS_AVAGO_TXRX_TUNE_PARAMS       hwsAvagoSerdesTxRxTuneParamsArray[];
extern MV_HWS_AVAGO_16NM_TXRX_TUNE_PARAMS  *hwsAvagoSerdesTxRx16nmTuneParamsArrayPtrs[HWS_AVAGO_16NM_TXRX_PARAMS_NUM_OF_PROFILES];
extern GT_U8                                hwsAvagoSerdesTxRx16nmTuneParamsArraySizes[HWS_AVAGO_16NM_TXRX_PARAMS_NUM_OF_PROFILES];

#if (defined(MV_HWS_FREE_RTOS) || defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG))

/**
* @internal mvHwsAvagoSerdesTxOffsetParamsSet function
* @endinternal
*
* @brief   Set a single Tx parameter (TxAmp, TxEmph0 or TxEmph1) into tx-offsets database.
*
* @param[in] sdIndex                  - serdes index
*                                      speedIndex  - serdes speed index
* @param[in] txValue                  - tx value to store in database
*                                      parameter   - parameter type (TxAmp, TxEmph0 or ExEmph1)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAvagoSerdesTxOffsetParamsSet
(
    GT_U8 sdIndex,
    GT_U8 sdSpeedIndex,
    GT_8  txValue,
    GT_U8 parameterType
)
{
#ifdef MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_IMPLEMENTATION_A
    if (parameterType==0)
    {
        mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][TX_AMP_RAW_VAL_IDX(sdIndex)] =
        (    (mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][TX_AMP_RAW_VAL_IDX(sdIndex)] & ~(0xF << TX_AMP_RAW_VAL_SHIFT_OP(sdIndex)))
        | (txValue << TX_AMP_RAW_VAL_SHIFT_OP(sdIndex)) );
    }
    else if (parameterType==1)
    {
        mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][TX_EMPH0_RAW_VAL_IDX(sdIndex)] =
         (   (mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][TX_EMPH0_RAW_VAL_IDX(sdIndex)] & ~(0xF << TX_EMPH0_RAW_VAL_SHIFT_OP(sdIndex)))
        | (txValue << TX_EMPH0_RAW_VAL_SHIFT_OP(sdIndex)) );
    }
    else
    {
        mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][TX_EMPH1_RAW_VAL_IDX(sdIndex)] =
          (  (mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][TX_EMPH1_RAW_VAL_IDX(sdIndex)] & ~(0xF << TX_EMPH1_RAW_VAL_SHIFT_OP(sdIndex)))
        | (txValue << TX_EMPH1_RAW_VAL_SHIFT_OP(sdIndex)));
    }
#else
    if(parameterType < 3)/* TxAmp/TxEmph0/TxEmph1 */
    {
        mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][sdIndex] =
            (mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][sdIndex] & ~(0x1F<<(5*parameterType)))
                | (txValue << (5*parameterType));
    }
    else /*RxType (preset/init)*/
    {
        mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][sdIndex] =
            (mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][sdIndex] & ~(0x1<<15)) | (txValue << 15);
    }
#endif
    return GT_OK;

}

/**
* @internal mvHwsAvagoSerdesTxParametersOffsetSet function
* @endinternal
*
* @brief   Initialize Avago related configurations
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] phyPortNum               - port number
* @param[in] serdesSpeed              port speed
* @param[in] serdesNumber             - serdes number
* @param[in] offsets                  - value which stores 12 bits of tx  values
*
* @param[out] baseAddrPtr              - (pointer to) base address of the tx offsets database in FW memory
* @param[out] sizePtr                  - (pointer to) size of the tx offsets database in FW memory
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxParametersOffsetSet
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    MV_HWS_SERDES_SPEED serdesSpeed,
    GT_U8  serdesNumber,
    GT_U16 offsets,
    GT_U32 *baseAddrPtr,
    GT_U32 *sizePtr
)
{
    GT_U8 sdSpeed, speedIdx;

    sdSpeed = serdesSpeed;

    speedIdx = mvHwsSerdesSpeedToIndex(devNum,sdSpeed);
    if (speedIdx == UNPERMITTED_SD_SPEED_INDEX)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"serdes speed not supported for serdes tx params offset API");
    }

    /* TxAmp */
    CHECK_STATUS_EXT(mvHwsAvagoSerdesTxOffsetParamsSet(serdesNumber, speedIdx, offsets&0x1F, 0),
                     LOG_ARG_STRING_MAC("setting tx offsets TxAmp"));
    /* TxEmph0 */
    CHECK_STATUS_EXT(mvHwsAvagoSerdesTxOffsetParamsSet(serdesNumber, speedIdx, ((offsets&0x3E0) >> 5), 1),
                     LOG_ARG_STRING_MAC("setting tx offsets TxEmph0"));
    /* TxEmph1 */
    CHECK_STATUS_EXT(mvHwsAvagoSerdesTxOffsetParamsSet(serdesNumber, speedIdx, ((offsets&0x7C00) >> 10), 2),
                     LOG_ARG_STRING_MAC("setting tx offsets TxEmph1"));

    /* Request Type - Preset/Init */
    CHECK_STATUS_EXT(mvHwsAvagoSerdesTxOffsetParamsSet(serdesNumber, speedIdx, ((offsets&0x8000) >> 15), 3),
                     LOG_ARG_STRING_MAC("setting tx offsets TxType"));

    /* TODO: enlarge the structure and the data base inside the FW to support CM4 Tx data */

    /* provide address and size of database in FW for the host usage */
    *baseAddrPtr = (GT_U32)(&mvHwsAvagoSerdesManualTxOffsetDb[0][0]);
    *sizePtr = sizeof(mvHwsAvagoSerdesManualTxOffsetDb);

    return GT_OK;
}
#endif /* (defined(MV_HWS_FREE_RTOS) || defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG)) */

#if (defined(MV_HWS_FREE_RTOS) || defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG))

    #define MV_HWS_AVAGO_SERDES_TX_ATTEN_OFFSET_MAC(_sdNum, _speedIdx) \
    ((_speedIdx != UNPERMITTED_SD_SPEED_INDEX) ? (GT_32)(TX_AMP_RAW_VAL(_sdNum, _speedIdx)) : 0)

    #define MV_HWS_AVAGO_SERDES_TX_POST_OFFSET_MAC(_sdNum, _speedIdx) \
    ((_speedIdx != UNPERMITTED_SD_SPEED_INDEX) ? (GT_32)(TX_EMPH0_RAW_VAL(_sdNum, _speedIdx)) : 0)

    #define MV_HWS_AVAGO_SERDES_TX_PRE_OFFSET_MAC(_sdNum, _speedIdx) \
    ((_speedIdx != UNPERMITTED_SD_SPEED_INDEX) ? (GT_32)(TX_EMPH1_RAW_VAL(_sdNum, _speedIdx)) : 0)

#else
    #define MV_HWS_AVAGO_SERDES_TX_ATTEN_OFFSET_MAC(_sdNum, _speedIdx)   (0)
    #define MV_HWS_AVAGO_SERDES_TX_POST_OFFSET_MAC(_sdNum, _speedIdx) (0)
    #define MV_HWS_AVAGO_SERDES_TX_PRE_OFFSET_MAC(_sdNum, _speedIdx) (0)
#endif /* (defined(MV_HWS_FREE_RTOS) || defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG)) */

/**
* @internal mvHwsAvagoSerdesManualDbInit function
* @endinternal
*
* @brief   Init Avago serdes manual db
*
* @param[in] devNum                   - device number
* @param[in] chipindex               - chip index
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAvagoSerdesManualDbInit
(
    GT_U8                                devNum,
    GT_U32                               chipIndex
)
{
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(chipIndex);

#ifndef BOBK_DEV_SUPPORT
    hwsOsMemSetFuncPtr(&mvHwsAvagoSerdesManualConfigDb[0][0], 0, sizeof(MV_HWS_AVAGO_TXRX_OVERRIDE_CONFIG_DATA)*MV_HWS_LANE_SERDES_SPEED_NUM*MV_HWS_IPC_MAX_PORT_NUM);
#endif
#ifdef RAVEN_DEV_SUPPORT
    hwsOsMemSetFuncPtr(&hwsAvagoSerdesPresetParamsManualConfigDb[0][0], TX_NA, sizeof(MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA)*MV_HWS_LANE_SERDES_SPEED_NUM*MV_HWS_IPC_MAX_PORT_NUM);
#endif

    return GT_OK;
}

#if !defined (MICRO_INIT) || defined (FALCON_DEV_SUPPORT)
/*******************************************************************************
* hwsAvagoSerdesTxRxTuneParamsSet
*
* DESCRIPTION:
*       Set the default Tx and CTLE(Rx) parameters of the defaults DB entry
*       for the proper port mode.
*
* APPLICABLE DEVICES:
*        Caelum; Aldrin; AC3X; Bobcat3; Pipe; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
*
* NOT APPLICABLE DEVICES:
*        Lion2; Bobcat2; xCat3; AC5
*
* INPUTS:
*       devNum    - device number
*       portGroup - port group (core) number
*       serdesNum - serdes number
*       baudRate  - SerDes speed/baud rate
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static GT_STATUS hwsAvagoSerdesTxRxTuneParamsSet
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    MV_HWS_SERDES_SPEED                     i;
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT        txConfig;
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT        rxConfig;
    MV_HWS_SERDES_SPEED                     baudRate = SPEED_NA;
    GT_BOOL                                 opticalMode = GT_FALSE;
    GT_U8                                   speedIdx = 0;
    GT_U8                                   paramsArraySize;
#ifndef BOBK_DEV_SUPPORT
    GT_BOOL                                 update = GT_FALSE;
#endif
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT)))
    MV_HWS_AVAGO_16NM_TXRX_TUNE_PARAMS     *paramsArrayPtr;
#endif
    hwsOsMemSetFuncPtr(&txConfig, 0, sizeof(txConfig));
    hwsOsMemSetFuncPtr(&rxConfig, 0, sizeof(rxConfig));

    baudRate = serdesConfigPtr->baudRate;
    opticalMode = serdesConfigPtr->opticalMode;

#ifndef BOBK_DEV_SUPPORT
    speedIdx = mvHwsSerdesSpeedToIndex(devNum,baudRate);
    if (speedIdx != UNPERMITTED_SD_SPEED_INDEX)
    {
        update = GT_TRUE;
    }
#endif
    /* write values for optical interconnect without changing the db baud rate */
    MV_HWS_CONVERT_TO_OPTICAL_BAUD_RATE_MAC(opticalMode, baudRate);

#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT)))
    if (HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO_16NM)
    {
        if (update && (mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].interconnectProfile < HWS_AVAGO_16NM_TXRX_PARAMS_NUM_OF_PROFILES))
        {   /* non-default interconnect profile */
            paramsArraySize = hwsAvagoSerdesTxRx16nmTuneParamsArraySizes[mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].interconnectProfile];
            paramsArrayPtr  = hwsAvagoSerdesTxRx16nmTuneParamsArrayPtrs[mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].interconnectProfile];
        }
        else
        {
            /* default interconnect profile */
            paramsArraySize = hwsAvagoSerdesTxRx16nmTuneParamsArraySizes[0];
            paramsArrayPtr =  hwsAvagoSerdesTxRx16nmTuneParamsArrayPtrs[0];
        }

        /* find the baudrate inside the requested profile */
        /* Search: */for (i = 0; (i < paramsArraySize) && (paramsArrayPtr[i].serdesSpeed != baudRate); i++);

        if ((i == paramsArraySize) && (hwsAvagoSerdesTxRx16nmTuneParamsArrayPtrs[0] != paramsArrayPtr))
        {   /* if not found - fallback to default profile */
            paramsArraySize = hwsAvagoSerdesTxRx16nmTuneParamsArraySizes[0];
            paramsArrayPtr =  hwsAvagoSerdesTxRx16nmTuneParamsArrayPtrs[0];
            /* Search: */for (i = 0; (i < paramsArraySize) && (paramsArrayPtr[i].serdesSpeed != baudRate); i++);
        }

        if (i == paramsArraySize) return GT_NOT_SUPPORTED;

        /* reset DFE */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0xA, 0xFFFF, NULL));
        if (HWS_DEV_SILICON_TYPE(devNum) == Raven || HWS_DEV_SILICON_TYPE(devNum) == Falcon)
        {
            /* wait 1ms  */
            ms_sleep(1);
        }

        hwsOsMemCopyFuncPtr(&txConfig.txAvago, &paramsArrayPtr[i].txTuneData, sizeof(txConfig.txAvago));
        hwsOsMemCopyFuncPtr(&rxConfig, &paramsArrayPtr[i].rxTuneData, sizeof(rxConfig));

        txConfig.txAvago.atten += MV_HWS_AVAGO_SERDES_TX_ATTEN_OFFSET_MAC(serdesNum, speedIdx); /* Support for offset params API */
        txConfig.txAvago.post  += MV_HWS_AVAGO_SERDES_TX_POST_OFFSET_MAC (serdesNum, speedIdx); /* Support for offset params API */
        txConfig.txAvago.pre   += MV_HWS_AVAGO_SERDES_TX_PRE_OFFSET_MAC  (serdesNum, speedIdx); /* Support for offset params API */

        if (update)
        {
            if (mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].valid & MV_HWS_SERDES_TXRX_TUNE_PARAMS_DB_TX_VALID)
            {
                APPLY_IF_VALID(txConfig.txAvago.atten  ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].txData.atten          ,NA_8BIT);
                APPLY_IF_VALID(txConfig.txAvago.post   ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].txData.post           ,NA_8BIT_SIGNED);
                APPLY_IF_VALID(txConfig.txAvago.pre    ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].txData.pre            ,NA_8BIT_SIGNED);
                APPLY_IF_VALID(txConfig.txAvago.pre2   ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].txData.pre2           ,NA_8BIT_SIGNED);
                APPLY_IF_VALID(txConfig.txAvago.pre3   ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].txData.pre3           ,NA_8BIT_SIGNED);
            }
            if (mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].valid & MV_HWS_SERDES_TXRX_TUNE_PARAMS_DB_RX_VALID)
            {
                APPLY_IF_VALID(rxConfig.rxAvago16nm.dcGain          ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.dcGain         ,NA_8BIT);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.lowFrequency    ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.lowFrequency   ,NA_8BIT);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.highFrequency   ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.highFrequency  ,NA_8BIT);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.bandWidth       ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.bandWidth      ,NA_8BIT);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.gainShape1      ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.gainShape1     ,NA_8BIT);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.gainShape2      ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.gainShape2     ,NA_8BIT);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.minLf           ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.minLf          ,NA_8BIT);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.maxLf           ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.maxLf          ,NA_8BIT);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.minHf           ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.minHf          ,NA_8BIT);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.maxHf           ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.maxHf          ,NA_8BIT);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.bfLf            ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.bfLf           ,NA_8BIT_SIGNED);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.bfHf            ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.bfHf           ,NA_8BIT_SIGNED);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.minPre1         ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.minPre1        ,NA_8BIT_SIGNED);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.maxPre1         ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.maxPre1        ,NA_8BIT_SIGNED);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.minPre2         ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.minPre2        ,NA_8BIT_SIGNED);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.maxPre2         ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.maxPre2        ,NA_8BIT_SIGNED);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.minPost         ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.minPost        ,NA_8BIT_SIGNED);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.maxPost         ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.maxPost        ,NA_8BIT_SIGNED);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.squelch         ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.squelch        ,NA_16BIT);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.shortChannelEn  ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.shortChannelEn ,NA_8BIT);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.termination     ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.termination    ,NA_8BIT);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.coldEnvelope    ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.coldEnvelope   ,NA_8BIT);
                APPLY_IF_VALID(rxConfig.rxAvago16nm.hotEnvelope     ,mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.hotEnvelope    ,NA_8BIT);
            }
        }

        CHECK_STATUS_EXT(mvHwsSerdesManualTxConfig(devNum, portGroup, serdesNum, AVAGO_16NM, &txConfig),
                         LOG_ARG_SERDES_IDX_MAC(serdesNum), LOG_ARG_STRING_MAC("mvHwsAvagoSerdesManualTxConfig"));

        CHECK_STATUS_EXT(mvHwsSerdesManualRxConfig(devNum, portGroup, serdesNum, AVAGO_16NM, &rxConfig),
                         LOG_ARG_SERDES_IDX_MAC(serdesNum), LOG_ARG_STRING_MAC("mvHwsAvago16nmSerdesManualRxConfig"));
        /* nrz max eye config */
        if (hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM)
        {
            mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_GLOBAL_TUNE_PARAMS,  0x1F, 1000 , NA_8BIT );
        }

        /* disable idle detect because detection is done on signal ok only*/
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x20, 0, NULL));

        if (((baudRate == _26_5625G_PAM4)||(baudRate == _27_1875_PAM4)) /*&& (update && (mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].interconnectProfile !=1 ))*/) {
            /* level 3 threshold */
            mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_GRADIENT_INPUTS,  0xB, 10 , NA_8BIT );
            /* min level 3 */
            mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_GRADIENT_INPUTS,  0x14, 10 , NA_8BIT );
            /* change CDR BB GAIN =1 */
            mvHwsAvago16nmSerdesHalSetIfValid(devNum, serdesNum, HWS_SERDES_PLL_GAIN,  0x1, 0x701 , NA_8BIT );
        }
        return GT_OK;
    }
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT)) */
#if (!defined(RAVEN_DEV_SUPPORT) && !defined(FALCON_DEV_SUPPORT))
    if (HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO)
    {
        paramsArraySize = hwsAvagoSerdesTxRxTuneParamsArrayGetSize();

        /* find the baudrate inside the database */
        /* Search: */for (i = 0; (i < paramsArraySize) && (hwsAvagoSerdesTxRxTuneParamsArray[i].serdesSpeed != baudRate); i++);

        if (i == paramsArraySize) return GT_NOT_SUPPORTED;

        hwsOsMemCopyFuncPtr(&txConfig.txAvago, &hwsAvagoSerdesTxRxTuneParamsArray[i].txParams, sizeof(txConfig.txAvago));
        hwsOsMemCopyFuncPtr(&rxConfig.rxAvago, &hwsAvagoSerdesTxRxTuneParamsArray[i].rxParams, sizeof(rxConfig.rxAvago));

        /* Tx values */
        txConfig.txAvago.atten += MV_HWS_AVAGO_SERDES_TX_ATTEN_OFFSET_MAC(serdesNum, speedIdx); /* Support for offset params API */
        txConfig.txAvago.post  += MV_HWS_AVAGO_SERDES_TX_POST_OFFSET_MAC (serdesNum, speedIdx); /* Support for offset params API */
        txConfig.txAvago.pre   += MV_HWS_AVAGO_SERDES_TX_PRE_OFFSET_MAC  (serdesNum, speedIdx); /* Support for offset params API */

#ifndef BOBK_DEV_SUPPORT
        if (update)
        {
            if (mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].valid & MV_HWS_SERDES_TXRX_TUNE_PARAMS_DB_TX_VALID)
            {
                APPLY_IF_VALID(txConfig.txAvago.atten, mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].txData.atten,          NA_8BIT);
                APPLY_IF_VALID(txConfig.txAvago.post,  mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].txData.post,           NA_8BIT_SIGNED);
                APPLY_IF_VALID(txConfig.txAvago.pre,   mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].txData.pre,            NA_8BIT_SIGNED);
            }
            if (mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].valid & MV_HWS_SERDES_TXRX_TUNE_PARAMS_DB_RX_VALID)
            {
                APPLY_IF_VALID(rxConfig.rxAvago.dcGain,        mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.dcGain,        NA_8BIT);
                APPLY_IF_VALID(rxConfig.rxAvago.lowFrequency,  mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.lowFrequency,  NA_8BIT);
                APPLY_IF_VALID(rxConfig.rxAvago.highFrequency, mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.highFrequency, NA_8BIT);
                APPLY_IF_VALID(rxConfig.rxAvago.bandWidth,     mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.bandWidth,     NA_8BIT);
                APPLY_IF_VALID(rxConfig.rxAvago.squelch,       mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNum].rxData.squelch,       NA_16BIT);
            }
        }
#endif
        CHECK_STATUS_EXT(mvHwsSerdesManualTxConfig(devNum, portGroup, serdesNum, AVAGO, &txConfig),
                         LOG_ARG_SERDES_IDX_MAC(serdesNum), LOG_ARG_STRING_MAC("mvHwsAvagoSerdesManualTxConfig"));

        CHECK_STATUS_EXT(mvHwsSerdesManualRxConfig(devNum, portGroup, serdesNum, AVAGO, &rxConfig),
                         LOG_ARG_SERDES_IDX_MAC(serdesNum), LOG_ARG_STRING_MAC("mvHwsAvagoSerdesManualCtleConfig"));

        return GT_OK;
    }
#endif
    hwsOsPrintf("wrong serdes type\n");
    return GT_BAD_PARAM;
    return ((LAST_MV_HWS_SERDES_SPEED == i) ? GT_NOT_SUPPORTED : GT_OK);
}
#else
/* Unique ports table structure typedef from Micro-init FW */
typedef struct UNQ_PORT_TABLE{
    GT_U8   portType;  /* of type UNQ_PORT_TYPE */
    GT_U8   devNum;    /* device number */
    GT_U8   PortNum;   /* port number of PP */
    GT_U8   portState; /* of type UNQ_PORT_STATE */
    GT_U8   portMode;  /* of type MV_HWS_PORT_STANDARD */
    GT_U16   portSpeed; /* of type MV_HWS_SERDES_SPEED */
    GT_U8   Channel;   /* XSMI interface number or I2C channel for SFP*/
    GT_U8   Addr;      /* PHY XSMI address or I2C address of SFP */

} UNQ_PORT_TABLE;
extern UNQ_PORT_TABLE *unqTableArrayGetPortByDevPort( GT_U8 devNum, GT_U8 portNum );
extern MV_HWS_AVAGO_TXRX_TUNE_PARAMS miAvagoSerdesTxRxTuneParamsArray[];

static GT_STATUS hwsAvagoSerdesTxRxTuneParamsSet
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    GT_STATUS                        rc;
    MV_HWS_SERDES_SPEED              i                 = LAST_MV_HWS_SERDES_SPEED;
    MV_HWS_AVAGO_TXRX_TUNE_PARAMS   *pTuneParamsArray = NULL;
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT txConfig;
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT rxConfig;
    MV_HWS_SERDES_SPEED              baudRate          = SPEED_NA;
    GT_BOOL                          opticalMode       = GT_FALSE;
    GT_U8                            speedIdx          = 0;

    GT_U8 portMode = unqTableArrayGetPortByDevPort(devNum, serdesNum)->portMode;
    baudRate = serdesConfigPtr->baudRate;
    opticalMode = serdesConfigPtr->opticalMode;
    /* apply SerDes optical values */
    MV_HWS_CONVERT_TO_OPTICAL_BAUD_RATE_MAC(opticalMode, baudRate);

    if ( (_10_3125G == baudRate || _3_125G == baudRate) && portMode != NON_SUP_MODE ) {
        pTuneParamsArray = &miAvagoSerdesTxRxTuneParamsArray[_3_125G==baudRate?0:1];
    } else {
        for (i = SPEED_NA; i < LAST_MV_HWS_SERDES_SPEED && hwsAvagoSerdesTxRxTuneParamsArray[i].serdesSpeed != baudRate; i++);
        pTuneParamsArray = &hwsAvagoSerdesTxRxTuneParamsArray[i];
    }

    if ( NULL != pTuneParamsArray ){
        /* debug print */
#if(defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_FW_DEBUG) && defined(MV_HWS_FREE_RTOS))
        speedIdx = mvHwsSerdesSpeedToIndex(devNum, baudRate);
        hwsOsPrintf("\n sdSpeedIterator=%d \n defaultTxAmp=%d, defaultTxEmph0=%d, defaultTxEmph1=%d \n "
                 " txAmpCalculated=%d, txEmph0Calculated=%d, txEmph1Calculated=%d \n "
                 " [32bit] dbTxAmp=%d, dbTxEmph0=%d, dbTxEmph1=%d \n "
                 " [8bit] dbTxAmp=%d, dbTxEmph0=%d, dbTxEmph1=%d",i,
                 pTuneParamsArray->txParams.txAmp,
                 pTuneParamsArray->txParams.txEmph0,
                 pTuneParamsArray->txParams.txEmph1,
            (pTuneParamsArray->txParams.txAmp+
                (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_ATTEN_OFFSET_MAC(serdesNum, speedIdx)),
            (pTuneParamsArray->txParams.txEmph0+
                (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_POST_OFFSET_MAC(serdesNum, speedIdx)),
            (pTuneParamsArray->txParams.txEmph1+
                (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_PRE_OFFSET_MAC(serdesNum, speedIdx)),
                 (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_ATTEN_OFFSET_MAC(serdesNum, speedIdx),
                 (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_POST_OFFSET_MAC(serdesNum, speedIdx),
                 (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_PRE_OFFSET_MAC(serdesNum, speedIdx),
                 (GT_8)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_ATTEN_OFFSET_MAC(serdesNum, speedIdx),
                 (GT_8)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_POST_OFFSET_MAC(serdesNum, speedIdx),
                 (GT_8)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_PRE_OFFSET_MAC(serdesNum, speedIdx));
#endif /* (defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_FW_DEBUG) && defined(MV_HWS_FREE_RTOS)) */
        speedIdx = mvHwsSerdesSpeedToIndex(devNum, baudRate);
        hwsOsMemSetFuncPtr(&txConfig, 0, sizeof(txConfig));
        txConfig.txAvago.atten = (0xFF & (pTuneParamsArray->txParams.atten + MV_HWS_AVAGO_SERDES_TX_ATTEN_OFFSET_MAC(serdesNum, speedIdx)));
        txConfig.txAvago.post  = (0xFF & (pTuneParamsArray->txParams.post  + MV_HWS_AVAGO_SERDES_TX_POST_OFFSET_MAC(serdesNum, speedIdx)));
        txConfig.txAvago.pre   = (0xFF & (pTuneParamsArray->txParams.pre   + MV_HWS_AVAGO_SERDES_TX_PRE_OFFSET_MAC(serdesNum, speedIdx)));
        rc = mvHwsAvagoSerdesManualTxConfig(devNum, portGroup, serdesNum, &txConfig);
        if (rc != GT_OK)
        {
            return rc;
        }

        hwsOsMemSetFuncPtr(&rxConfig, 0, sizeof(rxConfig));
        rxConfig.rxAvago.dcGain        = pTuneParamsArray->rxParams.dcGain;
        rxConfig.rxAvago.bandWidth     = pTuneParamsArray->rxParams.bandWidth;
        rxConfig.rxAvago.highFrequency = pTuneParamsArray->rxParams.highFrequency;
        rxConfig.rxAvago.lowFrequency  = pTuneParamsArray->rxParams.lowFrequency;
        rxConfig.rxAvago.squelch       = pTuneParamsArray->rxParams.squelch;
        rc = mvHwsAvagoSerdesManualCtleConfig(devNum, portGroup, serdesNum, &rxConfig);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}
#endif

#if defined(MV_HWS_REDUCED_BUILD_EXT_CM3)

void mvHwsAvagoAaplInit
(
    unsigned char devNum, unsigned char chipIndex
)
{
    unsigned int i;
#if AAPL_ALLOW_JTAG || AAPL_ALLOW_AACS
    aaplSerdesDbDef[chipIndex].buf_cmd = NULL;
    aaplSerdesDbDef[chipIndex].buf_cmd_end = NULL;
#endif /* AAPL_ALLOW_JTAG || AAPL_ALLOW_AACS */
    aaplSerdesDbDef[chipIndex].aacs_server_buffer = NULL;
    aaplSerdesDbDef[chipIndex].aacs_server = NULL;
    for(i = 0; i < AAPL_MAX_CHIPS; i++)
    {
        aaplSerdesDbDef[chipIndex].chip_name[i] = NULL;
        aaplSerdesDbDef[chipIndex].chip_rev[i] = NULL;
    }
    aaplSerdesDbDef[chipIndex].data_char_end = NULL;
    aaplSerdesDbDef[chipIndex].log = NULL;
    aaplSerdesDbDef[chipIndex].log_end = NULL;
    aaplSerdesDbDef[chipIndex].data_char = NULL;
    aaplSerdesDbDef[chipIndex].client_data = NULL;
    if(HWS_DEV_SILICON_TYPE(devNum) ==  Bobcat3 || HWS_DEV_SILICON_TYPE(devNum) ==  Aldrin2)
    {
        aaplSerdesDb[chipIndex]->sbus_rings = 2;
    }
    else
    {
        aaplSerdesDb[chipIndex]->sbus_rings = 1;
    }
    /* Register User-supplied Communication Functions */
    aapl_register_sbus_fn(aaplSerdesDb[chipIndex], user_supplied_sbus_function, 0, 0);

    /*reset devNumber and portGroup*/
    aaplSerdesDb[chipIndex]->devNum = devNum;
    aaplSerdesDb[chipIndex]->portGroup = 0;
    aaplSerdesDbDef[chipIndex].devNum = devNum;
    aaplSerdesDbDef[chipIndex].portGroup = 0;

    aapl_user_supplied_log_func_ptr = mvHwsGeneralLogStrMsgWrapper;
}

#endif

void mvHwsAvagoAaplAddrGet
(
    unsigned int chipIndex,
    GT_UINTPTR   *devAddr,
    unsigned int *devSize
)
{
    *devAddr = (GT_UINTPTR)(aaplSerdesDb[chipIndex]);
    *devSize = (unsigned int)sizeof(Aapl_t);
}

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
GT_BOOL mvHwsAvagoFalconIsCpuSerdes
(
    GT_U8 devNum,
    unsigned int serdesNum,
    unsigned int *chipIndex
)
{
    unsigned int numRegularPorts = 64*hwsDeviceSpecInfo[devNum].numOfTiles;
#ifdef CHX_FAMILY
    /* Check Falcon 2T/4T/3.2T devices*/
    if ((hwsFalconTypeGet(devNum) == HWS_FALCON_DEV_98CX8525_E) ||
        (hwsFalconTypeGet(devNum) == HWS_FALCON_DEV_98CX8535_E) ||
        (hwsFalconTypeGet(devNum) == HWS_FALCON_DEV_98CX8514_E) ||
        (hwsFalconTypeGet(devNum) == HWS_FALCON_DEV_98EX5614_E))
    {
        if(serdesNum == 80)
        {
            *chipIndex = 5;
            return GT_TRUE;
        }
        else if(serdesNum == 81)
        {
            *chipIndex = 7;
            return GT_TRUE;
        }
        return GT_FALSE;
    }
#endif

    if(serdesNum >= numRegularPorts)
    {
        /* the Raven index (taking into account multi device setup*/
        *chipIndex = (RAVEN_MAX_AVAGO_CHIP_NUMBER * devNum) + (serdesNum - numRegularPorts);
        /* CPU port (reduced port) */
        return GT_TRUE;
    }
    *chipIndex = 0;
    /* regular port */
    return GT_FALSE;
}
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

unsigned int mvHwsAvagoGetAaplChipIndex
(
    unsigned char devNum,
    unsigned int serdesNum
)
{
    unsigned int chipIndex = devNum;
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) || defined (FALCON_DEV_SUPPORT)
    if (HWS_DEV_SILICON_TYPE(devNum) == Falcon)
    {

#ifndef FALCON_DEV_SUPPORT
        /* special check for CPU ports */
        if (mvHwsAvagoFalconIsCpuSerdes(devNum,serdesNum,&chipIndex))
        {
            return chipIndex;
        }
#else
        devNum = 0; /* In Micro Init DevNum is always 0 */
#endif

#ifdef CHX_FAMILY
        /* Check Falcon 2T/4T/3.2T devices*/
        if ((hwsFalconTypeGet(devNum) == HWS_FALCON_DEV_98CX8525_E) ||
            (hwsFalconTypeGet(devNum) == HWS_FALCON_DEV_98CX8535_E) ||
            (hwsFalconTypeGet(devNum) == HWS_FALCON_DEV_98CX8514_E) ||
            (hwsFalconTypeGet(devNum) == HWS_FALCON_DEV_98EX5614_E))
        {
            switch((serdesNum / RAVEN_MAX_AVAGO_SERDES_NUMBER))
            {
                 case 1:
                    chipIndex = 2;
                    break;
                case 2:
                    chipIndex = 4;
                    break;
                case 3:
                    chipIndex = 5;
                    break;
                case 4:
                    chipIndex = 7;
                    break;
                case 0:
                default:
                    chipIndex = 0;
                    break;
            }
            chipIndex += (RAVEN_MAX_AVAGO_CHIP_NUMBER * devNum);
        }
        else
#endif
        {
            chipIndex = (RAVEN_MAX_AVAGO_CHIP_NUMBER * devNum) + (serdesNum / RAVEN_MAX_AVAGO_SERDES_NUMBER);
        }
    }
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */
    return chipIndex;
}


int mvHwsAvagoInitializationCheck
(
    unsigned char devNum,
    unsigned int  serdesNum
)
{
    unsigned int chipIndex = devNum;
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if (HWS_DEV_SILICON_TYPE(devNum) == Falcon)
    {
        chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    }
#endif

    return (aaplSerdesDb[chipIndex] == NULL) ? GT_NOT_INITIALIZED : GT_OK;
}

GT_STATUS mvHwsAvagoConvertSerdesToSbusAddr
(
    unsigned char devNum,
    unsigned int  serdesNum,
    GT_U32  *sbusAddr
)
{
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    unsigned int chipIndex;
#endif
    CHECK_STATUS(mvHwsAvagoInitializationCheck(devNum, serdesNum));

    if(serdesNum > hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesNumber)
    {
        return GT_BAD_PARAM;
    }
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) || defined (FALCON_DEV_SUPPORT)
    if (HWS_DEV_SILICON_TYPE(devNum) == Falcon)
    {
#ifndef FALCON_DEV_SUPPORT
        /* special check for CPU ports */
        if (mvHwsAvagoFalconIsCpuSerdes(devNum,serdesNum,&chipIndex))
        {
            *sbusAddr = 17;
        }
        else
#endif
        {
            *sbusAddr = hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesToAvagoMapPtr[serdesNum % 16];
        }
    }
    else
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */
    {
        *sbusAddr = hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesToAvagoMapPtr[serdesNum];
    }
    return GT_OK;
}

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
/**
* @internal mvHwsAvagoEthDriverInit function
* @endinternal
*
* @brief   Initialize Avago related configurations
*
* @param[in] devNum                  - Device Number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
unsigned int mvHwsAvagoEthDriverInit
(
    IN unsigned char chipIndex
)
{
#ifndef ASIC_SIMULATION
    /* Set default values: */

    char ip_address[] = "10.5.32.124";
    int     tcp_port     = 90; /* Default port for Avago HS1/PS1 */

    aapl_connect(aaplSerdesDb[chipIndex], ip_address, tcp_port);
    if(aaplSerdesDb[chipIndex]->return_code < 0)
    {
        hwsOsPrintf("aapl_connect failed (return code 0x%x)\n", aaplSerdesDb[chipIndex]->return_code);
        return GT_INIT_ERROR;
    }

#endif /* ASIC_SIMULATION */
    return GT_OK;
}

#if AAPL_ENABLE_AACS_SERVER

extern unsigned int osTaskCreate(const char *name, unsigned int prio, unsigned int stack,
                                 unsigned (__TASKCONV *start_addr)(void*), void *arglist, GT_U32 *tid);

typedef struct
{
    GT_U8           devNum;    /* device number */
    unsigned int    tcpPort;   /* TCP port connection to AACS Server */

} AVAGO_AACS_SERVER_TASK_PARAMS_STC;

static AVAGO_AACS_SERVER_TASK_PARAMS_STC    aacsServerTaskParamsArr[RAVEN_MAX_AVAGO_SERDES_NUMBER];

/**
* @brief   Create Avago AACS Server Process
* @retval 0 - on success
* @retval 1 - on error
*/
static unsigned __TASKCONV avagoAacsServerStartTask
(
    IN GT_VOID * param
)
{
    GT_U8           chipIndex;
    unsigned int    tcpPort;
    AVAGO_AACS_SERVER_TASK_PARAMS_STC   aacsServerParam;    /* task parameters */

    aacsServerParam = *(AVAGO_AACS_SERVER_TASK_PARAMS_STC*)param;

    chipIndex  = aacsServerParam.devNum;
    tcpPort = aacsServerParam.tcpPort;

    avago_aacs_server(aaplSerdesDb[chipIndex], tcpPort);

    return GT_OK;
}

/*******************************************************************************
* mvHwsAvagoSerdesAacsServerOpen
*
* DESCRIPTION:
*       Create Avago AACS Server Task on each device
*
* INPUTS:
*       task name - AACS Server Task identifier
*       startDevNum - First device number to initialized
*       endDevNum   - Last  device number to initialized
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static GT_STATUS mvHwsAvagoSerdesAacsServerOpen
(
    char            *name,
    unsigned char   startDevNum,
    unsigned char   endDevNum
)
{
    unsigned char   tcpPort;
    unsigned char   devNum;
#ifdef CHX_FAMILY
    if ((HWS_DEV_SILICON_TYPE(startDevNum) == Raven) || (HWS_DEV_SILICON_TYPE(startDevNum) == Falcon))
    {
        if ((endDevNum > (hwsFalconNumOfRavens-1)) || (startDevNum > endDevNum))
        {
            return GT_BAD_PARAM;
        }
    }
    else
#endif
    if (startDevNum != endDevNum)
    {
        return GT_BAD_PARAM;
    }

    tcpPort = 90+startDevNum;
    for (devNum = startDevNum; devNum <= endDevNum; devNum++)
    {
#ifdef CHX_FAMILY
        if ((HWS_DEV_SILICON_TYPE(startDevNum) == Raven) || (HWS_DEV_SILICON_TYPE(startDevNum) == Falcon))
        {
            if ((hwsRavenDevBmp & (1<<devNum)) == 0)
            {
                continue;
            }
        }
#endif
        aacsServerTaskParamsArr[devNum].devNum  = devNum;
        aacsServerTaskParamsArr[devNum].tcpPort = tcpPort;

        if ((osTaskCreate(name, 250, _64K, avagoAacsServerStartTask, &aacsServerTaskParamsArr[devNum], &avagoAacsServerTid)) != GT_OK)
        {
            return GT_FAIL;
        }

        tcpPort++;
    }

    return GT_OK;
}

/*******************************************************************************
* avagoSerdesAacsServerExec
*
* DESCRIPTION:
*       Initialize the Avago AACS Server on device
*
* INPUTS:
*       startDevNum - First device number to initialized
*       endDevNum   - Last  device number to initialized
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0  - on success
*       1  - on error
*******************************************************************************/
#ifndef ASIC_SIMULATION
int avagoSerdesAacsServerExec
(
    unsigned char   startDevNum,
    unsigned char   endDevNum
)
{
    int res;

    res = mvHwsAvagoSerdesAacsServerOpen("mvHwsAvagoAACS_Server", startDevNum, endDevNum);
    if (res != GT_OK)
    {
        hwsOsPrintf("Failed to init Avago AACS Server\n");
        return res;
    }

    return GT_OK;
}
#endif /* ASIC_SIMULATION */
#endif /* AAPL_ENABLE_AACS_SERVER */
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT)
/**
* @internal mvHwsAvagoSpicoLoad function
* @endinternal
*
*/
static int mvHwsAvagoSpicoLoad(unsigned char devNum, unsigned int chipIndex, unsigned int sbus_addr)
{
    /*
     * Dynamically select between micro_init Fw vs AP Fw at run time
     * for micro_init swapFwSize is set to binary file size from super-image
     */
    BOOL bist = FALSE;
#ifndef MICRO_INIT
    int serdesFwSize = 0;
    const int *serdesFwPtr = NULL;
    GT_BOOL memAllocated = GT_FALSE;

    if (HWS_DEV_SERDES_TYPE(devNum, 0 /* serdesNum*/) == AVAGO_16NM)
    {
        serdesFwSize = AVAGO_SERDES_FW_16NM_IMAGE_SIZE;
        serdesFwPtr = serdes_0x10B9_208D_009_Data;

        if((PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesExternalFirmware))
           && (avago_load_rom_from_file(aaplSerdesDb[chipIndex],"serdes_fw//avago_16nm//serdes.fw.rom",&serdesFwSize,(int**)&serdesFwPtr) == 0))
        {
            hwsOsPrintf("Using SerDes FW from file:%s size:%d\n", "serdes_fw//avago_16nm//serdes.fw.rom", serdesFwSize);
            memAllocated = GT_TRUE;
        }

        bist = TRUE;
    }
    else
    {
        serdesFwSize = AVAGO_SERDES_FW_28NM_IMAGE_SIZE;
        serdesFwPtr  = serdes0x2464_0245Data;

        if((PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesExternalFirmware))
           && (avago_load_rom_from_file(aaplSerdesDb[chipIndex],"serdes_fw//avago//serdes.fw.rom",&serdesFwSize,(int**)&serdesFwPtr) == 0))
        {
            hwsOsPrintf("Using SerDes FW from file:%s size:%d\n", "serdes_fw//avago//serdes.fw.rom", serdesFwSize);
            memAllocated = GT_TRUE;
        }
    }
#endif
    AVAGO_DBG(("Loading Spico Avago FW to SBus address %x, serdesFwSize=%d, data[0]=%x\n", sbus_addr, serdesFwSize, serdesFwPtr[0]));
    avago_spico_upload(aaplSerdesDb[chipIndex], sbus_addr, bist, serdesFwSize, serdesFwPtr);
    if (aaplSerdesDb[chipIndex]->return_code < 0)
    {
        hwsOsPrintf("mvHwsAvagoSpicoLoad: Loading Spico FW Load Failed on Raven_%d, (sbus_addr=%d), (return code 0x%x)\n", chipIndex, sbus_addr, aaplSerdesDb[chipIndex]->return_code);
        return GT_INIT_ERROR;
    }
#ifndef MICRO_INIT
    if (memAllocated)
    {
        aapl_free(aaplSerdesDb[chipIndex], (int*)serdesFwPtr, __func__);
    }
#endif
    return GT_OK;

}

/**
* @internal mvHwsAvagoSbusMasterLoad function
* @endinternal
*
*/
static int mvHwsAvagoSbusMasterLoad(unsigned char devNum, unsigned int chipIndex, unsigned int sbus_addr)
{
    /*
     * Dynamically select between micro_init Fw vs AP Fw at run time
     * for micro_init swapFwSize is set to binary file size from super-image
     */
    BOOL bist = FALSE;
#ifndef MICRO_INIT
    int swapFw28nmSize = 0;
    const int *serdesFw28nmSwapPtr = NULL;
    int sbusMasterFwSize = 0;
    const int *sbusMasterFwPtr = NULL;

    if (HWS_DEV_SERDES_TYPE(devNum, 0 /* serdesNum*/) == AVAGO_16NM)
    {
        sbusMasterFwSize = AVAGO_SBUS_MASTER_FW_16NM_IMAGE_SIZE;
        sbusMasterFwPtr  = sbusMaster0x0226_2001Data;
        bist = TRUE;
    }
    else
    {
        sbusMasterFwSize = AVAGO_SBUS_MASTER_FW_28NM_IMAGE_SIZE;
        swapFw28nmSize   = AVAGO_SERDES_FW_28NM_SWAP_IMAGE_SIZE;
        sbusMasterFwPtr     = sbusMaster01x101A_0001Data;
        serdesFw28nmSwapPtr = serdes0x2464_0245Swap;
    }
#endif

    AVAGO_DBG(("Loading Sbus Master file to SBus address: 0x%x  data[0]=%x\n", sbus_addr , sbusMasterFwPtr[0]));
    avago_spico_upload(aaplSerdesDb[chipIndex], sbus_addr, bist, sbusMasterFwSize, sbusMasterFwPtr);
    if (aaplSerdesDb[chipIndex]->return_code < 0)
    {
        hwsOsPrintf("Loading Sbus Master Avago FW Failed (chipIndex=%d) (return code 0x%x)\n", chipIndex, aaplSerdesDb[chipIndex]->return_code);
        return GT_INIT_ERROR;
    }
#ifndef MICRO_INIT
    if (HWS_DEV_SERDES_TYPE(devNum, 0 /* serdesNum*/) == AVAGO)
    {
        AVAGO_DBG(("Loading SWAP file to SBus Master address: 0x%x  data[0]=%x\n", sbus_addr , serdesFw28nmSwapPtr[0]));
        avago_spico_upload_swap_image(aaplSerdesDb[chipIndex], sbus_addr, swapFw28nmSize, serdesFw28nmSwapPtr);
        if (aaplSerdesDb[chipIndex]->return_code < 0)
        {
            hwsOsPrintf("Loading Sbus Master SWAP Avago FW Failed (return code 0x%x)\n", aaplSerdesDb[chipIndex]->return_code);
            return GT_INIT_ERROR;
        }
    }
#else
    if (HWS_DEV_SERDES_TYPE(devNum, 0 /* serdesNum*/) == AVAGO)
    {
        AVAGO_DBG(("Loading SWAP file to SBus Master address: 0x%x  data[0]=%x\n", sbus_addr , serdesFw28nmSwapPtr[0]));
        avago_spico_upload_swap_image(aaplSerdesDb[chipIndex], sbus_addr, swapFwSize, serdesFwDataSwapPtr);
        if (aaplSerdesDb[chipIndex]->return_code < 0)
        {
            hwsOsPrintf("Loading Sbus Master SWAP Avago FW Failed (return code 0x%x)\n", aaplSerdesDb[chipIndex]->return_code);
            return GT_INIT_ERROR;
        }
    }
#endif

    return GT_OK;
}
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT) */

#ifndef ASIC_SIMULATION

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
/**
 * mvHwsAaplSerdesDbInit
 *
 * @param devNum
 *
 * @return int
 *       0  - on success
 *       1  - on error
 */
int mvHwsAaplSerdesDbInit(unsigned char devNum, unsigned int chipsInDevBmp)
{
    GT_U32 chipInd;
    GT_U32 numOfChipsInDev = (HWS_DEV_SILICON_TYPE(devNum) == Falcon) ? (RAVEN_MAX_AVAGO_CHIP_NUMBER) : (1);

    for (chipInd = 0; chipInd < numOfChipsInDev; chipInd++)
    {
        if(chipsInDevBmp & (1 << chipInd))
        {
            if ( NULL == aaplSerdesDb[chipInd + devNum * numOfChipsInDev] )
            {
                aaplSerdesDb[chipInd + devNum * numOfChipsInDev] = (Aapl_t*)cpssOsMalloc(sizeof(Aapl_t));
            }
        }
    }

    return GT_OK;
}
#endif

/*******************************************************************************
* mvHwsAvagoSerdesInit
*
* DESCRIPTION:
*       Initialize Avago related configurations
*
* INPUTS:
*       devNum - system device number
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0  - on success
*       1  - on error
*******************************************************************************/
int mvHwsAvagoSerdesInit(unsigned char devNum, unsigned int chipIndex)
{
    unsigned int chip=0;    /* we have only one chip per aaplSerdesDb[devNum] */
    Avago_addr_t addr_struct;
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT)
    unsigned int sbus_addr;
    unsigned int curr_adr;
    unsigned int ring = 0;   /* we have only one ring */
    GT_U32 serdes_num;
#endif
    unsigned int regAddr = 0;
    unsigned int regAddr1 = 0;
    GT_U32 sbusClkDiv = 3;

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    GT_STATUS rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }
#endif
    /* Validate AAPL */
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if (aaplSerdesDb[chipIndex] == NULL)
    {
        /* structures weren't initialized */
        return GT_NOT_INITIALIZED;
    }
#else
   if (aaplSerdesDb[chipIndex] != NULL)
    {
        /* structures were already initialized */
        return GT_ALREADY_EXIST;
    }
#endif

    /* Construct AAPL structure */
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    aapl_init(aaplSerdesDb[chipIndex]);
#else
    aapl_init(&(aaplSerdesDbDef[chipIndex]));
    aaplSerdesDb[chipIndex] = &(aaplSerdesDbDef[chipIndex]);
#endif
    aaplSerdesDb[chipIndex]->devNum    = devNum;
    aaplSerdesDb[chipIndex]->portGroup = 0;
    aaplSerdesDb[chipIndex]->chipIndex = chipIndex;

    /* In BC3 device we have two different serdes chains. In all
       other devices there is a single chain. Chains are indexed
       by "ring" field name. */

    if(HWS_DEV_SILICON_TYPE(devNum) ==  Bobcat3 || HWS_DEV_SILICON_TYPE(devNum) ==  Aldrin2)
    {
        aaplSerdesDb[chipIndex]->sbus_rings = 2;
    }
    else
    {
        aaplSerdesDb[chipIndex]->sbus_rings = 1;
    }

    /* Register User-supplied Communication Functions */
    aapl_register_sbus_fn(aaplSerdesDb[chipIndex], user_supplied_sbus_function, 0, 0);
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E){
#endif
    /* Take Avago device out of reset */
    user_supplied_sbus_soft_reset(aaplSerdesDb[chipIndex]);
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    }
#endif
    /* Change the SBUS master clk divider to 16. 0x6007F828 is indirect address */
    if(HWS_DEV_SILICON_TYPE(devNum) == Pipe)
    {
        regAddr = 0x0607F828;
        regAddr1 = 0x06042404;
    }
    else if((HWS_DEV_SILICON_TYPE(devNum) == Raven) || (HWS_DEV_SILICON_TYPE(devNum) == Falcon))
    {
        CHECK_STATUS(user_supplied_get_raven_sbus_unit_address(chipIndex, SBC_CLK_DIV_FALCON_UNIT0_BASE_ADDRESS, &regAddr));
        CHECK_STATUS(user_supplied_get_raven_sbus_unit_address(chipIndex, SBC_TSEN_CLK_DIV_FALCON_UNIT0_BASE_ADDRESS, &regAddr1));
    }
    else
    {
        regAddr = 0x6007F828;
        regAddr1 = 0x60042404;
        if(HWS_DEV_SILICON_TYPE(devNum) == Aldrin) /* Since Aldrin works in a different ref clock it should have different sbusClkDiv */
            sbusClkDiv = 0x4;
    }

    if(regAddr != 0xFFFFFFFF)
    {
        /* change SBUS clock divider to div_8 */
        CHECK_STATUS_EXT(hwsRegisterSetFuncPtr(devNum, aaplSerdesDb[chipIndex]->portGroup, regAddr, sbusClkDiv/*0x3*/, 0xFFFF), LOG_ARG_NONE_MAC );
    }

    if(regAddr1 != 0xFFFFFFFF)
    {
        if((HWS_DEV_SILICON_TYPE(devNum) == Raven) || (HWS_DEV_SILICON_TYPE(devNum) == Falcon))
        {
            /* Change the Sensor clk divider to 83 */
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, aaplSerdesDb[chipIndex]->portGroup, regAddr1, 0x53, 0xFFFF));
        }
        else
        {
            /* Change the Sensor clk divider to 78 */
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, aaplSerdesDb[chipIndex]->portGroup, regAddr1, 0x4E, 0xFFFF));
        }
    }

#ifndef MICRO_INIT /* Only communication through SBUS used */
    if (avagoConnection == AVAGO_ETH_CONNECTION)
    {
        aaplSerdesDb[chipIndex]->communication_method = AAPL_DEFAULT_COMM_METHOD;

        #ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
        CHECK_STATUS(mvHwsAvagoEthDriverInit(chipIndex));
        #endif
    }
    else if (avagoConnection == AVAGO_I2C_CONNECTION)
    {
        aaplSerdesDb[chipIndex]->communication_method = AVAGO_USER_SUPPLIED_I2C;
        #ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
        CHECK_STATUS(mvHwsAvagoInitI2cDriver(devNum));
        #endif
    }
    else
#endif /*MICRO_INIT*/

    if (avagoConnection == AVAGO_SBUS_CONNECTION)
    {
        aaplSerdesDb[chipIndex]->communication_method = AVAGO_USER_SUPPLIED_SBUS_DIRECT;
    }
    else
    {
        #ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
        hwsOsPrintf("mvHwsAvagoSerdesInit: unknown communication method %x\n", aaplSerdesDb[chipIndex]->communication_method);
        #endif
        return GT_INIT_ERROR;
    }

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    /* let know to AVAGO that system recovery process is active */
    if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        recovery_process = 1;
    }
    else
    {
        recovery_process = 0;
    }
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

    /* Validate access to Avago device */
    mvHwsAvagoAccessValidate(chipIndex, 0);

    /* Initialize AAPL structure */

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT)
    aapl_get_ip_info(aaplSerdesDb[chipIndex],1);
#endif

    AVAGO_DBG(("Loading Avago Firmware.......\n"));

    /* registration of Log function. Using in aapl_log_printf according to the debug level(0...15).
       by default aapl->debug = AVAGO_DEBUG0 */
    aapl_user_supplied_log_func_ptr = mvHwsGeneralLogStrMsgWrapper;

    /* Converts the address into an address structure */
    addr_struct.chip = chip;

    /* set the attributes in Avago_chip_id_t avago_chip_id[] according to jtag_idcode */
    if (HWS_DEV_SILICON_TYPE(devNum) == Raven || HWS_DEV_SILICON_TYPE(devNum) == Falcon)
    {
        aaplSerdesDb[chipIndex]->jtag_idcode[addr_struct.chip] = 0x0000057f;
    }
    else
    {
        aaplSerdesDb[chipIndex]->jtag_idcode[addr_struct.chip] = 0x0912557f;
    }

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
#endif
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT)
    {
        for (ring=0 ; ring < aaplSerdesDb[chipIndex]->sbus_rings ; ring++)
        {
            addr_struct.ring = ring;

            /* Download Serdes's Firmware */
            /* ========================== */
            if ((HWS_DEV_SILICON_TYPE(devNum) == Raven) || (HWS_DEV_SILICON_TYPE(devNum) == Falcon))
            {
                addr_struct.sbus = AVAGO_SERDES_M4_BROADCAST;
                addr_struct.lane = AVAGO_ADDR_IGNORE_LANE;
                sbus_addr = avago_struct_to_addr(&addr_struct);
                CHECK_STATUS(mvHwsAvagoSpicoLoad(devNum, chipIndex, sbus_addr));
                if (aaplSerdesDb[chipIndex]->return_code < 0)
                {
                    hwsOsPrintf("Avago Spico FW Load Failed on Raven (return code 0x%x)\n", aaplSerdesDb[chipIndex]->return_code);
                    return GT_INIT_ERROR;
                }
            }
            else
            {
                for( curr_adr = 0; curr_adr <= aaplSerdesDb[chipIndex]->max_sbus_addr[chip][ring]; curr_adr++ )
                {
                    if (aaplSerdesDb[chipIndex]->ip_type[chip][ring][curr_adr] == AVAGO_SERDES)
                    {
                        addr_struct.sbus = curr_adr;
                        addr_struct.lane = AVAGO_ADDR_IGNORE_LANE;
                        sbus_addr = avago_struct_to_addr(&addr_struct);
                        CHECK_STATUS(hwsDeviceSpecInfo[devNum].avagoSerdesInfo.sbusAddrToSerdesFuncPtr(devNum, &serdes_num, sbus_addr));
                        if (mvHwsAvagoCheckSerdesAccess(devNum, 0, serdes_num) == GT_NOT_INITIALIZED)
                        {
                            CHECK_STATUS(mvHwsAvagoSpicoLoad(devNum, chipIndex, sbus_addr));
                            if(aaplSerdesDb[chipIndex]->return_code < 0)
                            {
                                hwsOsPrintf("Avago Spico FW Load Failed (return code 0x%x)\n", aaplSerdesDb[chipIndex]->return_code);
                                return GT_INIT_ERROR;
                            }
                        }
                    }
                }
            }

            /* Download SBus_Master Firmware */
            /* ============================= */
            addr_struct.lane = AVAGO_ADDR_IGNORE_LANE;
            addr_struct.sbus = AVAGO_SBUS_MASTER_ADDRESS;
            sbus_addr = avago_struct_to_addr(&addr_struct);
            if (HWS_DEV_SILICON_TYPE(devNum) == Raven || HWS_DEV_SILICON_TYPE(devNum) == Falcon)
            {
                /* wait 200ms before loading firmware to sbus master */
                ms_sleep(200);
            }
            CHECK_STATUS(mvHwsAvagoSbusMasterLoad(devNum, chipIndex, sbus_addr));
            if(aaplSerdesDb[chipIndex]->return_code < 0)
            {
                hwsOsPrintf("SBUS Master Avago FW Load Failed, ring: %d (return code 0x%x)\n", ring, aaplSerdesDb[chipIndex]->return_code);
                return GT_INIT_ERROR;
            }
        }
    }
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT) */
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        if ( (HWS_DEV_SILICON_TYPE(devNum) == BobK) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin) )
        {
            /* update 5 fields in AAPL */
            aaplSerdesDb[chipIndex]->ip_rev[0][0][254] = 0xbe;
            aaplSerdesDb[chipIndex]->ip_type[0][0][253] = 0x3;
            aaplSerdesDb[chipIndex]->lsb_rev[0][0][253] = 0xff;
            aaplSerdesDb[chipIndex]->lsb_rev[0][0][255] = 0x4;
            aaplSerdesDb[chipIndex]->spico_running[0][0][253]=0x1;
        }
        else
            if ( (HWS_DEV_SILICON_TYPE(devNum) == Raven) || (HWS_DEV_SILICON_TYPE(devNum) == Falcon) )
            {
                aaplSerdesDb[chipIndex]->spico_running[0][0][253]=0x1;
                aaplSerdesDb[chipIndex]->ip_type[0][0][253] = 0x3;
                aaplSerdesDb[chipIndex]->lsb_rev[0][0][253] = -1;
            }
    }
#endif /*MICRO_INIT*/

#if defined(CHX_FAMILY) || defined(PX_FAMILY)
    /* Init Avago Access Protection in multi-process environment */
    /* This protection is implemented at the scope of the Host!! */
    /* ========================================================= */
    if(!avagoAccessMutex)
    {
        hwsOsMutexCreate("avagoAccess", &avagoAccessMutex);
    }
#endif /* defined(CHX_FAMILY) || defined(PX_FAMILY) */

#if AAPL_ENABLE_AACS_SERVER
    if (avagoConnection != AVAGO_ETH_CONNECTION)
    {
        if (aacsServerEnable)
        {
            CHECK_STATUS(avagoSerdesAacsServerExec(devNum, devNum));
        }
    }
#endif /* AAPL_ENABLE_AACS_SERVER */
#ifndef MV_HWS_AVAGO_NO_VOS_WA
    if (hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesVosOverride)
    {
        /* Initializing the VOS override params DB in order to prevent DFX
        register access every time those values are needed. In the CM3 service
        CPU those values will be set manually with the values sent from host.*/
#ifndef ALDRIN_DEV_SUPPORT
        if ((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) ||
                        (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
        {
            /* Initializing the VOS override params DB for Bobcat3 in order to prevent DFX
            register access every time those values are needed.*/
            hwsOsMemSetFuncPtr(&serdesVosParams64BitDB[0], 0, sizeof(serdesVosParams64BitDB));
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
            if (!serdesVos64BitDbInitialized)
            {
                CHECK_STATUS(mvHwsAvagoSerdesBc3VosConfig(devNum));
            }
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */
        }
        else
#endif
        {
            hwsOsMemSetFuncPtr(&serdesVosParamsDB[0], 0, sizeof(serdesVosParamsDB));
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
            if (!serdesVosParamsDBInitialized)
            {
                CHECK_STATUS(mvHwsAvagoSerdesManualVosParamsDBInit(devNum));
            }
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */
        }
    }
#endif /* MV_HWS_AVAGO_NO_VOS_WA */

    /*init manual serdes db*/
    mvHwsAvagoSerdesManualDbInit(devNum, chipIndex);

    return GT_OK;
}

/**
* @internal mvHwsAvagoSpicoInterrupt function
* @endinternal
*
* @brief   Issue the interrupt to the Spico processor.
*         The return value is the interrupt number.
*
* @param[out] result                   - spico interrupt return value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSpicoInterrupt
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroup,
    IN  GT_U32 chipIndex,
    IN  GT_U32 sbusAddr,
    IN  GT_U32 interruptCode,
    IN  GT_U32 interruptData,
    OUT GT_32  *result
)
{
    portGroup = portGroup; /* Avoid Warnings */

    AVAGO_LOCK(devNum, chipIndex);
    if (result == NULL)
    {
        avago_spico_int(aaplSerdesDb[chipIndex], sbusAddr, interruptCode, interruptData);
    }
    else
    {
#ifndef BOBK_DEV_SUPPORT
        *result = SIGNED16_TO_SIGNED32(avago_spico_int(aaplSerdesDb[chipIndex], sbusAddr, interruptCode, interruptData));
#else
        *result = avago_spico_int(aaplSerdesDb[chipIndex], sbusAddr, interruptCode, interruptData);
#endif
    }
    AVAGO_UNLOCK(devNum, chipIndex);

    CHECK_AVAGO_RET_CODE();

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesSpicoInterrupt function
* @endinternal
*
* @brief   Issue the interrupt to the Spico processor.
*         The return value is the interrupt number.
*
* @param[out] result                   - spico interrupt return value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSpicoInterrupt
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroup,
    IN  GT_U32 serdesNum,
    IN  GT_U32 interruptCode,
    IN  GT_U32 interruptData,
    OUT GT_32  *result
)
{
    GT_U32  sbus_addr;
    unsigned int chipIndex;
    portGroup = portGroup; /* Avoid Warnings */

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    return mvHwsAvagoSpicoInterrupt(devNum,portGroup,chipIndex,sbus_addr,interruptCode,interruptData,result);
}

int temperatureGlobal = 0;
unsigned int timeInterval = 5000;
unsigned int lastTimeStamp = 0;

extern GT_U32 mvPortCtrlCurrentTs(void);

#ifndef ALDRIN_DEV_SUPPORT
/*******************************************************************************
*
* @brief   Get the Low Power mode from Avago Serdes
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - physical serdes number
*
* @param[out] mode - enable/disable low power mode
*
* @retval 0 - on success
* @retval 1 - on error
*/
int mvHwsAvagoSerdesLowPowerModeEnableGet
(
    unsigned char   devNum,
    unsigned int    portGroup,
    unsigned int    serdesNum,
    GT_BOOL         *mode
)
{
    GT_U32  sbus_addr;
    unsigned int chipIndex;
    /* avoid warning */
    portGroup = portGroup;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);

    if (hwsDeviceSpecInfo[devNum].serdesType != AVAGO_16NM)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("Serdes type is not supported"));
    }

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    AVAGO_LOCK(devNum, chipIndex);
    *mode = 0x70 == (avago_serdes_mem_rd(aaplSerdesDb[chipIndex], sbus_addr, AVAGO_ESB, 0x0f8) & 0x70);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();

    return GT_OK;
}
#endif
/*******************************************************************************
*
* @brief   Get the Temperature (in C) from Avago Serdes
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] chipIndex - Chip Index (Relevant for Falcon Only)
* @param[out] temperature - Serdes  degree value (In Celsius)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTemperatureGet
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  chipIndex,
    OUT GT_32  *temperature
)
{
#ifndef ASIC_SIMULATION
    unsigned int sensor_addr;

#if defined(MV_HWS_FREE_RTOS)
    GT_U32 miliSeconds=0;
    miliSeconds = mvPortCtrlCurrentTs();
    if ((miliSeconds > lastTimeStamp + timeInterval) || (miliSeconds < lastTimeStamp))
    {
#endif /* defined(MV_HWS_FREE_RTOS) */

        /* avoid warning */
        portGroup = portGroup;

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
            sensor_addr = 1; /* AVAGO TSEN0 in ring0 */
        }
        else if ((HWS_DEV_SILICON_TYPE(devNum) == Pipe) || (HWS_DEV_SILICON_TYPE(devNum) == Raven) || (HWS_DEV_SILICON_TYPE(devNum) == Falcon))
        {
            sensor_addr = 18;
        }
        else
        {
            return GT_NOT_SUPPORTED;
        }

        /* trying to get the Temperature in degrees maximum 10 times and set 1ms delay between each time */
        AVAGO_LOCK(devNum, chipIndex);
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT)
        if ((HWS_DEV_SILICON_TYPE(devNum) != Raven) && (HWS_DEV_SILICON_TYPE(devNum) != Falcon))
        {
            chipIndex = devNum;
        }
        temperatureGlobal = avago_sensor_get_temperature(aaplSerdesDb[chipIndex], sensor_addr, 0, 0);
        temperatureGlobal = temperatureGlobal/1000;
        /* avago_sensor_get_temperature return the value in milli Celsius while all system works in Celsius */
#else
        temperatureGlobal = avago_sbm_get_temperature(aaplSerdesDb[chipIndex], sensor_addr, 0);
#endif
        AVAGO_UNLOCK(devNum, chipIndex);
#if defined(MV_HWS_FREE_RTOS)
        lastTimeStamp = miliSeconds;
    }
#endif /* defined(MV_HWS_FREE_RTOS) */
    *temperature = temperatureGlobal;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/*******************************************************************************
* mvHwsAvagoSerdesCalCodeSet
*
* DESCRIPTION:
*       Set the calibration code(value) for Rx or Tx
*
* INPUTS:
*       devNum    - device number
*       portGroup - port group (core) number
*       serdesNum - SERDES number
*       mode      - True for Tx mode, False for Rx mode
*       therm     - Thermometer of VCO (0...0xFFFF)
*       bin       - bin of VCO (0...0xFFFF)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static unsigned int mvHwsAvagoSerdesCalCodeSet(int devNum, int portGroup, int serdesNum, BOOL mode, int therm, int bin)
{
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x4002 | (mode << 9), NULL));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, therm, NULL));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x4003 | (mode << 9), NULL));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, bin, NULL));

    return GT_OK;
}

/*******************************************************************************
* mvHwsAvagoSerdesCalCodeGet
*
* DESCRIPTION:
*       Get the calibration code(value) for Rx or Tx
*
* INPUTS:
*       devNum    - device number
*       portGroup - port group (core) number
*       serdesNum - SERDES number
*       mode      - True for Tx mode, False for Rx mode
*
* OUTPUTS:
*       therm - Thermometer of VCO  (0...0xFFFF)
*       bin   - bin of VCO  (0...0xFFFF)
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static unsigned int mvHwsAvagoSerdesCalCodeGet
(
    int     devNum,
    int     portGroup,
    int     serdesNum,
    BOOL    mode,
    GT_32     *therm,
    GT_32     *bin
)
{
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x4002 | (mode << 9), NULL));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x1A, 0, therm));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x4003 | (mode << 9), NULL));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x1A, 0, bin));

    return GT_OK;
}

/*******************************************************************************
* mvHwsAvagoCalCodeShift
*
* DESCRIPTION:
*       Shift the calcode according to the amount value
*
* INPUTS:
*       devNum    - device number
*       portGroup - port group (core) number
*       serdesNum - SERDES number
*       mode      - True for Tx mode, False for Rx mode
*       shift     - amount of cal-code shift
*       therm     - Thermometer of VCO  (0...0xFFFF)
*       bin       - bin of VCO  (0...0xFFFF)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static unsigned int mvHwsAvagoCalCodeShift
(
    int     devNum,
    int     portGroup,
    int     serdesNum,
    BOOL    mode,
    int     shift,
    int     therm,
    int     bin
)
{
    unsigned int    res;
    int             bin_portion;

    bin_portion = (bin & 0x7) + shift;

    therm = (bin_portion > 7) ? ((therm  <<1 )+1): therm;
    therm = (bin_portion < 0) ? (therm >> 1) : therm;

    bin_portion &=0x7;
    bin = (bin & 0xFFF8)| bin_portion;

    res = mvHwsAvagoSerdesCalCodeSet(devNum, portGroup, serdesNum, mode, therm, bin);
    if (res != GT_OK)
    {
        #ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
        hwsOsPrintf("mvHwsAvagoSerdesCalCodeSet failed (%d)\n", res);
        #endif
        return GT_FAIL;
    }

    return GT_OK;
}
#endif

#ifndef RAVEN_DEV_SUPPORT
/*******************************************************************************
* mvHwsAvagoSerdesShiftCalc
*
* DESCRIPTION:
*       Calculate the Shift value according to the Temperature
*
* INPUTS:
*       devNum      - device number
*       portGroup   - port group (core) number
*       temperature - Temperature (in C) from Avago Serdes
*
* OUTPUTS:
*       shift - Shift value for calibration code in Rx and Tx
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static unsigned int mvHwsAvagoSerdesShiftCalc
(
    unsigned char   devNum,
    unsigned int    portGroup,
    int             temperature,
    int             *shift
)
{
    *shift = 0;
    devNum = devNum; /* Avoid Warnings */
    portGroup = portGroup; /* Avoid Warnings */

    if (temperature < -20)
        *shift = 2;
    else if ((temperature >= -20) && (temperature <= 0))
        *shift = 1;
    else if ((temperature > 30) && (temperature <= 75))
        *shift = -1;
    else if (temperature > 75)
        *shift = -2;

    return GT_OK;
}
/*******************************************************************************
* mvHwsAvagoSerdesVcoConfig
*
* DESCRIPTION:
*       Compensate the VCO calibration value according to Temperature in order
*       to enable Itemp operation
*
* INPUTS:
*       devNum      - device number
*       portGroup   - port group (core) number
*       serdesNum   - SERDES number
*       temperature - Temperature (in C) from Avago Serdes
*       txTherm     - Tx Thermometer of VCO  (0...0xFFFF)
*       txBin       - Tx bin of VCO  (0...0xFFFF)
*       rxTherm     - Rx Thermometer of VCO  (0...0xFFFF)
*       rxBin       - Rx bin of VCO  (0...0xFFFF)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static unsigned int mvHwsAvagoSerdesVcoConfig
(
    unsigned char   devNum,
    unsigned int    portGroup,
    unsigned int    serdesNum,
    int     temperature,
    int     txTherm,
    int     txBin,
    int     rxTherm,
    int     rxBin
)
{
    int shift=0;
    unsigned int res;

    /* Calculate the Shift value according to the Temperature */
    res = mvHwsAvagoSerdesShiftCalc(devNum, portGroup, temperature, &shift);
    if (res != GT_OK)
    {
        #ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
        hwsOsPrintf("mvHwsAvagoSerdesShiftCalc failed (%d)\n", res);
        #endif
        return GT_FAIL;
    }

    /* Shift the calibration code for Tx */
    res = mvHwsAvagoCalCodeShift(devNum, portGroup, serdesNum, TRUE, shift, txTherm, txBin);
    if (res != GT_OK)
    {
        #ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
        hwsOsPrintf("mvHwsAvagoCalCodeShift failed (%d)\n", res);
        #endif
        return GT_FAIL;
    }

    /* shift the calibration code for Rx */
    res = mvHwsAvagoCalCodeShift(devNum, portGroup, serdesNum, FALSE, shift, rxTherm, rxBin);
    if (res != GT_OK)
    {
        #ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
        hwsOsPrintf("mvHwsAvagoCalCodeShift failed (%d)\n", res);
        #endif
        return GT_FAIL;
    }

    return GT_OK;
}
#endif
#endif /* ASIC_SIMULATION */

#ifndef MV_HWS_AVAGO_NO_VOS_WA

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsAvagoSerdesManualVosConfigSet function
* @endinternal
*
* @brief   Applying VOS correction parameters from eFuze to improve eye openning.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAvagoSerdesManualVosConfigSet
(
    unsigned char  devNum,
    unsigned int   serdesNum
)
{
    GT_U32 serdesVosValue;
    GT_U32 serdesVosArrIdx;
    GT_U32 dfxVosValue1;
    GT_U32 dfxVosValue2;
    GT_U32 vosIdx;

    /* In BobK there are 12 sequential serdeses applicable for VOS override. To save space, they are arrange
       in an array by they modulu index, 12 indexes from 0 to 11. */
    /* TBD_BOOKMARK_BOBCAT3 */
    if (HWS_DEV_SILICON_TYPE(devNum) == BobK)
    {
        serdesVosArrIdx = serdesNum%12;
    }
    else
    {
        serdesVosArrIdx = serdesNum;
    }

    /* Checking if the serdes is in the list of serdeses applicable for VOS override.
       If not, no VOS override is needed. */
    /* Non CM3 code */
#ifdef BOBK_DEV_SUPPORT
    if (serdesNum != serdesVosOffsetBobK[serdesVosArrIdx].serdes)
#else /* Not BobK */
/* BC3/PIPE TBD update VOS offset */
#if defined(ALDRIN_DEV_SUPPORT) /* || defined (BC3_DEV_SUPPORT) || defined (PIPE_DEV_SUPPORT) || defined (ALDRIN2_DEV_SUPPORT) */
    if (serdesNum != serdesVosOffsetAldrin[serdesVosArrIdx].serdes)
#endif /* defined(ALDRIN_DEV_SUPPORT) */
#endif /* defined(BOBK_DEV_SUPPORT) */
    {
        CPSS_LOG_INFORMATION_MAC("Not performing VOS config on serdes %d ", serdesNum);
        return GT_OK;
    }

    /* Getting VOS0-6 from the DB. For each serdes the values are divided into two group.
       one for VOS0-3 and one for VOS4-6, as they are stored in the DFX. */
    dfxVosValue1 = serdesVosParamsDB[(serdesVosArrIdx*2)];
    dfxVosValue2 = serdesVosParamsDB[(serdesVosArrIdx*2)+1];

    CPSS_LOG_INFORMATION_MAC("Serdes Num %d, vos0-3 32bits: 0x%08x, vos4-6 32bits: 0x%08x ", serdesNum, dfxVosValue1, dfxVosValue2);

    /* Checking for corrupted data in VOS0-3. If such exist, the operation can not be completed.
       corrupted data is considered as a value less than 0x60 */
    if ( ((dfxVosValue1 & 0xff) < 0x60) || (((dfxVosValue1 >> 8) & 0xff) < 0x60) ||
         (((dfxVosValue1 >> 16) & 0xff) < 0x60) || (((dfxVosValue1 >> 24) & 0xff) < 0x60) )
    {
        CPSS_LOG_INFORMATION_MAC("VOS override failed. Corrupted DFX value for VOS0-3: 0x%08x at db index: %d", dfxVosValue1, (serdesVosArrIdx*2));
        /* Returning GT_OK as lack of VOS values should not fail the serdes and the entire system */
        return GT_OK;
    }
    /* Checking for corrupted data in VOS4-6 */
    if ( ((dfxVosValue2 & 0xff) < 0x60) || (((dfxVosValue2 >> 8) & 0xff) < 0x60) ||
         (((dfxVosValue2 >> 16) & 0xff) < 0x60) )
    {
        CPSS_LOG_INFORMATION_MAC("VOS override failed. Corrupted DFX value for VOS4-6: 0x%08x at db index: %d", dfxVosValue1, (serdesVosArrIdx*2)+1);
        /* Returning GT_OK as lack of VOS values should not fail the serdes and the entire system */
        return GT_OK;
    }

    /* Calculating the VOS value for each VOS index */
    for (vosIdx = 0; vosIdx <=3 ; vosIdx++)
    {
        /* Calculating the generic formula */
        serdesVosValue = ( 0x1900 + (vosIdx * 0x100) );
        /* Adding the matching 8bits from the 32bit DFX value */
        serdesVosValue += ( ( dfxVosValue1 >> ( vosIdx * 8 ) ) & 0xff );
        /* Applying final VOS value to serdes */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, 0, serdesNum, 0x26, serdesVosValue, NULL));
        /* Log */
        CPSS_LOG_INFORMATION_MAC("Serdes Num %d, VOS index: %d, Value: 0x%08x ", serdesNum, vosIdx, serdesVosValue);
    }

    /* Calculating the VOS value for each VOS index */
    for (vosIdx = 4; vosIdx <=6 ; vosIdx++)
    {
        /* Calculating the generic formula */
        serdesVosValue = ( 0x1900 + (vosIdx * 0x100) );
        /* Adding the matching 8bits from the 32bit DFX value */
        serdesVosValue += ( (dfxVosValue2 >> ( (vosIdx-4) * 8) ) & 0xff );
        /* Applying final VOS value to serdes */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, 0, serdesNum, 0x26, serdesVosValue, NULL));
        /* Log */
        CPSS_LOG_INFORMATION_MAC("Serdes Num %d, VOS index: %d, Value: 0x%08x ", serdesNum, vosIdx, serdesVosValue);
    }

    return GT_OK;
}
#endif
#ifdef MV_HWS_REDUCED_BUILD_EXT_CM3
/**
* @internal mvHwsAvagoSerdesManualVosParamsSet function
* @endinternal
*
* @brief   Set the VOS Override parameters in to the DB. Used to manually set the
*         VOS override parameters instead of using the eFuze burned values.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualVosParamsSet
(
    unsigned char  devNum,
    unsigned long  *vosParamsPtr,
    unsigned long  bufferIdx,
    unsigned long  bufferLength
)
{
    GT_U32 offset;
    GT_U32 baseOffset;

    /* Calculating the offset of the VOS param DB in which the given buffer should be written */
    baseOffset = bufferLength * bufferIdx;

    /* Null pointer check */
    if (vosParamsPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)
    /* Boundary check */
    if ( (baseOffset+bufferLength) > (sizeof(serdesVosParams64BitDB)/sizeof(GT_U32)) )
    {
        return GT_BAD_PARAM;
    }
#else
    /* Boundary check */
    if ( (baseOffset+bufferLength) > (sizeof(serdesVosParamsDB)/sizeof(GT_U32)) )
    {
        return GT_BAD_PARAM;
    }
#endif

#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)
    baseOffset = baseOffset >> 1;
#endif /* BC3_DEV_SUPPORT */

    /* Writing the values */
    for (offset = 0; offset < bufferLength; offset++)
    {
#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)
        if ((HWS_DEV_SILICON_TYPE(devNum) ==  Bobcat3 ) || (HWS_DEV_SILICON_TYPE(devNum) ==  Aldrin2 ) ||
                (HWS_DEV_SILICON_TYPE(devNum) ==  Pipe ))
        {

            if (offset*2 >= bufferLength)
            {
                break;
            }

            /* 64bit per element */
            serdesVosParams64BitDB[baseOffset + offset] = (GT_UL64)(((GT_UL64)vosParamsPtr[(offset*2)+1]) << 32 | ((GT_UL64)(vosParamsPtr[offset*2]) & 0xFFFFFFFF));
        }
        else
#endif /* defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) */
        {
            /* 32bit per element */
            serdesVosParamsDB[baseOffset + offset] = vosParamsPtr[offset];
        }
    }

#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)
    serdesVos64BitDbInitialized = GT_TRUE;
#endif
    serdesVosParamsDBInitialized = GT_TRUE;

    return GT_OK;
}
#ifndef ALDRIN_DEV_SUPPORT
GT_STATUS mvHwsAvagoSerdesManualVosMinAndBaseForBc3ParamsSet
(
    unsigned char  devNum,
    unsigned long  minVosValueToSet,
    unsigned long  baseVosValueToSet
)
{
    minVosValue = minVosValueToSet;
    baseVosValue = baseVosValueToSet;

    return GT_OK;
}
#endif
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

/**
* @internal mvHwsAvagoSerdesManualVosParamsDBInit function
* @endinternal
*
* @brief   Init the VOS override DB from the DFX.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualVosParamsDBInit
(
    unsigned char  devNum
)
{
    GT_U32 dfxVosAddress1;
    GT_U32 dfxVosAddress2;
    GT_U32 arrSize;
    GT_U32 index;
    GT_U32 dfxVosVal=0;
    GT_U32 eFuseVosRegion;
    HWS_AVAGO_SERDES_VOS_OFFSET * serdesVosOffsetArrayPtr;

    /* Setting the starting serdes and end serdes, eFuse region address and temp array pointer
       according to device type */
/* Code for Non CM3 CPU (host) and for Bobk CM3  */
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && defined(BOBK_DEV_SUPPORT))
    if (HWS_DEV_SILICON_TYPE(devNum) == BobK)
    {
        eFuseVosRegion = SERVER_HD_EFUSE_VOS_REGION_BOBK;
        serdesVosOffsetArrayPtr = serdesVosOffsetBobK;
        arrSize = 12;
    }
    else
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && defined(BOBK_DEV_SUPPORT)) */
/* Code for Non CM3 CPU (host) and for Aldrin CM3  */
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && defined(ALDRIN_DEV_SUPPORT))
    if (HWS_DEV_SILICON_TYPE(devNum) == Aldrin)
    {
        eFuseVosRegion = SERVER_HD_EFUSE_VOS_REGION_ALDRIN;
        serdesVosOffsetArrayPtr = serdesVosOffsetAldrin;
        arrSize = 32;
    }
    else
#endif /* #if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && defined(BOBK_DEV_SUPPORT)) */
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("silicon type not supported"));
    }

    /* For each serdes, reading the values from the DFX and writing to the DB */
    for (index=0; index<arrSize; index++)
    {
        /* Total DFX address calculation for VOS0-3 is:
           DFX eFuze addess + VOS region + serdes offset */
        dfxVosAddress1 = (SERVER_HD_EFUSE_SLAVE_REG + eFuseVosRegion + serdesVosOffsetArrayPtr[index].serdesVosOffset);
        /* Reading 32bit VOS0-VOS3 value for the serdes from DFX */
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, dfxVosAddress1, &dfxVosVal));
        serdesVosParamsDB[index*2] = dfxVosVal;
        /* Total DFX address calculation for VOS4-6 is:
           DFX eFuze addess + VOS region + serdes offset + 4 */
        dfxVosAddress2 = (SERVER_HD_EFUSE_SLAVE_REG + eFuseVosRegion + serdesVosOffsetArrayPtr[index].serdesVosOffset + 4);
        /* Reading 32bit VOS4-VOS6 value for the serdes from DFX. */
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, dfxVosAddress2, &dfxVosVal));
        serdesVosParamsDB[(index*2)+1] = dfxVosVal;
    }

    serdesVosParamsDBInitialized = GT_TRUE;

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesManualVosParamsGet function
* @endinternal
*
* @brief   Get the VOS Override parameters from the local DB.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualVosParamsGet
(
    unsigned char  devNum,
    unsigned long  *vosParamsPtr
)
{
    /* Null pointer check */
    if (vosParamsPtr==NULL)
    {
        return GT_BAD_PARAM;
    }
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    if ((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) ||
        (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
    {
        if (!serdesVos64BitDbInitialized)
        {
            CHECK_STATUS(mvHwsAvagoSerdesBc3VosConfig(devNum));
        }
    }
    else
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) */
    {
        /* Initializing VOS db if not initialized by now */
        if (!serdesVosParamsDBInitialized)
        {
            CHECK_STATUS(mvHwsAvagoSerdesManualVosParamsDBInit(devNum));
        }
    }

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    /* Copying values from our local DB */
    if ((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
    {
        hwsOsMemCopyFuncPtr(vosParamsPtr, serdesVosParams64BitDB, sizeof(serdesVosParams64BitDB));
    }
    else
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) */
    {
        hwsOsMemCopyFuncPtr(vosParamsPtr, serdesVosParamsDB, sizeof(serdesVosParamsDB));
    }

    return GT_OK;
}
#endif /*#ifndef MV_HWS_AVAGO_NO_VOS_WA*/

static GT_STATUS mvHwsAvagoSerdesArrayPowerCtrlCfg
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroup,
    IN GT_UOPT                      numOfSer,
    IN GT_UOPT                      *serdesArr,
    IN GT_U32                       divider,
    IN MV_HWS_SERDES_CONFIG_STC     *serdesConfigPtr,
    Avago_serdes_init_config_t      *configDef

)
{
    Avago_serdes_init_config_t *config;
    GT_UOPT i;

    /* Serdes PowerUp */
    /* Initialize the SerDes Avago_serdes_init_config_t struct */
    avago_serdes_init_config(configDef);
    config = configDef;
    config->signal_ok_threshold = 2;
    config->tx_divider = config->rx_divider = divider;
    config->skip_crc = 1;

#if !defined(BOBK_DEV_SUPPORT) && !defined(ALDRIN_DEV_SUPPORT)
    if ( serdesConfigPtr->serdesType == AVAGO_16NM )
    {
        if (serdesConfigPtr->encoding == SERDES_ENCODING_PAM4)
        {
            config->rx_encoding = config->tx_encoding = AVAGO_SERDES_PAM4;
        }
        else /* SERDES_ENCODING_AVAGO_NRZ */
        {
            config->rx_encoding = config->tx_encoding = AVAGO_SERDES_NRZ;
        }
        config->init_mode = AVAGO_CORE_DATA_ELB/*AVAGO_PRBS31_ILB*/;

        /* In Raven need to enable the Tx output in Serdes level
           since in previous device it is done in port level    */
        config->tx_output_en = FALSE;

        /* signal OK threshold is not reliable - need to be disabled  */
        config->signal_ok_en = TRUE;
    }
#endif

    if (serdesConfigPtr->serdesType == AVAGO)
    {
        if (serdesConfigPtr->encoding == SERDES_ENCODING_PAM4)
        {
            AVAGO_DBG(("mvHwsAvagoSerdesArrayPowerCtrlImpl: for Avago Serdes 28nm only SERDES_ENCODING_AVAGO_NRZ mode is supported\n"));
            return GT_BAD_PARAM;
        }
        config->init_mode = AVAGO_INIT_ONLY;
        config->tx_output_en = FALSE;
        config->rx_encoding = config->tx_encoding = AVAGO_SERDES_NRZ;
    }

#ifdef MICRO_INIT
    config->tx_output_en = TRUE;
#endif

    config->tx_phase_cal = TRUE;

    /* Select the Rx & Tx data path width */
    if (serdesConfigPtr->busWidth == _10BIT_ON)
    {
        config->rx_width = config->tx_width = 10;

        /* Save the width setting for PRBS test in AVAGO_SD_METAL_FIX register.
           It is needed for GPCS modes: the PRBS test can run in these modes only with Rx/Tx width=20BIT.
           If the Rx/Tx width=10BIT then it will be changed to 20BIT in mvHwsAvagoSerdesTestGen function */
        for (i = 0; i < numOfSer; i++)
        {
            CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], AVAGO_SD_METAL_FIX, (1 << 9), (1 << 9)));
        }
    }
    else if (serdesConfigPtr->busWidth == _20BIT_ON) config->rx_width = config->tx_width = 20;
    else if (serdesConfigPtr->busWidth == _40BIT_ON) config->rx_width = config->tx_width = 40;
    else if (serdesConfigPtr->busWidth == _80BIT_ON) config->rx_width = config->tx_width = 80;

    AVAGO_DBG(("\nmvHwsAvagoSerdesArrayPowerCtrlImpl init_configuration:\n"));
    AVAGO_DBG(("   sbus_reset = %x \n",config->sbus_reset));
    AVAGO_DBG(("   spico_reset = %x \n",config->spico_reset));
    AVAGO_DBG(("   init_mode= %x \n",config->init_mode));
    AVAGO_DBG(("   init_tx = %x \n",config->init_tx));
    AVAGO_DBG(("   init_rx = %x \n",config->init_rx));
    AVAGO_DBG(("   tx_divider = 0x%x \n",config->tx_divider));
    AVAGO_DBG(("   rx_divider = 0x%x \n",config->rx_divider));
    AVAGO_DBG(("   tx_width = 0x%x \n",config->tx_width));
    AVAGO_DBG(("   rx_width = 0x%x \n",config->rx_width));
    AVAGO_DBG(("   tx_phase_cal = %x \n",config->tx_phase_cal));
    AVAGO_DBG(("   tx_output_en = %x \n",config->tx_output_en));
    AVAGO_DBG(("   signal_ok_en = %x \n",config->signal_ok_en));
    AVAGO_DBG(("   signal_ok_threshold= %x \n",config->signal_ok_threshold));
    AVAGO_DBG(("   tx_encoding= %x \n",config->tx_encoding));
    AVAGO_DBG(("   rx encoding= %x \n",config->tx_encoding));

    return GT_OK;
}



/**
 * mvHwsAvagoSerdesArrayPowerCtrlImplStub
 *
 *
 * @param devNum
 * @param portGroup
 * @param numOfSer
 * @param serdesArr
 * @param powerUp
 * @param divider
 * @param serdesConfigPtr
 * @param config
 * @param isGenericPwcl
 *
 * @return GT_STATUS
 */
static GT_STATUS mvHwsAvagoSerdesArrayPowerCtrlImplStub
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroup,
    IN GT_UOPT                      numOfSer,
    IN GT_UOPT                      *serdesArr,
    IN GT_U8                        powerUp,
    IN MV_HWS_SERDES_CONFIG_STC     *serdesConfigPtr,
    IN Avago_serdes_init_config_t   *config,
    IN GT_BOOL                      isGenericPwcl
)
{
   GT_U32 sbus_addr;
    unsigned int errors;
    GT_32 data;
    GT_UREG_DATA data_ureg;
    GT_UOPT i;
#if defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    GT_U32  tryCount=0;
#endif

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BOBK_DEV_SUPPORT)
    GT_U32 sdOffset = 0;
    GT_U32 macOffset = 0;
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    GT_UOPT j;
#endif
#endif

#ifndef RAVEN_DEV_SUPPORT
    unsigned int ctleBitMapIndex = 0;
    GT_U32 ctleBias;
    GT_U8 numOfLanes;
    GT_U32 t; /* timeout iterator */
    GT_BOOL pllStable;
    GT_STATUS rc;
#endif
    int st;
    unsigned int chipIndex;

    if(serdesConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)

    if (prvCpssSystemRecoveryInProgress())
    {
        return GT_OK;
    }
#endif

    /* Get chip number to fill AAPL array */
    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesArr[0]);

    /* for Serdes PowerDown */
    if (powerUp == GT_FALSE)
    {
        /* if Training is in progress give it chance to finish and turn serdes off anyway */
        AVAGO_LOCK(devNum, chipIndex);
        for (i = 0; i < numOfSer; i++)
        {
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
            for (j=0; j<25; j++)
            {
                st = hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], AVAGO_CORE_STATUS, &data_ureg, 0x2);
                if (st != GT_OK)
                {
                    AVAGO_UNLOCK(devNum, chipIndex);
                    CHECK_STATUS(st);
                }
                if (data_ureg != 0)
                {
                    ms_sleep(20);
                }
                else
                {
                    break;
                }
            }
#else
            do
            {
               st = hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], AVAGO_CORE_STATUS, &data_ureg, 0x2);
               if (st != GT_OK)
               {
                   AVAGO_UNLOCK(devNum, chipIndex);
                   CHECK_STATUS(st);
               }
            }while ((data_ureg != 0) && (tryCount++ < 5));
#endif
            /* turn serdes off */
            st = mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesArr[i], &sbus_addr);
            if (st != GT_OK)
            {
                AVAGO_UNLOCK(devNum, chipIndex);
                CHECK_STATUS(st);
            }

            /* 0x7 , 0x0
            disable the spico and return its parameters to default. */
            avago_sbus_wr(aaplSerdesDb[chipIndex], sbus_addr, 0x7, 0x0);

            /* 0x7 , 0x2
            enable the spico which is now at init state (like after loading the spico FW). */
            avago_sbus_wr(aaplSerdesDb[chipIndex], sbus_addr, 0x7, 0x2);

            avago_serdes_set_tx_rx_enable(aaplSerdesDb[chipIndex], sbus_addr, GT_FALSE, GT_FALSE, GT_FALSE);
            if (aaplSerdesDb[chipIndex]->return_code != 0)
            {
                aaplSerdesDb[chipIndex]->return_code = 0;
            }
        }
        AVAGO_UNLOCK(devNum, chipIndex);

        /* TxClk/RxClk Tlat_en=0. Return to default */
        for (i = 0; i < numOfSer; i++)
        {
            CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], SERDES_EXTERNAL_CONFIGURATION_0, 0, (3 << 13)));
        }

        /* Read the saved value from AVAGO_SD_METAL_FIX register to check the Rx/Tx width mode */
        for (i = 0; i < numOfSer; i++)
        {
            CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], AVAGO_SD_METAL_FIX, &data_ureg, 0xFFFF));
            if ((data_ureg >> 9) & 0x1)
            {
                /* Set back the value of AVAGO_SD_METAL_FIX register bit #9 if it was changed in 10BIT_ON mode for GPCS modes */
                CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], AVAGO_SD_METAL_FIX, (0 << 9), (1 << 9)));
            }
        }

        /* Serdes Reset */
        for (i = 0; i < numOfSer; i++)
        {
            CHECK_STATUS(mvHwsAvagoSerdesResetImpl(devNum, portGroup, serdesArr[i], GT_TRUE, GT_TRUE, GT_TRUE));
        }

        return GT_OK;
    }

/* Moved to higher function */

    /* Serdes Analog Un Reset*/
    for (i = 0; i < numOfSer; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesResetImpl(devNum, portGroup, serdesArr[i], GT_FALSE, GT_TRUE, GT_TRUE));
    }
    /* config media */
    data = (serdesConfigPtr->media == RXAUI_MEDIA) ? (1 << 2) : 0;
    for (i = 0; i < numOfSer; i++)
    {
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], SERDES_EXTERNAL_CONFIGURATION_0, data, (1 << 2)));
    }
    /* Reference clock source */
    data = ((serdesConfigPtr->refClockSource == PRIMARY) ? 0 : 1) << 8;
    for (i = 0; i < numOfSer; i++)
    {
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], SERDES_EXTERNAL_CONFIGURATION_0, data, (1 << 8)));
    }
    /* TxClk/RxClk Tlat_en=1. The logic of the Tlat need to be 1 in order to enable Clk to MAC/PCS regardless of Serdes readiness */
    for (i = 0; i < numOfSer; i++)
    {
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], SERDES_EXTERNAL_CONFIGURATION_0, (3 << 13), (3 << 13)));
    }
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT)
    if (serdesConfigPtr->serdesType == AVAGO_16NM)
    {
        Avago_addr_t addr_struct[MV_HWS_MAX_LANES_NUM_PER_PORT];

        for (i = 0; i < numOfSer; i++)
        {   /* build broadcast group */
            CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesArr[i], &sbus_addr));
            avago_addr_to_struct(sbus_addr,&addr_struct[i]);
            if ((i+1)<numOfSer)
            {
                addr_struct[i].next = &addr_struct[i+1];
            }
            else
            {
                addr_struct[i].next = 0;
            }
        }
        AVAGO_LOCK(devNum, chipIndex);
        errors = avago_parallel_serdes_init(aaplSerdesDb[chipIndex],&addr_struct[0],config);
        if ((errors > 0) || (aaplSerdesDb[chipIndex]->return_code != 0))
        {

            AVAGO_UNLOCK(devNum, chipIndex);
            CHECK_AVAGO_RET_CODE();
#ifndef MV_HWS_REDUCED_BUILD
            AVAGO_DBG(("SerDes init complete for SerDes at addr %s; Errors in ILB: %d. \n", aapl_addr_to_str(sbus_addr), errors));
#else
            AVAGO_DBG(("SerDes init complete for SerDes at addr 0x%x; Errors in ILB: %d. \n", sbus_addr, errors));
#endif /* MV_HWS_REDUCED_BUILD */
        }
        else
        {
#ifndef MV_HWS_REDUCED_BUILD
            AVAGO_DBG(("The SerDes at address %s is initialized.\n", aapl_addr_to_str(sbus_addr)));
#else
            AVAGO_DBG(("The SerDes at address 0x%x is initialized.\n", sbus_addr));
#endif /* MV_HWS_REDUCED_BUILD */
        }
        AVAGO_UNLOCK(devNum, chipIndex);
    }
    else /* AVAGO_16NM */
#endif
    {
        AVAGO_LOCK(devNum, chipIndex);
        for (i = 0; i < numOfSer; i++)
        {
            st = mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesArr[i], &sbus_addr);
            if (st != GT_OK)
            {
                AVAGO_UNLOCK(devNum, chipIndex);
                CHECK_STATUS(st);
            }

            errors = avago_serdes_init(aaplSerdesDb[chipIndex], sbus_addr, config);
            if ((errors > 0) || (aaplSerdesDb[chipIndex]->return_code != 0))
            {
                AVAGO_UNLOCK(devNum, chipIndex);
                CHECK_AVAGO_RET_CODE();
#ifndef MV_HWS_REDUCED_BUILD
                AVAGO_DBG(("SerDes init complete for SerDes at addr %s; Errors in ILB: %d. \n", aapl_addr_to_str(sbus_addr), errors));
#else
                AVAGO_DBG(("SerDes init complete for SerDes at addr 0x%x; Errors in ILB: %d. \n", sbus_addr, errors));
#endif /* MV_HWS_REDUCED_BUILD */
            }
            else
            {
#ifndef MV_HWS_REDUCED_BUILD
                AVAGO_DBG(("The SerDes at address %s is initialized.\n", aapl_addr_to_str(sbus_addr)));
#else
                AVAGO_DBG(("The SerDes at address 0x%x is initialized.\n", sbus_addr));
#endif /* MV_HWS_REDUCED_BUILD */
            }
        }
        AVAGO_UNLOCK(devNum, chipIndex);
    }

#if !defined(BOBK_DEV_SUPPORT) && !defined(ALDRIN_DEV_SUPPORT)
    /* It is not needed for Generic power control */
    if (hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM && !isGenericPwcl )
    {
#if 0
        /* let the serdes run in PRBs ILB for 5 ms before ILB undo */
        ms_sleep(5);
        AVAGO_LOCK(devNum, chipIndex);
        for (i = 0; i < numOfSer; i++)
        {
            st = mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesArr[i], &sbus_addr);
            if (st != GT_OK)
            {
                AVAGO_UNLOCK(devNum, chipIndex);
                CHECK_STATUS(st);
            }
            /* Sets the TX data source and disable PRBS */
            avago_serdes_set_tx_data_sel(aaplSerdesDb[chipIndex], sbus_addr, AVAGO_SERDES_TX_DATA_SEL_CORE);
            avago_serdes_set_rx_cmp_data(aaplSerdesDb[chipIndex], sbus_addr, AVAGO_SERDES_RX_CMP_DATA_OFF);
            /* disable ILB */
            avago_serdes_set_rx_input_loopback(aaplSerdesDb[chipIndex], sbus_addr, FALSE);
        }
        AVAGO_UNLOCK(devNum, chipIndex);
        CHECK_AVAGO_RET_CODE();
#endif

        if(serdesConfigPtr->baudRate != _1_25G)
        {
            for (i = 0; i < numOfSer; i++)
            {
                /* operate SerDes in low power mode */
                CHECK_STATUS(mvHwsAvagoSerdesLowPowerModeEnable(devNum, portGroup, serdesArr[i], GT_TRUE));
            }
        }

    }
#endif

#ifndef RAVEN_DEV_SUPPORT /* for RAVEN (cm3) - this code is done in mvHwsAvagoSerdesArrayPowerCtrlImplPhase */
    if(hwsDeviceSpecInfo[devNum].serdesType == AVAGO)
    {
#if !defined(ASIC_SIMULATION)
        /*
           By default the VCO calibration is operated on Avago Serdes.
           To bypass the VCO calibration for AP port, change the value of
           apPortGroup parameter in mvHwsPortCtrlApEng.c:
           apPortGroup = AVAGO_SERDES_INIT_BYPASS_VCO
        */
        if (portGroup != AVAGO_SERDES_INIT_BYPASS_VCO)
        {
            int     res;
            GT_32     temperature;
            BOOL    mode;  /* True for Tx mode, False for Rx mode */
            GT_32    txTherm, txBin, rxTherm, rxBin;

            /* get the Avago Serdes Temperature (in C) */
            CHECK_STATUS(mvHwsAvagoSerdesTemperatureGet(devNum, portGroup, chipIndex, &temperature));

            mode = GT_TRUE;
            /* get the calibration code(value) for Tx */
            CHECK_STATUS(mvHwsAvagoSerdesCalCodeGet(devNum, portGroup, serdesArr[0], mode, &txTherm, &txBin));

            mode = GT_FALSE;
            /* get the calibration code(value) for Rx */
            CHECK_STATUS(mvHwsAvagoSerdesCalCodeGet(devNum, portGroup, serdesArr[0], mode, &rxTherm, &rxBin));

            /* Compensate the VCO calibration value according to Temperature */
            for (i = 0; i < numOfSer; i++)
            {
                res = mvHwsAvagoSerdesVcoConfig(devNum, portGroup, serdesArr[i], temperature, txTherm, txBin, rxTherm, rxBin);
                if (res != GT_OK)
                {
                    AVAGO_DBG(("mvHwsAvagoSerdesVcoConfig failed (%d)\n", res));
                    return GT_FAIL;
                }
            }
        }
#endif /* !defined(ASIC_SIMULATION)*/

        /* Disable TAP1 before Training */
        for (i = 0; i < numOfSer; i++)
        {
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x26, TAP1_AVG_DISABLE, NULL));
        }

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BOBK_DEV_SUPPORT)
        /* calculate the bitMap index */
        if ((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (serdesArr[0] >= 20))
        {
            /* get the mac number */
            macOffset = hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasMacNum;
            ctleBitMapIndex = macOffset/AVAGO_CTLE_BITMAP;
            /* update SerDes offset */
            sdOffset = macOffset - (ctleBitMapIndex*AVAGO_CTLE_BITMAP);
        }
        else
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BOBK_DEV_SUPPORT) */
        {
            /* calculate CTLE Bias bitMap index */
            ctleBitMapIndex = serdesArr[0]/AVAGO_CTLE_BITMAP;
        }

        if ((((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (serdesArr[0] <20))) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin)
            || (HWS_DEV_SILICON_TYPE(devNum) == Pipe) || (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3)|| (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2))
        {
            numOfLanes = hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasLanesNum;
            if ((numOfLanes == 4) || (numOfLanes == 2))
            {
                CHECK_STATUS(mvHwsAvagoSerdesMultiLaneCtleBiasConfig(devNum, serdesArr[0], numOfLanes, ctleBitMapIndex, &ctleBias));
            }
            else
            {
                /* get CTLE Bias value */
                ctleBias = (((hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasValue[ctleBitMapIndex]) >> (serdesArr[0]%AVAGO_CTLE_BITMAP)) & 0x1);
            }
        }
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BOBK_DEV_SUPPORT)
        else if (((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (serdesArr[0] >=20)))
        {
            numOfLanes = hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasLanesNum;
            if ((numOfLanes == 4) || (numOfLanes == 2))
            {
                CHECK_STATUS(mvHwsAvagoSerdesMultiLaneCtleBiasConfig(devNum, sdOffset, numOfLanes, ctleBitMapIndex, &ctleBias));
            }
            else
            {
                /* get CTLE Bias value */
                ctleBias = (((hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasValue[ctleBitMapIndex]) >> (sdOffset%AVAGO_CTLE_BITMAP)) & 0x1);
            }
        }
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BOBK_DEV_SUPPORT) */
        else
        {
            /* get CTLE Bias value */
            ctleBias = (((hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasValue[ctleBitMapIndex]) >> (serdesArr[0] % AVAGO_CTLE_BITMAP)) & 0x1);
        }

        /* change CTLE bias from 1 to 0 */
        for (i = 0; i < numOfSer; i++)
        {
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x18, 0x4033, NULL));

            /* read the CTLE bias */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x1A, 0x0, &data));

            if (ctleBias)
            {
                /* set Ctle Bias to 1 */
                data |= 0x1000;
            }
            else
            {
                /* set Ctle Bias to 0 */
                data &= 0xCFFF;
            }

            /* apply CTLE Bias parameter */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, data, NULL));
        }


#if !defined(MV_HWS_AVAGO_NO_VOS_WA) || ( defined(MICRO_INIT) && !defined (BOBK_DEV_SUPPORT) )
        /* Configure serdes VOS parameters*/
        if (hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesVosOverride)
        {
            /* VOS Override parameters should be configured on ports with speed >= 10G, so filtering is needed.
               Here in serdes level, we can detect the speed by examine the 'mode' and 'baudRate' parameters*/
            /* First, all of the 10G and above speeds have 20bit and 40bit mode */
            if (serdesConfigPtr->busWidth == _20BIT_ON || serdesConfigPtr->busWidth == _40BIT_ON)
            {
                /* Second, some of the speeds less than 10G also have 20bit mode, so baudRate filtering needed */
                if (serdesConfigPtr->baudRate != _5_15625G && serdesConfigPtr->baudRate != _5G)
                {
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)
                    /* for BC3 only 10G baudRate ports */
                    if ((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
                    {
                        if (_10_3125G == serdesConfigPtr->baudRate)
                        {
                            for (i = 0; i < numOfSer; i++)
                            {
                                CHECK_STATUS(mvHwsAvagoSerdesVosDfxParamsSet(devNum,serdesArr[i]));
                            }
                        }
                    }
                    else
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) */
                    {
                        for (i = 0; i < numOfSer; i++)
                        {
                            CHECK_STATUS(mvHwsAvagoSerdesManualVosConfigSet(devNum, serdesArr[i]));
                        }
                    }
                }
            }
        }
#endif /* MV_HWS_AVAGO_NO_VOS_WA */

        /* implement SW WA for linkDown problem: release the Spico driver of Serdes */
        if ((serdesConfigPtr->baudRate == _1_25G)  ||
            (serdesConfigPtr->baudRate == _5G)     ||
            (serdesConfigPtr->baudRate == _3_125G) ||
            (serdesConfigPtr->baudRate == _10_3125G))
        {
            for (i = 0; i < numOfSer; i++)
            {
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x18, 0x8000, NULL));
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, 0x2, NULL));
            }
        }

#ifdef MV_HWS_FREE_RTOS /* for AP port only */
        if (serdesConfigPtr->baudRate == _3_125G)
        {
            for (i = 0; i < numOfSer; i++)
            {   /* Improve the CDR lock process */
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x18, 0x40FB, NULL));
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, 0x1200, NULL));
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x18, 0x40A9, NULL));
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, 0x1100, NULL));
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x18, 0x4001, NULL));
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, 0x1FF,  NULL));
            }
        }
#endif
        /* TxRx tune is not needed for Generic */
        if ( !isGenericPwcl) {
            for (i = 0; i < numOfSer; i++)
            {
                CHECK_STATUS(hwsAvagoSerdesTxRxTuneParamsSet(devNum, portGroup, serdesArr[i],
                                                             serdesConfigPtr));
            }
        }

        /* Verify PLLs are ready */
        for (i = 0; i < numOfSer; i++)
        {
           for (t = 0; t < MV_HWS_AVAGO_STABLE_PLL_TIMEOUT; t++) /* Wait for PLL to stablize*/
            {
                pllStable = GT_FALSE;
                rc = mvHwsAvagoPllStableGet(devNum,portGroup,serdesArr[i],&pllStable);
                if (((GT_OK == rc) && (pllStable == GT_TRUE)) || (GT_NOT_IMPLEMENTED == rc))
                {
                    pllStable = GT_TRUE;
                    break;
                }
                /* Delay */
                hwsOsExactDelayPtr(devNum, portGroup, 5);
            }

            /* in case pll is not stable */
            if ((pllStable != GT_TRUE) || (t == MV_HWS_AVAGO_STABLE_PLL_TIMEOUT))
            {
                return GT_TIMEOUT;
            }
        }
    }
    else /* (hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM) */
    {
        /* TxRx tune is not needed for Generic power control */
        if ( !isGenericPwcl) {
            for (i = 0; i < numOfSer; i++)
            {
                CHECK_STATUS(hwsAvagoSerdesTxRxTuneParamsSet(devNum, portGroup, serdesArr[i],
                                                             serdesConfigPtr));
            }
        }

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
        /* It is not needed for Generic power control */
        if (!isGenericPwcl){
            for (i = 0; i < numOfSer; i++)
            {
                /* enable EID Filter
               CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x2c, 0x11c, NULL));
               CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x6c, 0xff1, NULL));
               */
               CHECK_STATUS(mvHwsAvagoSignalDetectInvert(devNum, portGroup, serdesArr[i], GT_TRUE));
            }
        }
#endif /* #if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT) */
    }
    /* Serdes Digital UnReset */
    for (i = 0; i < numOfSer; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesResetImpl(devNum, portGroup, serdesArr[i], GT_FALSE, GT_FALSE, GT_FALSE));
    }
#endif
    return GT_OK;
}


GT_STATUS mvHwsAvagoSerdesArrayPowerCtrlImpl
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_UOPT     numOfSer,
    IN GT_UOPT     *serdesArr,
    IN GT_U8       powerUp,
    IN GT_U32      divider,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    Avago_serdes_init_config_t configDef;

    CHECK_STATUS(mvHwsAvagoSerdesArrayPowerCtrlCfg(devNum,portGroup,numOfSer,serdesArr,divider,serdesConfigPtr,&configDef));

    CHECK_STATUS(mvHwsAvagoSerdesArrayPowerCtrlImplStub(devNum,portGroup,numOfSer,serdesArr,powerUp,serdesConfigPtr,&configDef,GT_FALSE));

    return GT_OK;
}

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)

/**
 * @internal mvHwsAvagoSerdesGenericPowerCtrl function
 * @endinternal
 *
 * @brief SerDes debug function
 *
 * @param devNum
 * @param portGroup
 * @param serdesNum
 * @param refclockSrc
 * @param powerUp
 * @param ref_clk_div
 * @param divider
 * @param busWidth
 * @param encoding
 * @param swizzle
 * @param precoding
 * @param gray
 * @param squelchLevel
 * @param rxPolarity
 * @param txPolarity
 * @param initRx
 * @param initTx
 * @param enabletTxOut
 * @param ILB
 * @param PRBS31
 *
 * @return GT_STATUS
 */
GT_STATUS mvHwsAvagoSerdesGenericPowerCtrl
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroup,
    IN GT_U32                       serdesNum,
    IN MV_HWS_REF_CLOCK_SRC         refclockSrc,
    IN GT_U8                        powerUp,
    IN GT_U32                       ref_clk_div,
    IN GT_U32                       divider,
    IN MV_HWS_SERDES_BUS_WIDTH_ENT  busWidth,
    IN MV_HWS_SERDES_ENCODING_TYPE  encoding,
    IN GT_BOOL                      swizzle,
    IN GT_BOOL                      precoding,
    IN GT_BOOL                      gray,
    IN GT_U32                       squelchLevel,
    IN GT_BOOL                      rxPolarity,
    IN GT_BOOL                      txPolarity,
    IN GT_BOOL                      initRx,
    IN GT_BOOL                      initTx,
    IN GT_BOOL                      enabletTxOut,
    IN GT_BOOL                      ilb,
    IN GT_BOOL                      prbs31
)
{
    Avago_serdes_init_config_t          configDef;
    Avago_serdes_init_config_t          *config;
    MV_HWS_SERDES_CONFIG_STC            serdesConfig;
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT    rxConfig;
    GT_UOPT serdesArr[1];

    serdesArr[0] = serdesNum;
    serdesConfig.refClockSource = refclockSrc;
    serdesConfig.busWidth   = busWidth;
    serdesConfig.media      = RXAUI_MEDIA;

    if ( SERDES_ENCODING_NRZ ==  encoding && (swizzle == GT_TRUE || precoding == GT_TRUE  || gray == GT_TRUE ) ) {
          return GT_FAIL;
    }

    if (encoding == SERDES_ENCODING_PAM4) {
        serdesConfig.encoding = AVAGO_SERDES_PAM4;
    } else {/* SERDES_ENCODING_NRZ */
        serdesConfig.encoding = AVAGO_SERDES_NRZ;
    }

    serdesConfig.serdesType = (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum);

    /** Initialize the SerDes Avago_serdes_init_config_t struct */

    avago_serdes_init_config(&configDef);
    config = &configDef;

    config->signal_ok_threshold = 2;
    config->tx_divider = config->rx_divider = divider;
    config->skip_crc = 1;
    config->rx_encoding = config->tx_encoding = serdesConfig.encoding;

    /** Configure datapath mask:
       < 0x01 = polarity_invert, 0x02 = gray_enable
       < 0x04 = precode_enable, 0x08 = swizzle_enable */

    config->tx_datapath.mask = (config->rx_datapath.mask |= (config->rx_datapath.gray_enable = config->tx_datapath.gray_enable = gray) ? 0x00000002 : 0);
    config->tx_datapath.mask = (config->rx_datapath.mask |= (config->rx_datapath.precode_enable = config->tx_datapath.precode_enable = precoding) ? 0x00000004 : 0);
    config->tx_datapath.mask = (config->rx_datapath.mask |= (config->rx_datapath.swizzle_enable = config->tx_datapath.swizzle_enable = swizzle) ? 0x00000008 : 0);
    config->rx_datapath.mask |= (config->rx_datapath.polarity_invert = rxPolarity) ? 0x00000001 : 0;
    config->tx_datapath.mask |= (config->tx_datapath.polarity_invert = txPolarity) ? 0x00000001 : 0;

    if ( ilb == TRUE && prbs31 == TRUE ) {          /** Run a PRBS check in ILB; leave SerDes in ILB running PRBS 31 data */
        config->init_mode = AVAGO_PRBS31_ILB;
    } else if  ( ilb == FALSE && prbs31 == TRUE ) { /** Run a PRBS check in ILB; leave SerDes in ELB running PRBS 31 data */
        config->init_mode = AVAGO_PRBS31_ELB;
    } else if ( ilb == TRUE && prbs31 == FALSE ) {  /** Run a PRBS check in ILB; leave SerDes in ELB sending data from the ASIC core */
        config->init_mode = AVAGO_CORE_DATA_ILB;
    } else {                                        /** Run a PRBS check in ILB; leave SerDes in ELB sending data from the ASIC core */
        config->init_mode = AVAGO_CORE_DATA_ELB;
    }

    config->init_rx = initRx;
    config->init_tx = initTx;
    config->tx_output_en = enabletTxOut;
    config->tx_phase_cal = TRUE;

    /** Select the Rx & Tx data path width */

    if (serdesConfig.busWidth == _10BIT_ON) {
        config->rx_width = config->tx_width = 10;

        /** Save the width setting for PRBS test in AVAGO_SD_METAL_FIX register.
           It is needed for GPCS modes: the PRBS test can run in these modes only with Rx/Tx width=20BIT.
           If the Rx/Tx width=10BIT then it will be changed to 20BIT in mvHwsAvagoSerdesTestGen function */

        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[0], AVAGO_SD_METAL_FIX, (1 << 9), (1 << 9)));
    }
    else if (serdesConfig.busWidth == _20BIT_ON) config->rx_width = config->tx_width = 20;
    else if (serdesConfig.busWidth == _40BIT_ON) config->rx_width = config->tx_width = 40;
    else if (serdesConfig.busWidth == _80BIT_ON) config->rx_width = config->tx_width = 80;

    hwsOsPrintf("\n\tmvHwsAvagoSerdesArrayPowerCtrlImpl init_configuration:\n");
    hwsOsPrintf("\tsbus_reset = 0x%x\n",config->sbus_reset);
    hwsOsPrintf("\tspico_reset = 0x%x\n",config->spico_reset);
    hwsOsPrintf("\tinit_mode= %x\n",config->init_mode);
    hwsOsPrintf("\tinit_tx = 0x%x\n",config->init_tx);
    hwsOsPrintf("\tinit_rx = 0x%x\n",config->init_rx);
    hwsOsPrintf("\ttx_divider = 0x%x\n",config->tx_divider);
    hwsOsPrintf("\trx_divider = 0x%x\n",config->rx_divider);
    hwsOsPrintf("\ttx_width = 0x%x\n",config->tx_width);
    hwsOsPrintf("\trx_width = 0x%x\n",config->rx_width);
    hwsOsPrintf("\ttx_phase_cal = 0x%x\n",config->tx_phase_cal);
    hwsOsPrintf("\ttx_output_en = 0x%x\n",config->tx_output_en);
    hwsOsPrintf("\tsignal_ok_en = 0x%x\n",config->signal_ok_en);
    hwsOsPrintf("\tsignal_ok_threshold= %x\n",config->signal_ok_threshold);
    hwsOsPrintf("\ttx_encoding = 0x%x\n",config->tx_encoding);
    hwsOsPrintf("\trx encoding = 0x%x\n",config->tx_encoding);
    hwsOsPrintf("\tconfig->tx_datapath.mask = 0x%x\n",config->tx_datapath.mask);
    hwsOsPrintf("\tconfig->rx_datapath.mask = 0x%x\n",config->rx_datapath.mask);

    /** Config ref_clock_div ratio bits 11-12 */

    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[0], SERDES_EXTERNAL_CONFIGURATION_0, (ref_clk_div << 11), (ref_clk_div << 11)));

    CHECK_STATUS(mvHwsAvagoSerdesArrayPowerCtrlImplStub(devNum,portGroup,1,serdesArr,powerUp,&serdesConfig,&configDef,GT_TRUE));

    /** Config CTLE squelch level */

    memset(&rxConfig,0xFF,sizeof(rxConfig));
    rxConfig.rxAvago.squelch = squelchLevel;

    if ( GT_OK != mvHwsAvagoSerdesManualCtleConfig ( devNum, portGroup, serdesNum,  &rxConfig) ) {
            return GT_FAIL;
    }

    return GT_OK;
}

#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) */

#ifdef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsAvagoSerdesArrayPowerCtrlImplPhase function
* @endinternal
*
* @brief   Only in RAVEN (cm3) this funcion is calling Power up
*          SERDES next phase.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesArrayPowerCtrlImplPhase
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_UOPT     numOfSer,
    IN GT_UOPT     *serdesArr,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr,
    IN GT_U8       phase

)
{
    GT_U32 i;
    if (phase == MV_HWS_AVAGO_SERDES_POWER_LAST_PHASE)
    {
        for (i = 0; i < numOfSer; i++)
        {
            CHECK_STATUS(hwsAvagoSerdesTxRxTuneParamsSet(devNum, portGroup, serdesArr[i],
                                                         serdesConfigPtr));
        }
        for (i = 0; i < numOfSer; i++)
        {
           CHECK_STATUS(mvHwsAvagoSignalDetectInvert(devNum, portGroup, serdesArr[i], GT_TRUE));
        }
        /* Serdes Digital UnReset */
        for (i = 0; i < numOfSer; i++)
        {
            CHECK_STATUS(mvHwsAvagoSerdesResetImpl(devNum, portGroup, serdesArr[i], GT_FALSE, GT_FALSE, GT_FALSE));
        }
    }
    return GT_OK;
}
#endif

/**
* @internal mvHwsAvagoSerdesPowerCtrlImpl function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - Serdes Number
* @param[in] powerUp                  - powerup (On/Off)
* @param[in] divider                  - Divider
* @param[in] serdesConfigPtr          - configuration params
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPowerCtrlImpl
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      serdesNum,
    IN GT_U8                       powerUp,
    IN GT_U32                      divider,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{

    GT_U32 sbus_addr;
    GT_UREG_DATA widthMode = 0;
    GT_UREG_DATA data_ureg;
#if !defined(BC3_DEV_SUPPORT) && !defined(ALDRIN2_DEV_SUPPORT) && !defined(PIPE_DEV_SUPPORT)
    Avago_serdes_init_config_t *config;
    Avago_serdes_init_config_t configDef;
    unsigned int errors;
    GT_32 data;
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BOBK_DEV_SUPPORT)
    GT_U8 macOffset = 0;
    GT_U8 sdOffset = 0;
#endif

#ifndef FALCON_DEV_SUPPORT
#ifndef RAVEN_DEV_SUPPORT
    GT_U8 numOfLanes;
    GT_U32 ctleBias;
    GT_U32 t; /* timeout iterator */
    GT_BOOL pllStable;
    GT_STATUS rc;
    unsigned int ctleBitMapIndex;
#endif
#endif
#endif
    unsigned int chipIndex;

#if  !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    GT_U32 i;
#else
    GT_U32  tryCount=0;
#endif

    if(serdesConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }
    /* Get chip number to fill AAPL array */
    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* for Serdes PowerDown */
    if (powerUp == GT_FALSE)
    {
#if  !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
        /* if Training is in progress give it chance to finish and turn serdes off anyway */
        for (i=0; i< 25; i++)
        {
            CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_CORE_STATUS, &data_ureg, 0x2));
            if ((data_ureg != 0))
            {
                ms_sleep(20);
            }
            else
            {
                break;
            }
        }
#else
        do
        {
            CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_CORE_STATUS, &data_ureg, 0x2));
        }while ((data_ureg != 0) && (tryCount++ < 5));
#endif
        /* turn serdes off */
        AVAGO_LOCK(devNum, chipIndex);

        /* 0x7 , 0x0
        disable the spico and return its parameters to default. */
        avago_sbus_wr(aaplSerdesDb[chipIndex], sbus_addr, 0x7, 0x0);

        /* 0x7 , 0x2
         enable the spico which is now at init state (like after loading the spico FW). */
        avago_sbus_wr(aaplSerdesDb[chipIndex], sbus_addr, 0x7, 0x2);

        if ( serdesConfigPtr->serdesType != AVAGO_16NM )
        {
            avago_serdes_set_tx_rx_enable(aaplSerdesDb[chipIndex], sbus_addr, GT_FALSE, GT_FALSE, GT_FALSE);
            if (aaplSerdesDb[chipIndex]->return_code != 0)
            {
                aaplSerdesDb[chipIndex]->return_code = 0;
            }

        }
        AVAGO_UNLOCK(devNum, chipIndex);

        /* TxClk/RxClk Tlat_en=0. Return to default */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, 0, (3 << 13)));

        /* Read the saved value from AVAGO_SD_METAL_FIX register to check the Rx/Tx width mode */
        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_SD_METAL_FIX, &data_ureg, 0xFFFF));
        widthMode = (data_ureg >> 9) & 0x1;
        if (widthMode == 1)
        {
            /* Set back the value of AVAGO_SD_METAL_FIX register bit #9 if it was changed in 10BIT_ON mode for GPCS modes */
            CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_SD_METAL_FIX, (0 << 9), (1 << 9)));
        }

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT)
        if (serdesConfigPtr->serdesType == AVAGO_16NM)
        {
            /****************************************/
            /* SerDes power-down optimization flow */
            /****************************************/

            avago_serdes_init_config(&configDef);   /* load default values */
            config = &configDef;

            /* override some struct fields */
            divider = 8;  /* corresponds to 1.25G */
            config->rx_divider = divider;
            config->tx_divider = divider;
            config->skip_crc = TRUE;
            config->init_rx = TRUE;
            config->init_tx = FALSE;
            config->tx_encoding = AVAGO_SERDES_NRZ;
            config->tx_output_en = FALSE;
            config->rx_encoding = AVAGO_SERDES_NRZ;
            config->init_mode = AVAGO_PRBS31_ILB;
            config->tx_phase_cal = FALSE;
            config->tx_width = 20;

            AVAGO_LOCK(devNum, chipIndex);

            errors = avago_serdes_init(aaplSerdesDb[chipIndex], sbus_addr, config);
            if ((errors > 0) || (aaplSerdesDb[chipIndex]->return_code != 0)) {
                AVAGO_UNLOCK(devNum, chipIndex);
                CHECK_AVAGO_RET_CODE();
#ifndef MV_HWS_REDUCED_BUILD
                AVAGO_DBG(("SerDes power-down complete for SerDes at addr %s; Errors in ILB: %d. \n", aapl_addr_to_str(sbus_addr), errors));
#else
                AVAGO_DBG(("SerDes power-down complete for SerDes at addr 0x%x; Errors in ILB: %d. \n", sbus_addr, errors));
#endif /* MV_HWS_REDUCED_BUILD */
            } else {
#ifndef MV_HWS_REDUCED_BUILD
                AVAGO_DBG(("The SerDes at address %s is powered-down.\n", aapl_addr_to_str(sbus_addr)));
#else
                AVAGO_DBG(("The SerDes at address 0x%x is powered-down.\n", sbus_addr));
#endif /* MV_HWS_REDUCED_BUILD */
            }

            AVAGO_UNLOCK(devNum, chipIndex);
        }
#endif

        /* Serdes Digital Reset */
        CHECK_STATUS(mvHwsAvagoSerdesResetImpl(devNum, portGroup, serdesNum, GT_TRUE, GT_TRUE, GT_TRUE));

        return GT_OK;
    }

#if !defined(BC3_DEV_SUPPORT) && !defined(ALDRIN2_DEV_SUPPORT) && !defined(PIPE_DEV_SUPPORT)

    /* Serdes PowerUp */
    /*================*/
    /* Initialize the SerDes Avago_serdes_init_config_t struct */
    avago_serdes_init_config(&configDef);
    config = &configDef;
    config->signal_ok_threshold = 2;
    config->tx_divider = config->rx_divider = divider;

#if !defined(BOBK_DEV_SUPPORT) && !defined(ALDRIN_DEV_SUPPORT)
    if (serdesConfigPtr->serdesType == AVAGO_16NM)
    {
        if (serdesConfigPtr->encoding == SERDES_ENCODING_PAM4)
        {
            config->rx_encoding = config->tx_encoding = AVAGO_SERDES_PAM4;
        }
        else /* SERDES_ENCODING_AVAGO_NRZ */
        {
            config->rx_encoding = config->tx_encoding = AVAGO_SERDES_NRZ;
        }
        config->init_mode = AVAGO_PRBS31_ILB;


        /* In Raven need to enable the Tx output in Serdes level
           since in previous device it is done in port level    */
        config->tx_output_en = TRUE;

        /* signal OK threshold is not reliable - need to be disabled  */
        config->signal_ok_en = TRUE;
    }
#endif

    if (serdesConfigPtr->serdesType == AVAGO)
    {
        if (serdesConfigPtr->encoding == SERDES_ENCODING_PAM4)
        {
            AVAGO_DBG(("mvHwsAvagoSerdesPowerCtrlImpl: for Avago Serdes 28nm only SERDES_ENCODING_AVAGO_NRZ mode is supported\n"));
            return GT_BAD_PARAM;
        }
        config->init_mode = AVAGO_INIT_ONLY;
        config->tx_output_en = FALSE;
        config->rx_encoding = config->tx_encoding = AVAGO_SERDES_NRZ;
    }

#ifdef MICRO_INIT
    config->tx_output_en = TRUE;
#endif

    config->tx_phase_cal = TRUE;

    /* Select the Rx & Tx data path width */
    if (serdesConfigPtr->busWidth == _10BIT_ON)
    {
        config->rx_width = config->tx_width = 10;

        /* Save the width setting for PRBS test in AVAGO_SD_METAL_FIX register.
           It is needed for GPCS modes: the PRBS test can run in these modes only with Rx/Tx width=20BIT.
           If the Rx/Tx width=10BIT then it will be changed to 20BIT in mvHwsAvagoSerdesTestGen function */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_SD_METAL_FIX, (1 << 9), (1 << 9)));
    }
    else if (serdesConfigPtr->busWidth == _20BIT_ON) config->rx_width = config->tx_width = 20;
    else if (serdesConfigPtr->busWidth == _40BIT_ON) config->rx_width = config->tx_width = 40;
    else if (serdesConfigPtr->busWidth == _80BIT_ON) config->rx_width = config->tx_width = 80;

    AVAGO_DBG(("\nmvHwsAvagoSerdesPowerCtrlImpl init_configuration:\n"));
    AVAGO_DBG(("   sbus_reset = %x \n",config->sbus_reset));
    AVAGO_DBG(("   spico_reset = %x \n",config->spico_reset));
    AVAGO_DBG(("   init_mode= %x \n",config->init_mode));
    AVAGO_DBG(("   init_tx = %x \n",config->init_tx));
    AVAGO_DBG(("   init_rx = %x \n",config->init_rx));
    AVAGO_DBG(("   tx_divider = 0x%x \n",config->tx_divider));
    AVAGO_DBG(("   rx_divider = 0x%x \n",config->rx_divider));
    AVAGO_DBG(("   tx_width = 0x%x \n",config->tx_width));
    AVAGO_DBG(("   rx_width = 0x%x \n",config->rx_width));
    AVAGO_DBG(("   tx_phase_cal = %x \n",config->tx_phase_cal));
    AVAGO_DBG(("   tx_output_en = %x \n",config->tx_output_en));
    AVAGO_DBG(("   signal_ok_en = %x \n",config->signal_ok_en));
    AVAGO_DBG(("   signal_ok_threshold= %x \n",config->signal_ok_threshold));
    AVAGO_DBG(("   tx encoding= %x \n",config->rx_encoding));
    AVAGO_DBG(("   rx encoding= %x \n",config->tx_encoding));


    /* Serdes Analog Un Reset*/
    CHECK_STATUS(mvHwsAvagoSerdesResetImpl(devNum, portGroup, serdesNum, GT_FALSE, GT_TRUE, GT_TRUE));

    /* config media */
    data = (serdesConfigPtr->media == RXAUI_MEDIA) ? (1 << 2) : 0;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, data, (1 << 2)));
    /* Reference clock source */
    data = ((serdesConfigPtr->refClockSource == PRIMARY) ? 0 : 1) << 8;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, data, (1 << 8)));
    /* TxClk/RxClk Tlat_en=1. The logic of the Tlat need to be 1 in order to enable Clk to MAC/PCS regardless of Serdes readiness */
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, (3 << 13), (3 << 13)));

    AVAGO_LOCK(devNum, chipIndex);
    errors = avago_serdes_init(aaplSerdesDb[chipIndex], sbus_addr, config);
    AVAGO_UNLOCK(devNum, chipIndex);

    CHECK_AVAGO_RET_CODE();
    if (errors > 0)
    {
#ifndef MV_HWS_REDUCED_BUILD
        AVAGO_DBG(("SerDes init complete for SerDes at addr %s; Errors in ILB: %d. \n", aapl_addr_to_str(sbus_addr), errors));
#else
        AVAGO_DBG(("SerDes init complete for SerDes at addr 0x%x; Errors in ILB: %d. \n", sbus_addr, errors));
#endif /* MV_HWS_REDUCED_BUILD */
    }
    if (errors == 0 && aapl_get_return_code(aaplSerdesDb[chipIndex]) == 0)
    {
#ifndef MV_HWS_REDUCED_BUILD
        AVAGO_DBG(("The SerDes at address %s is initialized.\n", aapl_addr_to_str(sbus_addr)));
#else
        AVAGO_DBG(("The SerDes at address 0x%x is initialized.\n", sbus_addr));
#endif /* MV_HWS_REDUCED_BUILD */
    }

#if !defined(BOBK_DEV_SUPPORT) && !defined(ALDRIN_DEV_SUPPORT)
    if (hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM)
    {
        /* disable PRBS and undo ILB */
        ms_sleep(5);
        AVAGO_LOCK(devNum, chipIndex);
        avago_serdes_set_tx_data_sel(aaplSerdesDb[chipIndex], sbus_addr, AVAGO_SERDES_TX_DATA_SEL_CORE);
        avago_serdes_set_rx_cmp_data(aaplSerdesDb[chipIndex], sbus_addr, AVAGO_SERDES_RX_CMP_DATA_OFF);
        avago_serdes_set_rx_input_loopback(aaplSerdesDb[chipIndex], sbus_addr, FALSE);
        AVAGO_UNLOCK(devNum, chipIndex);
        CHECK_AVAGO_RET_CODE();

        if(serdesConfigPtr->baudRate != _1_25G)
        {
            /* operate SerDes in low power mode */
            CHECK_STATUS(mvHwsAvagoSerdesLowPowerModeEnable(devNum, portGroup, serdesNum, GT_TRUE));
        }

    }
#endif

#ifndef FALCON_DEV_SUPPORT
#ifndef RAVEN_DEV_SUPPORT
    if (hwsDeviceSpecInfo[devNum].serdesType == AVAGO)
    {
#if !defined(ASIC_SIMULATION)
        /*
           By default the VCO calibration is operated on Avago Serdes.
           To bypass the VCO calibration for AP port, change the value of
           apPortGroup parameter in mvHwsPortCtrlApEng.c:
           apPortGroup = AVAGO_SERDES_INIT_BYPASS_VCO
        */
        if (portGroup != AVAGO_SERDES_INIT_BYPASS_VCO)
        {
            int     res;
            GT_32     temperature;
            BOOL    mode;  /* True for Tx mode, False for Rx mode */
            GT_32     txTherm, txBin, rxTherm, rxBin;

            /* get the Avago Serdes Temperature (in C) */
            CHECK_STATUS(mvHwsAvagoSerdesTemperatureGet(devNum, portGroup, chipIndex, &temperature));

            mode = GT_TRUE;
            /* get the calibration code(value) for Tx */
            CHECK_STATUS(mvHwsAvagoSerdesCalCodeGet(devNum, portGroup, serdesNum, mode, &txTherm, &txBin));

            mode = GT_FALSE;
            /* get the calibration code(value) for Rx */
            CHECK_STATUS(mvHwsAvagoSerdesCalCodeGet(devNum, portGroup, serdesNum, mode, &rxTherm, &rxBin));

                /* Compensate the VCO calibration value according to Temperature */
                res = mvHwsAvagoSerdesVcoConfig(devNum, portGroup, serdesNum, temperature, txTherm, txBin, rxTherm, rxBin);
                if (res != GT_OK)
                {
                    AVAGO_DBG(("mvHwsAvagoSerdesVcoConfig failed (%d)\n", res));
                    return GT_FAIL;
                }
            }
#endif /* !defined(ASIC_SIMULATION) */

            /* Disable TAP1 before Training */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, TAP1_AVG_DISABLE, NULL));

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BOBK_DEV_SUPPORT)
            /* calculate the bitMap index */
            if ((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (serdesNum >= 20))
            {
                /* get the mac number */
                macOffset = hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasMacNum;
                ctleBitMapIndex = macOffset / AVAGO_CTLE_BITMAP;
                /* update SerDes offset */
                sdOffset = macOffset - (ctleBitMapIndex * AVAGO_CTLE_BITMAP);
            }
            else
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BOBK_DEV_SUPPORT) */
            {
                ctleBitMapIndex = serdesNum / AVAGO_CTLE_BITMAP;
            }

            if ((((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (serdesNum < 20))) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin)
                || (HWS_DEV_SILICON_TYPE(devNum) == Pipe) || (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2))
            {
                numOfLanes = hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasLanesNum;
                if ((numOfLanes == 4) || (numOfLanes == 2))
                {
                    CHECK_STATUS(mvHwsAvagoSerdesMultiLaneCtleBiasConfig(devNum, serdesNum, numOfLanes, ctleBitMapIndex, &ctleBias));
                }
                else
                {
                    /* get CTLE Bias value */
                    ctleBias = (((hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasValue[ctleBitMapIndex]) >> (serdesNum % AVAGO_CTLE_BITMAP)) & 0x1);
                }
            }
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BOBK_DEV_SUPPORT)
            else if (((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (serdesNum >= 20)))
            {
                numOfLanes = hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasLanesNum;
                if ((numOfLanes == 4) || (numOfLanes == 2))
                {
                    CHECK_STATUS(mvHwsAvagoSerdesMultiLaneCtleBiasConfig(devNum, sdOffset, numOfLanes, ctleBitMapIndex, &ctleBias));
                }
                else
                {
                    /* get CTLE Bias value */
                    ctleBias = (((hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasValue[ctleBitMapIndex]) >> sdOffset) & 0x1);
                }
            }
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BOBK_DEV_SUPPORT) */
            else
            {
                /* get CTLE Bias value */
                ctleBias = (((hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasValue[ctleBitMapIndex]) >> (serdesNum % AVAGO_CTLE_BITMAP)) & 0x1);
            }

            /* change CTLE bias from 1 to 0 */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x4033, NULL));

            /* read the CTLE bias */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x1A, 0x0, &data));

            if (ctleBias)
            {
                /* set CTLE Bias to 1 */
                data |= 0x1000;
            }
            else
            {
                /* set CTLE Bias to 0 */
                data &= 0xCFFF;
            }

            /* apply CTLE Bias */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, data, NULL));

#if !defined(MV_HWS_AVAGO_NO_VOS_WA) || ( defined(MICRO_INIT) && !defined (BOBK_DEV_SUPPORT) )

            /* Configure serdes VOS parameters*/
        if (hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesVosOverride)
            {
                /* VOS Override parameters should be configured on ports with speed >= 10G, so filtering is needed.
                   Here in serdes level, we can detect the speed by examine the 'mode' and 'baudRate' parameters*/
                /* First, all of the 10G and above speeds have 20bit and 40bit mode */
                if (serdesConfigPtr->busWidth == _20BIT_ON || serdesConfigPtr->busWidth == _40BIT_ON)
                {
                    /* Second, some of the speeds less than 10G also have 20bit mode, so baudRate filtering needed */
                    if (serdesConfigPtr->baudRate != _5_15625G && serdesConfigPtr->baudRate != _5G)
                    {
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)
                    /* for BC3 only 10G baudRate ports */
                    if ((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
                        {
                            if (_10_3125G == serdesConfigPtr->baudRate)
                            {
                                CHECK_STATUS(mvHwsAvagoSerdesVosDfxParamsSet(devNum, serdesNum));
                            }
                        }
                        else
#endif /* #if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) */
                        {
                            CHECK_STATUS(mvHwsAvagoSerdesManualVosConfigSet(devNum, serdesNum));
                        }
                    }
                }
            }
#endif /* MV_HWS_AVAGO_NO_VOS_WA */

        /* implement SW WA for linkDown problem: release the Spico driver of Serdes */
        if ((serdesConfigPtr->baudRate == _1_25G) ||
            (serdesConfigPtr->baudRate == _5G) ||
            (serdesConfigPtr->baudRate == _10_3125G))
        {
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x8000, NULL));
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 0x2, NULL));
        }

#ifdef MV_HWS_FREE_RTOS /* for AP port only */
        if (serdesConfigPtr->baudRate == _3_125G)
        {   /* Improve the CDR lock process */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x40FB, NULL));
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 0x1200, NULL));
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x40A9, NULL));
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 0x1100, NULL));
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x4001, NULL));
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 0x1FF,  NULL));
        }
#endif /* MV_HWS_FREE_RTOS */

        CHECK_STATUS(hwsAvagoSerdesTxRxTuneParamsSet(devNum, portGroup, serdesNum, serdesConfigPtr));
        for (t = 0; t < MV_HWS_AVAGO_STABLE_PLL_TIMEOUT; t++) /* Wait for PLL to stablize*/
        {
            pllStable = GT_FALSE;
            rc = mvHwsAvagoPllStableGet(devNum,portGroup,serdesNum,&pllStable);
            if (((GT_OK == rc) && (pllStable == GT_TRUE)) || (GT_NOT_IMPLEMENTED == rc))
            {
                pllStable = GT_TRUE;
                break;
            }
            /* Delay */
            hwsOsExactDelayPtr(devNum, portGroup, 5);
        }

        /* in case pll is not stable */
        if ((pllStable != GT_TRUE) || (t == MV_HWS_AVAGO_STABLE_PLL_TIMEOUT))
        {
            return GT_TIMEOUT;
        }
    }
    else /* 16nm */
    {
        CHECK_STATUS(hwsAvagoSerdesTxRxTuneParamsSet(devNum, portGroup, serdesNum, serdesConfigPtr));
        /* enable EID Filter */
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
        /* enable eid filter
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x2c, 0x11c, NULL));
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x6c, 0xff1, NULL));
        */
        CHECK_STATUS(mvHwsAvagoSignalDetectInvert(devNum, portGroup, serdesNum, GT_TRUE));
#endif
    }

    /* Serdes Digital UnReset */
    CHECK_STATUS(mvHwsAvagoSerdesResetImpl(devNum, portGroup, serdesNum, GT_FALSE, GT_FALSE, GT_FALSE));
#endif /* RAVEN_DEV_SUPPORT */
#endif
#endif /* defined(BC3_DEV_SUPPORT) && !defined(ALDRIN2_DEV_SUPPORT) && !defined(PIPE_DEV_SUPPORT) */
    return GT_OK;
}

/**
* @internal mvHwsAvagoSbusReset function
* @endinternal
*
* @brief   Per SERDES perform sbus reset
*
* @param[in] devNum                  - system device number
* @param[in] serdesNum               - physical serdes number
* @param[in] reset                   - reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSbusReset
(
    IN GT_U8 devNum,
    IN GT_U32 serdesNum,
    IN GT_BOOL reset
)
{
    GT_U32  sbus_addr, val = 0;
    unsigned int chipIndex;

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));
     if (reset == GT_FALSE)
        val = 2;
    AVAGO_LOCK(devNum, chipIndex);
    avago_sbus_wr(aaplSerdesDb[chipIndex], sbus_addr, 0x7, val);
    AVAGO_UNLOCK(devNum, chipIndex);
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesMultiLaneCtleBiasConfig function
* @endinternal
*
* @brief   Per SERDES set the CTLE Bias value, according to data base.
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - physical serdes number
* @param[in] numOfLanes               - number of lanes
* @param[in] ctleBitMapIndex          - index of ctle bias bit
*       map
*
* @param[out] ctleBiasPtr              - pointer to ctle bias value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesMultiLaneCtleBiasConfig
(
    IN GT_U8 devNum,
    IN unsigned int serdesNum,
    IN GT_U8 numOfLanes,
    IN unsigned int ctleBitMapIndex,
    OUT GT_U32 *ctleBiasPtr
)
{
    serdesNum = serdesNum%AVAGO_CTLE_BITMAP;
    *ctleBiasPtr = ((hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasValue[ctleBitMapIndex]) >> (serdesNum - (serdesNum % numOfLanes))) & 0x1;    return GT_OK;
}

#if !defined(CPSS_BLOB)
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT) || defined(RAVEN_DEV_SUPPORT)
/*******************************************************************************
* mvHwsAvagoSerdesAdaptiveStatusGet
*
* DESCRIPTION:
*       Per SERDES check is adaptive pcal is running
*
* INPUTS:
*       devNum    - system device number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       adaptiveIsRuning - true= adaptive running, false=not running
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsAvagoSerdesAdaptiveStatusGet
(
    unsigned char   devNum,
    unsigned int    serdesNum,
    GT_BOOL         *adaptiveIsRuning
)
{
    GT_32 data;

    *adaptiveIsRuning = GT_TRUE;

    /* get DFE status */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, 0, serdesNum, 0x126, ((0 << 12) | (0xB << 8)), &data));
    if((data & 0x83)== 0x80) /* bit 7 = 1 and bits 0,1 =0 */
    {
       *adaptiveIsRuning = GT_FALSE;
    }

    return GT_OK;
}
#endif
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
/**
* @internal mvHwsAvagoSerdesEyeGet function
* @endinternal
*
* @brief   Per SERDES return the adapted tuning results Can be run after create port.
*
* @param[in] devNum                  - system device number
* @param[in] serdesNum               - physical serdes number
* @param[in] eye_inputPtr            - pointer to input parameters structure
*
* @param[out] eye_resultsPtr         - pointer to results structure
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
unsigned int mvHwsAvagoSerdesEyeGet
(
    IN unsigned char                       devNum,
    IN unsigned int                        serdesNum,
    IN MV_HWS_AVAGO_SERDES_EYE_GET_INPUT  *eye_inputPtr,
    OUT MV_HWS_AVAGO_SERDES_EYE_GET_RESULT *eye_resultsPtr

)
{
    GT_STATUS rc = GT_OK;
    unsigned int sbus_addr;
    int errors, counter, return_code;
    Avago_serdes_eye_config_t *configp;
    Avago_serdes_eye_data_t *datap;
    GT_BOOL    adaptiveIsRuning, adaptiveIsOn = GT_FALSE;
    GT_BOOL    enableLowPower = GT_FALSE;
    unsigned int chipIndex;

#ifndef ALDRIN_DEV_SUPPORT
    if (hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM)
    {
        /* operate SerDes in low power mode */
        CHECK_STATUS(mvHwsAvagoSerdesLowPowerModeEnableGet(devNum, 0, serdesNum, &enableLowPower));
        if(enableLowPower == GT_TRUE)
        {
            CHECK_STATUS(mvHwsAvagoSerdesLowPowerModeEnable(devNum, 0, serdesNum, GT_FALSE));
        }
    }
#endif
    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));
    configp = avago_serdes_eye_config_construct(aaplSerdesDb[chipIndex]);
    if (configp == NULL) {
        rc = GT_BAD_PTR;
        goto return_error;
    }

    configp->ec_min_dwell_bits = (bigint)eye_inputPtr->min_dwell_bits;
    configp->ec_max_dwell_bits = (bigint)eye_inputPtr->max_dwell_bits;

    datap = avago_serdes_eye_data_construct(aaplSerdesDb[chipIndex]);
    if (datap == NULL) {
        avago_serdes_eye_config_destruct(aaplSerdesDb[chipIndex],configp);
        rc = GT_BAD_PTR;
        goto return_error;
    }
    if (hwsDeviceSpecInfo[devNum].serdesType != AVAGO_16NM)
    {
        if(mvHwsAvagoSerdesAdaptiveStatusGet(devNum,serdesNum, &adaptiveIsRuning)!= GT_OK)
        {
            avago_serdes_eye_data_destruct(aaplSerdesDb[chipIndex],datap);
            avago_serdes_eye_config_destruct(aaplSerdesDb[chipIndex],configp);
            rc = GT_FAIL;
            goto return_error;
        }
        else
        {
            if (adaptiveIsRuning) { /* adaptive pcal is running */
                adaptiveIsOn = GT_TRUE;
                CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, 0, serdesNum, DFE_STOP_ADAPTIVE, NULL));
                counter = 0;
                while(adaptiveIsRuning)
                {
                    CHECK_STATUS(mvHwsAvagoSerdesAdaptiveStatusGet(devNum,serdesNum, &adaptiveIsRuning));
                    ms_sleep(20);
                    counter++;
                    if (counter> 50) {
                        avago_serdes_eye_data_destruct(aaplSerdesDb[chipIndex],datap);
                        avago_serdes_eye_config_destruct(aaplSerdesDb[chipIndex],configp);
                        rc = GT_FAIL;
                        goto return_error;
                    }
                }
            }
        }
    }
    AVAGO_LOCK(devNum, chipIndex);
    errors = avago_serdes_eye_get(aaplSerdesDb[chipIndex], sbus_addr, configp, datap);
    return_code = aaplSerdesDb[chipIndex]->return_code;
    AVAGO_UNLOCK(devNum, chipIndex);

    aaplSerdesDb[chipIndex]->return_code = return_code;
    if ((GT_OK != errors) ||
        (return_code < 0))
    {
        avago_serdes_eye_data_destruct(aaplSerdesDb[chipIndex],datap);
        avago_serdes_eye_config_destruct(aaplSerdesDb[chipIndex],configp);
        rc = (GT_OK != errors) ? GT_FAIL:GT_OK;
        goto return_error;
    }

    if ((hwsDeviceSpecInfo[devNum].serdesType != AVAGO_16NM) && adaptiveIsOn)
    {   /* need to start adaptive again after reading the eye*/
        rc = mvHwsAvagoSerdesDfeConfig(devNum, 0, serdesNum, DFE_START_ADAPTIVE, NULL);
        if (GT_OK != rc)
        {
            avago_serdes_eye_data_destruct(aaplSerdesDb[chipIndex],datap);
            avago_serdes_eye_config_destruct(aaplSerdesDb[chipIndex],configp);
            goto return_error;
        }
    }

    eye_resultsPtr->matrixPtr = avago_serdes_eye_plot_format(datap);
    eye_resultsPtr->x_points = datap->ed_x_points;
    eye_resultsPtr->y_points = datap->ed_y_points;
    eye_resultsPtr->height_mv = datap->ed_height_mV;
    eye_resultsPtr->width_mui = datap->ed_width_mUI;
    eye_resultsPtr->vbtcPtr = avago_serdes_eye_vbtc_format(&datap->ed_vbtc[0]);
    eye_resultsPtr->hbtcPtr = avago_serdes_eye_hbtc_format(&datap->ed_hbtc[0]);

    avago_serdes_eye_data_destruct(aaplSerdesDb[chipIndex],datap);
    avago_serdes_eye_config_destruct(aaplSerdesDb[chipIndex],configp);

return_error:
    if ((hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM) && (enableLowPower == GT_TRUE))
    {

        /* operate SerDes in low power mode */
        CHECK_STATUS(mvHwsAvagoSerdesLowPowerModeEnable(devNum, 0, serdesNum, GT_TRUE));
    }
    CHECK_AVAGO_RET_CODE();

    return rc;
}
#endif /*!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) */
#endif /*CPSS_BLOB*/
/**
* @internal mvHwsAvagoSerdesResetImpl function
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
GT_STATUS mvHwsAvagoSerdesResetImpl
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_BOOL     analogReset,
    IN GT_BOOL     digitalReset,
    IN GT_BOOL     syncEReset
)
{
    GT_U32  data;

    /* SERDES SD RESET/UNRESET init */
    data = (analogReset == GT_TRUE) ? 0 : 1;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_1, (data << 2), (1 << 2)));

    /* SERDES RF RESET/UNRESET init */
    data = (digitalReset == GT_TRUE) ? 0 : 1;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_1, (data << 3), (1 << 3)));

    /* SERDES SYNCE RESET init */
    if(syncEReset == GT_TRUE)
    {
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, 0, (1 << 6)));
    }
    else /* SERDES SYNCE UNRESET init */
    {
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, (1 << 6), (1 << 6)));
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_1, 0xDD00, 0xFF00));
    }

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesPolarityConfigImpl function
* @endinternal
*
* @brief   Per Serdes invert the Tx or Rx.
*         Can be run after create port.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPolarityConfigImpl
(
    IN GT_U8      devNum,
    IN GT_U32     portGroup,
    IN GT_U32     serdesNum,
    IN GT_U32     invertTx,
    IN GT_U32     invertRx
)
{
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)

    if (prvCpssSystemRecoveryInProgress())
    {
        return GT_OK;
    }
#endif

#ifndef ASIC_SIMULATION
    GT_U32 sbus_addr;
    unsigned int chipIndex;
    portGroup = portGroup; /* Avoid Warnings */

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    AVAGO_LOCK(devNum, chipIndex);
    /* Tx polarity En */
    avago_serdes_set_tx_invert(aaplSerdesDb[chipIndex], sbus_addr, invertTx);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();

    AVAGO_LOCK(devNum, chipIndex);
    /* Rx Polarity En */
    avago_serdes_set_rx_invert(aaplSerdesDb[chipIndex], sbus_addr, invertRx);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();
#endif /* ASIC_SIMULATION */
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesPolarityConfigGetImpl function
* @endinternal
*
* @brief   Returns the Tx and Rx SERDES invert state.
*         Can be run after create port.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPolarityConfigGetImpl
(
    IN GT_U8       devNum,
    IN GT_U32      portGroup,
    IN GT_U32      serdesNum,
    OUT GT_U32     *invertTx,
    OUT GT_U32     *invertRx
)
{
#ifndef ASIC_SIMULATION
    GT_U32 sbus_addr;
    unsigned int chipIndex;
    portGroup = portGroup; /* Avoid Warnings */

    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum, serdesNum);
    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    AVAGO_LOCK(devNum, chipIndex);
    /* Get the TX inverter polarity mode: TRUE - inverter is enabled, FALSE - data is not being inverted */
    *invertTx = avago_serdes_get_tx_invert(aaplSerdesDb[chipIndex], sbus_addr);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();

    AVAGO_LOCK(devNum, chipIndex);
    /* Get the RX inverter polarity mode: TRUE - inverter is enabled, FALSE - data is not being inverted */
    *invertRx = avago_serdes_get_rx_invert(aaplSerdesDb[chipIndex], sbus_addr);
    AVAGO_UNLOCK(devNum, chipIndex);
    CHECK_AVAGO_RET_CODE();

#endif /* ASIC_SIMULATION */
    return GT_OK;
}

/**
* @internal mvHwsAvagoAccessValidate function
* @endinternal
*
* @brief   Validate access to Avago device
*/
void mvHwsAvagoAccessValidate
(
    unsigned int chipIndex,
    uint sbus_addr)
{
    AVAGO_DBG(("Validate SBUS access (sbus_addr 0x%x)- ", sbus_addr));
    if (avago_diag_sbus_rw_test(aaplSerdesDb[chipIndex], avago_make_sbus_controller_addr(sbus_addr), 2) == TRUE)
    {
        AVAGO_DBG(("Access Verified\n"));
    }
    else
    {
        AVAGO_DBG(("Access Failed\n"));
    }
}

/**
* @internal mvHwsAvagoCheckSerdesAccess function
* @endinternal
*
* @brief   Validate access to Avago Serdes
*/
GT_STATUS mvHwsAvagoCheckSerdesAccess
(
    unsigned int  devNum,
    unsigned char portGroup,
    unsigned char serdesNum
)
{
  GT_UREG_DATA data;

  /* check analog reset */
  CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                      SERDES_EXTERNAL_CONFIGURATION_1, &data, 0));

  if(((data >> 3) & 1) == 0)
      return GT_NOT_INITIALIZED;

  return GT_OK;
}

/**
* @internal mvHwsAvagoAccessLock function
* @endinternal
*
* @brief   Protection Definition
*         =====================
*         Avago Firmware cannot be accessed by more than one client concurrently
*         Concurrent access might result in invalid data read from the firmware
*         There are three scenarios that require protection.
*         1. Multi-Process Application
*         This case is protected by SW Semaphore
*         SW Semaphore should be defined for each supported OS: FreeRTOS, Linux,
*         and any customer OS
*         This protection is relevant for Service CPU and Host
*         - Service CPU includes multi-process application, therefore protection is required
*         - Host customer application might / might not include multi-process, but from CPSS
*         point of view protection is required
*         2. Multi-Processor Environment
*         This case is protected by HW Semaphore
*         HW Semaphore is defined based in MSYS / CM3 resources
*         In case customer does not use MSYS / CM3 resources,
*         the customer will need to implement its own HW Semaphore
*         This protection is relevant ONLY in case Service SPU Firmware is loaded to CM3
*         3. Debug Capability
*         Avago GUI provides for extensive debug capabilities,
*         But it interface Avago Firmware directly via SBUS commands
*         Therefore No Semaphore protection can be used
*         Debug flag configured by the Host will enable / disable the
*         periodic behavior of AP State machine
*         Debug flag is out of the scope of this API
*
* @param[in] devNum                   - system device number
*
*          */
void mvHwsAvagoAccessLock
(
    IN GT_U8 devNum
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) || defined (FALCON_DEV_SUPPORT)
    ,
    IN GT_U8 chipIndex
#endif
)
{
    /*
    ** SW Semaphore Protection Section
    ** ===============================
    */
#if defined(CHX_FAMILY) || defined(PX_FAMILY)

    /* Host SW Protection */
    if (avagoAccessMutex)
    {
        hwsOsMutexLock(avagoAccessMutex);
    }
#elif defined(MV_HWS_FREE_RTOS)

    /* Service CPU SW Protection */
    mvPortCtrlSerdesLock();
#endif /* defined(CHX_FAMILY) || defined(PX_FAMILY) */

    /*
    ** HW Semaphore Protection Section
    ** ===============================
    */
#if defined MV_HWS_REDUCED_BUILD_EXT_CM3 && !defined (FALCON_DEV_SUPPORT)
    /* When running on any Service CPU, HW semaphore always used */
    mvSemaLock(devNum, MV_SEMA_AVAGO);
#else
    /* When running on Host CPU, HW semaphore always used only when a service CPU is present */
    if ( mvHwsServiceCpuEnableGet(devNum) ) {
        mvSemaLock(devNum, chipIndex, MV_SEMA_AVAGO);
    }
#endif
}

/**
* @internal mvHwsAvagoAccessUnlock function
* @endinternal
*
* @brief   See description in mvHwsAvagoAccessLock API
* @param[in] devNum                   - system device number
*/
void mvHwsAvagoAccessUnlock
(
    IN GT_U8 devNum
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) || defined (FALCON_DEV_SUPPORT)
    ,
    IN GT_U8 chipIndex
#endif
)
{
    /*
    ** HW Semaphore Protection Section
    ** ===============================
    */
#if defined MV_HWS_REDUCED_BUILD_EXT_CM3 && !defined (FALCON_DEV_SUPPORT)
    /* When running on any Service CPU, HW semaphore always used */
    mvSemaUnlock(devNum, MV_SEMA_AVAGO);
#else
    /* When running on Host CPU, HW semaphore always used only when a service CPU is present */
    if ( mvHwsServiceCpuEnableGet(devNum) ) {
        mvSemaUnlock(devNum, chipIndex, MV_SEMA_AVAGO);
    }
#endif

    /*
    ** SW Semaphore Protection Section
    ** ===============================
    */
#if defined(CHX_FAMILY) || defined(PX_FAMILY)

    /* Host SW Protection */
    if (avagoAccessMutex)
    {
        hwsOsMutexUnlock(avagoAccessMutex);
    }
#elif defined(MV_HWS_FREE_RTOS)

    /* Service CPU SW Protection */
    mvPortCtrlSerdesUnlock();

#endif /* defined(CHX_FAMILY) || defined(PX_FAMILY) */
}

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(MICRO_INIT) && defined(CM3) && !defined(BOBK_DEV_SUPPORT) && !defined(ALDRIN_DEV_SUPPORT))
#define MAX_HDFUSE_LINES 64
/**
* @internal mvHwsAvagoSerdesBc3VosConfig function
* @endinternal
*
* @brief   Reading the HDFuse and locating the AVG block where the compressed VOS
*         data parameters are.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/

GT_STATUS mvHwsAvagoSerdesBc3VosConfig(unsigned char devNum)
{
    GT_U32 i; /* iterrators */
    GT_BOOL isBurn = GT_FALSE; /* identify wheather the HDFuse line is burned */
    GT_U16 lineOffset = 0; /* AVG block data lines number */
    GT_U32 dfxVosWord1 = 0, dfxVosWord2 = 0, dfxVosWord3 = 0; /* each HDFuse line contains 3 words */
    GT_U8  numOfFuseLines = 0, numOfSerdes, minHdEfuseLines; /* minimum lines of AVG data block */
    GT_U32 avgBlockStart = 0;
    GT_U8  allSdsDataArr[MAX_AVG_SIZE] = {0};
    GT_U32 secondaryEfuseData = 0;

    if (HWS_DEV_SILICON_TYPE(devNum) == Pipe)
    {
        numOfSerdes = 16;
        minHdEfuseLines = 8;
    }
    else /* Bobcat3 and Aldrin2 */
    {
        numOfSerdes = 72;
        minHdEfuseLines = 28;
    }

    if (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2)
    {
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, SERVER_SECONDARY_EFUSE, &secondaryEfuseData));
        /* Select Secondary Efuse server */
        secondaryEfuseData |= (0x1 << 17);
        CHECK_STATUS(hwsServerRegSetFuncPtr(devNum, SERVER_SECONDARY_EFUSE, secondaryEfuseData));
    }

    /* reading the HDFuse block and finding the address of first data line (AVG block address) */
    for (i=0; i< MAX_HDFUSE_LINES; i++)
    {
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, SERVER_HD_EFUSE_SLAVE_REG+lineOffset, &dfxVosWord1));
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, SERVER_HD_EFUSE_WORD_2+lineOffset, &dfxVosWord2));
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, SERVER_HD_EFUSE_WORD_3+lineOffset, &dfxVosWord3));
        lineOffset += 0x10;
        if ((dfxVosWord1 !=0 && dfxVosWord3 != 0) || (dfxVosWord2 != 0 && dfxVosWord3 !=0))
        {
            isBurn = GT_TRUE;
            numOfFuseLines++;
            avgBlockStart = ((SERVER_HD_EFUSE_SLAVE_REG+lineOffset)-0x10);
        }
        else
        {
            if ((isBurn) && numOfFuseLines >= minHdEfuseLines)
            {
                break;
            }
            else
            {
                numOfFuseLines = 0;
                continue;
            }
        }
    }

    if (numOfFuseLines < minHdEfuseLines)
    {
        return GT_OK;
    }


    /* extract AVG params (30b of first word)*/
    CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, avgBlockStart, &dfxVosWord1));
    avgBlockData.revision = (dfxVosWord1 & 0x3);
    avgBlockData.avgBlockLines = (dfxVosWord1 >> 2) & 0x3F;
    avgBlockData.minVal = (dfxVosWord1 >> 8) & 0xFF;
    avgBlockData.baseVal = (dfxVosWord1 >> 16) & 0xFF;
    avgBlockData.bitsPerSds = (dfxVosWord1 >> 24) & 0x3F;

    if (avgBlockData.avgBlockLines != numOfFuseLines)
    {
        return GT_OK;
    }

    /* defining bits per word according to the revision parameter */

    lineOffset = 0;
    i = 0;

    /* reads 56 or 57 (according to revision) bits of each AVG line and set them in a  binary string
      which contains the whole data */
    while (i < avgBlockData.avgBlockLines)
    {
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, avgBlockStart-lineOffset, &dfxVosWord1));
        hwsMemCpy(&allSdsDataArr[i * 8], &dfxVosWord1, sizeof(GT_U32));
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, avgBlockStart-lineOffset+0x4, &dfxVosWord1));
        hwsMemCpy(&allSdsDataArr[i * 8 + 4], &dfxVosWord1, sizeof(GT_U32) - 1);
        if (avgBlockData.revision)
            allSdsDataArr[i * 8 + 7] = (dfxVosWord1 & 0x1000000) ? 1 : 0;
        lineOffset += 0x10;
        i++;
    }

    minVosValue = avgBlockData.minVal;
    baseVosValue = avgBlockData.baseVal;

    for (i = 0; i < numOfSerdes; i++)
    {
        /* get the 7 vos values word for each serdes */
        serdesVosParams64BitDB[i] = mvHwsAvagoSerdesCalcVosDfxParams(devNum, i, avgBlockData.bitsPerSds, allSdsDataArr);
    }

    serdesVos64BitDbInitialized = GT_TRUE;
    /*hdeFuseBc3WaEnable = GT_TRUE;*/

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesCalcVosDfxParams function
* @endinternal
*
* @brief   Calculates each 7 VOS values per serdes.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X.
* @param[in] serdesNum             - serdes number.
* @param[in] bitsPerSds            - number of bits to hold 7 VOS values.
* @param[in] dataArr               - Array with bits from EFuse
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_UL64 mvHwsAvagoSerdesCalcVosDfxParams
(
    unsigned char devNum,
    GT_U32 serdesNum,
    GT_U8  bitsPerSds,
    GT_U8  dataArr[]
)
{
    GT_UL64 tempVal1 = 0;
    char tempVal2, sds_word[63] = {0};
    GT_U32 j = 0, bitIndex; /* itterators */
    GT_U8 i, numOfSdsIndex, wordSize, words;
    static GT_U32 nextWord = 0;

    /* dateArr[] stores 56 or 57 bits (depending on revision) per 64bit space, i.e.
     * per each 64bit of space, we have 7-8 bits MSB unused. */
    /* Calculate word size - 56 or 57 */
    wordSize = 56 + (avgBlockData.revision ? 1 : 0);

    if (HWS_DEV_SILICON_TYPE(devNum) == Pipe)
    {
        numOfSdsIndex = 15;
    }
    else /* Bobcat3 and Aldrin2 */
    {
        numOfSdsIndex = 71;
    }

    for (i = 0; i < bitsPerSds; i++)
    {
        bitIndex = 30 + i + nextWord;
        /* Per bit, calculate index of byte and bit in dataArr */
        words = bitIndex / wordSize;
        j = words * 8 + (bitIndex - words * wordSize) / 8;
        bitIndex = (bitIndex - words * wordSize) % 8;

        sds_word[i] = (dataArr[j] & (1 << bitIndex)) != 0;

        if (i == (bitsPerSds-1))
        {
            for (j = 0; j < bitsPerSds / 2; j++)
            {
                tempVal2 = sds_word[j];
                sds_word[j] = sds_word[bitsPerSds-j-1];
                sds_word[bitsPerSds-j-1] = tempVal2;
            }
            j = 0;
            while (j < bitsPerSds)
            {
                tempVal1 *= 2;
                if (sds_word[j] == 1)
                {
                    tempVal1 += 1;
                }
                j++;
            }
        }
    }
    nextWord += bitsPerSds;

    if (serdesNum == numOfSdsIndex)
    {
        nextWord = 0;
    }

    return tempVal1;
}
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 || defined(MICRO_INIT) */

/**
* @internal mvHwsAvagoSerdesVosDfxParamsSet function
* @endinternal
*
* @brief   Decoding the VOS data to each sampler of the serdes and
*         applying VOS correction parameters from HDFuze to improve eye openning.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X.
* @param[in] devNum                   - system device number.
* @param[in] serdesNum                - serdes number.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if defined(PIPE_DEV_SUPPORT) ||  defined(ALDRIN2_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
GT_STATUS mvHwsAvagoSerdesVosDfxParamsSet
(
    GT_U32 devNum,
    GT_U32 serdesNum
)
{
    GT_UL64 sds_values, samplersData;
    GT_U32 i; /* itterator */
    GT_U32 delta;
    GT_U32 serdesVosValue;
    GT_U32 maxVosSerdes;

    if (!serdesVos64BitDbInitialized)
    {
        CPSS_LOG_INFORMATION_MAC("Not performing VOS config on serdes %d ", serdesNum);
        return GT_OK;
    }

    if (HWS_DEV_SILICON_TYPE(devNum) == Pipe)
    {
        maxVosSerdes = 15;
    }
    else /* Bobcat3 and Aldrin2 */
    {
        maxVosSerdes = 71;
    }

    if (serdesNum > maxVosSerdes)
    {
        CPSS_LOG_INFORMATION_MAC("eFuse VOS params exceed SerDes index limit");
        return GT_OK;
    }
    else
    {
        samplersData = serdesVosParams64BitDB[serdesNum];
    }

    for (i=0; i< 7; i++)
    {
        delta = samplersData % baseVosValue;
        sds_values = delta+minVosValue;
        /* Calculating the generic formula */
        serdesVosValue = ( 0x1900 + (i * 0x100) );
        /* Applying final VOS value to serdes */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, 0, serdesNum, 0x26, serdesVosValue+(sds_values & 0xFF), NULL));
        samplersData = (samplersData-delta)/ baseVosValue;
    }

    return GT_OK;
}
#endif /* #if defined(PIPE_DEV_SUPPORT) ||  defined(ALDRIN2_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) */

/**
* @internal mvHwsAvagoSerdesManualTxDBSet function
* @endinternal
*
* @brief   Initialize Avago tx related configurations
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] phyPortNum               - port number
* @param[in] serdesSpeed              - port speed
* @param[in] serdesNumber             - serdes number
* @param[in] valid                    - data is valid
* @param[in] txConfigPtr              - tx config params
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualTxDBSet
(
    GT_U8                                devNum,
    GT_U32                               portGroup,
    GT_U32                               phyPortNum,
    MV_HWS_SERDES_SPEED                  serdesSpeed,
    GT_U16                               serdesNumber,
    MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA *txConfigPtr
)
{
#ifndef BOBK_DEV_SUPPORT
    GT_U8 sdSpeed, speedIdx;
#endif

    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(phyPortNum);

#ifndef BOBK_DEV_SUPPORT
    if (txConfigPtr == NULL) {
        return GT_BAD_PTR;
    }

    sdSpeed = serdesSpeed;

    if (serdesNumber >= MV_HWS_IPC_MAX_PORT_NUM)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"serdes number not supported for serdes tx params offset API");
    }

    speedIdx = mvHwsSerdesSpeedToIndex(devNum, sdSpeed);

    if (speedIdx == UNPERMITTED_SD_SPEED_INDEX)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"serdes speed not supported for serdes tx params offset API");
    }

    mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNumber].valid |= MV_HWS_SERDES_TXRX_TUNE_PARAMS_DB_TX_VALID;
    hwsMemCpy(&mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNumber].txData,
              txConfigPtr, sizeof(MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA));
#endif
    return GT_OK;
}

#ifdef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsAvagoSerdesManualTxPresetDBSet function
* @endinternal
*
* @brief   Initialize Avago preset parameters
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] rxTermination            - rxTermination
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if 0
MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA hwsAvagoSerdesTxTune16nmPresetParams[3][AN_PRESETS_NUM] = /* Spico int 0x3D */
{                      /* Preset1[CL136] / Preset[CL72]          Preset2[CL136] / Initialize[CL72]     Preset3[CL136] */
                       /*  atten  post   pre    pre2   pre3      atten  post   pre    pre2   pre3      atten  post   pre    pre2   pre3 */
/*_10_3125G            */{{4     ,0     ,0     ,TX_NA ,TX_NA }, {4     ,12    ,2     ,TX_NA ,TX_NA }, {TX_NA ,TX_NA ,TX_NA ,TX_NA ,TX_NA ,}},
/*_25_78125G           */{{0     ,0     ,0     ,TX_NA ,TX_NA }, {0     ,14    ,2     ,TX_NA ,TX_NA }, {TX_NA ,TX_NA ,TX_NA ,TX_NA ,TX_NA ,}},

                    /* 25.3.2020 doron changed PRESET for PAM4 to 0,2,6 performance improved*/
/*_26_5625G_PAM4       */{{0     ,2     ,6     ,0     ,TX_NA }, {0     ,6     ,6     ,0     ,TX_NA }, {0     ,0     ,10    ,3     ,TX_NA ,}}
};
#endif
GT_STATUS mvHwsAvagoSerdesManualTxPresetDBSet
(
    GT_U8                                devNum,
    GT_U16                               serdesNumber,
    GT_U8                                serdesSpeed,
    GT_U8                                pre,
    GT_U8                                atten,
    GT_U8                                post
)
{
    GT_U8 sdSpeed, speedIdx;
   sdSpeed = serdesSpeed;


    speedIdx = mvHwsSerdesSpeedToIndex(devNum,sdSpeed);
    if (speedIdx == UNPERMITTED_SD_SPEED_INDEX)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"serdes speed not supported for serdes tx params offset API");
    }
    if (serdesNumber >= MV_HWS_IPC_MAX_PORT_NUM)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"serdes number not supported for serdes tx params offset API");
    }

    hwsAvagoSerdesPresetParamsManualConfigDb[speedIdx][serdesNumber].pre = pre;
    hwsAvagoSerdesPresetParamsManualConfigDb[speedIdx][serdesNumber].atten = atten;
    hwsAvagoSerdesPresetParamsManualConfigDb[speedIdx][serdesNumber].post = post;
    return GT_OK;
}
#endif

/**
* @internal mvHwsAvagoSerdesManualDBClear function
* @endinternal
*
* @brief   Clear Avago serdes manual db
*
* @param[in] devNum                   - device number
* @param[in] phyPortNum               - port number
* @param[in] serdesNumber             - serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualDBClear
(
    GT_U8                                devNum,
    GT_U32                               phyPortNum,
    GT_U16                               serdesNumber
)
{
#ifndef BOBK_DEV_SUPPORT
    GT_U32   speedIdx;
#endif

    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(phyPortNum);
    if ( serdesNumber >=  MV_HWS_IPC_MAX_PORT_NUM )
    {
        return GT_BAD_PARAM;
    }

#ifndef BOBK_DEV_SUPPORT
    for (speedIdx = 0; speedIdx < MV_HWS_LANE_SERDES_SPEED_NUM; speedIdx++)
    {
        hwsOsMemSetFuncPtr(&mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNumber], 0, sizeof(MV_HWS_AVAGO_TXRX_OVERRIDE_CONFIG_DATA));
#ifdef RAVEN_DEV_SUPPORT
        hwsOsMemSetFuncPtr(&hwsAvagoSerdesPresetParamsManualConfigDb[speedIdx][serdesNumber], TX_NA, sizeof(MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA));
#endif
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesManualRxDBSet function
* @endinternal
*
* @brief   Initialize Avago rx related configurations
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] phyPortNum               - port number
* @param[in] serdesSpeed              - port speed
* @param[in] serdesNumber             - serdes number
* @param[in] rxConfigPtr              - rx config params
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualRxDBSet
(
    GT_U8                                devNum,
    GT_U32                               portGroup,
    GT_U32                               phyPortNum,
    MV_HWS_SERDES_SPEED                  serdesSpeed,
    GT_U16                               serdesNumber,
    MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA *rxConfigPtr
)
{
#ifndef BOBK_DEV_SUPPORT
    GT_U8 sdSpeed, speedIdx;
#endif

    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(phyPortNum);

#ifndef BOBK_DEV_SUPPORT
    sdSpeed = serdesSpeed;

    if (rxConfigPtr == NULL) {
        return GT_BAD_PTR;
    }

    if (serdesNumber >= MV_HWS_IPC_MAX_PORT_NUM)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"serdes number not supported for serdes tx params offset API");
    }

    speedIdx = mvHwsSerdesSpeedToIndex(devNum, sdSpeed);
    if (speedIdx >= MV_HWS_LANE_SERDES_SPEED_NUM)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"serdes speed not supported for serdes tx params offset API");
    }

    mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNumber].valid |= MV_HWS_SERDES_TXRX_TUNE_PARAMS_DB_RX_VALID;
    hwsMemCpy(&mvHwsAvagoSerdesManualConfigDb[speedIdx][serdesNumber].rxData,
              rxConfigPtr, sizeof(MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA));
#endif
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesManualInterconnectDBSet function
* @endinternal
*
* @brief   Initialize interconnect profile configurations
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] phyPortNum               - port number
* @param[in] serdesNumber             - serdes number
* @param[in] interconnectProfile      - interconnect profile
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualInterconnectDBSet
(
    GT_U8                                devNum,
    GT_U32                               portGroup,
    GT_U32                               phyPortNum,
    GT_U16                               serdesNumber,
    GT_U8                                interconnectProfile
)
{
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT)) || defined (FALCON_DEV_SUPPORT)
    GT_U8 i;
#endif

    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(phyPortNum);
    GT_UNUSED_PARAM(serdesNumber);
    GT_UNUSED_PARAM(interconnectProfile);

#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT)) || defined (FALCON_DEV_SUPPORT)
    for (i = 0; i < MV_HWS_LANE_SERDES_SPEED_NUM; i++)
    {
        mvHwsAvagoSerdesManualConfigDb[i][serdesNumber].interconnectProfile = interconnectProfile;
    }

#endif
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesManualInterconnectDBGet function
* @endinternal
*
* @brief   Get interconnect profile configurations
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] phyPortNum               - port number
* @param[in] serdesNumber             - serdes number
* @param[out] interconnectProfile      - interconnect profile
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualInterconnectDBGet
(
    GT_U8                                devNum,
    GT_U32                               portGroup,
    GT_U32                               phyPortNum,
    GT_U16                               serdesNumber,
    GT_U32                               *interconnectProfilePtr
)
{
    /* avoid warnings */
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(phyPortNum);
    GT_UNUSED_PARAM(serdesNumber);
    GT_UNUSED_PARAM(interconnectProfilePtr);

#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT))
    *interconnectProfilePtr = mvHwsAvagoSerdesManualConfigDb[0][serdesNumber].interconnectProfile;
#endif
    return GT_OK;
}

GT_STATUS mvHwsAvagoSerdesCalcLevel3
(
    GT_U8                                devNum,
    GT_U32                               portGroup,
    GT_U16                               serdesNum
)
{
    GT_32 Level_x3x_even, Level_x3x_odd;
    GT_STATUS rc;
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x2C, 0x1F06, &Level_x3x_even));
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x2C, 0x1F07, &Level_x3x_odd));

    Level_x3x_odd = (Level_x3x_odd+Level_x3x_even)/2;
    Level_x3x_odd = (4*Level_x3x_odd)/10+90;
    rc = mvHwsAvago16nmSerdesHalSet(devNum,  serdesNum, HWS_SERDES_GRADIENT_INPUTS, 0x14, Level_x3x_odd);
    return rc;
}


/**
* @internal mvHwsAvagoSerdesOperation function
* @endinternal
*
* @brief   Sets on port extra operations needed
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] operation                - operation to do
* @param[in] data                     - data
* @param[out] result                  - return data
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
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
#ifndef ASIC_SIMULATION
    GT_BOOL needRetrain = GT_FALSE;
    GT_32   calStatus, dfeCommon;
    MV_HWS_SERDES_CONFIG_STC   serdesConfig;
    MV_HWS_PORT_INIT_PARAMS    curPortParams;
#endif

    if ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum))
    {
        return GT_BAD_PARAM;
    }

    if ( result != NULL) {
        *result = 0;
    }
#ifndef ASIC_SIMULATION

    /* on each related serdes */
    switch ( operation ) {
        case MV_HWS_PORT_SERDES_OPERATION_PRECODING_E:
           /* Set PMD PRBS Sequence (PMD Lane ID #) based on Value */
            if (HWS_PAM4_MODE_CHECK(portMode))
            {
                /* set precoding */
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(serdesNum),
                        0x3d, 0x2400 , NULL));
            }
            break;

        case MV_HWS_PORT_SERDES_OPERATION_DFE_COMMON_E:
            /* Set PMD PRBS Sequence (PMD Lane ID #) based on Value */
            if (HWS_PAM4_MODE_CHECK(portMode))
            {
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(serdesNum), 0x2C, 0x11B, &calStatus));
                if (ABS(calStatus & 0x7FFF) > 2) {
                    /* read dfe common and reduce 1*/
                    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(serdesNum), 0x409e, 0, &dfeCommon));
                    dfeCommon = ((dfeCommon>>2)&0x7);
                    needRetrain = GT_TRUE;
                    if ( dfeCommon > 0) {
                        dfeCommon--;
                        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(serdesNum), 0x809e, 0x220+(dfeCommon<<2), NULL));
                    }
                }
            }
            if ( result != NULL) {
                *result = needRetrain;
            }
            break;

        case MV_HWS_PORT_SERDES_OPERATION_CALC_LEVEL3_E:
            /* Set PMD PRBS Sequence (PMD Lane ID #) based on Value */
            if (HWS_PAM4_MODE_CHECK(portMode))
            {
                mvHwsAvagoSerdesCalcLevel3(devNum, portGroup, MV_HWS_SERDES_NUM(serdesNum));
            }
            break;

        case MV_HWS_PORT_SERDES_OPERATION_RESET_RXTX_E:
            hwsPortModeParamsGetToBuffer(devNum, portGroup, data[0], NON_SUP_MODE, &curPortParams);
            serdesConfig.serdesType  = AVAGO_16NM;
            serdesConfig.baudRate    = curPortParams.serdesSpeed;
            if(PRV_CPSS_OPTICAL_PORT_MODE_MAC(curPortParams.portStandard))
            {
                serdesConfig.opticalMode = GT_TRUE;
            }
            else
            {
                serdesConfig.opticalMode = GT_FALSE;
            }

            hwsAvagoSerdesTxRxTuneParamsSet(devNum, portGroup, MV_HWS_SERDES_NUM(serdesNum), &serdesConfig);

            break;

        default:
            break;
    }

#endif
    return GT_OK;
}

/*******************************************************************************/
/******************************* DEBUG functions *******************************/
/*******************************************************************************/

GT_STATUS mvHwsAvagoSerdesTxOffsetParamsDbgTest(GT_U32 v0, GT_U32 v1, GT_U32 v2, GT_U32 v3);
/*GT_STATUS mvHwsAvagoSerdesTxOffsetParamsDbgDumpArray(GT_U32 addr);*/
GT_STATUS dbgSpicoInterrupt(GT_U32 serdesNum, unsigned int interNum, unsigned int interData);

#ifndef MV_HWS_FREE_RTOS
GT_STATUS mvHwsAvagoFalconDbgSerdesSpicoInterrupt
(
    IN GT_U8     devNum,
    IN GT_U32    chipIndex,
    IN GT_U32    serdesNum,
    IN GT_U32    interruptCode,
    IN GT_U32    interruptData
)
{
    GT_U32  sbus_addr;
    GT_U32 result;

    if (serdesNum == 0xFD)
    {
        sbus_addr = 0xfd;
    }
    else if (serdesNum == 0xEE)
    {
        sbus_addr = 0xee;
    }
    else
    {
        CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));
    }

    AVAGO_LOCK(devNum, chipIndex);

     result = avago_spico_int(aaplSerdesDb[chipIndex], sbus_addr, interruptCode, interruptData);

    AVAGO_UNLOCK(devNum, chipIndex);

    CHECK_AVAGO_RET_CODE();

    hwsOsPrintf("\n result 0x%08x ", result);

    return GT_OK;
}

#include <cpss/common/srvCpu/prvCpssGenericSrvCpuIpc.h>

#ifdef MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG
#define POSITIVE_NEGATIVE_VALUE_DBG(_val) \
    (((_val & 0x10))?                                  \
     ( (GT_8)255 - (GT_8)(((_val & 0xF) *2) - 1) ) :    \
     (_val)*2)
#endif /* MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG */


/* Global macro in order to read tx offset parameters database from FW memory */
/*PRV_SHARED_LAB_SERVICES_DIR_PORT_SRC_GLOBAL_VAR(hwsIpcChannelHandlers)*/

/* parameters in order to read tx offset parameters database from FW memory */
GT_U32 txParamesOffsetsFwBaseAddr;
GT_U32 txParamesOffsetsFwSize;

#ifdef MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG
/**
* @internal mvHwsAvagoSerdesTxOffsetParamsDbgTest function
* @endinternal
*
* @brief   This API tests the txOffsets database, it's set API and the affect on the
*         serdes TX parameters which should match the values under test.
*         Note: This API needs to be compiled with the database and the set API.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Caelum; Aldrin.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxOffsetParamsDbgTest
(
    GT_U32 v0,
    GT_U32 v1,
    GT_U32 v2,
    GT_U32 v3
)
{
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT  configParams;
    GT_STATUS rc;
#ifdef MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_IMPLEMENTATION_A
    GT_U16 val1, val2, val3;
#else
    GT_U16 val0;
#endif
    GT_U8 speedIndex;
    GT_U32 portNum;
    GT_U32 serdesNum;
    GT_U8 i,j;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    speedIndex = 0;

    for (portNum=0; portNum<72; portNum++)
    {
        if (GT_OK != hwsPortModeParamsGetToBuffer(0, 0, portNum, _10GBase_KR, &curPortParams))
        {
            continue;
        }

        for (i = 0; i < curPortParams.numOfActLanes; i++)
        {
            serdesNum = curPortParams.activeLanesList[i];
#ifdef MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_IMPLEMENTATION_A
            val1 = v1/*9*/;
            val2 = v2/*9*/;
            val3 = v3/*9*/;
            mvHwsAvagoSerdesTxOffsetParamsSet(serdesNum, speedIndex, val1, 0);
            mvHwsAvagoSerdesTxOffsetParamsSet(serdesNum, speedIndex, val2, 1);
            mvHwsAvagoSerdesTxOffsetParamsSet(serdesNum, speedIndex, val3, 2);
#else
            val0 = v0 /*2457*/;
            /*settt2(serdesNum, speedIndex, 2457);*/
            mvHwsAvagoSerdesTxOffsetParamsSet(serdesNum, speedIndex, ((val0)&0x1F), 0);
            mvHwsAvagoSerdesTxOffsetParamsSet(serdesNum, speedIndex, ((val0>>5)&0x1F), 1);
            mvHwsAvagoSerdesTxOffsetParamsSet(serdesNum, speedIndex, ((val0>>10)&0x1F), 2);
#endif
            rc = hwsAvagoSerdesTxRxTuneParamsSet(0,0,serdesNum,curPortParams.serdesSpeed);
            if (rc!= GT_OK)
            {
                hwsOsPrintf("\n hwsAvagoSerdesTxRxTuneParamsSet failed=%d \n",rc);
            }

            rc = mvHwsAvagoSerdesManualTxConfigGet(0,0,serdesNum,&configParams);
            if (rc!= GT_OK)
            {
                hwsOsPrintf("\n mvHwsAvagoSerdesManualTxConfigGet failed=%d \n",rc);
            }

#ifdef MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_IMPLEMENTATION_A
            for (j = SPEED_NA; j < LAST_MV_HWS_SERDES_SPEED; j++)
            {
                if (hwsAvagoSerdesTxRxTuneParamsArray[j].serdesSpeed == curPortParams.serdesSpeed)
                {
                    hwsOsPrintf("\nperforming...\n");
                    if ( ((GT_32)hwsAvagoSerdesTxRxTuneParamsArray[j].txParams.txAmp+(GT_32)POSITIVE_NEGATIVE_VALUE_DBG(val1)) !=
                            (GT_32)configParams.txAmp ||
                         ((GT_32)hwsAvagoSerdesTxRxTuneParamsArray[j].txParams.txEmph0+(GT_32)POSITIVE_NEGATIVE_VALUE_DBG(val2)) !=
                            (GT_32)configParams.txEmph0 ||
                         ((GT_32)hwsAvagoSerdesTxRxTuneParamsArray[j].txParams.txEmph1+(GT_32)POSITIVE_NEGATIVE_VALUE_DBG(val3)) !=
                            (GT_32)configParams.txEmph1 )
                    {
                        hwsOsPrintf("\n Not match!!  %d, %d, %d, %d, %d, %d \n",
                                 val1, val2, val3,
                                 (GT_32)configParams.txAmp,
                                 (GT_32)configParams.txEmph0,
                                 (GT_32)configParams.txEmph1);
                    }
                    break;
                }
            }
#else
            for (j = SPEED_NA; j < LAST_MV_HWS_SERDES_SPEED; j++)
            {
                if (hwsAvagoSerdesTxRxTuneParamsArray[j].serdesSpeed == curPortParams.serdesSpeed)
                {
                    hwsOsPrintf("\nperforming...\n");
                    if ( ( ((GT_32)hwsAvagoSerdesTxRxTuneParamsArray[j].txParams.txAmp+(GT_32)POSITIVE_NEGATIVE_VALUE_DBG(val0&0x1F)) !=
                                (GT_32)configParams.txAmp ) ||
                         ( ((GT_32)hwsAvagoSerdesTxRxTuneParamsArray[j].txParams.txEmph0+(GT_32)POSITIVE_NEGATIVE_VALUE_DBG((val0>>5)&0x1F)) !=
                                (GT_32)configParams.txEmph0 ) ||
                         ( ((GT_32)hwsAvagoSerdesTxRxTuneParamsArray[j].txParams.txEmph1+(GT_32)POSITIVE_NEGATIVE_VALUE_DBG((val0>>10)&0x1F)) !=
                                (GT_32)configParams.txEmph1 ) )
                    {
                        hwsOsPrintf("\n Not match!!  %d, %d, %d, %d, %d, %d \n",
                                 (val0&0x1F), ((val0>>5)&0x1F), ((val0>>10)&0x1F),
                                 (GT_32)configParams.txAmp,
                                 (GT_32)configParams.txEmph0,
                                 (GT_32)configParams.txEmph1);
                    }
                    break;
                }
            }
#endif
        }
    }
    return GT_OK;

}
#endif /* MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG */

/**
* @internal mvHwsAvagoSerdesTxOffsetParamsDbgDumpArray function
* @endinternal
*
* @brief   This API dumps the content of the database from the FW memory.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Caelum; Aldrin.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if 0
GT_STATUS mvHwsAvagoSerdesTxOffsetParamsDbgDumpArray(GT_U32 addr)
{
    GT_U32 i;
    GT_U8 *dataArrPtr;

    if (txParamesOffsetsFwBaseAddr==0 || txParamesOffsetsFwSize==0)
    {
        hwsOsPrintf("\n null pointer or zero size %d, %d", txParamesOffsetsFwBaseAddr, txParamesOffsetsFwSize);
        return GT_FAIL;
    }

    dataArrPtr = (GT_U8 *)hwsOsMallocFuncPtr(txParamesOffsetsFwSize);
    hwsOsPrintf("\n reading offset 0x%08x, size=%d", ((addr == 0) ? txParamesOffsetsFwBaseAddr : addr), txParamesOffsetsFwSize);
    CHECK_STATUS_EXT(prvCpssGenericSrvCpuRead(PRV_SHARED_LAB_SERVICES_DIR_PORT_SRC_GLOBAL_VAR(hwsIpcChannelHandlers)[0], ((addr == 0) ? txParamesOffsetsFwBaseAddr : addr), dataArrPtr, txParamesOffsetsFwSize),
                     LOG_ARG_STRING_MAC("reading tx offsets db from FW memory"));

    hwsOsPrintf("\n\n");
    for (i=0; i<txParamesOffsetsFwSize; i++)
    {
        if (dataArrPtr[i]!=0)
        {
            hwsOsPrintf("arr[%d]={** %d **}", i, dataArrPtr[i]);
        }
        else
        {
            hwsOsPrintf("arr[%d]={%d}", i, dataArrPtr[i]);
        }
        if (i%20==0)
        {
            hwsOsPrintf("\n");
        }
    }
    hwsOsPrintf("\n\n");

    hwsOsFreeFuncPtr(dataArrPtr);

    return GT_OK;
}
#endif
#endif /* MV_HWS_FREE_RTOS */
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
GT_STATUS dbgSpicoInterrupt(GT_U32 serdesNum, unsigned int interNum, unsigned int interData)
{
    int data;

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(0, 0, serdesNum, interNum, interData,  &data));
    hwsOsPrintf("dbgSpicoInterrupt: serdes %d data %d\n",serdesNum, data);
    return GT_OK;
}

GT_STATUS dbgSbusReset(GT_U32 serdesNum, GT_U32 chipIndex)
{
        unsigned int  sbus_addr;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(0, serdesNum, &sbus_addr));
        AVAGO_LOCK(0, chipIndex);
        avago_sbus_wr(aaplSerdesDb[chipIndex], sbus_addr, 0x7, 0x0);
        avago_sbus_wr(aaplSerdesDb[chipIndex], sbus_addr, 0x07, 0x2);
        AVAGO_UNLOCK(0, chipIndex);

        return GT_OK;
}

#endif
