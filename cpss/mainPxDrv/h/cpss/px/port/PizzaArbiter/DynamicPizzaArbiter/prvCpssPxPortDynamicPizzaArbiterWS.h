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
* @file prvCpssPxPortDynamicPizzaArbiterWS.h
*
* @brief pipe and higher dynamic (algorithmic) pizza arbiter Data structures (device specific)
*
* @version   1
********************************************************************************
*/
#ifndef __PRV_CPSS_PX_PORT_DYNAMIC_PIZZA_ARBITER_WS_H
#define __PRV_CPSS_PX_PORT_DYNAMIC_PIZZA_ARBITER_WS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
 

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/PizzaArbiter/cpssPxPortPizzaArbiter.h>
 
/**
* @internal prvCpssPxPortPABuildPizzaDistribution_NO_GCD function
* @endinternal
*
* @brief   Compute distrinution (no use of GCD)
*
* @note   APPLICABLE DEVICES:      None.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] portsConfigArrayPtr      - port configuration array to be used for calculation
* @param[in] size                     -  of array (portsConfigArrayPtr)
* @param[in] maxPipeSize              - pipe size
* @param[in] pizzaArray               - size of pizza array (where result shall be stored)
*
* @param[out] pizzaArray               - distribution
* @param[out] numberOfSlicesPtr        - number of slice to configure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortPABuildPizzaDistribution_NO_GCD  
(
    IN   GT_FLOAT64  *portsConfigArrayPtr,
    IN   GT_U32       size,
    IN   GT_U32       maxPipeSize,
    IN   GT_U32       pizzaArraySize,
    OUT  GT_U32      *pizzaArray,
    OUT  GT_U32      *numberOfSlicesPtr
);

/**
* @internal prvCpssPxPortPABuildPizzaDistribution function
* @endinternal
*
* @brief   Compuet distrinution (use of GCD)
*
* @note   APPLICABLE DEVICES:      None.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] portsConfigArrayPtr      - port configuration array to be used for calculation
* @param[in] size                     -  of array (portsConfigArrayPtr)
*                                      maxPipeSize         - pipe size
* @param[in] pizzaArray               - size of pizza array (where result shall be stored)
*
* @param[out] pizzaArray               - distribution
* @param[out] numberOfSlicesPtr        - number of slice to configure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortPABuildPizzaDistribution
(
    IN   GT_FLOAT64 *portsConfigArrayPtr,
    IN   GT_U32      size,
    IN   GT_U32      maxPipeSizeMbps,
    IN   GT_U32      minSliceResolutionMpbs,
    IN   GT_U32      pizzaArraySize,
    OUT  GT_U32     *pizzaArray,
    OUT  GT_U32     *numberOfSlicesPtr
);

/**
* @internal prvCpssPxPortPABuildPizzaDistributionWithDummySlice function
* @endinternal
*
* @brief   Compuet distrinution (use dummy slices)
*
* @note   APPLICABLE DEVICES:      None.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] portsConfigArrayPtr      - port configuration array to be used for calculation
* @param[in] size                     -  of array (portsConfigArrayPtr)
* @param[in] maxPipeSize              - pipe size
* @param[in] pizzaArray               - size of pizza array (where result shall be stored)
*
* @param[out] pizzaArray               - distribution
* @param[out] numberOfSlicesPtr        - number of slice to configure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
extern GT_STATUS prvCpssPxPortPABuildPizzaDistributionWithDummySlice
(
    IN   GT_FLOAT64  *portsConfigArrayPtr,
    IN   GT_U32       size,
    IN   GT_U32       maxPipeSize,
    IN   GT_U32       minSliceResolution,
    IN   GT_U32       pizzaArraySize,
    OUT  GT_U32      *pizzaArray,
    OUT  GT_U32      *numberOfSlicesPtr
);


/* function proptype that converts from Mapping to unit number */

typedef GT_STATUS (*PRV_CPSS_PX_MAPPING_2_UNIT_LIST_CONV_FUN)
(
    IN GT_SW_DEV_NUM devNum,
    IN CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr,
    OUT GT_U32  portArgArr[CPSS_PX_PA_UNIT_MAX_E]
);

typedef GT_STATUS (* PRV_CPSS_PX_BUILD_PIZZA_DISTRBUTION_FUN)
(
    IN   GT_FLOAT64 *portsConfigArrayPtr,
    IN   GT_U32      size,
    IN   GT_U32      maxPipeSize,
    IN   GT_U32      minSliceResolutionInMBps,
    IN   GT_U32      pizzaArraySize,
    OUT  GT_U32     *pizzaArray,
    OUT  GT_U32     *numberOfSlicesPtr
);

/**
* @struct PRV_CPSS_PX_CC_x_DEVICE_BW_STC
 *
 * @brief structure of device core clock and BW
*/
typedef struct{

    /** core clock */
    GT_U32 coreClock;

    /** system BW in Gbps */
    GT_U32 sysBwGbps;

    /** number of data pathes */
    GT_U32 dataPathN;

    GT_U32 coreBWGbps[10];

    /** number of DQ at TXQ */
    GT_U32 txqDqN;

    GT_U32 txqDqBWGbps[10];

} PRV_CPSS_PX_CC_x_DEVICE_BW_STC;

/**
* @struct PRV_CPSS_PX_REV_x_DEVICE_BW_STC
 *
 * @brief structure of device core clock and BW per revision
*/
typedef struct{

    /** @brief core clock
     *  devBWPtr  - pointer on device structure
     */
    GT_U32 revision;

    PRV_CPSS_PX_CC_x_DEVICE_BW_STC *devBWPtr;

} PRV_CPSS_PX_REV_x_DEVICE_BW_STC;


/*
 *-----------------------------------------------------------------------------------------------*
 * configuration data (static,depends on device)                                                 *
 *      used to control the flow of PA engine                                                    *
 *-----------------------------------------------------------------------------------------------*
 * prv_DeviceUnitListPtr                                                                         *
 *       list of units, supported by device                                                      *
 *       ex : Pipe :                                                                             *
 *                 rx-dma, txq, tx-dma, tx-fifo                                                  *
 *                                                                                               *
 * prv_mappingType2UnitConfArr                                                                   *
 *       list of units to configure for given mapping type with/wo TM                            *
 * prv_unit2PizzaAlgoFunArr                                                                      *
 *       algorithm to be used for pizza computa6ion for specific unit                            *
 *         ex:  DP units : Tx-Rx-DMA TxFIfo -- dma algo ( with dummy slices)                     *
 *              TxQ      : TxQ algo : no empty slice, number of slices defined as sum of speeds  *
 *              NULL     : unit is not configured                                                *
 * prv_unit2HighSpeedPortConfFunArr                                                              *
 *       function to configure high speed ports (relevant just for TxQ                           *
 *         all units but TxQ shall be initilized to NULL                                         *
 *-----------------------------------------------------------------------------------------------*
 * working data (stored intermediate results )                                                   *
 *      used as input data for pizza compurtaion                                                 *
 *      updated after each compuation cycle                                                      *
 *-----------------------------------------------------------------------------------------------*
 * prv_portSpeedConfigArr                                                                        *
 *      stored speed of each port for each unit                                                  *
 *-----------------------------------------------------------------------------------------------*
 */

typedef struct PRV_CPSS_PX_PA_WORKSPACE_STCT
{
    PRV_CPSS_PX_REV_x_DEVICE_BW_STC                 *devBwPtr;
    GT_U32                                           prv_speedEnt2MBitConvArr[CPSS_PORT_SPEED_NA_E];  /* CPSS API --> MBps speed conversion table */
    CPSS_PX_PA_UNIT_ENT                             *prv_DeviceUnitListPtr;
    GT_BOOL                                          prv_DeviceUnitListBmp                 [CPSS_PX_PA_UNIT_MAX_E];
    CPSS_PX_PA_UNIT_ENT                             *prv_unitListConfigByPipeBwSetPtr;
    GT_BOOL                                          prv_unitListConfigByPipeBwSetBmp      [CPSS_PX_PA_UNIT_MAX_E];
    CPSS_PX_PA_UNIT_ENT                             *prv_mappingType2UnitConfArr           [CPSS_PX_PORT_MAPPING_TYPE_MAX_E];
    PRV_CPSS_PX_BUILD_PIZZA_DISTRBUTION_FUN          prv_unit2PizzaAlgoFunArr              [CPSS_PX_PA_UNIT_MAX_E];
    PRV_CPSS_PX_MAPPING_2_UNIT_LIST_CONV_FUN         mapping2unitConvFunPtr;
    CPSS_PX_PA_UNIT_ENT                             *workConservingModeOnUnitListPtr;
    CPSS_PORT_SPEED_ENT                              cpuSpeedEnm;
}PRV_CPSS_PX_PA_WORKSPACE_STC;


typedef struct
{
    CPSS_PP_SUB_FAMILY_TYPE_ENT      subFamily;
    PRV_CPSS_PX_PA_WORKSPACE_STC    *workSpacePtr;
}PRV_CPSS_PX_BC2_PA_SUBFAMILY_2_WORKSPCE_STC;

typedef struct
{
    GT_BOOL                                  isInit;
    PRV_CPSS_PX_PA_WORKSPACE_STC             paWorkSpace_pipe;
}PRV_CPSS_PX_PA_WS_SET_STC;


/**
* @internal prvCpssPxPortDynamicPizzaArbiterWSInit function
* @endinternal
*
* @brief   Init Pizza Arbiter WSs (objects)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
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
GT_STATUS prvCpssPxPortDynamicPizzaArbiterWSInit
(
    IN GT_SW_DEV_NUM devNum
);


/**
* @internal prvCpssPxPortDynamicPizzaArbiterWSGet function
* @endinternal
*
* @brief   Get Pizza Work space (object) by device
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] workSpacePtrPtr          - pointer to pointer on workspace.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - NULL ptr
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterWSGet
(
    IN  GT_SW_DEV_NUM                    devNum,
    OUT PRV_CPSS_PX_PA_WORKSPACE_STC   **workSpacePtrPtr
);

/**
* @internal prvCpssPxPortDynamicPizzaArbiterWSSuportedUnitListGet function
* @endinternal
*
* @brief   Get list of supported units
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - NULL ptr
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterWSSuportedUnitListGet
(
    IN  GT_SW_DEV_NUM         devNum,
    OUT CPSS_PX_PA_UNIT_ENT  **supportedUnitListPtrPtr
);

/**
* @internal prvCpssPxPortDynamicPizzaArbiterWSdevBwGet function
* @endinternal
*
* @brief   Get device BW structure according to revision, cc
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] devBwPtrptr              - pointer to device BW structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - NULL ptr
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterWSdevBwGet
(
    IN  GT_SW_DEV_NUM                    devNum,
    OUT PRV_CPSS_PX_CC_x_DEVICE_BW_STC **devBwPtrptr
);

/**
* @internal prvCpssPxPortDynamicPizzaArbiterWScpuPortSpeedGet function
* @endinternal
*
* @brief   Get device CPU speed
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] cpuSpeedEnmPtr           - pointer to CPU speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - NULL ptr
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterWScpuPortSpeedGet
(
    IN  GT_SW_DEV_NUM            devNum,
    OUT CPSS_PORT_SPEED_ENT     *cpuSpeedEnmPtr
);



#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif






