/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxPortDynamicPizzaArbiterDrv.c
*
* DESCRIPTION:
*       pipe dynamic (algorithmic) pizza arbiter unit driver
*
* FILE REVISION NUMBER:
*       $Revision: 1 $ 
*******************************************************************************/
#include <cpss/px/config/cpssPxCfgInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxRegsVer1.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAUnitDrv.h>
#include <cpss/px/port/cpssPxPortMapping.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>

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
| RXDMA-0      | 0x10002A00 |  0-8 | 14-14 | 15-15 | 0x10002A08 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | RXDMA/Units/<RXDMA_IP> RxDMA %a/<Pizza Arbiter> SIP_COMMON_MODULES_IP Units/Pizza Arbiter
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+-----------------------------------------------------------------------------
| TXDMA-0      | 0x02004000 |  0-8 | 14-14 | 15-15 | 0x02004008 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | TXDMA/Units/<TXDMA_IP> TxDMA %a/<Pizza Arb> TxDMA Pizza Arb/Pizza Arbiter
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+-----------------------------------------------------------------------------
| TXFIFO-0     | 0x04000800 |  0-8 | 14-14 | 15-15 | 0x04000808 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per DMA BW on DP, with dummy slices      | TX_FIFO/Units/<TXFIFO_IP> TxFIFO %a/<Pizza Arb> TxFIFO Pizza Arb/Pizza Arbiter/
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+----------------------------------------------------
| TXQ-0        | 0x21001500 |  0-8 | 12-12 |  9-9  | 0x21001650 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per unit TXQ BW, no dummy slices (*)     | TXQ_IP/<TXQ_IP> TXQ_DQ/Units/TXQ_IP_dq/Scheduler/Port Arbiter Configuration/
| TXQ-1        | 0x22001500 |  0-8 | 12-12 |  9-9  | 0x22001650 |   85   |   4  |  340  | 0-6  | 7-7  |Local  DMA              |  per unit TXQ BW, no dummy slices         | 
+--------------+------------+------+-------+-------+------------+--------+------+-------+------+------+------------------------+-------------------------------------------+----------------------------------------------------
* (cycle is defined as sum of all ports passing via that DQ)

#endif



/*-------------------------------------------------------------------------
 *   Level UnitId :
 *        prvCpssPxPizzaArbiterUnitDrvRegAddrSet
 *        prvCpssPxPizzaArbiterUnitDrvRegAddrGet
 *        prvCpssPxPizzaArbiterUnitDrvWorkConservingModeGet
 *        prvCpssPxPizzaArbiterUnitDrvWorkConservingModeSet
 *        prvCpssPxPizzaArbiterUnitDrvInit
 *
 *   Level UnitDescription :
 *        prvCpssPxPizzaArbiterUnitDescrDrvDescrByUnitIdGet(IN unitId, OUT unitDescrPtr);
 *        internal_prvCpssPxPizzaArbiterUnitDescrDrvInit
 *        prvCpssPxPizzaArbiterUnitDescrDrvGet
 *        prvCpssPxPizzaArbiterUnitDescrDrvSet
 *        internal_prvCpssPxPizzaArbiterUnitDescrDrvWorkConservingModeGet
 *        internal_prvCpssPxPizzaArbiterUnitDescrDrvWorkConservingModeSet
 *
 *   Data structures
 *        Unit Description List
 *-------------------------------------------------------------------------
 */
#define FLD_OFF(STR,fld)      (GT_U32)offsetof(STR,fld)
#define FLD_OFFi(STR,idx,fld) idx*sizeof(STR) + offsetof(STR,fld)


/**
* @struct PRV_CPSS_PX_PA_UNIT_CTRL_REG_FLD_DESCR_STC
 *
 * @brief This struct defines the structure of PA control register : user description
*/
typedef struct{

    GT_U32 ctrlRegSliceNum2Run_Offs;

    GT_U32 ctrlRegSliceNum2Run_Len;

    GT_U32 ctrlRegMapLoadEn_Offs;

    GT_U32 ctrlRegMapLoadEn_Len;

    /** offset of the field WorkConservEn */
    GT_U32 ctrlRegWorkConservEn_Offs;

    /** @brief length of the field WorkConservEn
     *  Comments:
     */
    GT_U32 ctrlRegWorkConservEn_Len;

} PRV_CPSS_PX_PA_UNIT_CTRL_REG_FLD_DESCR_STC;


/**
* @struct PRV_CPSS_PX_PA_UNIT_MAP_REG_FLD_DESCR_STC
 *
 * @brief This struct defines the structure of PA map register : user description
*/
typedef struct{

    GT_U32 paMapRegNum;

    GT_U32 paMapFldPerReg;

    GT_U32 paMapTotalFlds;

    /** offset of the field SliceMap */
    GT_U32 paMapRegWorkSliceMapOffs;

    /** length of the field SliceMap */
    GT_U32 paMapRegWorkSliceMapLen;

    /** offset of the field SliceEn */
    GT_U32 paMapRegWorkSliceEnOffs;

    /** @brief length of the field SliceEn
     *  Comments:
     */
    GT_U32 paMapRegWorkSliceEnLen;

} PRV_CPSS_PX_PA_UNIT_MAP_REG_FLD_DESCR_STC;

/**
* @struct PRV_CPSS_PX_PA_UNIT_DEF_STC
 *
 * @brief This struct defines the structure of PA UNIT : user description
*/
typedef struct{

    CPSS_PX_PA_UNIT_ENT unitType;

    GT_U32 ctrlRegBaseAddrOffs;

    PRV_CPSS_PX_PA_UNIT_CTRL_REG_FLD_DESCR_STC *ctrlRegFldDefPtr;

    GT_U32 mapRegBaseAddrOffs;

    PRV_CPSS_PX_PA_UNIT_MAP_REG_FLD_DESCR_STC *mapRegFldDefPtr;

} PRV_CPSS_PX_PA_UNIT_DEF_STC;


static PRV_CPSS_PX_PA_UNIT_CTRL_REG_FLD_DESCR_STC prv_rxDma_txDma_txFifo_CTRL_REG =
{
    /* ctrlRegSliceNum2Run_Offs  */ PRV_CPSS_PX_PA_CTRLREG_SLICE_NUM_2_RUN_OFFS_CNS
    /* ctrlRegSliceNum2Run_Len   */,PRV_CPSS_PX_PA_CTRLREG_SLICE_NUM_2_RUN_LEN_CNS
    /* ctrlRegMapLoadEn_Offs     */,PRV_CPSS_PX_PA_CTRLREG_MAP_LOAD_EN_OFFS_CNS
    /* ctrlRegMapLoadEn_Len      */,PRV_CPSS_PX_PA_CTRLREG_MAP_LOAD_EN_LEN_CNS
    /* ctrlRegWorkConservEn_Offs */,PRV_CPSS_PX_PA_CTRLREG_WORK_CONSERV_EN_OFFS_CNS
    /* ctrlRegWorkConservEn_Len  */,PRV_CPSS_PX_PA_CTRLREG_WORK_CONSERV_EN_LEN_CNS
};

static PRV_CPSS_PX_PA_UNIT_CTRL_REG_FLD_DESCR_STC prv_txQ_CTRL_REG =
{
    /* ctrlRegSliceNum2Run_Offs  */ PRV_CPSS_PX_PA_CTRLREG_TXQ_SLICE_NUM_2_RUN_OFFS_CNS
    /* ctrlRegSliceNum2Run_Len   */,PRV_CPSS_PX_PA_CTRLREG_TXQ_SLICE_NUM_2_RUN_LEN_CNS
    /* ctrlRegMapLoadEn_Offs     */,PRV_CPSS_PX_PA_CTRLREG_TXQ_MAP_LOAD_EN_OFFS_CNS
    /* ctrlRegMapLoadEn_Len      */,PRV_CPSS_PX_PA_CTRLREG_TXQ_MAP_LOAD_EN_LEN_CNS
    /* ctrlRegWorkConservEn_Offs */,PRV_CPSS_PX_PA_CTRLREG_TXQ_WORK_CONSERV_EN_OFFS_CNS
    /* ctrlRegWorkConservEn_Len  */,PRV_CPSS_PX_PA_CTRLREG_TXQ_WORK_CONSERV_EN_LEN_CNS
};


static PRV_CPSS_PX_PA_UNIT_MAP_REG_FLD_DESCR_STC prv_rxDma_TxDma_TxQ_TxFifo_MAP_REG =
{
    /* paMapRegNum              */  PRV_CPSS_PX_PA_MAP_REGNUM_CNS
    /* paMapFldPerReg           */ ,PRV_CPSS_PX_PA_MAP_FLD_PER_REG_CNS
    /* paMapTotalFlds           */ ,PRV_CPSS_PX_PA_MAP_TOTAL_FLDS_REG_CNS
    /* paMapRegWorkSliceMapOffs */ ,PRV_CPSS_PX_PA_MAPREG_WORK_SLICEMAP_OFFS_CNS
    /* paMapRegWorkSliceMapLen  */ ,PRV_CPSS_PX_PA_MAPREG_WORK_SLICEMAP_LEN_CNS
    /* paMapRegWorkSliceEnOffs  */ ,PRV_CPSS_PX_PA_MAPREG_WORK_SLICE_EN_OFFS_CNS
    /* paMapRegWorkSliceEnLen   */ ,PRV_CPSS_PX_PA_MAPREG_WORK_SLICE_EN_LEN_CNS
};



/*---------------------------------------------------------*
 * unit definition
 *---------------------------------------------------------*/

/*---------------------------------------------------------*
 * RXDMA unit definition
 *---------------------------------------------------------*/
static PRV_CPSS_PX_PA_UNIT_DEF_STC prv_pipe_rxDma_0_Def =
{
   /* CPSS_PX_PA_UNIT_ENT                           unitType;           */  CPSS_PX_PA_UNIT_RXDMA_0_E
   /* GT_U32                                        ctrlRegBaseAddrOffs */ ,FLD_OFF(PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC, rxDMA.SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterCtrlReg)
   /* PRV_CPSS_PX_PA_UNIT_CTRL_REG_FLD_DESCR_STC   *ctrlRegFldDefPtr    */ ,&prv_rxDma_txDma_txFifo_CTRL_REG
   /* GT_U32                                        mapRegBaseAddrOffs  */ ,FLD_OFF(PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC, rxDMA.SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterConfigReg[0])
   /* PRV_CPSS_PX_PA_UNIT_MAP_REG_FLD_DESCR_STC    *mapRegFldDefPtr     */ ,&prv_rxDma_TxDma_TxQ_TxFifo_MAP_REG
};



/*---------------------------------------------------------*
 * TXQ unit definition
 *---------------------------------------------------------*/
static PRV_CPSS_PX_PA_UNIT_DEF_STC prv_pipe_txQDef_0 =
{
    /* CPSS_PX_PA_UNIT_ENT                           unitType;           */ CPSS_PX_PA_UNIT_TXQ_0_E
    /* GT_U32                                        ctrlRegBaseAddrOffs */,FLD_OFF(PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC, TXQ.dq[0].scheduler.portArbiterConfig.portsArbiterConfig)
    /* PRV_CPSS_PX_PA_UNIT_CTRL_REG_FLD_DESCR_STC   *ctrlRegFldDefPtr    */,&prv_txQ_CTRL_REG
    /* GT_U32                                        mapRegBaseAddrOffs  */,FLD_OFF(PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC, TXQ.dq[0].scheduler.portArbiterConfig.portsArbiterMap[0])
    /* PRV_CPSS_PX_PA_UNIT_MAP_REG_FLD_DESCR_STC    *mapRegFldDefPtr     */,&prv_rxDma_TxDma_TxQ_TxFifo_MAP_REG
};


static PRV_CPSS_PX_PA_UNIT_DEF_STC prv_pipe_txQDef_1 =
{
    /* CPSS_PX_PA_UNIT_ENT                           unitType;           */ CPSS_PX_PA_UNIT_TXQ_1_E
    /* GT_U32                                        ctrlRegBaseAddrOffs */,FLD_OFF(PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC, TXQ.dq[1].scheduler.portArbiterConfig.portsArbiterConfig)
    /* PRV_CPSS_PX_PA_UNIT_CTRL_REG_FLD_DESCR_STC   *ctrlRegFldDefPtr    */,&prv_txQ_CTRL_REG
    /* GT_U32                                        mapRegBaseAddrOffs  */,FLD_OFF(PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC, TXQ.dq[1].scheduler.portArbiterConfig.portsArbiterMap[0])
    /* PRV_CPSS_PX_PA_UNIT_MAP_REG_FLD_DESCR_STC    *mapRegFldDefPtr     */,&prv_rxDma_TxDma_TxQ_TxFifo_MAP_REG
};

/*---------------------------------------------------------*
 * TXDMA unit definition
 *---------------------------------------------------------*/
static PRV_CPSS_PX_PA_UNIT_DEF_STC prv_pipe_txDma_0_Def =
{
    /* CPSS_PX_PA_UNIT_ENT                           unitType;           */ CPSS_PX_PA_UNIT_TXDMA_0_E
    /* GT_U32                                        ctrlRegBaseAddrOffs */,FLD_OFF(PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC, txDMA.txDMAPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg)
    /* PRV_CPSS_PX_PA_UNIT_CTRL_REG_FLD_DESCR_STC   *ctrlRegFldDefPtr    */,&prv_rxDma_txDma_txFifo_CTRL_REG
    /* GT_U32                                        mapRegBaseAddrOffs  */,FLD_OFF(PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC, txDMA.txDMAPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0])
    /* PRV_CPSS_PX_PA_UNIT_MAP_REG_FLD_DESCR_STC    *mapRegFldDefPtr     */,&prv_rxDma_TxDma_TxQ_TxFifo_MAP_REG
};


/*---------------------------------------------------------*
 * TXFIFO unit definition
 *---------------------------------------------------------*/
static PRV_CPSS_PX_PA_UNIT_DEF_STC prv_pipe_txFifo_0_Def =
{
    /* CPSS_PX_PA_UNIT_ENT                           unitType;           */ CPSS_PX_PA_UNIT_TX_FIFO_0_E
    /* GT_U32                                        ctrlRegBaseAddrOffs */,FLD_OFF(PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC, txFIFO.txFIFOPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg)
    /* PRV_CPSS_PX_PA_UNIT_CTRL_REG_FLD_DESCR_STC   *ctrlRegFldDefPtr    */,&prv_rxDma_txDma_txFifo_CTRL_REG
    /* GT_U32                                        mapRegBaseAddrOffs  */,FLD_OFF(PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC, txFIFO.txFIFOPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[0])
    /* PRV_CPSS_PX_PA_UNIT_MAP_REG_FLD_DESCR_STC    *mapRegFldDefPtr     */,&prv_rxDma_TxDma_TxQ_TxFifo_MAP_REG
};



/*---------------------------------------*
 * data structures                       *
 *---------------------------------------*/
/**
* @struct PRV_CPSS_PX_DEV_X_UNITDEFLIST
 *
 * @brief This struct defines the structure of all PA units on device : user description
*/
typedef struct{

    /** @brief device family
     *  unitDefListPtr  - unit definitoon (user perspective)
     *  unitsDescrAllPtr - operational data (internal use, initialized from user data)
     *  Comments:
     */
    CPSS_PP_FAMILY_TYPE_ENT family;

    PRV_CPSS_PX_PA_UNIT_DEF_STC **unitDefListPtr;

    PRV_CPSS_PX_PA_UNIT_STC *unitsDescrAllPtr;

} PRV_CPSS_PX_DEV_X_UNITDEFLIST;

/*
 * typedef: struct PRV_CPSS_PX_PA_UNITS_DRV_STC
 *
 * Description:
 *      This struct defines the structure of Unit drivers and whether it is initilized
 *
 * Fields:
 *     isInitilaized     - whether initilizad, (GT_TRUE)
 *     dev_x_unitDefList - pointer to list of unit init structure
 * Comments:
 *
 */
typedef struct PRV_CPSS_PX_PA_UNITS_DRV_STCT
{
    GT_BOOL                          isInitilaized;
    PRV_CPSS_PX_DEV_X_UNITDEFLIST *dev_x_unitDefList;
}PRV_CPSS_PX_PA_UNITS_DRV_STC;



/**
* @internal prvCpssPxPizzaArbiterUnitDrvUnitDescrInit function
* @endinternal
*
* @brief   init PA unit (HW) definiton by declaration
*
* @note   APPLICABLE DEVICES:      None.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] prv_paUnitDef_All        - list of unit definiton (user view)
* @param[in] size                     -  of list
*
* @param[out] unitsDescrArr            - (pointer to )unit description
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
static GT_STATUS prvCpssPxPizzaArbiterUnitDrvUnitDescrInit
(
    IN  PRV_CPSS_PX_PA_UNIT_DEF_STC      **prv_paUnitDef_All,
    IN  GT_U32                             size,
    OUT PRV_CPSS_PX_PA_UNIT_STC           *unitsDescrArr
)
{
    GT_U32 i;
    PRV_CPSS_PX_PA_UNIT_DEF_STC  *pUnitDefPtr;
    PRV_CPSS_PX_PA_UNIT_STC  *unitsDescrPtr;

    /* init all possible definitions as invalide */
    for (i = 0 ; i < size; i++)
    {
        cpssOsMemSet(&unitsDescrArr[i],0xFF,sizeof(unitsDescrArr[i]));
        unitsDescrArr[i].unit = CPSS_PX_PA_UNIT_UNDEFINED_E;
    }

    /* init just specified units */
    for (i = 0 ;  prv_paUnitDef_All[i] != NULL ; i++)
    {
        pUnitDefPtr = prv_paUnitDef_All[i];
        unitsDescrPtr = &unitsDescrArr[pUnitDefPtr->unitType];

        if (unitsDescrPtr->unit == pUnitDefPtr->unitType)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* module already initialized !!! something bad  */
        }
        unitsDescrPtr->unit                                = pUnitDefPtr->unitType;
        unitsDescrPtr->ctrlReg.baseAddressPtrOffs          = pUnitDefPtr->ctrlRegBaseAddrOffs;
        unitsDescrPtr->ctrlReg.fldSliceNum2run.offset      = pUnitDefPtr->ctrlRegFldDefPtr->ctrlRegSliceNum2Run_Offs;
        unitsDescrPtr->ctrlReg.fldSliceNum2run.len         = pUnitDefPtr->ctrlRegFldDefPtr->ctrlRegSliceNum2Run_Len;
        unitsDescrPtr->ctrlReg.fldMapLoadEn.offset         = pUnitDefPtr->ctrlRegFldDefPtr->ctrlRegMapLoadEn_Offs;
        unitsDescrPtr->ctrlReg.fldMapLoadEn.len            = pUnitDefPtr->ctrlRegFldDefPtr->ctrlRegMapLoadEn_Len;
        unitsDescrPtr->ctrlReg.fldWorkConservModeEn.offset = pUnitDefPtr->ctrlRegFldDefPtr->ctrlRegWorkConservEn_Offs;
        unitsDescrPtr->ctrlReg.fldWorkConservModeEn.len    = pUnitDefPtr->ctrlRegFldDefPtr->ctrlRegWorkConservEn_Len;

        unitsDescrPtr->mapReg.baseAddressPtrOffs           = pUnitDefPtr->mapRegBaseAddrOffs;
        unitsDescrPtr->mapReg.totalReg                     = pUnitDefPtr->mapRegFldDefPtr->paMapRegNum;
        unitsDescrPtr->mapReg.numberFldsPerReg             = pUnitDefPtr->mapRegFldDefPtr->paMapFldPerReg;
        unitsDescrPtr->mapReg.totalFlds                    = pUnitDefPtr->mapRegFldDefPtr->paMapTotalFlds;
        unitsDescrPtr->mapReg.fldSlice2PortMap.offset      = pUnitDefPtr->mapRegFldDefPtr->paMapRegWorkSliceMapOffs;
        unitsDescrPtr->mapReg.fldSlice2PortMap.len         = pUnitDefPtr->mapRegFldDefPtr->paMapRegWorkSliceMapLen;
        unitsDescrPtr->mapReg.fldSliceEn.offset            = pUnitDefPtr->mapRegFldDefPtr->paMapRegWorkSliceEnOffs;
        unitsDescrPtr->mapReg.fldSliceEn.len               = pUnitDefPtr->mapRegFldDefPtr->paMapRegWorkSliceEnLen;
    }
    return GT_OK;
}



/*------------------------------------------------------------*/
/* Unit Desctition Level                                      */
/*   - Init                                                   */
/*   - Set (All)                                              */
/*   - Get (All)                                              */
/*------------------------------------------------------------*/


/**
* @internal prvCpssPxPizzaArbiterUnitDrvRegAddrGet function
* @endinternal
*
* @brief   get address of register from register address data base
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - dev number
* @param[in] offsInStr                - size of list
*
* @param[out] regAddrPtrPtr            - (pointer to )register address' address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
static GT_STATUS prvCpssPxPizzaArbiterUnitDrvRegAddrGet
(
    IN GT_SW_DEV_NUM devNum,
    IN GT_U32        offsInStr,
    OUT GT_U32     **regAddrPtrPtr
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC  *regsAddrVer1;
    GT_U8                                *regsAddrVer1_U8;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(regAddrPtrPtr);

    regsAddrVer1 = &PRV_CPSS_PX_PP_MAC(devNum)->regsAddrVer1;
    regsAddrVer1_U8 = (GT_U8 *)regsAddrVer1;
    *regAddrPtrPtr = (GT_U32*)&regsAddrVer1_U8[offsInStr];
    return GT_OK;
}

/**
* @internal prvCpssPxPizzaArbiterUnitDrvCtrlRegAddr function
* @endinternal
*
* @brief   get address of register from register address data base
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - dev number
* @param[in] unitDescrPtr             - unit description
*
* @param[out] regAddrPtr               - (pointer to )register address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
static GT_STATUS prvCpssPxPizzaArbiterUnitDrvCtrlRegAddr
(
    IN GT_SW_DEV_NUM            devNum,
    IN PRV_CPSS_PX_PA_UNIT_STC *unitDescrPtr,
    OUT GT_U32                 *regAddrPtr
)
{
    GT_STATUS rc;
    GT_U32    *regBaseAddrPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(unitDescrPtr);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);


    rc = prvCpssPxPizzaArbiterUnitDrvRegAddrGet(devNum,unitDescrPtr->ctrlReg.baseAddressPtrOffs,/*OUT*/&regBaseAddrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    *regAddrPtr = *regBaseAddrPtr;
    if (*regAddrPtr == 0xfffffffb)  /* bad address stored at DB */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssPxPizzaArbiterUnitDrvMapRegAddrFldOffsLenGet function
* @endinternal
*
* @brief   get field reg address, offset and len for PA map register
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - dev number
* @param[in] unitDescrPtr             - unit description
* @param[in] sliceId                  - slice number
*
* @param[out] regAddrPtr               - (pointer to )register address
* @param[out] fldLenPtr                - (pointer to )fld Length
* @param[out] fldOffsPtr               - (pointer to )fld offset
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
static GT_STATUS prvCpssPxPizzaArbiterUnitDrvMapRegAddrFldOffsLenGet
(
    IN GT_SW_DEV_NUM                  devNum,
    IN PRV_CPSS_PX_PA_UNIT_STC       *unitDescrPtr,
    IN GT_U32                         sliceId,
    OUT GT_U32                       *regAddrPtr,
    OUT GT_U32                       *fldLenPtr,
    OUT GT_U32                       *fldOffsPtr
)
{
    GT_STATUS rc;
    GT_U32    *regBaseAddrPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(unitDescrPtr);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);
    CPSS_NULL_PTR_CHECK_MAC(fldLenPtr );
    CPSS_NULL_PTR_CHECK_MAC(fldOffsPtr);


    rc = prvCpssPxPizzaArbiterUnitDrvRegAddrGet(devNum,unitDescrPtr->mapReg.baseAddressPtrOffs,/*OUT*/&regBaseAddrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (sliceId >= unitDescrPtr->mapReg.totalFlds)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *regAddrPtr = regBaseAddrPtr[sliceId / unitDescrPtr->mapReg.numberFldsPerReg];
    *fldLenPtr  = unitDescrPtr->mapReg.fldSlice2PortMap.len + unitDescrPtr->mapReg.fldSliceEn.len;
    *fldOffsPtr = (*fldLenPtr) * (sliceId % unitDescrPtr->mapReg.numberFldsPerReg);
    return GT_OK;
}

/**
* @internal prvCpssPxPizzaArbiterUnitDescrDrvGet function
* @endinternal
*
* @brief   get unit state (dump of HW)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - dev number
* @param[in] unitDescrPtr             - unit description
*
* @param[out] unitStatePtr             - (pointer to )unit state
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
static GT_STATUS prvCpssPxPizzaArbiterUnitDescrDrvGet
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  PRV_CPSS_PX_PA_UNIT_STC              *unitDescrPtr,
    OUT CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC *unitStatePtr
)
{
    GT_STATUS   rc;
    GT_U32      sliceId;
    GT_U32      regAddrCtrl;
    GT_U32      regAddrMap;
    GT_U32      fldOffs;
    GT_U32      fldLen;
    GT_U32      fldVal;
    GT_PORT_NUM portNum;
    GT_U32      portNumMask;
    GT_U32      workConservModeEn;
    GT_U32      sliceNum;
    GT_U32      mapLoadEn;


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(unitStatePtr);

    /*---------------------------*/
    /* control register          */
    /*---------------------------*/
    rc = prvCpssPxPizzaArbiterUnitDrvCtrlRegAddr(devNum,unitDescrPtr,&regAddrCtrl);
    if (rc != GT_OK)
    {
        return rc;
    }
         
    rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldMapLoadEn.offset,
                                            unitDescrPtr->ctrlReg.fldMapLoadEn.len,
                                            &mapLoadEn);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldWorkConservModeEn.offset,
                                            unitDescrPtr->ctrlReg.fldWorkConservModeEn.len,
                                            &workConservModeEn);


    /*------------------*/
    /* slice number get */
    /*------------------*/
    rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),  0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldSliceNum2run.offset,
                                            unitDescrPtr->ctrlReg.fldSliceNum2run.len,
                                            &sliceNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    unitStatePtr->totalSlicesOnUnit     = unitDescrPtr->mapReg.totalFlds;
    unitStatePtr->totalConfiguredSlices = sliceNum;
    unitStatePtr->workConservingBit     = (GT_BOOL)workConservModeEn;
    unitStatePtr->pizzaMapLoadEnBit     = (GT_BOOL)mapLoadEn;

    /*-----------------------------*/
    /* mapping register            */
    /*-----------------------------*/
    /*----------------------*/
    /* enable mapped slices */
    /*----------------------*/
    for (sliceId = 0; sliceId < unitDescrPtr->mapReg.totalFlds; sliceId++)
    {
        rc = prvCpssPxPizzaArbiterUnitDrvMapRegAddrFldOffsLenGet(devNum,unitDescrPtr,sliceId,
                                                                /*OUT*/&regAddrMap,&fldLen,&fldOffs);
        if (rc != GT_OK)
        {
            return rc;
        }

        fldVal  = 0;
        rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), 0, regAddrMap,
                                                fldOffs, fldLen, &fldVal);
        if (rc != GT_OK)
        {
            return rc;
        }
        portNumMask = (1 << unitDescrPtr->mapReg.fldSlice2PortMap.len) - 1;
        portNum  = (fldVal>>unitDescrPtr->mapReg.fldSlice2PortMap.offset) & portNumMask;

        unitStatePtr->slice_occupied_by[sliceId] = portNum;
        if (0 != (fldVal >>unitDescrPtr->mapReg.fldSliceEn.offset))
        {
            unitStatePtr->slice_enable[sliceId] = GT_TRUE;
        }
        else
        {
            unitStatePtr->slice_enable[sliceId] = GT_FALSE;
        }
    }
    return GT_OK;
}


/**
* @internal prvCpssPxPizzaArbiterUnitDescrDrvSet function
* @endinternal
*
* @brief   set unit state (configure HW)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - dev number
* @param[in] unitDescrPtr             - unit description
* @param[in] sliceNum                 - slice number of configure
* @param[in] slice2PortMapArr         - slice to client relation array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
static GT_STATUS prvCpssPxPizzaArbiterUnitDescrDrvSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN PRV_CPSS_PX_PA_UNIT_STC *unitDescrPtr,
    IN GT_U32                   sliceNum,
    IN GT_U32                  *slice2PortMapArr
)
{
    GT_STATUS rc;
    GT_U32    sliceId;
    GT_U32    regAddrCtrl;
    GT_U32    regAddrMap;
    GT_U32    fldOffs;
    GT_U32    fldLen;
    GT_U32    fldVal;
    GT_PORT_NUM portNum;


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (sliceNum > 0)
    {
        CPSS_NULL_PTR_CHECK_MAC(slice2PortMapArr);
        if (sliceNum > unitDescrPtr->mapReg.totalFlds)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

    }
    if (unitDescrPtr->unit == CPSS_PX_PA_UNIT_TXQ_0_E)
    {
        unitDescrPtr->unit = unitDescrPtr->unit;
    }

    /*---------------------------*/
    /* load disable              */
    /*---------------------------*/
    rc = prvCpssPxPizzaArbiterUnitDrvCtrlRegAddr(devNum,unitDescrPtr,/*OUT*/&regAddrCtrl);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldMapLoadEn.offset,
                                            unitDescrPtr->ctrlReg.fldMapLoadEn.len,
                                            0);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*------------------*/
    /* slice number set */
    /*------------------*/
    rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldSliceNum2run.offset,
                                            unitDescrPtr->ctrlReg.fldSliceNum2run.len,
                                            sliceNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*-----------------------------*/
    /* mapping register            */
    /*----------------------*/
    /* enable mapped slices */
    /*----------------------*/
    if (slice2PortMapArr != NULL)
    {
        for (sliceId = 0; sliceId < sliceNum ; sliceId++)
        {
            rc = prvCpssPxPizzaArbiterUnitDrvMapRegAddrFldOffsLenGet(devNum,unitDescrPtr,sliceId,
                                                                    /*OUT*/&regAddrMap,&fldLen,&fldOffs);
            if (rc != GT_OK)
            {
                return rc;
            }

            fldVal = 0;
            portNum = slice2PortMapArr[sliceId];
            if (portNum != PA_INVALID_PORT_CNS)
            {
                fldVal  = fldVal | (portNum << unitDescrPtr->mapReg.fldSlice2PortMap.offset);
                fldVal  = fldVal | (1 << unitDescrPtr->mapReg.fldSliceEn.offset);
            }

            rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), 0, regAddrMap,
                                                    fldOffs, fldLen, fldVal);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    /*---------------------------*/
    /* disable non-mapped slices */
    /*---------------------------*/
    for (sliceId = sliceNum; sliceId < unitDescrPtr->mapReg.totalFlds; sliceId++)
    {
        rc = prvCpssPxPizzaArbiterUnitDrvMapRegAddrFldOffsLenGet(devNum,unitDescrPtr,sliceId,
                                                                /*OUT*/&regAddrMap,&fldLen,&fldOffs);
        if (rc != GT_OK)
        {
            return rc;
        }

        fldVal  = 0;
        rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), 0, regAddrMap,
                                                fldOffs, fldLen, fldVal);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /*---------------------------*/
    /* load enable               */
    /*---------------------------*/
    rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldMapLoadEn.offset,
                                            unitDescrPtr->ctrlReg.fldMapLoadEn.len,
                                            1);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal internal_prvCpssPxPizzaArbiterUnitDescrDrvWorkConservingModeSet function
* @endinternal
*
* @brief   This function sets Work Conserving mode for specific PA unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to assign in CPSS DB for the initialized PX device.
* @param[in] unitDescrPtr             - unit description
* @param[in] status                   - status
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_prvCpssPxPizzaArbiterUnitDescrDrvWorkConservingModeSet
(
    IN GT_SW_DEV_NUM             devNum,
    IN PRV_CPSS_PX_PA_UNIT_STC * unitDescrPtr,
    IN GT_BOOL                   status
)
{
    GT_STATUS rc;
    GT_U32    regAddrCtrl;
    GT_U32    value;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(unitDescrPtr);
    /*------------------*/
    /* control register */
    /*------------------*/
    rc = prvCpssPxPizzaArbiterUnitDrvCtrlRegAddr(devNum,unitDescrPtr,&regAddrCtrl);
    if (rc != GT_OK)
    {
        return rc;
    }

    value = (GT_U32)status;
    rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldWorkConservModeEn.offset,
                                            unitDescrPtr->ctrlReg.fldWorkConservModeEn.len,
                                            value);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal internal_prvCpssPxPizzaArbiterUnitDescrDrvWorkConservingModeGet function
* @endinternal
*
* @brief   This function gets Work Conserving mode for specific PA unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to assign in CPSS DB for the initialized PX device.
* @param[in] unitDescrPtr             - unit description
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_prvCpssPxPizzaArbiterUnitDescrDrvWorkConservingModeGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN PRV_CPSS_PX_PA_UNIT_STC *unitDescrPtr,
    IN GT_BOOL                 *statusPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddrCtrl;
    GT_U32    value;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(unitDescrPtr);
    CPSS_NULL_PTR_CHECK_MAC(statusPtr);
    /*------------------*/
    /* control register */
    /*------------------*/
    rc = prvCpssPxPizzaArbiterUnitDrvCtrlRegAddr(devNum,unitDescrPtr,&regAddrCtrl);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldWorkConservModeEn.offset,
                                            unitDescrPtr->ctrlReg.fldWorkConservModeEn.len,
                                            &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    *statusPtr = (GT_BOOL)value;
    return GT_OK;
}


/**
* @internal internal_cpssPxPizzaArbiterUnitDrvInit function
* @endinternal
*
* @brief   This function init spicific PA unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to assign in CPSS DB for the initialized PX device.
* @param[in] unitDescrPtr             - unit descrition
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPizzaArbiterUnitDrvInit
(
    IN GT_SW_DEV_NUM            devNum,
    IN PRV_CPSS_PX_PA_UNIT_STC *unitDescrPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddrCtrl;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    /*------------------*/
    /* control register */
    /*------------------*/
    rc = prvCpssPxPizzaArbiterUnitDrvCtrlRegAddr(devNum,unitDescrPtr,&regAddrCtrl);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldSliceNum2run.offset,
                                            unitDescrPtr->ctrlReg.fldSliceNum2run.len,
                                            0);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), 0, regAddrCtrl,
                                            unitDescrPtr->ctrlReg.fldWorkConservModeEn.offset,
                                            unitDescrPtr->ctrlReg.fldWorkConservModeEn.len,
                                            0);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*---------------------------*/
    /* disable non-mapped slices */
    /*---------------------------*/
    rc = prvCpssPxPizzaArbiterUnitDescrDrvSet(devNum,unitDescrPtr,0,(GT_U32*)NULL);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}




/*-------------------------------------------------------------------------
 *   Level UnitId :
 *        pipePizzaArbiterUnitDrvSet
 *        pipePizzaArbiterUnitDrvGet
 *        pipePizzaArbiterUnitDrvWorkConservingModeSet
 *        pipePizzaArbiterUnitDrvWorkConservingModeGet
 *        pipePizzaArbiterUnitDrvInit
 *-------------------------------------------------------------------------*/
static PRV_CPSS_PX_PA_UNIT_DEF_STC *prv_paUnitDef_All_pipe[] = 
{
     /* RxDma */
     &prv_pipe_rxDma_0_Def
     /* TxQ */
    ,&prv_pipe_txQDef_0
    ,&prv_pipe_txQDef_1
     /* TxDma */
    ,&prv_pipe_txDma_0_Def
     /* TxFifo */
    ,&prv_pipe_txFifo_0_Def
    ,(PRV_CPSS_PX_PA_UNIT_DEF_STC *)NULL
};


PRV_CPSS_PX_PA_UNIT_STC   prv_unitsDescrAll_pipe     [CPSS_PX_PA_UNIT_MAX_E];

PRV_CPSS_PX_DEV_X_UNITDEFLIST    prv_dev_unitDefList_pipe[] = 
{
     /*------------------------------------------------------------------------------------*
      *       device family,      unit definition-user        unit-description-operational *
      *------------------------------------------------------------------------------------*/
     { CPSS_PX_FAMILY_PIPE_E,    &prv_paUnitDef_All_pipe[0]  , &prv_unitsDescrAll_pipe[0] }
    ,{       CPSS_MAX_FAMILY,          NULL                  , NULL}
};

PRV_CPSS_PX_PA_UNITS_DRV_STC prv_pipe_paUnitsDrv = 
{
    GT_FALSE,
    &prv_dev_unitDefList_pipe[0]
};


/*---------------------------------------*
 * function                              *
 *---------------------------------------*/
/**
* @internal prvCpssPxPizzaArbiterUnitDescrDrvDescrByUnitIdGet function
* @endinternal
*
* @brief   get unit description by unit ID
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to assign in CPSS DB for the initialized PX device.
* @param[in] unitId                   - unit id
*
* @param[out] unitDescrPtrPtr          - (pointer to) unit description
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPizzaArbiterUnitDescrDrvDescrByUnitIdGet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  CPSS_PX_PA_UNIT_ENT       unitId,
    OUT PRV_CPSS_PX_PA_UNIT_STC **unitDescrPtrPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(unitDescrPtrPtr);

    if (unitId < 0 || unitId >= CPSS_PX_PA_UNIT_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (prv_pipe_paUnitsDrv.isInitilaized == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* something bad, driver is called to get data BEFORE it is initialized */
    }

    *unitDescrPtrPtr = &(PRV_CPSS_PX_PP_MAC(devNum)->paData.paUnitDrvList[unitId]);
    return GT_OK;
}

/**
* @internal prvCpssPxPizzaArbiterUnitDrvSet function
* @endinternal
*
* @brief   configure pizza arbiter unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to assign in CPSS DB for the initialized PX device.
* @param[in] unitId                   - unit id
* @param[in] sliceNum                 - slice Number to run
* @param[in] slice2PortMapArr         - client array
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPizzaArbiterUnitDrvSet
(
    IN GT_SW_DEV_NUM       devNum,
    IN CPSS_PX_PA_UNIT_ENT unitId,
    IN GT_U32              sliceNum,
    IN GT_U32             *slice2PortMapArr
)
{
    GT_STATUS   rc;
    PRV_CPSS_PX_PA_UNIT_STC * unitDescrPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(slice2PortMapArr);

    rc = prvCpssPxPizzaArbiterUnitDescrDrvDescrByUnitIdGet(devNum,unitId, /*OUT*/&unitDescrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (unitDescrPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPizzaArbiterUnitDescrDrvSet(devNum,unitDescrPtr,sliceNum,slice2PortMapArr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssPxPizzaArbiterUnitDrvGet function
* @endinternal
*
* @brief   This function gets unit info from HW
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to assign in CPSS DB for the initialized PX device.
* @param[in] unitId                   - unit id
*
* @param[out] unitStatePtr             - (pointer to) unit state
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPizzaArbiterUnitDrvGet
(
    IN GT_SW_DEV_NUM                       devNum,
    IN CPSS_PX_PA_UNIT_ENT                 unitId,
    CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC * unitStatePtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_PX_PA_UNIT_STC * unitDescrPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(unitStatePtr);
    rc = prvCpssPxPizzaArbiterUnitDescrDrvDescrByUnitIdGet(devNum,unitId, /*OUT*/&unitDescrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssPxPizzaArbiterUnitDescrDrvGet(devNum,unitDescrPtr,unitStatePtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssPxPizzaArbiterUnitDrvWorkConservingModeSet function
* @endinternal
*
* @brief   This function define Work Conserving mode for specific PA unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to assign in CPSS DB for the initialized PX device.
* @param[in] unitId                   - unit id
* @param[in] status                   - (GT_TRUE when on
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPizzaArbiterUnitDrvWorkConservingModeSet
(
    IN GT_SW_DEV_NUM         devNum,
    IN CPSS_PX_PA_UNIT_ENT   unitId,
    IN GT_BOOL               status
)
{
    GT_STATUS   rc;
    PRV_CPSS_PX_PA_UNIT_STC * unitDescrPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = prvCpssPxPizzaArbiterUnitDescrDrvDescrByUnitIdGet(devNum, unitId, /*OUT*/&unitDescrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = internal_prvCpssPxPizzaArbiterUnitDescrDrvWorkConservingModeSet(devNum,unitDescrPtr,status);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssPxPizzaArbiterUnitDrvWorkConservingModeGet function
* @endinternal
*
* @brief   This function gets Work Conserving mode for specific PA unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to assign in CPSS DB for the initialized PX device.
* @param[in] unitId                   - unit id
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPizzaArbiterUnitDrvWorkConservingModeGet
(
    IN GT_SW_DEV_NUM         devNum,
    IN CPSS_PX_PA_UNIT_ENT   unitId,
    IN GT_BOOL              *statusPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_PX_PA_UNIT_STC * unitDescrPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = prvCpssPxPizzaArbiterUnitDescrDrvDescrByUnitIdGet(devNum,unitId, /*OUT*/&unitDescrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = internal_prvCpssPxPizzaArbiterUnitDescrDrvWorkConservingModeGet(devNum,unitDescrPtr,statusPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;

}

/**
* @internal prvCpssPxPizzaArbiterUnitDrvInit function
* @endinternal
*
* @brief   This function init spicific PA unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to assign in CPSS DB for the initialized PX device.
* @param[in] unitId                   - unit id
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPizzaArbiterUnitDrvInit
(
    IN GT_SW_DEV_NUM       devNum,
    IN CPSS_PX_PA_UNIT_ENT unitId
)
{
    GT_STATUS rc;
    PRV_CPSS_PX_PA_UNIT_STC * unitDescrPtr;
    GT_U32 i;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (prv_pipe_paUnitsDrv.isInitilaized == GT_FALSE)
    {
        for (i = 0 ; prv_pipe_paUnitsDrv.dev_x_unitDefList[i].family != CPSS_MAX_FAMILY; i++)
        {
            rc = prvCpssPxPizzaArbiterUnitDrvUnitDescrInit(prv_pipe_paUnitsDrv.dev_x_unitDefList[i].unitDefListPtr,
                                                                        CPSS_PX_PA_UNIT_MAX_E,
                                                                        /*OUT*/prv_pipe_paUnitsDrv.dev_x_unitDefList[i].unitsDescrAllPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        prv_pipe_paUnitsDrv.isInitilaized = GT_TRUE;
    }
    /*-------------------------------*
     * find for specific device      *
     *-------------------------------*/
    if (PRV_CPSS_PX_PP_MAC(devNum)->paData.paUnitDrvList == NULL)
    {
        for (i = 0 ; prv_pipe_paUnitsDrv.dev_x_unitDefList[i].family != CPSS_MAX_FAMILY; i++)
        {
            if (   prv_pipe_paUnitsDrv.dev_x_unitDefList[i].family == PRV_CPSS_PX_PP_MAC(devNum)->genInfo.devFamily)
            {
                break;
            }
        }
        if (prv_pipe_paUnitsDrv.dev_x_unitDefList[i].family == CPSS_MAX_FAMILY)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        PRV_CPSS_PX_PP_MAC(devNum)->paData.paUnitDrvList = prv_pipe_paUnitsDrv.dev_x_unitDefList[i].unitsDescrAllPtr;
    }

    rc = prvCpssPxPizzaArbiterUnitDescrDrvDescrByUnitIdGet(devNum, unitId, /*OUT*/&unitDescrPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if ((tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E) &&
        (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E))
    {
        rc = internal_cpssPxPizzaArbiterUnitDrvInit(devNum,unitDescrPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}



/**
* @internal prvCpssPxPortDynamicPizzaArbiterIfTxQPortWorkConservingModeSet function
* @endinternal
*
* @brief   Configure TxQ port work conserving mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] txqPort                  - txq port
* @param[in] status                   - enable (1) disable (0)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterIfTxQPortWorkConservingModeSet
(
    IN GT_SW_DEV_NUM             devNum,
    IN GT_U32                    txqPort,
    IN GT_BOOL                   status
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    dpIdx;
    GT_U32    localTxqPort;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = prvCpssPxPpResourcesTxqGlobal2LocalConvert(devNum,txqPort,/*OUT*/&dpIdx,&localTxqPort);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,dpIdx).scheduler.portArbiterConfig.portsArbiterPortWorkConservingEnable[localTxqPort];
    rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), 0, regAddr,
                                            PRV_CPSS_PX_PORT_WORK_CONSERV_OFFS_CNS,
                                            PRV_CPSS_PX_PORT_WORK_CONSERV_LEN_CNS,
                                            (GT_U32)status);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}


/**
* @internal prvCpssPxPortDynamicPizzaArbiterIfTxQPortWorkConservingModeGet function
* @endinternal
*
* @brief   get TxQ port work conserving mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - device number
* @param[in] txqPort                  - txq port
*
* @param[out] statusPtr                - (pointer to )enable (1) disable (0)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterIfTxQPortWorkConservingModeGet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  GT_U32                    txqPort,
    OUT GT_BOOL                  *statusPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    value;
    GT_U32    dpIdx;
    GT_U32    localTxqPort;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    rc = prvCpssPxPpResourcesTxqGlobal2LocalConvert(devNum,txqPort,/*OUT*/&dpIdx,&localTxqPort);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_INDEX_MAC(devNum,dpIdx).scheduler.portArbiterConfig.portsArbiterPortWorkConservingEnable[localTxqPort];
    rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), 0, regAddr,
                                            PRV_CPSS_PX_PORT_WORK_CONSERV_OFFS_CNS,
                                            PRV_CPSS_PX_PORT_WORK_CONSERV_LEN_CNS,
                                            &value);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if (value > GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }
    *statusPtr = (GT_BOOL)value;
    return GT_OK;
}






