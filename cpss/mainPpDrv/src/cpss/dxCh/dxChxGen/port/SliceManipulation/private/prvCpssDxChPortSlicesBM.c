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
* @file prvCpssDxChPortSlicesBM.c
*
* @brief Buffer Management Unit Slices Manipulation Interface
*
* @version   6
********************************************************************************
*/
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/SliceManipulation/private/prvCpssDxChPortSlicesBM.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
* @internal prvLion2PortPizzaArbiterBMSlicesNumSet function
* @endinternal
*
* @brief   The function sets slice number used in Pizza Arbiter for BM unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] slicesNum                - slices number to be set in pizza arbiter
* @param[in] enableStrictPriority4CPU - strict priority to CPU bit shall be set or not
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_HW_ERROR              - hw error
* @retval GT_OUT_OF_RANGE          - number of slice to be set is greater than available
*/
GT_STATUS prvLion2PortPizzaArbiterBMSlicesNumSet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 slicesNum,
    IN GT_BOOL enableStrictPriority4CPU
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      regAddr;            /* register's address */
    GT_U32      slicesNum2Set;
    GT_U32      fieldData;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    /* When 0 all the slots are in use */
    if (slicesNum > BM_TOTAL_SLICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    if (slicesNum == BM_TOTAL_SLICES_CNS)
    {
        slicesNum2Set = 0;
    }
    else
    {
        slicesNum2Set = slicesNum;
    }

    /* number of slices in RX DMA pizza arbiter and
       enable strict priority for CPU (allocate slice for it)
       problem to do on all cores because anyway CPU connected just to 0 */

    fieldData = slicesNum2Set << BM_CTRL_SLICE_NUM_OFFS_CNS;
    if (enableStrictPriority4CPU)
    {
        fieldData |= BM_CTRL_STRICT_PRIORITY_BIT_CNS; /* set bit of strict priority for CPU */
    }
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngPZGenConfiglReg;
    rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                            0, 6, fieldData);
    return rc;
}


/**
* @internal prvLion2PortPizzaArbiterBMSlicesNumGet function
* @endinternal
*
* @brief   The function get slice number used in Pizza Arbiter and strict priority state bit
*         for BM unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
*
* @param[out] slicesNumPtr             - place to store slices number used in pizza arbiter
* @param[out] enableStrictPriority4CPUPtr - where to store strict priority to CPU bit
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_BAD_PTR               - bad slicesNumPtr, enableStrictPriority4CPUPtr
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterBMSlicesNumGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   portGroupId,
    OUT GT_U32  *slicesNumPtr,
    OUT GT_BOOL *enableStrictPriority4CPUPtr
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      regAddr;            /* register's address */
    GT_U32      fieldData;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    if (NULL == slicesNumPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    if (NULL == enableStrictPriority4CPUPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngPZGenConfiglReg;
    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                            0, 6, &fieldData);

    if (rc != GT_OK)
    {
        return rc;
    }

    *slicesNumPtr = (fieldData & BM_CTRL_SLICE_NUM_BITS_CNS) >> BM_CTRL_SLICE_NUM_OFFS_CNS ;
    if (*slicesNumPtr == 0)
    {
        *slicesNumPtr = 32;
    }
    *enableStrictPriority4CPUPtr = (fieldData & BM_CTRL_STRICT_PRIORITY_BIT_CNS) == BM_CTRL_STRICT_PRIORITY_BIT_CNS;
    return rc;

}

/**
* @internal prvLion2PortPizzaArbiterBMSliceOccupy function
* @endinternal
*
* @brief   The function occupy the slice (i.e. set the spicific slice
*         be asigned to specific port for BM unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] sliceId                  - slice to be occupied
* @param[in] portId                   - local port id (inside portGroup) to which slice is assigned
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_OUT_OF_RANGE          - sliceId or portId are greater than available
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterBMSliceOccupy
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 sliceId,
    IN GT_PHYSICAL_PORT_NUM portId
)
{
    GT_STATUS rc;
    /* GT_U32    regAddr; */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    rc = prvLion2PortPizzaArbiterBMSliceStateSet(devNum,portGroupId,sliceId,GT_TRUE,portId);
    if (GT_OK != rc)
    {
        return rc;
    }
    return GT_OK;
}


/**
* @internal prvLion2PortPizzaArbiterBMSliceRelease function
* @endinternal
*
* @brief   The function release the slice (i.e. set the spicific slice
*         to be disable to any port) for BM unit)
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] sliceId                  - slice to be released
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_OUT_OF_RANGE          - sliceId or portId are greater than available
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterBMSliceRelease
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 sliceId
)
{
    GT_STATUS rc;
    /* GT_U32 regAddr; */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    rc = prvLion2PortPizzaArbiterBMSliceStateSet(devNum,portGroupId,sliceId,GT_FALSE,0);
    if (GT_OK != rc)
    {
        return rc;
    }
    return GT_OK;
}


/**
* @internal prvLion2PortPizzaArbiterBMSliceStateSet function
* @endinternal
*
* @brief   The function sets the state of the slice (i.e. if slace is disable or enable
*         and if it enbaled to which port it is assigned) for BM unit.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] sliceId                  - slice to be released
* @param[in] isEnabled                - is slace enabled
* @param[in] portId                   - pord id slace is enabled to
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_OUT_OF_RANGE          - sliceId are greater than available
* @retval GT_BAD_PTR               - isOccupiedPtr or portNumPtr are NULL
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterBMSliceStateSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 sliceId,
    IN GT_BOOL              isEnabled,
    IN GT_PHYSICAL_PORT_NUM portId
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    data;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    if (sliceId >= BM_TOTAL_SLICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* field is 4 bit length */
    if (portId >= PRV_CPSS_GEN_PP_MAX_PORTS_IN_PORT_GROUP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* is the following  sequence more right
    ** slice disable
    ** port num set
    ** slice enable ,
    ** currently by definition since NO traffic is passed through the port
    **  it seems it does not matter
    */

    /* first set port id  */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngPZSlotConfigReg[sliceId/8];
    rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                            (sliceId%8)*4, 4, portId);
    if(rc != GT_OK)
    {
        return rc;
    }
    data = (GT_U32)isEnabled;
    /* enable mapping of buffer management pizza slot of this port by arbiter */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngPZSlotEnableReg;
    rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, sliceId, 1, data);
    if (GT_OK != rc)
    {
        return rc;
    }
    return GT_OK;
}


/**
* @internal prvLion2PortPizzaArbiterBMSliceGetState function
* @endinternal
*
* @brief   The function gets the state of the slice (i.e. if slace is disable or enable
*         and if it enbaled to which port it is assigned) for BM unit.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] sliceId                  - slice to be released
*
* @param[out] isOccupiedPtr            - place to store whether slice is occupied
* @param[out] portNumPtr               - plavce to store to which port slice is assigned
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_OUT_OF_RANGE          - sliceId are greater than available
* @retval GT_BAD_PTR               - isOccupiedPtr or portNumPtr are NULL
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterBMSliceGetState
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 sliceId,
    OUT GT_BOOL              *isOccupiedPtr,
    OUT GT_PHYSICAL_PORT_NUM *portNumPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    occupyingPort;       /* number of local port currenty occupying slice */
    GT_U32    isOccupied;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);


    if (NULL == isOccupiedPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    if (NULL == portNumPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if (sliceId >= BM_TOTAL_SLICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    *isOccupiedPtr = GT_FALSE;
    *portNumPtr    = 0xFF;

    /* read whether port occupies current slice is enabled*/
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngPZSlotEnableReg;
    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, sliceId, 1, &isOccupied);
    if(rc != GT_OK)
    {
        return rc;
    }
    *isOccupiedPtr = BIT2BOOL_MAC(isOccupied);
    /* read what port occupies current slice */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.bufMngPZSlotConfigReg[sliceId/8];
    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                            (sliceId%8)*4, 4, /*OUT*/&occupyingPort);
    if(rc != GT_OK)
    {
        return rc;
    }
    * portNumPtr = occupyingPort;
    return GT_OK;
}

/**
* @internal prvLion2PortPizzaArbiterBMInit function
* @endinternal
*
* @brief   Init BM unit (sets number of slice to use , strict priority to CPU bit
*         and disables all slices of the device.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] sliceNum2Init            - number of slices to be used
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_OUT_OF_RANGE          - sliceNum2Init is greater than available
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS prvLion2PortPizzaArbiterBMInit
(
     IN GT_U8  devNum,
     IN GT_U32 portGroupId,
     IN GT_U32 sliceNum2Init
)
{
    GT_STATUS rc;
    GT_U32    sliceId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    rc = prvLion2PortPizzaArbiterBMSlicesNumSet(/*IN*/devNum,/*IN*/portGroupId,
                                                /*IN*/sliceNum2Init,
                                                /*IN*/GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }
    for (sliceId = 0 ; sliceId < BM_TOTAL_SLICES_CNS; sliceId++)
    {
        rc = prvLion2PortPizzaArbiterBMSliceRelease(devNum,portGroupId,sliceId);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    return rc;
}


