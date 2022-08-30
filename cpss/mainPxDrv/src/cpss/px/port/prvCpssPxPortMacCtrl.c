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
* @file prvCpssPxPortMacCtrl.c
*
* @brief pipe mac control
*
* @version   5
********************************************************************************
*/

#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/px/port/private/prvCpssPxPortMacCtrl.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>

/*---------------------------------------------------------------------------------------------
 * /Cider/EBU/Pipe/Pipe {Current}/Switching Core/GOP/<Gige MAC IP> Gige MAC IP Units%g/Tri-Speed Port MAC Configuration/Port MAC Control Register1
 * 15 - 15  Short preable    0x0 -- 8 bytes
 *                           0x1 -- 4 bytes
 * /Cider/EBU/Pipe/Pipe {Current}/Switching Core/GOP/<Gige MAC IP> Gige MAC IP Units%g/Tri-Speed Port MAC Configuration/Port MAC Control Register3
 * 6-14     IPG
 *---------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------
 * /Cider/EBU/Pipe/Pipe {Current}/Switching Core/GOP/<XLG MAC IP> XLG MAC IP Units%p/Port MAC Control Register5
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
* @internal prvCpssPxPortGigaMacPreambleLengthSet function
* @endinternal
*
* @brief   GE mac set preamble length
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - RX/TX/BOTH
* @param[in] length                   - preamble length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
static GT_STATUS prvCpssPxPortGigaMacPreambleLengthSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    fldVal;
    GT_U32    portMacNum;
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if (length != GIGA_MAC_CTRL1_PREAMBLE_LEN_4_D && length != GIGA_MAC_CTRL1_PREAMBLE_LEN_8_D)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (direction != CPSS_PORT_DIRECTION_TX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PX_PORT_MAC_CTRL1_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_GE_E, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (length == GIGA_MAC_CTRL1_PREAMBLE_LEN_4_D)
    {
        fldVal = 1;
    }
    else
    {
        fldVal = 0;
    }

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = fldVal;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = GIGA_MAC_CTRL1_PREAMBLE_LEN_FLD_OFFS_D;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = GIGA_MAC_CTRL1_PREAMBLE_LEN_FLD_OFFS_D;

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
    return rc;
}

/**
* @internal prvCpssPxPortXLGMacPreambleLengthSet function
* @endinternal
*
* @brief   XLG mac control register 5 set Preamble length
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - max number
* @param[in] direction                - RX/TX/both
* @param[in] length                   - preamble length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
static GT_STATUS prvCpssPxPortXLGMacPreambleLengthSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
)
{
    GT_STATUS rc;
    GT_U32    portMacNum;
    GT_U32    regAddr;
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if (length < XLG_MAC_CTRL5_PREAMBLE_MIN_VAL_D || length > XLG_MAC_CTRL5_PREAMBLE_MAX_VAL_D)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (length == XLG_MAC_CTRL5_PREAMBLE_MAX_VAL_D) /* see comment at cider  swicthing code/GOP/XLG mac IP/units/Port Mac Control register 5 */
    {
        length = 0;
    }
    PRV_CPSS_PX_PORT_MAC_CTRL5_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    switch (direction)
    {
        case CPSS_PORT_DIRECTION_RX_E:
            regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldData = length;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = XLG_MAC_CTRL5_PREAMBLE_LEN_RX_FLD_LEN_D;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = XLG_MAC_CTRL5_PREAMBLE_LEN_RX_FLD_OFFS_D;
        break;

        case CPSS_PORT_DIRECTION_TX_E:
            regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldData = length;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = XLG_MAC_CTRL5_PREAMBLE_LEN_TX_FLD_LEN_D;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = XLG_MAC_CTRL5_PREAMBLE_LEN_TX_FLD_OFFS_D;
        break;

        case CPSS_PORT_DIRECTION_BOTH_E:
            regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldData = length;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = XLG_MAC_CTRL5_PREAMBLE_LEN_RX_FLD_LEN_D;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = XLG_MAC_CTRL5_PREAMBLE_LEN_RX_FLD_OFFS_D;
            rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
            if (rc != GT_OK)
            {
                return rc;
            }
            if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

            regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldData = length;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = XLG_MAC_CTRL5_PREAMBLE_LEN_TX_FLD_LEN_D;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = XLG_MAC_CTRL5_PREAMBLE_LEN_TX_FLD_OFFS_D;
        break;

        default:
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssPxPortCGMacPreambleLengthSet function
* @endinternal
*
* @brief   Set Preamble length for CG MAC
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - RX/TX/both
* @param[in] length                   - preamble length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssPxPortCGMacPreambleLengthSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      portMacNum; /* MAC port number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    direction = direction; /* avoid warning */
    if ((length != 4) && (length != 8))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PX_REG1_CG_PORT_MAC_CMD_CFG_REG_MAC(devNum, portMacNum, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    regValue = (length == 4) ? 1 : 0;

    /* SHORT_PREAMBLE */
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 30, 1, regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* RX_SFD_ANY */
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 21, 1, regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    PRV_CPSS_PX_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum, portMacNum, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 14, 1, regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal prvCpssPxPortMacPreambleLengthSet function
* @endinternal
*
* @brief   set XLG mac Preable length
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                -  (RX/TX/both)
* @param[in] length                   = ipg  in bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssPxPortMacPreambleLengthSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
)
{
    GT_STATUS rcGiga;
    GT_STATUS rcXLG;
    GT_STATUS rcCG;
    GT_U32    mac;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, mac);
    if (direction > CPSS_PORT_DIRECTION_BOTH_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rcXLG = prvCpssPxPortGigaMacPreambleLengthSet(devNum, portNum, direction, length);
    rcGiga = prvCpssPxPortXLGMacPreambleLengthSet(devNum, portNum, direction, length);
    rcCG = prvCpssPxPortCGMacPreambleLengthSet(devNum, portNum, direction, length);

    if (rcXLG != GT_OK && rcGiga != GT_OK && rcCG != GT_OK)
    {
        return rcXLG;
    }
    return GT_OK;
}

/**
* @internal prvCpssPxPortMacPreambleLengthGet function
* @endinternal
*
* @brief   get XLG mac Preable length
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portType                 - port type
* @param[in] direction                -  (RX/TX/both)
*
* @param[out] lengthPtr                - preable ipg length in bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssPxPortMacPreambleLengthGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_PORT_TYPE_ENT   portType,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                  *lengthPtr
)
{
    GT_STATUS rc;
    GT_U32    mac, fldVal, regAddr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(lengthPtr);
    *lengthPtr = 0;
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, mac);

    switch (portType)
    {
        case  PRV_CPSS_PORT_GE_E:
            if (direction != CPSS_PORT_DIRECTION_TX_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            PRV_CPSS_PX_PORT_MAC_CTRL1_REG_MAC(devNum, mac, PRV_CPSS_PORT_GE_E, &regAddr);
            if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            rc = prvCpssHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), 0, regAddr
                                                    ,GIGA_MAC_CTRL1_PREAMBLE_LEN_FLD_OFFS_D
                                                    ,GIGA_MAC_CTRL1_PREAMBLE_LEN_FLD_LEN_D
                                                    ,/*OUT*/&fldVal);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (fldVal == 1)
            {
                *lengthPtr = GIGA_MAC_CTRL1_PREAMBLE_LEN_4_D;
            }
            else
            {
                *lengthPtr = GIGA_MAC_CTRL1_PREAMBLE_LEN_8_D;
            }
            break;

        case  PRV_CPSS_PORT_XLG_E:
            PRV_CPSS_PX_PORT_MAC_CTRL5_REG_MAC(devNum, mac, PRV_CPSS_PORT_XLG_E, &regAddr);
            if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            switch (direction)
            {
            case CPSS_PORT_DIRECTION_RX_E:
                rc = prvCpssHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), 0, regAddr,
                                                        XLG_MAC_CTRL5_PREAMBLE_LEN_RX_FLD_OFFS_D,
                                                        XLG_MAC_CTRL5_PREAMBLE_LEN_RX_FLD_LEN_D,
                                                        &fldVal);
                break;
            case CPSS_PORT_DIRECTION_TX_E:
                    rc = prvCpssHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), 0, regAddr,
                                                            XLG_MAC_CTRL5_PREAMBLE_LEN_TX_FLD_OFFS_D,
                                                            XLG_MAC_CTRL5_PREAMBLE_LEN_TX_FLD_LEN_D,
                                                            &fldVal);
                break;
             default:
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
            if (rc != GT_OK)
            {
                return rc;
            }
            if (fldVal == 0) /* see comment at cider  swicthing code/GOP/XLG mac IP/units/Port Mac Control register 5 */
            {
                *lengthPtr = XLG_MAC_CTRL5_PREAMBLE_MAX_VAL_D;
            }
            else
            {
                *lengthPtr = fldVal;
            }
            break;

        case PRV_CPSS_PORT_CG_E:
            if(!((direction == CPSS_PORT_DIRECTION_RX_E) || (direction == CPSS_PORT_DIRECTION_TX_E)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            PRV_CPSS_PX_REG1_CG_PORT_MAC_CMD_CFG_REG_MAC(devNum, mac, &regAddr);
            if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 30, 1, &fldVal);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            *lengthPtr = (fldVal == 0) ? 8 : 4;
            break;
        default:
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}
/**
* @internal prvCpssPxPortGigaMacIPGLengthSet function
* @endinternal
*
* @brief   GE mac set IPG length
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] length                   - ipg length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
static GT_STATUS prvCpssPxPortGigaMacIPGLengthSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   length
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    mac;
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, mac);

    if (length > GIGA_MAC_CTRL3_IPG_MAX_LEN_D)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PX_PORT_MAC_CTRL3_REG_MAC(devNum, mac, PRV_CPSS_PORT_GE_E, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = length;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = GIGA_MAC_CTRL3_IPG_LEN_FLD_LEN_D;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = GIGA_MAC_CTRL3_IPG_LEN_FLD_OFFS_D;

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
    return rc;
}

/**
* @internal prvCpssPxPortXLGMacIPGLengthSet function
* @endinternal
*
* @brief   XLG mac control register 5 set IPG length
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
*                                      mac      - physical port number
* @param[in] length                   - ipg length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
static GT_STATUS prvCpssPxPortXLGMacIPGLengthSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   length
)
{
    GT_STATUS rc;
    GT_U32    mac;
    GT_U32    regAddr;
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, mac);

    /* if (length < XLG_MAC_CTRL5_TX_IPG_MIN_VAL_D || */ /* currently minimal possible length 0 -- technically */
    if (length > XLG_MAC_CTRL5_TX_IPG_MAX_VAL_D)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_PX_PORT_MAC_CTRL5_REG_MAC(devNum, mac, PRV_CPSS_PORT_XLG_E, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = length;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = XLG_MAC_CTRL5_TX_IPG_LEN_FLD_LEN_D;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = XLG_MAC_CTRL5_TX_IPG_LEN_FLD_OFFS_D;

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
    return rc;
}

/**
* @internal prvCpssPxPortCGMacIPGLengthSet function
* @endinternal
*
* @brief   Set IPG length for CG MAC
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] length                   - ipg length
*                                       1-8 means IPG depend on
*                                       packet size, move
*                                       between 1byte to 8byte
*                                       12 -> mechanism of IPG
*                                       is enabled to create
*                                       average IPG of 12 any
*                                       other setting is
*                                       discarded
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
static GT_STATUS prvCpssPxPortCGMacIPGLengthSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   length
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      portMacNum; /* MAC port number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /*
        For 0 – DIC enabled. Mechanism of IPG is enabled to create average IPG of 12.
        For 1 – DIC disabled. IPG depend on packet size, move between 1byte to 8byte.
    */

    regAddr = PRV_PX_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portNum).CGPORTMACTxIpgLength;
    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if ((length >=1) && (length <=8))
    {
        regValue = 1;
    }
    else if (length == 12)
    {
        regValue = 0;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}
/**
* @internal prvCpssPxPortMacIPGLengthSet function
* @endinternal
*
* @brief   set XLG mac IPG length
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] length                   = ipg  in bytes
*                                       for CG MAC -->
*                                       1-8 means IPG depend on
*                                       packet size, move
*                                       between 1byte to 8byte
*                                       12 -> mechanism of IPG
*                                       is enabled to create
*                                       average IPG of 12 any
*                                       other setting are
*                                       discarded
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssPxPortMacIPGLengthSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   length
)
{
    GT_STATUS rcGiga;
    GT_STATUS rcXLG;
    GT_STATUS rcCG;
    GT_U32    portMacNum;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);


    rcGiga = prvCpssPxPortGigaMacIPGLengthSet(devNum,portNum,length);
    rcXLG  = prvCpssPxPortXLGMacIPGLengthSet (devNum,portNum,length);
    rcCG   = prvCpssPxPortCGMacIPGLengthSet (devNum,portNum,length);
    if (rcXLG != GT_OK && rcGiga != GT_OK && rcCG != GT_OK)
    {
        return rcXLG;
    }
    return GT_OK;
}

/**
* @internal prvCpssPxPortMacIPGLengthGet function
* @endinternal
*
* @brief   get XLG mac IPG length
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portType                 - port type
*
* @param[out] lengthPtr                = pointer to length in bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad ptr
*/
GT_STATUS prvCpssPxPortMacIPGLengthGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_PORT_TYPE_ENT   portType,
    IN GT_U32                  *lengthPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr, mac;
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, mac);
    CPSS_NULL_PTR_CHECK_MAC(lengthPtr);
    *lengthPtr = 0;

    switch (portType)
    {
        case  PRV_CPSS_PORT_GE_E:
            PRV_CPSS_PX_PORT_MAC_CTRL3_REG_MAC(devNum, mac, PRV_CPSS_PORT_GE_E, &regAddr);
            if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            rc = prvCpssHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), 0, regAddr
                                                    ,GIGA_MAC_CTRL3_IPG_LEN_FLD_OFFS_D
                                                    ,GIGA_MAC_CTRL3_IPG_LEN_FLD_LEN_D
                                                    ,lengthPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;

        case  PRV_CPSS_PORT_XLG_E:
            PRV_CPSS_PX_PORT_MAC_CTRL5_REG_MAC(devNum, mac, PRV_CPSS_PORT_XLG_E, &regAddr);
            if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            rc = prvCpssHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), 0, regAddr,
                                            XLG_MAC_CTRL5_TX_IPG_LEN_FLD_OFFS_D,
                                            XLG_MAC_CTRL5_TX_IPG_LEN_FLD_LEN_D,
                                            lengthPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;

        case  PRV_CPSS_PORT_CG_E:
            regAddr = PRV_PX_REG1_UNIT_CG_PORT_MAC_MAC(devNum, mac).CGPORTMACTxIpgLength;
            if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }

            rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, &regValue);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            /*
                For 0 – DIC enabled. Mechanism of IPG is enabled to create average IPG of 12.
                For 1 – DIC disabled. IPG depend on packet size, move between 1byte to 8byte.
            */
            *lengthPtr = (regValue == 0) ? 12 : 1;
            break;

        default:
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssPxPortCGMacNumCRCByteshSet function
* @endinternal
*
* @brief   Set number of CRC bytes for CG MAC
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - RX/TX/both
* @param[in] crcBytesNum              - number of CRC bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
static GT_STATUS prvCpssPxPortCGMacNumCRCByteshSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   crcBytesNum
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      portMacNum; /* MAC port number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    direction = direction;/* avoid warning */

    if ((crcBytesNum != 4) && (crcBytesNum != 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regValue = (crcBytesNum == 0) ? 1 : 0;

    /* CRC_FWD */
    PRV_CPSS_PX_REG1_CG_PORT_MAC_CMD_CFG_REG_MAC(devNum, portMacNum, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 6, 1, regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* disable_tx_crc */
    PRV_CPSS_PX_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum, portMacNum, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 24, 1, regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* strip_crc_ind */
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 15, 1, regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    PRV_CPSS_PX_REG1_CG_PORT_MAC_RX_CRC_OPT_REG_MAC(devNum, portMacNum, &regAddr);

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    /* CRC_CHECK_DIS */
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 16, 1, regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortXLGMacNumCRCByteshSet function
* @endinternal
*
* @brief   XLG mac control register 5 set number of CRC bytes
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - RX/TX/both
* @param[in] crcBytesNum              - number of CRC bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
static GT_STATUS prvCpssPxPortXLGMacNumCRCByteshSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   crcBytesNum
)
{
    GT_STATUS rc;
    GT_U32    portMacNum;
    GT_U32    regAddr;    /* register address */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if (crcBytesNum < XLG_MAC_CTRL5_NUM_CRC_MIN_VAL_D  || crcBytesNum > XLG_MAC_CTRL5_NUM_CRC_MAX_VAL_D)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (direction)
    {
        case CPSS_PORT_DIRECTION_RX_E:
            PRV_CPSS_PX_PORT_MAC_CTRL5_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &regAddr);
            if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);
            regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldData = crcBytesNum;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = XLG_MAC_CTRL5_RX_NUM_CRC_BYTES_FLD_LEN_D;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = XLG_MAC_CTRL5_RX_NUM_CRC_BYTES_FLD_OFFS_D;
            rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
            if (rc != GT_OK)
            {
                return rc;
            }
        break;
        case CPSS_PORT_DIRECTION_TX_E:
            PRV_CPSS_PX_PORT_MAC_CTRL5_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &regAddr);
            if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);
            regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldData = crcBytesNum;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = XLG_MAC_CTRL5_TX_NUM_CRC_BYTES_FLD_LEN_D;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = XLG_MAC_CTRL5_TX_NUM_CRC_BYTES_FLD_OFFS_D;
            rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
            if (rc != GT_OK)
            {
                return rc;
            }
        break;
        case CPSS_PORT_DIRECTION_BOTH_E:
            PRV_CPSS_PX_PORT_MAC_CTRL5_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &regAddr);
            if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);
            regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldData = crcBytesNum;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = XLG_MAC_CTRL5_RX_NUM_CRC_BYTES_FLD_LEN_D;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = XLG_MAC_CTRL5_RX_NUM_CRC_BYTES_FLD_OFFS_D;
            rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
            if (rc != GT_OK)
            {
                return rc;
            }

            PRV_CPSS_PX_PORT_MAC_CTRL5_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &regAddr);
            if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);
            regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldData = crcBytesNum;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = XLG_MAC_CTRL5_TX_NUM_CRC_BYTES_FLD_LEN_D;
            regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = XLG_MAC_CTRL5_TX_NUM_CRC_BYTES_FLD_OFFS_D;
            rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
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
* @internal prvCpssPxPortMacCrcModeSet function
* @endinternal
*
* @brief   Set XLG mac CRC number of bytes
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*                                      direction - direction (RX/TX/both)
* @param[in] numCrcBytes              - number of bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssPxPortMacCrcModeSet
(
    IN GT_SW_DEV_NUM                     devNum,
    IN GT_PHYSICAL_PORT_NUM              portNum,
    IN CPSS_PORT_DIRECTION_ENT           portDirection,
    IN GT_U32                            numCrcBytes
)
{
    GT_STATUS   rcCGMac = GT_BAD_PARAM;
    GT_STATUS   rcXLGMac = GT_BAD_PARAM;
    GT_U32      mac;

    PRV_CPSS_PORT_TYPE_ENT portMacType; /* mac type to use */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, mac);

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,mac);
    if(portMacType == PRV_CPSS_PORT_CG_E)
    {
        if(!((numCrcBytes == 0) || (numCrcBytes == 4)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    rcXLGMac = prvCpssPxPortXLGMacNumCRCByteshSet(devNum, portNum, portDirection, numCrcBytes);
    rcCGMac = prvCpssPxPortCGMacNumCRCByteshSet(devNum , portNum, portDirection, numCrcBytes);

    if ((rcXLGMac != GT_OK) && (rcCGMac != GT_OK))
    {
        return rcXLGMac;
    }
    return GT_OK;

}

/**
* @internal prvCpssPxPortMacCrcModeGet function
* @endinternal
*
* @brief   Get XLG mac CRC number of bytes
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*                                      direction - direction (RX/TX/both)
*
* @param[out] numCrcBytesPtr           - number of bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssPxPortMacCrcModeGet
(
    IN GT_SW_DEV_NUM                     devNum,
    IN GT_PHYSICAL_PORT_NUM              portNum,
    IN CPSS_PORT_DIRECTION_ENT           portDirection,
    IN GT_U32                           *numCrcBytesPtr
)
{
    GT_STATUS       rc;
    GT_U32          mac;
    GT_U32          regValue, regAddr;    /* register address */
    PRV_CPSS_PORT_TYPE_ENT          portMacType; /* port MAC type */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, mac);

        portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,mac);
    switch(portMacType)
    {
        case PRV_CPSS_PORT_GE_E:
            /* GE mac, there no option to get CRC, by definition 4 is used */
            *numCrcBytesPtr = 4;
            break;
        case PRV_CPSS_PORT_XG_E:
        case PRV_CPSS_PORT_XLG_E:
        case PRV_CPSS_PORT_HGL_E:
            switch (portDirection)
            {
                case CPSS_PORT_DIRECTION_RX_E:
                    PRV_CPSS_PX_PORT_MAC_CTRL5_REG_MAC(devNum, mac, PRV_CPSS_PORT_XLG_E, &regAddr);
                    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                    rc = prvCpssHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), 0, regAddr,
                                                            XLG_MAC_CTRL5_RX_NUM_CRC_BYTES_FLD_OFFS_D,
                                                            XLG_MAC_CTRL5_RX_NUM_CRC_BYTES_FLD_LEN_D,
                                                            numCrcBytesPtr);

                    break;
                case CPSS_PORT_DIRECTION_TX_E:
                    PRV_CPSS_PX_PORT_MAC_CTRL5_REG_MAC(devNum, mac, PRV_CPSS_PORT_XLG_E, &regAddr);
                    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                    rc = prvCpssHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), 0, regAddr,
                                                            XLG_MAC_CTRL5_TX_NUM_CRC_BYTES_FLD_OFFS_D,
                                                            XLG_MAC_CTRL5_TX_NUM_CRC_BYTES_FLD_LEN_D,
                                                            numCrcBytesPtr);
                    break;
                default:
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            break;
        case PRV_CPSS_PORT_CG_E:
            /* CRC_FWD */
            PRV_CPSS_PX_REG1_CG_PORT_MAC_CMD_CFG_REG_MAC(devNum, mac, &regAddr);
            if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }

            rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 6, 1, &regValue);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            *numCrcBytesPtr = (regValue == 1) ? 0 : 4;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

