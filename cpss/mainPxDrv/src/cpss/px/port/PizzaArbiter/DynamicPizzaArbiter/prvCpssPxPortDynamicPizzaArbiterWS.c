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
* @file prvCpssPxPortDynamicPizzaArbiterWS.c
*
* @brief pipe and higher dynamic (algorithmic) pizza arbiter Data structures (device specific)
*
* @version   93
********************************************************************************
*/
#include <cpss/px/config/cpssPxCfgInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPizzaArbiter.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPizzaArbiterWS.h>




#if 0
PIPE
+--------------+-----------------------------------+--------------------------------------------------+--------------------------------------------------------------------+------------------------------
|              |                       Control     |           Map                                    |                           Configuration                            |    Cider path
|              +------------+----------------------+------------+--------+--------------+-------------+------------------------+-------------------------------------------|
|   Unit       |            |          Fields      |            |        | Flds Statist | Single Fld  | Client                 |  Algo to be used                          |
|              | Register   +------+-------+-------|  Register  | number +------+-------+------+------|  Code                  |                                           |
|              |            | Slice| Map   |  WC   |            |        |  per | Total | Slice|Slice |                        |                                           |
|              |            |To-Run|Load-En|  mode |            |        |  reg |       |  Map | En   |                        |                                           |/Cider/EBU/PIPE/PIPE {Current}/Switching core
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+--------------------------------
| RXDMA-0      | 0x10002A00 |  0-8 | 14-14 | 15-15 | 0x10002A08 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | RXDMA/Units/<RXDMA_IP> RxDMA %a/<Pizza Arbiter>
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+------------------------------------------------------------
| TXDMA-0      | 0x02004000 |  0-8 | 14-14 | 15-15 | 0x02004008 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | TXDMA/Units/<TXDMA_IP> TxDMA %a/<Pizza Arb> TxDMA Pizza
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+---------------------------------------------------------------
| TXFIFO-0     | 0x04000800 |  0-8 | 14-14 | 15-15 | 0x04000808 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | TX_FIFO/Units/<TXFIFO_IP> TxFIFO %a/<Pizza Arb> TxFIFO
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+----------------------------------------------------
| TXQ-0        | 0x21001500 |  0-8 | 12-12 |  9-9  | 0x21001650 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per unit TXQ BW, no dummy slices (*)     | TXQ_IP/<TXQ_IP> TXQ_DQ/Units/TXQ_IP_dq/Scheduler/Port
| TXQ-1        | 0x22001500 |  0-8 | 12-12 |  9-9  | 0x22001650 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per unit TXQ BW, no dummy slices         |
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+----------------------------------------------------
* (cycle is defined as sum of all ports passing via that DQ)

#endif

/*-----------------------------------------*/
/* data declaration                        */
/*-----------------------------------------*/
/*
 * struct: PRV_CPSS_PX_PA_SPEEDENM_SPEEDGBPS_STC
 *          CPSS speed into Gbps conversion table
 *
 * Description: init params for PA object
 *
 *    speedEnm               - speed CPSS
 *    speedGbps              - speed in Gbps
 */
typedef struct
{
    CPSS_PORT_SPEED_ENT speedEnm;
    GT_U32              speedGbps;
}PRV_CPSS_PX_PA_SPEEDENM_SPEEDGBPS_STC;



/*
 * struct: PRV_CPSS_PX_MAPPING_2_UNITLIST_STC
 *          list of unit that shall be configured for each mapping type
 *
 * Description: init params for PA object
 *
 *    mappingType            - mapping type
 *    unitList2ConfigurePtr  - list of unit to configure
 */
typedef struct
{
    CPSS_PX_PORT_MAPPING_TYPE_ENT   mappingType;
    CPSS_PX_PA_UNIT_ENT            *unitList2ConfigurePtr;
}PRV_CPSS_PX_MAPPING_2_UNITLIST_STC;


/*
 * struct: PRV_CPSS_PX_UNIT_2_PIZZA_FUN_STC
 *          which algorith shall be used for speicfic unit
 *
 * Description: init params for PA object
 *
 *    unit     - PA unit
 *    fun      - fun to be used
 */
typedef struct
{
    CPSS_PX_PA_UNIT_ENT                       unit;
    PRV_CPSS_PX_BUILD_PIZZA_DISTRBUTION_FUN   fun;
}PRV_CPSS_PX_UNIT_2_PIZZA_FUN_STC;


typedef GT_STATUS  (*prvCpssPxMapping2UnitConvFUN)
(
    IN GT_SW_DEV_NUM                      devNum,
    IN CPSS_PX_DETAILED_PORT_MAP_STC     *portMapShadowPtr,
    OUT GT_U32                            portArgArr[CPSS_PX_PA_UNIT_MAX_E]
);

/*-----------------------------------------------*/
/*  Pizza Arbiter Workspace (object int params)  */
/*-----------------------------------------------*/
/*
 * struct: PRV_CPSS_PX_PA_WS_INIT_PARAMS_STC
 *          this params are used to initilize the specific PA object
 *
 * Description: init params for PA object
 *
 *    supportedUnitListPtr               - list of unit supported
 *                                                       (ended by CPSS_PX_PA_UNIT_UNDEFINED_E )
 *    unitListNotConfByPipeBwSetListPtr  - list of units not configured by cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionSet()
 *                                                       (ended by CPSS_PX_PA_UNIT_UNDEFINED_E )
 *    speedEnm2GbpsPtr                   - list of converstion CPSS speed to Gbps (ended { CPSS_PORT_SPEED_NA_E, 0}
 *    mapping2UnitConvFun                - function that convert CPSS mapping to device Mapping
 *                                         e.g. at DMA      -- to corresponding data path,
 *    mappingTm2UnitListPtr              - list of units to be configured at specific mapping with/wo Traffic Manager
 *                                                     ended     ,{ CPSS_PX_PORT_MAPPING_TYPE_INVALID_E, (GT_U32)(~0) ,   (CPSS_PX_PA_UNIT_ENT *)NULL}
 *    unit_2_pizzaCompFun                - list of unit --> Pizza Algorithm (for aech configured unit the correspong Pizza Algorithm)
 *                                                     ended  ,{ CPSS_PX_PA_UNIT_UNDEFINED_E    , (BuildPizzaDistributionFUN)NULL   }
 *   workConservingModeOnUnitListPtr     - list of unit where WorkConserving mode shall be enabled from start
 *                                                       (ended by CPSS_PX_PA_UNIT_UNDEFINED_E )
 *   cpuSpeedEnm                         - speed of CPU
 */

typedef struct
{
    PRV_CPSS_PX_REV_x_DEVICE_BW_STC                  *devBwPtr;
    CPSS_PX_PA_UNIT_ENT                              *supportedUnitListPtr;
    CPSS_PX_PA_UNIT_ENT                              *unitListConfByPipeBwSetListPtr;
    PRV_CPSS_PX_PA_SPEEDENM_SPEEDGBPS_STC            *speedEnm2GbpsPtr;
    prvCpssPxMapping2UnitConvFUN                      mapping2UnitConvFun;
    PRV_CPSS_PX_MAPPING_2_UNITLIST_STC               *mapping2UnitListPtr;
    PRV_CPSS_PX_UNIT_2_PIZZA_FUN_STC                 *unit_2_pizzaCompFun;
    CPSS_PX_PA_UNIT_ENT                              *workConservingModeOnUnitListPtr;
    CPSS_PORT_SPEED_ENT                               cpuSpeedEnm;
}PRV_CPSS_PX_PA_WS_INIT_PARAMS_STC;


/*----------------------------------------------------------*
 *  PIPE                                                     *
 *----------------------------------------------------------*/

static const  PRV_CPSS_PX_CC_x_DEVICE_BW_STC  prv_pipe_devBW[]        =
{
     /*  cc ,sysBw, dataPathN,  coreBWGbps[], txqDqN,    txqDqBW[]*/
     {  288 ,  101,         1, {      101 },       2,     {   60 ,  60}}
    ,{  350 ,  205,         1, {      205 },       2,     {  105 , 101}}/* 105 to support 8*12.5G (8*13=104) , 205 to support 104+101*/
    ,{  450 ,  260,         1, {      260 },       2,     {  150 , 150}}
    ,{  500 ,  260,         1, {      260 },       2,     {  160 , 160}}
    ,{    0 ,    0,         0, {        0 },       0,     {    0 ,   0}}
};



static const  PRV_CPSS_PX_REV_x_DEVICE_BW_STC prv_pipe_rev_x_devBW[] =
{
    { 0,              (PRV_CPSS_PX_CC_x_DEVICE_BW_STC*)&prv_pipe_devBW[0]    }
   ,{ (GT_U32)(-1),   (PRV_CPSS_PX_CC_x_DEVICE_BW_STC*)NULL                  }
};

static const  CPSS_PX_PA_UNIT_ENT   prv_pipe_UnitList[] =            /* list of units used in PIPE */
{
     CPSS_PX_PA_UNIT_RXDMA_0_E
    ,CPSS_PX_PA_UNIT_TXQ_0_E
    ,CPSS_PX_PA_UNIT_TXQ_1_E
    ,CPSS_PX_PA_UNIT_TXDMA_0_E
    ,CPSS_PX_PA_UNIT_TX_FIFO_0_E
    ,CPSS_PX_PA_UNIT_UNDEFINED_E         /* last unit */
};

static const  CPSS_PX_PA_UNIT_ENT   prv_pipe_unitListConfByPipeBwSetList[] =
{
     CPSS_PX_PA_UNIT_UNDEFINED_E           /* last unit */
};

static const  PRV_CPSS_PX_PA_SPEEDENM_SPEEDGBPS_STC prv_pipe_speedEnm2Gbps[]=
{
     {         CPSS_PORT_SPEED_1000_E,    1 }
    ,{         CPSS_PORT_SPEED_2500_E,    3 }
    ,{         CPSS_PORT_SPEED_5000_E,    5 }
    ,{        CPSS_PORT_SPEED_10000_E,   10 }
    ,{        CPSS_PORT_SPEED_11800_E,   12 }
    ,{        CPSS_PORT_SPEED_12500_E,   13 }
    ,{        CPSS_PORT_SPEED_20000_E,   20 }
    ,{        CPSS_PORT_SPEED_23600_E,   24 }
    ,{        CPSS_PORT_SPEED_25000_E,   25 }
    ,{        CPSS_PORT_SPEED_26700_E,   27 }
    ,{        CPSS_PORT_SPEED_40000_E,   40 }
    ,{        CPSS_PORT_SPEED_47200_E,   48 }
    ,{        CPSS_PORT_SPEED_50000_E,   50 }
    ,{        CPSS_PORT_SPEED_52500_E,   53 }
    ,{         CPSS_PORT_SPEED_100G_E,  100 }
    ,{         CPSS_PORT_SPEED_102G_E,  102 }
    ,{         CPSS_PORT_SPEED_107G_E,  107 }
    ,{           CPSS_PORT_SPEED_NA_E,    0 }
};



static const  PRV_CPSS_PX_MAPPING_2_UNITLIST_STC prv_pipe_mapping_2_unitlist[] =
{
     { CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E         ,   (CPSS_PX_PA_UNIT_ENT *)&prv_pipe_UnitList[0]           }
    ,{ CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E             ,   (CPSS_PX_PA_UNIT_ENT *)&prv_pipe_UnitList[0]           }
    ,{ CPSS_PX_PORT_MAPPING_TYPE_INVALID_E              ,   (CPSS_PX_PA_UNIT_ENT *)NULL                            }
};

static const  PRV_CPSS_PX_UNIT_2_PIZZA_FUN_STC prv_pipe_unit_2_pizzaFun[] =
{
     { CPSS_PX_PA_UNIT_RXDMA_0_E     , prvCpssPxPortPABuildPizzaDistributionWithDummySlice     }
    ,{ CPSS_PX_PA_UNIT_TXQ_0_E       , prvCpssPxPortPABuildPizzaDistribution                   }
    ,{ CPSS_PX_PA_UNIT_TXQ_1_E       , prvCpssPxPortPABuildPizzaDistribution                   }
    ,{ CPSS_PX_PA_UNIT_TXDMA_0_E     , prvCpssPxPortPABuildPizzaDistributionWithDummySlice     }
    ,{ CPSS_PX_PA_UNIT_TX_FIFO_0_E   , prvCpssPxPortPABuildPizzaDistributionWithDummySlice     }
    ,{ CPSS_PX_PA_UNIT_UNDEFINED_E   , (PRV_CPSS_PX_BUILD_PIZZA_DISTRBUTION_FUN)NULL}
};

static GT_STATUS  prvCpssPxPipeMapping2UnitConvFun
(
    IN GT_SW_DEV_NUM                      devNum,
    IN CPSS_PX_DETAILED_PORT_MAP_STC     *portMapShadowPtr,
    OUT GT_U32                            portArgArr[CPSS_PX_PA_UNIT_MAX_E]
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_U32      txqDqIdx;
    GT_U32      txq;


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    for (i = 0; i < CPSS_PX_PA_UNIT_MAX_E; i++)
    {
        portArgArr[i] = CPSS_PX_PORT_MAPPING_INVALID_PORT_CNS;
    }

    /*-------------------------------------------------------------------*
     *  convert global rxDma,TxDma,TxFifo --> to [dp, local instance]    *
     *-------------------------------------------------------------------*/
    rc = prvCpssPxPpResourcesTxqGlobal2LocalConvert(devNum,portMapShadowPtr->portMap.txqNum, /*OUT*/&txqDqIdx, &txq);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "TXQ port %d above max (20)", portMapShadowPtr->portMap.txqNum );
    }
    portArgArr[CPSS_PX_PA_UNIT_RXDMA_0_E           ] = portMapShadowPtr->portMap.dmaNum;
    portArgArr[CPSS_PX_PA_UNIT_TXQ_0_E +  txqDqIdx ] = txq;
    portArgArr[CPSS_PX_PA_UNIT_TXDMA_0_E           ] = portMapShadowPtr->portMap.dmaNum;
    portArgArr[CPSS_PX_PA_UNIT_TX_FIFO_0_E         ] = portMapShadowPtr->portMap.dmaNum;
    return GT_OK;
}

static CPSS_PX_PA_UNIT_ENT   prv_pipe_workConservingModeOnUnitList[] =
{
    CPSS_PX_PA_UNIT_TXQ_0_E
   ,CPSS_PX_PA_UNIT_TXQ_1_E
   ,CPSS_PX_PA_UNIT_UNDEFINED_E           /* last unit */
};


/*----------------------------------------------------------
 *  WS related function
 *----------------------------------------------------------*/
/**
* @internal prvCpssPxPortPaWorSpaceInitStatusGet function
* @endinternal
*
* @brief   get WS init status
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_TRUE                  - on success
* @retval GT_FALSE                 - not init
*/
static GT_BOOL prvCpssPxPortPaWorSpaceInitStatusGet
(
    INOUT PRV_CPSS_PX_PA_WS_SET_STC     *paWsPtr
)
{
    return paWsPtr->isInit;
}
/**
* @internal prvCpssPxPortPaWorSpaceInitStatusSet function
* @endinternal
*
* @brief   set WS init status
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*/
static GT_VOID prvCpssPxPortPaWorSpaceInitStatusSet
(
    INOUT PRV_CPSS_PX_PA_WS_SET_STC     *paWsPtr
)
{
    paWsPtr->isInit = GT_TRUE;
}

/**
* @internal prvCpssPxPortDynamicPizzaArbiterWSByParamsInit function
* @endinternal
*
* @brief   Init device WS by params
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] paWs                     - device work space
* @param[in] wsInitParamsPtr          - params
* @param[in,out] paWs                     - device work space
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - NULL ptr
* @retval GT_FAIL                  - on error
*/
GT_VOID prvCpssPxPortDynamicPizzaArbiterWSByParamsInit
(
    INOUT PRV_CPSS_PX_PA_WORKSPACE_STC * paWs,
    IN    PRV_CPSS_PX_PA_WS_INIT_PARAMS_STC * wsInitParamsPtr
)
{
    GT_U32      unitIdx;
    CPSS_PX_PA_UNIT_ENT unitType;
    GT_U32      i;

    /*------------------------------------*
     * rex x dev_bw copy
     *------------------------------------*/
    paWs->devBwPtr = wsInitParamsPtr->devBwPtr;
    cpssOsMemSet(paWs->prv_speedEnt2MBitConvArr,  0,sizeof(paWs->prv_speedEnt2MBitConvArr));
    for (i = 0 ; wsInitParamsPtr->speedEnm2GbpsPtr[i].speedEnm != CPSS_PORT_SPEED_NA_E; i++)
    {
        CPSS_PORT_SPEED_ENT speedEnm;
        GT_U32              speedGbps;
        /* get enumerator/ speed */
        speedEnm  = wsInitParamsPtr->speedEnm2GbpsPtr[i].speedEnm;
        speedGbps = wsInitParamsPtr->speedEnm2GbpsPtr[i].speedGbps;
        /* write to proper place */
        paWs->prv_speedEnt2MBitConvArr[speedEnm] = PRV_CPSS_PX_PA_BW_COEFF*speedGbps;
    }

    paWs->prv_DeviceUnitListPtr = wsInitParamsPtr->supportedUnitListPtr;
    paWs->prv_unitListConfigByPipeBwSetPtr = wsInitParamsPtr->unitListConfByPipeBwSetListPtr;

    /*-----------------------------*/
    /* init map of supported units */
    /*-----------------------------*/
    for (unitIdx = 0 ; unitIdx < CPSS_PX_PA_UNIT_MAX_E; unitIdx++)
    {
        paWs->prv_DeviceUnitListBmp[unitIdx]             = GT_FALSE;
        paWs->prv_unitListConfigByPipeBwSetBmp[unitIdx]  = GT_FALSE;
    }
    for (unitIdx = 0 ; paWs->prv_DeviceUnitListPtr[unitIdx] != CPSS_PX_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        unitType = paWs->prv_DeviceUnitListPtr[unitIdx];
        paWs->prv_DeviceUnitListBmp[unitType] = GT_TRUE;
    }

    for (unitIdx = 0 ; wsInitParamsPtr->unitListConfByPipeBwSetListPtr[unitIdx] != CPSS_PX_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        unitType = wsInitParamsPtr->unitListConfByPipeBwSetListPtr[unitIdx];
        paWs->prv_unitListConfigByPipeBwSetBmp[unitType] = GT_TRUE;
    }

    paWs->mapping2unitConvFunPtr = wsInitParamsPtr->mapping2UnitConvFun;

    /*-------------------------------------------------------------*/
    /* define list of configuration units for each mapping type    */
    /*-------------------------------------------------------------*/
    cpssOsMemSet(&paWs->prv_mappingType2UnitConfArr,0,sizeof(paWs->prv_mappingType2UnitConfArr));
    for (i = 0 ; wsInitParamsPtr->mapping2UnitListPtr[i].mappingType != CPSS_PX_PORT_MAPPING_TYPE_INVALID_E ; i++)
    {
        CPSS_PX_PORT_MAPPING_TYPE_ENT mappingType      = wsInitParamsPtr->mapping2UnitListPtr[i].mappingType;
        paWs->prv_mappingType2UnitConfArr[mappingType] =  wsInitParamsPtr->mapping2UnitListPtr[i].unitList2ConfigurePtr;
    }

    cpssOsMemSet(&paWs->prv_unit2PizzaAlgoFunArr,0,sizeof(paWs->prv_unit2PizzaAlgoFunArr));
    for ( i = 0 ; wsInitParamsPtr->unit_2_pizzaCompFun[i].unit !=  CPSS_PX_PA_UNIT_UNDEFINED_E ; i++)
    {
        PRV_CPSS_PX_BUILD_PIZZA_DISTRBUTION_FUN fun;
        unitType = wsInitParamsPtr->unit_2_pizzaCompFun[i].unit;
        fun      = wsInitParamsPtr->unit_2_pizzaCompFun[i].fun;
        paWs->prv_unit2PizzaAlgoFunArr[unitType] = fun;
    }
    /*--------------------------------------------*
     * unit list where work conserving mode on    *
     *--------------------------------------------*/
    paWs->workConservingModeOnUnitListPtr = wsInitParamsPtr->workConservingModeOnUnitListPtr;
    /* */
    paWs->cpuSpeedEnm = wsInitParamsPtr->cpuSpeedEnm;
}

static PRV_CPSS_PX_PA_WS_INIT_PARAMS_STC        prv_pipe_ws_init_params =
{
    /* PRV_CPSS_PX_REV_x_DEVICE_BW_STC         *devBwPtr                 */ (PRV_CPSS_PX_REV_x_DEVICE_BW_STC*)      &prv_pipe_rev_x_devBW[0]
    /* CPSS_PX_PA_UNIT_ENT                     *supportedUnitListPtr     */,(CPSS_PX_PA_UNIT_ENT*)                  &prv_pipe_UnitList[0]
    /* CPSS_PX_PA_UNIT_ENT            *unitListConfByPipeBwSetListPtr    */,(CPSS_PX_PA_UNIT_ENT*)                  &prv_pipe_unitListConfByPipeBwSetList[0]
    /* PRV_CPSS_PX_PA_SPEEDENM_SPEEDGBPS_STC   *speedEnm2GbpsPtr;        */,(PRV_CPSS_PX_PA_SPEEDENM_SPEEDGBPS_STC*)&prv_pipe_speedEnm2Gbps[0]
    /* prvCpssPxMapping2UnitConvFUN          mapping2UnitConvFun;        */,                                        prvCpssPxPipeMapping2UnitConvFun
    /* PRV_CPSS_PX_MAPPING_2_UNITLIST_STC   *mappingTm2UnitListPtr       */,(PRV_CPSS_PX_MAPPING_2_UNITLIST_STC*)   &prv_pipe_mapping_2_unitlist[0]
    /* PRV_CPSS_PX_UNIT_2_PIZZA_FUN_STC        *unit_2_pizzaCompFun;     */,(PRV_CPSS_PX_UNIT_2_PIZZA_FUN_STC *)    &prv_pipe_unit_2_pizzaFun[0]
    /* CPSS_PX_PA_UNIT_ENT            *workConservingModeOnUnitListPtr;  */,(CPSS_PX_PA_UNIT_ENT *)                 &prv_pipe_workConservingModeOnUnitList[0]
    /* CPSS_PORT_SPEED_ENT                               cpuSpeedEnm;    */,                                        CPSS_PORT_SPEED_1000_E
};


static PRV_CPSS_PX_PA_WS_SET_STC                prv_paWsSet;

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
    IN  GT_SW_DEV_NUM                  devNum,
    OUT PRV_CPSS_PX_PA_WORKSPACE_STC **workSpacePtrPtr
)
{
    PRV_CPSS_PX_PA_WORKSPACE_STC *paWsPtr;


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);


    CPSS_NULL_PTR_CHECK_MAC(workSpacePtrPtr);

    paWsPtr = PRV_CPSS_PX_PP_MAC(devNum)->paData.paWsPtr;

    *workSpacePtrPtr = paWsPtr;
    return GT_OK;
}

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
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - NULL ptr
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterWSSuportedUnitListGet
(
    IN  GT_SW_DEV_NUM           devNum,
    OUT CPSS_PX_PA_UNIT_ENT **supportedUnitListPtrPtr
)
{
    PRV_CPSS_PX_PA_WORKSPACE_STC *paWsPtr;


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(supportedUnitListPtrPtr);

    paWsPtr = PRV_CPSS_PX_PP_MAC(devNum)->paData.paWsPtr;

    *supportedUnitListPtrPtr = paWsPtr->prv_DeviceUnitListPtr;
    return GT_OK;
}

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
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - NULL ptr
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterWSdevBwGet
(
    IN  GT_SW_DEV_NUM           devNum,
    OUT PRV_CPSS_PX_CC_x_DEVICE_BW_STC **devBwPtrPtr
)
{
    GT_U32 revision;
    GT_U32 i;
    GT_U32 coreClockMHz;
    PRV_CPSS_PX_REV_x_DEVICE_BW_STC * rev_x_devBWPtr;
    GT_U32  lastFoundRevision;
    GT_BOOL isRevFound;
    PRV_CPSS_PX_CC_x_DEVICE_BW_STC *devBWPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(devBwPtrPtr);

    *devBwPtrPtr = (PRV_CPSS_PX_CC_x_DEVICE_BW_STC*)NULL;

    rev_x_devBWPtr = PRV_CPSS_PX_PP_MAC(devNum)->paData.paWsPtr->devBwPtr;

    revision     = PRV_CPSS_PX_PP_MAC(devNum)->genInfo.revision;
    coreClockMHz = PRV_CPSS_PX_PP_MAC(devNum)->genInfo.coreClock;
    /*-------------------------------------------------------------------
     * find gretest revision smaller or equal to ASIC revision
     *-------------------------------------------------------------------*/
    isRevFound = GT_FALSE;
    lastFoundRevision = (GT_U32)-1;
    devBWPtr = (PRV_CPSS_PX_CC_x_DEVICE_BW_STC *)NULL;
    for (i = 0 ;  rev_x_devBWPtr[i].devBWPtr != NULL; i++)
    {
        if (revision >= rev_x_devBWPtr[i].revision)
        {
            if (isRevFound == GT_FALSE)
            {
                isRevFound = GT_TRUE;
                lastFoundRevision = rev_x_devBWPtr[i].revision;
                devBWPtr = rev_x_devBWPtr[i].devBWPtr;
            }
            else /* (isRevFound == GT_TRUE) */
            {
                if (rev_x_devBWPtr[i].revision > lastFoundRevision)
                {
                    lastFoundRevision = rev_x_devBWPtr[i].revision;
                    devBWPtr = rev_x_devBWPtr[i].devBWPtr;
                }
            }
        }
    }
    if (devBWPtr == NULL)
    {
        return GT_OK;
    }
    /*------------------------------------------------
     *  find core clock
     *------------------------------------------------*/
    for (i = 0 ; devBWPtr[i].coreClock != 0 ; i++)
    {
        if (devBWPtr[i].coreClock == coreClockMHz)
        {
            *devBwPtrPtr = &devBWPtr[i]; /* coreclock found */
            break;
        }
    }
    return GT_OK;
}

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
    IN  GT_SW_DEV_NUM           devNum,
    OUT CPSS_PORT_SPEED_ENT     *cpuSpeedEnmPtr
)
{
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(cpuSpeedEnmPtr);

    *cpuSpeedEnmPtr = PRV_CPSS_PX_PP_MAC(devNum)->paData.paWsPtr->cpuSpeedEnm;

    return GT_OK;
}



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
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterWSInit
(
    IN  GT_SW_DEV_NUM           devNum
)
{

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (GT_FALSE == prvCpssPxPortPaWorSpaceInitStatusGet(&prv_paWsSet))
    {
        prvCpssPxPortDynamicPizzaArbiterWSByParamsInit(&prv_paWsSet.paWorkSpace_pipe,      /*IN*/&prv_pipe_ws_init_params);
        prvCpssPxPortPaWorSpaceInitStatusSet(&prv_paWsSet);
    }

    PRV_CPSS_PX_PP_MAC(devNum)->paData.paWsPtr = (PRV_CPSS_PX_PA_WORKSPACE_STC*)NULL; /* PA data still are not selected */

    if (PRV_CPSS_PX_PP_MAC(devNum)->genInfo.devFamily == CPSS_PX_FAMILY_PIPE_E)
    {
        PRV_CPSS_PX_PP_MAC(devNum)->paData.paWsPtr = &prv_paWsSet.paWorkSpace_pipe;
    }

    /* selection was done ? */
    if (PRV_CPSS_PX_PP_MAC(devNum)->paData.paWsPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Pizza Arbiter WS selection");
    }

    return GT_OK;
}


