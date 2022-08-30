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
* @file prvCpssPxPortDynamicPAUnitDrv.h
*
* @brief pipe and higher dynamic (algorithmic) pizza arbiter unit driver
*
* @version   1
********************************************************************************
*/
#ifndef __PRV_CPSS_PX_PORT_DYNAMIC_PA_UNIT_DRV_H
#define __PRV_CPSS_PX_PORT_DYNAMIC_PA_UNIT_DRV_H

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/PizzaArbiter/cpssPxPortPizzaArbiter.h>

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


 
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */ 


#define PA_INVALID_PORT_CNS  0xFFFF

/**
* @struct PRV_CPSS_PX_PA_REG_FIELDC_STC
 *
 * @brief This struct defines field's ofsset and len inside the register
*/
typedef struct{

    /** offset of the field */
    GT_U32 offset;

    /** @brief length of the field
     *  Comments:
     */
    GT_U32 len;

} PRV_CPSS_PX_PA_REG_FIELDC_STC;

/**
* @struct PRV_CPSS_PX_PA_CTRL_REG_STC
 *
 * @brief This struct defines the structure of PA control register operational definition
*/
typedef struct{

    /** offset of the field in register DB */
    GT_U32 baseAddressPtrOffs;

    /** field : sliceN to run */
    PRV_CPSS_PX_PA_REG_FIELDC_STC fldSliceNum2run;

    /** field : load map */
    PRV_CPSS_PX_PA_REG_FIELDC_STC fldMapLoadEn;

    /** @brief field : unit work consrving
     *  Comments:
     */
    PRV_CPSS_PX_PA_REG_FIELDC_STC fldWorkConservModeEn;

} PRV_CPSS_PX_PA_CTRL_REG_STC;


/**
* @struct PRV_CPSS_PX_PA_MAP_REG_STC
 *
 * @brief This struct defines the structure of PA map register
*/
typedef struct{

    /** offset of the field in register DB */
    GT_U32 baseAddressPtrOffs;

    /** @brief field : to which client belong slice
     *  fldMapLoadEn     - field : load map
     */
    PRV_CPSS_PX_PA_REG_FIELDC_STC fldSlice2PortMap;

    PRV_CPSS_PX_PA_REG_FIELDC_STC fldSliceEn;

    /** total registers in register array */
    GT_U32 totalReg;

    /** how many fields is there per register */
    GT_U32 numberFldsPerReg;

    /** @brief total fields in register array
     *  Comments:
     */
    GT_U32 totalFlds;

} PRV_CPSS_PX_PA_MAP_REG_STC;

/*
 * typedef: struct PRV_CPSS_PX_PA_UNIT_STC
 *
 * Description:
 *      This struct defines the structure of PA Unit
 *
 * Fields:
 *      unit                 - unit type (eg. RX-DMA-i, TXQ-DQ-i, TX-DMA-i, TX-FIFO-i)
 *      ctrlReg              - control register structure
 *      mapReg               - map register array structure
 * Comments:
 *
 */

typedef struct PRV_CPSS_PX_PA_UNIT_STCT
{
    CPSS_PX_PA_UNIT_ENT         unit;
    PRV_CPSS_PX_PA_CTRL_REG_STC ctrlReg;
    PRV_CPSS_PX_PA_MAP_REG_STC  mapReg;
}PRV_CPSS_PX_PA_UNIT_STC;

/* RXDMA TXDMA TX-FIFO  */

#define PRV_CPSS_PX_PA_CTRLREG_SLICE_NUM_2_RUN_OFFS_CNS  0
#define PRV_CPSS_PX_PA_CTRLREG_SLICE_NUM_2_RUN_LEN_CNS   9

#define PRV_CPSS_PX_PA_CTRLREG_MAP_LOAD_EN_OFFS_CNS      14
#define PRV_CPSS_PX_PA_CTRLREG_MAP_LOAD_EN_LEN_CNS       1

#define PRV_CPSS_PX_PA_CTRLREG_WORK_CONSERV_EN_OFFS_CNS  15
#define PRV_CPSS_PX_PA_CTRLREG_WORK_CONSERV_EN_LEN_CNS   1

/* TXQ */
#define PRV_CPSS_PX_PA_CTRLREG_TXQ_SLICE_NUM_2_RUN_OFFS_CNS  0
#define PRV_CPSS_PX_PA_CTRLREG_TXQ_SLICE_NUM_2_RUN_LEN_CNS   9

#define PRV_CPSS_PX_PA_CTRLREG_TXQ_WORK_CONSERV_EN_OFFS_CNS  9
#define PRV_CPSS_PX_PA_CTRLREG_TXQ_WORK_CONSERV_EN_LEN_CNS   1

#define PRV_CPSS_PX_PA_CTRLREG_TXQ_MAP_LOAD_EN_OFFS_CNS      12
#define PRV_CPSS_PX_PA_CTRLREG_TXQ_MAP_LOAD_EN_LEN_CNS       1


/*-------------------------*/
/* MAP register definition */
/*-------------------------*/
/* RXDMA TXDMA TXQ TX-FIFO  */
#define PRV_CPSS_PX_PA_MAP_REGNUM_CNS                    85
#define PRV_CPSS_PX_PA_MAP_FLD_PER_REG_CNS               4
#define PRV_CPSS_PX_PA_MAP_TOTAL_FLDS_REG_CNS            340

#define PRV_CPSS_PX_PA_MAPREG_WORK_SLICEMAP_OFFS_CNS     0
#define PRV_CPSS_PX_PA_MAPREG_WORK_SLICEMAP_LEN_CNS      7

#define PRV_CPSS_PX_PA_MAPREG_WORK_SLICE_EN_OFFS_CNS     7
#define PRV_CPSS_PX_PA_MAPREG_WORK_SLICE_EN_LEN_CNS      1

/**
* @internal prvCpssPxPizzaArbiterUnitDrvGet function
* @endinternal
*
* @brief   This function gets unit info from HW
*
* @note   APPLICABLE DEVICES:      Pipe.
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
    IN  GT_SW_DEV_NUM                         devNum,
    IN  CPSS_PX_PA_UNIT_ENT                   unitId,
    OUT CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC *unitStatePtr
);

/**
* @internal prvCpssPxPizzaArbiterUnitDrvSet function
* @endinternal
*
* @brief   configure pizza arbiter unit
*
* @note   APPLICABLE DEVICES:      Pipe.
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
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_PA_UNIT_ENT  unitId,
    IN GT_U32               sliceNum,
    IN GT_U32              *slice2PortMapArr
);

/*------------------------------------*/
/* Work conserving mode per TxQ port  */
/*------------------------------------*/
#define PRV_CPSS_PX_PORT_WORK_CONSERV_OFFS_CNS 0
#define PRV_CPSS_PX_PORT_WORK_CONSERV_LEN_CNS  1
/**
* @internal prvCpssPxPortDynamicPizzaArbiterIfTxQPortWorkConservingModeSet function
* @endinternal
*
* @brief   Configure TxQ port work conserving mode
*
* @note   APPLICABLE DEVICES:      Pipe.
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
);

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
);


/*------------------------------------*/
/* Work conserving mode per unit      */
/*------------------------------------*/
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
);

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
);


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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

