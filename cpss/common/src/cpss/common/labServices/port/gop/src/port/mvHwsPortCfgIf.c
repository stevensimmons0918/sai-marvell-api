/*******************************************************************************
*           Copyright 2001, Marvell International Ltd.
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
* @file mvHwsPortCfgIf.c
*
* @brief This file contains API for port configuartion and tuning parameters
*
* @version   48
********************************************************************************
*/

#ifndef MV_HWS_REDUCED_BUILD
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyHRev2/mvComPhyHRev2If.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#ifndef MV_HWS_REDUCED_BUILD
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpssCommon/private/prvCpssEmulatorMode.h>
#endif
#if !defined(PX_FAMILY) && !defined(PIPE_DEV_SUPPORT)
#include <cpss/generic/labservices/port/gop/silicon/lion2/mvHwsLion2PortIf.h>
#endif
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApDefs.h>

#if defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT) ||(!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && defined(CHX_FAMILY)) /* Falcon */
#include <cpss/generic/labservices/port/gop/silicon/falcon/mvHwsFalconPortIf.h>
#endif
#define NEW_PORT_INIT_SEQ
#define HWS_CTLE_BIT_MAP 32
/**************************** Globals ****************************************************/
static const GT_U32 align90Delay = 60;

extern GT_U32 optPrintLevel;

#ifndef CM3
/* enables/disabled training results copy */
extern GT_BOOL copyTuneResults;
#endif

static const GT_U8 portModeNumArr[] =
    {1,     /* SGMII */
     2,     /* _1000Base_X */
     3,     /* SGMII2_5 */
     4,     /* QSGMII */
     5,     /* _10GBase_KX4 */
     7,     /* _10GBase_KR */
     8,     /* _20GBase_KR2 */
     9,     /* _40GBase_KR */
     14,    /* RXAUI */
     15,    /* _20GBase_KX4 */
     16,    /* _10GBase_SR_LR */
     17,    /* _20GBase_SR_LR2 */
     18,    /* _40GBase_SR_LR */
     19,    /* _12_5GBase_KR */
     20,    /* XLHGL_KR4 */
     44,    /* _12GBaseR */
     45,    /* _5_625GBaseR */
     46,    /* _48GBaseR */
     47,    /* _12GBase_SR */
     48,    /* _48GBase_SR */
     49,    /* _5GBaseR */
     50,    /*_22GBase_SR */
     67,    /* _40GBase_CR4 */
     82,    /* _40GBase_KR4 */
     84     /* _40GBase_SR_LR4 */
    };

static const MV_HWS_MAN_TUNE_COMPHY_H_TX_CONFIG_DATA hwPortTxDefaults[] =
{
  /* PortMode          TxAmp   TxAmpAdj TxEmph0  xEmph1  TxAmpShft  txEmphEn  slewRate  slewCtrlEn */
/* SGMII */            { 19,      1,        0,       0,        0,     0,        0,      0},
/* _1000Base_X */      { 19,      1,        0,       0,        0,     0,        0,      0},
/* SGMII2_5 */         { 20,      1,        0,       0,        0,     0,        0,      0},
/* QSGMII */           { 24,      1,        1,       9,        0,     0,        0,      0},
/* _10GBase_KX4 */     { 20,      1,        0,       0,        0,     0,        0,      0},
/* _10GBase_KR */      { 26,      1,        0,       0,        0,     0,        0,      0},
/* _20GBase_KR2 */     { 26,      1,        0,       0,        0,     0,        0,      0},
/* _40GBase_KR */      { 26,      1,        0,       0,        0,     0,        0,      0},
/* RXAUI */            { 30,      1,        0,       0,        0,     0,        0,      0},
/* _20GBase_KX4 */     { 30,      1,        0,       0,        0,     0,        0,      0},
/* _10GBase_SR_LR */   { 17,      1,        2,       8,        0,     0,        0,      0},
/* _20GBase_SR_LR2 */  { 17,      1,        2,       8,        0,     0,        0,      0},
/* _40GBase_SR_LR */   { 17,      1,        2,       8,        0,     0,        0,      0},
/* _12_5GBase_KR */    { 26,      1,        3,       3,        0,     0,        0,      0},
/* XLHGL_KR4 */        { 26,      1,        3,       3,        0,     0,        0,      0},
/* _12GBaseR */        { 26,      1,        3,       3,        0,     0,        0,      0},
/* _5_625GBaseR */     { 30,      1,        0,       0,        0,     0,        0,      0},
/* _48GBaseR */        { 26,      1,        3,       3,        0,     0,        0,      0},
/* _12GBase_SR */      { 26,      1,        3,       3,        0,     0,        0,      0},
/* _48GBase_SR */      { 26,      1,        3,       3,        0,     0,        0,      0},
/* _5GBaseR */         { 24,      1,        1,       9,        0,     0,        0,      0},
/* _22GBase_SR */      { 26,      1,        3,       3,        0,     0,        0,      0},
/* _40GBase_CR4 */     { 26,      1,        0,       0,        0,     0,        0,      0},
/* _40GBase_KR4 */     { 26,      1,        0,       0,        0,     0,        0,      0},
/* _40GBase_SR_LR4 */  { 17,      1,        2,       8,        0,     0,        0,      0}
};

#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && defined  CHX_FAMILY ) || defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT)
static const GT_U32 falconMuxSeq[][2] =
{ /* macNum               regAddr           macFieldOffset */
    /*  0  */  { DEVICE_GENERAL_CONTROL_19,        0        },
    /*  1  */  { DEVICE_GENERAL_CONTROL_19,        6        },
    /*  2  */  { DEVICE_GENERAL_CONTROL_20,        0        },
    /*  3  */  { DEVICE_GENERAL_CONTROL_20,        6        },
    /*  4  */  { DEVICE_GENERAL_CONTROL_20,        12       },
    /*  5  */  { DEVICE_GENERAL_CONTROL_20,        18       },
    /*  6  */  { DEVICE_GENERAL_CONTROL_20,        24       },
    /*  7  */  { DEVICE_GENERAL_CONTROL_21,        0        },
    /*  8  */  { DEVICE_GENERAL_CONTROL_21,        6        },
    /*  9  */  { DEVICE_GENERAL_CONTROL_21,        12       },
    /* 10  */  { DEVICE_GENERAL_CONTROL_21,        18       },
    /* 11  */  { DEVICE_GENERAL_CONTROL_21,        24       },
    /* 12  */  { DEVICE_GENERAL_CONTROL_22,        0        },
    /* 13  */  { DEVICE_GENERAL_CONTROL_22,        6        },
    /* 14  */  { DEVICE_GENERAL_CONTROL_22,        12       },
    /* 15  */  { DEVICE_GENERAL_CONTROL_22,        18       },
};
#endif
/**************************** Definition *************************************************/
/* #define GT_DEBUG_HWS */
#ifdef  GT_DEBUG_HWS
#include <common/os/gtOs.h>
#define DEBUG_HWS_FULL(level,s) if (optPrintLevel >= level) {hwsOsPrintf s;}
#else
#define DEBUG_HWS_FULL(level,s)
#endif

/**************************** Pre-Declaration ********************************************/

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPCSMarkModeSet function
* @endinternal
*
* @brief   Mark/Un-mark PCS unit
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] enable                   - GT_TRUE  for mark the PCS,
*                                      GT_FALSE for un-mark the PCS
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*/
static GT_STATUS mvHwsPCSMarkModeSet
(
   IN GT_U8                   devNum,
   IN GT_UOPT                 portGroup,
   IN GT_U32                  phyPortNum,
   IN MV_HWS_PORT_STANDARD    portMode,
   IN GT_BOOL                 enable
)
{
   MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

   hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

   if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc == NULL)
   {
           return GT_OK;
   }

   return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc(devNum, portGroup, phyPortNum, portMode, enable);
}

/**
* @internal mvHwsPortPartialPowerDown function
* @endinternal
*
* @brief   Enable/Disable the power down Tx and Rx of on Port.
*         The configuration performs Enable/Disable of Tx and Rx on specific Serdes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] powerDownRx              - Enable/Disable the Port Rx power down
* @param[in] powerDownTx              - Enable/Disable the Port Tx power down
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPartialPowerDown
(
    IN GT_U8                 devNum,
    IN GT_U32                portGroup,
    IN GT_U32                phyPortNum,
    IN MV_HWS_PORT_STANDARD  portMode,
    IN GT_BOOL               powerDownRx,
    IN GT_BOOL               powerDownTx
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        /* Enable/Disable power down of Tx and Rx on Serdes */
        CHECK_STATUS(mvHwsSerdesPartialPowerDown(devNum, portGroup, (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), powerDownRx, powerDownTx));
    }

    return GT_OK;
}

/**
* @internal mvHwsPortPartialPowerStatusGet function
* @endinternal
*
* @brief   Get the status of power Tx and Rx on port
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] powerRxStatus            - GT_TRUE - Serdes power Rx is down
*                                      GT_FALSE - Serdes power Rx is up
* @param[in] powerTxStatus            - GT_TRUE - Serdes power Tx is down
*                                      GT_FALSE - Serdes power Tx is up
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPartialPowerStatusGet
(
    IN GT_U8                 devNum,
    IN GT_U32                portGroup,
    IN GT_U32                phyPortNum,
    IN MV_HWS_PORT_STANDARD  portMode,
    IN GT_BOOL               *powerRxStatus,
    IN GT_BOOL               *powerTxStatus
)
{
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* Get the status of Serdes Tx and Rx power */
    CHECK_STATUS(mvHwsSerdesPartialPowerStatusGet(devNum, portGroup, (curLanesList[0] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[0] & 0xFFFF)),
                                                  powerRxStatus, powerTxStatus));

    return GT_OK;
}

/**
* @internal hwsPortTxAutoTuneStartSet function
* @endinternal
*
* @brief   Sets the port Tx only parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port TX related tuning mode
* @param[in] optAlgoMask              - Opt algorithm
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortTxAutoTuneStartSet
(
   IN GT_U8                        devNum,
   IN GT_U32                       portGroup,
   IN GT_U32                       phyPortNum,
   IN MV_HWS_PORT_STANDARD         portMode,
   IN MV_HWS_PORT_AUTO_TUNE_MODE   portTuningMode,
   IN GT_U32                       optAlgoMask
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   MV_HWS_AUTO_TUNE_STATUS txStatus;
   GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
   GT_BOOL laneFail = GT_FALSE;
   GT_U32 i;
   MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

   /* avoid warnings */
   optAlgoMask = optAlgoMask;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
       ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

    if ((portTuningMode == RxTrainingAdative) ||
        (portTuningMode == RxTrainingStopAdative))
    {
        return GT_NOT_SUPPORTED;
    }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   /* TRxTuneCfg flow */
   if (portTuningMode == TRxTuneCfg)
   {
        /* on each related serdes */
        for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
        {
            CHECK_STATUS(mvHwsSerdesAutoTuneCfg(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)),
                                                GT_FALSE, GT_TRUE));
        }

#if !defined(BC2_DEV_SUPPORT) && !defined(AC3_DEV_SUPPORT) && !defined(AC5_DEV_SUPPORT) && !defined(CM3) && defined(CHX_FAMILY)
        if ((HooperA0 == hwsDeviceSpecInfo[devNum].devType) || (Lion2B0 == hwsDeviceSpecInfo[devNum].devType))
        {
            CHECK_STATUS(mvHwsComHRev2SerdesPostTrainingConfig(devNum, portGroup,
                                curPortParams.numOfActLanes, curLanesList));
        }
#endif /* #if !defined(BC2_DEV_SUPPORT) && !defined(AC3_DEV_SUPPORT) && !defined(CM3) && defined(CHX_FAMILY) */

        return GT_OK;
  }

   /* TRxTuneStart flow */
   if (portTuningMode == TRxTuneStart)
   {
           /* on each related serdes */
           for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
           {
                   /* enable TX tunning */
                   CHECK_STATUS(mvHwsSerdesTxAutoTuneStart(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), GT_TRUE));
           }

           /* mark PCS to be under TRx training */
           CHECK_STATUS(mvHwsPCSMarkModeSet(devNum, portGroup, phyPortNum, portMode, GT_TRUE));

           /* Reset MAC */
           CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode,
                                           curPortParams.portMacType, RESET));

           /* Reset PCS */
           CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode,
                                           curPortParams.portPcsType, RESET));

           /* on each related Serdes apply Rf reset */
           for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
           {
                   CHECK_STATUS(mvHwsSerdesDigitalReset(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), RESET));
           }

           return GT_OK;
   }

   /* TRxTuneStatus flow */
   for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
   {
           txStatus = TUNE_PASS;
           mvHwsSerdesAutoTuneStatus(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), NULL, &txStatus);
           if (txStatus != TUNE_PASS)
           {
                   laneFail = GT_TRUE;
           }
   }

   if (laneFail == GT_TRUE)
   {
           /* if training failed - unreset all units to allow farther configurations */
           CHECK_STATUS(mvHwsPortPostTraining(devNum, portGroup, phyPortNum, portMode));
           return GT_FAIL;
   }

   if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetWaFunc != NULL)
   {
           hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetWaFunc(devNum, portGroup, phyPortNum, portMode, &(curLanesList[0])) ;
   }

   /* UnReset RF on each related Serdes */
   for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
   {
           CHECK_STATUS(mvHwsSerdesDigitalReset(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)), (curLanesList[i] & 0xFFFF),
                                            HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), UNRESET));
   }

   /* UnReset PCS because FixAlign90 need signal */
   CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode,
                                           curPortParams.portPcsType, UNRESET));

   return GT_OK;
}

/**
* @internal hwsPortTxAutoTuneActivateSet function
* @endinternal
*
* @brief   Unreset MAC and PCS after TRx training
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port TX related tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortTxAutoTuneActivateSet
(
    IN GT_U8                      devNum,
    IN GT_U32                     portGroup,
    IN GT_U32                     phyPortNum,
    IN MV_HWS_PORT_STANDARD       portMode,
    IN MV_HWS_PORT_AUTO_TUNE_MODE portTuningMode
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* avoid warnings */
   portTuningMode = portTuningMode;

   /* UnReset MAC: at this point FixAlign90 already finished and we are after TRX-Training */
   CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode,
                                           curPortParams.portMacType, UNRESET));

   /* unmark PCS to be under TRx training */
   CHECK_STATUS(mvHwsPCSMarkModeSet(devNum, portGroup, phyPortNum, portMode, GT_FALSE));

   return GT_OK;
}
#endif
/**
* @internal mvHwsPortAutoTuneSet function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port tuning mode
*
* @param[out] results                 - results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroup,
    IN GT_U32                       phyPortNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN MV_HWS_PORT_AUTO_TUNE_MODE   portTuningMode,
    OUT void *                      results
)
{
   GT_U32 serdesOptAlgoMask = 0x0;

   return mvHwsPortAutoTuneSetExt(devNum, portGroup, phyPortNum, portMode,
                                           portTuningMode, serdesOptAlgoMask, results);
}

/**
* @internal mvHwsPortAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port tuning mode
* @param[in] optAlgoMask              - bit mask for optimization algorithms
*
* @param[out] results                 - results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneSetExt
(
   IN GT_U8                       devNum,
   IN GT_U32                      portGroup,
   IN GT_U32                      phyPortNum,
   IN MV_HWS_PORT_STANDARD        portMode,
   IN MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
   IN GT_U32                      optAlgoMask,
   OUT void *                     results
)
{
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetExtFunc == NULL)
    {
        return GT_OK;
    }

#ifndef MV_HWS_REDUCED_BUILD
    if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
        return (mvHwsPortAutoTuneSetExtIpc(devNum, portGroup, phyPortNum, portMode, portTuningMode, optAlgoMask, results));
#endif

    return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetExtFunc(devNum, portGroup, phyPortNum,
                                                                           portMode, (MV_HWS_PORT_AUTO_TUNE_MODE)portTuningMode,
                                                                           optAlgoMask,  results);
}
/**
* @internal mvHwsPortOperation function
* @endinternal
*
* @brief   Sets operation on serdes that belongs to the port
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] operation                - port operation mode
* @param[in] dataPtr                     - data
*
* @param[out] resultPtr                 - results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortOperation
(
   IN GT_U8                       devNum,
   IN GT_U32                      portGroup,
   IN GT_U32                      macPortNum,
   IN MV_HWS_PORT_STANDARD        portMode,
   IN MV_HWS_PORT_SERDES_OPERATION_MODE_ENT  operation,
   IN GT_U32                     *dataPtr,
   OUT GT_U32                    *resultPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i,tempResult = 0;


    if ((macPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)) ||
        (dataPtr == NULL) || (resultPtr == NULL) )
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, macPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }
    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, macPortNum, portMode, curLanesList));
    if (operation == MV_HWS_PORT_SERDES_OPERATION_RESET_RXTX_E)
    {
        dataPtr[0] = macPortNum;
    }

    /* on each related serdes*/
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
       CHECK_STATUS(mvHwsSerdesOperation(devNum, portGroup, curLanesList[i], portMode, operation, dataPtr, &tempResult));
       if ( tempResult != 0 ) {
           if ( (*resultPtr != 0) && (*resultPtr != tempResult)) {
               return GT_BAD_PARAM;
           }
           *resultPtr = tempResult;
           tempResult = 0;
       }
   }
   return GT_OK;

}

/**
* @internal mvHwsPortManTuneSet function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port tuning mode
* @param[in] tunParams                - pointer to tune parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManTuneSet
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_MAN_TUNE_MODE       portTuningMode,
    IN MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT        *tuneParams
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT  manTxTuneStcPtr;
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT  manRxTuneStcPtr;

    portTuningMode = portTuningMode;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    hwsOsMemSetFuncPtr(&manRxTuneStcPtr, 0, sizeof(manTxTuneStcPtr));
    hwsOsMemSetFuncPtr(&manTxTuneStcPtr, 0, sizeof(manTxTuneStcPtr));

#ifndef ALDRIN_DEV_SUPPORT
    if (HWS_DEV_GOP_REV(devNum) <= GOP_28NM_REV2)
    {
        manTxTuneStcPtr.txComphyH.txAmp        = tuneParams->comphyHResults.txAmp;
        manTxTuneStcPtr.txComphyH.txAmpAdj     = GT_TRUE;
        manTxTuneStcPtr.txComphyH.txAmpShft    = tuneParams->comphyHResults.txAmpShft;
        manTxTuneStcPtr.txComphyH.txEmph0      = tuneParams->comphyHResults.txEmph0;
        manTxTuneStcPtr.txComphyH.txEmph1      = tuneParams->comphyHResults.txEmph1;
        manTxTuneStcPtr.txComphyH.slewCtrlEn   = tuneParams->comphyHResults.slewCtrlEn;
        manTxTuneStcPtr.txComphyH.slewRate     = tuneParams->comphyHResults.slewRate;

        manRxTuneStcPtr.rxComphyH.sqlch           = tuneParams->comphyHResults.sqleuch;
        manRxTuneStcPtr.rxComphyH.ffeRes          = tuneParams->comphyHResults.ffeR;
        manRxTuneStcPtr.rxComphyH.ffeCap          = tuneParams->comphyHResults.ffeC;
        manRxTuneStcPtr.rxComphyH.dfeEn           = GT_TRUE;
        manRxTuneStcPtr.rxComphyH.alig            = tuneParams->comphyHResults.align90;
        manRxTuneStcPtr.rxComphyH.portTuningMode  = portTuningMode;
    }
    else
#endif
    {
        manTxTuneStcPtr.txAvago.atten = tuneParams->avagoResults.avagoStc.atten;
        manTxTuneStcPtr.txAvago.post  = tuneParams->avagoResults.avagoStc.post;
        manTxTuneStcPtr.txAvago.pre   = tuneParams->avagoResults.avagoStc.pre;
        manTxTuneStcPtr.txAvago.pre2  = tuneParams->avagoResults.avagoStc.pre2;
        manTxTuneStcPtr.txAvago.pre3  = tuneParams->avagoResults.avagoStc.pre3;
    }

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS(mvHwsSerdesManualTxConfig(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                               (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &manTxTuneStcPtr));
        CHECK_STATUS(mvHwsSerdesManualRxConfig(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                               (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &manRxTuneStcPtr));
    }

    return GT_OK;
}

/**
* @internal mvHwsPortManualRxConfig function
* @endinternal
*
* @brief   configures SERDES Rx parameters for all SERDES lanes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port tuning mode
* @param[in] configParams             - pointer to array of the
*                                        config params structures
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualRxConfig
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   portGroup,
    IN GT_U32                                   phyPortNum,
    IN MV_HWS_PORT_STANDARD                     portMode,
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT         *configParams
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum))
        || (HWS_DEV_SILICON_TYPE(devNum) == BobK))
    {
        return GT_BAD_PARAM;
    }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS(mvHwsSerdesManualRxConfig(devNum, portGroup,(curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), configParams));
    }

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetect function
* @endinternal
*
* @brief   Detect TxAmp peak for positive or negative changing
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum  - physical port number
*                                      portMode    - port standard metric
* @param[in] serdesNum                -  serdes number
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
);

/**
* @internal mvHwsPortManualTxConfig function
* @endinternal
*
* @brief   configures SERDES tx parameters for all SERDES lanes.
*           MV_HWS_MAN_TUNE_COMPHY_H_TX_CONFIG_DATA - ComPhyH params
*           MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA - AVAGO params
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] configParams             - pointer to array of the
*                                       config params structures
* @param[in] manTxTuneStcPtr          - pointer to Tx params
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualTxConfig
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    IN MV_HWS_PORT_MAN_TUNE_MODE       portTuningMode,
    IN MV_HWS_SERDES_TX_CONFIG_DATA_UNT  *manTxTuneStcPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    portTuningMode = portTuningMode; /* avoid warnings */

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
       CHECK_STATUS(mvHwsSerdesManualTxConfig(devNum, portGroup,(curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), manTxTuneStcPtr));
   }

    return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPortManualTxConfigGet function
* @endinternal
*
* @brief   Get SERDES tx parameters for first SERDES lane.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port tuning mode
*
* @param[out] configParams             pointer to array of the config params structures:
*                                      txAmp     Tx amplitude
*                                      txAmpAdj  TX adjust
*                                      txemph0   TX emphasis 0
*                                      txemph1   TX emphasis 1
*                                      txAmpShft TX amplitude shift
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortManualTxConfigGet
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    IN MV_HWS_PORT_MAN_TUNE_MODE       portTuningMode,
    OUT MV_HWS_SERDES_TX_CONFIG_DATA_UNT *manTxTuneStcPtr
)
{
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];

    portTuningMode = portTuningMode; /* avoid warnings */

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on first lane related serdes */
    CHECK_STATUS(mvHwsSerdesManualTxConfigGet(devNum, portGroup, (curLanesList[0] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[0] & 0xFFFF)), manTxTuneStcPtr));

    return GT_OK;
}
#endif
#ifndef ALDRIN_DEV_SUPPORT
/**
* @internal mvHwsPortTxDefaultsSet function
* @endinternal
*
* @brief   Set the default Tx parameters on port.
*         Relevant for Bobcat2 and Alleycat3 devices.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTxDefaultsSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_MAN_TUNE_MODE       portTuningMode;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;
    GT_U8 arrSize;
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT  manTxTuneValues;

    portTuningMode = StaticLongReach;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    /* relevant for Bobcat2 and Alleycat3 devices */
    if ((HWS_DEV_GOP_REV(devNum) != GOP_28NM_REV2) && (HWS_DEV_GOP_REV(devNum) != GOP_28NM_REV1))
    {
        return GT_NOT_SUPPORTED;
    }

    if(HWS_DEV_SERDES_TYPE(devNum, curPortParams.activeLanesList[0]) > D2D_PHY_GW16)
    {
        return GT_NOT_SUPPORTED;
    }

    /* validation of PortNum and portMode */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    arrSize =  (sizeof(portModeNumArr)/sizeof(GT_U8));
    for (i=0; i < arrSize; i++)
    {
        if (portMode == portModeNumArr[i])
            break;
    }
    if ( i == arrSize )
    {
        return GT_NOT_FOUND;
    }

    hwsOsMemSetFuncPtr(&manTxTuneValues, 0, sizeof(manTxTuneValues));

    manTxTuneValues.txComphyH.txAmp     = hwPortTxDefaults[i].txAmp;
    manTxTuneValues.txComphyH.txAmpAdj  = hwPortTxDefaults[i].txAmpAdj;
    manTxTuneValues.txComphyH.txAmpShft = hwPortTxDefaults[i].txAmpShft;
    manTxTuneValues.txComphyH.txEmph0   = hwPortTxDefaults[i].txEmph0;
    manTxTuneValues.txComphyH.txEmph1   = hwPortTxDefaults[i].txEmph1;
    /* configure defaults Tx parameter values on port */
    CHECK_STATUS(mvHwsPortManualTxConfig(devNum, portGroup, phyPortNum, portMode, portTuningMode, &manTxTuneValues));

    /* perform Serdes Digital Reset/Unreset */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        CHECK_STATUS(mvHwsSerdesReset(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curLanesList[i])),
                                      GT_FALSE, GT_TRUE, GT_FALSE));
        /* delay 1ms */
        hwsOsExactDelayPtr(devNum, portGroup, 1);

        CHECK_STATUS(mvHwsSerdesReset(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curLanesList[i])),
                                      GT_FALSE, GT_FALSE, GT_FALSE));
    }

    return GT_OK;
}
#endif
/**
* @internal mvHwsPortTestGenerator function
* @endinternal
*
* @brief   Activate the port related PCS Tx generator and Rx checker control.
*         Can be run any time after create port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portPattern              - port tuning mode
* @param[in] actionMode               - action mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTestGenerator
(
   IN GT_U8                           devNum,
   IN GT_U32                          portGroup,
   IN GT_U32                          phyPortNum,
   IN MV_HWS_PORT_STANDARD            portMode,
   IN MV_HWS_PORT_TEST_GEN_PATTERN    portPattern,
   IN MV_HWS_PORT_TEST_GEN_ACTION     actionMode
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;

   actionMode = actionMode;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* perform MAC RESET */
   CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, RESET));

   /* perform PCS RESET */
   CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, curPortParams.portPcsType, RESET));

   CHECK_STATUS(mvHwsPcsTestGenCfg(devNum, portGroup, curPortParams.portPcsNumber, curPortParams.portPcsType, portPattern));

   /* perform PCS UNRESET */
   CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, curPortParams.portPcsType, UNRESET));

   /* perform MAC UNRESET */
   CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, UNRESET));

   return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPortTestGeneratorStatus function
* @endinternal
*
* @brief   Get test errors - every get clears the errors.
*         Can be run any time after delete port or after power up
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portPattern              - port pattern
* @param[out] status                  - status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTestGeneratorStatus
(
   IN GT_U8                           devNum,
   IN GT_U32                          portGroup,
   IN GT_U32                          phyPortNum,
   IN MV_HWS_PORT_STANDARD            portMode,
   IN MV_HWS_PORT_TEST_GEN_PATTERN    portPattern,
   OUT MV_HWS_TEST_GEN_STATUS         *status
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   CHECK_STATUS(mvHwsPcsTestGenStatus(devNum, portGroup, curPortParams.portPcsNumber,
                                                   curPortParams.portPcsType, portPattern, status));

   return GT_OK;
}
#endif
/**
* @internal  mvHwsPortPPMSet function
* @endinternal
*
* @brief   Increase/decrease  Tx clock on port (added/sub ppm).
*       Can be run only after create port not under traffic.up
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portPPM                  - limited to +/- 3 taps
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))/*AC5-TBD*/
GT_STATUS mvHwsPortPPMSet
(
   IN GT_U8                   devNum,
   IN GT_U32                  portGroup,
   IN GT_U32                  phyPortNum,
   IN MV_HWS_PORT_STANDARD    portMode,
   IN MV_HWS_PPM_VALUE        portPPM
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i, tmpPortNum;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortPPMSetIpc(devNum,portGroup, phyPortNum,portMode,portPPM));
#endif

   if(QSGMII == portMode)
   {
       tmpPortNum = phyPortNum & 0xFFFFFFFC; /* only the first port in the quad is available*/
   }
   else
   {
       tmpPortNum = phyPortNum;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* MAC Reset */
   CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, RESET));

   /* PCS Reset */
   CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, tmpPortNum, portMode, curPortParams.portPcsType, RESET));

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, tmpPortNum, portMode, curLanesList));

   /* on each related serdes */
   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
           CHECK_STATUS(mvHwsSerdesPpmConfig(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                            (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), portPPM));
   }

   /* PCS UnReset */
   CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, tmpPortNum, portMode, curPortParams.portPcsType, UNRESET));

   /* MAC UnReset */
   CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, UNRESET));

   return GT_OK;
}

/**
* @internal mvHwsPortPPMGet function
* @endinternal
*
* @brief   Check the entire line configuration, return ppm value in case of match in all
*         or error in case of different configuration.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] portPPM                 - current PPM
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPPMGet
(
    IN GT_U8                     devNum,
    IN GT_U32                    portGroup,
    IN GT_U32                    phyPortNum,
    IN MV_HWS_PORT_STANDARD      portMode,
    OUT MV_HWS_PPM_VALUE         *portPPM
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   MV_HWS_PPM_VALUE lanePpm, curPpm;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i, tmpPortNum;

   if ((portPPM == NULL) ||
       (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
       ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortPPMGetIpc(devNum,portGroup, phyPortNum,portMode,portPPM));
#endif

    if(QSGMII == portMode)
    {
        tmpPortNum = phyPortNum & 0xFFFFFFFC; /* only the first port in the quad is available*/
    }
    else
    {
        tmpPortNum = phyPortNum;
    }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   if (curPortParams.numOfActLanes == 0)
   {
           *portPPM = (MV_HWS_PPM_VALUE)0;
           return GT_FAIL;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, tmpPortNum, portMode, curLanesList));

   /* read first lane PPM */
   CHECK_STATUS(mvHwsSerdesPpmConfigGet(devNum, (portGroup + ((curLanesList[0] >> 16) & 0xFF)),
                                                   (curLanesList[0] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[0] & 0xFFFF)),&lanePpm));
   *portPPM = lanePpm;

   /* on each related serdes */
   for (i = 1; i < curPortParams.numOfActLanes; i++)
   {
           CHECK_STATUS(mvHwsSerdesPpmConfigGet(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &curPpm));
           if (lanePpm != curPpm)
           {
                   return GT_FAIL;
           }
   }

   return GT_OK;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal hwsPortFaultSendSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending fault signals to partner.
*         on single lane, the result will be local-fault on the sender and remote-fault on the receiver,
*         on multi-lane there will be local-fault on both sides, and there won't be align lock
*         at either side.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] send                     - send or stop sending
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortFaultSendSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 send
)
{
    GT_U32 pcsNum;
    MV_HWS_PORT_PCS_TYPE    pcsType;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        CHECK_STATUS_EXT(GT_BAD_PARAM,LOG_ARG_GEN_PARAM_MAC(portMode),LOG_ARG_MAC_IDX_MAC(phyPortNum));
    }

#if 0 /* TBD */
#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (hwsPortFaultSendSetIpc(devNum,portGroup, phyPortNum,portMode));
#endif
#endif

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    pcsType = curPortParams.portPcsType;
    pcsNum = curPortParams.portPcsNumber;

    if( ( (pcsType == XPCS) || (pcsType == MMPCS) ) &&
          (HWS_DEV_SILICON_TYPE(devNum) == Pipe) )
    {   /* Workaround for XGMII/XAUI/RXAUI - remote fault is not implemented for XPCS */
        /* Apllicable only for Pipe */
        CHECK_STATUS_EXT(  genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, phyPortNum, MSM_PORT_MAC_CONTROL_REGISTER2, (send == GT_TRUE) ? (1<<12) : 0, (1<<12)),
                           LOG_ARG_MAC_IDX_MAC(phyPortNum), LOG_ARG_GEN_PARAM_MAC(portMode));
        return GT_OK;
    }

    CHECK_STATUS_EXT(mvHwsPcsSendFaultSet(devNum, portGroup, pcsNum, pcsType, portMode, send), LOG_ARG_MAC_IDX_MAC(phyPortNum), LOG_ARG_GEN_PARAM_MAC(portMode));

    return GT_OK;
}

/**
* @internal mvHwsPortSendLocalFaultSet function
* @endinternal
*
* @brief   Configure the PCS to Enable/Disable Send Local Fault.
*          This is done by sending fault signals to partner, the result will be local-fault on the
*          sender and remote-fault on the receiver.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] enable                   - GT_TRUE for Fast link DOWN, otherwise normal/UP.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortSendLocalFaultSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 enable
)
{
    GT_U32                  pcsNum;
    MV_HWS_PORT_PCS_TYPE    pcsType;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    /*Validating portnumber and portmode*/
    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        CHECK_STATUS_EXT(GT_BAD_PARAM,LOG_ARG_GEN_PARAM_MAC(portMode),LOG_ARG_MAC_IDX_MAC(phyPortNum));
    }

    /*Get current port params*/
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    pcsType = curPortParams.portPcsType;
    pcsNum = curPortParams.portPcsNumber;

    /* To set/un-set fault signaling */
    CHECK_STATUS_EXT(mvHwsPcsSendLocalFaultSet(devNum, portGroup, pcsNum, pcsType, portMode, enable), LOG_ARG_MAC_IDX_MAC(phyPortNum), LOG_ARG_GEN_PARAM_MAC(portMode));

    return GT_OK;
}

/**
* @internal mvHwsPortFastLinkDownGet function
* @endinternal
*
* @brief   Get status of Fast link Down/Up.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] pcsType                  - port pcs type
*
* @param[out] status                  - current status of Fast Link.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFastLinkDownGet
(
    IN GT_U8                devNum,
    IN GT_U32               portGroup,
    IN GT_U32               phyPortNum,
    IN MV_HWS_PORT_PCS_TYPE pcsType,
    OUT GT_BOOL            *status
)
{
    GT_UREG_DATA  regData;

    /* Check for valid pcs type and portnumber */
    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || (pcsType == PCS_NA) || (pcsType >= LAST_PCS))
    {
        return GT_BAD_PARAM;
    }

    switch (pcsType)
    {
        case CGPCS:
            /*Read Tx_local_fault i.e Bit[0] of CG_control_0 register*/
            CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, phyPortNum, CG_CONTROL_0, &regData, 1<<0));
            break;
        case MMPCS:
            /*Read Receive test pattern i.e Bit[0] of Test_pattern_configuration register*/
            CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MMPCS_UNIT, phyPortNum, TEST_PATTERN_CONFIGURATION, &regData, 1<<0));
            break;
        default:
            return GT_BAD_PARAM;
    }
    *status = (GT_BOOL)regData;

    return GT_OK;
}

/**
* @internal mvHwsPortLoopbackSet function
* @endinternal
*
* @brief   Activates the port loopback modes.
*         Can be run only after create port not under traffic.
*
* @param[in] devNum     - system device number
* @param[in] portGroup  - port group (core) number
* @param[in] phyPortNum - physical port number
* @param[in] portMode   - port standard metric
* @param[in] lpPlace    - unit for loopback configuration
* @param[in] lpType     - loopback type
*
* @retval 0 - on success
* @retval 1 - on error
*
* @note  For Serdes Loopback modes:
*        - In 'AN_TX_RX' mode, the Rx & Tx Serdes Polarity
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
GT_STATUS mvHwsPortLoopbackSet
(
    IN GT_U8                 devNum,
    IN GT_U32                portGroup,
    IN GT_U32                phyPortNum,
    IN MV_HWS_PORT_STANDARD  portMode,
    IN MV_HWS_UNIT           lpPlace,
    IN MV_HWS_PORT_LB_TYPE   lbType
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_SERDES_LB        serdesLbType;
    GT_U32                  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32                  i;
    MV_HWS_PORT_MAC_TYPE    macType;
    MV_HWS_PORT_PCS_TYPE    pcsType;
    GT_U32                  macNum, pcsNum;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortLoopbackSetIpc(devNum,portGroup, phyPortNum,portMode,lpPlace,lbType));
#endif

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    macType = curPortParams.portMacType;
    macNum  = curPortParams.portMacNumber;
    pcsType = curPortParams.portPcsType;
    pcsNum  = curPortParams.portPcsNumber;

    switch (lpPlace)
    {
        case HWS_MAC:
            if ((macType == XGMAC) || (macType == XLGMAC) || (macType == CGMAC))
            {
                /* perform MAC RESET */
                CHECK_STATUS(mvHwsMacReset(devNum, portGroup, macNum, portMode, macType, RESET));

                /* perform PCS RESET */
                CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, pcsType, RESET));
            }

            CHECK_STATUS(mvHwsMacLoopbackCfg(devNum, portGroup, macNum, portMode, macType, lbType));

            if ((macType == XGMAC) || (macType == XLGMAC) || (macType == CGMAC))
            {
                /* perform PCS UNRESET */
                CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, pcsType, UNRESET));

                /* perform MAC UNRESET */
                CHECK_STATUS(mvHwsMacReset(devNum, portGroup, macNum, portMode, macType, UNRESET));
            }
            break;
        case HWS_PCS:
            if ((macType == XGMAC) || (macType == XLGMAC) || (macType == CGMAC))
            {
                /* You must perform MAC RESET before performing PCS Reset*/
                CHECK_STATUS(mvHwsMacReset(devNum, portGroup, macNum, portMode, macType, RESET));
            }
            CHECK_STATUS(mvHwsPcsLoopbackCfg(devNum, portGroup, phyPortNum, portMode, pcsType, lbType));
            if ((macType == XGMAC) || (macType == XLGMAC) || (macType == CGMAC))
            {
                /* perform MAC UNRESET */
                CHECK_STATUS(mvHwsMacReset(devNum, portGroup, macNum, portMode, macType, UNRESET));
            }
            break;
        case HWS_PMA:
            /* convert port LB type to Serdes LB type */
            switch (lbType)
            {
                case DISABLE_LB:
                    serdesLbType = SERDES_LP_DISABLE;
                    /* signal detect mask set 0 per pcs lane */
                    mvHwsPcsSignalDetectMaskSet(devNum, portGroup, pcsNum, pcsType, GT_FALSE);
                    break;
                case RX_2_TX_LB:
                    serdesLbType = SERDES_LP_DIG_RX_TX;
                    break;
                case TX_2_RX_LB:
                    serdesLbType = SERDES_LP_AN_TX_RX;
                    /* signal detect mask set 1 per pcs lane */
                    mvHwsPcsSignalDetectMaskSet(devNum, portGroup, pcsNum, pcsType, GT_TRUE);
                    break;
                case TX_2_RX_DIGITAL_LB:
                    serdesLbType = SERDES_LP_DIG_TX_RX;
                    /* signal detect mask set 1 per pcs lane */
                    mvHwsPcsSignalDetectMaskSet(devNum, portGroup, pcsNum, pcsType, GT_TRUE);
                    break;
                default:
                    return GT_BAD_PARAM;
            }

            if ((macType == XGMAC) || (macType == XLGMAC) || (macType == CGMAC))
            {
                /* You must perform MAC RESET before performing PCS Reset*/
                CHECK_STATUS(mvHwsMacReset(devNum, portGroup, macNum, portMode, macType, RESET));
            }
            /* for port modes with Multi-lanes, need to perform Reset/Unreset to PCS units*/
            CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, pcsType, RESET));

            /* on each related serdes */
            for (i = 0; i < curPortParams.numOfActLanes; i++)
            {
                CHECK_STATUS(mvHwsSerdesLoopback(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curLanesList[i])),
                                                 serdesLbType));

                /* disable Polarity on Rx & Tx */
                if (serdesLbType == SERDES_LP_AN_TX_RX || (serdesLbType == SERDES_LP_DIG_RX_TX && (HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curLanesList[i])) == AVAGO_16NM)))
                {
                    CHECK_STATUS(mvHwsSerdesPolarityConfig(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                                           HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curLanesList[i])), GT_FALSE, GT_FALSE));
                }
            }

            /* perform PCS UNRESET */
            CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, pcsType, UNRESET));
            if ((macType == XGMAC) || (macType == XLGMAC) || (macType == CGMAC))
            {
                /* perform MAC UNRESET */
                CHECK_STATUS(mvHwsMacReset(devNum, portGroup, macNum, portMode, macType, UNRESET));
            }
            break;
#if defined(RAVEN_DEV_SUPPORT) || (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && defined(CHX_FAMILY)) /* Falcon */
        case HWS_MPF:
            CHECK_STATUS(mvHwsMpfLoobackSet(devNum,phyPortNum,portMode,lbType));
            break;
#endif
        default:
            return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}
#endif

#if (!defined MV_HWS_REDUCED_BUILD)
/**
* @internal hwsPulse1msRegCfg function
* @endinternal
*
* @brief   configure the pulse 1ms register according the
*          portmode and core clock
*
* @param[in] devNum     - system device number
* @param[in] portGroup  - port group (core) number
* @param[in] phyPortNum - physical port number
* @param[in] portMode   - port standard metric
* @param[in] coreClk    - core clock
*
* @retval 0 - on success
* @retval 1 - on error
*
*/
GT_STATUS hwsPulse1msRegCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_U32                  coreClk
)
{
    GT_U32 regVal;

    if ((portMode == SGMII) || (portMode == SGMII2_5))
    {
        regVal = 2 * coreClk;
    }
    else if ((portMode == _1000Base_X) || (portMode == _2500Base_X))
    {
        regVal = (GT_U32)(1.5 * coreClk);
    }
    else
    {
         regVal = 0xff; /*default value*/
    }

    /* write timer to register low and high */
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, phyPortNum, GIG_PORT_pulse_1ms_Low_Reg, regVal&0xffff, 0xffff));
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, phyPortNum, GIG_PORT_pulse_1ms_High_Reg, regVal>>16, 0xffff));

    return GT_OK;
}

/**
* @internal mvHwsGeneralClkSelCfg function
* @endinternal
*
* @brief   Configures the DP/Core Clock Selector on port according to core clock,
*         port mode and port number.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeneralClkSelCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
)
{
#ifndef MV_HWS_FREE_RTOS
    MV_HWS_PORT_INIT_PARAMS   curPortParams;
    GT_U32 coreClk = 0, coreClkHw;
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);
    CHECK_STATUS(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].coreClockGetFunc(devNum,&coreClk,&coreClkHw));

    CHECK_STATUS(hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams));

    if ((portMode == SGMII2_5) || (portMode == SGMII) || (portMode == _1000Base_X) || (portMode == _2500Base_X))
    {
        CHECK_STATUS(hwsPulse1msRegCfg(devNum, portGroup, curPortParams.portMacNumber, portMode, coreClk));
    }
#endif
    return GT_OK;
}
#endif

#ifndef CO_CPU_RUN
#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPortLoopbackStatusGet function
* @endinternal
*
* @brief   Retrive MAC loopback status.
*         Can be run only after create port not under traffic.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lpPlace                  - unit for loopback configuration
*
* @param[out] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLoopbackStatusGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_UNIT             lpPlace,
    OUT MV_HWS_PORT_LB_TYPE    *lbType
)
{
    MV_HWS_PORT_INIT_PARAMS  curPortParams;
    MV_HWS_SERDES_LB         serdesLbType;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
       ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

#ifndef MV_HWS_REDUCED_BUILD
    if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
        return (mvHwsPortLoopbackStatusGetIpc(devNum,portGroup, phyPortNum,portMode,lpPlace,lbType));
#endif

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    switch (lpPlace)
    {
        case HWS_MAC:
            CHECK_STATUS(mvHwsMacLoopbackStatusGet(devNum, portGroup, curPortParams.portMacNumber, portMode,
                                                  curPortParams.portMacType, lbType));
            break;
        case HWS_PCS:
            CHECK_STATUS(mvHwsPcsLoopbackCfgGet(devNum, portGroup, phyPortNum,
                                                   portMode, lbType));
            break;
        case HWS_PMA:
            /* rebuild active lanes list according to current configuration (redundancy) */
            CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));
            /* read loopback status on first lane */
            CHECK_STATUS(mvHwsSerdesLoopbackGet(devNum, (portGroup + ((curLanesList[0] >> 16) & 0xFF)),
                                                   (curLanesList[0] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[0] & 0xFFFF)), &serdesLbType));
            /* convert serdes LB type to port LB type */
            switch (serdesLbType)
            {
                case SERDES_LP_DISABLE:
                    *lbType = DISABLE_LB;
                    break;
                case SERDES_LP_DIG_RX_TX:
                    *lbType = RX_2_TX_LB;
                    break;
                case SERDES_LP_AN_TX_RX:
                    *lbType = TX_2_RX_LB;
                    break;
                case SERDES_LP_DIG_TX_RX:
                    *lbType = TX_2_RX_DIGITAL_LB;
                    break;
                default:
                    return GT_FAIL;
            }
            break;
#if defined(RAVEN_DEV_SUPPORT) || (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && defined(CHX_FAMILY)) /* Falcon */
        case HWS_MPF:
            CHECK_STATUS(mvHwsMpfLoobackGet(devNum,phyPortNum,portMode,lbType));
            break;
#endif
        default:
            return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}
#endif
#endif

/**
* @internal mvHwsPortLinkStatusGet function
* @endinternal
*
* @brief   Returns the port link status.
*          Can be run any time.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] linkStatus              - link UP status (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLinkStatusGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                *linkStatus
)
{

   MV_HWS_PORT_INIT_PARAMS curPortParams;

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortLinkStatusGetIpc(devNum,portGroup, phyPortNum,portMode,linkStatus));
#endif
   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

   CHECK_STATUS(mvHwsMacLinkStatusGet(devNum,portGroup,curPortParams.portMacNumber, portMode,
                                      curPortParams.portMacType,linkStatus));

   return GT_OK;
}

/**
* @internal mvHwsPortPolaritySet function
* @endinternal
*
* @brief   Defines the port polarity of the Serdes lanes (Tx/Rx).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] txInvMask                - bitmap of 32 bit, each bit represent Serdes
* @param[in] rxInvMask                - bitmap of 32 bit, each bit represent Serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPolaritySet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_32                   txInvMask,
    IN GT_32                   rxInvMask
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_BOOL txInv, rxInv;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortPolaritySetIpc(devNum,portGroup, phyPortNum,portMode,txInvMask,rxInvMask));
#endif
   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* MAC Reset */
   CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, RESET));

   /* PCS Reset */
   CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, curPortParams.portPcsType, RESET));

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   /* on each related serdes */
   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
           txInv = ((txInvMask >> i) & 1);
           rxInv = ((rxInvMask >> i) & 1);
           CHECK_STATUS(mvHwsSerdesPolarityConfig(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                   (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), txInv, rxInv));
   }

   /* PCS UnReset */
   CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, curPortParams.portPcsType, UNRESET));

   /* MAC UnReset */
   CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, UNRESET));

   return GT_OK;
}

/**
* @internal mvHwsPortTxEnable function
* @endinternal
*
* @brief   Turn of the port Tx according to selection.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] enable                   - enable/disable port Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTxEnable
(
    IN GT_U8                 devNum,
    IN GT_U32                portGroup,
    IN GT_U32                phyPortNum,
    IN MV_HWS_PORT_STANDARD  portMode,
    IN GT_BOOL               enable
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }
#ifndef MV_HWS_REDUCED_BUILD
   if(cpssDeviceRunCheck_onEmulator() )
   {
       return GT_OK;
   }

#ifdef ASIC_SIMULATION
CPSS_TBD_BOOKMARK_PHOENIX
      if (HWS_DEV_SIP_6_10_CHECK_MAC(devNum))
      {
         return GT_OK;
      }
#endif

   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortTxEnableIpc(devNum,portGroup, phyPortNum,portMode,enable));
#endif

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   /* on each related serdes */
   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
           CHECK_STATUS(mvHwsSerdesTxEnable(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                            (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), enable));
   }

   return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPortTxEnableGet function
* @endinternal
*
* @brief   Retrieve the TX status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] serdesTxStatus          - serdes Tx status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTxEnableGet
(
   IN GT_U8                   devNum,
   IN GT_U32                  portGroup,
   IN GT_U32                  phyPortNum,
   IN MV_HWS_PORT_STANDARD    portMode,
   OUT GT_BOOL                serdesTxStatus[]
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }
#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortTxEnableGetIpc(devNum,portGroup, phyPortNum,portMode,serdesTxStatus));
#endif

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   /* on each related serdes */
   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
           CHECK_STATUS(mvHwsSerdesTxEnableGet(devNum, portGroup,(curLanesList[i] & 0xFFFF),
                                            HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &serdesTxStatus[i]));
   }

   return GT_OK;
}
#endif

/**
* @internal mvHwsPortSignalDetectGet function
* @endinternal
*
* @brief   Retrieve the signal detected status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] signalDet[]              per serdes.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortSignalDetectGet
(
   IN GT_U8                   devNum,
   IN GT_U32                  portGroup,
   IN GT_U32                  phyPortNum,
   IN MV_HWS_PORT_STANDARD    portMode,
   OUT GT_BOOL                 signalDet[]
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }
#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortSignalDetectGetIpc(devNum,portGroup, phyPortNum,portMode,signalDet));
#endif

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   /* on each related serdes */
   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
           CHECK_STATUS(mvHwsSerdesSignalDetectGet(devNum, portGroup,(curLanesList[i] & 0xFFFF),
                                                HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &signalDet[i]));
   }

   return GT_OK;
}

/**
* @internal mvHwsPortCdrLockStatusGet function
* @endinternal
*
* @brief   Retrieve the CDR lock status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] cdrLockStatus[]         - cdrLock status per serdes.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))/*AC5-TBD*/
GT_STATUS mvHwsPortCdrLockStatusGet
(
   IN GT_U8                   devNum,
   IN GT_U32                  portGroup,
   IN GT_U32                  phyPortNum,
   IN MV_HWS_PORT_STANDARD    portMode,
   OUT GT_BOOL                cdrLockStatus[]
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }
#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortCdrLockStatusGetIpc(devNum,portGroup, phyPortNum,portMode,cdrLockStatus));
#endif

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   /* on each related serdes */
   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
           CHECK_STATUS(mvHwsSerdesCdrLockStatusGet(devNum, portGroup,(curLanesList[i] & 0xFFFF),
                                                HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &cdrLockStatus[i]));
   }

   return GT_OK;
}
#endif /* ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT))) */

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal hwsPortExtendedModeCfg function
* @endinternal
*
* @brief   Enable / disable extended mode on port specified.
*         Extended ports supported only in Lion2 and Alleycat3 devices.
*         For Lion2:   1G, 10GBase-R, 20GBase-R2, RXAUI - can be normal or extended
*         XAUI, DXAUI, 40GBase-R - only extended
*         For Alleycat3: ports 25 and 27 can be 10GBase_KR, 10GBase_SR_LR - normal or extended modes
*         port 27 can be 20GBase_KR, 20GBase_SR_LR - only in extended mode
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] extendedMode             - enable / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortExtendedModeCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                 extendedMode
)
{
   MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

   hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

   if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgFunc == NULL)
   {
           return GT_OK;
   }

   return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgFunc(devNum, portGroup,
                                                                           phyPortNum, portMode, extendedMode);
}

/**
* @internal mvHwsPortExtendedModeCfgGet function
* @endinternal
*
* @brief   Returns the extended mode status on port specified.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] extendedMode            - enable / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortExtendedModeCfgGet
(
    IN GT_U8                  devNum,
    IN GT_U32                 portGroup,
    IN GT_U32                 phyPortNum,
    IN MV_HWS_PORT_STANDARD   portMode,
    OUT GT_BOOL               *extendedMode
)
{
   MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

   hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

   if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgGetFunc == NULL)
   {
           return GT_OK;
   }

   return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgGetFunc(devNum, portGroup,
                                                                           phyPortNum, portMode, extendedMode);
}

/**
* @internal mvHwsPortFixAlign90Ext function
* @endinternal
*
* @brief   Fix Align90 parameters
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFixAlign90Ext
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portFixAlign90Func == NULL)
    {
        return GT_OK;
    }

    return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portFixAlign90Func(devNum, portGroup, phyPortNum, portMode);
}
#endif

/**
* @internal mvHwsPortAutoTuneStateCheck function
* @endinternal
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] rxTune                  - Rx Tune state
* @param[out] txTune                  - Tx Tune state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneStateCheck
(
    IN GT_U8                     devNum,
    IN GT_U32                    portGroup,
    IN GT_U32                    phyPortNum,
    IN MV_HWS_PORT_STANDARD      portMode,
    OUT MV_HWS_AUTO_TUNE_STATUS  *rxTune,
    OUT MV_HWS_AUTO_TUNE_STATUS  *txTune
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   MV_HWS_AUTO_TUNE_STATUS rxStatus = TUNE_PASS;
   MV_HWS_AUTO_TUNE_STATUS txStatus = TUNE_PASS;
   GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i;
#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortAutoTuneStateCheckIpc(devNum,portGroup, phyPortNum,portMode,rxTune,txTune));

#endif

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   *rxTune = TUNE_PASS;
   *txTune = TUNE_PASS;

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
   {
       rxStatus = TUNE_PASS;
       txStatus = TUNE_PASS;
       CHECK_STATUS(mvHwsSerdesAutoTuneStatusShort(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                   (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &rxStatus, &txStatus));

       if (txStatus != TUNE_PASS)
       {
           *txTune = txStatus;
       }

       if (rxStatus != TUNE_PASS)
       {
           *rxTune = rxStatus;
       }
   }

   return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPortPostTraining function
* @endinternal
*
* @brief   Perform UnReset for Serdes(RF), PCS and MAC units after performing TRX-Training
*         Note: this function is used mainly used by AP process where
*         training is executed in non-blocking mode (unlike "regular"
*         port mode training where status functionality is not completed
*         till all lanes finished their training).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPostTraining
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
   GT_32  j;
   GT_BOOL performPcsWA = GT_TRUE; /* in Lion2/Hooper, this variable allows PCS WA or PCS Rx unreset if set to FALSE */


   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   if (performPcsWA == GT_FALSE)
   {
        /* UnReset PCS Rx*/
        CHECK_STATUS(mvHwsPcsRxReset(devNum, portGroup, curPortParams.portPcsNumber,
                                                curPortParams.portPcsType, UNRESET));
   }
   else
   {
        hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

        /* rebuild active lanes list according to current configuration (redundancy) */
        CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

        /* Serdes digital UnReset */
        for (j = curPortParams.numOfActLanes-1; j >= 0; j--)
        {
            CHECK_STATUS(mvHwsSerdesDigitalReset(devNum, (portGroup + ((curLanesList[j] >> 16) & 0xFF)),
                                    (curLanesList[j] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[j] & 0xFFFF)), UNRESET));
        }

        /* UnReset PCS */
        CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode,
                                                curPortParams.portPcsType, UNRESET));

        /* Run fix Align90 */
        CHECK_STATUS(mvHwsPortFixAlign90Ext(devNum, portGroup, phyPortNum, portMode));

        /* UnReset MAC */
        CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode,
                                                curPortParams.portMacType, UNRESET));
   }

   /* un-mark PCS to be under TRx training */
   CHECK_STATUS(mvHwsPCSMarkModeSet(devNum, portGroup, phyPortNum, portMode, GT_FALSE));

   return GT_OK;
}

/**
* @internal mvHwsPortAutoTuneStop function
* @endinternal
*
* @brief   Stop Tx and Rx training.
*         Note: this function is used mainly used by AP process where
*         training is executed in non-blocking mode (unlike "regular"
*         port mode training where status functionality is not completed
*         till all lanes finished their training).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] stopRx                   - stopping rx training
* @param[in] stopTx                   - stopping tx training
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneStop
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroup,
    IN GT_U32                   phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL                  stopRx,
    IN GT_BOOL                  stopTx
)
{
   MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32  i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortAutoTuneStopIpc(devNum,portGroup, phyPortNum, portMode, stopRx,stopTx));
#endif

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneStopFunc != NULL)
    {
        return(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneStopFunc(devNum, portGroup, phyPortNum, portMode, stopRx,stopTx));
    }

   /* perform UnReset for Serdes(RF), PCS and MAC units BEFORE TRX-Training is stopped */
    if (!( (stopRx == GT_TRUE) && (HWS_DEV_SIP_6_10_CHECK_MAC(devNum) || (AC5 == hwsDeviceSpecInfo[devNum].devType)) ))
    {
        CHECK_STATUS(mvHwsPortPostTraining(devNum, portGroup, phyPortNum, portMode));
    }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode,curLanesList));

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* on each related serdes */
   for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
   {
           /* stop an appropriate engine */
           if (stopTx == GT_TRUE)
           {
                   CHECK_STATUS(mvHwsSerdesTxAutoTuneStop(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF))));
           }

           if (stopRx == GT_TRUE)
           {
                   CHECK_STATUS(mvHwsSerdesRxAutoTuneStart(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), GT_FALSE));
           }
   }

   return GT_OK;
}
#endif

/**
* @internal mvHwsPortFecCofig function
* @endinternal
*
* @brief   Configure FEC disable/enable on port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portFecType              - 0-AP_ST_HCD_FEC_RES_NONE, 1-AP_ST_HCD_FEC_RES_FC, 2-AP_ST_HCD_FEC_RES_RS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFecCofig
(
    IN GT_U8                devNum,
    IN GT_U32               portGroup,
    IN GT_U32               phyPortNum,
    IN MV_HWS_PORT_STANDARD portMode,
    IN MV_HWS_PORT_FEC_MODE portFecType
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
        ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortFecConfigIpc(devNum,portGroup, phyPortNum,portMode, portFecType));
#endif

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

    /* MAC Reset */
    CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, RESET));

    /* PCS Reset */
    CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, curPortParams.portPcsType, RESET));

    CHECK_STATUS(mvHwsPcsFecConfig(devNum,portGroup, curPortParams.portPcsNumber, curPortParams.portPcsType, portFecType));

    /* PCS UnReset */
    CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, curPortParams.portPcsType, UNRESET));

    /* MAC UnReset */
    CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, UNRESET));

    return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPortFecCofigGet function
* @endinternal
*
* @brief   Return FEC status disable/enable on port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] fecMode                 - fec Mode type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFecCofigGet
(
    IN GT_U8                  devNum,
    IN GT_U32                 portGroup,
    IN GT_U32                 phyPortNum,
    IN MV_HWS_PORT_STANDARD   portMode,
    OUT MV_HWS_PORT_FEC_MODE  *fecMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    /* GT_BOOL                 fecEn; */
    GT_UREG_DATA            data;
    GT_UREG_DATA            data13;
    GT_UREG_DATA            data17;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
        ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

#ifndef MV_HWS_REDUCED_BUILD
    if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
        return (mvHwsPortFecConfigGetIpc(devNum, portGroup, phyPortNum, portMode, fecMode));
#endif

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    if (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3 || HWS_DEV_SILICON_TYPE(devNum) == Aldrin2 || HWS_DEV_SILICON_TYPE(devNum) == Pipe)
    {
        /* check if 25G/20GR1 port mode use CG unit, if yes RS_FEC mode is the only available option */
        if(HWS_25G_MODE_CHECK(portMode))
        {
            /* check that RS-FEC in CG unit in use */
            CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, (phyPortNum & HWS_2_LSB_MASK_CNS), CG_CONTROL_2, &data, 0));
            data = (data >> 17) & 0xF;
            if(data & (1 << phyPortNum % 4))
            {
                *fecMode = RS_FEC;
                return GT_OK;
            }
        }

        /* check if 50G port mode use CG unit, if yes RS_FEC or NO-FEC mode are available and should be checked in CGPCS FEC get function.
           if CG unit not in use - only FC-FEC mode is the only available option  */
        if(HWS_TWO_LANES_MODE_CHECK(portMode))
        {
            /* check that CG unit in use */
            CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, CG_UNIT, (phyPortNum & HWS_2_LSB_MASK_CNS), CG_CONTROL_2, &data, 0));
            data13 = (data >> 13) & 0xF;
            data17 = (data >> 17) & 0xF;
            if(data13 == 0)
            {
                *fecMode = FC_FEC;
                return GT_OK;
            }
            if((data17 & (0x3 << phyPortNum % 4)) == 0)
            {
                *fecMode = FEC_OFF;
            }
            else
            {
                *fecMode = RS_FEC;
            }
            return GT_OK;
        }
    }

    CHECK_STATUS(mvHwsPcsFecConfigGet(devNum, portGroup, phyPortNum, portMode, fecMode));

    return GT_OK;
}
#endif

/**
* @internal mvHwsPortFixAlign90 function
* @endinternal
*
* @brief   Run fix Align90 process on current port.
*         Can be run any time after create port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] dummyForCompilation      - dummy for compilation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFixAlign90
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_U32                  dummyForCompilation
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   MV_HWS_ALIGN90_PARAMS serdesParams[HWS_MAX_SERDES_NUM];
   GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
   MV_HWS_AUTO_TUNE_STATUS status[HWS_MAX_SERDES_NUM];
   GT_BOOL allLanesPass;
   GT_U32 i, loopNum;

   /* avoid warnings */
   dummyForCompilation = dummyForCompilation;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   hwsOsMemSetFuncPtr(serdesParams, 0, sizeof(serdesParams));
   hwsOsMemSetFuncPtr(status, TUNE_NOT_COMPLITED, sizeof(status));

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   /* start Align90 on each related serdes */
   for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
   {
           mvHwsSerdesFixAlign90Start(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                      (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &(serdesParams[i]));
   }

   loopNum = 0;
   do
   {
           allLanesPass = GT_TRUE;
           hwsOsExactDelayPtr(devNum, portGroup, align90Delay);
           /* check status on each related serdes */
           for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
           {
                   mvHwsSerdesFixAlign90Status(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                               (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &status[i]);
                   if (status[i] != TUNE_PASS)
                   {
                           DEBUG_HWS_FULL(0, ("\nFix Align90 failed on serdes %d (device %d).",(curLanesList[i] & 0xFFFF), devNum));
                           allLanesPass = GT_FALSE;
                   }
           }
           loopNum++;
   } while ((!allLanesPass) && (loopNum < 10));

   /* stop Align90 on each related serdes */
   for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
   {
           mvHwsSerdesFixAlign90Stop(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &(serdesParams[i]),
                                           ((status[i] == TUNE_PASS) ? GT_TRUE : GT_FALSE));
           mvHwsSerdesRev2DfeCheck(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                           (curLanesList[i] & 0xFFFF), (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)));
   }

   if (allLanesPass)
   {
           return GT_OK;
   }

   return GT_FAIL;
}

/**
* @internal mvHwsPortFixAlign90Start function
* @endinternal
*
* @brief   Start fix Align90 process on current port.
*         Can be run any time after create port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] serdesParams            - serdes parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFixAlign90Start
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_ALIGN90_PARAMS  *serdesParams
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 numOfActLanes;
   GT_U32 i;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   numOfActLanes = curPortParams.numOfActLanes;
   if (numOfActLanes > HWS_MAX_SERDES_NUM)
   {
           return GT_BAD_PARAM;
   }

   /* start Align90 on each related serdes */
   for (i = 0; i < numOfActLanes; i++)
   {
           DEBUG_HWS_FULL(2, ("Start Align90 for Serdes %d\n", curLanesList[i] & 0xFFFF));

           CHECK_STATUS(mvHwsSerdesFixAlign90Start(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &(serdesParams[i])));
   }

   return GT_OK;
}

/**
* @internal mvHwsPortFixAlign90Stop function
* @endinternal
*
* @brief   Stop fix Align90 process on current port.
*         Can be run any time after create port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] serdesParams            - serdes parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFixAlign90Stop
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_ALIGN90_PARAMS   *serdesParams
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 numOfActLanes;
   GT_U32 i;
   MV_HWS_AUTO_TUNE_STATUS status;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   numOfActLanes = curPortParams.numOfActLanes;

   /* stop Align90 on each related serdes */
   for (i = 0; i < numOfActLanes; i++)
   {
           DEBUG_HWS_FULL(2, ("Fix Align90 stop for serdes %d (device %d)\n", (curLanesList[i] & 0xFFFF), devNum));

           status = TUNE_PASS;
           CHECK_STATUS(mvHwsSerdesFixAlign90Status(devNum,(portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &status));

           CHECK_STATUS(mvHwsSerdesFixAlign90Stop(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                   (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &(serdesParams[i]),
                                                   ((status == TUNE_PASS) ? GT_TRUE : GT_FALSE)));
   }

   return GT_OK;
}

/**
* @internal mvHwsPortFixAlign90Status function
* @endinternal
*
* @brief   Run fix Align90 process on current port.
*         Can be run any time after create port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] statusPtr                - do all lanes of port passed align90 successfully
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFixAlign90Status
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroup,
    IN GT_U32                   phyPortNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT MV_HWS_AUTO_TUNE_STATUS *statusPtr
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 numOfActLanes;
   GT_U32 i;
   MV_HWS_AUTO_TUNE_STATUS status;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));
   numOfActLanes = curPortParams.numOfActLanes;

   *statusPtr = TUNE_PASS;
   /* check status on each related serdes */
   for (i = 0; i < numOfActLanes; i++)
   {
           status = TUNE_PASS;
           CHECK_STATUS(mvHwsSerdesFixAlign90Status(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                           (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &status));

           if (status != TUNE_PASS)
           {
                   if (status == TUNE_FAIL)
                   {
                           *statusPtr = TUNE_FAIL;
                           DEBUG_HWS_FULL(2, ("Fix Align90 failed for serdes %d (device %d)\n", (curLanesList[i] & 0xFFFF), devNum));

                           /* if one lane failed no reason to continue */
                           return GT_OK;
                   }
                   else if (status == TUNE_NOT_COMPLITED)
                   {
                           *statusPtr = TUNE_NOT_COMPLITED;

                           DEBUG_HWS_FULL(2, ("Fix Align90 not completed for serdes %d (device %d)\n", (curLanesList[i] & 0xFFFF), devNum));
                   }
           }
   }

   return GT_OK;
}

/**
* @internal hwsPortFixAlign90Flow function
* @endinternal
*
* @brief   Run Align90 flow.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] numOfActLanes            - number of active lanes
*
* @param[out] serdesList                - list of the serdes
* @param[out] allLanesPass              - checking all lanes
*                                         if pass
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortFixAlign90Flow
(
    IN GT_U8         devNum,
    IN GT_U32        portGroup,
    OUT GT_U32       *serdesList,
    IN GT_U32        numOfActLanes,
    OUT GT_BOOL      *allLanesPass
)
{
   GT_U32 i;
   GT_U32 loopNum;
   MV_HWS_ALIGN90_PARAMS serdesParams[HWS_MAX_SERDES_NUM];
   MV_HWS_AUTO_TUNE_STATUS status[HWS_MAX_SERDES_NUM];

   if (numOfActLanes > HWS_MAX_SERDES_NUM)
   {
           return GT_BAD_PARAM;
   }

   hwsOsMemSetFuncPtr(serdesParams, 0, sizeof(serdesParams));
   hwsOsMemSetFuncPtr(status, TUNE_NOT_COMPLITED, sizeof(status));

   /* start Align90 on each related serdes */
   for (i = 0; i < numOfActLanes; i++)
   {
           if (serdesList[i] == MV_HWS_SERDES_NOT_ACTIVE)
           {
                   continue;
           }

           DEBUG_HWS_FULL(2, ("Start Align90 for Serdes %d\n", serdesList[i] & 0xFFFF));

           mvHwsSerdesFixAlign90Start(devNum, (portGroup + ((serdesList[i] >> 16) & 0xFF)),
                                      (serdesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (serdesList[i] & 0xFFFF)), &(serdesParams[i]));
   }

   loopNum = 0;

   do
   {
           *allLanesPass = GT_TRUE;
           hwsOsExactDelayPtr(devNum, portGroup, align90Delay);
           /* check status on each related serdes */
           for (i = 0; i < numOfActLanes; i++)
           {
                   if ((serdesList[i] == MV_HWS_SERDES_NOT_ACTIVE) || (status[i] == TUNE_PASS))
                   {
                           continue;
                   }

                   mvHwsSerdesFixAlign90Status(devNum, (portGroup + ((serdesList[i] >> 16) & 0xFF)),
                                                   (serdesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (serdesList[i] & 0xFFFF)), &status[i]);

                   if (status[i] != TUNE_PASS)
                   {
                           *allLanesPass = GT_FALSE;

                           if (status[i] == TUNE_FAIL)
                           {
                                   DEBUG_HWS_FULL(2, ("Fix Align90 failed for serdes %d (device %d)\n",
                                                           (serdesList[i] & 0xFFFF), devNum));
                           }
                           else if (status[i] == TUNE_NOT_COMPLITED)
                           {
                                   DEBUG_HWS_FULL(2, ("Fix Align90 no completed for serdes %d (device %d)\n",
                                                           (serdesList[i] & 0xFFFF), devNum));
                           }
                   }
           }

           loopNum++;

   } while ((!(*allLanesPass)) && (loopNum < 10));

   /* stop Align90 on each related serdes */
   for (i = 0; i < numOfActLanes; i++)
   {
           if (serdesList[i] == MV_HWS_SERDES_NOT_ACTIVE)
           {
                   continue;
           }

           mvHwsSerdesFixAlign90Stop(devNum, (portGroup + ((serdesList[i] >> 16) & 0xFF)),
                                           (serdesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (serdesList[i] & 0xFFFF)), &(serdesParams[i]),
                                           ((status[i] == TUNE_PASS) ? GT_TRUE : GT_FALSE));

           /* after we stop the Serdes, we can mark it as not active */
           if (status[i] == TUNE_PASS)
           {
                   DEBUG_HWS_FULL(2, ("Align90 completed for Serdes %d\n", serdesList[i] & 0xFFFF));
                   serdesList[i] = MV_HWS_SERDES_NOT_ACTIVE;
           }
   }

   return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPortAutoTuneOptimization function
* @endinternal
*
* @brief   Run optimization for the training results.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] optAlgoMask              bit mask of optimization algorithms
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneOptimization
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_U32  optAlgoMask
)
{
   MV_HWS_PORT_MAN_TUNE_MODE       tuningMode;
   GT_BOOL isTrainingMode;
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
   GT_STATUS rc = GT_OK;

   CHECK_STATUS(hwsPortGetTuneMode(portMode, &tuningMode, &isTrainingMode));
   if (isTrainingMode == GT_FALSE)
   {
           return GT_BAD_PARAM;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));
#ifdef CHX_FAMILY
   rc = mvHwsComHRev2SerdesTrainingOptimization(devNum, portGroup, curLanesList,
                                                   curPortParams.numOfActLanes, tuningMode, optAlgoMask);
#else
   optAlgoMask = optAlgoMask;
#endif
   return rc;
}

/**
* @internal hwsPortGetTuneMode function
* @endinternal
*
* @brief   returns the tune mode according to port mode..
*
* @param[in] portMode                 - port standard metric
*
* @param[out] tuneMode                - tune mode type
* @param[out] isTrainingMode          - in training mode or not
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortGetTuneMode
(
    IN  MV_HWS_PORT_STANDARD         portMode,
    OUT MV_HWS_PORT_MAN_TUNE_MODE    *tuneMode,
    OUT GT_BOOL                      *isTrainingMode
)
{
   *isTrainingMode = GT_TRUE;

   if (IS_KR_MODE(portMode))
   {
           /*hwsOsPrintf("Long Reach\n");*/
           *tuneMode = StaticLongReach;
   }
   else if (IS_SR_MODE(portMode))
   {
           /*hwsOsPrintf("Short Reach\n");*/
           *tuneMode = StaticShortReach;
   }
   else
   {
           /*hwsOsPrintf("Not a training mode\n");*/
           *isTrainingMode = GT_FALSE;
   }

   return GT_OK;
}

/**
* @internal mvHwsPortInterfaceGet function
* @endinternal
*
* @brief   Gets Interface mode and speed of a specified port.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number (or CPU port)
*
* @param[out] portModePtr              - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS mvHwsPortInterfaceGet
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroup,
    IN GT_U32                   phyPortNum,
    OUT MV_HWS_PORT_STANDARD    *portModePtr
)
{
   MV_HWS_SERDES_SPEED rate;
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 pcsLaneNums, macLaneNums;
   GT_U32 i;
   GT_STATUS rc = GT_OK;
#ifndef CM3
   MV_HWS_SERDES_MEDIA     serdesMediaType;
   GT_UREG_DATA  data = 0;
#endif /* CM3 */

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || (portModePtr == NULL))
   {
           return GT_BAD_PARAM;
   }

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
           return (mvHwsPortInterfaceGetIpc(devNum,portGroup, phyPortNum,portModePtr));
#endif

   *portModePtr = NON_SUP_MODE;
   for (i = 0; i < hwsDeviceSpecInfo[devNum].lastSupPortMode; i++)
   {
       if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, i, &curPortParams))
       {
           continue;
       }
       if ((curPortParams.numOfActLanes != 0) && (curPortParams.portMacNumber != NA_NUM) && (curPortParams.serdesSpeed != SPEED_NA))
       {
           /* rebuild active lanes list according to current configuration (redundancy) */
           CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum,
                                                   curPortParams.portStandard, curLanesList));
           /* get serdes rate */
           rc = mvHwsSerdesSpeedGet(devNum, (portGroup + ((curLanesList[0] >> 16) & 0xFF)), (curLanesList[0] & 0xFFFF),
                                    HWS_DEV_SERDES_TYPE(devNum, (curLanesList[0] & 0xFFFF)), &rate);
           if (rc == GT_NOT_INITIALIZED)
           {
               /* It may be not initialized for current port mode but initialized in the next one*/
               /* So should continue in the loop                                                 */
               continue;
           }
           if (rc != GT_OK)
           {
               return rc;
           }


           if ((rate == SPEED_NA) || (rate != curPortParams.serdesSpeed))
           {
               continue;
           }

           /* get number of active PCSs */
           mvHwsPcsActiveStatusGet(devNum, portGroup, phyPortNum, curPortParams.portStandard, &pcsLaneNums);
           if ((pcsLaneNums == 0) || ((pcsLaneNums != curPortParams.numOfActLanes)
                                      && (curPortParams.portPcsType  != XPCS)/* RXAUI uses 2 serdes but 4 XPCS lanes */))
           {
               if ((curPortParams.portPcsType == HGLPCS) && ((rate == _3_125G) || (rate == _6_25G)))
               {
                   pcsLaneNums = 6;
               }
               else
               {
                   continue;
               }
           }

           /* get number of active MACs */
           mvHwsMacActiveStatusGet(devNum, portGroup, curPortParams.portMacNumber, curPortParams.portStandard, curPortParams.portMacType, &macLaneNums);
           if (macLaneNums == 0)
           {
               continue;
           }
#ifndef CM3
           if ((hwsDeviceSpecInfo[devNum].devType == BobcatA0) || (hwsDeviceSpecInfo[devNum].devType == Alleycat3A0) || (hwsDeviceSpecInfo[devNum].devType == AC5))
           {
               /* check current serdes media type matches configured in hw one */
               CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, curPortParams.firstLaneNum,
                                                   SERDES_EXTERNAL_CONFIGURATION_0, &data, 0));
               serdesMediaType = (((data >> 15) & 0x1) == GT_TRUE) ? RXAUI_MEDIA : XAUI_MEDIA;
               if (curPortParams.serdesMediaType != serdesMediaType)
               {
                   continue;
               }
           }
           else if ((HooperA0 == hwsDeviceSpecInfo[devNum].devType) || (Lion2B0 == hwsDeviceSpecInfo[devNum].devType))
           {
               /* check if it is RXAUI_MEDIA */
               CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, MG_Media_Interface_Reg, &data, 0));
               serdesMediaType = (((data >> curPortParams.firstLaneNum) & 1) == GT_TRUE) ? RXAUI_MEDIA : XAUI_MEDIA;
               if (curPortParams.serdesMediaType != serdesMediaType)
               {
                   continue;
               }
           }
           else if (Pipe == hwsDeviceSpecInfo[devNum].devType)
           {
               /* check current serdes media type matches configured in hw one */
               CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, curPortParams.firstLaneNum,
                                                   SERDES_EXTERNAL_CONFIGURATION_0, &data, 0));
               serdesMediaType = (((data >> 2) & 0x1) == GT_TRUE) ? RXAUI_MEDIA : XAUI_MEDIA;
               if (curPortParams.serdesMediaType != serdesMediaType)
               {
                   continue;
               }
           }
#endif /* CM3 */
           *portModePtr = (MV_HWS_PORT_STANDARD)i;
           return GT_OK;
       }
   }

   return GT_OK;
}
#endif

/**
* @internal mvHwsPortClearChannelCfg function
* @endinternal
*
* @brief   Configures MAC advanced feature accordingly.
*         Can be run before create port or after delete port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] txIpg                    - TX_IPG
* @param[in] txPreamble               - TX Preamble
* @param[in] rxPreamble               - RX Preamble
* @param[in] txCrc                    - TX CRC
* @param[in] rxCrc                    - RX CRC
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortClearChannelCfg
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_U32  txIpg,
    IN GT_U32  txPreamble,
    IN GT_U32  rxPreamble,
    IN MV_HWS_PORT_CRC_MODE    txCrc,
    IN MV_HWS_PORT_CRC_MODE    rxCrc
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           (portMode >= (MV_HWS_PORT_STANDARD)HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   CHECK_STATUS(mvHwsMacClearChannelCfg(devNum, portGroup,curPortParams.portMacNumber, portMode,
                                           curPortParams.portMacType,
                                           txIpg,txPreamble,rxPreamble,txCrc,rxCrc));

   return GT_OK;
}

/**
* @internal mvHwsPortAcTerminationCfg function
* @endinternal
*
* @brief   Configures AC termination in all port serdes lanes according to mode.
*         Can be run after create port only.
*
*  @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portAcTermEn             - enable or disable AC termination
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAcTerminationCfg
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_BOOL portAcTermEn
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;
   GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
   GT_U32 i;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
           return GT_BAD_PARAM;
   }

   if ( (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams)) || curPortParams.numOfActLanes == 0)
   {
           return GT_NOT_SUPPORTED;
   }

   /* rebuild active lanes list according to current configuration (redundancy) */
   CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

   for (i = 0; i < curPortParams.numOfActLanes; i++)
   {
           CHECK_STATUS(mvHwsSerdesAcTerminationCfg(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                                           (curLanesList[i] & 0xFFFF), (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF))),
                                                           portAcTermEn));
   }

   return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT

/**
* @internal mvHwsPortCheckGearBox function
* @endinternal
*
* @brief   Check Gear Box Status on related lanes.
*         Can be run after create port only.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] laneLock                 - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortCheckGearBox
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL *laneLock
)
{
   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) || ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
       return GT_BAD_PARAM;
   }

   CHECK_STATUS(mvHwsPcsCheckGearBox(devNum, portGroup, phyPortNum, portMode,laneLock));

   return GT_OK;
}
#endif
/**
* @internal mvHwsExtendedPortSerdesTxIfSelectSet function
* @endinternal
*
* @brief   Configures the Serdes Tx interface selector for Extended Ports
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsExtendedPortSerdesTxIfSelectSet
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
)
{
   /* relevant for ports 25 and 27 */
   switch (phyPortNum)
   {
   case 25:
           switch (portMode)
           {
           case SGMII:
           case _1000Base_X:
           case SGMII2_5:
           case _2500Base_X:
           case _5GBase_DQX:
           case _10GBase_KR:
           case _10GBase_SR_LR:
           case _12_1GBase_KR:
           case _5_625GBaseR:
           case _12GBaseR:
           case _5GBaseR:
                   CHECK_STATUS(mvHwsSerdesTxIfSelect(devNum, portGroup, 10,
                                                           (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, 10)), 0));
                   break;
           default:
                   return GT_NOT_SUPPORTED;
           }
           break;
   case 27:
           switch (portMode)
           {
           case SGMII:
           case _1000Base_X:
           case SGMII2_5:
           case _2500Base_X:
           case _5GBase_DQX:
           case _10GBase_KR:
           case _10GBase_SR_LR:
           case _12_1GBase_KR:
           case _5_625GBaseR:
           case _12GBaseR:
           case _2_5GBase_QX:
           case _5GBaseR:
                   CHECK_STATUS(mvHwsSerdesTxIfSelect(devNum, portGroup, 11,
                                                           (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, 11)), 0));
                   break;
           case _5GBase_HX:
           case _20GBase_KR2:
           case _20GBase_SR_LR2:
           case _22GBase_SR:
                   CHECK_STATUS(mvHwsSerdesTxIfSelect(devNum, portGroup, 10,
                                                           (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, 10)), 1));
                   CHECK_STATUS(mvHwsSerdesTxIfSelect(devNum, portGroup, 11,
                                                           (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, 11)), 0));
                   break;
           default:
                   return GT_NOT_SUPPORTED;
           }
           break;
   default:
           return GT_NOT_SUPPORTED;
   }

   return GT_OK;
}

/**
* @internal mvHwsPortUnitReset function
* @endinternal
*
* @brief   Reset/Unreset the MAC/PCS unit number of port.
*         For PMA (Serdes) unit, performs Power-Down for all port lanes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] unitType                 - Type of unit: MAC, PCS or PMA(Serdes)
* @param[in] action                   - RESET:   Reset the MAC/PCS.  Power-down for PMA(Serdes)
*                                      UNRESET: Unreset the MAC/PCS.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortUnitReset
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_UNIT             unitType,
    IN MV_HWS_RESET            action
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;
    MV_HWS_SERDES_CONFIG_STC    serdesConfig;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    switch (unitType)
    {
        case HWS_MAC:
            CHECK_STATUS(mvHwsMacReset(devNum, portGroup, curPortParams.portMacNumber, portMode, curPortParams.portMacType, action));
            break;
        case HWS_PCS:
            CHECK_STATUS(mvHwsPcsReset(devNum, portGroup, phyPortNum, portMode, curPortParams.portPcsType, action));
            break;
        case HWS_PMA: /* Serdes */
            if (action == UNRESET) /* for Serdes only Power-down is enabled */
            {
                return GT_NOT_SUPPORTED;
            }
            /* rebuild active lanes list according to current configuration (redundancy) */
            CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

            for (i = 0; i < curPortParams.numOfActLanes; i++)
            {
                serdesConfig.baudRate = curPortParams.serdesSpeed;
                serdesConfig.media = curPortParams.serdesMediaType;
                serdesConfig.busWidth = curPortParams.serdes10BitStatus;
                serdesConfig.refClock = _156dot25Mhz;
                serdesConfig.refClockSource = PRIMARY;
                serdesConfig.encoding = SERDES_ENCODING_NA;
                serdesConfig.serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curLanesList[i])));

                /* for Serdes Power-Down operation, refClock and refClockSource values are not relevant */
                CHECK_STATUS(mvHwsSerdesPowerCtrl(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), GT_FALSE, &serdesConfig));
            }
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPortPcsActiveStatusGet function
* @endinternal
*
* @brief   Return number of PCS active lanes or 0 if PCS unit is under RESET.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPcsActiveStatusGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT GT_U32                  *numOfLanes
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 pcsLaneNums;

#ifndef MV_HWS_REDUCED_BUILD
    if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
        return (mvHwsPortPcsActiveStatusGetIpc(devNum,portGroup, phyPortNum, portMode, numOfLanes));
#endif

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvHwsPcsActiveStatusGet(devNum, portGroup, phyPortNum, portMode, &pcsLaneNums));

    *numOfLanes = pcsLaneNums;

    return GT_OK;
}
#endif

/**
* @internal mvHwsPortPsyncBypassCfg function
* @endinternal
*
* @brief   Configures the bypass of synchronization module.
*         Controls bypass of synchronization module. When working at 8 bits interface,
*         bypass is used. When working at 64 bits interface, bypass is not used.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPsyncBypassCfg
(
    IN GT_U8                  devNum,
    IN GT_U32                 portGroup,
    IN GT_U32                 phyPortNum,
    IN MV_HWS_PORT_STANDARD   portMode
)
{
    MV_HWS_PORT_INIT_PARAMS   curPortParams;
    GT_UOPT macNum;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CHECK_STATUS_EXT(GT_NOT_SUPPORTED, LOG_ARG_MAC_IDX_MAC(phyPortNum));
    }

    macNum = curPortParams.portMacNumber;

    /**************************************************/
    /* in BC3 the minimum interface width is 64 bits: */
    /* relevant for: SGMII/1000BaseX/SGMII2.5G/QSGMII */
    /**************************************************/
    if(((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe)
        || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2)) &&
        (portMode <= QSGMII))
    {
        /* set P Sync Bypass */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, macNum,
            GIG_PORT_MAC_CONTROL_REGISTER4, (1 << 6), (1 << 6)));
        return GT_OK;
    }

    if ((portMode == SGMII2_5) || (portMode == _2500Base_X))
    {
        /* set P Sync Bypass */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, macNum,
            GIG_PORT_MAC_CONTROL_REGISTER4, (1 << 6), (1 << 6)));
        return GT_OK;
    }

    /* unset P Sync Bypass */
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, curPortParams.portMacNumber,
        GIG_PORT_MAC_CONTROL_REGISTER4, 0, (1 << 6)));

    return GT_OK;
}

#if 0
/**
* @internal mvHwsPortBetterAdaptationSet function
* @endinternal
*
* @brief   Run the better Align90 adaptation algorithm on specific port number.
*         The function returns indication if the Better adaptation algorithm
*         succeeded to run on port or not.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] betterAlign90            - indicates if the Better adaptation algorithm run on
*                                      specific port or not
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsPortBetterAdaptationSet
(
    GT_U8                   devNum,
    GT_U32                     portGroup,
    GT_U32                     phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                    *betterAlign90
)
{
    MV_HWS_PORT_INIT_PARAMS *curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
    GT_U32 i;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portBetterAdapSetFunc == NULL)
    {
        return GT_OK;
    }

    *betterAlign90 = GT_FALSE;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
        ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    curPortParams = hwsPortModeParamsGet(devNum, portGroup, phyPortNum, portMode);

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; i < curPortParams->numOfActLanes; i++)
    {
        /* find a better Align90 adaptation value when temperature on device is changed from cold->hot or from hot->cold */
        CHECK_STATUS(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portBetterAdapSetFunc(devNum, portGroup, (curLanesList[i] & 0xFFFF), betterAlign90));
    }

    return GT_OK;
}
#endif

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPortFlowControlStateSet function
* @endinternal
*
* @brief   Activates the port loopback modes.
*         Can be run only after create port not under traffic.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] fcState                  - flow control state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFlowControlStateSet
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    IN MV_HWS_PORT_FLOW_CONTROL_ENT    fcState
)
{
   MV_HWS_PORT_INIT_PARAMS curPortParams;

   if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
           ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
   {
       return GT_BAD_PARAM;
   }

#ifndef MV_HWS_REDUCED_BUILD
   if (hwsDeviceSpecInfo[devNum].ipcConnType == HOST2SERVICE_CPU_IPC_CONNECTION)
       return (mvHwsPortFlowControlStateSetIpc(devNum,portGroup, phyPortNum,portMode,fcState));
#endif

   if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
   {
       return GT_NOT_SUPPORTED;
   }

   CHECK_STATUS(mvHwsMacFcStateCfg(devNum, portGroup, curPortParams.portMacNumber, portMode,
                                               curPortParams.portMacType, fcState));

   return GT_OK;
}
#endif

#ifndef MV_HWS_AVAGO_NO_VOS_WA
/**
* @internal mvHwsPortVosOverrideControlModeSet function
* @endinternal
*
* @brief   Set the override mode of the VOS parameters for all ports. If the override mode
*         is set to true, the VOS parameters will be overriden, if set to false, they will
*         not be overriden.
*
* @param[in] devNum                   - device number.
* @param[in] phyPortNum               - physical port number
* @param[in] vosOverride              - GT_TRUE means override, GT_FALSE means no override.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortVosOverrideControlModeSet
(
    IN GT_U8 devNum,
    IN GT_U32 phyPortNum,
    IN GT_BOOL vosOverride
)
{
    CHECK_STATUS(mvHwsAvagoSerdesVosOverrideModeSet(devNum, vosOverride));
#ifndef MV_HWS_REDUCED_BUILD
    if (mvHwsServiceCpuEnableGet(devNum))
    {
        CHECK_STATUS(mvHwsPortVosOverrideModeSetIpc(devNum, phyPortNum, vosOverride));
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsPortVosOverrideControlModeGet function
* @endinternal
*
* @brief   Set the override mode of the VOS parameters for all ports. If the override mode
*         is set to true, the VOS parameters will be overriden, if set to false, they will
*         not be overriden.
*
* @param[in] devNum                   - device number.
*
* @param[out] vosOverridePtr           - (pointer to) current VOS override mode.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortVosOverrideControlModeGet
(
    IN GT_U8   devNum,
    OUT GT_BOOL *vosOverridePtr
)
{
    CHECK_STATUS(mvHwsAvagoSerdesVosOverrideModeGet(devNum, vosOverridePtr));

    return GT_OK;
}
#endif /*#ifndef MV_HWS_AVAGO_NO_VOS_WA*/

/**
* @internal mvHwsPortEnableSet function
* @endinternal
*
* @brief   Reseting port PTP unit then performing port enable on port MAC unit.
*         This API currently support only port-enable and not port-disable,
*         and only for MAC units supported in AP, and for devices without PHYs
*         that requires furthur port enable configurations and erratas.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] enable                   - enable or disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortEnableSet
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    IN GT_BOOL                         enable
)
{
#ifndef BC2_DEV_SUPPORT /* not applicable for BC2 and AC3 */
    GT_U32      ptpResetMask;      /* ptp reset bits mask */
    GT_U32      ptpResetData;      /* ptp reset bits data */
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT)
    /*GT_U32      baseAddr, unitIndex, unitNum;*/
    /*GT_U32      controlRegAddr;*/
#endif
#endif

    MV_HWS_PORT_INIT_PARAMS curPortParams;

    /* Currently The operation is supported on several devices, with have no
       errata nor external phys need furthur port enable implementation. */
    if (HWS_DEV_SILICON_TYPE(devNum) == Alleycat3A0  || HWS_DEV_SILICON_TYPE(devNum) == AC5)
    {
        return GT_OK;
    }

    /* Validity checks */
    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
            ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    /* Getting port params */
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_INITIALIZED;
    }

    /* Checking MAC type so that 'mvHwsMacPortEnable()'  that is called next, will not be called with
       unsupported MAC after PTP (which is MAC type independant) was reset */
    if ((curPortParams.portMacType == MAC_NA) || (curPortParams.portMacType >= LAST_MAC))
        return GT_NOT_SUPPORTED;

#ifndef BC2_DEV_SUPPORT
    /* Performing port PTP reset
         Bobcat2:
            bit[1] PTP reset
         BobK, Aldrin, AC3X:
            bit[1] PTP TX reset, bit[13] PTP RX reset
         Bobcat3:
            bit[1] PTP TX reset, bit[8] PTP RX reset
    */

    /* Calculating PTP reset mask and bits */
    ptpResetData = 0;
    ptpResetMask = 2;
    /* First, PTP reset bit (or PTP TX reset bit on supported devices) */
    if (enable)
    {
        ptpResetData |= 2;
    }
    /* Second, PTP RX reset bit on supported devices */
    if ( HWS_DEV_SILICON_TYPE(devNum) == BobK || HWS_DEV_SILICON_TYPE(devNum) == Aldrin )
    {
        ptpResetMask |= 0x2000;
        if (enable)
        {
            ptpResetData |= 0x2000;
        }
    }
    else if ((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe) )
    {
        ptpResetMask |= 0x100;
        if (enable)
        {
            ptpResetData |= 0x100;
        }
    }

    /* If the operation is Enable, first Unreset the PTP then perform MAC enable */
    if (enable)
    {
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT)
        if ((HWS_DEV_SILICON_TYPE(devNum) == Falcon) || (HWS_DEV_SILICON_TYPE(devNum) == Raven))
        {
#if 0
            if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
            {
                CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_TSU_UNIT,( curPortParams.portMacNumber & 0xfffffff8), &baseAddr, &unitIndex, &unitNum));
                if (curPortParams.portMacNumber%8 == 0) { /*in seg mode port 0 is taken from mpf 8 */
                    baseAddr += 8*HWS_MPF_CHANNEL_OFFSET;
                }
                else /*in seg mode port 4 is taken from mpf 9 */
                {
                    baseAddr += 9*HWS_MPF_CHANNEL_OFFSET;
                }
            }
            else
            {
                CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_TSU_UNIT, curPortParams.portMacNumber, &baseAddr, &unitIndex, &unitNum));
            }

            controlRegAddr = baseAddr + TSU_CONTROL;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, controlRegAddr, 0, 0x1));
#endif
        }
        else
#endif
        {
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, PTP_UNIT, curPortParams.portMacNumber,
                PTP_PORT_CONTROL_REGISTER, ptpResetData, ptpResetMask));
        }
    }
#endif /* ndef BC2_DEV_SUPPORT */
    /* Performing MAC port enable */
    CHECK_STATUS(mvHwsMacPortEnable(devNum, portGroup, curPortParams.portMacNumber,  portMode,
                                                curPortParams.portMacType, enable));
#ifndef BC2_DEV_SUPPORT
    /* If the operation is Disable, first disable the MAC, then reset PTP */
    if (!enable)
    {
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT)
        if ((HWS_DEV_SILICON_TYPE(devNum) == Falcon) || (HWS_DEV_SILICON_TYPE(devNum) == Raven))
        {
#if 0
            if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
            {
                CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_TSU_UNIT,( curPortParams.portMacNumber & 0xfffffff8), &baseAddr, &unitIndex, &unitNum));
                if (curPortParams.portMacNumber%8 == 0) { /*in seg mode port 0 is taken from mpf 8 */
                    baseAddr += 8*HWS_MPF_CHANNEL_OFFSET;
                }
                else /*in seg mode port 4 is taken from mpf 9 */
                {
                    baseAddr += 9*HWS_MPF_CHANNEL_OFFSET;
                }
            }
            else
            {
                CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_TSU_UNIT, curPortParams.portMacNumber, &baseAddr, &unitIndex, &unitNum));
            }

            controlRegAddr = baseAddr + TSU_CONTROL;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, controlRegAddr, 1, 0x1));
#endif
        }
        else
#endif
        {
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, PTP_UNIT, curPortParams.portMacNumber,
                    PTP_PORT_CONTROL_REGISTER, ptpResetData, ptpResetMask));
        }
    }
#endif

    return GT_OK;
}

/**
 * mvHwsPortApGetMaxLanesPortModeGet
 *
 * @brief  Get max lanes port mode
 *
 * @param IN capability
 * @param OUT portModePtr
 *
 * @return GT_STATUS
 */
GT_STATUS mvHwsPortApGetMaxLanesPortModeGet(GT_U32 capability, MV_HWS_PORT_STANDARD *portModePtr)
{

    if (capability & ((1<<AP_CTRL_400GBase_CR8_SHIFT) | (1<<AP_CTRL_400GBase_KR8_SHIFT)|
                      (1<<AP_CTRL_200GBase_CR8_SHIFT)| (1<<AP_CTRL_200GBase_KR8_SHIFT)))
    {
        if(AP_CTRL_400GBase_CR8_GET(capability))
        {
            *portModePtr = _400GBase_CR8;
        }
        else if (AP_CTRL_400GBase_KR8_GET(capability))
        {
            *portModePtr = _400GBase_KR8;
        }
        else if (AP_CTRL_200GBase_CR8_GET(capability))
        {
            *portModePtr = _200GBase_CR8;
        }
        else
        {
            *portModePtr = _200GBase_KR8;
        }
    }
    else if (capability & ((1<<AP_CTRL_200GBase_CR4_SHIFT) | (1<<AP_CTRL_200GBase_KR4_SHIFT)|
                           (1<<AP_CTRL_100GBase_CR4_SHIFT)| (1<<AP_CTRL_100GBase_KR4_SHIFT)|
                           (1<<AP_CTRL_40GBase_CR4_SHIFT)| (1<<AP_CTRL_40GBase_KR4_SHIFT) | (1<<AP_CTRL_10GBase_KX4_SHIFT)))
    {
        if(AP_CTRL_200GBase_CR4_GET(capability))
        {
            *portModePtr = _200GBase_CR4;
        }
        else if (AP_CTRL_200GBase_KR4_GET(capability))
        {
            *portModePtr = _200GBase_KR4;
        }
        else if (AP_CTRL_100GBase_CR4_GET(capability))
        {
            *portModePtr = _100GBase_CR4;
        }
        else if (AP_CTRL_100GBase_KR4_GET(capability))
        {
            *portModePtr = _100GBase_KR4;
        }
        else if (AP_CTRL_40GBase_CR4_GET(capability))
        {
            *portModePtr = _40GBase_CR4;
        }
        else if (AP_CTRL_40GBase_KR4_GET(capability))
        {
            *portModePtr = _40GBase_KR4;
        }
        else
        {
            *portModePtr = _10GBase_KX4;
        }
    }
    else if (capability & ((1<<AP_CTRL_100GBase_CR2_SHIFT) | (1<<AP_CTRL_100GBase_KR2_SHIFT)|
                           (1<<AP_CTRL_50GBase_CR2_CONSORTIUM_SHIFT)| (1<<AP_CTRL_50GBase_KR2_CONSORTIUM_SHIFT)|
                           (1<<AP_CTRL_10GBase_KX2_SHIFT) | (1<<AP_CTRL_20GBase_KR2_SHIFT)))
    {
        if(AP_CTRL_100GBase_CR2_GET(capability))
        {
            *portModePtr = _100GBase_CR2;
        }
        else if (AP_CTRL_100GBase_KR2_GET(capability))
        {
            *portModePtr = _100GBase_KR2;
        }
        else if (AP_CTRL_50GBase_CR2_CONSORTIUM_GET(capability))
        {
            *portModePtr = _50GBase_CR2_C;
        }
        else if (AP_CTRL_50GBase_KR2_CONSORTIUM_GET(capability))
        {
            *portModePtr = _50GBase_KR2_C;
        }
        else if (AP_CTRL_10GBase_KX2_GET(capability))
        {
            *portModePtr = _10GBase_KX2;
        }
        else
        {
            *portModePtr = _20GBase_KR2;
        }
    }
    else if (capability != 0)
    {
        if(AP_CTRL_10GBase_KR_GET(capability))
        {
            *portModePtr = _10GBase_KR;
        }
        else if(AP_CTRL_1000Base_KX_GET(capability))
        {
            *portModePtr = _1000Base_X;
        }
        else if(AP_CTRL_25GBase_KR1_GET(capability))
        {
            *portModePtr = _25GBase_KR;
        }
        else if(AP_CTRL_25GBase_KR1S_GET(capability))
        {
            *portModePtr = _25GBase_KR_S;
        }
        else if(AP_CTRL_25GBase_KR_CONSORTIUM_GET(capability))
        {
            *portModePtr = _25GBase_KR_C;
        }
        else if(AP_CTRL_25GBase_CR_CONSORTIUM_GET(capability))
        {
            *portModePtr = _25GBase_CR_C;
        }
        else if(AP_CTRL_25GBase_CR1S_GET(capability))
        {
            *portModePtr = _25GBase_CR_S;
        }
        else if(AP_CTRL_25GBase_CR1_GET(capability))
        {
            *portModePtr = _25GBase_CR;
        }
        else if(AP_CTRL_50GBase_KR1_GET(capability))
        {
            *portModePtr = _50GBase_KR;
        }
        else if(AP_CTRL_50GBase_CR1_GET(capability))
        {
            *portModePtr = _50GBase_CR;
        }
        else
        {
            return GT_BAD_STATE;
        }
    }
    else
    {
        return GT_BAD_STATE;
    }

    return GT_OK;
}

#ifndef BOBK_DEV_SUPPORT /*AC5-TBD*/
/**
* @internal mvHwsPortEnableGet function
* @endinternal
*
* @brief  Get port enable from mac.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[out] enablePtr                   - enable or disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortEnableGet
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    OUT GT_BOOL                        *enablePtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    /* Getting port params */
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_INITIALIZED;
    }
    /* Performing MAC port enable */
    CHECK_STATUS(mvHwsMacPortEnableGet(devNum, portGroup, curPortParams.portMacNumber,  portMode,
                                                curPortParams.portMacType, enablePtr));
    return GT_OK;
}
#endif /*bobk*/


#if !defined(BC2_DEV_SUPPORT) && !defined(AC3_DEV_SUPPORT) && !defined(AC5_DEV_SUPPORT) && !defined(CM3) && defined(CHX_FAMILY)
/**
* @internal mvHwsPortTuneResCopyEnable function
* @endinternal
*
* @brief   Enable copy of auto tune results.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] enable                   - enable or disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTuneResCopyEnable
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_BOOL                 enable
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;

    copyTuneResults = enable;

    return GT_OK;
}

/**
* @internal mvHwsPortSetEyeCheckEn function
* @endinternal
*
* @brief   Enables/Disabled eye check according to the receive value.
*         Actual configuration (to the device) will be done in mvHwsComHRev2SerdesPostTrainingConfig().
*
* @param[in] devNum                   - device number.
* @param[in] portGroup                - port group (core) number
* @param[in] eyeCheckEn               - value for enables/disabled check eye
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortSetEyeCheckEn
(
    IN GT_U8     devNum,
    IN GT_U32    portGroup,
    IN GT_BOOL   eyeCheckEn
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;

    return mvHwsComHRev2SerdesSetEyeCheckEn(eyeCheckEn);
}

/**
* @internal mvHwsPortSetPresetCommand function
* @endinternal
*
* @brief   Setting PRESET command according to the receive value.
*         If PRESET command configure to PRESET, eye check will disable.
*         Actual configuration (to the device) will be done in mvHwsComHRev2SerdesPostTrainingConfig().
*
* @param[in] devNum                   - device number.
* @param[in] portGroup                - port group (core) number
* @param[in] presetCmdVal             - value for setting PRESET command:
*                                      1 = INIT
*                                      2 = PRESET
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortSetPresetCommand
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN MV_HWS_PORT_PRESET_CMD  presetCmdVal
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;

    CHECK_STATUS(mvHwsComHRev2SerdesSetPresetCommand(presetCmdVal));

    /* in case Preset command == PRESET -> disable eye check */
    if (presetCmdVal == PRESET)
        return mvHwsComHRev2SerdesSetEyeCheckEn(GT_FALSE);

    return GT_OK;
}

#endif /* #if !defined(BC2_DEV_SUPPORT) && !defined(AC3_DEV_SUPPORT) && !defined(CM3) && defined(CHX_FAMILY) */

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
/**
* @internal mvHwsPortCtleBiasOverrideEnableSet function
* @endinternal
*
* @brief   write to hws dataBase the override mode and value of Ctle Bias Parameter
*         per port.
*
* @param[in] devNum                   - device number.
* @param[in] phyPortNum               - physical port number.
* @param[in] ctleBiasOverride         - override mode.
* @param[in] ctleBiasValue            - CTLE Bias value [0..1].
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortCtleBiasOverrideEnableSet
(
    IN GT_U8      devNum,
    IN GT_U32     phyPortNum,
    IN GT_U32    ctleBiasOverride,
    IN GT_U32    ctleBiasValue
)
{
    GT_U32 bitMapIndex;

    if ((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (phyPortNum < 48))
    {
        phyPortNum /= 4;
    }

    bitMapIndex = phyPortNum / HWS_CTLE_BIT_MAP;

    if (ctleBiasOverride)
    {
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasOverride[bitMapIndex] |= (0x1 << (phyPortNum%HWS_CTLE_BIT_MAP));
    }
    else
    {
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasOverride[bitMapIndex] &= ~(0x1 << (phyPortNum%HWS_CTLE_BIT_MAP));
    }

    if (ctleBiasValue)
    {
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasValue[bitMapIndex] |= (0x1 << (phyPortNum%HWS_CTLE_BIT_MAP));
    }
    else
    {
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasValue[bitMapIndex] &= ~(0x1 << (phyPortNum%HWS_CTLE_BIT_MAP));
    }

    return GT_OK;
}

/**
* @internal mvHwsPortCtleBiasOverrideEnableGet function
* @endinternal
*
* @brief   read from hws dataBase the values of override mode and CTLE Bias.
*
* @param[in] devNum                   - device number.
* @param[in] phyPortNum               - physical port number.
*
* @param[out] ctleBiasOverridePtr      - pointer to override mode.
* @param[out] ctleBiasValuePtr         - pointer to CTLE Bias value.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortCtleBiasOverrideEnableGet
(
    IN GT_U8      devNum,
    IN GT_U32     phyPortNum,
    OUT GT_U32    *ctleBiasOverridePtr,
    OUT GT_U32    *ctleBiasValuePtr
)
{
    GT_U32 bitMapIndex;

    if ((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (phyPortNum < 48))
    {
        phyPortNum /= 4;
    }

    bitMapIndex = phyPortNum/HWS_CTLE_BIT_MAP;

    *ctleBiasOverridePtr = (hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasOverride[bitMapIndex] >> (phyPortNum%HWS_CTLE_BIT_MAP)) & 0x1;
    *ctleBiasValuePtr    = (hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasValue[bitMapIndex] >> (phyPortNum%HWS_CTLE_BIT_MAP)) & 0x1;

    return GT_OK;
}
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

#if defined (CHX_FAMILY) && (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) || defined (ALDRIN2_DEV_SUPPORT)
/*************************************************************************
* @internal mvHwsAldrin2PortLaneMacToSerdesMuxSet
* @endinternal
 *
* @param[in] devNum              - system device number
* @param[in] portGroup           - port group (core) number
* @param[in] phyPortNum          - physical port number
* @param[in] macToSerdesMuxStc   - struct that contain the connectivity of the mux
************************************************************************/
static GT_STATUS mvHwsAldrin2PortLaneMacToSerdesMuxSet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           portGroup,
    IN  GT_U32                          phyPortNum,
    IN  MV_HWS_PORT_SERDES_TO_MAC_MUX   *macToSerdesMuxStc
)
{
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) || defined(ALDRIN2_DEV_SUPPORT)
    GT_U16          tmpCheckingCounterArray[] = {0,0,0,0};
    GT_U16          i;
    GT_U16          macNum;
    GT_U16          serdesNum;
    GT_UREG_DATA    tempRegValue = 0;

    GT_UREG_DATA    serdesToMacTxRegValue[4][4] = /* [serdesNum][macNum] */
    { /*  mac0      mac1       mac2        mac3 */
        { 0x0<<0,   0,         0x1<<0,     0x2<<0  },   /*serdes0*/
        { 0,        0x0<<4,    0x1<<4,     0x2<<4  },   /*serdes1*/
        { 0x1<<8,   0x2<<8,    0x0<<8,     0       },   /*serdes2*/
        { 0x1<<12,  0x2<<12,   0,          0x0<<12 }    /*serdes3*/
    };

    GT_UREG_DATA    macToSerdesRxRegValue[4][4] = /* [macNum][serdesNum] */
    { /*  serdes0   serdes1    serdes2     serdes3 */
        { 0x0<<2,   0,         0x1<<2,     0x2<<2  },   /*mac0*/
        { 0,        0x0<<6,    0x1<<6,     0x2<<6  },   /*mac1*/
        { 0x1<<10,  0x2<<10,   0x0<<10,    0       },   /*mac2*/
        { 0x1<<14,  0x2<<14,   0,          0x0<<14 }    /*mac3*/
    };

    /* serdes muxing work only on ports 0-23 on Aldrin2*/
    if(!((HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) && (phyPortNum < 24)))
    {
        return GT_NOT_SUPPORTED;
    }
    /* serdes muxing work only on ports that are multiple of 4 */
    if ((phyPortNum % 4) != 0)
    {
        return GT_NOT_SUPPORTED;
    }

    /* checking for legal connectivity for each serdes and that its in the range between 0-3
     mac lane 0 -> Serdes lane 0,2,3 Return GT_BAD_PARAM for 1
     mac lane 1 -> Serdes lane 1,2,3 Return GT_BAD_PARAM for 0
     mac lane 2 -> Serdes lane 0,1,2 Return GT_BAD_PARAM for 3
     mac lane 3 -> Serdes lane 0,1,3 Return GT_BAD_PARAM for 2 */
    if ((macToSerdesMuxStc->serdesLanes[0] == 1) || (macToSerdesMuxStc->serdesLanes[0] > 3))
    {
                return GT_BAD_PARAM;
    }
    if ((macToSerdesMuxStc->serdesLanes[1] == 0) || (macToSerdesMuxStc->serdesLanes[1] > 3))
    {
                return GT_BAD_PARAM;
    }
    if ((macToSerdesMuxStc->serdesLanes[2] == 3) || (macToSerdesMuxStc->serdesLanes[2] > 3))
    {
                return GT_BAD_PARAM;
    }
    if ((macToSerdesMuxStc->serdesLanes[3] == 2) || (macToSerdesMuxStc->serdesLanes[3] > 3))
    {
                return GT_BAD_PARAM;
    }

    /* checking validity of the values from the input: each number 0,1,2,3 should appear EXACTLY one time */
    for(i = 0; i < 4; i++)
    {   /* counting the number of occurrences of each of one the serdes numbers */
        tmpCheckingCounterArray[macToSerdesMuxStc->serdesLanes[i]]++;
    }

    for (i = 0; i < 4; i++)
    {
        if (tmpCheckingCounterArray[i] != 1)
        {
            return GT_BAD_PARAM;
        }
    }

    /* Aldrin2 Serdes Muxing has sillicon bug in the following permutations:
       mac[0,1,2,3] -> serdes[3,2,0,1]
       mac[0,1,2,3] -> serdes[2,3,1,0] */
    if((macToSerdesMuxStc->serdesLanes[0] == 3 && macToSerdesMuxStc->serdesLanes[1] == 2 && macToSerdesMuxStc->serdesLanes[2] == 0 && macToSerdesMuxStc->serdesLanes[3] == 1) ||
       (macToSerdesMuxStc->serdesLanes[0] == 2 && macToSerdesMuxStc->serdesLanes[1] == 3 && macToSerdesMuxStc->serdesLanes[2] == 1 && macToSerdesMuxStc->serdesLanes[3] == 0))
    {
       return GT_NOT_SUPPORTED;
    }

    for(macNum = 0 ; macNum < 4; macNum++)
    {
        serdesNum       = macToSerdesMuxStc->serdesLanes[macNum];

        tempRegValue    |= serdesToMacTxRegValue[serdesNum][macNum];
        tempRegValue    |= macToSerdesRxRegValue[macNum][serdesNum];
    }

    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, phyPortNum, MACRO_CONTROL, tempRegValue, 0xffff));
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT,  phyPortNum, GIG_PORT_MAC_MACRO_CONTROL, tempRegValue, 0xffff));

    return GT_OK;
#else
    devNum = devNum;
    portGroup = portGroup;
    phyPortNum = phyPortNum;
    macToSerdesMuxStc = macToSerdesMuxStc;
    return GT_NOT_SUPPORTED;
#endif
}

/*************************************************************************
* @internal mvHwsAldrin2PortLaneMacToSerdesMuxGet
* @endinternal
 *
* @param[in] devNum              - physical device number
* @param[in] portGroup           - port group (core) number
* @param[in] phyPortNum          - physical port number
* @param[out] *macToSerdesMuxStc -ptr to struct that will contaion the connectivity of the mux we got from the register
************************************************************************/
static GT_STATUS mvHwsAldrin2PortLaneMacToSerdesMuxGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           portGroup,
    IN  GT_U32                          phyPortNum,
    OUT MV_HWS_PORT_SERDES_TO_MAC_MUX   *macToSerdesMuxStc
)
{
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) || defined(ALDRIN2_DEV_SUPPORT)
    GT_UREG_DATA    data;
    GT_U8           tmpCheckingCounterArray[] = {0,0,0,0};
    GT_U16          i;
    GT_U16          macNum;

    macToSerdesMuxStc->enableSerdesMuxing = GT_FALSE;
    if(!((HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) && (phyPortNum < 24)))
    {
        return GT_NOT_SUPPORTED;
    }

    if ((phyPortNum % 4) != 0)
    {    /* serdes muxing work only on ports that are multiple of 4 */
        phyPortNum = phyPortNum & 0xfffffffc; /* use the first port in the quad instead */
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, GEMAC_UNIT, phyPortNum, GIG_PORT_MAC_MACRO_CONTROL, &data, 0x0000FFFF));
    if (data == 0)
    {
        macToSerdesMuxStc->serdesLanes[0] = 0;
        macToSerdesMuxStc->serdesLanes[1] = 0;
        macToSerdesMuxStc->serdesLanes[2] = 0;
        macToSerdesMuxStc->serdesLanes[3] = 0;
        return GT_NOT_INITIALIZED;
    }

    /* Convert the data from the register */
    /*serdes 0*/
    if ((data & 0x3) == 0)
    {
        macNum = 0;
    }
    else if ((data & 0x3) == 1)
    {
        macNum = 2;
    }
    else
    {
        macNum = 3;
    }
    macToSerdesMuxStc->serdesLanes[macNum] = 0;

    /*serdes 1*/
    if (((data & 0x30) >> 4) == 0)
    {
        macNum = 1;
    }
    if (((data & 0x30) >> 4) == 1)
    {
        macNum = 2;
    }
    if ((((data & 0x30) >> 4) == 2)||(((data & 0x30) >> 4) == 3))
    {
        macNum = 3;
    }
    macToSerdesMuxStc->serdesLanes[macNum] = 1;

    /*serdes 2*/
    if (((data & 0x300) >> 8) == 0)
    {
        macNum = 2;
    }
    if (((data & 0x300) >> 8) == 1)
    {
        macNum = 0;
    }
    if ((((data & 0x300) >> 8) == 2)||(((data & 0x300) >> 8) == 3))
    {
        macNum = 1;
    }
    macToSerdesMuxStc->serdesLanes[macNum] = 2;

    /*serdes 3*/
    if (((data & 0x3000) >> 12) == 0)
    {
        macNum = 3;
    }
    if (((data & 0x3000) >> 12) == 1)
    {
        macNum = 0;
    }
    if ((((data & 0x3000) >> 12) == 2)||(((data & 0x3000) >> 12) == 3))
    {
        macNum = 1;
    }
    macToSerdesMuxStc->serdesLanes[macNum] = 3;

    /* checking validity of the values from resgisers: each number 0,1,2,3 should appear EXACTLY one time */
    for(i = 0; i < 4; i++)
    {   /* counting the number of occurrences of each of one the serdes numbers */
        tmpCheckingCounterArray[macToSerdesMuxStc->serdesLanes[i]]++;
    }
    for (i = 0; i < 4; i++)
    {
        if (tmpCheckingCounterArray[i] != 1)
        {
            macToSerdesMuxStc->serdesLanes[0] = 0;
            macToSerdesMuxStc->serdesLanes[1] = 0;
            macToSerdesMuxStc->serdesLanes[2] = 0;
            macToSerdesMuxStc->serdesLanes[3] = 0;
            return GT_NOT_INITIALIZED;
        }
    }

    macToSerdesMuxStc->enableSerdesMuxing = GT_TRUE;
    return GT_OK;
#else
    devNum = devNum;
    portGroup = portGroup;
    phyPortNum = phyPortNum;
    macToSerdesMuxStc = macToSerdesMuxStc;
    return GT_NOT_SUPPORTED;
#endif
}
#endif

#if ((defined (CHX_FAMILY) && (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3))) || defined (RAVEN_DEV_SUPPORT) || defined (FALCON_DEV_SUPPORT) )
 /*************************************************************************
* @internal mvHwsFalconPortLaneMacToSerdesMuxSet
* @endinternal
 *
* @param[in] devNum              - system device number
* @param[in] portGroup           - port group (core) number
* @param[in] phyPortNum          - physical port number
* @param[in] macToSerdesMuxStc   - struct that contain the connectivity of the mux
************************************************************************/

GT_STATUS mvHwsFalconPortLaneMacToSerdesMuxSet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           portGroup,
    IN  GT_U32                          phyPortNum,
    IN  MV_HWS_PORT_SERDES_TO_MAC_MUX   *macToSerdesMuxStc
)
{
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) || defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT)
    GT_U16          tmpCheckingCounterArray[] = {0,0,0,0,0,0,0,0};
    GT_U16          i;
    GT_U16          macNum;
    GT_U16          serdesNum;

    GT_BOOL         secondRavenHalf;

    GT_U32          baseAddr;
    GT_U32          unitIndex;
    GT_U32          unitNum;
    GT_U32          regAddr;

    if(((HWS_DEV_SILICON_TYPE(devNum) != Falcon) && (HWS_DEV_SILICON_TYPE(devNum) != Raven)) || (phyPortNum % 8 != 0) || mvHwsMtipIsReducedPort(devNum,phyPortNum))
    {
        return GT_NOT_SUPPORTED;
    }

    secondRavenHalf = ((phyPortNum % 16) > 7);

    /* checking validity of the values from the input: each number 0,1,2,3,4,5,6,7 should appear EXACTLY one time */
    for(i = 0; i < 8; i++)
    {   /* counting the number of occurrences of each of one the serdes numbers */
        tmpCheckingCounterArray[macToSerdesMuxStc->serdesLanes[i]]++;
    }

    for (i = 0; i < 8; i++)
    {
        if (tmpCheckingCounterArray[i] != 1)
        {
            return GT_BAD_PARAM;
        }
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_DFX_UNIT, phyPortNum, &baseAddr, &unitIndex, &unitNum));

    for(i = 0 ; i < 8; i++)
    {
        macNum    = i + (secondRavenHalf?8:0);
        serdesNum = macToSerdesMuxStc->serdesLanes[i] + (secondRavenHalf?8:0);

        /* Rx muxing */
        regAddr = baseAddr + falconMuxSeq[macNum][0];
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, regAddr, falconMuxSeq[macNum][1], 3, serdesNum - (secondRavenHalf?8:0)));
        /* Tx muxing */
        regAddr = baseAddr + falconMuxSeq[serdesNum][0];
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, regAddr, falconMuxSeq[serdesNum][1]+3, 3, macNum - (secondRavenHalf?8:0)));
    }

    return GT_OK;
#else
    devNum = devNum;
    portGroup = portGroup;
    phyPortNum = phyPortNum;
    macToSerdesMuxStc = macToSerdesMuxStc;
    return GT_NOT_SUPPORTED;
#endif
}

/*************************************************************************
* @internal mvHwsFalconPortLaneMacToSerdesMuxGet
* @endinternal
 *
* @param[in] devNum              - physical device number
* @param[in] portGroup           - port group (core) number
* @param[in] phyPortNum          - physical port number
* @param[out] *macToSerdesMuxStc -ptr to struct that will contaion the connectivity of the mux we got from the register
************************************************************************/
static GT_STATUS mvHwsFalconPortLaneMacToSerdesMuxGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           portGroup,
    IN  GT_U32                          phyPortNum,
    OUT MV_HWS_PORT_SERDES_TO_MAC_MUX   *macToSerdesMuxStc
)
{
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) || defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT)
    GT_U16          tmpCheckingCounterArray[] = {0,0,0,0,0,0,0,0};
    GT_U16          i;
    GT_U16          macNum;
    GT_U32          serdesNum;

    GT_BOOL         secondRavenHalf;

    GT_U32          baseAddr;
    GT_U32          unitIndex;
    GT_U32          unitNum;
    GT_U32          regAddr;

    macToSerdesMuxStc->enableSerdesMuxing = GT_FALSE;

    if(((HWS_DEV_SILICON_TYPE(devNum) != Falcon) && (HWS_DEV_SILICON_TYPE(devNum) != Raven))
#if !defined FALCON_DEV_SUPPORT
       || (mvHwsMtipIsReducedPort(devNum,phyPortNum))
#endif
    )
    {
        return GT_NOT_SUPPORTED;
    }

    secondRavenHalf = ((phyPortNum % 16) > 7);

    if (phyPortNum % 8 != 0)
    {    /* serdes muxing work only on ports that are multiple of 8 on falcon */
        phyPortNum = phyPortNum & 0xfffffff8; /* use the first port in the gop instead */
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_DFX_UNIT, phyPortNum, &baseAddr, &unitIndex, &unitNum));

    for(i = 0; i < 8; i++)
    {
        macNum    = i + (secondRavenHalf?8:0);

        regAddr = baseAddr + falconMuxSeq[macNum][0];
        CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, portGroup, regAddr, falconMuxSeq[macNum][1], 3,&serdesNum));

        macToSerdesMuxStc->serdesLanes[i] = (GT_U8)serdesNum;
    }

    /* checking validity of the values from resgisers: each number 0,1,2,3,4,5,6,7 should appear EXACTLY one time */
    for(i = 0; i < 8; i++)
    {   /* counting the number of occurrences of each of one the serdes numbers */
        tmpCheckingCounterArray[macToSerdesMuxStc->serdesLanes[i]]++;
    }

    for (i = 0; i < 8; i++)
    {
        if (tmpCheckingCounterArray[i] != 1)
        {
            for(i = 0 ; i < 8; i++)
            {
                macToSerdesMuxStc->serdesLanes[i] = 0;
            }
            return GT_NOT_INITIALIZED;
        }
    }

    macToSerdesMuxStc->enableSerdesMuxing = GT_TRUE;
    return GT_OK;
#else
    devNum = devNum;
    portGroup = portGroup;
    phyPortNum = phyPortNum;
    macToSerdesMuxStc = macToSerdesMuxStc;
    return GT_NOT_SUPPORTED;
#endif
}
#endif

#if ((defined (CHX_FAMILY) && (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3))))
/*************************************************************************
* @internal mvHwsSip6_10PortLaneMacToSerdesMuxSet
* @endinternal
*
* @param[in] devNum              - system device number
* @param[in] portGroup           - port group (core) number
* @param[in] phyPortNum          - physical port number
* @param[in] macToSerdesMuxStc   - struct that contain the connectivity of the mux
************************************************************************/
GT_STATUS mvHwsSip6_10PortLaneMacToSerdesMuxSet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           portGroup,
    IN  GT_U32                          phyPortNum,
    IN  MV_HWS_PORT_SERDES_TO_MAC_MUX   *macToSerdesMuxStc
)
{
    GT_U16                  tmpCheckingCounterArray[8] = {0};
    GT_U16                  i;
    GT_U16                  size;
    MV_HWS_HAWK_ANP_UNITS   fieldName;
    GT_U32                  regData;

    if(mvHwsMtipIsReducedPort(devNum,phyPortNum))
    {
        return GT_NOT_SUPPORTED;
    }
    if((HWS_DEV_SILICON_TYPE(devNum) == AC5X) && (phyPortNum == 50))
    {
        size = 4;
    }
    else if ((HWS_DEV_SILICON_TYPE(devNum) == AC5P) && (phyPortNum % MV_HWS_AC5P_GOP_PORT_NUM_CNS == 0))
    {
        size = 8;
    }
    else if (HWS_DEV_SILICON_TYPE(devNum) == Harrier)
    {
        if((phyPortNum % MV_HWS_HARRIER_GOP_PORT_NUM_CNS == 0) && (phyPortNum < 32))
        {
            size = 8;
        }
        else
        {
            size = 4;
        }
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }

    /* checking validity of the values from the input: each number 0,1,2,3 should appear EXACTLY one time */
    for(i = 0; i < size; i++)
    {   /* counting the number of occurrences of each of one the serdes numbers */
        tmpCheckingCounterArray[macToSerdesMuxStc->serdesLanes[i]]++;
    }

    for (i = 0; i < size; i++)
    {
        if (tmpCheckingCounterArray[i] != 1)
        {
            return GT_BAD_PARAM;
        }
    }

    for (i = 0; i < size; i++)
    {
        if(i < 4)
        {
            if(HWS_DEV_SILICON_TYPE(devNum) == AC5X)
            {
                fieldName = PHOENIX_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E + i;
            }
            else if(HWS_DEV_SILICON_TYPE(devNum) == AC5P)
            {
                fieldName = HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E + i;
            }
            else
            {
                fieldName = HARRIER_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E + i;
            }

        }
        else
        {
            if(HWS_DEV_SILICON_TYPE(devNum) == AC5P)
            {
                fieldName = HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL4_E + (i - 4);
            }
            else
            {
                fieldName = HARRIER_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL4_E + (i - 4);
            }
        }

        regData = macToSerdesMuxStc->serdesLanes[i];
        if (HWS_DEV_SILICON_TYPE(devNum) == Harrier)
        {
            /*
                Special treatment for Harrier (SD8..SD15):

                SERDES Lane#    DP#   EPI Port    SDW Index   SDW Inst    SDW Port
                        0       0     P0[0]           0           0            0
                        1             P1[0]                                    1
                        2             P2[0]                                    2
                        3             P3[0]                                    3
                        4             P0[1]           1           1            0
                        5             P1[1]                                    1
                        6             P2[1]                                    2
                        7             P3[1]                                    3
                        8       1     P0[1]           3           1            0
                        9             P1[1]                                    1
                        10            P2[1]                                    2
                        11            P3[1]                                    3
                        12            P0[0]           2           0            0
                        13            P1[0]                                    1
                        14            P2[0]                                    2
                        15            P3[0]                                    3
                        16      2     P0[0]           4           0            0
                        17            P1[0]                                    1
                        18            P2[0]                                    2
                        19            P3[0]                                    3


            */
            if((phyPortNum >= 16) && (phyPortNum <= 30))
            {
                regData = (regData + 4) % 8;
            }
        }

        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup,  phyPortNum, ANP_400_UNIT, fieldName, regData, NULL));
    }

    return GT_OK;
}

/*************************************************************************
* @internal mvHwsSip6_10PortLaneMacToSerdesMuxGet
* @endinternal
*
* @param[in] devNum              - system device number
* @param[in] portGroup           - port group (core) number
* @param[in] phyPortNum          - physical port number
* @param[out] macToSerdesMuxStc   - struct that contain the connectivity of the mux
************************************************************************/
GT_STATUS mvHwsSip6_10PortLaneMacToSerdesMuxGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           portGroup,
    IN  GT_U32                          phyPortNum,
    OUT MV_HWS_PORT_SERDES_TO_MAC_MUX   *macToSerdesMuxStc
)
{
    GT_UREG_DATA            fieldData[8];
    GT_U16                  i;
    GT_U16                  size;
    MV_HWS_HAWK_ANP_UNITS   fieldName;
    GT_U32                  firstGopPortNum = phyPortNum;

    if(mvHwsMtipIsReducedPort(devNum,phyPortNum))
    {
        return GT_NOT_SUPPORTED;
    }
    if((HWS_DEV_SILICON_TYPE(devNum) == AC5X) && (phyPortNum == 50))
    {
        size = 4;
    }
    else if ((HWS_DEV_SILICON_TYPE(devNum) == AC5P))
    {
        size = 8;
        firstGopPortNum = (phyPortNum / MV_HWS_AC5P_GOP_PORT_NUM_CNS) * MV_HWS_AC5P_GOP_PORT_NUM_CNS;
    }
    else if ((HWS_DEV_SILICON_TYPE(devNum) == Harrier))
    {
        if(phyPortNum < 32)
        {
            size = 8;
        }
        else
        {
            size = 4;
        }
        firstGopPortNum = (phyPortNum / MV_HWS_HARRIER_GOP_PORT_NUM_CNS) * MV_HWS_HARRIER_GOP_PORT_NUM_CNS;
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }

    macToSerdesMuxStc->enableSerdesMuxing = GT_FALSE;
    hwsOsMemSetFuncPtr(macToSerdesMuxStc, 0, sizeof(MV_HWS_PORT_SERDES_TO_MAC_MUX));


    for (i = 0; i < size; i++)
    {
        if(i < 4)
        {
            if(HWS_DEV_SILICON_TYPE(devNum) == AC5X)
            {
                fieldName = PHOENIX_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E + i;
            }
            else if ((HWS_DEV_SILICON_TYPE(devNum) == AC5P))
            {
                fieldName = HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E + i;
            }
            else
            {
                fieldName = HARRIER_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E + i;
            }
        }
        else
        {
            if ((HWS_DEV_SILICON_TYPE(devNum) == AC5P))
            {
                fieldName = HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL4_E + (i - 4);
            }
            else
            {
                fieldName = HARRIER_ANP_UNITS_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL4_E + (i - 4);
            }
        }

#ifdef GM_USED
        fieldData[i] = i;/* the GM returns 0xbadad ... modify to i */
#else
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, portGroup,  firstGopPortNum, ANP_400_UNIT, fieldName, &fieldData[i], NULL));
#endif /*GM_USED*/

        if(fieldData[i] >= 8)
        {
            GT_U32 swpIndex = fieldData[i] - 6;
            fieldName = HAWK_ANP_UNITS_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E;
            /*CHECK_STATUS(genUnitRegisterFieldGet(devNum, portGroup, (firstGopPortNum+2+(swpIndex-2)*4), ANP_USX_UNIT, fieldName, &fieldData[i], NULL));*/
            CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, portGroup, (firstGopPortNum+2+(swpIndex-2)*4), ANP_USX_UNIT, _20G_DXGMII, fieldName, &fieldData[i], NULL));
        }
    }

    for(i = 0; i < size; i++)
    {
        macToSerdesMuxStc->serdesLanes[i] = (GT_U8)fieldData[i];
        if (HWS_DEV_SILICON_TYPE(devNum) == Harrier)
        {
            if((phyPortNum >= 16) && (phyPortNum <= 30))
            {
                macToSerdesMuxStc->serdesLanes[i] = (GT_U8)((fieldData[i] + 4) % 8);
            }
        }
    }

    macToSerdesMuxStc->enableSerdesMuxing = GT_TRUE;
    return GT_OK;
}

#endif

/*************************************************************************
* @internal mvHwsPortLaneMacToSerdesMuxSet
* @endinternal
 *
* @param[in] devNum              - system device number
* @param[in] portGroup           - port group (core) number
* @param[in] phyPortNum          - physical port number
* @param[in] macToSerdesMuxStc   - struct that contain the connectivity of the mux
************************************************************************/
GT_STATUS mvHwsPortLaneMacToSerdesMuxSet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           portGroup,
    IN  GT_U32                          phyPortNum,
    IN  MV_HWS_PORT_SERDES_TO_MAC_MUX   *macToSerdesMuxStc
)
{
    GT_STATUS rc = GT_OK;
    portGroup = portGroup; /* avoid compilation warnings */
    phyPortNum = phyPortNum; /* avoid compilation warnings */

    if (macToSerdesMuxStc == NULL)
    {
        return GT_BAD_PTR;
    }

    switch(HWS_DEV_SILICON_TYPE(devNum))
    {
#if defined (CHX_FAMILY) || defined (MV_HWS_REDUCED_BUILD_EXT_CM3)
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) || defined (ALDRIN2_DEV_SUPPORT)
    case Aldrin2:
        rc = mvHwsAldrin2PortLaneMacToSerdesMuxSet(devNum,portGroup,phyPortNum,macToSerdesMuxStc);
        break;
#endif
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) || defined (RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT)
    case Falcon:
    case Raven:
        rc = mvHwsFalconPortLaneMacToSerdesMuxSet(devNum,portGroup,phyPortNum,macToSerdesMuxStc);
        break;
#endif
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3))
    case AC5X:
    case AC5P:
    case Harrier:
        rc = mvHwsSip6_10PortLaneMacToSerdesMuxSet(devNum,portGroup,phyPortNum,macToSerdesMuxStc);
        break;
#endif
#endif
    default:
        macToSerdesMuxStc->enableSerdesMuxing = GT_FALSE;
        return GT_NOT_SUPPORTED;
    }
    return rc;
}
/*************************************************************************
* @internal mvHwsPortLaneMacToSerdesMuxGet
* @endinternal
 *
* @param[in] devNum              - physical device number
* @param[in] portGroup           - port group (core) number
* @param[in] phyPortNum          - physical port number
* @param[out] *macToSerdesMuxStc -ptr to struct that will contaion the connectivity of the mux we got from the register
************************************************************************/
GT_STATUS mvHwsPortLaneMacToSerdesMuxGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           portGroup,
    IN  GT_U32                          phyPortNum,
    OUT MV_HWS_PORT_SERDES_TO_MAC_MUX   *macToSerdesMuxStc
)
{
    GT_STATUS rc;
    portGroup = portGroup; /* avoid compilation warnings */
    phyPortNum = phyPortNum; /* avoid compilation warnings */
    if (macToSerdesMuxStc == NULL)
    {
        return GT_BAD_PTR;
    }

    switch(HWS_DEV_SILICON_TYPE(devNum))
    {
#if defined (CHX_FAMILY) || defined (MV_HWS_REDUCED_BUILD_EXT_CM3)
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) || defined (ALDRIN2_DEV_SUPPORT)
    case Aldrin2:
        rc = mvHwsAldrin2PortLaneMacToSerdesMuxGet(devNum,portGroup,phyPortNum,macToSerdesMuxStc);
        break;
#endif
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) || defined (RAVEN_DEV_SUPPORT) || defined (FALCON_DEV_SUPPORT)
    case Falcon:
    case Raven:
        rc = mvHwsFalconPortLaneMacToSerdesMuxGet(devNum,portGroup,phyPortNum,macToSerdesMuxStc);
        break;
#endif
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3))
    case AC5P:
    case AC5X:
    case Harrier:
        rc = mvHwsSip6_10PortLaneMacToSerdesMuxGet(devNum,portGroup,phyPortNum,macToSerdesMuxStc);
        break;
#endif
#endif
    default:
        macToSerdesMuxStc->enableSerdesMuxing = GT_FALSE;
        return GT_NOT_SUPPORTED;
    }
    return rc;
}


/*************************************************************************
* @internal mvHwsPortLaneMuxGetMacFromSerdes
* @endinternal
 *
* @param[in] devNum             - physical device number
* @param[in] portGroup          - port group (core) number
* @param[in] phyPortNum         - physical port number
* @param[in] serdesNum          - the num of serdes for which to return the macNum
************************************************************************/
GT_U16 mvHwsPortLaneMuxingGetMacFromSerdes
(
    IN GT_U8                           devNum,
    IN GT_U8                           portGroup,
    IN GT_U32                          phyPortNum,
    IN GT_U16                          serdesNum
)
{
    GT_U8                           i;
    GT_U8                           serdesOffset;
    GT_U8                           serdesToMac[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U8                           numOfLanesInMux;
    GT_U32                          retVal;
    MV_HWS_PORT_SERDES_TO_MAC_MUX   macToSerdesStc;

    if( GT_OK != mvHwsPortLaneMacToSerdesMuxGet(devNum,portGroup,phyPortNum,&macToSerdesStc))
    {
        return serdesNum;
    }
    switch(HWS_DEV_SILICON_TYPE(devNum))
    {
    case Aldrin2:
        numOfLanesInMux = 4;
        retVal = phyPortNum & 0xfffffffc; /* first num in group of ports */
        serdesOffset = serdesNum & 0x3;
        break;
    case Falcon:
    case Raven:
        numOfLanesInMux = 8;
        retVal = phyPortNum & 0xfffffff8; /* first num in group of ports */
        serdesOffset = serdesNum & 0x7;
        break;
    default:
        return serdesNum;
    }

    for(i = 0; i < numOfLanesInMux ; i++)
    {
        serdesToMac[macToSerdesStc.serdesLanes[i]] = i;
    }

    return  (GT_U16)(retVal + serdesToMac[serdesOffset]) /* relative MAC num in the group of ports */;
}

#ifndef MV_HWS_REDUCED_BUILD
/**
* @internal mvHwsMifPfcModeSet function
* @endinternal
*
* @brief  MIF pfc16Mode set
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical port number
* @param[in] portMode              - port mode
* @param[in] pfc16ModeEnable       - flow control type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMifPfcModeSet
(
    GT_U8               devNum,
    GT_U32              portNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL             pfc16ModeEnable
)
{
    MV_HWS_UNITS_ID     unitId;
    GT_U32              fieldName;
    MV_HWS_HAWK_CONVERT_STC convertIdx;

#ifdef CHX_FAMILY
    if (hwsIsIronmanAsPhoenix())
    {
        /* Ironman's MIF unit has no configuration of PFC size */
        return GT_OK;
    }
#endif /*CHX_FAMILY*/
    CHECK_STATUS(mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx));
    if ((mvHwsUsxModeCheck(devNum, portNum, portMode)) || (mvHwsMtipIsReducedPort(devNum, portNum)))
    {
        if (mvHwsMtipIsReducedPort(devNum, portNum))
        {
            unitId = MIF_CPU_UNIT;
        }
        else
        {
            unitId = MIF_USX_UNIT;
        }
        fieldName = MIF_GLOBAL_REGISTER_MIF_T8_CHANNEL_MAPPING_REGISTER0_T8_PFC_SIZE_E;
    }
    else if (HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        unitId = MIF_400_SEG_UNIT;
        fieldName = MIF_GLOBAL_REGISTER_MIF_T128_CHANNEL_MAPPING_REGISTER0_T128_PFC_SIZE_E;
    }
    else
    {
        unitId = MIF_400_UNIT;
        fieldName = MIF_GLOBAL_REGISTER_MIF_T32_CHANNEL_MAPPING_REGISTER0_T32_PFC_SIZE_E;
    }
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, fieldName, pfc16ModeEnable, NULL));

    if ((convertIdx.preemptionChannelId != 0x3F) && (unitId == MIF_400_UNIT))
    {
        unitId = MIF_400_BR_UNIT;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, fieldName, pfc16ModeEnable, NULL));
    }

    return GT_OK;
}

/**
* @internal mvHwsMifPfcModeGet function
* @endinternal
*
* @brief  MIF pfc16Mode get
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical port number
* @param[in] portMode              - port mode
* @param[in] pfc16ModeEnablePtr    - flow control type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMifPfcModeGet
(
    GT_U8           devNum,
    GT_U32          portNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL         *pfc16ModeEnablePtr
)
{
    MV_HWS_UNITS_ID     unitId;
    GT_U32              fieldName, fieldData;
    MV_HWS_HAWK_CONVERT_STC convertIdx;

#ifdef CHX_FAMILY
    if (hwsIsIronmanAsPhoenix())
    {
        /* Ironman's MIF unit has no configuration of PFC size */
        *pfc16ModeEnablePtr = GT_FALSE;
        return GT_OK;
    }
#endif /*CHX_FAMILY*/

    CHECK_STATUS(mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx));

    if ((mvHwsUsxModeCheck(devNum, portNum, portMode)) || (mvHwsMtipIsReducedPort(devNum, portNum)))
    {
        if (mvHwsMtipIsReducedPort(devNum, portNum))
        {
            unitId = MIF_CPU_UNIT;
        }
        else
        {
            unitId = MIF_USX_UNIT;
        }
        fieldName = MIF_GLOBAL_REGISTER_MIF_T8_CHANNEL_MAPPING_REGISTER0_T8_PFC_SIZE_E;
    }
    else if (HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        unitId = MIF_400_SEG_UNIT;
        fieldName = MIF_GLOBAL_REGISTER_MIF_T128_CHANNEL_MAPPING_REGISTER0_T128_PFC_SIZE_E;
    }
    else
    {
        unitId = MIF_400_UNIT;
        fieldName = MIF_GLOBAL_REGISTER_MIF_T32_CHANNEL_MAPPING_REGISTER0_T32_PFC_SIZE_E;
    }
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, fieldName, &fieldData, NULL));
    *pfc16ModeEnablePtr = (GT_BOOL)fieldData;

    return GT_OK;
}



/**
* @internal mvHwsMifPfcEnable function
* @endinternal
*
* @brief  handle MIF pfc enable/disable
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical port number
* @param[in] portMode              - port mode
* @param[in] isTx                  - direction of flow control
* @param[in] modify                - if set to GT_TRUE then update field,otherwise only read
* @param[inout] enablePtr          - (pointer to)enable TX/RX PFC
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMifPfcEnable
(
    GT_U8                devNum,
    GT_U32               portNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              isTx,
    GT_BOOL              modify,
    GT_BOOL              *enablePtr
)
{
    MV_HWS_UNITS_ID     unitId;
    GT_U32              fieldNameTxEn,fieldNameRxEn,fieldName;
    GT_U32              fieldNameTxEn1,fieldNameRxEn1, fieldName1;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_UREG_DATA    fieldData;

#ifdef CHX_FAMILY
    if (hwsIsIronmanAsPhoenix())
    {
        /* Ironman's MIF unit supportded by specific CPSS code */
        return GT_OK;
    }
#endif /*CHX_FAMILY*/

    fieldNameTxEn1 = MIF_REGISTER_LAST_E;
    fieldNameRxEn1 = MIF_REGISTER_LAST_E;

    CHECK_STATUS(mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx));

    if ((mvHwsUsxModeCheck(devNum, portNum, portMode)) || (mvHwsMtipIsReducedPort(devNum, portNum)))
    {
        if (mvHwsMtipIsReducedPort(devNum, portNum))
        {
            unitId = MIF_CPU_UNIT;
        }
        else
        {
            unitId = MIF_USX_UNIT;
        }

        if(HWS_DEV_SILICON_TYPE(devNum) == AC5X)
        {
            if (mvHwsMtipIsReducedPort(devNum, portNum))
            {
                MV_HWS_SET_FIELD_IDX0_24_MAC(fieldNameTxEn, convertIdx.ciderUnit, MIF_TX_REGISTER_MIF_T8_TX_PFC_CONTROL_REGISTER_MIF_T8_TX_PFC_EN, _E);
                MV_HWS_SET_FIELD_IDX0_24_MAC(fieldNameRxEn, convertIdx.ciderUnit, MIF_RX_REGISTER_MIF_T8_RX_PFC_CONTROL_REGISTER_MIF_T8_RX_PFC_EN, _E);
            }
            else
            {
                MV_HWS_SET_FIELD_IDX0_24_MAC(fieldNameTxEn, convertIdx.ciderIndexInUnit + 8 * (convertIdx.ciderUnit % 2), MIF_TX_REGISTER_MIF_T8_TX_PFC_CONTROL_REGISTER_MIF_T8_TX_PFC_EN, _E);
                MV_HWS_SET_FIELD_IDX0_24_MAC(fieldNameRxEn, convertIdx.ciderIndexInUnit + 8 * (convertIdx.ciderUnit % 2), MIF_RX_REGISTER_MIF_T8_RX_PFC_CONTROL_REGISTER_MIF_T8_RX_PFC_EN, _E);
            }
        }
        else /* AC5P, Harrier*/
        {
            MV_HWS_SET_FIELD_IDX0_24_MAC(fieldNameTxEn, convertIdx.ciderIndexInUnit, MIF_TX_REGISTER_MIF_T8_TX_PFC_CONTROL_REGISTER_MIF_T8_TX_PFC_EN, _E);
            MV_HWS_SET_FIELD_IDX0_24_MAC(fieldNameRxEn, convertIdx.ciderIndexInUnit, MIF_RX_REGISTER_MIF_T8_RX_PFC_CONTROL_REGISTER_MIF_T8_RX_PFC_EN, _E);
        }
    }
    else if (HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        unitId = MIF_400_SEG_UNIT;
        MV_HWS_SET_FIELD_IDX0_1_MAC(fieldNameTxEn, (convertIdx.ciderIndexInUnit/4), MIF_TX_REGISTER_MIF_T128_TX_PFC_CONTROL_REGISTER_MIF_T128_TX_PFC_EN, _E);
        MV_HWS_SET_FIELD_IDX0_1_MAC(fieldNameRxEn, (convertIdx.ciderIndexInUnit/4), MIF_RX_REGISTER_MIF_T128_RX_PFC_CONTROL_REGISTER_MIF_T128_RX_PFC_EN, _E);

    }
    else
    {
        unitId = MIF_400_UNIT;
        MV_HWS_SET_FIELD_IDX0_15_MAC(fieldNameTxEn, convertIdx.ciderIndexInUnit, MIF_TX_REGISTER_MIF_T32_TX_PFC_CONTROL_REGISTER_MIF_T32_TX_PFC_EN, _E);
        MV_HWS_SET_FIELD_IDX0_15_MAC(fieldNameRxEn, convertIdx.ciderIndexInUnit, MIF_RX_REGISTER_MIF_T32_RX_PFC_CONTROL_REGISTER_MIF_T32_RX_PFC_EN, _E);

        MV_HWS_SET_FIELD_IDX0_15_MAC(fieldNameTxEn1, (convertIdx.ciderIndexInUnit + 8), MIF_TX_REGISTER_MIF_T32_TX_PFC_CONTROL_REGISTER_MIF_T32_TX_PFC_EN, _E);
        MV_HWS_SET_FIELD_IDX0_15_MAC(fieldNameRxEn1, (convertIdx.ciderIndexInUnit + 8), MIF_RX_REGISTER_MIF_T32_RX_PFC_CONTROL_REGISTER_MIF_T32_RX_PFC_EN, _E);
    }

    fieldName=(GT_TRUE==isTx)?fieldNameTxEn:fieldNameRxEn;
    fieldName1=(GT_TRUE==isTx)?fieldNameTxEn1:fieldNameRxEn1;

    if(GT_TRUE == modify)
    {
        fieldData = *enablePtr;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, fieldName,fieldData, NULL));

        if ((convertIdx.preemptionChannelId != 0x3F) && (unitId == MIF_400_UNIT))
        {
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MIF_400_BR_UNIT, fieldName1,fieldData, NULL));
        }
    }
    else
    {
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, fieldName, &fieldData, NULL));
        *enablePtr = (((fieldData) == 1) ? GT_TRUE : GT_FALSE);
    }

    return GT_OK;
}
#endif


