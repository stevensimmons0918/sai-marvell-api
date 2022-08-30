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
* @file prvCpssDxChXLGMacCtrl.c
*
* @brief bobcat2 XLG mac control reg 5 API
*
* @version   7
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChXLGMacCtrl.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMacCtrl.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*---------------------------------------------------------------------------------------------
 * /Cider/EBU/Bobcat2/Bobcat2 {Current}/Switching Core/GOP/<Gige MAC IP> Gige MAC IP Units%g/Tri-Speed Port MAC Configuration/Port MAC Control Register1
 * 15 - 15  Short preable    0x0 -- 8 bytes
 *                           0x1 -- 4 bytes
 * /Cider/EBU/Bobcat2/Bobcat2 {Current}/Switching Core/GOP/<Gige MAC IP> Gige MAC IP Units%g/Tri-Speed Port MAC Configuration/Port MAC Control Register3
 * 6-14     IPG
 *---------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------
 * /Cider/EBU/Bobcat2/Bobcat2 {Current}/Switching Core/GOP/<XLG MAC IP> XLG MAC IP Units%p/Port MAC Control Register5
 *
 *
 *  0 -  3  TxIPGLength          minimal vaue is 8 for 10G and 40G
 *  4 -  6  PreambleLengthTx     0 -- 8 bytes
 *                               1..7 -- 1..7-bytes
 *                               for 10G 4,8 are only allowed
 *  7 -  9  PreambleLengthRx     0 -- 8 bytes
 *                               1..7 -- 1..7-bytes
 * 10 - 12  TxNumCrcBytes        legal value 1,2,3,4
 * 13 - 15  RxNumCrcBytes        legal value 1,2,3,4
 *---------------------------------------------------------------------------------------------
 */



/**
* @internal prvCpssDxChBobcat2PortMacXLGIsSupported function
* @endinternal
*
* @brief   check whether XLG mac is supported for specific mac
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mac                      -  number
*
* @param[out] isSupportedPtr           is mac supported result
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad ptr
*/
GT_STATUS prvCpssDxChBobcat2PortMacXLGIsSupported
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     mac,
    OUT GT_BOOL                *isSupportedPtr
)
{
    GT_U32 regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(isSupportedPtr);

    PRV_CPSS_DXCH_PORT_MAC_CTRL5_REG_MAC(devNum, mac, PRV_CPSS_PORT_XLG_E, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        *isSupportedPtr = GT_FALSE;
    }
    else
    {
        *isSupportedPtr = GT_TRUE;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChBobcat2PortXLGMacCtrl5FldSet function
* @endinternal
*
* @brief   XLG mac control register 5 set specific field
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] fldOffs                  - fld offset
* @param[in] fldLen                   - fld len
* @param[in] fldVal                   - value to write
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChBobcat2PortXLGMacCtrl5FldSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   fldOffs,
    IN GT_U32                   fldLen,
    IN GT_U32                   fldVal
)
{
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_PHYSICAL_PORT_NUM    portMacNum;
    PRV_CPSS_DXCH_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    rc = prvCpssDxChMacByPhysPortGet(devNum, portNum, &portMacNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL5_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(prvCpssDxChPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = fldVal;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = fldLen;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = fldOffs;

    rc = prvCpssDxChPortMacConfiguration(devNum, portNum, regDataArray);

    return rc;
}

/**
* @internal prvCpssDxChBobcat2PortXLGMacCtrl5FldGet function
* @endinternal
*
* @brief   XLG mac control register 5 get specific field
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mac                      - physical port number
* @param[in] fldOffs                  - fld offset
* @param[in] fldLen                   - fld len
*
* @param[out] fldValPtr                - value to write
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssDxChBobcat2PortXLGMacCtrl5FldGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     mac,
    IN  GT_U32                   fldOffs,
    IN  GT_U32                   fldLen,
    OUT GT_U32                  *fldValPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(fldValPtr);

    PRV_CPSS_DXCH_PORT_MAC_CTRL5_REG_MAC(devNum, mac, PRV_CPSS_PORT_XLG_E, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssHwPpPortGroupGetRegField(devNum, 0, regAddr,
                                            fldOffs,
                                            fldLen,
                                            fldValPtr);
    return rc;
}

/**
* @internal prvCpssDxChBobcat2PortXLGMacIPGLengthSet function
* @endinternal
*
* @brief   XLG mac control register 5 set IPG length
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
*                                      mac      - physical port number
* @param[in] length                   - ipg length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChBobcat2PortXLGMacIPGLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   length
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    /* if (length < XLG_MAC_CTRL5_TX_IPG_MIN_VAL_D || */ /* currently minimal possible length 0 -- technically */
    if (length > XLG_MAC_CTRL5_TX_IPG_MAX_VAL_D)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChBobcat2PortXLGMacCtrl5FldSet(devNum,portNum
                                ,XLG_MAC_CTRL5_TX_IPG_LEN_FLD_OFFS_D
                                ,XLG_MAC_CTRL5_TX_IPG_LEN_FLD_LEN_D
                                ,length);
    return rc;
}


/**
* @internal prvCpssDxChBobcat2PortXLGMacIPGLengthGet function
* @endinternal
*
* @brief   XLG mac control register 5 get IPG length
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mac                      - physical port number
*
* @param[out] lengthPtr                - pointer to length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, mac
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChBobcat2PortXLGMacIPGLengthGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     mac,
    IN GT_U32                  *lengthPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(lengthPtr);
    *lengthPtr = 0;

    rc = prvCpssDxChBobcat2PortXLGMacCtrl5FldGet(devNum,mac
                                ,XLG_MAC_CTRL5_TX_IPG_LEN_FLD_OFFS_D
                                ,XLG_MAC_CTRL5_TX_IPG_LEN_FLD_LEN_D
                                ,/*OUT*/lengthPtr);
    if (rc != GT_OK)
    {
        *lengthPtr = 0;
        return rc;
    }
    return GT_OK;
}



/**
* @internal prvCpssDxChBobcat2PortXLGMacPreambleLengthSet function
* @endinternal
*
* @brief   XLG mac control register 5 set Preamble length
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - max number
* @param[in] direction                - RX/TX/both
* @param[in] length                   - preamble length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChBobcat2PortXLGMacPreambleLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    if (length < XLG_MAC_CTRL5_PREAMBLE_MIN_VAL_D || length > XLG_MAC_CTRL5_PREAMBLE_MAX_VAL_D)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (length == XLG_MAC_CTRL5_PREAMBLE_MAX_VAL_D) /* see comment at cider  swicthing code/GOP/XLG mac IP/units/Port Mac Control register 5 */
    {
        length = 0;
    }

    switch (direction)
    {
        case CPSS_PORT_DIRECTION_RX_E:
            rc = prvCpssDxChBobcat2PortXLGMacCtrl5FldSet(devNum,portNum
                                        ,XLG_MAC_CTRL5_PREAMBLE_LEN_RX_FLD_OFFS_D
                                        ,XLG_MAC_CTRL5_PREAMBLE_LEN_RX_FLD_LEN_D
                                        ,length);
            if (rc != GT_OK)
            {
                return rc;
            }
        break;
        case CPSS_PORT_DIRECTION_TX_E:
            rc = prvCpssDxChBobcat2PortXLGMacCtrl5FldSet(devNum,portNum
                                        ,XLG_MAC_CTRL5_PREAMBLE_LEN_TX_FLD_OFFS_D
                                        ,XLG_MAC_CTRL5_PREAMBLE_LEN_TX_FLD_LEN_D
                                        ,length);
            if (rc != GT_OK)
            {
                return rc;
            }
        break;
        case CPSS_PORT_DIRECTION_BOTH_E:
            rc = prvCpssDxChBobcat2PortXLGMacCtrl5FldSet(devNum,portNum
                                        ,XLG_MAC_CTRL5_PREAMBLE_LEN_RX_FLD_OFFS_D
                                        ,XLG_MAC_CTRL5_PREAMBLE_LEN_RX_FLD_LEN_D
                                        ,length);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = prvCpssDxChBobcat2PortXLGMacCtrl5FldSet(devNum,portNum
                                        ,XLG_MAC_CTRL5_PREAMBLE_LEN_TX_FLD_OFFS_D
                                        ,XLG_MAC_CTRL5_PREAMBLE_LEN_TX_FLD_LEN_D
                                        ,length);
            if (rc != GT_OK)
            {
                return rc;
            }
        break;
        default:
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChBobcat2PortXLGMacPreambleLengthGet function
* @endinternal
*
* @brief   XLG mac control register 5 get Preamble length
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mac                      - max number
* @param[in] direction                - RX/TX/both
*
* @param[out] lengthPtr                - pointer to preamble length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssDxChBobcat2PortXLGMacPreambleLengthGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     mac,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                  *lengthPtr
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(lengthPtr);

    switch (direction)
    {
        case CPSS_PORT_DIRECTION_RX_E:
            rc = prvCpssDxChBobcat2PortXLGMacCtrl5FldGet(devNum,mac
                                        ,XLG_MAC_CTRL5_PREAMBLE_LEN_RX_FLD_OFFS_D
                                        ,XLG_MAC_CTRL5_PREAMBLE_LEN_RX_FLD_LEN_D
                                        ,lengthPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        break;
        case CPSS_PORT_DIRECTION_TX_E:
            rc = prvCpssDxChBobcat2PortXLGMacCtrl5FldGet(devNum,mac
                                        ,XLG_MAC_CTRL5_PREAMBLE_LEN_TX_FLD_OFFS_D
                                        ,XLG_MAC_CTRL5_PREAMBLE_LEN_TX_FLD_LEN_D
                                        ,lengthPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        break;
        default:
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    if (*lengthPtr == 0) /* see comment at cider  swicthing code/GOP/XLG mac IP/units/Port Mac Control register 5 */
    {
        *lengthPtr = XLG_MAC_CTRL5_PREAMBLE_MAX_VAL_D;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChBobcat2PortXLGMacNumCRCByteshSet function
* @endinternal
*
* @brief   XLG mac control register 5 set number of CRC bytes
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - RX/TX/both
* @param[in] crcBytesNum              - number of CRC bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChBobcat2PortXLGMacNumCRCByteshSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   crcBytesNum
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    if (crcBytesNum < XLG_MAC_CTRL5_NUM_CRC_MIN_VAL_D  || crcBytesNum > XLG_MAC_CTRL5_NUM_CRC_MAX_VAL_D)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (direction)
    {
        case CPSS_PORT_DIRECTION_RX_E:
            rc = prvCpssDxChBobcat2PortXLGMacCtrl5FldSet(devNum,portNum
                                        ,XLG_MAC_CTRL5_RX_NUM_CRC_BYTES_FLD_OFFS_D
                                        ,XLG_MAC_CTRL5_RX_NUM_CRC_BYTES_FLD_LEN_D
                                        ,crcBytesNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        break;
        case CPSS_PORT_DIRECTION_TX_E:
            rc = prvCpssDxChBobcat2PortXLGMacCtrl5FldSet(devNum,portNum
                                        ,XLG_MAC_CTRL5_TX_NUM_CRC_BYTES_FLD_OFFS_D
                                        ,XLG_MAC_CTRL5_TX_NUM_CRC_BYTES_FLD_LEN_D
                                        ,crcBytesNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        break;
        case CPSS_PORT_DIRECTION_BOTH_E:
            rc = prvCpssDxChBobcat2PortXLGMacCtrl5FldSet(devNum,portNum
                                        ,XLG_MAC_CTRL5_RX_NUM_CRC_BYTES_FLD_OFFS_D
                                        ,XLG_MAC_CTRL5_RX_NUM_CRC_BYTES_FLD_LEN_D
                                        ,crcBytesNum);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = prvCpssDxChBobcat2PortXLGMacCtrl5FldSet(devNum,portNum
                                        ,XLG_MAC_CTRL5_TX_NUM_CRC_BYTES_FLD_OFFS_D
                                        ,XLG_MAC_CTRL5_TX_NUM_CRC_BYTES_FLD_LEN_D
                                        ,crcBytesNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        break;
        default:
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChBobcat2PortXLGMacNumCRCByteshGet function
* @endinternal
*
* @brief   XLG mac control register 5 get number of CRC bytes
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] mac                      - max number
* @param[in] direction                - RX/TX/both
*
* @param[out] crcBytesNumPtr           - pointer to number of CRC bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChBobcat2PortXLGMacNumCRCByteshGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     mac,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                  *crcBytesNumPtr
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(crcBytesNumPtr);

    switch (direction)
    {
        case CPSS_PORT_DIRECTION_RX_E:
            rc = prvCpssDxChBobcat2PortXLGMacCtrl5FldGet(devNum,mac
                                        ,XLG_MAC_CTRL5_RX_NUM_CRC_BYTES_FLD_OFFS_D
                                        ,XLG_MAC_CTRL5_RX_NUM_CRC_BYTES_FLD_LEN_D
                                        ,/*OUT*/crcBytesNumPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        break;
        case CPSS_PORT_DIRECTION_TX_E:
            rc = prvCpssDxChBobcat2PortXLGMacCtrl5FldGet(devNum,mac
                                        ,XLG_MAC_CTRL5_TX_NUM_CRC_BYTES_FLD_OFFS_D
                                        ,XLG_MAC_CTRL5_TX_NUM_CRC_BYTES_FLD_LEN_D
                                        ,/*OUT*/crcBytesNumPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        break;
        default:
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}


