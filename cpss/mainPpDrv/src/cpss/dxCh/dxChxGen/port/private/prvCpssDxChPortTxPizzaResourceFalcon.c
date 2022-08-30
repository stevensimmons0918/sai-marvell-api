
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
* @file prvCpssDxChPortTxPizzaResourceFalcon.c
*
* @brief CPSS implementation for Tx pizza and resources configuration.
*
*
* @version   116
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceFalcon.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMapping.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqFcUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_SHARED_PORT_PRV_DB_VAR(_var)\
      PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChPortDir.portPrvSrc._var)

/* SIP_6 only: falcon txDMA txFFO Meta-Pizza, mapping between serdes ( the array indexes) to the serdes's slices (the bit map in each node)
   the Meta pizza have 33 slots (0 - 32).
   solt 32 - cpu slot: cpu slot not include in  txMetaPizza since it fixed after initilazition                                                                  .
   slice #32 belong to cpu BW even if the DP does not have cpu port                                                                                              .
   slice 0 - 31 : the mapping between slices and SERDES in slices 0 - 7 is duplicated in 8 - 15, 16 - 23 and 24 - 31.                                                                                                                                                               .
   the code have only Meta Pizza of 8 slices. the 32 slice Meta Pizza is:                                                                                                                                                                                                                                                                                  .
   {0x01010101, 0x10101010, 0x4040404, 0x40404040, 0x2020202, 0x20202020, 0x8080808, 0x80808080}                                                                                                                                                                                                                                                                                                                                                             .
   example: serdes 0 have slices number 0, 8, 16, 24 in the tx pizza in falcon */
static const GT_U8 txMetaPizza[FALCON_PORTS_PER_DP_CNS] =  {0x01 /*SD0*/, 0x10/*SD1*/, 0x04/*SD2*/, 0x40/*SD3*/, 0x02/*SD4*/, 0x20/*SD5*/, 0x08/*SD6*/,0x80/*SD7*/};

#define PRV_DIP6_TX_CPU_PORT_SLICE_NUM_CNS    32

/*local cpu port number in tx units*/
#define PRV_DIP6_TX_CPU_PORT_NUM_CNS(_devNum)    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].cpuPortDmaNum

/*slice number in TXDMA pizza */
#define TXD_PIZZA_SLICE_NUM 33
#define TXD_PIZZA_MAX_SLICE (TXD_PIZZA_SLICE_NUM - 1)

/*slice number in TXFIFO pizza */
#define TXF_PIZZA_SLICE_NUM 33
#define TXF_PIZZA_MAX_SLICE (TXF_PIZZA_SLICE_NUM - 1)

/**
* @internal prvCpssDxChPizzaSlicesFromSerdesGet function
* @endinternal
*
* @brief   get BMP of 32 pizza slices according to port .
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:
*         xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] numOfSlicesPer8Slices    - number of slices per 8 slices
*
* @param[out] pizzaSliceBitMapPtr     - (pointer to) pizza slice
*       bit map
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChPizzaSlicesFromSerdesGet
(
    IN  GT_U8                devNum,
    IN  GT_U8                numOfSlicesPer8Slices,
    IN  GT_U32               localPortIndex,
    OUT GT_U32               *pizzaSliceBitMapPtr
)
{
    GT_U8                   sliceIndex;
    GT_U8                   pizza8SlicesBitMap;

    pizza8SlicesBitMap = 0;

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "prvCpssDxChPizzaSlicesFromSerdesGet:  not implemented for HAWK ");
    }

    if ((numOfSlicesPer8Slices == 0) || ((numOfSlicesPer8Slices + localPortIndex) > FALCON_PORTS_PER_DP_CNS ))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "prvCpssDxChPizzaSlicesFromSerdesGet:  worng 'per 8 slices' number %d for local port index %d ",
            numOfSlicesPer8Slices,localPortIndex);
    }
    for (sliceIndex = 0; sliceIndex < numOfSlicesPer8Slices; sliceIndex++)
    {
        pizza8SlicesBitMap |= txMetaPizza[localPortIndex + sliceIndex];
    }
    /* the pizza is 32 slices and the meta pizza is 8 slices. need to duplicate the 8 bitmap to 32 bitmap*/
    *pizzaSliceBitMapPtr = (GT_U32)(pizza8SlicesBitMap) | (GT_U32)(pizza8SlicesBitMap << FALCON_PORTS_PER_DP_CNS) | (GT_U32)(pizza8SlicesBitMap << (FALCON_PORTS_PER_DP_CNS*2)) | (GT_U32)(pizza8SlicesBitMap << (FALCON_PORTS_PER_DP_CNS*3));
    return GT_OK;
}

/**
* @internal prvCpssDxChTxLoadNewPizzaSet function
* @endinternal
*
* @brief   Pizza arbiter in txFIFO load new pizza bit in control register set.
*
* @note   APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] txDmaIndex            - global Tx index
* @param[in] unitType              - PRV_CPSS_DXCH_UNIT_TXDMA_E,
*       PRV_CPSS_DXCH_UNIT_TX_FIFO_E
* @param[in] enable               - set new slices ( by add port number to the slice and by set bit valid to 1)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChTxLoadNewPizzaSet
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       txIndex,
    IN  PRV_CPSS_DXCH_UNIT_ENT       unitType,
    IN  GT_BOOL                      enable
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      value;

    /* write enable value */
    value = BOOL2BIT_MAC(enable);

    switch(unitType)
    {
        case PRV_CPSS_DXCH_UNIT_TXDMA_E:
            regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg;
            break;
        case PRV_CPSS_DXCH_UNIT_TX_FIFO_E:
            regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE, "prvCpssDxChTxLoadNewPizzaSet:  not applicable unit ");
    }

    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 31, 1, value);
    if (rc != GT_OK)
    {
       return rc;
    }

    /* read the register to generate delay, to make sure that this write command and the write command right after with enable = false
       will not “stick” and there will be more than pizza time (~40ns) between the commands.*/
    if (enable == GT_TRUE)
    {
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 31, 1, &value);
    }

    return rc;

}

/**
* @internal prvCpssDxChTxPizzaSlicesSet function
* @endinternal
*
* @brief    set port number for Pizza arbiter in txFIFO slices . also set valid control bit.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] txDmaIndex            - global Tx index
* @param[in] localChannelIndex     - local channel index ( 0..7)
* @param[in] portPizzaSlicesBitMap - bit map representing the slices sould be configured
* @param[in] enable                -  set new slices ( by add port number to the slice and by set bit valid to 1)
* @param[in] unitType              - PRV_CPSS_DXCH_UNIT_TXDMA_E,
*       PRV_CPSS_DXCH_UNIT_TX_FIFO_E
* @param[in] isCpu                 - is cpu port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode value received
*/
static GT_STATUS prvCpssDxChTxPizzaSlicesSet
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       txIndex,
    IN  GT_U32                       localChannelIndex,
    IN  GT_U32                       portPizzaSlicesBitMap,
    IN  GT_BOOL                      enable,
    IN  PRV_CPSS_DXCH_UNIT_ENT       unitType,
    IN  GT_BOOL                      isCpu
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      enableValue;
    GT_U32      value;
    GT_U32      ii;
    GT_U32      pizzaMaxSlice;
    GT_U32      pizzaMaxSlicePattern;
    GT_U32      tmpPort;


    /* write enable value */
    enableValue = BOOL2BIT_MAC(enable);

    if (GT_TRUE == isCpu)
    {
        /* not cpu port */
        switch(unitType)
        {
            case PRV_CPSS_DXCH_UNIT_TXDMA_E:
                regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[PRV_DIP6_TX_CPU_PORT_SLICE_NUM_CNS];
                break;
            case PRV_CPSS_DXCH_UNIT_TX_FIFO_E:
                regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[PRV_DIP6_TX_CPU_PORT_SLICE_NUM_CNS];
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE, "prvCpssDxChTxLoadNewPizzaSet:  not applicable unit ");
        }
        /*set cpu port index*/
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 4, localChannelIndex);
        if(rc != GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxPizzaSlicesSet [0x%8.8x] ", regAddr );
        }
        /* set the slice valid bit to 1 */
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 31, 1, enableValue);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxPizzaSlicesSet [0x%8.8x] ", regAddr );
        }
    }
    else
    {
       /* not cpu port */
        switch(unitType)
        {
            case PRV_CPSS_DXCH_UNIT_TXDMA_E:
                regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg;
                pizzaMaxSlicePattern = TXD_PIZZA_MAX_SLICE;
                break;
            case PRV_CPSS_DXCH_UNIT_TX_FIFO_E:
                regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg;
                pizzaMaxSlicePattern = TXF_PIZZA_MAX_SLICE;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE, "prvCpssDxChTxLoadNewPizzaSet:  not applicable unit ");
        }

        /*get pizza slices number*/
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 7, &pizzaMaxSlice);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxPizzaSlicesSet  [0x%8.8x] ", regAddr );
        }
        if (pizzaMaxSlice != pizzaMaxSlicePattern)
        {
            if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
            {
                /* The GM not supports the TXDMA unit , so it read 0xbadad value
                   from the non-existing register */
                return GT_OK;
            }

            /*falcon pizza size is 32*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(
                GT_BAD_PARAM, "prvCpssDxChTxPizzaSlicesSet: pizza size %d should be %d instead.",
                pizzaMaxSlice, pizzaMaxSlicePattern);
        }

        /* run on 32 (0 - 31) pizza slices. slice #32 is belong to cpu*/
        for (ii = 0; (ii < pizzaMaxSlice); ii++)
        {
            if (portPizzaSlicesBitMap & 0x1)
            {
                switch(unitType)
                {
                    case PRV_CPSS_DXCH_UNIT_TXDMA_E:
                        regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[ii];
                        break;
                    case PRV_CPSS_DXCH_UNIT_TX_FIFO_E:
                        regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[ii];
                        break;
                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE, "prvCpssDxChTxLoadNewPizzaSet:  not applicable unit ");
                }

                /* set the slice's port */
                if (GT_TRUE == enable)
                {
                     /*first check that the slice not belong to other port, its have to be invalid !!*/
                    rc = prvCpssDrvHwPpGetRegField(devNum,regAddr,31,1, &value);
                    if(rc != GT_OK)
                    {
                         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxPizzaSlicesSet prvCpssDrvHwPpGetRegField  [0x%8.8x] ", regAddr );
                    }

                    /* valid bit should be 0*/
                    if (0 != value)
                    {
                        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 4, &tmpPort);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "prvCpssDxChTxPizzaSlicesSet: slice %d is already belong to port %d !! ", ii,tmpPort );
                    }

                    /*set port number in the the slice*/
                    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 4, localChannelIndex);
                    if(rc != GT_OK)
                    {
                         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxPizzaSlicesSet  [0x%8.8x] ", regAddr );
                    }
                }

                /* set the slice valid bit as disable or enable */
                rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 31, 1, enableValue);
                if(rc != GT_OK)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxDmaPizzaSlicesSet prvCpssDrvHwPpGetRegField  [0x%8.8x] ", regAddr );
                }
            }

            portPizzaSlicesBitMap = portPizzaSlicesBitMap >> 1;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxPizzaConfigurationSet function
* @endinternal
*
* @brief  Pizza arbiter in txDMA configuration set.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] txIndex               - global Tx index
* @param[in] localChannelIndex     - local channel index ( 0..7)
* @param[in] portPizzaSlicesBitMap - bit map representing the slices sould be configured
* @param[in] enable                -  set new slices ( by add port number to the slice and by set bit valid to 1)
* @param[in] unitType              - PRV_CPSS_DXCH_UNIT_TXDMA_E,
*       PRV_CPSS_DXCH_UNIT_TX_FIFO_E
* @param[in] isCpu                 - is cpu port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxPizzaConfigurationSet
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       txIndex,
    IN  GT_U32                       localChannelIndex,
    IN  GT_U32                       portPizzaSlicesBitMap,
    IN  GT_BOOL                      enable,
    IN  PRV_CPSS_DXCH_UNIT_ENT       unitType,
    IN  GT_BOOL                      isCpu
)
{
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_AC3X_E);

    rc = prvCpssDxChTxLoadNewPizzaSet(devNum, txIndex, unitType, GT_FALSE);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxLoadNewPizzaSet faild for tx number %d", txIndex );
    }

    rc = prvCpssDxChTxPizzaSlicesSet(devNum, txIndex, localChannelIndex, portPizzaSlicesBitMap, enable, unitType, isCpu);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxPizzaConfigSet faild for tx number %d, localchannel %d", txIndex, localChannelIndex);
    }

    rc = prvCpssDxChTxLoadNewPizzaSet(devNum, txIndex, unitType, GT_TRUE);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxLoadNewPizzaSet faild for tx number %d", txIndex );
    }

    rc = prvCpssDxChTxLoadNewPizzaSet(devNum, txIndex, unitType, GT_FALSE);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxLoadNewPizzaSet faild for tx number %d", txIndex );
    }

    return rc;
}

/**
* @internal prvCpssDxChTxPortPizzaArbiterMaxSliceSet function
* @endinternal
*
* @brief   Pizza arbiter set max slice ( pizza size) in pizza configuration in txdma
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] txIndex               - global Tx index
* @param[in] unitType              - PRV_CPSS_DXCH_UNIT_TXDMA_E,
*       PRV_CPSS_DXCH_UNIT_TX_FIFO_E
* @param[in] maxSliceNum           - max slice number to confige
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxPortPizzaArbiterMaxSliceSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  txIndex,
    IN  PRV_CPSS_DXCH_UNIT_ENT  unitType,
    IN  GT_U32                  maxSliceNum
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_AC3X_E);

    rc = prvCpssDxChTxLoadNewPizzaSet(devNum, txIndex, unitType, GT_FALSE);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxLoadNewPizzaSet failed ");
    }

    switch(unitType)
    {
        case PRV_CPSS_DXCH_UNIT_TXDMA_E:
            regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg;
            break;
        case PRV_CPSS_DXCH_UNIT_TX_FIFO_E:
            regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FALSE, "prvCpssDxChTxLoadNewPizzaSet:  not applicable unit ");
    }
    /* set slice number in DMA domain */
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 7, maxSliceNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChTxLoadNewPizzaSet(devNum, txIndex, unitType, GT_TRUE);
    if(rc != GT_OK)
    {
         return rc;
    }

    rc = prvCpssDxChTxLoadNewPizzaSet(devNum, txIndex, unitType, GT_FALSE);
    return rc;
}

/**
* @internal prvCpssDxChFalconPortPizzaArbiterInit function
* @endinternal
*
* @brief   Pizza arbiter initialization in all Units where it's present for falcon
*       the function set the slices number and  go over All pizza slices (except cpu slice) in all DP and set the slice valid bit to 0.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChFalconPortPizzaArbiterInit
(
    IN  GT_U8                   devNum
)
{
    GT_STATUS                    rc;
    GT_U32                       ii;
    GT_U32                       PizzaArbitersNum;/*support the GM with single DP*/
    GT_U32                       portSlicesBitMap  = 0xFFFFFFFF; /*go over all pizza slices*/


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_AC3X_E);

    PizzaArbitersNum = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;

    /* go over all DP, each DP has 2 pizza configuration: TxDMA and TxFIFO*/
    for (ii = 0 ; ii < PizzaArbitersNum ; ii++ )
    {
         /* set slice number in DMA domain */
         rc = prvCpssDxChTxPortPizzaArbiterMaxSliceSet(devNum, ii, PRV_CPSS_DXCH_UNIT_TXDMA_E, TXD_PIZZA_MAX_SLICE);
         if (rc != GT_OK)
         {
             return rc;
         }

         /* set slice number in FIFO domain */
         rc = prvCpssDxChTxPortPizzaArbiterMaxSliceSet(devNum, ii, PRV_CPSS_DXCH_UNIT_TX_FIFO_E, TXF_PIZZA_MAX_SLICE);
         if (rc != GT_OK)
         {
             return rc;
         }
         /*set 0 to valid bit for each pizza slice*/
         rc = prvCpssDxChTxPizzaConfigurationSet(devNum,ii/*txglobal index*/,0/*local channel index*/,portSlicesBitMap, GT_FALSE /*config enable*/, PRV_CPSS_DXCH_UNIT_TXDMA_E, GT_FALSE /*is cpu*/);
         if (rc != GT_OK)
         {
             return rc;
         }
         rc = prvCpssDxChTxPizzaConfigurationSet(devNum,ii/*txglobal index*/,0/*local channel index*/,portSlicesBitMap, GT_FALSE /*config enable*/, PRV_CPSS_DXCH_UNIT_TX_FIFO_E, GT_FALSE /*is cpu*/);
         if (rc != GT_OK)
         {
             return rc;
         }

   }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortTxPizzaSliceGet function
* @endinternal
*
* @brief   Get Pizza arbiter slices per port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
* @param[in] txIndex                - global Tx index
* @param[out] txPizzaSliceArrPtr   - (pointer to) array in size
*             FALCON_PORTS_PER_DP_CNS (8). each node should hold
*             the number of the port slices. the index is the
*             port number in the pizza slice.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortTxPizzaSliceGet
(
    IN   GT_U8       devNum,
    IN   GT_U32      txIndex,
    OUT  GT_U32     *txPizzaSliceArrPtr
)
{
    GT_STATUS                           rc;
    GT_U32                              regAddrTxDMA;
    GT_U32                              regAddrTxFIFO;
    GT_U32                              txDmaValue; /* get the valid bit and the port number, the port number use as index for txPizzaSliceArrPtr*/
    GT_U32                              txFifoValue;/* get the valid bit and the port number, the port number use as index for txFIFOPizzaSliceArry*/
    GT_U32                              ii;
    GT_U32                              txFIFOPizzaSliceArry[FALCON_PORTS_PER_DP_CNS] = {0};

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(txPizzaSliceArrPtr);


    /*reset the array*/
    for (ii = 0 ; ii <= FALCON_PORTS_PER_DP_CNS ; ii++)
    {
        txPizzaSliceArrPtr[ii] = 0;
    }
    /*txDMA*/
    for (ii = 0 ; ii < (TXD_PIZZA_SLICE_NUM - 1) ; ii++ )
    {
        regAddrTxDMA = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum,txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[ii];
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxDMA, 31, 1, &txDmaValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (txDmaValue == 0)
        {
            /* the slice is not valid*/
            continue;
        }
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxDMA, 0, 4, &txDmaValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (txDmaValue >= FALCON_PORTS_PER_DP_CNS )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "the slice's port number %d is higher than %d\n", txDmaValue, FALCON_PORTS_PER_DP_CNS);
        }
        txPizzaSliceArrPtr[txDmaValue]++;
    }
    /*txFIFO*/
    for (ii = 0 ; ii < (TXF_PIZZA_SLICE_NUM - 1 ) ; ii++ )
    {
        regAddrTxFIFO = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[ii];
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxFIFO, 31, 1, &txFifoValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (txFifoValue == 0)
        {
            /* the slice is not valid*/
            continue;
        }
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxFIFO, 0, 4, &txFifoValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (txFifoValue >= FALCON_PORTS_PER_DP_CNS )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "the slice's port number %d is higher than %d", txFifoValue, FALCON_PORTS_PER_DP_CNS);

        }
        txFIFOPizzaSliceArry[txFifoValue]++;
    }

    /* the txDMA pizza and txFIFO pizza have to the same*/
    for (ii = 0 ; ii < FALCON_PORTS_PER_DP_CNS; ii++)
    {
        if (txPizzaSliceArrPtr[ii] != txFIFOPizzaSliceArry[ii])
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "port number %d have different slice number in txdma %d, txfifo %d\n", ii, txPizzaSliceArrPtr[ii],txFIFOPizzaSliceArry[ii]);
        }
    }
    /* cpu port check*/
    regAddrTxDMA = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum,txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[PRV_DIP6_TX_CPU_PORT_SLICE_NUM_CNS];
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxDMA, 31, 1, &txDmaValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    txPizzaSliceArrPtr[8] = txDmaValue;

    regAddrTxFIFO = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[PRV_DIP6_TX_CPU_PORT_SLICE_NUM_CNS];
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxFIFO, 31, 1, &txFifoValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (txPizzaSliceArrPtr[8] != txFifoValue)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "port number %d have different slice number in txdma %d, txfifo %d\n",8,txPizzaSliceArrPtr[8],txFifoValue);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChTxCpuPizzaGet function
* @endinternal
*
* @brief   Get Pizza arbiter slices for cpu port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
* @param[in] txIndex                - global Tx index
* @param[out] cpuSliceNumber        - (pointer to) - (pointer to) amount of
*       slices for CPU port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
**/
GT_STATUS prvCpssDxChTxCpuPizzaGet
(
    IN   GT_U8       devNum,
    IN   GT_U32      txIndex,
    OUT  GT_U32      *cpuSliceNumber
)
{
    GT_STATUS                           rc;
    GT_U32                              ii;
    GT_U32                              regAddrTxDMA;
    GT_U32                              regAddrTxFIFO;
    GT_U32                              txDmaValue; /* get the valid bit and the port number, the port number use as index for txPizzaSliceArrPtr*/
    GT_U32                              txFifoValue;/* get the valid bit and the port number, the port number use as index for txFIFOPizzaSliceArry*/
    GT_U32                              txFifoCpuSlicesNum;

    *cpuSliceNumber = 0;
    txFifoCpuSlicesNum = 0;
    /* txdma*/
    for (ii = 0 ; ii< TXD_PIZZA_SLICE_NUM; ii++)
    {
        regAddrTxDMA = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[ii];
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxDMA, 31, 1, &txDmaValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (txDmaValue == 0)
        {
            /* the slice is not valid*/
            continue;
        }
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxDMA, 0, 4, &txDmaValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* the slice belong to regular port */
        if (txDmaValue != PRV_DIP6_TX_CPU_PORT_NUM_CNS(devNum))
        {
            continue;
        }
        if (ii != PRV_DIP6_TX_CPU_PORT_SLICE_NUM_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " cpu port %d found on slice %d \n",PRV_DIP6_TX_CPU_PORT_NUM_CNS(devNum),ii);
        }
        (*cpuSliceNumber)++;
    }

    /*txfifo*/
    for (ii = 0 ; ii< TXD_PIZZA_SLICE_NUM; ii++)
    {
        regAddrTxFIFO = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[ii];
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxFIFO, 31, 1, &txFifoValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (txFifoValue == 0)
        {
            /* the slice is not valid*/
            continue;
        }
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxFIFO, 0, 4, &txFifoValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* the slice belong to regular port */
        if (txFifoValue != PRV_DIP6_TX_CPU_PORT_NUM_CNS(devNum))
        {
            continue;
        }
        if (ii != PRV_DIP6_TX_CPU_PORT_SLICE_NUM_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " cpu port %d found on slice %d \n",PRV_DIP6_TX_CPU_PORT_NUM_CNS(devNum),ii);
        }
        txFifoCpuSlicesNum++;
    }
    if ((*cpuSliceNumber)!= txFifoCpuSlicesNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "port number %d have different slice number in txdma %d, txfifo %d\n",PRV_DIP6_TX_CPU_PORT_NUM_CNS(devNum),(*cpuSliceNumber),txFifoCpuSlicesNum);
    }
    return GT_OK;
}
/**
* @internal prvCpssDxChTxDPPizzaSliceBitMapGet function
* @endinternal
*
* @brief   Gets ports pizza slices bitmap for specified dp.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number.
* @param[in] txIndex                - tx global index.
*
* @param[out] txPizzaSliceArrBitMapPtr     - (pointer to) slices
*                                     bit map array in size
*                                     FALCON_PORTS_PER_DP_CNS
*                                     (8). each node should hold
*                                     bit map of the port slices
*                                     in the pizza.
*                                     0: the slice not belong to
*                                     the port
*                                     1: the slice belong to the
*                                     port
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxDPPizzaSliceBitMapGet
(
    IN   GT_U8                       devNum,
    IN   GT_U32                      txIndex,
    OUT  GT_U32                      *txPizzaSliceArrBitMapPtr
)
{
    GT_STATUS                           rc;
    GT_U32                              regAddrTxDMA;
    GT_U32                              regAddrTxFIFO;
    GT_U32                              txDmaValue; /* get the valid bit and the port number, the port number use as index for txPizzaSliceArrPtr*/
    GT_U32                              txFifoValue;/* get the valid bit and the port number, the port number use as index for txFIFOPizzaSliceArry*/
    GT_U32                              portIndex, ii;
    GT_U32                              txFIFOPizzaSliceArry[FALCON_PORTS_PER_DP_CNS] = {0};

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(txPizzaSliceArrBitMapPtr);

    /*reset the array*/
    for (portIndex = 0 ; portIndex < FALCON_PORTS_PER_DP_CNS ; portIndex++)
    {
        txPizzaSliceArrBitMapPtr[portIndex] = 0;
    }

    /*txDMA*/
    /* go over pizza slices except cpu slice, 33 */
    for (ii = 0 ; ii < (TXD_PIZZA_SLICE_NUM - 1) ; ii++ )
    {
        regAddrTxDMA = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum,txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[ii];
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxDMA, 31, 1, &txDmaValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (txDmaValue == 0)
        {
            /* the slice is not valid*/
            continue;
        }
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxDMA, 0, 4, &txDmaValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (txDmaValue >= FALCON_PORTS_PER_DP_CNS )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "the slice's port number %d is higher than %d\n", txDmaValue, FALCON_PORTS_PER_DP_CNS);
        }
        txPizzaSliceArrBitMapPtr[txDmaValue] |= (0x1 << ii);
    }

    /*txFIFO*/
    /* go over pizza slices except cpu slice, 33 */
    for (ii = 0 ; ii < (TXF_PIZZA_SLICE_NUM - 1 ) ; ii++ )
    {
        regAddrTxFIFO = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[ii];
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxFIFO, 31, 1, &txFifoValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (txFifoValue == 0)
        {
            /* the slice is not valid*/
            continue;
        }
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxFIFO, 0, 4, &txFifoValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (txFifoValue >= FALCON_PORTS_PER_DP_CNS )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "the slice's port number %d is higher than %d", txFifoValue, FALCON_PORTS_PER_DP_CNS);
        }
        txFIFOPizzaSliceArry[txFifoValue] |= (0x1 << ii);
    }

    /* the txDMA pizza and txFIFO pizza have to the same*/
    for (ii = 0 ; ii < FALCON_PORTS_PER_DP_CNS; ii++)
    {
        if (txPizzaSliceArrBitMapPtr[ii] != txFIFOPizzaSliceArry[ii])
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "port number %d have different slice number in txdma %d, txfifo %d\n", ii, txPizzaSliceArrBitMapPtr[ii],txFIFOPizzaSliceArry[ii]);
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChPortTxPizzaSizeGet function
* @endinternal
*
* @brief   Get Pizza arbiter maximum slice number
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
* @param[in] txIndex                - global Tx index
*
* @param[out] txPizzaSliceNumPtr   - (pointer to) the number of
*       max pizza slices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortTxPizzaSizeGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      txIndex,
    OUT  GT_U32     *txPizzaSliceNumPtr
)
{
    GT_STATUS                            rc;
    GT_U32                               regAddrTxFIFO;
    GT_U32                               regAddrTxDMA;
    GT_U32                               txDmaValue;
    GT_U32                               txFifoValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E |
      CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(txPizzaSliceNumPtr);


    regAddrTxDMA = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg;
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxDMA, 0, 7, &txDmaValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddrTxFIFO = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, txIndex).configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg;
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxFIFO, 0, 7, &txFifoValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (txDmaValue != txFifoValue)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }

    /* the field contains maximal slot number, not number of slots */
    *txPizzaSliceNumPtr = (txDmaValue + 1);

    return rc;
}

/**
* @internal prvCpssDxChTxDmaChannelReset function
* @endinternal
*
* @brief  Resets Packet Buffer counters in the TxDma unit for specified port on specified device in falcon.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number.
* @param[in] txIndex                - global tx index (0..32)
* @param[in] localChannelIndex       - local channel index
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChTxDmaChannelReset
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       txIndex,
    IN  GT_U32                       localChannelIndex
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr &&
       PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->isWaNeeded_channel_reset_part == GT_TRUE)
    {
        /* we must not reset the channel (not on delete port and not on create port) */
        return GT_OK;
    }

    /*clear counter done by de assert and assert. */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, txIndex).configs.channelConfigs.channelReset[localChannelIndex];
    if ((txIndex == 0) && (localChannelIndex == PRV_DIP6_TX_CPU_PORT_NUM_CNS(devNum)))
    {
        /* the register DB for DP0 indexed by global port - the CPU ports are */
        /* the last local in DPs and placed at the end of the array           */
        /* The Formula DB[0][localChannel] for 0-th CPU port is wrong         */
        GT_U32      globalDmaNum;
        rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
            devNum, txIndex, localChannelIndex, &globalDmaNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, 0).configs.channelConfigs.channelReset[globalDmaNum];
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, 0);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, 1);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, 0);

    return rc;
}

/**
* @internal prvCpssDxChTxDmaSpeedProfileConfigSet function
* @endinternal
*
* @brief   Sets speed profile for specified port on specified
*          device in falcon.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number.
* @param[in] portSpeed              - port speed
* @param[in] speedProfile           - port speed profile
* @param[in] txIndex                - tx global index
* @param[in] localChannelIndex      -  local channel index
*
* GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxDmaSpeedProfileConfigSet
(
    IN  GT_U8                        devNum,
    IN  CPSS_PORT_SPEED_ENT          portSpeed,
    IN  PRV_CPSS_SPEED_PROFILE_ENT   speedProfile,
    IN  GT_U32                       txIndex,
    IN  GT_U32                       localChannelIndex
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;
    GT_U32 fifoDepth=0, sdqMaxCredits=0;
    GT_U32 sdqThresholdBytes=0, maxWordCredits=0;
    GT_U32 maxCellsCredits=0, maxDescCredits=0;
    GT_U32 interCellRateLimiter=0, interPacketRateLimiter=0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E |
      CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_AC3X_E);
    if (speedProfile >= PRV_CPSS_PORT_SPEED_PROFILE_NA_E )
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum,txIndex).configs.channelConfigs.speedProfile[localChannelIndex];
    if ((txIndex == 0) && (localChannelIndex == PRV_DIP6_TX_CPU_PORT_NUM_CNS(devNum)))
    {
        /* the register DB for DP0 indexed by global port - the CPU ports are */
        /* the last local in DPs and placed at the end of the array           */
        /* The Formula DB[0][localChannel] for 0-th CPU port is wrong         */
        GT_U32      globalDmaNum;
        rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
            devNum, txIndex, localChannelIndex, &globalDmaNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, 0).configs.channelConfigs.speedProfile[globalDmaNum];
    }
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 4, speedProfile);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch (portSpeed)
    {
        case CPSS_PORT_SPEED_42000_E:
            fifoDepth = 2;
            sdqMaxCredits = 6;
            sdqThresholdBytes = 504;
            maxWordCredits = 147;
            maxCellsCredits = 2;
            maxDescCredits = 2;
            interCellRateLimiter = 640;
            interPacketRateLimiter = 1680;
            break;
        case CPSS_PORT_SPEED_47200_E:
            fifoDepth = 2;
            sdqMaxCredits = 6;
            sdqThresholdBytes = 504;
            maxWordCredits = 147;
            maxCellsCredits = 2;
            maxDescCredits = 2;
            interCellRateLimiter = 614;
            interPacketRateLimiter = 1920;
            break;
        case CPSS_PORT_SPEED_102G_E:
            /* use 100G profile and update rate limiter to be 102G */
            fifoDepth = 2;
            sdqMaxCredits = 10;
            sdqThresholdBytes = 840;
            maxWordCredits = 284;
            maxCellsCredits = 2;
            maxDescCredits = 2;
            interCellRateLimiter = 1310;
            interPacketRateLimiter = 4092;
            break;
        case CPSS_PORT_SPEED_106G_E:
            fifoDepth = 4;
            sdqMaxCredits = 19;
            sdqThresholdBytes = 1596;
            maxWordCredits = 558;
            maxCellsCredits = 4;
            maxDescCredits = 4;
            interCellRateLimiter = 1360;
            interPacketRateLimiter = 4240;
            break;
        case CPSS_PORT_SPEED_53000_E:
            fifoDepth = 2;
            sdqMaxCredits = 10;
            sdqThresholdBytes = 840;
            maxWordCredits = 284;
            maxCellsCredits = 2;
            maxDescCredits = 2;
            interCellRateLimiter = 680;
            interPacketRateLimiter = 2120;
            break;
        case CPSS_PORT_SPEED_424G_E:
            fifoDepth = 8;
            sdqMaxCredits = 36;
            sdqThresholdBytes = 3024;
            maxWordCredits = 1110;
            maxCellsCredits = 8;
            maxDescCredits = 8;
            interCellRateLimiter = 5440;
            interPacketRateLimiter = 16960;
            break;
        default:
            return GT_OK; /* profile configuration not needed */
            break;
    }

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum,txIndex).configs.speedProfileConfigs.descFIFODepth[speedProfile];
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 6, fifoDepth*7);
    if(rc != GT_OK)
    {
        return rc;
    }
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum,txIndex).configs.speedProfileConfigs.sdqMaxCredits[speedProfile];
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 6, sdqMaxCredits);
    if(rc != GT_OK)
    {
        return rc;
    }
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum,txIndex).configs.speedProfileConfigs.sdqThresholdBytes[speedProfile];
    /* SDQ min bytes */
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 12, sdqThresholdBytes);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* SDQ max bytes */
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 16, 15 , sdqThresholdBytes+20000);
    if(rc != GT_OK)
    {
        return rc;
    }
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum,txIndex).configs.speedProfileConfigs.maxWordCredits[speedProfile];
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 11, maxWordCredits);
    if(rc != GT_OK)
    {
        return rc;
    }
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum,txIndex).configs.speedProfileConfigs.maxCellsCredits[speedProfile];
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 10, maxCellsCredits*69);
    if(rc != GT_OK)
    {
        return rc;
    }
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum,txIndex).configs.speedProfileConfigs.maxDescCredits[speedProfile];
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 9, maxDescCredits*36);
    if(rc != GT_OK)
    {
        return rc;
    }
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum,txIndex).configs.speedProfileConfigs.interCellRateLimiter[speedProfile];
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 16, 16, interCellRateLimiter);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum,txIndex).configs.speedProfileConfigs.interPacketRateLimiter[speedProfile];
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 16, 16, interPacketRateLimiter);

    return rc;
}

/**
* @internal prvCpssDxChTxFifoSpeedProfileConfigSet function
* @endinternal
*
* @brief   Sets speed profile for specified port on specified
*          device in falcon.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number.
* @param[in] speedProfile           - port speed profile.
* @param[in] txIndex                - tx global index.
* @param[in] localChannelIndex      - local channel index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxFifoSpeedProfileConfigSet
(
    IN  GT_U8                        devNum,
    IN  PRV_CPSS_SPEED_PROFILE_ENT   speedProfile,
    IN  GT_U32                       txIndex,
    IN  GT_U32                       localChannelIndex
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E |
      CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_AC3X_E);

    if (speedProfile >= PRV_CPSS_PORT_SPEED_PROFILE_NA_E)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, txIndex).configs.channelConfigs.speedProfile[localChannelIndex];
    if ((txIndex == 0) && (localChannelIndex == PRV_DIP6_TX_CPU_PORT_NUM_CNS(devNum)))
    {
        /* the register DB for DP0 indexed by global port - the CPU ports are */
        /* the last local in DPs and placed at the end of the array           */
        /* The Formula DB[0][localChannel] for 0-th CPU port is wrong         */
        GT_U32      globalDmaNum;
        rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
            devNum, txIndex, localChannelIndex, &globalDmaNum);
        if (rc != GT_OK)
        {
            return rc;
        }
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, 0).configs.channelConfigs.speedProfile[globalDmaNum];
    }

    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 4, speedProfile);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;

}

/**
* @internal prvCpssDxChSpeedProfileTxDmaFromPortSpeedGet function
* @endinternal
*
* @brief   Gets speed profile for specified port speed on specified device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] speed                  - port speed
*
* @param[out] speedProfilePtr       - (pointer to) speed
*       profile.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChSpeedProfileTxDmaFromPortSpeedGet
(
    IN  CPSS_PORT_SPEED_ENT         speed,
    OUT PRV_CPSS_SPEED_PROFILE_ENT  *speedProfilePtr
)
{
    switch ( speed )
    {
        case CPSS_PORT_SPEED_400G_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_0_E;
            break;
        case CPSS_PORT_SPEED_200G_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_1_E;
            break;
        case CPSS_PORT_SPEED_100G_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_2_E;
            break;
        case CPSS_PORT_SPEED_50000_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_3_E;
            break;
        case CPSS_PORT_SPEED_40000_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_4_E;
            break;
        case CPSS_PORT_SPEED_25000_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_5_E;
            break;
        case CPSS_PORT_SPEED_20000_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_6_E;
            break;
        case CPSS_PORT_SPEED_10000_E:
        case CPSS_PORT_SPEED_5000_E:
        case CPSS_PORT_SPEED_2500_E:
        case CPSS_PORT_SPEED_1000_E:
        case CPSS_PORT_SPEED_100_E:
        case CPSS_PORT_SPEED_10_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_7_E;
            break;
        case CPSS_PORT_SPEED_424G_E:
            /* this speed applicable for Falcon only  */
            /* and uses all channels of datapath      */
            /* profile will be configured for it only */
            /* and other ports will not be affected   */
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_9_E;
            break;
        case CPSS_PORT_SPEED_42000_E:
        case CPSS_PORT_SPEED_47200_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_9_E;
            break;
        case CPSS_PORT_SPEED_53000_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_10_E;
            break;
        case CPSS_PORT_SPEED_102G_E:
        case CPSS_PORT_SPEED_106G_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_11_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"prvCpssDxChGetSpeedProfileFromPortSpeed: port speed %d not supported \n",speed);
            break;
    }
    return GT_OK;

}

/**
* @internal prvCpssDxChSpeedProfileTxFifoFromPortSpeedGet function
* @endinternal
*
* @brief   Gets speed profile for specified port speed on specified device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] speed                  - port speed
*
* @param[out] speedProfilePtr       - (pointer to) speed
*       profile.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChSpeedProfileTxFifoFromPortSpeedGet
(
    IN  CPSS_PORT_SPEED_ENT         speed,
    OUT PRV_CPSS_SPEED_PROFILE_ENT  *speedProfilePtr
)
{
    switch ( speed )
    {
        case CPSS_PORT_SPEED_400G_E:
        case CPSS_PORT_SPEED_424G_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_0_E;
            break;
        case CPSS_PORT_SPEED_200G_E:
        case CPSS_PORT_SPEED_106G_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_1_E;
            break;
        case CPSS_PORT_SPEED_100G_E:
        case CPSS_PORT_SPEED_102G_E:
        case CPSS_PORT_SPEED_53000_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_2_E;
            break;
        case CPSS_PORT_SPEED_50000_E:
        case CPSS_PORT_SPEED_42000_E:
        case CPSS_PORT_SPEED_47200_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_3_E;
            break;
        case CPSS_PORT_SPEED_40000_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_4_E;
            break;
        case CPSS_PORT_SPEED_25000_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_5_E;
            break;
        case CPSS_PORT_SPEED_20000_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_6_E;
            break;
        case CPSS_PORT_SPEED_10000_E:
        case CPSS_PORT_SPEED_5000_E:
        case CPSS_PORT_SPEED_2500_E:
        case CPSS_PORT_SPEED_1000_E:
        case CPSS_PORT_SPEED_100_E:
        case CPSS_PORT_SPEED_10_E:
            *speedProfilePtr = PRV_CPSS_SPEED_PROFILE_7_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"prvCpssDxChSpeedProfileTxFifoFromPortSpeedGet: port speed %d not supported \n",speed);
            break;
    }
    return GT_OK;

}


/**
* @internal prvCpssDxChPortTxSpeedProfileGet function
* @endinternal
*
* @brief   Gets speed profile for specified port speed on specified device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number.
* @param[in] txIndex                - tx global index.
*
* @param[out] txSpeedProfilePtr     - (pointer to) speed profile array in size FALCON_PORTS_PER_DP_CNS (8).
*                                     each node should hold the number of his speedProfile
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortTxSpeedProfileGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      txIndex,
    OUT PRV_CPSS_SPEED_PROFILE_ENT  *txSpeedProfilePtr
)
{
    GT_STATUS                            rc;
    GT_U32                               regAddrTxDMA;
    GT_U32                               regAddrTxFIFO;
    GT_U32                               txDmaValue;
    GT_U32                               txFifoValue;
    GT_U32                               ii;
    PRV_CPSS_SPEED_PROFILE_ENT           txFIFOSpeedProfileArry[FALCON_PORTS_PER_DP_CNS+1];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E |
      CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(txSpeedProfilePtr);


    /*txDMA*/
    for (ii = 0 ; ii <= FALCON_PORTS_PER_DP_CNS ; ii++ )
    {
        regAddrTxDMA = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, txIndex).configs.channelConfigs.speedProfile[ii];
        if ((txIndex == 0) && (ii == PRV_DIP6_TX_CPU_PORT_NUM_CNS(devNum)))
        {
            /* the register DB for DP0 indexed by global port - the CPU ports are */
            /* the last local in DPs and placed at the end of the array           */
            /* The Formula DB[0][localChannel] for 0-th CPU port is wrong         */
            GT_U32      globalDmaNum;
            rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
                devNum, txIndex, ii, &globalDmaNum);
            if (rc != GT_OK)
            {
                return rc;
            }
            regAddrTxDMA = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, 0).configs.channelConfigs.speedProfile[globalDmaNum];
        }
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxDMA, 0,4 , &txDmaValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (txDmaValue > PRV_CPSS_SPEED_PROFILE_7_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "speed profile %d is not supported\n",txDmaValue);
        }
        txSpeedProfilePtr[ii] = txDmaValue;
    }

    /*txFIFO*/
    for (ii = 0 ; ii <= FALCON_PORTS_PER_DP_CNS ; ii++ )
    {
        regAddrTxFIFO = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, txIndex).configs.channelConfigs.speedProfile[ii];
        if ((txIndex == 0) && (ii == PRV_DIP6_TX_CPU_PORT_NUM_CNS(devNum)))
        {
            /* the register DB for DP0 indexed by global port - the CPU ports are */
            /* the last local in DPs and placed at the end of the array           */
            /* The Formula DB[0][localChannel] for 0-th CPU port is wrong         */
            GT_U32      globalDmaNum;
            rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
                devNum, txIndex, ii, &globalDmaNum);
            if (rc != GT_OK)
            {
                return rc;
            }
            regAddrTxFIFO = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, 0).configs.channelConfigs.speedProfile[globalDmaNum];
        }
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxFIFO, 0,4 , &txFifoValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (txFifoValue > PRV_CPSS_SPEED_PROFILE_7_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "speed profile %d is not supported\n",txFifoValue);
        }
        txFIFOSpeedProfileArry[ii] = txFifoValue;
    }

    /* txDMA and txFIFO speed profile configurations have to the identical*/
    for (ii = 0 ; ii <= FALCON_PORTS_PER_DP_CNS ; ii++ )
    {
        if (txSpeedProfilePtr[ii] != txFIFOSpeedProfileArry[ii])
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "speed profile in local channel %d is %d in txdma and %d in txfifo. speed profile have to be the same in txdma, txfifo\n",ii,txSpeedProfilePtr[ii],txFIFOSpeedProfileArry[ii] );;
        }
    }
    return GT_OK;
}

/* debug mode to allow slices according to speed and not according to SERDESes */
GT_STATUS debug_falcon_sliceMode_set(GT_U32 debug_mode)
{
    PRV_SHARED_PORT_PRV_DB_VAR(debug_falcon_sliceMode) = debug_mode;
    return GT_OK;
}

/* debug function to allow slices according to speed and not according to SERDESes */
static GT_STATUS  debug_falcon_sliceMode_prvCpssDxChPortFalconSlicesFromPortSpeedGet
(
    IN   GT_U8                        devNum,
    IN   CPSS_PORT_SPEED_ENT          portSpeed,
    OUT  GT_U32                        *numOfSlicesPtr
)
{
    GT_U32               numOfSlices;

    devNum = devNum;/* not used */

    switch (portSpeed)
    {
        default:
            numOfSlices = 1;
            break;
        case CPSS_PORT_SPEED_53000_E:
        case CPSS_PORT_SPEED_100G_E:
        case CPSS_PORT_SPEED_102G_E:
            numOfSlices = 2;
            break;
        case CPSS_PORT_SPEED_106G_E:
        case CPSS_PORT_SPEED_200G_E:
            numOfSlices = 4;
            break;
        case CPSS_PORT_SPEED_400G_E:
        case CPSS_PORT_SPEED_424G_E:
            numOfSlices = 8;
            break;
    }

    *numOfSlicesPtr = numOfSlices;

    return GT_OK;
}

/**
* @internal prvCpssDxChPortFalconSlicesFromPortSpeedGet function
* @endinternal
*
* @brief     function to get number of slices according to port speed and
*            interface that implies the number of SERDESes needed for the port.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
*
* @param[in] devNum                 - physical device number.
* @param[in] portNum                - physical port number.
* @param[in] ifMode                 - cpss interface mode.
* @param[in] portSpeed              - cpss port speed.
*
* @param[out] activeLanesListPtr     - (pointer to) active lanes
* @param[out] numOfActLanesPtr     - (pointer to) num of active lanes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS  prvCpssDxChPortFalconSlicesFromPortSpeedGet
(
    IN   GT_U8                        devNum,
    IN   GT_PHYSICAL_PORT_NUM         portNum,
    IN   CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    IN   CPSS_PORT_SPEED_ENT          portSpeed,
    OUT  GT_U32                        *numOfSlicesPtr
)
{
    GT_STATUS               rc;
    MV_HWS_PORT_STANDARD    portMode; /* HWS port mode */
    GT_U32                  portMacNum;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if (PRV_SHARED_PORT_PRV_DB_VAR(debug_falcon_sliceMode))
    {
        /* debug mode to allow slices according to speed and not according to SERDESes */
        return debug_falcon_sliceMode_prvCpssDxChPortFalconSlicesFromPortSpeedGet(devNum,portSpeed,numOfSlicesPtr);
    }

    /*convert port number to mac port*/
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* translate cpss ifMode and speed to single Hws port mode */
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode,
                                            portSpeed, &portMode);

    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssCommonPortIfModeToHwsTranslate\n" );
    }

    /* get hws parameters for a given Hws port mode */
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0 /* port group*/, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "hwsPortModeParamsGet returned null " );
    }

    if (0 == curPortParams.numOfActLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "port %d has no serdeses",portNum );
    }

    *numOfSlicesPtr = curPortParams.numOfActLanes;

    return GT_OK;
}



/**
* @internal prvCpssDxChTxDmaPortSpeedSet function
* @endinternal
*
* @brief  Sets speed for specified port on specified device in falcon.
*       do the following steps:
*       1. config pizza
*       2. reset PB counters
*       3. config speed profile
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number.
* @param[in] txDmaIndex            - tx global index.
* @param[in] localChannelIndex     - local channel index.
* @param[in] portSpeed             - port Speed.
* @param[in] speedProfile          - speed Profile.
* @param[in] portPizzaSlicesBitMap - bit map represent the slices should be associated to the port
* @param[in] enable                - config new speed or delete current speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChTxDmaPortSpeedSet
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       txDmaIndex,
    IN  GT_U32                       localChannelIndex,
    IN  CPSS_PORT_SPEED_ENT          portSpeed,
    IN  PRV_CPSS_SPEED_PROFILE_ENT   speedProfile,
    IN  GT_U32                       pizzaSlicesBitMap,
    IN  GT_BOOL                      enable
)
{
    GT_STATUS rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
    /* to change port speed need to set the following in this order:
     1. set new pizza configuration
     2. reset PB counters
     3. set new speed profile
       */

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* HAWK - TODO */
        return GT_OK;
    }
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        rc = prvCpssDxChTxPizzaConfigurationSet(devNum, txDmaIndex, localChannelIndex, pizzaSlicesBitMap, enable, PRV_CPSS_DXCH_UNIT_TXDMA_E, GT_FALSE /* is cpu port*/);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxPizzaConfigurationSet failed for TXDMA number %d", txDmaIndex );
        }
    }

    rc = prvCpssDxChTxDmaChannelReset(devNum, txDmaIndex, localChannelIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxDmaTDSChannelReset failed for TXDMA number %d", txDmaIndex );
    }

    if (GT_TRUE == enable)
    {
        rc = prvCpssDxChTxDmaSpeedProfileConfigSet(devNum, portSpeed, speedProfile, txDmaIndex, localChannelIndex);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxDmaSpeedProfileConfigSet failed for TXDMA number %d", txDmaIndex );
        }
    }

    return rc;

}

/**
* @internal prvCpssDxChTxFifoPortSpeedSet function
* @endinternal
*
* @brief  Sets speed for specified port on specified device in falcon.
*       do the following steps:
*       1. config pizza
*       2. config speed profile
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number.
* @param[in] txFifoIndex            - tx global index.
* @param[in] localChannelIndex     - local channel index.
* @param[in] speedProfile          - speed Profile.
* @param[in] portPizzaSlicesBitMap - bit map represent the slices should be associated to the port
* @param[in] enable                - config new speed or delete current speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChTxFifoPortSpeedSet
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       txFifoIndex,
    IN  GT_U32                       localChannelIndex,
    IN  PRV_CPSS_SPEED_PROFILE_ENT   speedProfile,
    IN  GT_U32                       portPizzaSlicesBitMap,
    IN  GT_BOOL                      enable
)
{

    GT_STATUS   rc = GT_OK;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* HAWK - TODO */
        return GT_OK;
    }
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        rc = prvCpssDxChTxPizzaConfigurationSet(devNum, txFifoIndex, localChannelIndex, portPizzaSlicesBitMap, enable, PRV_CPSS_DXCH_UNIT_TX_FIFO_E, GT_FALSE /* is cpu port*/);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxPizzaConfigurationSet failed for TXFIFO number %d", txFifoIndex);
        }
    }

    if (GT_TRUE == enable)
    {
        rc = prvCpssDxChTxFifoSpeedProfileConfigSet(devNum, speedProfile, txFifoIndex, localChannelIndex);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxFifoSpeedProfileConfigSet failed for TXFIFO number %d", txFifoIndex);
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChGetTxIndexAndChannelIndexfromPortNum function
* @endinternal
*
* @brief   Gets speed profile for specified port speed on specified device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
*
* @param[out] txDmaIndexPtr       - (pointer to)  Tx global
*       index (0..31)
* @param[out] portGroupPtr        - (pointer to)  channel local
*      index in DP (0..7)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChGetTxIndexAndChannelIndexfromPortNum
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    OUT GT_U32                       *txDmaIndexPtr,
    OUT GT_U32                       *channelIndexPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_SHADOW_PORT_MAP_STC detailedPortMap;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E |
      CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_AC3X_E);

    if (portNum > PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortPhysicalPortMapGet(
        devNum, portNum,/*OUT*/&detailedPortMap);
    if (GT_OK != rc )
    {
        return rc;
    }

    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum, detailedPortMap.txDmaNum, txDmaIndexPtr, channelIndexPtr);

    return rc;
}

/**
* @internal prvCpssDxChTxCpuPortSpeedSet function
* @endinternal
*
* @brief    set cpu configuration for falcon Tx unit.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
* @param[in] txDmaIndex             - Tx global index
* @param[in] channelIndex           - local channel Index
* @param[in] enable                        - set new resorce and pizza or delete pizza
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChTxCpuPortSpeedSet
(
    IN GT_U8                         devNum,
    IN GT_U32                        txGlobalIndex,
    IN GT_U32                        channelIndex,
    IN  CPSS_PORT_SPEED_ENT          portSpeed,
    IN  GT_BOOL                      enable
)
{
    GT_STATUS                  rc;
    GT_U32                     portPizzaSlices = 0;
    PRV_CPSS_SPEED_PROFILE_ENT speedProfile;
    GT_U32                    tileId;
    GT_U32                    localDpId;
    GT_U32                    tcBmp;

    /* pizza #32 slice associated with the cpu port */
    rc = prvCpssDxChTxPizzaConfigurationSet(devNum, txGlobalIndex, channelIndex, portPizzaSlices, GT_TRUE, PRV_CPSS_DXCH_UNIT_TXDMA_E, GT_TRUE /* is cpu port*/);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChTxPizzaConfigurationSet failed" );
    }
    rc = prvCpssDxChTxPizzaConfigurationSet(devNum, txGlobalIndex, channelIndex, portPizzaSlices, GT_TRUE, PRV_CPSS_DXCH_UNIT_TX_FIFO_E, GT_TRUE /* is cpu port*/);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChTxPizzaConfigurationSet failed" );
    }

      /*TxQ*/

    rc = prvCpssFalconGlobalDpToTileAndLocalDp(devNum,txGlobalIndex,&localDpId,&tileId);
    if (rc != GT_OK)
    {
         return rc;
    }

    rc  = prvCpssDxChTxQFalconPizzaConfigurationSet(devNum, tileId,localDpId, channelIndex, portPizzaSlices, enable,GT_TRUE);
    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChTxQFalconPizzaConfigurationSet failed");
    }


    /*check maybe PFCC table should be updated*/
    rc = prvCpssFalconTxqPfcValidGet(devNum, tileId,localDpId, channelIndex,&tcBmp);
    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssFalconTxqPfcValidGet failed");
    }
    if(tcBmp!=0)/*Mean that PFC was configured*/
    {
        rc = prvCpssFalconTxqPffcTableExplicitSet(devNum, tileId,localDpId, channelIndex,portPizzaSlices,enable?tcBmp:0);
        if (rc != GT_OK)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssFalconTxqPffcTableExplicitSet failed");
        }
    }

    /*speed profile*/
    /* get port TX DMA profile according to port speed*/
    rc = prvCpssDxChSpeedProfileTxDmaFromPortSpeedGet(portSpeed, &speedProfile);
    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChSpeedProfileTxDmaFromPortSpeedGet failed");
    }

    rc = prvCpssDxChTxDmaSpeedProfileConfigSet(devNum, portSpeed, speedProfile, txGlobalIndex, channelIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChTxFifoSpeedProfileConfigSet failed for TXFIFO number %d", txGlobalIndex );
    }

    /* get port TX FIFO profile according to port speed */
    rc = prvCpssDxChSpeedProfileTxFifoFromPortSpeedGet(portSpeed, &speedProfile);
    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChSpeedProfileTxDmaFromPortSpeedGet failed");
    }

    rc = prvCpssDxChTxFifoSpeedProfileConfigSet(devNum, speedProfile, txGlobalIndex, channelIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChTxFifoSpeedProfileConfigSet failed for TXFIFO number %d", txGlobalIndex );
    }
    return rc;
}

/**
* @internal prvCpssDxChTxPortSpeedProfileAndPizzaSliceGet function
* @endinternal
*
* @brief    Get speed profile and pizza slices.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
* @param[in] ifMode                 - interface mode
* @param[in] portSpeed              - port speed
* @param[in] portGroup              - port group
*
* @param[out] portSlicesNumPtr      - (pointer to) number of slice for the port
* @param[out] portSpeedProfilePtr   - (pointer to) the port speed profile
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxPortSpeedProfileAndPizzaSliceGet
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    IN  CPSS_PORT_SPEED_ENT          portSpeed,
    OUT  GT_U32                      *portSlicesNumPtr,
    OUT  PRV_CPSS_SPEED_PROFILE_ENT  *portSpeedProfilePtr
)
{
    GT_STATUS rc;
    GT_U32     numOfSlices;   /* (per 8 slices) number of slices for port speed configuration*/
    GT_U32    pizzaSliceBitMap;    /* ports slices */
    GT_U32    txGlobalIndex; /*txdma , txfifo index */
    GT_U32    channelDpIndex;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E |
      CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(portSlicesNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(portSpeedProfilePtr);

    cpssOsMemSet(&pizzaSliceBitMap,0,sizeof(GT_U32));

    /*get txindex, channel index*/
    rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(devNum, portNum, &txGlobalIndex, &channelDpIndex );
    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChGetTxDmaIndexAndChannelIndexfromPortNum faild");
    }

    /* check if its cpu port */
    if (channelDpIndex == PRV_DIP6_TX_CPU_PORT_NUM_CNS(devNum))
    {
        *portSlicesNumPtr = 1;
    }
    else
    {
        /* get port serdes */
        rc = prvCpssDxChPortFalconSlicesFromPortSpeedGet(devNum, portNum , ifMode , portSpeed, &numOfSlices);
        if (rc != GT_OK)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChSerdesFromPortSpeedGet faild");
        }
        *portSlicesNumPtr = numOfSlices*4 ;/* each slice 'per 8 slices' have 4 slices */
    }
    /* get port profile according to port speed*/
    rc = prvCpssDxChSpeedProfileTxDmaFromPortSpeedGet(portSpeed, portSpeedProfilePtr);
    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChGetSpeedProfileFromPortSpeed faild");
    }
    return rc;
}



/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesSet function
* @endinternal
*
* @brief    Sets speed for specified port on specified device in falcon pizza arbiter .
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - pysical port number
* @param[in] ifMode                 - interface mode
* @param[in] portSpeed              - port speed
* @param[in] enable                 - set new resorce and pizza or delete pizza
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesSet
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    IN  CPSS_PORT_SPEED_ENT          portSpeed,
    IN  GT_BOOL                      enable
)
{
    GT_STATUS                 rc;
    PRV_CPSS_SPEED_PROFILE_ENT speedProfile;             /* Tx speed profile*/
    GT_U32                     numOfSlicesPer8Slices;     /* (per 8 slices) num on slices  for port speed configuration*/
    GT_U32                    pizzaSliceBitMap;          /* ports slices bit map */
    GT_U32                    txGlobalIndex;             /* DP index (0- 31)*/
    GT_U32                    channelDpIndex;            /* channel index (0 ..7)*/
    GT_U32                    tileId;
    GT_U32                    localDpId;
    GT_U32                    tcBmp;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E |
    CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_AC3X_E);

    cpssOsMemSet(&pizzaSliceBitMap,0,sizeof(GT_U32));

    /*get txindex, and channel index */
    rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(devNum, portNum, &txGlobalIndex, &channelDpIndex );
    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChGetTxDmaIndexAndChannelIndexfromPortNum faild");
    }

    /* check if its cpu port */
    if (channelDpIndex == PRV_DIP6_TX_CPU_PORT_NUM_CNS(devNum))
    {
        rc = prvCpssDxChTxCpuPortSpeedSet(devNum, txGlobalIndex, channelDpIndex, portSpeed,enable);
        return rc;
    }

    /* get for the speed the number of slices 'per 8 slices' */
    rc = prvCpssDxChPortFalconSlicesFromPortSpeedGet(devNum, portNum , ifMode , portSpeed, &numOfSlicesPer8Slices);
    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChSerdesFromPortSpeedGet faild\n");
    }

    /* get port needed slices from meta pizza according to port speed */
    rc = prvCpssDxChPizzaSlicesFromSerdesGet(devNum,(GT_U8)numOfSlicesPer8Slices, channelDpIndex, &pizzaSliceBitMap);
    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChPizzaSlicesFromSerdesGet failed");
    }

    /* get port needed TX DMA profile according to port speed*/
    rc = prvCpssDxChSpeedProfileTxDmaFromPortSpeedGet(portSpeed, &speedProfile);
    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChGetSpeedProfileFromPortSpeed failed");
    }

    /* Set TxDMA speed by speed pizza and profile configurations, reset previous counters */
    rc = prvCpssDxChTxDmaPortSpeedSet(devNum, txGlobalIndex, channelDpIndex, portSpeed, speedProfile, pizzaSliceBitMap, enable);
    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChTxPortSpeedSet failed");
    }


    rc = prvCpssSip6TxqUtilsDataPathEvent(devNum, portNum,GT_TRUE== enable?PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXD_RESOURCE_SET:
        PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_TXD_RESOURCE_CLEAR,GT_FALSE);

    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsDataPathEvent  failed for port  %d  ", portNum);
    }

    /* get port needed TX FIFO profile according to port speed*/
    rc = prvCpssDxChSpeedProfileTxFifoFromPortSpeedGet(portSpeed, &speedProfile);
    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChGetSpeedProfileFromPortSpeed failed");
    }

    /* Set TxFIFO speed by speed profile and pizza configurations */
    rc = prvCpssDxChTxFifoPortSpeedSet(devNum, txGlobalIndex, channelDpIndex, speedProfile, pizzaSliceBitMap, enable);
    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChSetTxFIFOportSpeed failed");
    }

    /*TxQ*/

    rc = prvCpssFalconGlobalDpToTileAndLocalDp(devNum,txGlobalIndex,&localDpId,&tileId);
    if (rc != GT_OK)
    {
         return rc;
    }

    rc  = prvCpssDxChTxQFalconPizzaConfigurationSet(devNum, tileId,localDpId, channelDpIndex, pizzaSliceBitMap, enable,GT_FALSE);
    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChTxQFalconPizzaConfigurationSet failed");
    }

    /*check maybe PFCC table should be updated*/
    rc = prvCpssFalconTxqPfcValidGet(devNum, tileId,localDpId, channelDpIndex,&tcBmp);
    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssFalconTxqPfcValidGet failed");
    }
    if(tcBmp!=0)/*Mean that PFC was configured*/
    {
         rc = prvCpssFalconTxqPffcTableExplicitSet(devNum, tileId,localDpId, channelDpIndex,pizzaSliceBitMap,enable?tcBmp:0);
        if (rc != GT_OK)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssFalconTxqPffcTableExplicitSet failed");
        }
    }

   return GT_OK;
}

/**
* @internal prevCpssDxChPortTxSpeedProfileDumpFalconAll function
* @endinternal
*
* @brief    Dumps all related Tx information:
*       go over all DP units and print the speed profile
*       configurations.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prevCpssDxChPortTxSpeedProfileDumpFalconAll
(
    IN  GT_U8 devNum
)
{
    GT_STATUS                            rc = GT_OK;
    GT_U32                               DPCoreNum;
    GT_U32                               ii;
    GT_U32                               jj;
    GT_U32                               regAddrTxDMA;
    GT_U32                               regAddrTxFIFO;
    GT_U32                               profileValue;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E |
    CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_AC3X_E);

    DPCoreNum = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;

    /* go over all DP, each DP has 2 pizza configuration: TxDMA and TxFIFO*/
    for (ii = 0 ; ii < DPCoreNum ; ii++ )
    {

         cpssOsPrintf("+=====================================================================================+\n");
         cpssOsPrintf("|                              Tx Global Index: %d                                    |\n",ii);
         cpssOsPrintf("+=====================================================================================+\n");
         /* tx dma*/
         cpssOsPrintf("\n\n----------------------Tx DMA Pizza ----------------------------------------------------\n");
         cpssOsPrintf("|  channel number    |  speed profile   |\n");
         cpssOsPrintf("|=======================================|\n");
         for (jj = 0 ; jj <= FALCON_PORTS_PER_DP_CNS /*8*/ ; jj++)
         {
             regAddrTxDMA = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, ii).configs.channelConfigs.speedProfile[jj];
             if ((ii == 0) && (jj == PRV_DIP6_TX_CPU_PORT_NUM_CNS(devNum)))
             {
                 /* the register DB for DP0 indexed by global port - the CPU ports are */
                 /* the last local in DPs and placed at the end of the array           */
                 /* The Formula DB[0][localChannel] for 0-th CPU port is wrong         */
                 GT_U32      globalDmaNum;
                 rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
                     devNum, ii, jj, &globalDmaNum);
                 if (rc != GT_OK)
                 {
                     return rc;
                 }
                 regAddrTxDMA = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, 0).configs.channelConfigs.speedProfile[globalDmaNum];
             }
             rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxDMA, 0, 6, &profileValue);
             if (rc != GT_OK)
             {
                 return rc;
             }
             cpssOsPrintf("|       %2d           |       %2d         |\n",jj, profileValue);
         }
         /*tx fifo*/
         cpssOsPrintf("\n\n----------------------Tx FIFO Pizza ----------------------------------------------------\n");
         cpssOsPrintf("|  channel number    |  speed profile   |\n");
         cpssOsPrintf("|=======================================|\n");
         for (jj = 0 ; jj <= FALCON_PORTS_PER_DP_CNS /*8*/ ; jj++)
         {
             regAddrTxFIFO = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, ii).configs.channelConfigs.speedProfile[jj];
             if ((ii == 0) && (jj == PRV_DIP6_TX_CPU_PORT_NUM_CNS(devNum)))
             {
                 /* the register DB for DP0 indexed by global port - the CPU ports are */
                 /* the last local in DPs and placed at the end of the array           */
                 /* The Formula DB[0][localChannel] for 0-th CPU port is wrong         */
                 GT_U32      globalDmaNum;
                 rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
                     devNum, ii, jj, &globalDmaNum);
                 if (rc != GT_OK)
                 {
                     return rc;
                 }
                 regAddrTxFIFO = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, 0).configs.channelConfigs.speedProfile[globalDmaNum];
             }
             rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxFIFO, 0, 6, &profileValue);
             if (rc != GT_OK)
             {
                 return rc;
             }
             cpssOsPrintf("|       %2d           |       %2d         |\n",jj,profileValue);
         }
         cpssOsPrintf("\n\n");
    }
    return GT_OK;
}

/**
* @internal prevCpssDxChPortTxPizzaDumpFalconAll function
* @endinternal
*
* @brief     Dumps all related Tx information:
*       go over all DP units and print the pizza slices configurations.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prevCpssDxChPortTxPizzaDumpFalconAll
(
    IN  GT_U8 devNum
)
{
    GT_STATUS                            rc = GT_OK;
    GT_U32                               ii;
    GT_U32                               jj;
    GT_U32                               PizzaArbitersNum;
    GT_U32                               regAddrTxDMA;
    GT_U32                               regAddrTxFIFO;
    GT_U32                               TxDmaPizzaSize;
    GT_U32                               TxFifoPizzaSize;
    GT_U32                               value ;
    GT_U32                               validBit;
    GT_U32                               slicePort;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E |
    CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_AC3X_E);

    PizzaArbitersNum = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;

    /* go over all DP, each DP has 2 pizza configuration: TxDMA and TxFIFO*/
    for (ii = 0 ; ii < PizzaArbitersNum ; ii++ )
    {

         cpssOsPrintf("+=====================================================================================+\n");
         cpssOsPrintf("|                              Tx Global Index: %d                                    |\n",ii);
         cpssOsPrintf("+=====================================================================================+\n");
          /* tx dma*/
         regAddrTxDMA = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, ii).configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg;
         /* get slice number in DMA domain */

         cpssOsPrintf("\n\n----------------------Tx DMA Pizza ----------------------------------------------------\n");
         rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxDMA, 0, 7, &TxDmaPizzaSize);
         if (rc != GT_OK)
         {
             return rc;
         }
         /* the field contains maximal slot number, not number of slots */
         TxDmaPizzaSize ++;

         cpssOsPrintf("Tx DMA Pizza size : %d\n",TxDmaPizzaSize);
         rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxDMA, 31, 1, &value);
         if (rc != GT_OK)
         {
             return rc;
         }

         cpssOsPrintf("Tx DMA Pizza Load New Pizza  : %d\n\n",value);

         cpssOsPrintf("|  pizza slice #    |   valid bit    |      port      |\n");
         cpssOsPrintf("|=====================================================|\n");
         for (jj = 0; jj < TxDmaPizzaSize ; jj++)
         {
              regAddrTxDMA = PRV_DXCH_REG1_UNIT_SIP6_TXDMA_INDEX_MAC(devNum, ii).configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[jj];
              rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxDMA, 31, 1, &validBit);
              if (rc != GT_OK)
              {
                  return rc;
              }
              rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxDMA, 0, 4, &slicePort);
              if (rc != GT_OK)
              {
                  return rc;
              }
           cpssOsPrintf("|      %3d          |     %2d         |        %3d     |\n",jj,validBit,slicePort);
         }
         cpssOsPrintf("\n\n----------------------Tx FIFO Pizza ----------------------------------------------------\n");
         /* tx fifo*/
         regAddrTxFIFO = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, ii).configs.globalConfigs.pizzaArbiter.pizzaArbiterCtrlReg;
         /* get slice number in FIFO domain */
         rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxFIFO, 0, 7, &TxFifoPizzaSize);
         if (rc != GT_OK)
         {
             return rc;
         }
         /* the field contains maximal slot number, not number of slots */
         TxFifoPizzaSize ++;

         cpssOsPrintf("Tx FIFO Pizza size : %d  \n",TxFifoPizzaSize);
         rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxFIFO, 31, 1, &value);
         if (rc != GT_OK)
         {
             return rc;
         }
         cpssOsPrintf("Tx FIFO Pizza Load New Pizza  : %d  \n",value);
         /*set 0 to valid bit for each pizza slice*/
         cpssOsPrintf("|  pizza slice #    |   valid bit    |      port      |\n");
         cpssOsPrintf("|=====================================================|\n");
         for (jj = 0; jj < TxFifoPizzaSize ; jj++)
         {
              regAddrTxFIFO = PRV_DXCH_REG1_UNIT_SIP6_TXFIFO_INDEX_MAC(devNum, ii).configs.globalConfigs.pizzaArbiter.pizzaArbiterConfigReg[jj];
              rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxFIFO, 31, 1, &validBit);
              if (rc != GT_OK)
              {
                  return rc;
              }
              rc = prvCpssDrvHwPpGetRegField(devNum, regAddrTxFIFO, 0, 4, &slicePort);
              if (rc != GT_OK)
              {
                  return rc;
              }
           cpssOsPrintf("|      %3d          |      %2d        |       %3d      |\n",jj,validBit,slicePort);
         }

       cpssOsPrintf("\n\n");
   }
    return GT_OK;

}

/**
* @internal prevCpssDxChPortTxDumpFalconAll function
* @endinternal
*
* @brief   Dumps all related Tx information:
*       go over all DP units and print the pizza slices configurations and the speed profile configuration
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prevCpssDxChPortTxDumpFalconAll
(
    IN  GT_U8 devNum
)
{

    GT_STATUS  rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E |
    CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_AC3X_E);


    cpssOsPrintf("+=====================================================================================+\n");
    cpssOsPrintf("|                              Tx PIZZA                                               |\n");
    cpssOsPrintf("+=====================================================================================+\n");
    rc = prevCpssDxChPortTxPizzaDumpFalconAll(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\n\n");
    cpssOsPrintf("+=====================================================================================+\n");
    cpssOsPrintf("|                              Tx SPEED PROFILE                                       |\n");
    cpssOsPrintf("+=====================================================================================+\n");
    rc = prevCpssDxChPortTxSpeedProfileDumpFalconAll(devNum);

    return rc;
}




