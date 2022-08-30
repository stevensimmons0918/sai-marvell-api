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
* @file prvCpssDxChPortDynamicPizzaArbiterWS.h
*
* @brief bobcat2 and higher dynamic (algorithmic) pizza arbiter Data structures (device specific)
*
* @version   9
********************************************************************************
*/
#ifndef __PRV_CPSS_DXCH_PORT_DYNAMIC_PIZZA_ARBITER_WS_H
#define __PRV_CPSS_DXCH_PORT_DYNAMIC_PIZZA_ARBITER_WS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortTxQHighSpeedPortsDrv.h>


typedef struct
{
    GT_U32      exactAlgoHighSpeedPortThresh;
    GT_U32      apprAlgoHighSpeedPortThresh_Q20;
}PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC;

extern GT_STATUS BuildPizzaDistribution_NO_GCD  /* for use with special TXQ client group */
(
    IN   GT_FLOAT64  *portsConfigArrayPtr,
    IN   GT_U32       size,
    IN   GT_U32       maxPipeSize,
    IN   GT_U32       pizzaArraySize,
    OUT  GT_U32      *pizzaArray,
    OUT  GT_U32      *numberOfSlicesPtr
);


extern GT_STATUS BuildPizzaDistribution
(
    IN   GT_FLOAT64 *portsConfigArrayPtr,
    IN   GT_U32      size,
    IN   GT_U32      maxPipeSizeMbps,
    IN   GT_U32      minSliceResolutionMpbs,
    IN   PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC *highSpeedPortThreshPtr,
    IN   GT_U32      pizzaArraySize,
    OUT  GT_U32     *pizzaArray,
    OUT  GT_U32     *numberOfSlicesPtr,
    OUT  GT_U32     *highSpeedPortNumberPtr,
    OUT  GT_U32      highSpeedPortArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS]
);


extern GT_STATUS BuildPizzaDistributionWithDummySlice
(
    IN   GT_FLOAT64  *portsConfigArrayPtr,
    IN   GT_U32       size,
    IN   GT_U32       maxPipeSize,
    IN   GT_U32       minSliceResolution,
    IN   PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC *highSpeedPortThreshPtr,
    IN   GT_U32       pizzaArraySize,
    OUT  GT_U32      *pizzaArray,
    OUT  GT_U32      *numberOfSlicesPtr,
    OUT  GT_U32      *highSpeedPortNumPtr,
    OUT  GT_U32       highSpeedPortArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS]
);

extern GT_STATUS BuildTxQPizzaDistribution
(
    IN   GT_FLOAT64 *portsConfigArrayPtr,
    IN   GT_U32      size,
    IN   GT_U32      maxPipeSizeMbps,
    IN   GT_U32      minSliceResolutionMpbs,
    IN   PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC *highSpeedPortThreshPtr,
    IN   GT_U32      pizzaArraySize,
    OUT  GT_U32     *pizzaArray,
    OUT  GT_U32     *numberOfSlicesPtr,
    OUT  GT_U32     *highSpeedPortNumberPtr,
    OUT  GT_U32      highSpeedPortArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS]
);

extern GT_STATUS BuildPizzaDistribution_AldrinTxDMAGlue
(
    IN   GT_FLOAT64  *portsConfigArrayPtr,
    IN   GT_U32       size,
    IN   GT_U32       maxPipeSize,
    IN   GT_U32       minSliceResolution,
    IN   PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC *highSpeedPortThreshPtr,
    IN   GT_U32       pizzaArraySize,
    OUT  GT_U32      *pizzaArray,
    OUT  GT_U32      *numberOfSlicesPtr,
    OUT  GT_U32      *highSpeedPortNumPtr,
    OUT  GT_U32       highSpeedPortArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS]
);

extern GT_STATUS BuildPizzaDistributionTxQRemotesBoundedSliceN
(
    IN   GT_FLOAT64  *portsConfigArrayPtr,
    IN   GT_U32       size,
    IN   GT_U32       maxPipeSize,
    IN   GT_U32       minSliceResolution,
    IN   PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC *highSpeedPortThreshPtr,
    IN   GT_U32       pizzaArraySize,
    OUT  GT_U32      *pizzaArray,
    OUT  GT_U32      *numberOfSlicesPtr,
    OUT  GT_U32      *highSpeedPortNumPtr,
    OUT  GT_U32       highSpeedPortArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS]
);


/* function prototype that converts from Mapping to unit number */

typedef GT_STATUS (*PRV_CPSS_DXCH_MAPPING_2_UNIT_LIST_CONV_FUN)
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr,
    OUT GT_U32  portArgArr[CPSS_DXCH_PA_UNIT_MAX_E]
);

typedef GT_STATUS (* PRV_CPSS_DXCH_BUILD_PIZZA_DISTRIBUTION_FUN)
(
    IN   GT_FLOAT64 *portsConfigArrayPtr,
    IN   GT_U32      size,
    IN   GT_U32      maxPipeSize,
    IN   GT_U32      minSliceResolutionInMBps,
    IN   PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC *highSpeedPortThreshPtr,
    IN   GT_U32      pizzaArraySize,
    OUT  GT_U32     *pizzaArray,
    OUT  GT_U32     *numberOfSlicesPtr,
    OUT  GT_U32     *highSpeedPortNumberPtr,
    OUT  GT_U32      highSpeedPortArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS]
);

typedef GT_STATUS (* PRV_CPSS_DXCH_HIGH_SPEED_PORT_CONFIG_FUN)
(
    IN  GT_U8  devNum,
    IN  GT_U32  highSpeedPortNumber,
    IN  GT_U32  highSpeedPortArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS]
);


typedef struct
{
    GT_U32 coreClock;
    GT_U32 sysBwGbps;        /* system BW  used for Rx/TxDMAGlue, MPPM */
    GT_U32 dataPathN;        /* number of dataPaths. if 1 system BW is used for each unit */
    GT_U32 coreBWGbps[10];   /* data path <==> core BW, used  at RxDMA/TxDMA/TxFIFO */
    GT_U32 txqDqN;           /* number of txqDq */
    GT_U32 txqDqBWGbps[10];  /* txqDq BW  */
}PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC;

typedef struct
{
    GT_U32 revision;
    PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC *devBWPtr;
}PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC;



typedef struct
{
    CPSS_DXCH_PA_UNIT_ENT     unit;
    GT_U32                    mppmClientCode;
    GT_U32                    clientWeight;
}PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC;


typedef struct
{
    CPSS_DXCH_PA_UNIT_ENT unit;
    GT_U32                clientId;
}PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC;

/*
 *------------------------------------------------------------------------------------------*
 * configuration data (static,depends on device)                                            *
 *      used to control the flow of PA engine                                               *
 *------------------------------------------------------------------------------------------*
 * prv_DeviceUnitListPtr                                                                    *
 *       list of units, supported by device                                                 *
 *       ex : BC2 :                                                                         *
 *                 rx-dma, txq, tx-fifo, eth-tx-fFifo, tx-dma, ilkn-txdma                   *
 *            BobK pipe0-pipe1                                                              *
 *                 rx-dma,   txq, tx-fifo,   eth-tx-fFifo,   tx-dma,                        *
 *                 rx-dma-1,      tx-fifo-1, eth-tx-fFifo-1, tx-dma-1                       *
 *                 rx-dma-glue tx-dma-glue                                                  *
 *            BobK pipe1                                                                    *
 *                 rx-dma-1, txq  tx-fifo-1, eth-tx-fFifo-1, tx-dma-1                       *
 *                                                                                          *
 * prv_mappingType2UnitConfArr                                                              *
 *       list of units to configure for given mapping type with/wo TM                       *
 * prv_unit2PizzaAlgoFunArr                                                                 *
 *       algorithm to be used for pizza computa6ion for specific unit                       *
 *         ex:  DP units : Tx-Rx-DMA TxFIfo EthTxFifo -- dma algo (no empty slices          *
 *              TxQ      : TxQ algo : full space, min-distance between slices               *
 *              NULL     : unit is not configured                                           *
 * prv_unit2HighSpeedPortConfFunArr                                                         *
 *       function to configure high speed ports (relevant just for TxQ                      *
 *         all units but TxQ shall be initialized to NULL                                   *
 *------------------------------------------------------------------------------------------*
 * working data (stored intermediate results )                                              *
 *      used as input data for pizza computation                                            *
 *      updated after each computation cycle                                                *
 *------------------------------------------------------------------------------------------*
 * prv_portSpeedConfigArr                                                                   *
 *      stored speed of each port for each unit                                             *
 *------------------------------------------------------------------------------------------*
 */

typedef struct PRV_CPSS_DXCH_PA_WORKSPACE_STC
{
    PRV_CPSS_DXCH_REV_x_DEVICE_BW_STC               *devBwPtr;
    GT_U32                                           prv_speedEnt2MBitConvArr[CPSS_PORT_SPEED_NA_E];  /* CPSS API --> MBps speed conversion table */
    CPSS_DXCH_PA_UNIT_ENT                           *prv_DeviceUnitListPtr;
    GT_BOOL                                          prv_DeviceUnitListBmp                 [CPSS_DXCH_PA_UNIT_MAX_E];
    CPSS_DXCH_PA_UNIT_ENT                           *prv_unitListConfigByPipeBwSetPtr;
    GT_BOOL                                          prv_unitListConfigByPipeBwSetBmp      [CPSS_DXCH_PA_UNIT_MAX_E];
    CPSS_DXCH_PA_UNIT_ENT                           *prv_mappingType2UnitConfArr           [CPSS_DXCH_PORT_MAPPING_TYPE_MAX_E][GT_TRUE+1];
    PRV_CPSS_DXCH_BUILD_PIZZA_DISTRIBUTION_FUN       prv_unit2PizzaAlgoFunArr              [CPSS_DXCH_PA_UNIT_MAX_E];
    PRV_CPSS_DXCH_HIGH_SPEED_PORT_THRESH_PARAMS_STC  prv_txQHighSpeedPortThreshParams;
    PRV_CPSS_DXCH_HIGH_SPEED_PORT_CONFIG_FUN         prv_unit2HighSpeedPortConfFunArr      [CPSS_DXCH_PA_UNIT_MAX_E];
    PRV_CPSS_DXCH_MAPPING_2_UNIT_LIST_CONV_FUN       mapping2unitConvFunPtr;
    PRV_CPSS_DXCH_MPPM_CLIENT_CODING_STC            *mppmClientCodingListPtr;
    const CPSS_DXCH_PA_UNIT_ENT                     *workConservingModeOnUnitListPtr;
    GT_BOOL                                          removeRemoteFromTxQWC;
    CPSS_PORT_SPEED_ENT                              cpuSpeedEnm;
    PRV_CPSS_DXCH_PA_UNIT_CLIENT_IDX_STC            *tmUnitClientListPtr;
}PRV_CPSS_DXCH_PA_WORKSPACE_STC;


typedef struct PRV_CPSS_DXCH_BC2_PA_SUBFAMILY_2_WORKSPCE_STC
{
    CPSS_PP_SUB_FAMILY_TYPE_ENT      subFamily;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC  *workSpacePtr;
}PRV_CPSS_DXCH_BC2_PA_SUBFAMILY_2_WORKSPCE_STC;

typedef struct PRV_CPSS_DXCH_PA_WS_SET_STC
{
    GT_BOOL                                    isInit;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC             paWorkSpace_BC2_with_ilkn;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC             paWorkSpace_BC2_wo_ilkn;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC             paWorkSpace_BobK_Caelum_pipe0_pipe1;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC             paWorkSpace_BobK_Cetus_pipe1;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC             paWorkSpace_Aldrin;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC             paWorkSpace_AC3X; /* AlleyCat 3x = Aldrin + Remote Ports */
    PRV_CPSS_DXCH_PA_WORKSPACE_STC             paWorkSpace_AC3plus; /* AlleyCat 3+ = Aldrin + Remote Ports */
    PRV_CPSS_DXCH_PA_WORKSPACE_STC             paWorkSpace_BC3;
    PRV_CPSS_DXCH_PA_WORKSPACE_STC             paWorkSpace_ALDRIN2;
}PRV_CPSS_DXCH_PA_WS_SET_STC;


/**
* @internal prvCpssDxChPortDynamicPizzaArbiterWSInit function
* @endinternal
*
* @brief   Init Pizza Arbiter WSs (objects)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note I forced to place this function here, because it needs number of port
*       group where CPU port is connected and there is just no more suitable
*       place.
*
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterWSInit
(
    IN  GT_U8                   devNum
);


/**
* @internal prvCpssDxChPortDynamicPizzaArbiterWSGet function
* @endinternal
*
* @brief   Get Pizza Work space (object) by device
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] workSpacePtrPtr          - pointer to pointer on workspace.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - NULL ptr
* @retval GT_FAIL                  - on error
*
* @note I forced to place this function here, because it needs number of port
*       group where CPU port is connected and there is just no more suitable
*       place.
*
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterWSGet
(
    IN  GT_U8                            devNum,
    OUT PRV_CPSS_DXCH_PA_WORKSPACE_STC **workSpacePtrPtr
);

/**
* @internal prvCpssDxChPortDynamicPizzaArbiterWSSupportedUnitListGet function
* @endinternal
*
* @brief   Get list of supported units
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - NULL ptr
* @retval GT_FAIL                  - on error
*
* @note I forced to place this function here, because it needs number of port
*       group where CPU port is connected and there is just no more suitable
*       place.
*
*/
GT_STATUS prvCpssDxChPortDynamicPizzaArbiterWSSupportedUnitListGet
(
    IN  GT_U8                    devNum,
    OUT CPSS_DXCH_PA_UNIT_ENT  **supportedUnitListPtrPtr
);

GT_STATUS prvCpssDxChPortDynamicPizzaArbiterWSdevBwGet
(
    IN  GT_U8                    devNum,
    OUT PRV_CPSS_DXCH_CC_x_DEVICE_BW_STC **devBwPtrptr
);

GT_STATUS prvCpssDxChPortDynamicPizzaArbiterWScpuPortSpeedGet
(
    IN  GT_U8                    devNum,
    OUT CPSS_PORT_SPEED_ENT     *cpuSpeedEnmPtr
);



#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif






