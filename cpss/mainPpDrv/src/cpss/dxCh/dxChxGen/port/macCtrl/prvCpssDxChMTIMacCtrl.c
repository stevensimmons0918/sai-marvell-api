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
* @file prvCpssDxChMTIMacCtrl.c
*
* @brief SIP6 MTI mac control API
*
* @version   1
********************************************************************************
*/

#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMacCtrl.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMTIMacCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwFalconInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
* @internal prvCpssDxChPortMTIMacPreambleLengthSet function
* @endinternal
*
* @brief   Set Preamble length for MTI MAC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X.
*                                  Bobcat3; Aldrin2;
*
* @param[in] devNum                - device number
* @param[in] portNum               - physical port number
* @param[in] length                -  of preamble in bytes
*                                      support only values of 1,4,8
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChPortMTIMacPreambleLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   length
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue = 0;   /* register value */
    GT_U32      portMacNum; /* MAC port number */
    PRV_CPSS_PORT_TYPE_ENT macType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if ((length !=  1) &&
        (length != 4) &&
        (length != 8))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    /* Not supported for MAC400 and MAC_CPU */
    if((macType == PRV_CPSS_PORT_MTI_100_E) ||
      (macType == PRV_CPSS_PORT_MTI_USX_E) ||
       ((macType == PRV_CPSS_PORT_MTI_CPU_E)&& PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
    {
        if(macType == PRV_CPSS_PORT_MTI_100_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.commandConfig;
        }
        else if(macType == PRV_CPSS_PORT_MTI_USX_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.commandConfig;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.commandConfig;
        }
        if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if(length == 1)
        {
            regValue = 2;/*bit 31 to set for 1 byte preamble length*/
        }
        else
        {
            /* SHORT_PREAMBLE */
            regValue = (length == 4) ? 1 : 0;
        }

        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                       portMacNum),
                                                                       regAddr, 30,
                                                                       2, regValue);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortMTIMacPreambleLengthGet function
* @endinternal
*
* @brief   Get Preamble length for MTI MAC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X.
*                                  Bobcat3; Aldrin2;
*
* @param[in] devNum                - device number
* @param[in] portNum               - physical port number
* @param[in] macType               - mac type
* @param[out] lengthPtr            - pointer to preamble length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssDxChPortMTIMacPreambleLengthGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_PORT_TYPE_ENT   macType,
    IN GT_U32                  *lengthPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value to be written */
    GT_U32      portMacNum; /* MAC port number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(lengthPtr);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if((macType == PRV_CPSS_PORT_MTI_100_E) ||
       (macType == PRV_CPSS_PORT_MTI_USX_E) ||
      ((macType == PRV_CPSS_PORT_MTI_CPU_E) && PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
    {
        if(macType == PRV_CPSS_PORT_MTI_100_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.commandConfig;
        }
        else if(macType == PRV_CPSS_PORT_MTI_USX_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.commandConfig;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.commandConfig;
        }
        if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                                                                           regAddr, 30, 2, &regValue);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        switch(regValue)
        {
            case 0:
                *lengthPtr = 8;
                break;
            case 1:
                *lengthPtr = 4;
                break;
            case 2:
                *lengthPtr = 1;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* Not supported for MAC400 and MAC_CPU retruning default value */
        *lengthPtr = 8;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortMTIMacIPGLengthSet function
* @endinternal
*
* @brief   Set IPG length for MTI MAC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X.
*                                  Bobcat3,Aldrin2
*
* @param[in] devNum                - device number
* @param[in] portNum               - physical port number
* @param[in] length                - ipg length
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChPortMTIMacIPGLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   length
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      portMacNum; /* MAC port number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_U32      offset;
    GT_U32      fieldLength;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_CAELUM_E
                                            | CPSS_ALDRIN_E | CPSS_AC3X_E
                                            | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    offset =0;

    switch(macType)
    {
        case PRV_CPSS_PORT_MTI_100_E:
        case PRV_CPSS_PORT_MTI_USX_E:
            if(length > MTI64_MAC_TX_IPG_MAX_VAL_D)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            if (length < 12)
            {
                regValue = 1;/* Set to 0x1 for minimal IPG else
                                set the average IPG in increments of 8(starting from 12) */
            }
            else
            {
                regValue = length;
            }

            if(macType == PRV_CPSS_PORT_MTI_100_E)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.txIpgLength;
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.txIpgLength;
            }
            fieldLength = 6;
            break;
        case PRV_CPSS_PORT_MTI_400_E:
            if( length > MTI400_MAC_TX_IPG_MAX_VAL_D)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.txIpgLength;

            regValue = length;
            fieldLength = 16;
            break;
        case PRV_CPSS_PORT_MTI_CPU_E:
            if(length > MTICPU_MAC_TX_IPG_MAX_VAL_D)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.txIpgLength;

            if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                if (length < 12)
                {
                    regValue = 1;/* Set to 0x1 for minimal IPG else
                                    set the average IPG in increments of 8(starting from 12) */
                }
                else
                {
                    regValue = length;
                }
                fieldLength = 6;
            }
            else
            {
                regValue = length;
                fieldLength = 7;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    /* TxIPG */
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,
             PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                        portMacNum),
                                                                       regAddr, offset, fieldLength, regValue);
    return rc;
}


/**
* @internal prvCpssDxChPortMTIMacIPGLengthGet function
* @endinternal
*
* @brief   Set IPG length for MTI MAC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X.
*                                  Bobcat3; Aldrin2;
*
* @param[in] devNum                - device number
* @param[in] portNum               - physical port number
* @param[in] portType              - port type
* @param[out] lengthPtr            - pointer to length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, mac
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChPortMTIMacIPGLengthGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_PORT_TYPE_ENT   macType,
    OUT GT_U32                  *lengthPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      portMacNum; /* MAC port number */
    GT_32       offset = 0;
    GT_32       fieldLength = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_CAELUM_E
                                            | CPSS_ALDRIN_E | CPSS_AC3X_E
                                            | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(lengthPtr);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    offset =0;

    switch(macType)
    {
        case PRV_CPSS_PORT_MTI_100_E:
        case PRV_CPSS_PORT_MTI_USX_E:
            if(macType == PRV_CPSS_PORT_MTI_100_E)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.txIpgLength;
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.txIpgLength;
            }
            fieldLength = 6;
            break;
        case PRV_CPSS_PORT_MTI_400_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.txIpgLength;
            fieldLength = 16;
            break;
        case PRV_CPSS_PORT_MTI_CPU_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.txIpgLength;
            if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                fieldLength = 6;
            }
            else
            {
                fieldLength = 7;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* TxIPG */
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
             PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                        portMacNum),
                                                                       regAddr, offset, fieldLength, &regValue);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    *lengthPtr = regValue;

    return GT_OK;
}


/**
* @internal prvCpssDxChPortMTIMacNumCRCBytesSet function
* @endinternal
*
* @brief   Set number of CRC bytes for MTI MAC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X.
*                                  Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] crcBytesNum              - number of CRC bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChPortMTIMacNumCRCBytesSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   crcBytesNum
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      commandConfigRegAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;    /* Command Config register address */
    GT_U32      crcModeRegAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;          /* CRC Mode register address */
    GT_U32      txCrcRegAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;              /* TX CRC Enable Register address */
    GT_U32      regValue;       /* register value (for Command Config and CRC Mode) */
    GT_U32      txCrcRegValue;    /* register value  */
    GT_U32      txCrcRegOffset;    /* register offset */
    GT_U32      portMacNum; /* MAC port number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    PRV_CPSS_REG_DB_INFO_STC        regDbInfo;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_CAELUM_E
                                            | CPSS_ALDRIN_E | CPSS_AC3X_E
                                            | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);


    if (!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = prvCpssSip6RegDbInfoGet(devNum,portMacNum,PRV_CPSS_REG_DB_TYPE_MTI_MPFS_E,&regDbInfo);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
        regDbInfo.regDbIndex = 0;

    if((macType == PRV_CPSS_PORT_MTI_100_E) ||
       (macType == PRV_CPSS_PORT_MTI_USX_E)||
       (macType == PRV_CPSS_PORT_MTI_CPU_E) )

    {
        regValue = (crcBytesNum == 0) ? 1: 0;
        txCrcRegValue = (crcBytesNum == 0) ? 0: 1;

        if ( macType == PRV_CPSS_PORT_MTI_100_E )
        {
            commandConfigRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.commandConfig;
            crcModeRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.crcMode;
            if (!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)) {
                txCrcRegAddr = PRV_DXCH_REG1_UNIT_MPFS_MAC(devNum, regDbInfo.regDbIndex).SAUControl;
                txCrcRegOffset = 0;
            } else{
                txCrcRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_EXT.portControl;
                txCrcRegOffset = 8;
            }
        }
        else if(macType == PRV_CPSS_PORT_MTI_USX_E)
        {
            commandConfigRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.commandConfig;
            crcModeRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.crcMode;
            /* Not relevant to Falcon so we will not get here */
            txCrcRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_EXT.portControl;
            txCrcRegOffset = 8;
        }
        else
        {
            commandConfigRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.commandConfig;
            crcModeRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.crcMode;
            if (!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                txCrcRegAddr = PRV_DXCH_REG1_UNIT_MPFS_CPU_MAC(devNum, regDbInfo.regDbIndex).SAUControl;
                txCrcRegOffset = 0;
            }
            else
            {
                txCrcRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_EXT.portControl;
                txCrcRegOffset = 9;
            }
        }

        if ((commandConfigRegAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED) ||
            (crcModeRegAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED) ||
            (txCrcRegAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED) )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        /* CRC_FWD */
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,
                   PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                              portMacNum),
                                                                              commandConfigRegAddr, 6, 1, regValue);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* DISABLE_RX_CRC */
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,
                 PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                                                                            crcModeRegAddr, 16, 1, regValue);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* TX_CRC_ENA / FF_TX_CRC */
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,
                 PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                                                                            txCrcRegAddr, txCrcRegOffset, 1, txCrcRegValue);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortMTIMacNumCRCBytesGet function
* @endinternal
*
* @brief   Get number of CRC bytes for MTI MAC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X.
*                                  Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] crcBytesNumPtr           - pointer to number of CRC bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChPortMTIMacNumCRCBytesGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_PORT_TYPE_ENT   macType,
    OUT GT_U32                  *crcBytesNumPtr
)
{
    GT_STATUS   rc = GT_OK;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      portMacNum; /* MAC port number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_CAELUM_E
                                            | CPSS_ALDRIN_E | CPSS_AC3X_E
                                            | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(crcBytesNumPtr);

    if((macType == PRV_CPSS_PORT_MTI_100_E) ||
       (macType == PRV_CPSS_PORT_MTI_USX_E) ||
       ((macType == PRV_CPSS_PORT_MTI_CPU_E) && PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
    {
        /* CRC_FWD */
        if(macType == PRV_CPSS_PORT_MTI_100_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.commandConfig;
        }
        else if(macType == PRV_CPSS_PORT_MTI_USX_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.commandConfig;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.commandConfig;
        }

        if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,
                 PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                                                                            regAddr, 6, 1, &regValue);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        *crcBytesNumPtr = (regValue == 1) ? 0 : 4;
    }
    else
    {
        /*Not suported , returning default */
        *crcBytesNumPtr = 4;
    }

    return rc;
}
