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
* @file cpssPxDiagPacketGenerator.c
*
* @brief Diag Packet Generator APIs for CPSS Px.
*
* Sequence for generating packets on port:
* 1. Connect the port to packet generator and set configurations
* (cpssPxDiagPacketGeneratorConnectSet)
* 2. Start transmission (cpssPxDiagPacketGeneratorTransmitEnable)
* 3. If single burst transmit mode used:
* Check transmission done status
* (cpssPxDiagPacketGeneratorBurstTransmitStatusGet)
* If continues transmit mode used:
* Stop transmission (cpssPxDiagPacketGeneratorTransmitEnable)
* 4. Return the port to normal operation, disconnect the packet generator
* (cpssPxDiagPacketGeneratorConnectSet)
*
* @version   1
********************************************************************************
*/

#define  CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/px/diag/cpssPxDiagPacketGenerator.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/port/private/prvCpssPortTypes.h>
#include <cpss/px/diag/private/prvCpssPxDiagLog.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/private/prvCpssPxPortIfModeCfgPipeResource.h>


/* Macro that return packGenConfig for current device and port MAC number */
#define  PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum)  \
    (PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->packGenConfig[portMacNum / 4])


/**
* @internal prvCpssPxDiagPacketGeneratorConnectedStateGet function
* @endinternal
*
* @brief   Get connect State (enable to port/disable)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portMacNum               - port MAC number
*
* @param[out] isCurrPortEnablePtr      - enabled/disabled state of packet generator
*                                      for current port.
* @param[out] isCurrPortGroupEnablePtr - enebled/disabled state of packet generator
*                                      for current port group (Pipe use 1 packet
*                                      generator per 4 ports)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number or port number
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssPxDiagPacketGeneratorConnectedStateGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U32                  portMacNum,
    OUT GT_BOOL                 *isCurrPortEnablePtr,
    OUT GT_BOOL                 *isCurrPortGroupEnablePtr
)
{
    GT_STATUS   rc;
    GT_U32      val;

    /* Read <Enable generator> field from <Control register 0> Register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
            0, 1, &val);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* if packet generator enabled for port group */
    if (val)
    {
        *isCurrPortGroupEnablePtr = GT_TRUE;
    }
    else
    {
        *isCurrPortGroupEnablePtr = GT_FALSE;
        *isCurrPortEnablePtr = GT_FALSE;

        return GT_OK;
    }

    /* Read <Connected port num> field from <Control register 0> Register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
            10, 3, &val);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* if packet generator connected to current port */
    if (val == (portMacNum % 4))
    {
        *isCurrPortEnablePtr = GT_TRUE;
    }
    else
    {
        *isCurrPortEnablePtr = GT_FALSE;
    }

    return GT_OK;
}


/**
* @internal prvCpssPxDiagPacketGeneratorConnectParamsCheck function
* @endinternal
*
* @brief   Check parameters of packet generator config
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] configPtr                - parames
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_BAD_PARAM             - on bad params
* @retval GT_OUT_OF_RANGE          - on param is out of range
*/
static GT_STATUS prvCpssPxDiagPacketGeneratorConnectParamsCheck
(
    IN  CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  *configPtr
)
{
    if ((configPtr->transmitMode != CPSS_DIAG_PG_TRANSMIT_CONTINUES_E) &&
        (configPtr->transmitMode != CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((configPtr->packetLengthType != CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E) &&
        (configPtr->packetLengthType != CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((configPtr->payloadType != CPSS_DIAG_PG_PACKET_PAYLOAD_CYCLIC_E) &&
        (configPtr->payloadType != CPSS_DIAG_PG_PACKET_PAYLOAD_RANDOM_E) &&
        (configPtr->payloadType != CPSS_DIAG_PG_PACKET_PAYLOAD_CONSTANT_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (configPtr->vlanTagEnable == GT_TRUE)
    {
        if (((configPtr->vid < 1) || (configPtr->vid > 4095)) ||
            (configPtr->vpt > 7) ||
            (configPtr->cfi > 1))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    if (configPtr->packetLengthType == CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E)
    {
        if ((configPtr->packetLength < 20) || (configPtr->packetLength > 16383))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    if (configPtr->transmitMode == CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E)
    {
        if ((configPtr->packetCount < 1) || (configPtr->packetCount > 8191))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if (configPtr->packetCountMultiplier > CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_16M_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if (configPtr->ifg > 0xFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssPxDiagPacketGeneratorEthAddrSet function
* @endinternal
*
* @brief   Set MAC DA/SA ethernet address
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ethAddr                  - MAC address
* @param[in] ethAddrRegistersArr      - MAC address register
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxDiagPacketGeneratorEthAddrSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_ETHERADDR    ethAddr,
    IN  GT_U32          *ethAddrRegistersArr
)
{
    GT_U32      value;
    GT_U32      i;
    GT_STATUS   rc;

    for (i = 0; i < 3; i++)
    {
        value = ((GT_U32) ethAddr.arEther[2 * i + 1]) |
                (((GT_U32) ethAddr.arEther[2 * i]) << 8);

        /* Set <MAC DA i> or <MAC SA i> register */
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), ethAddrRegistersArr[i], 0, 16, value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPxDiagPacketGeneratorEthAddrGet function
* @endinternal
*
* @brief   Get MAC DA/SA ethernet address
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ethAddr                  - MAC address registers
*                                      OUTPUT:
* @param[in] ethAddr                  - pointer to MAC address
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxDiagPacketGeneratorEthAddrGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          *ethAddrRegistersArr,
    OUT GT_ETHERADDR    *ethAddr
)
{
    GT_U32      value;
    GT_U32      i;
    GT_STATUS   rc;

    for (i = 0; i < 3; i++)
    {
        /* Get <MAC DA i> or <MAC SA i> register */
        rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), ethAddrRegistersArr[i], 0, 16, &value);
        if (rc != GT_OK)
        {
            return rc;
        }

        ethAddr->arEther[2 * i + 1] = (GT_U8) (value & 0xFF);
        ethAddr->arEther[2 * i]     = (GT_U8) ((value >> 8) & 0xFF);
    }

    return GT_OK;
}

/**
* @internal prvCpssPxDiagPacketGeneratorVlanTagSet function
* @endinternal
*
* @brief   Set Vlan tag to <VLAN Tag> packet generator register
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portMacNum               - port MAC number
* @param[in] configPtr                - pointer to valid packet generator configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssPxDiagPacketGeneratorVlanTagSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portMacNum,
    IN  CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  *configPtr
)
{
    GT_STATUS   rc;

    if (configPtr->vlanTagEnable)
    {
        /* Set VLAN Id to <VLAN Tag> register */
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).vlanTag,
                0, 12, configPtr->vid);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Set VLAN CFI to <VLAN Tag> register */
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).vlanTag,
                12, 1, configPtr->cfi);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Set VLAN priority to <VLAN Tag> register */
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).vlanTag,
                13, 3, configPtr->vpt);
    }
    else
    {
        /* Set zeros (no VLAN tag) to <VLAN Tag> register */
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).vlanTag,
                0, 16, 0);
    }

    return rc;
}

/**
* @internal prvCpssPxDiagPacketGeneratorInterfaceSizeSet function
* @endinternal
*
* @brief   Interface size set
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portMacNum               - port MAC number
* @param[in] configPtr                - pointer to valid packet generator configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
*/
static GT_STATUS prvCpssPxDiagPacketGeneratorInterfaceSizeSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              portMacNum,
    IN  CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  *configPtr
)
{
    GT_STATUS                           rc;
    GT_U32                              value;
    PRV_CPSS_PX_PIPE_PORT_RESOURCE_STC  resource;

    value = configPtr->interfaceSize;

    if (configPtr->interfaceSize == CPSS_DIAG_PG_IF_SIZE_DEFAULT_E)
    {
        cpssOsMemSet(&resource, 0, sizeof(resource));
        rc = prvCpssPxPipePortResourceConfigGet(
                devNum, portNum,/*OUT*/ &resource);
        if(rc != GT_OK)
        {
            return rc;
        }

        switch (resource.txfifoScdmaShiftersOutgoingBusWidth)
        {
            case PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_8B_E:
                value = CPSS_DIAG_PG_IF_SIZE_8_BYTES_E;         /* 8 bytes */
                break;
            case PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_32B_E:
                value = CPSS_DIAG_PG_IF_SIZE_32_BYTES_E;        /* 16 bytes */
                break;
            case PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_64B_E:
                value = CPSS_DIAG_PG_IF_SIZE_64_BYTES_E;        /* 64 bytes */
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
        }
    }

    /* Interface size */
    /* Set <Interface size> field of <Control register 0> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
            13, 3, value);

    return rc;
}

/**
* @internal internal_cpssPxDiagPacketGeneratorConnectSet function
* @endinternal
*
* @brief   Connect/Disconnect port to packet generator. Set packet generator's
*         configurations.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number to connect its MAC to packet
*                                      generator.
* @param[in] connect                  - GT_TRUE:   the given port's MAC to packet generator.
*                                      GT_FALSE: disconnect the given port's MAC from packet
*                                      generator; return the port's MAC to normal
*                                      egress pipe.
* @param[in] configPtr                - (pointer to) packet generator configurations. Relevant
*                                      only if enable == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*
* @note 1. In case packet generator connected to other port the function perfroms
*       the following:
*       - stops traffic.
*       - connects packet generator to new port
*       - overrides packet generator's configurations
*       2. Before enabling the packet generator, port must be initialized,
*       i.e. port interface and speed must be set.
*
*/
static GT_STATUS internal_cpssPxDiagPacketGeneratorConnectSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             connect,
    IN  CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  *configPtr
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      value;                  /* temporary variable */
    GT_U32      i;                      /* iterator */
    GT_U32      portMacNum;             /* port MAC number */
    GT_BOOL     isCurrPortGroupEnabled; /* is packet generator enabled for  */
                                        /* current group of ports (Pipe use */
                                        /* 1 PG per group of 4 ports)       */
    GT_BOOL     isCurrPortEnabled;      /* is packet generator enabled for */
                                        /* current port                    */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* Get the current state */
    rc = prvCpssPxDiagPacketGeneratorConnectedStateGet(devNum, portMacNum,
            &isCurrPortEnabled, &isCurrPortGroupEnabled);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (connect == GT_FALSE)
    {
        if (isCurrPortEnabled)
        {
            /* Disable PG */
            /* Set <Enable generator> field of <Control register 0> register */
            rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
                    PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
                    0, 1, 0);
            return rc;
        }
        else
        {
            return GT_OK;
        }
    }

    CPSS_NULL_PTR_CHECK_MAC(configPtr);

    /* Check packet generator config parameters */
    rc = prvCpssPxDiagPacketGeneratorConnectParamsCheck(configPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* if some port from port group is connected to packet generator */
    if (isCurrPortGroupEnabled)
    {
        /* Stop traffic */
        /* Set <Stop trigger> field of <Control register 0> register */
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
                5, 1, 1);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* MAC DA */
    rc = prvCpssPxDiagPacketGeneratorEthAddrSet(devNum, configPtr->macDa,
                PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).macDa);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* MAC DA increment */
    /* Set <Increment MAC DA> field of <Control register 0> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
            7, 1, BOOL2BIT_MAC(configPtr->macDaIncrementEnable));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* MAC DA increment limit */
    /* Set <MAC DA Increment limit> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).macDaIncrementLimit,
            0, 16, configPtr->macDaIncrementLimit);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* MAC SA */
    rc = prvCpssPxDiagPacketGeneratorEthAddrSet(devNum, configPtr->macSa,
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).macSa);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Ethernet type */
    /* Set <EtherType> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).etherType,
            0, 16, configPtr->etherType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* VLAN tag */
    rc = prvCpssPxDiagPacketGeneratorVlanTagSet(devNum, portMacNum, configPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Packet length */
    /* Set <Packet length> field of <Packet length> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).packetLength,
            0, 14, configPtr->packetLength);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Packet payload type */
    /* Set <Data type> field of <Packet length> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).packetLength,
            14, 2, configPtr->payloadType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Packet length type */
    /* Set <Length type> field of <Control register 0> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
            2, 1, configPtr->packetLengthType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Undersized packets transmission */
    /* Set <Undersize enable> field of <Control register 0> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
            9, 1, BOOL2BIT_MAC(configPtr->undersizeEnable));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Packet transmission mode */
    /* Set <Burst type> field of <Control register 0> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
            1, 1, configPtr->transmitMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (configPtr->transmitMode == CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E)
    {
        /* Packet count */
        /* Set <Packet count> field of <Packet count> register */
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).packetCount,
                0, 13, configPtr->packetCount);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Packet multiplier */
        /* Set <Packet count multiplier> field of <Packet count> register */
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).packetCount,
                13, 3, configPtr->packetCountMultiplier);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Inter frame gap */
    /* Set <Inter frame gap> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).ifg,
            0, 16, configPtr->ifg);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Interface size */
    rc = prvCpssPxDiagPacketGeneratorInterfaceSizeSet(devNum, portNum,
                                                      portMacNum, configPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Data pattern */
    for (i = 0; i < 32; i++)
    {
        value = ((GT_U16) configPtr->cyclicPatternArr[i * 2]) << 8 |
                (((GT_U16) configPtr->cyclicPatternArr[1 + (i * 2)]));

        /* Set <Data pattern n> register */
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).dataPattern[i],
                0, 16, value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Number of port in port group */
    /* Set <Connected port num> field of <Control register 0> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
            10, 3, portMacNum % 4);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Enable connect */
    /* Set <Enable generator> field of <Control register 0> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
            0, 1, 1);

    return rc;
}

/**
* @internal cpssPxDiagPacketGeneratorConnectSet function
* @endinternal
*
* @brief   Connect/Disconnect port to packet generator. Set packet generator's
*         configurations.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number to connect its MAC to packet
*                                      generator.
* @param[in] connect                  - GT_TRUE:   the given port's MAC to packet generator.
*                                      GT_FALSE: disconnect the given port's MAC from packet
*                                      generator; return the port's MAC to normal
*                                      egress pipe.
* @param[in] configPtr                - (pointer to) packet generator configurations. Relevant
*                                      only if enable == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*
* @note 1. In case packet generator connected to other port the function perfroms
*       the following:
*       - stops traffic.
*       - connects packet generator to new port
*       - overrides packet generator's configurations
*       2. Before enabling the packet generator, port must be initialized,
*       i.e. port interface and speed must be set.
*
*/
GT_STATUS cpssPxDiagPacketGeneratorConnectSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             connect,
    IN  CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  *configPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPacketGeneratorConnectSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, connect, configPtr));

    rc = internal_cpssPxDiagPacketGeneratorConnectSet(devNum, portNum, connect, configPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, connect, configPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagPacketGeneratorConnectGet function
* @endinternal
*
* @brief   Get the connect status of specified port. Get packet generator's
*         configurations.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number to connect its MAC to packet
*                                      generator.
*
* @param[out] connectPtr               - (pointer to) port connection state
*                                      GT_TRUE:  port connected to packet generator.
*                                      GT_FALSE: port not connected to packet generator.
* @param[out] configPtr                - (pointer to) packet generator configurations.
*                                      Relevant only if enable == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxDiagPacketGeneratorConnectGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_BOOL                             *connectPtr,
    OUT CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  *configPtr
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      value;                  /* temporary variable */
    GT_U32      i;                      /* iterator */
    GT_U32      portMacNum;             /* port MAC number */
    GT_BOOL     isCurrPortGroupEnabled; /* is packet generator enabled for  */
                                        /* current group of ports (Pipe use */
                                        /* 1 PG per group of 4 ports)       */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(connectPtr);
    CPSS_NULL_PTR_CHECK_MAC(configPtr);

    /* Get the current state */
    rc = prvCpssPxDiagPacketGeneratorConnectedStateGet(devNum, portMacNum,
            connectPtr, &isCurrPortGroupEnabled);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* MAC DA */
    rc = prvCpssPxDiagPacketGeneratorEthAddrGet(devNum,
                PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).macDa,
                &(configPtr->macDa));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* MAC DA increment */
    /* Get <Increment MAC DA> field of <Control register 0> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
            7, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    configPtr->macDaIncrementEnable = BIT2BOOL_MAC(value);

    /* MAC DA increment limit */
    /* Get <MAC DA Increment limit> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).macDaIncrementLimit,
            0, 16, &(configPtr->macDaIncrementLimit));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* MAC SA */
    rc = prvCpssPxDiagPacketGeneratorEthAddrGet(devNum,
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).macSa,
            &(configPtr->macSa));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Ethernet type */
    /* Get <EtherType> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).etherType,
            0, 16, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    configPtr->etherType = (GT_U16) value;

    /* VLAN tag */
    /* Get VLAN Id from <VLAN Tag> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).vlanTag,
            0, 12, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    configPtr->vid = (GT_U16) value;

    /* Get VLAN CFI from <VLAN Tag> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).vlanTag,
            12, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    configPtr->cfi = (GT_U8) value;

    /* Get VLAN priority from <VLAN Tag> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).vlanTag,
            13, 3, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    configPtr->vpt = (GT_U8) value;

    configPtr->vlanTagEnable = ((configPtr->vid | configPtr->cfi | configPtr->vpt) == 0)
                               ? GT_FALSE : GT_TRUE;

    /* Packet length */
    /* Get <Packet length> field of <Packet length> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).packetLength,
            0, 14, &(configPtr->packetLength));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Packet payload type */
    /* Get <Data type> field of <Packet length> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).packetLength,
            14, 2, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    configPtr->payloadType = (CPSS_DIAG_PG_PACKET_PAYLOAD_TYPE_ENT) value;

    /* Packet length type */
    /* Get <Length type> field of <Control register 0> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
            2, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    configPtr->packetLengthType = (CPSS_DIAG_PG_PACKET_LENGTH_TYPE_ENT) value;

    /* Undersized packets transmission */
    /* Get <Undersize enable> field of <Control register 0> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
            9, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    configPtr->undersizeEnable = BIT2BOOL_MAC(value);

    /* Packet transmission mode */
    /* Get <Burst type> field of <Control register 0> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
            1, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    configPtr->transmitMode = (CPSS_DIAG_PG_TRANSMIT_MODE_ENT) value;

    if (configPtr->transmitMode == CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E)
    {
        /* Packet count */
        /* Get <Packet count> field of <Packet count> register */
        rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).packetCount,
                0, 13, &(configPtr->packetCount));
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Packet multiplier */
        /* Get <Packet count multiplier> field of <Packet count> register */
        rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).packetCount,
                13, 3, &value);
        if (rc != GT_OK)
        {
            return rc;
        }

        configPtr->packetCountMultiplier = (CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_ENT) value;
    }

    /* Inter frame gap */
    /* Get <Inter frame gap> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).ifg,
            0, 16, &(configPtr->ifg));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Interface size */
    /* Get <Interface size> field of <Control register 0> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
            13, 3, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    configPtr->interfaceSize = (CPSS_DIAG_PG_IF_SIZE_ENT) value;

    /* Data pattern */
    for (i = 0; i < 32; i++)
    {
        /* Get <Data pattern n> register */
        rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).dataPattern[i],
                0, 16, &value);
        if (rc != GT_OK)
        {
            return rc;
        }

        configPtr->cyclicPatternArr[i * 2] = (GT_U8) ((value >> 8) & 0xFF);
        configPtr->cyclicPatternArr[1 + (i * 2)] = (GT_U8) (value & 0xFF);
    }

    return GT_OK;
}

/**
* @internal cpssPxDiagPacketGeneratorConnectGet function
* @endinternal
*
* @brief   Get the connect status of specified port. Get packet generator's
*         configurations.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number to connect its MAC to packet
*                                      generator.
*
* @param[out] connectPtr               - (pointer to) port connection state
*                                      GT_TRUE:  port connected to packet generator.
*                                      GT_FALSE: port not connected to packet generator.
* @param[out] configPtr                - (pointer to) packet generator configurations.
*                                      Relevant only if enable == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagPacketGeneratorConnectGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_BOOL                             *connectPtr,
    OUT CPSS_PX_DIAG_PG_CONFIGURATIONS_STC  *configPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPacketGeneratorConnectGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, connectPtr, configPtr));

    rc = internal_cpssPxDiagPacketGeneratorConnectGet(devNum, portNum, connectPtr, configPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, connectPtr, configPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagPacketGeneratorTransmitEnable function
* @endinternal
*
* @brief   Enable/Disable(Start/Stop) transmission on specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE:   (start) transmission
*                                      GT_FALSE: disable (stop) transmission
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_STATE             - on port not connected to packet generator
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*
* @note 1. Before calling this function the port must be connected to packet
*       generator (cpssPxDiagPacketGeneratorConnectSet),
*       otherwise GT_BAD_STATE is returned.
*
*/
static GT_STATUS internal_cpssPxDiagPacketGeneratorTransmitEnable
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      portMacNum;             /* MAC number */
    GT_BOOL     isCurrPortGroupEnabled; /* is packet generator enabled for  */
                                        /* current group of ports (Pipe use */
                                        /* 1 PG per group of 4 ports)       */
    GT_BOOL     isCurrPortEnabled;      /* is packet generator enabled for */
                                        /* current port                    */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* Get the current state */
    rc = prvCpssPxDiagPacketGeneratorConnectedStateGet(devNum, portMacNum,
            &isCurrPortEnabled, &isCurrPortGroupEnabled);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* if current port not connected to packet generator */
    if (!isCurrPortEnabled)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if (enable == GT_TRUE)
    {
        /* Start transmission */
        /* Set <Start trigger> field of <Control register 0> register */
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
                4, 1, 1);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* Stop transmission */
        /* Set <Stop trigger> field of <Control register 0> register */
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
                5, 1, 1);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssPxDiagPacketGeneratorTransmitEnable function
* @endinternal
*
* @brief   Enable/Disable(Start/Stop) transmission on specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE:   (start) transmission
*                                      GT_FALSE: disable (stop) transmission
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_STATE             - on port not connected to packet generator
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port
*
* @note 1. Before calling this function the port must be connected to packet
*       generator (cpssPxDiagPacketGeneratorConnectSet),
*       otherwise GT_BAD_STATE is returned.
*
*/
GT_STATUS cpssPxDiagPacketGeneratorTransmitEnable
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPacketGeneratorTransmitEnable);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxDiagPacketGeneratorTransmitEnable(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxDiagPacketGeneratorBurstTransmitStatusGet function
* @endinternal
*
* @brief   Get burst transmission status on specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] burstTransmitDonePtr     - (pointer to) burst transmit done status
*                                      GT_TRUE:  burst transmission done
*                                      GT_FALSE: burst transmission not done
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on port not connected to packet generator
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Burst transmission status is clear on read.
*       2. Before calling this function the port must be connected to packet
*       generator (cpssPxDiagPacketGeneratorConnectSet),
*       otherwise GT_BAD_STATE is returned.
*
*/
static GT_STATUS internal_cpssPxDiagPacketGeneratorBurstTransmitStatusGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL              *burstTransmitDonePtr
)
{
    GT_U32      value;                  /* temporary variable */
    GT_STATUS   rc;                     /* return code */
    GT_U32      portMacNum;             /* MAC number */
    GT_BOOL     isCurrPortGroupEnabled;   /* is packet generator enabled for  */
                                        /* current group of ports (Pipe use */
                                        /* 1 PG per group of 4 ports)       */
    GT_BOOL     isCurrPortEnabled;        /* is packet generator enabled for */
                                        /* current port                    */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(burstTransmitDonePtr);

    /* Get status of transmission */
    /* Get <Burst transmit done> field of <Control register 0> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_PX_DIAG_PACK_GEN_CONFIG_MAC(devNum, portMacNum).controlReg0,
            6, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get the current connect state */
    rc = prvCpssPxDiagPacketGeneratorConnectedStateGet(devNum, portMacNum,
            &isCurrPortEnabled, &isCurrPortGroupEnabled);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* If current port not connected to packet generator */
    if (isCurrPortEnabled == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    *burstTransmitDonePtr = (GT_BOOL) value;

    return GT_OK;
}

/**
* @internal cpssPxDiagPacketGeneratorBurstTransmitStatusGet function
* @endinternal
*
* @brief   Get burst transmission status on specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] burstTransmitDonePtr     - (pointer to) burst transmit done status
*                                      GT_TRUE:  burst transmission done
*                                      GT_FALSE: burst transmission not done
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on port not connected to packet generator
* @retval GT_NOT_INITIALIZED       - on port is not initialized (interface/speed)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Burst transmission status is clear on read.
*       2. Before calling this function the port must be connected to packet
*       generator (cpssPxDiagPacketGeneratorConnectSet),
*       otherwise GT_BAD_STATE is returned.
*
*/
GT_STATUS cpssPxDiagPacketGeneratorBurstTransmitStatusGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL              *burstTransmitDonePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagPacketGeneratorBurstTransmitStatusGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, burstTransmitDonePtr));

    rc = internal_cpssPxDiagPacketGeneratorBurstTransmitStatusGet(devNum,
            portNum, burstTransmitDonePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, burstTransmitDonePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

