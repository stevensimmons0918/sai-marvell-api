/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file cpssDxChCscd.c
*
* @brief Cascading implementation -- Cheetah.
*
* @version   91
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/dxCh/dxChxGen/trunk/private/prvCpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/cscd/private/prvCpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMacCtrl.h>
#include <cpss/dxCh/dxChxGen/cscd/private/prvCpssDxChCscdLog.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqPreemptionUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>



/*******************************************************************************
 * External usage variables
 ******************************************************************************/

#define PRV_GLOBAL_PPDRV_HWINIT(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.hwInitDir.hwInitSrc._var)

/* number of registers 'per target device' in lion */
#define LION_DEV_MAP_TABLE_REG_PER_DEV_CNS  8

/* indication on 'not initialized'*/
#define NOT_INITIALIZED_CNS     0xFFFFFFFF

/* CPU port used as cascade in device map table. Relevant devices: up to xCat2 */
#define XCAT_DEV_MAP_TABLE_CPU_PORT_NUM_CNS     28

#define PRV_CPSS_CSCD_PORT_CRC_HW_TO_SW_VALUE_CONVERT_MAC(hwValue,swValue)  \
    switch(hwValue)                                                         \
    {                                                                       \
        case 1:                                                             \
            swValue = CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E;                   \
            break;                                                          \
        case 2:                                                             \
            swValue = CPSS_DXCH_CSCD_PORT_CRC_TWO_BYTES_E;                  \
            break;                                                          \
        case 3:                                                             \
            swValue = CPSS_DXCH_CSCD_PORT_CRC_THREE_BYTES_E;                \
            break;                                                          \
        case 4:                                                             \
            swValue = CPSS_DXCH_CSCD_PORT_CRC_FOUR_BYTES_E;                 \
            break;                                                          \
        case 0:                                                             \
            swValue = CPSS_DXCH_CSCD_PORT_CRC_ZERO_BYTES_E;                 \
            break;                                                          \
        default:                                                            \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);  \
    }

#define PRV_CPSS_CSCD_PORT_CRC_SW_TO_HW_VALUE_CONVERT_MAC(swValue,hwValue)  \
    switch(swValue)                                                         \
    {                                                                       \
        case CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E:                            \
            hwValue = 1;                                                    \
            break;                                                          \
        case CPSS_DXCH_CSCD_PORT_CRC_TWO_BYTES_E:                           \
            hwValue = 2;                                                    \
            break;                                                          \
        case CPSS_DXCH_CSCD_PORT_CRC_THREE_BYTES_E:                         \
            hwValue = 3;                                                    \
            break;                                                          \
        case CPSS_DXCH_CSCD_PORT_CRC_FOUR_BYTES_E:                          \
            hwValue = 4;                                                    \
            break;                                                          \
        case CPSS_DXCH_CSCD_PORT_CRC_ZERO_BYTES_E:                          \
            hwValue = 0;                                                    \
            break;                                                          \
        default:                                                            \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);  \
    }

#define NUM_BITS_FOR_LINK_NUM_MAC(_devNum)       \
   (PRV_CPSS_SIP_6_10_CHECK_MAC(_devNum) ?  8 :/* 8 for trunkId , 7 for portNum */  \
    PRV_CPSS_SIP_6_CHECK_MAC(_devNum)    ? 10 :  \
    PRV_CPSS_SIP_5_20_CHECK_MAC(_devNum) ?  9 :  \
    PRV_CPSS_SIP_5_10_CHECK_MAC(_devNum) ?  8 :  \
    12)


typedef struct
{
    GT_U16 xsmiAddr;
    GT_U16 xsmiData;
} XSMI_STC;


/**
* @internal prvCpssDxChPortProfileValidate function
* @endinternal
*
* @brief   Validate that port profile is applicable to port .
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
 @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - port's direction.
* @param[in] portProfile              - port's profile values.
*
* @retval GT_OK                    -                     on success
* @retval GT_NOT_SUPPORTED             -profile configuration is not supported for this port
*/

static GT_STATUS prvCpssDxChPortProfileValidate
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    IN    CPSS_PORT_DIRECTION_ENT    portDirection,
    IN    CPSS_DXCH_PORT_PROFILE_ENT portProfile
)

{
    GT_STATUS rc = GT_OK;
    GT_BOOL   isAllowed;

    /*This is not AC5P device*/
    if(!PRV_CPSS_PP_MAC(devNum)->preemptionSupported)
    {
        if(portProfile == CPSS_DXCH_PORT_PROFILE_NETWORK_PREEMPTIVE_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "PROFILE_NETWORK_PREEMPTIVE supported only for AC3P devices\n");
        }
    }
    else
    {

        if(portProfile == CPSS_DXCH_PORT_PROFILE_NETWORK_PREEMPTIVE_E)
        {
          if(portDirection!=CPSS_PORT_DIRECTION_TX_E)
          {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "PROFILE_NETWORK_PREEMPTIVE supported only for TX direction\n");
          }
            /*Check if preemption can be activated on this port*/
          rc = prvCpssFalconTxqUtilsPortPreemptionAllowedGet(devNum,portNum,&isAllowed);
          if(rc==GT_OK)
          {
            if(GT_FALSE==isAllowed)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Physical port %d cannot work in preemptive mode\n",portNum);
            }
          }
        }
    }

    return rc;
}

/**************************** Forward declaration******************************/

/**
* @internal prvCpssDxChIngressCscdPortSet function
* @endinternal
*
* @brief   Set ingress port as cascaded/non-cascade .
*         (effect all traffic on ingress pipe only)
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - The physical port number.
*                                      NOTE: this port is not covered and is NOT checked for 'mapping'
* @param[in] portRxDmaNum             - The RXDMA number for cascading.
* @param[in] portType                 - cascade  type regular/extended DSA tag port or network port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChIngressCscdPortSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_U32                       portRxDmaNum,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
)
{
    GT_STATUS       rc;
    GT_U32          value;       /* value of field */
    GT_U32          fieldOffset; /* The start bit number in the register */
    GT_U32          regAddr;     /* address of register */
    GT_U32          portGroupId;      /*the port group Id - support multi port groups device */
    GT_U32           localPort;   /* local port - support multi-port-group device */

    if (0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* not applicable for those devices */
        return GT_OK;
    }

    if(portType == CPSS_CSCD_PORT_NETWORK_E)
    {
        value = 0;
    }
    else
    {
        value = 1;
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* Used to enable Multicast filtering over cascade ports even if the
           packet is assigned <Phy Src MC Filter En> = 0.
           This value should be set to 1 over cascade ports. */
        rc = prvCpssDxChHwEgfShtFieldSet(devNum,portNum,
            PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_IGNORE_PHY_SRC_MC_FILTER_E,
                GT_TRUE, /*accessPhysicalPort*/
                GT_FALSE, /*accessEPort*/
                (portType == CPSS_CSCD_PORT_NETWORK_E) ? 0 : 1);

        if (rc != GT_OK)
        {
            return rc;
        }

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi port group device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portRxDmaNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portRxDmaNum);

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            fieldOffset = 0;
            regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).
                configs.channelConfig.channelCascadePort[localPort];
        }
        else
        {
            /* each RXDMA port hold it's own registers , so bit index is not parameter of port number */
            fieldOffset = 3;

            if(localPort >= (sizeof(PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAConfigs.SCDMAConfig0) /
                             sizeof(PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAConfigs.SCDMAConfig0[0])))
            {
                /* out of range value */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                singleChannelDMAConfigs.SCDMAConfig0[localPort];
        }
    }
    else
    {
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi port group device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        fieldOffset = (localPort == CPSS_CPU_PORT_NUM_CNS) ? 15 : localPort;

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                           cutThroughRegs.ctCascadingPortReg;
    }

    /* Enable/disable DSA tag recognition on rxDma of ingress port */
    return  prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,
                               regAddr, fieldOffset, 1, value);
}

/**
* @internal prvCpssDxChIngressCscdPortGet function
* @endinternal
*
* @brief   Get if ingress port set as cascaded/non-cascade .
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - The port to be configured for Cut Through forwarding
* @param[in] portRxDmaNum             - The RXDMA number for cascading.
*
* @param[out] enablePtr                - (pointer to) is the port enabled to recognize incoming packets
*                                      as DSA tag packets or as non-DSA tag packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChIngressCscdPortGet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_U32                       portRxDmaNum,
    OUT GT_BOOL                      *enablePtr
)
{
    GT_STATUS       rc;
    GT_U32          value;       /* value of field */
    GT_U32          fieldOffset; /* The start bit number in the register */
    GT_U32          regAddr;     /* address of register */
    GT_U32          portGroupId;      /*the port group Id - support multi port groups device */
    GT_U32           localPort;   /* local port - support multi-port-group device */

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi port group device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portRxDmaNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portRxDmaNum);

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            fieldOffset = 0;
            regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).
                configs.channelConfig.channelCascadePort[localPort];
        }
        else
        {
            /* each RXDMA port hold it's own registers , so bit index is not parameter of port number */
            fieldOffset = 3;

            if(localPort >= (sizeof(PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAConfigs.SCDMAConfig0) /
                             sizeof(PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAConfigs.SCDMAConfig0[0])))
            {
                /* out of range value */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).
                singleChannelDMAConfigs.SCDMAConfig0[localPort];
        }
    }
    else
    {
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi port group device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        fieldOffset = (localPort == CPSS_CPU_PORT_NUM_CNS) ? 15 : localPort;

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                           cutThroughRegs.ctCascadingPortReg;
    }

    /* Enable Cut Through forwarding on cascaded port */
    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,
                               regAddr, fieldOffset, 1, &value);


    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal prvCpssDxChEgressCscdPortSet function
* @endinternal
*
* @brief   Set egress port as cascaded/non-cascade .
*         (effect all traffic that egress the physical port)
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - The physical port number.
*                                      NOTE: this port is not covered and is NOT checked for 'mapping'
* @param[in] portMacNum               - The MAC number for cascading.
* @param[in] portType                 - cascade  type regular/extended DSA tag port or network port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChEgressCscdPortSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_U32                       portMacNum,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
)
{
    GT_STATUS rc;
    GT_U32          regAddr;  /* address of register */
    GT_U32          value;    /* value of field */
    GT_U32          index;    /* index of bit in the register */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_BOOL supportPfc;/*indication if the device support PFC*/
    GT_BOOL enablePfcCascade; /* indication to enable the PFC on cascade port */
    CPSS_DXCH_PORT_FC_MODE_ENT portFcMode;
    GT_U32   localPort;/* local port - support multi-port-groups device */
    GT_BOOL  isCpuPort;      /* is Cpu Port */

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        portGroupId = 0;/* not used */
        index = 0;/* not used */
    }
    else
    {
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        index = (localPort == CPSS_CPU_PORT_NUM_CNS) ?
                     PRV_CPSS_DXCH_CH1_SPECIAL_BIT_HA_CPU_PORT_BIT_INDEX_MAC(devNum,27) :
                     localPort;
    }

    if(portType != CPSS_CSCD_PORT_NETWORK_E)
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* Cascade port enable */
            rc = prvCpssDxChWriteTableEntryField(devNum,
                                                  CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                                  portNum, /*global port*/
                                                  PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                  SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_CASCADE_PORT_ENABLE_E, /* field name */
                                                  PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                  1);

            if(rc != GT_OK)
                return rc;

            switch(portType)
            {
                case CPSS_CSCD_PORT_DSA_MODE_1_WORD_E:
                    value = 0;
                    break;
                case CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E:
                    value = 1;
                    break;
                case CPSS_CSCD_PORT_DSA_MODE_3_WORDS_E:
                    value = 2;
                    break;
                case CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E:
                    value = 3;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* set DSA support mode */
            rc = prvCpssDxChWriteTableEntryField(devNum,
                                                  CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                                  portNum, /*global port*/
                                                  PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                  SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EGRESS_DSA_TAG_TYPE_E, /* field name */
                                                  PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                  value);
            if (rc != GT_OK)
                  return rc;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.
                        cscdHeadrInsrtConf[OFFSET_TO_WORD_MAC(index)];

            /* The port is a cascading port used for inter-device connectivity and
               all packets received by it and transmitted by it must be DSA-tagged*/
            if (prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, OFFSET_TO_BIT_MAC(index), 1, 1) != GT_OK)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);


            /* set DSA support mode */
            value = (portType == CPSS_CSCD_PORT_DSA_MODE_1_WORD_E) ? 0 : 1;

            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.
                eggrDSATagTypeConf[OFFSET_TO_WORD_MAC(index)];

            /* set relevant bit of Egress DSA tag type Configuration register */
            if (prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, OFFSET_TO_BIT_MAC(index), 1, value) != GT_OK)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }

    }
    else
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* Cascade port disable */
            rc = prvCpssDxChWriteTableEntryField(devNum,
                                                  CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                                  portNum, /*global port*/
                                                  PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                  SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_CASCADE_PORT_ENABLE_E, /* field name */
                                                  PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                  0);
            if (rc != GT_OK)
                  return rc;

        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.
                        cscdHeadrInsrtConf[OFFSET_TO_WORD_MAC(index)];

            /* The port is a regular network port and all packets received by it
               are standard Ethernet packets.  */
            if (prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, OFFSET_TO_BIT_MAC(index), 1, 0) != GT_OK)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }

    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {

        rc = prvCpssDxChPortPhysicalPortIsCpu(devNum, portNum, &isCpuPort);
        if (rc != GT_OK)
              return rc;

        if (isCpuPort == GT_TRUE)
        {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE)
            {
                /* there is no MAC for this CPU port , so can't set PFC  */
                supportPfc = GT_FALSE;
            }
            else
            {
                 supportPfc = GT_TRUE;
            }
        }
        else
        {
            /* check PRV_CPSS_DXCH_PORT_TYPE_MAC DB boundaries */
            if (portMacNum >= PRV_CPSS_PP_MAC(devNum)->numOfPorts)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            if (PRV_CPSS_PORT_HGL_E == PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum))
            {
                /* HGL port do not support PFC */
                supportPfc = GT_FALSE;
            }
            else
            {
                supportPfc = GT_TRUE;
            }
        }
    }
    else
    {
        supportPfc = GT_FALSE;
    }

    if(prvCpssDxChPortRemotePortCheck(devNum,portNum))
    {
        /* only local ports should set the PFC and not 'many to 1 '*/
        supportPfc = GT_FALSE;
    }

    if (supportPfc == GT_TRUE)
    {
        /* for cascade port and pfc mode, enable insertion of DSA tag to PFC packets */
        if (portType != CPSS_CSCD_PORT_NETWORK_E)
        {
            rc = cpssDxChPortFlowControlModeGet(devNum,portNum,&portFcMode);
            if (rc != GT_OK)
            {
                return rc;
            }
            enablePfcCascade = (portFcMode == CPSS_DXCH_PORT_FC_MODE_PFC_E)
               ? GT_TRUE : GT_FALSE;
        }
        else
        {
            enablePfcCascade = GT_FALSE;
        }

        rc = prvCpssDxChPortPfcCascadeEnableSet(devNum,portNum,enablePfcCascade);
        if (rc != GT_OK)
        {
            return rc;
        }
    }/*supportPfc == GT_TRUE*/


    return GT_OK;
}

/**
* @internal prvCpssDxChCscdPortSet function
* @endinternal
*
* @brief   Set ingress and egress port as cascaded/non-cascade .
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - The physical port number.
*                                      NOTE: this port is not covered and is NOT checked for 'mapping'
* @param[in] portRxDmaNum             - The RXDMA number for cascading.
* @param[in] doIngress                - indication to configure the ingress part
* @param[in] doEgress                 - indication to configure the egress part
* @param[in] portType                 - cascade  type regular/extended DSA tag port or network port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCscdPortSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_U32                       portRxDmaNum,
    IN GT_U32                       doIngress,
    IN GT_U32                       doEgress,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
)
{
    GT_STATUS rc;

    if(doIngress)
    {
        /* set the ingress port to be 'cascade' enable/disabled */
        rc = prvCpssDxChIngressCscdPortSet(devNum, portNum , portRxDmaNum, portType);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(doEgress)
    {
        /* set the egress port to be 'cascade' enable/disabled */
        rc = prvCpssDxChEgressCscdPortSet(devNum, portNum , portRxDmaNum/*this is also the 'MAC num'*/ ,portType);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChCscdPortTypeSet function
* @endinternal
*
* @brief   Configure a PP port to be a cascade port. Application is responsible
*         for setting the default values of the port.
*         APPLICABLE DEVICES:
*         xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - physical device number
*         portNum - The port to be configured as cascade
*         portDirection  - port's direction.
*         (APPLICABLE DEVICES Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.)
*         portType - cascade type DSA tag port or network port
*         OUTPUTS:
*         None
*         RETURNS:
*         GT_OK          - on success
*         GT_BAD_PARAM       - on wrong port number or device
*         GT_HW_ERROR       - on hardware error
*         GT_NOT_APPLICABLE_DEVICE - on not applicable device
*         COMMENTS:
*         1. This function also configures insertion of DSA tag for PFC frames.
*         Relevant to egress direction.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - The port to be configured as cascade
* @param[in] portDirection            - port's direction.
*                                      (APPLICABLE DEVICES Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.)
* @param[in] portType                 - cascade  type DSA tag port or network port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function also configures insertion of DSA tag for PFC frames.
*       Relevant to egress direction.
*       APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
*/
static GT_STATUS internal_cpssDxChCscdPortTypeSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_PORT_DIRECTION_ENT      portDirection,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
)
{
    GT_STATUS   rc;
    GT_BOOL supportDirection;/* indication if the device support 'direction' for the cascading */
    GT_U32 doIngress,doEgress;/*indication about the directions to do : ingress,egress */
    GT_U32  portDmaNum;/* the DMA number for the rxDMA and txDMA of the physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        supportDirection = GT_TRUE;
    }
    else
    {
        supportDirection = GT_FALSE;
    }

    if(supportDirection == GT_FALSE)
    {
        portDirection = CPSS_PORT_DIRECTION_BOTH_E;
    }

    switch(portDirection)
    {
        case CPSS_PORT_DIRECTION_RX_E:
            doIngress = 1;
            doEgress  = 0;
            break;
        case CPSS_PORT_DIRECTION_TX_E:
            doIngress = 0;
            doEgress  = 1;
            break;
        case CPSS_PORT_DIRECTION_BOTH_E:
            doIngress = 1;
            doEgress  = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(portType)
    {
        case CPSS_CSCD_PORT_DSA_MODE_1_WORD_E :
        case CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E:
        case CPSS_CSCD_PORT_NETWORK_E:
            break;

        case CPSS_CSCD_PORT_DSA_MODE_3_WORDS_E:
        case CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }

            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* check remote port because 'SDMA port' not have MAC */
    if(prvCpssDxChPortRemotePortCheck(devNum,portNum))
    {
        CPSS_MACDRV_OBJ_STC * portMacObjPtr;  /* port object pointer */
        GT_U32              portMacNum;      /* MAC number */

        /* Get PHY MAC object ptr */
        PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
        portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

        /* check if MACPHY callback should run */
        if ((portMacObjPtr != NULL) &&
            (portMacObjPtr->macDrvMacCscdPortTypeSetFunc))
        {
            GT_BOOL doPpMacConfig = GT_TRUE;

            rc = portMacObjPtr->macDrvMacCscdPortTypeSetFunc(devNum,portNum,
                                      CPSS_MACDRV_STAGE_PRE_E,
                                      &doPpMacConfig,
                                      portDirection,
                                      portType);
            if(rc!=GT_OK)
            {
                return rc;
            }

            if(doPpMacConfig == GT_FALSE)
            {
                return GT_OK;
            }
        }

    }

    /* default value */
    portDmaNum = portNum;

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);
    }
    else
    {
        /* supported physical ports that hold rxDMA (physical port mapped to rxDma) */
        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_RXDMA_NUM_GET_MAC(devNum,portNum,portDmaNum);

        if(doEgress)
        {
            /* all range of physical ports supported (0..255) */
            PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
        }
    }

    /* call the function that do no port conversions / port checking !!! */
    rc = prvCpssDxChCscdPortSet(devNum, portNum , portDmaNum,doIngress,doEgress, portType);

    return rc;
}

/**
* @internal cpssDxChCscdPortTypeSet function
* @endinternal
*
* @brief   Configure a PP port to be a cascade port. Application is responsible
*         for setting the default values of the port.
*         APPLICABLE DEVICES:
*         xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - physical device number
*         portNum - The port to be configured as cascade
*         portDirection  - port's direction.
*         (APPLICABLE DEVICES Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.)
*         portType - cascade type DSA tag port or network port
*         OUTPUTS:
*         None
*         RETURNS:
*         GT_OK          - on success
*         GT_BAD_PARAM       - on wrong port number or device
*         GT_HW_ERROR       - on hardware error
*         GT_NOT_APPLICABLE_DEVICE - on not applicable device
*         COMMENTS:
*         1. This function also configures insertion of DSA tag for PFC frames.
*         Relevant to egress direction.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - The port to be configured as cascade
* @param[in] portDirection            - port's direction.
*                                      (APPLICABLE DEVICES Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.)
* @param[in] portType                 - cascade  type DSA tag port or network port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function also configures insertion of DSA tag for PFC frames.
*       Relevant to egress direction.
*       APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
*/
GT_STATUS cpssDxChCscdPortTypeSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_PORT_DIRECTION_ENT      portDirection,
    IN CPSS_CSCD_PORT_TYPE_ENT      portType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortTypeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portDirection, portType));

    rc = internal_cpssDxChCscdPortTypeSet(devNum, portNum, portDirection, portType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portDirection, portType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdPortTypeGet function
* @endinternal
*
* @brief   Retrieve a PP port cascade port configuration.
*         APPLICABLE DEVICES:
*         xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - physical device number
*         portNum - port number or CPU port
*         portDirection  - port's direction.
*         (APPLICABLE DEVICES Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.)
*         OUTPUTS:
*         portTypePtr - (pointer to) cascade type DSA tag port or network port.
*         RETURNS:
*         GT_OK          - on success
*         GT_BAD_PARAM       - on wrong port number or device
*         GT_BAD_STATE       - when portDirection = 'both' but ingress value
*         conflicts the egress value.
*         GT_HW_ERROR       - on hardware error
*         GT_BAD_PTR        - one of the parameters is NULL pointer
*         GT_NOT_APPLICABLE_DEVICE - on not applicable device
*         COMMENTS:
*         1. When portDirection = 'Ingress direction' , the (portTypePtr) will get value of:
*         CPSS_CSCD_PORT_NETWORK_E - network port indication.
*         CPSS_CSCD_PORT_DSA_MODE_1_WORD_E - cascade port indication.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number or CPU port
* @param[in] portDirection            - port's direction.
*                                      (APPLICABLE DEVICES Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.)
*
* @param[out] portTypePtr              - (pointer to) cascade type DSA tag port or network port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_STATE             - when portDirection = 'both' but ingress value
*                                       conflicts the egress value.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. When portDirection = 'Ingress direction' , the (portTypePtr) will get value of:
*       CPSS_CSCD_PORT_NETWORK_E - network port indication.
*       CPSS_CSCD_PORT_DSA_MODE_1_WORD_E - cascade port indication.
*       APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
*/
static GT_STATUS internal_cpssDxChCscdPortTypeGet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_PORT_DIRECTION_ENT      portDirection,
    OUT CPSS_CSCD_PORT_TYPE_ENT     *portTypePtr
)
{
    GT_STATUS       rc;       /* return code */
    GT_U32          regAddr;  /* address of register */
    GT_U32          value;    /* value of field */
    GT_U32          index;    /* index of bit in the register */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */
    GT_BOOL supportDirection;/* indication if the device support 'direction' for the cascading */
    GT_U32 doIngress,doEgress;/*indication about the directions to do : ingress,egress */
    GT_BOOL ingressCascadeEnable;/*indication that the port enabled for ingress cascading */
    GT_U32  portRxDmaNum;/* the dma number for the rxDma of the physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(portTypePtr);

    if (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        supportDirection = GT_TRUE;
    }
    else
    {
        supportDirection = GT_FALSE;
    }

    if(supportDirection == GT_FALSE)
    {
        /* for legacy devices get info from the egress only */
        portDirection = CPSS_PORT_DIRECTION_TX_E;
    }

    switch(portDirection)
    {
        case CPSS_PORT_DIRECTION_RX_E:
            doIngress = 1;
            doEgress  = 0;
            break;
        case CPSS_PORT_DIRECTION_TX_E:
            doIngress = 0;
            doEgress  = 1;
            break;
        case CPSS_PORT_DIRECTION_BOTH_E:
            doIngress = 1;
            doEgress  = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* check remote port because 'SDMA port' not have MAC */
    if(prvCpssDxChPortRemotePortCheck(devNum,portNum))
    {
        CPSS_MACDRV_OBJ_STC * portMacObjPtr;  /* port object pointer */
        GT_U32          portMacNum;      /* MAC number */

        /* Get PHY MAC object ptr */
        PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
        portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

        /* check if MACPHY callback should run */
        if ((portMacObjPtr != NULL) &&
            (portMacObjPtr->macDrvMacCscdPortTypeGetFunc))
        {
            GT_BOOL doPpMacConfig = GT_TRUE;

            rc = portMacObjPtr->macDrvMacCscdPortTypeGetFunc(devNum,portNum,
                                      CPSS_MACDRV_STAGE_PRE_E,
                                      &doPpMacConfig,
                                      portDirection,
                                      portTypePtr);
            if(rc!=GT_OK)
            {
                return rc;
            }

            if(doPpMacConfig == GT_FALSE)
            {
                return GT_OK;
            }
        }

    }

    /* default value */
    portRxDmaNum = portNum;

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);
    }
    else
    {
        if(doIngress)
        {
            /* supported physical ports that hold rxDMA (physical port mapped to rxDma) */
            PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_RXDMA_NUM_GET_MAC(devNum,portNum,portRxDmaNum);
        }

        if(doEgress)
        {
            /* all range of physical ports supported (0..255) */
            PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
        }
    }

    if(doEgress)
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            portGroupId = 0;/* not used */
            index = 0;/* not used */
        }
        else
        {
            /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
            portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
            localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

            index = (localPort == CPSS_CPU_PORT_NUM_CNS) ?
                         PRV_CPSS_DXCH_CH1_SPECIAL_BIT_HA_CPU_PORT_BIT_INDEX_MAC(devNum,27) :
                         localPort;
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* Cascade port enable */
            rc = prvCpssDxChReadTableEntryField(devNum,
                                                  CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                                  portNum, /*global port*/
                                                  PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                  SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_CASCADE_PORT_ENABLE_E, /* field name */
                                                  PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                  &value);
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.
                        cscdHeadrInsrtConf[OFFSET_TO_WORD_MAC(index)];
            rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, OFFSET_TO_BIT_MAC(index), 1, &value);
        }

        if( rc != GT_OK )
        {
            return rc;
        }

        if( value == 0)
        {
            *portTypePtr = CPSS_CSCD_PORT_NETWORK_E;
        }
        else /* value == 1 */
        {
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
            {
                /* get DSA support mode */
                rc = prvCpssDxChReadTableEntryField(devNum,
                                                      CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                                      portNum, /*global port*/
                                                      PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                      SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EGRESS_DSA_TAG_TYPE_E, /* field name */
                                                      PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                      &value);
                if(rc != GT_OK)
                    return rc;

                switch(value)
                {
                    case 0:
                        *portTypePtr = CPSS_CSCD_PORT_DSA_MODE_1_WORD_E;
                        break;
                    case 1:
                        *portTypePtr = CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E;
                        break;
                    case 2:
                        *portTypePtr = CPSS_CSCD_PORT_DSA_MODE_3_WORDS_E;
                        break;
                    case 3:
                        *portTypePtr = CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E;
                        break;
                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                 }
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.
                    eggrDSATagTypeConf[OFFSET_TO_WORD_MAC(index)];
                rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, OFFSET_TO_BIT_MAC(index), 1, &value);
                if( rc != GT_OK )
                {
                    return rc;
                }

                if( value == 0 )
                {
                    *portTypePtr = CPSS_CSCD_PORT_DSA_MODE_1_WORD_E;
                }
                else
                {
                    *portTypePtr = CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E;
                }
            }
        }
    }

    if(doIngress)
    {
        rc = prvCpssDxChIngressCscdPortGet(devNum, portNum, portRxDmaNum ,&ingressCascadeEnable);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(doEgress)
        {
            /* check that ingress and egress are synch , when we need to get value for 'both' */
            if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
            {
                if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                {
                    /* the RxDma unit is not implemented at this stage in GM */
                    /* so the 'ingressCascadeEnable' is reading '0x000badad' from the 'register' */
                    /* and is set to 'GT_TRUE' ... so do not check it */
                    return GT_OK;
                }
            }

            if((ingressCascadeEnable == GT_TRUE && ((*portTypePtr) == CPSS_CSCD_PORT_NETWORK_E)))
            {
                /* the ingress state - cascade port
                   the egress state  - network port */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            if((ingressCascadeEnable == GT_FALSE && ((*portTypePtr) != CPSS_CSCD_PORT_NETWORK_E)))
            {
                /* the ingress state - network port
                   the egress state  - cascade port */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

        }
        else  /* egress was not queried , so the only indication is network port / cascade port */
        {
            (*portTypePtr) = (ingressCascadeEnable == GT_TRUE) ?
                            CPSS_CSCD_PORT_DSA_MODE_1_WORD_E : /* indication that port is 'cascade' */
                            CPSS_CSCD_PORT_NETWORK_E;          /* port is network (non-cascade) */
        }
    }


    return GT_OK;
}

/**
* @internal cpssDxChCscdPortTypeGet function
* @endinternal
*
* @brief   Retrieve a PP port cascade port configuration.
*         APPLICABLE DEVICES:
*         xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - physical device number
*         portNum - port number or CPU port
*         portDirection  - port's direction.
*         (APPLICABLE DEVICES Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.)
*         OUTPUTS:
*         portTypePtr - (pointer to) cascade type DSA tag port or network port.
*         RETURNS:
*         GT_OK          - on success
*         GT_BAD_PARAM       - on wrong port number or device
*         GT_BAD_STATE       - when portDirection = 'both' but ingress value
*         conflicts the egress value.
*         GT_HW_ERROR       - on hardware error
*         GT_BAD_PTR        - one of the parameters is NULL pointer
*         GT_NOT_APPLICABLE_DEVICE - on not applicable device
*         COMMENTS:
*         1. When portDirection = 'Ingress direction' , the (portTypePtr) will get value of:
*         CPSS_CSCD_PORT_NETWORK_E - network port indication.
*         CPSS_CSCD_PORT_DSA_MODE_1_WORD_E - cascade port indication.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number or CPU port
* @param[in] portDirection            - port's direction.
*                                      (APPLICABLE DEVICES Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.)
*
* @param[out] portTypePtr              - (pointer to) cascade type DSA tag port or network port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_STATE             - when portDirection = 'both' but ingress value
*                                       conflicts the egress value.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. When portDirection = 'Ingress direction' , the (portTypePtr) will get value of:
*       CPSS_CSCD_PORT_NETWORK_E - network port indication.
*       CPSS_CSCD_PORT_DSA_MODE_1_WORD_E - cascade port indication.
*       APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
*/
GT_STATUS cpssDxChCscdPortTypeGet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_PORT_DIRECTION_ENT      portDirection,
    OUT CPSS_CSCD_PORT_TYPE_ENT     *portTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortTypeGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
   CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portDirection, portTypePtr));

    rc = internal_cpssDxChCscdPortTypeGet(devNum, portNum, portDirection, portTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portDirection, portTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cscdDevMapTableIndexGet function
* @endinternal
*
* @brief   Get index in index in Device Map table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] targetHwDevNum           - the HW device to be reached via cascade (APPLICABLE RANGES: 0..31)
* @param[in] sourceHwDevNum           - source HW device number (APPLICABLE RANGES: 0..31)
*                                       Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Relevant only for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E,
*                                      modes, otherwise ignored.
* @param[in] portNum                  - target / source port number
*                                       Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
*                                      Target for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E mode,
*                                      Source for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_PHYSICAL_SRC_PORT_E,
*                                      modes, otherwise ignored.
* @param[in] hash                     - packet hash value
*                                      (APPLICABLE RANGES: 0..4095)
*                                      used for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_LSB_HASH_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_MSB_HASH_E
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*                                      modes, otherwise ignored.
*                                      NOTE: either 6 bits of hash[5:0] or 6 bits of hash[11:6] will be taken depending on
*                                      the mode used.
*
* @param[out] numOfIndexesPtr          - pointer to number of indexes in indexArrPtr.
*                                      On success: always >= 1.
* @param[out] indexArrPtr[PRV_CPSS_DXCH_CSCD_DEV_MAP_INDEX_ARRAY_MAX_SIZE] - pointer to list of indexes in Device Map table.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device
*                                       or bad target Port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS cscdDevMapTableIndexGet
(
    IN  GT_U8               devNum,
    IN  GT_HW_DEV_NUM       targetHwDevNum,
    IN  GT_HW_DEV_NUM       sourceHwDevNum,
    IN  GT_PORT_NUM         portNum,
    IN  GT_U32              hash,
    OUT GT_U32              *numOfIndexesPtr,
    OUT GT_U32              indexArrPtr[PRV_CPSS_DXCH_CSCD_DEV_MAP_INDEX_ARRAY_MAX_SIZE]
)
{
    CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT   mode;     /* device Map lookup mode. */
    GT_STATUS                           rc;       /* return status */
    GT_U32                              i;        /* iterator */
    GT_U32                              startBit_srcHwDevNum = 0;
    GT_U32                              startBit_trgHwDevNum = 0;

    PRV_CPSS_DXCH_DUAL_HW_DEVICE_CHECK_MAC(targetHwDevNum);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        indexArrPtr[0] = targetHwDevNum;
        *numOfIndexesPtr = 1;
        if (PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC(targetHwDevNum))
        {
            /* part 1 of fix CPSS-6333 : Lion2: CPSS should configure both DevMapTable entries
                (even,odd) for a PRV_CPSS_IS_DUAL_HW_DEVICE_MAC */

            /* for dual devices need to update the targetHwDevNum +1 also */
            indexArrPtr[(*numOfIndexesPtr)++] = targetHwDevNum + 1;
        }
        return GT_OK;
    }

    rc = cpssDxChCscdDevMapLookupModeGet(devNum, &mode);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* check for dual devices */
    if(mode == CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E)
    {
        /* port num is target port */
        PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(targetHwDevNum,portNum);
        targetHwDevNum = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(targetHwDevNum,portNum);
        portNum = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(targetHwDevNum,portNum);
    }
    else if (mode != CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E)
    {
        PRV_CPSS_DXCH_DUAL_HW_DEVICE_CHECK_MAC(sourceHwDevNum);

        if (mode != CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E)
        {
            /* port num is source port */
            PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(sourceHwDevNum,portNum);
            sourceHwDevNum = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(sourceHwDevNum,portNum);
            portNum = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(sourceHwDevNum,portNum);
        }
    }

    if(targetHwDevNum > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(mode)
    {
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E:
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E:
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E:
            if(portNum >= BIT_6)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E:
            if(sourceHwDevNum >= BIT_5)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E:
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E:
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E:
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E:
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E:
            if(sourceHwDevNum >= BIT_5 || portNum >= BIT_6)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_PORT_E:
            if(portNum >= BIT_12)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_PHYSICAL_SRC_PORT_E:
            if(targetHwDevNum >= BIT_4 || portNum >= BIT_8)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_LSB_HASH_E:
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_MSB_HASH_E:
            if(hash >= BIT_12)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    *numOfIndexesPtr = 1;

    switch(mode)
    {
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E:
            indexArrPtr[0] = targetHwDevNum;
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E:
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E:
            startBit_trgHwDevNum = 6;
            indexArrPtr[0] = portNum | targetHwDevNum << 6;
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E:
            startBit_srcHwDevNum = 0;
            startBit_trgHwDevNum = 5;
            indexArrPtr[0] = sourceHwDevNum | targetHwDevNum << 5;
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E:
            startBit_srcHwDevNum = 5;
            startBit_trgHwDevNum = 6;
            indexArrPtr[0] = (portNum & 0x1F) | (sourceHwDevNum & 0x1) << 5 |
                                       targetHwDevNum << 6;
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E:
            startBit_srcHwDevNum = 4;
            startBit_trgHwDevNum = 6;
            indexArrPtr[0] = (portNum & 0xF) | (sourceHwDevNum & 0x3) << 4 |
                                        targetHwDevNum << 6;
            break;

        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E:
            startBit_srcHwDevNum = 3;
            startBit_trgHwDevNum = 6;
            *indexArrPtr = (portNum & 0x7) | (sourceHwDevNum & 0x7) << 3 |
                                        targetHwDevNum << 6;
            break;

        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E:
            startBit_srcHwDevNum = 2;
            startBit_trgHwDevNum = 6;
            indexArrPtr[0] = (portNum & 0x3) | (sourceHwDevNum & 0xF) << 2 |
                                        targetHwDevNum << 6;
            break;

        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E:
            startBit_srcHwDevNum = 1;
            startBit_trgHwDevNum = 6;
            indexArrPtr[0] = (portNum & 0x1) | (sourceHwDevNum & 0x1F) << 1 |
                                        targetHwDevNum << 6;
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_PORT_E:
            indexArrPtr[0] = portNum;
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_PHYSICAL_SRC_PORT_E:
            startBit_trgHwDevNum = 8;
            indexArrPtr[0] = (portNum & 0xFF) | (targetHwDevNum << 8);
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_LSB_HASH_E:
            startBit_trgHwDevNum = 6;
            indexArrPtr[0] = (hash & 0x3F) | (targetHwDevNum << 6);
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_MSB_HASH_E:
            startBit_trgHwDevNum = 6;
            indexArrPtr[0] = ((hash & 0xFC0) >> 6) | (targetHwDevNum << 6);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /***************************************************/
    /* handle cases that relate to 'Dual HW device Id' */
    /***************************************************/
    /* part 2 of fix CPSS-6333 : Lion2: CPSS should configure both DevMapTable entries
        (even,odd) for a PRV_CPSS_IS_DUAL_HW_DEVICE_MAC */
    switch(mode)
    {
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E:
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_LSB_HASH_E:
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_MSB_HASH_E:
            if (PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC(targetHwDevNum))
            {
                /* for dual devices need to update the targetHwDevNum +1 also */
                indexArrPtr[(*numOfIndexesPtr)++] = targetHwDevNum + 1;
            }
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E:
            /* note : this case use explicit {dev,port} pair , meaning no extra entry needed */
            /* so no need to add extra index in case of 'Dual HW device Id' */
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E:
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_PHYSICAL_SRC_PORT_E:
            /* note : this case use 6 bits of 'src' port and 5 bits of 'trg' dev */
            if (PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC(targetHwDevNum))
            {
                /* for dual devices need to update the targetHwDevNum +1 also */
                indexArrPtr[(*numOfIndexesPtr)++] = indexArrPtr[0] +
                        (1 << startBit_trgHwDevNum);
            }
            break;
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E:
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E:
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E:
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E:
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E:
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E:
             /* for dual devices need to update all combinations of
               'Dual HW device Id' for the targetHwDevNum,sourceHwDevNum */
            if (PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC(targetHwDevNum))
            {
                /* for dual devices need to update the targetHwDevNum +1 also */
                indexArrPtr[(*numOfIndexesPtr)++] = indexArrPtr[0] +
                        (1 << startBit_trgHwDevNum);
            }
            if (PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC(sourceHwDevNum))
            {
                /* for dual devices need to update the sourceHwDevNum +1 also */
                indexArrPtr[(*numOfIndexesPtr)++] = indexArrPtr[0] +
                        (1 << startBit_srcHwDevNum);
            }
            if (PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC(sourceHwDevNum) &&
                PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC(targetHwDevNum))
            {
                /* for dual devices need to update the
                    (sourceHwDevNum +1) and (targetHwDevNum +1) also */
                indexArrPtr[(*numOfIndexesPtr)++] = indexArrPtr[0] +
                        (1 << startBit_trgHwDevNum) +
                        (1 << startBit_srcHwDevNum);
            }

            break;
        default:
            break;
    }


    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        for (i=0;i<(*numOfIndexesPtr);i++)
        {
            /* this is common check for <targetHwDevNum> that is not 'oversize' that
               cause index out of range */
            if(indexArrPtr[i] >= BIT_12)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

    }
    return GT_OK;

}

/**
* @internal prvCpssDxChCscdDevMapTableEntryHwFormatGet function
* @endinternal
*
* @brief   Get Cascade Device Map table entry in HW format
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cascadeLinkPtr           - (pointer to)A structure holding the cascade link type
*                                      (port (APPLICABLE RANGES: 0..63) or
*                                      trunk(APPLICABLE RANGES: 0..127)) and the link number
*                                      leading to the target device.
* @param[in] srcPortTrunkHashEn       - Relevant when (cascadeLinkPtr->linkType ==
*                                      CPSS_CSCD_LINK_TYPE_TRUNK_E)
*                                      Enabled to set the load balancing trunk hash for packets
*                                      forwarded via an trunk uplink to be based on the
*                                      packet's source port, and not on the packets data.
*                                      Indicates the type of uplink.
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E -
*                                      Load balancing trunk hash is based on the ingress pipe
*                                      hash mode as configured by function
*                                      cpssDxChTrunkHashGeneralModeSet(...)
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E - Load
*                                      balancing trunk hash for this cascade trunk interface is
*                                      based on the packet's source port, regardless of the
*                                      ingress pipe hash mode
*                                      NOTE : this parameter is relevant only to DXCH2
*                                      and above devices
* @param[in] egressAttributesLocallyEn - Determines whether the egress attributes
*                                      are determined by the target ePort even if the target
*                                      device is not the local device.
*                                      (APPLICABLE DEVICES Lion2)
*
* @param[out] hwDataPtr                - Pointer to Device Map Table entry format data.
* @param[out] numBitsPtr               - Pointer to number of bits in the entry.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad trunk hash mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCscdDevMapTableEntryHwFormatGet
(
    IN  GT_U8                        devNum,
    IN  CPSS_CSCD_LINK_TYPE_STC      *cascadeLinkPtr,
    IN  CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn,
    IN GT_BOOL                      egressAttributesLocallyEn,
    OUT GT_U32                       *hwDataPtr,
    OUT GT_U32                       *numBitsPtr
)
{
    GT_U32 tempValue;
    GT_U32 linkNum;
    GT_U32 numBitsForLinkNum;  /* number of bits for <Port Number / TrunkNum> */

    CPSS_NULL_PTR_CHECK_MAC(cascadeLinkPtr);
    CPSS_NULL_PTR_CHECK_MAC(hwDataPtr);
    CPSS_NULL_PTR_CHECK_MAC(numBitsPtr);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        *hwDataPtr = 0;

        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr,0,1,BOOL2BIT_MAC(egressAttributesLocallyEn));

        linkNum = cascadeLinkPtr->linkNum;

        numBitsForLinkNum = NUM_BITS_FOR_LINK_NUM_MAC(devNum);

        switch(cascadeLinkPtr->linkType)
        {
            case CPSS_CSCD_LINK_TYPE_TRUNK_E:
                if(linkNum >= PRV_CPSS_DXCH_PP_HW_INFO_TRUNK_MAC(devNum).nonTrunkTable1NumEntries)
                {
                    /* although 12 bits in HW , there is no support for more than 8 bits */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "(trunkId) linkNum[%d] >= max[%d] (from nonTrunkTable1NumEntries)",
                        linkNum ,
                        PRV_CPSS_DXCH_PP_HW_INFO_TRUNK_MAC(devNum).nonTrunkTable1NumEntries);
                }

                U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr,1,1,1);

                switch(srcPortTrunkHashEn)
                {
                    case CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E:
                        tempValue = 0;
                        break;
                    case CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E:
                        tempValue = 1;
                        break;
                    case CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E:
                        tempValue = 2;
                        break;
                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                /*<Cascade Trunk Hash Mode>*/
                U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr,2+numBitsForLinkNum,2,tempValue);

                break;
            case CPSS_CSCD_LINK_TYPE_PORT_E:
                if(linkNum >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum))
                {
                    /* although 8/12 bits in HW ... do not let application make mistake with non valid port */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "(portNum) linkNum[%d] >= max[%d]",
                        linkNum ,
                        PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum));
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr,2,numBitsForLinkNum,linkNum);

        /* 2 bits <Cascade Trunk Hash Mode> ,
           1 bit <Uplink Is Trunk>  ,
           1 bit <Assign Egress Attributes Locally>

           numBitsForLinkNum <Port Number / TrunkNum>
           */
        *numBitsPtr = 4 + numBitsForLinkNum;

        return GT_OK;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        *numBitsPtr = 10;
    }
    else
    {
        *numBitsPtr = 9;
    }

    switch(cascadeLinkPtr->linkType)
    {
        case CPSS_CSCD_LINK_TYPE_TRUNK_E:
            linkNum = cascadeLinkPtr->linkNum;
            if(linkNum > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *hwDataPtr = BIT_0;
            switch(srcPortTrunkHashEn)
            {
                case CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E:
                    break;
                case CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E:
                    *hwDataPtr |= BIT_8;
                    break;
                case CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E:
                    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }

                    *hwDataPtr |= BIT_9;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;

        case CPSS_CSCD_LINK_TYPE_PORT_E:
            PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,cascadeLinkPtr->linkNum);
            linkNum = PRV_CPSS_DXCH_GLOBAL_TO_HEM_LOCAL_PORT(devNum,cascadeLinkPtr->linkNum);
            if (linkNum == CPSS_CPU_PORT_NUM_CNS)
            {
                if (0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
                {
                    /* Assign CPU cascade port in the device map table */
                    linkNum = XCAT_DEV_MAP_TABLE_CPU_PORT_NUM_CNS;
                }
            }
            *hwDataPtr = 0;

            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    *hwDataPtr |= ((linkNum & 0x7F) << 1);  /* set bits 1..7 */

    return GT_OK;
}

/**
* @internal prvCpssDxChCscdDevMapTableHemisphereGet function
* @endinternal
*
* @brief   get the hemisphere which represents the cascade port/trunk.
*         relevant only to multi-hemisphere devices.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] cascadeLinkPtr           - (pointer to)A structure holding the cascade link type
*                                      (port (APPLICABLE RANGES: 0..63) or
*                                      trunk(APPLICABLE RANGES: 0..127)) and the link number
*                                      leading to the target device.
*
* @param[out] hemisphereNumPtr         - (pointer to) hemisphere Number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - the trunk is in bad state , one of:
*                                       1. empty trunk
*                                       2. hold members from no local device
*                                       3. hold members from more than single hemisphere
*/
static GT_STATUS prvCpssDxChCscdDevMapTableHemisphereGet
(
    IN GT_U8                        devNum,
    IN CPSS_CSCD_LINK_TYPE_STC      *cascadeLinkPtr,
    OUT GT_U32                      *hemisphereNumPtr
)
{
    GT_STATUS rc;
    CPSS_TRUNK_TYPE_ENT trunkType;/*trunk type*/
    CPSS_TRUNK_MEMBER_STC   trunkMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];/*trunk members*/
    GT_TRUNK_ID trunkId;/*trunk Id for the device map table*/
    GT_U32  numOfEnabledMembers,numOfDisabledMembers;/* number of enabled/disabled members in the trunk */
    GT_U32  ii;/*iterator*/
    CPSS_PORTS_BMP_STC  trunkMembersBmp;

    switch (cascadeLinkPtr->linkType)
    {
        case CPSS_CSCD_LINK_TYPE_PORT_E:
             *hemisphereNumPtr = PRV_CPSS_DXCH_GLOBAL_TO_HEM_NUMBER(devNum,cascadeLinkPtr->linkNum);
             break;
        case CPSS_CSCD_LINK_TYPE_TRUNK_E:
            trunkId = (GT_TRUNK_ID)cascadeLinkPtr->linkNum;

            if(trunkId == 0)
            {
                *hemisphereNumPtr = 0;/* use hemisphere 0 as the 'default' for this 'Discarded traffic'*/
                /* special settings that used for initialization , that need to be allowed */
                return GT_OK;
            }

            rc = cpssDxChTrunkDbTrunkTypeGet(devNum,trunkId,&trunkType);
            if(rc != GT_OK)
            {
                return rc;
            }

            switch(trunkType)
            {
                case CPSS_TRUNK_TYPE_FREE_E:
                    /* we don't handle empty trunk , because can't know the
                       hemisphere */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                case CPSS_TRUNK_TYPE_REGULAR_E:
                    /* get the enabled members as first in the array */
                    numOfEnabledMembers = CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
                    rc = cpssDxChTrunkDbEnabledMembersGet(devNum, trunkId, &numOfEnabledMembers, &trunkMembersArray[0]);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    /* get the disabled members as last in the array */
                    numOfDisabledMembers = CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS - numOfEnabledMembers;
                    rc = cpssDxChTrunkDbDisabledMembersGet(devNum, trunkId, &numOfDisabledMembers, &trunkMembersArray[numOfEnabledMembers]);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    if((numOfDisabledMembers + numOfEnabledMembers) == 0)
                    {
                        /* we don't handle empty trunk , because can't know the
                           hemisphere */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                    }

                    *hemisphereNumPtr = PRV_CPSS_DXCH_GLOBAL_TO_HEM_NUMBER(devNum,trunkMembersArray[ 0].port);

                    for(ii = 0 ; ii < (numOfDisabledMembers + numOfEnabledMembers) ; ii++)
                    {
                        if(PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC(PRV_CPSS_HW_DEV_NUM_MAC(devNum)) &&
                           ((PRV_CPSS_HW_DEV_NUM_MAC(devNum) & (~1)) != (trunkMembersArray[ii].hwDevice & (~1))))
                        {
                            /* this trunk can't be used as cascade trunk on this device */
                            /* return 'bad state' and not 'bad param' to show that
                               trunkId is ok but the trunk state is not ! */
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                        }
                        else if(trunkMembersArray[ii].hwDevice != PRV_CPSS_HW_DEV_NUM_MAC(devNum))
                        {
                            /* this trunk can't be used as cascade trunk on this device */
                            /* return 'bad state' and not 'bad param' to show that
                               trunkId is ok but the trunk state is not ! */
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                        }

                        /* all the ports must be in the same hemisphere */
                        if((*hemisphereNumPtr) !=
                           PRV_CPSS_DXCH_GLOBAL_TO_HEM_NUMBER(devNum,trunkMembersArray[ii].port))
                        {
                            /* return 'bad state' and not 'bad param' to show that
                               trunkId is ok but the trunk state is not ! */
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                        }
                    }

                    break;
                case CPSS_TRUNK_TYPE_CASCADE_E:
                    rc = cpssDxChTrunkCascadeTrunkPortsGet(devNum,trunkId,&trunkMembersBmp);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    (*hemisphereNumPtr) = NOT_INITIALIZED_CNS;

                    for(ii = 0 ; ii < PRV_CPSS_PP_MAC(devNum)->numOfPorts ; ii++)
                    {
                        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&trunkMembersBmp,ii))
                        {
                            if((*hemisphereNumPtr) == NOT_INITIALIZED_CNS)
                            {
                                (*hemisphereNumPtr) = PRV_CPSS_DXCH_GLOBAL_TO_HEM_NUMBER(devNum,ii);

                                ii = (ii & ~(0x3f)) + 64;/* jump to next hemisphere */

                                ii --; /* compensation done by (ii++) in loop iteration */
                                continue;
                            }
                            else
                            {
                                /* ports on 2 hemispheres */
                                /* all the ports must be in the same hemisphere */
                                /* return 'bad state' and not 'bad param' to show that
                                   trunkId is ok but the trunk state is not ! */
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                            }
                        }
                    }

                    if((*hemisphereNumPtr) == NOT_INITIALIZED_CNS)
                    {
                        /* we don't handle empty trunk , because can't know the
                           hemisphere */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                    }

                    break;
                default:/* should not happen */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }/*switch(trunkType)*/

            break; /*case CPSS_CSCD_LINK_TYPE_TRUNK_E:*/

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }/*switch (cascadeLinkPtr->linkType)*/
    return GT_OK;
}

/**
* @internal prvCpssDxChCscdDevMapByArrayIndexesTableSet function
* @endinternal
*
* @brief   Set the 'device map table' (used for cascaded system) by pre-calculated indexes.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] numOfIndexes             - number of indexes in indexArrPtr. (value 1..PRV_CPSS_DXCH_CSCD_DEV_MAP_INDEX_ARRAY_MAX_SIZE)
* @param[in] indexesArr[PRV_CPSS_DXCH_CSCD_DEV_MAP_INDEX_ARRAY_MAX_SIZE] - pointer to list of indexes in Device Map table.
* @param[in] cascadeLinkPtr           - (pointer to)A structure holding the cascade link type
*                                      (port (APPLICABLE RANGES: 0..63)
*                                      or trunk(APPLICABLE RANGES: 0..127)) and the link number
*                                      leading to the target device.
* @param[in] srcPortTrunkHashEn       - Relevant when (cascadeLinkPtr->linkType ==
*                                      CPSS_CSCD_LINK_TYPE_TRUNK_E)
*                                      Enabled to set the load balancing trunk hash for packets
*                                      forwarded via an trunk uplink to be based on the
*                                      packet's source port, and not on the packets data.
*                                      Indicates the type of uplink.
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E -
*                                      Load balancing trunk hash is based on the ingress pipe
*                                      hash mode as configured by function
*                                      cpssDxChTrunkHashGeneralModeSet(...)
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E - Load
*                                      balancing trunk hash for this cascade trunk interface is
*                                      based on the packet's source port, regardless of the
*                                      ingress pipe hash mode
*                                      NOTE : this parameter is relevant only to DXCH2
*                                      and above devices
* @param[in] egressAttributesLocallyEn - Determines whether the egress attributes
*                                      are determined by the target port even if the target
*                                      device is not the local device.
*                                      (APPLICABLE DEVICES Lion2)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or bad trunk hash
*                                       mode or bad target Port
* @retval GT_BAD_STATE             - the trunk is in bad state , one of:
*                                       1. empty trunk (APPLICABLE DEVICES Lion2)
*                                       2. hold members from no local device (APPLICABLE DEVICES Lion2)
*                                       3. hold members from more than single hemisphere (APPLICABLE DEVICES Lion2)
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCscdDevMapByArrayIndexesTableSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       numOfIndexes,
    IN GT_U32                       indexesArr[PRV_CPSS_DXCH_CSCD_DEV_MAP_INDEX_ARRAY_MAX_SIZE],
    IN CPSS_CSCD_LINK_TYPE_STC      *cascadeLinkPtr,
    IN CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn,
    IN GT_BOOL                      egressAttributesLocallyEn
)
{
    GT_U32       valueArr[1];  /* HW format of entry */
    GT_U32       regAddr;      /* register address */
    GT_U32       numBits;      /* number of bit to update */
    GT_STATUS    rc;           /* return status */
    GT_U32       hemisphereNum = 0;/* hemisphere number */
    GT_U32       fieldOffset;      /* The start bit number in the register */
    GT_U32       i;  /* iterator */

    CPSS_NULL_PTR_CHECK_MAC(indexesArr);
    if(numOfIndexes == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get Cascade Device Map table entry in HW format */
    rc = prvCpssDxChCscdDevMapTableEntryHwFormatGet(devNum, cascadeLinkPtr,
                                                    srcPortTrunkHashEn,
                                                    egressAttributesLocallyEn,
                                                    valueArr, &numBits);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {

        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
        {
            /* no 2 hemispheres issues */
        }
        else
        if (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(devNum))
        {
            rc = prvCpssDxChCscdDevMapTableHemisphereGet(devNum,cascadeLinkPtr,&hemisphereNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* loop on indexes */
        for (i = 0 ; i < numOfIndexes; i++)
        {
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
            {
                rc = prvCpssDxChWriteTableEntry(devNum,
                                                  CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_DEVICE_MAP_TABLE_E,
                                                  indexesArr[i],
                                                  &valueArr[0]);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    txqVer1.sht.egressTables.deviceMapTableBaseAddress + (0x4 * indexesArr[i]);

                rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, numBits, valueArr[0]);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        txqVer1.dist.deviceMapTable.distEntryDevMapTableBaseAddress + 0x4 * (indexesArr[i] / 32);

                    fieldOffset = indexesArr[i] % 32;

                    rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 1, hemisphereNum);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.devMapTbl[indexesArr[0]];
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, numBits, valueArr[0]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChCscdDevMapTableSet function
* @endinternal
*
* @brief   Set the cascade map table . the map table define the local port or
*         trunk that packets destined to a destination device
*         should be transmitted to.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] targetHwDevNum           - the HW device to be reached via cascade (APPLICABLE RANGES: 0..31)
* @param[in] sourceHwDevNum           - source HW device number (APPLICABLE RANGES: 0..31)
*                                       Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Relevant only for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E,
*                                      modes, otherwise ignored.
* @param[in] portNum                  - target / source port number
*                                       Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Target for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E mode,
*                                      Source for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_PHYSICAL_SRC_PORT_E,
*                                      modes, otherwise ignored.
* @param[in] hash                     - packet hash value
*                                      (APPLICABLE RANGES: 0..4095)
*                                      used for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_LSB_HASH_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_MSB_HASH_E
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*                                      modes, otherwise ignored.
*                                      NOTE: either 6 bits of hash[5:0] or 6 bits of hash[11:6] will be taken depending on
*                                      the mode used.
* @param[in] cascadeLinkPtr           - (pointer to)A structure holding the cascade link type
*                                      (port (APPLICABLE RANGES: 0..63)
*                                      or trunk(APPLICABLE RANGES: 0..127)) and the link number
*                                      leading to the target device.
* @param[in] srcPortTrunkHashEn       - Relevant when (cascadeLinkPtr->linkType ==
*                                      CPSS_CSCD_LINK_TYPE_TRUNK_E)
*                                      Enabled to set the load balancing trunk hash for packets
*                                      forwarded via an trunk uplink to be based on the
*                                      packet's source port, and not on the packets data.
*                                      Indicates the type of uplink.
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E -
*                                      Load balancing trunk hash is based on the ingress pipe
*                                      hash mode as configured by function
*                                      cpssDxChTrunkHashGeneralModeSet(...)
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E - Load
*                                      balancing trunk hash for this cascade trunk interface is
*                                      based on the packet's source port, regardless of the
*                                      ingress pipe hash mode
*                                      NOTE : this parameter is relevant only to DXCH2
*                                      and above devices
* @param[in] egressAttributesLocallyEn - Determines whether the egress attributes
*                                      are determined by the target port even if the target
*                                      device is not the local device.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or source device,
*                                       or port number or bad trunk hash mode
* @retval GT_BAD_STATE             - the trunk is in bad state , one of:
*                                       1. empty trunk (APPLICABLE DEVICES : Lion2)
*                                       2. hold members from no local device (APPLICABLE DEVICES : Lion2)
*                                       3. hold members from more than single hemisphere (APPLICABLE DEVICES : Lion2)
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChCscdDevMapTableSet
(
    IN GT_U8                        devNum,
    IN GT_HW_DEV_NUM                targetHwDevNum,
    IN GT_HW_DEV_NUM                sourceHwDevNum,
    IN GT_PORT_NUM                  portNum,
    IN GT_U32                       hash,
    IN CPSS_CSCD_LINK_TYPE_STC      *cascadeLinkPtr,
    IN CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn,
    IN GT_BOOL                      egressAttributesLocallyEn
)
{
    GT_STATUS    rc;           /* return status */
    /* indexes in device Map table */
    GT_U32       indexesArr[PRV_CPSS_DXCH_CSCD_DEV_MAP_INDEX_ARRAY_MAX_SIZE] = {0};
    GT_U32       numOfIndexes; /* num of entries to set in the table */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cascadeLinkPtr);

    if(targetHwDevNum > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cscdDevMapTableIndexGet(devNum, targetHwDevNum,
                                            sourceHwDevNum, portNum,
                                            hash,
                                            &numOfIndexes,indexesArr);
    if(rc != GT_OK)
    {
        return rc;
    }

    return prvCpssDxChCscdDevMapByArrayIndexesTableSet(devNum,numOfIndexes,indexesArr,
            cascadeLinkPtr ,srcPortTrunkHashEn , egressAttributesLocallyEn);
}

/**
* @internal internal_cpssDxChCscdDevMapTableSet function
* @endinternal
*
* @brief   Set the cascade map table . the map table define the local port or
*         trunk that packets destined to a destination device
*         should be transmitted to.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] targetHwDevNum           - the HW device to be reached via cascade (APPLICABLE RANGES: 0..31)
* @param[in] sourceHwDevNum           - source HW device number (APPLICABLE RANGES: 0..31)
*                                       Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Relevant only for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E,
*                                      modes, otherwise ignored.
* @param[in] portNum                  - target / source port number
*                                       Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Target for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E mode,
*                                      Source for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_PHYSICAL_SRC_PORT_E,
*                                      modes, otherwise ignored.
* @param[in] hash                     - packet hash value
*                                      (APPLICABLE RANGES: 0..4095)
*                                      used for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_LSB_HASH_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_MSB_HASH_E
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*                                      modes, otherwise ignored.
*                                      NOTE: either 6 bits of hash[5:0] or 6 bits of hash[11:6] will be taken depending on
*                                      the mode used.
* @param[in] cascadeLinkPtr           - (pointer to)A structure holding the cascade link type
*                                      (port (APPLICABLE RANGES: 0..63)
*                                      or trunk(APPLICABLE RANGES: 0..127)) and the link number
*                                      leading to the target device.
* @param[in] srcPortTrunkHashEn       - Relevant when (cascadeLinkPtr->linkType ==
*                                      CPSS_CSCD_LINK_TYPE_TRUNK_E)
*                                      Enabled to set the load balancing trunk hash for packets
*                                      forwarded via an trunk uplink to be based on the
*                                      packet's source port, and not on the packets data.
*                                      Indicates the type of uplink.
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E -
*                                      Load balancing trunk hash is based on the ingress pipe
*                                      hash mode as configured by function
*                                      cpssDxChTrunkHashGeneralModeSet(...)
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E - Load
*                                      balancing trunk hash for this cascade trunk interface is
*                                      based on the packet's source port, regardless of the
*                                      ingress pipe hash mode
*                                      NOTE : this parameter is relevant only to DXCH2
*                                      and above devices
* @param[in] egressAttributesLocallyEn - Determines whether the egress attributes
*                                      are determined by the target port even if the target
*                                      device is not the local device.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or source device,
*                                       or port number or bad trunk hash mode
* @retval GT_BAD_STATE             - the trunk is in bad state , one of:
*                                       1. empty trunk (APPLICABLE DEVICES : Lion2)
*                                       2. hold members from no local device (APPLICABLE DEVICES : Lion2)
*                                       3. hold members from more than single hemisphere (APPLICABLE DEVICES : Lion2)
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdDevMapTableSet
(
    IN GT_U8                        devNum,
    IN GT_HW_DEV_NUM                targetHwDevNum,
    IN GT_HW_DEV_NUM                sourceHwDevNum,
    IN GT_PORT_NUM                  portNum,
    IN GT_U32                       hash,
    IN CPSS_CSCD_LINK_TYPE_STC      *cascadeLinkPtr,
    IN CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn,
    IN GT_BOOL                      egressAttributesLocallyEn
)
{

    return prvCpssDxChCscdDevMapTableSet(devNum, targetHwDevNum,
                                         sourceHwDevNum,portNum, hash,
                                         cascadeLinkPtr,srcPortTrunkHashEn,
                                         egressAttributesLocallyEn);
}

/**
* @internal cpssDxChCscdDevMapTableSet function
* @endinternal
*
* @brief   Set the cascade map table . the map table define the local port or
*         trunk that packets destined to a destination device
*         should be transmitted to.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] targetHwDevNum           - the HW device to be reached via cascade (APPLICABLE RANGES: 0..31)
* @param[in] sourceHwDevNum           - source HW device number (APPLICABLE RANGES: 0..31)
*                                       Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Relevant only for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E,
*                                      modes, otherwise ignored.
* @param[in] portNum                  - target / source port number
*                                       Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Target for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E mode,
*                                      Source for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_PHYSICAL_SRC_PORT_E,
*                                      modes, otherwise ignored.
* @param[in] hash                     - packet hash value
*                                      (APPLICABLE RANGES: 0..4095)
*                                      used for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_LSB_HASH_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_MSB_HASH_E
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*                                      modes, otherwise ignored.
*                                      NOTE: either 6 bits of hash[5:0] or 6 bits of hash[11:6] will be taken depending on
*                                      the mode used.
* @param[in] cascadeLinkPtr           - (pointer to)A structure holding the cascade link type
*                                      (port (APPLICABLE RANGES: 0..63)
*                                      or trunk(APPLICABLE RANGES: 0..127)) and the link number
*                                      leading to the target device.
* @param[in] srcPortTrunkHashEn       - Relevant when (cascadeLinkPtr->linkType ==
*                                      CPSS_CSCD_LINK_TYPE_TRUNK_E)
*                                      Enabled to set the load balancing trunk hash for packets
*                                      forwarded via an trunk uplink to be based on the
*                                      packet's source port, and not on the packets data.
*                                      Indicates the type of uplink.
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E -
*                                      Load balancing trunk hash is based on the ingress pipe
*                                      hash mode as configured by function
*                                      cpssDxChTrunkHashGeneralModeSet(...)
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E - Load
*                                      balancing trunk hash for this cascade trunk interface is
*                                      based on the packet's source port, regardless of the
*                                      ingress pipe hash mode
*                                      NOTE : this parameter is relevant only to DXCH2
*                                      and above devices
* @param[in] egressAttributesLocallyEn - Determines whether the egress attributes
*                                      are determined by the target port even if the target
*                                      device is not the local device.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or source device,
*                                       or port number or bad trunk hash mode
* @retval GT_BAD_STATE             - the trunk is in bad state , one of:
*                                       1. empty trunk (APPLICABLE DEVICES : Lion2)
*                                       2. hold members from no local device (APPLICABLE DEVICES : Lion2)
*                                       3. hold members from more than single hemisphere (APPLICABLE DEVICES : Lion2)
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdDevMapTableSet
(
    IN GT_U8                        devNum,
    IN GT_HW_DEV_NUM                targetHwDevNum,
    IN GT_HW_DEV_NUM                sourceHwDevNum,
    IN GT_PORT_NUM                  portNum,
    IN GT_U32                       hash,
    IN CPSS_CSCD_LINK_TYPE_STC      *cascadeLinkPtr,
    IN CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn,
    IN GT_BOOL                      egressAttributesLocallyEn
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdDevMapTableSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, targetHwDevNum, sourceHwDevNum, portNum, hash, cascadeLinkPtr, srcPortTrunkHashEn, egressAttributesLocallyEn));

    rc = internal_cpssDxChCscdDevMapTableSet(devNum, targetHwDevNum, sourceHwDevNum, portNum, hash, cascadeLinkPtr, srcPortTrunkHashEn, egressAttributesLocallyEn);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, targetHwDevNum, sourceHwDevNum, portNum, hash, cascadeLinkPtr, srcPortTrunkHashEn, egressAttributesLocallyEn));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChCscdDevMapTableEntryGet function
* @endinternal
*
* @brief   Get the cascade map table entry data from hardware format.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] hwData                   - hardware format data
* @param[in] hemisphereNum            - hemisphere Number (relevant only to dual TXQs devices)
*
* @param[out] cascadeLinkPtr           - (pointer to)A structure holding the cascade link type
*                                      (port (APPLICABLE RANGES: 0..63)
*                                      or trunk(APPLICABLE RANGES: 0..127)) and the link number
*                                      leading to the target device.
* @param[out] srcPortTrunkHashEnPtr    - Relevant when (cascadeLinkPtr->linkType ==
*                                      CPSS_CSCD_LINK_TYPE_TRUNK_E)
*                                      (pointer to) Enabled to set the load balancing trunk
*                                      hash for packets forwarded via an trunk uplink to be
*                                      based on the packet's source port, and not on the
*                                      packets data.
*                                      Indicates the type of uplink.
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E -
*                                      Load balancing trunk hash is based on the ingress pipe
*                                      hash mode as configured by function
*                                      cpssDxChTrunkHashGeneralModeSet(...)
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E - Load
*                                      balancing trunk hash for this cascade trunk interface is
*                                      based on the packet's source port, regardless of the
*                                      ingress pipe hash mode
*                                      NOTE : this parameter is relevant only to DXCH2 and
*                                      above devices
* @param[out] egressAttributesLocallyEnPtr - Determines whether the egress attributes
*                                      are determined by the target port even if the target
*                                      device is not the local device.
*                                      (APPLICABLE DEVICES Lion2)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on bad value in HW
*/
GT_STATUS prvCpssDxChCscdDevMapTableEntryGet
(
    IN GT_U8                         devNum,
    IN GT_U32                        hwData,
    IN GT_U32                        hemisphereNum,
    OUT CPSS_CSCD_LINK_TYPE_STC      *cascadeLinkPtr,
    OUT CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT *srcPortTrunkHashEnPtr,
    OUT GT_BOOL                      *egressAttributesLocallyEnPtr
)
{
    GT_U32 cascadeTrunkHashMode;       /* Cascade Trunk Hash Mode */
    GT_U32 numBitsForLinkNum;  /* number of bits for <Port Number / TrunkNum> */

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* check NULL pointer */
        CPSS_NULL_PTR_CHECK_MAC(srcPortTrunkHashEnPtr);
        CPSS_NULL_PTR_CHECK_MAC(egressAttributesLocallyEnPtr);

        *egressAttributesLocallyEnPtr = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwData,0,1));

        cascadeLinkPtr->linkType = U32_GET_FIELD_MAC(hwData,1,1) ?
                        CPSS_CSCD_LINK_TYPE_TRUNK_E :
                        CPSS_CSCD_LINK_TYPE_PORT_E;

        numBitsForLinkNum = NUM_BITS_FOR_LINK_NUM_MAC(devNum);

        cascadeLinkPtr->linkNum = U32_GET_FIELD_MAC(hwData,2,numBitsForLinkNum);
        cascadeTrunkHashMode = U32_GET_FIELD_MAC(hwData,2+numBitsForLinkNum,2);

        /*<Cascade Trunk Hash Mode>*/
        switch(cascadeTrunkHashMode)
        {
            case 0:
                *srcPortTrunkHashEnPtr = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E;
                break;
            case 1:
                *srcPortTrunkHashEnPtr = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;
                break;
            case 2:
                *srcPortTrunkHashEnPtr = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E;
                break;
            default:/*3*/
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

    }
    else
    {
        cascadeLinkPtr->linkType = U32_GET_FIELD_MAC(hwData,0,1) ?
                        CPSS_CSCD_LINK_TYPE_TRUNK_E :
                        CPSS_CSCD_LINK_TYPE_PORT_E;
        cascadeLinkPtr->linkNum = U32_GET_FIELD_MAC(hwData,1,7);
        if (0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
        {
            if (cascadeLinkPtr->linkType == CPSS_CSCD_LINK_TYPE_PORT_E &&
                cascadeLinkPtr->linkNum == XCAT_DEV_MAP_TABLE_CPU_PORT_NUM_CNS)
            {
                /* Get CPU cascade port from the device map table */
                cascadeLinkPtr->linkNum = CPSS_CPU_PORT_NUM_CNS;
            }
        }
        else
        if (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(devNum) &&
            (cascadeLinkPtr->linkType == CPSS_CSCD_LINK_TYPE_PORT_E))
        {
            cascadeLinkPtr->linkNum |= hemisphereNum << 6;
        }

        /* check null pointer only when cheetah2 and above */
        CPSS_NULL_PTR_CHECK_MAC(srcPortTrunkHashEnPtr);

        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
        {
            /* set the parameter regardless to the port/trunk target */
            *srcPortTrunkHashEnPtr = U32_GET_FIELD_MAC(hwData,8,1) ?
                CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E :
                CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E;
        }
        else
        {
            switch (U32_GET_FIELD_MAC(hwData,8,2))
            {
                case 0:
                    *srcPortTrunkHashEnPtr = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E;
                    break;
                case 1:
                    *srcPortTrunkHashEnPtr = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;
                    break;
                case 2:
                    *srcPortTrunkHashEnPtr = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E;
                    break;
                default:
                    break;
            }
        }

    }

    return GT_OK;
}


/**
* @internal internal_cpssDxChCscdDevMapTableGet function
* @endinternal
*
* @brief   Get the cascade map table . the map table define the local port or
*         trunk that packets destined to a destination device
*         should be transmitted to.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] targetHwDevNum           - the HW device to be reached via cascade (APPLICABLE RANGES: 0..31)
* @param[in] sourceHwDevNum           - source HW device number (APPLICABLE RANGES: 0..31)
*                                       Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Relevant only for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E,
*                                      modes, otherwise ignored.
* @param[in] portNum                  - target / source port number
*                                       Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Target for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E mode,
*                                      Source for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_PHYSICAL_SRC_PORT_E,
*                                      modes, otherwise ignored.
* @param[in] hash                     - packet hash value
*                                      (APPLICABLE RANGES: 0..4095)
*                                      used for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_LSB_HASH_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_MSB_HASH_E
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*                                      modes, otherwise ignored.
*                                      NOTE: either 6 bits of hash[5:0] or 6 bits of hash[11:6] will be taken depending on
*                                      the mode used.
*
* @param[out] cascadeLinkPtr           - (pointer to)A structure holding the cascade link type
*                                      (port (APPLICABLE RANGES: 0..63)
*                                      or trunk(APPLICABLE RANGES: 0..127)) and the link number
*                                      leading to the target device.
* @param[out] srcPortTrunkHashEnPtr    - Relevant when (cascadeLinkPtr->linkType ==
*                                      CPSS_CSCD_LINK_TYPE_TRUNK_E)
*                                      (pointer to) Enabled to set the load balancing trunk
*                                      hash for packets forwarded via an trunk uplink to be
*                                      based on the packet's source port, and not on the
*                                      packets data.
*                                      Indicates the type of uplink.
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E -
*                                      Load balancing trunk hash is based on the ingress pipe
*                                      hash mode as configured by function
*                                      cpssDxChTrunkHashGeneralModeSet(...)
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E - Load
*                                      balancing trunk hash for this cascade trunk interface is
*                                      based on the packet's source port, regardless of the
*                                      ingress pipe hash mode
*                                      NOTE : this parameter is relevant only to DXCH2 and
*                                      above devices
* @param[out] egressAttributesLocallyEnPtr - Determines whether the egress attributes
*                                      are determined by the target port even if the target
*                                      device is not the local device.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or source device,
*                                       or port number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on bad value in HW
*/
static GT_STATUS internal_cpssDxChCscdDevMapTableGet
(
    IN GT_U8                         devNum,
    IN GT_HW_DEV_NUM                 targetHwDevNum,
    IN GT_HW_DEV_NUM                 sourceHwDevNum,
    IN GT_PORT_NUM                   portNum,
    IN GT_U32                        hash,
    OUT CPSS_CSCD_LINK_TYPE_STC      *cascadeLinkPtr,
    OUT CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT *srcPortTrunkHashEnPtr,
    OUT GT_BOOL                      *egressAttributesLocallyEnPtr
)
{
    GT_STATUS   rc;       /* return code */
    GT_U32      value;    /* register value */
    GT_U32      regAddr;  /* register address */
    GT_U32      indexesArr[PRV_CPSS_DXCH_CSCD_DEV_MAP_INDEX_ARRAY_MAX_SIZE] = {0};
    GT_U32      numOfBits;
    GT_U32      numOfIndexes = 0; /* num of entries to set in the table */
    GT_U32      hemisphereNum = 0;
    GT_U32      fieldOffset; /* The start bit number in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cascadeLinkPtr);

    if(targetHwDevNum > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.devMapTbl[targetHwDevNum];
        rc = prvCpssHwPpGetRegField(devNum,regAddr, 0, 9, &value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        rc = cscdDevMapTableIndexGet(devNum, targetHwDevNum,
                                                sourceHwDevNum, portNum,
                                                hash,
                                                &numOfIndexes,indexesArr);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            rc = prvCpssDxChReadTableEntry(devNum,
                                              CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_DEVICE_MAP_TABLE_E,
                                              indexesArr[0],
                                              &value);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                txqVer1.sht.egressTables.deviceMapTableBaseAddress
                +  0x4 * indexesArr[0];

            numOfBits = 10;

            rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, numOfBits, &value);
            if(rc != GT_OK)
            {
                return rc;
            }

            if (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(devNum))
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    txqVer1.dist.deviceMapTable.distEntryDevMapTableBaseAddress + 0x4 * (indexesArr[0] / 32);
                fieldOffset = indexesArr[0] % 32;
                rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 1, &hemisphereNum);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    /* Get the cascade map table entry data from hardware format. */
    return prvCpssDxChCscdDevMapTableEntryGet(devNum, value, hemisphereNum ,cascadeLinkPtr,
                                              srcPortTrunkHashEnPtr,egressAttributesLocallyEnPtr);
}

/**
* @internal cpssDxChCscdDevMapTableGet function
* @endinternal
*
* @brief   Get the cascade map table . the map table define the local port or
*         trunk that packets destined to a destination device
*         should be transmitted to.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] targetHwDevNum           - the HW device to be reached via cascade (APPLICABLE RANGES: 0..31)
* @param[in] sourceHwDevNum           - source HW device number (APPLICABLE RANGES: 0..31)
*                                       Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Relevant only for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E,
*                                      modes, otherwise ignored.
* @param[in] portNum                  - target / source port number
*                                       Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Target for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E mode,
*                                      Source for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_PORT_E,
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_PHYSICAL_SRC_PORT_E,
*                                      modes, otherwise ignored.
* @param[in] hash                     - packet hash value
*                                      (APPLICABLE RANGES: 0..4095)
*                                      used for
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_LSB_HASH_E
*                                      CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_MSB_HASH_E
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*                                      modes, otherwise ignored.
*                                      NOTE: either 6 bits of hash[5:0] or 6 bits of hash[11:6] will be taken depending on
*                                      the mode used.
*
* @param[out] cascadeLinkPtr           - (pointer to)A structure holding the cascade link type
*                                      (port (APPLICABLE RANGES: 0..63)
*                                      or trunk(APPLICABLE RANGES: 0..127)) and the link number
*                                      leading to the target device.
* @param[out] srcPortTrunkHashEnPtr    - Relevant when (cascadeLinkPtr->linkType ==
*                                      CPSS_CSCD_LINK_TYPE_TRUNK_E)
*                                      (pointer to) Enabled to set the load balancing trunk
*                                      hash for packets forwarded via an trunk uplink to be
*                                      based on the packet's source port, and not on the
*                                      packets data.
*                                      Indicates the type of uplink.
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E -
*                                      Load balancing trunk hash is based on the ingress pipe
*                                      hash mode as configured by function
*                                      cpssDxChTrunkHashGeneralModeSet(...)
*                                      CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E - Load
*                                      balancing trunk hash for this cascade trunk interface is
*                                      based on the packet's source port, regardless of the
*                                      ingress pipe hash mode
*                                      NOTE : this parameter is relevant only to DXCH2 and
*                                      above devices
* @param[out] egressAttributesLocallyEnPtr - Determines whether the egress attributes
*                                      are determined by the target port even if the target
*                                      device is not the local device.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or target device or source device,
*                                       or port number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on bad value in HW
*/
GT_STATUS cpssDxChCscdDevMapTableGet
(
    IN GT_U8                         devNum,
    IN GT_HW_DEV_NUM                 targetHwDevNum,
    IN GT_HW_DEV_NUM                 sourceHwDevNum,
    IN GT_PORT_NUM                   portNum,
    IN GT_U32                        hash,
    OUT CPSS_CSCD_LINK_TYPE_STC      *cascadeLinkPtr,
    OUT CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT *srcPortTrunkHashEnPtr,
    OUT GT_BOOL                      *egressAttributesLocallyEnPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdDevMapTableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, targetHwDevNum, sourceHwDevNum, portNum, hash, cascadeLinkPtr, srcPortTrunkHashEnPtr, egressAttributesLocallyEnPtr));

    rc = internal_cpssDxChCscdDevMapTableGet(devNum, targetHwDevNum, sourceHwDevNum, portNum, hash, cascadeLinkPtr, srcPortTrunkHashEnPtr, egressAttributesLocallyEnPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, targetHwDevNum, sourceHwDevNum, portNum, hash, cascadeLinkPtr, srcPortTrunkHashEnPtr, egressAttributesLocallyEnPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdRemapQosModeSet function
* @endinternal
*
* @brief   Enables/disables remapping of Tc and Dp for Data and Control Traffic
*         on a port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] remapType                - traffic type to remap
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdRemapQosModeSet
(
   IN GT_U8                             devNum,
   IN GT_PHYSICAL_PORT_NUM              portNum,
   IN CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT remapType
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;    /* register address */
    GT_U32      fieldValue; /* registers field value */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    if(remapType > CPSS_DXCH_CSCD_QOS_REMAP_ALL_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    switch (remapType)
    {
        /* Disable TC and DP remapping */
        case CPSS_DXCH_CSCD_QOS_REMAP_DISABLED_E:
            fieldValue = 0;
            break;
        /* Enable TC and DP remap for control packets */
        case CPSS_DXCH_CSCD_QOS_REMAP_CNTRL_ONLY_E:
            fieldValue = 1;
            break;
        /* Enable TC and DP remap for data packets */
        case CPSS_DXCH_CSCD_QOS_REMAP_DATA_ONLY_E:
            fieldValue = 2;
            break;
        /* Enable TC and DP remapping */
        case CPSS_DXCH_CSCD_QOS_REMAP_ALL_E:
            fieldValue = 3;
            break;
        /* Wrong QoS remap mode type */
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        PRV_CPSS_DXCH_PORT_TXQ_REG_MAC(devNum,portNum,&regAddr);

        /* Enable TC and DP remapping */
        return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, 22, 2, fieldValue);
    }
    else
    {
        /* control remap */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.distributor.
            controlTcRemapEn[OFFSET_TO_WORD_MAC(portNum)];

        rc =  prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,
                regAddr,OFFSET_TO_BIT_MAC(portNum), 1,
                (fieldValue & 1)? 1 : 0);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* data remap */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.distributor.
            dataTcRemapEn[OFFSET_TO_WORD_MAC(portNum)];

        rc =  prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,
                regAddr,OFFSET_TO_BIT_MAC(portNum), 1,
                (fieldValue & 2) ? 1 : 0);

        return rc;
    }
}

/**
* @internal cpssDxChCscdRemapQosModeSet function
* @endinternal
*
* @brief   Enables/disables remapping of Tc and Dp for Data and Control Traffic
*         on a port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] remapType                - traffic type to remap
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdRemapQosModeSet
(
   IN GT_U8                             devNum,
   IN GT_PHYSICAL_PORT_NUM              portNum,
   IN CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT remapType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdRemapQosModeSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, remapType));

    rc = internal_cpssDxChCscdRemapQosModeSet(devNum, portNum, remapType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, remapType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdRemapQosModeGet function
* @endinternal
*
* @brief   Get remapping status of Tc and Dp for Data and Control Traffic
*         on a port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] remapTypePtr             - traffic type to remap
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static GT_STATUS internal_cpssDxChCscdRemapQosModeGet
(
   IN  GT_U8                             devNum,
   IN  GT_PHYSICAL_PORT_NUM              portNum,
   OUT CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT *remapTypePtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;     /* register address */
    GT_U32      hwValue;     /* whole hw value */
    GT_U32      hwValue1;    /* first bit of hw value */
    GT_U32      hwValue2;    /* second bit of hw value */
    GT_U32      portGroupId; /*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(remapTypePtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        PRV_CPSS_DXCH_PORT_TXQ_REG_MAC(devNum,portNum,&regAddr);

        /* Get hw value of status */
        rc =  prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 22, 2, &hwValue);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* control remap */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.distributor.
            controlTcRemapEn[OFFSET_TO_WORD_MAC(portNum)];

        rc =  prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,
                regAddr,OFFSET_TO_BIT_MAC(portNum), 1, &hwValue1);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* data remap */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.distributor.
            dataTcRemapEn[OFFSET_TO_WORD_MAC(portNum)];

        rc =  prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,
                regAddr,OFFSET_TO_BIT_MAC(portNum), 1, &hwValue2);
        if(rc != GT_OK)
        {
            return rc;
        }

        hwValue = hwValue1 | (hwValue2<<1);
    }

    switch (hwValue)
    {
        case 0:
            *remapTypePtr = CPSS_DXCH_CSCD_QOS_REMAP_DISABLED_E;
            break;
        case 1:
            *remapTypePtr = CPSS_DXCH_CSCD_QOS_REMAP_CNTRL_ONLY_E;
            break;
        case 2:
            *remapTypePtr = CPSS_DXCH_CSCD_QOS_REMAP_DATA_ONLY_E;
            break;
        case 3:
            *remapTypePtr = CPSS_DXCH_CSCD_QOS_REMAP_ALL_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChCscdRemapQosModeGet function
* @endinternal
*
* @brief   Get remapping status of Tc and Dp for Data and Control Traffic
*         on a port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] remapTypePtr             - traffic type to remap
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChCscdRemapQosModeGet
(
   IN  GT_U8                             devNum,
   IN  GT_PHYSICAL_PORT_NUM              portNum,
   OUT CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT *remapTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdRemapQosModeGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, remapTypePtr));

    rc = internal_cpssDxChCscdRemapQosModeGet(devNum, portNum, remapTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, remapTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCtrlQosSet function
* @endinternal
*
* @brief   Set control packets TC and DP if Control Remap QoS enabled on a port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ctrlTc                   - The TC assigned to control packets forwarded to
*                                      cascading ports (APPLICABLE RANGES: 0..7)
* @param[in] ctrlDp                   - The DP assigned to CPU-to-network control traffic or
*                                      network-to-CPU traffic.
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
* @param[in] cpuToCpuDp               - The DP assigned to CPU-to-CPU control traffic
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_VALUE             - on wrong ctrlDp or cpuToCpuDp level value or tc value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdCtrlQosSet
(
   IN GT_U8             devNum,
   IN GT_U8             ctrlTc,
   IN CPSS_DP_LEVEL_ENT ctrlDp,
   IN CPSS_DP_LEVEL_ENT cpuToCpuDp
)
{
    GT_STATUS   status;
    GT_U32      regAddr;    /* register address                 */
    GT_U32      value;      /* field value */
    GT_U32      startBit;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_COS_CHECK_DP_MAC(cpuToCpuDp);
    PRV_CPSS_DXCH_COS_CHECK_DP_MAC(ctrlDp);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(ctrlTc);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueConfigExt;
        startBit = 6;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.distributor.generalConfig;
        startBit = 0;
    }

    /* The TC assigned to control packets forwarded to cascading ports. */
    status = prvCpssHwPpSetRegField(devNum, regAddr, startBit + 0, 3, ctrlTc);
    if (status != GT_OK)
    {
        return status;
    }

    if (cpuToCpuDp == CPSS_DP_GREEN_E)
    {
        value = 0;
    }
    else
    {
        value = 1;
    }
    /* The DP assigned to CPU-to-CPU control traffic */
    status = prvCpssHwPpSetRegField(devNum, regAddr, startBit + 3, 1, value);
    if (status != GT_OK)
    {
        return status;
    }

    if (ctrlDp == CPSS_DP_GREEN_E)
    {
        value = 0;
    }
    else
    {
        value = 1;
    }

    /* The DP assigned to CPU-to-network control traffic or
       network-to-CPU traffic. */
    return prvCpssHwPpSetRegField(devNum, regAddr, startBit + 5, 1, value);
}

/**
* @internal cpssDxChCscdCtrlQosSet function
* @endinternal
*
* @brief   Set control packets TC and DP if Control Remap QoS enabled on a port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ctrlTc                   - The TC assigned to control packets forwarded to
*                                      cascading ports (APPLICABLE RANGES: 0..7)
* @param[in] ctrlDp                   - The DP assigned to CPU-to-network control traffic or
*                                      network-to-CPU traffic.
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
* @param[in] cpuToCpuDp               - The DP assigned to CPU-to-CPU control traffic
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_VALUE             - on wrong ctrlDp or cpuToCpuDp level value or tc value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCtrlQosSet
(
   IN GT_U8             devNum,
   IN GT_U8             ctrlTc,
   IN CPSS_DP_LEVEL_ENT ctrlDp,
   IN CPSS_DP_LEVEL_ENT cpuToCpuDp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCtrlQosSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ctrlTc, ctrlDp, cpuToCpuDp));

    rc = internal_cpssDxChCscdCtrlQosSet(devNum, ctrlTc, ctrlDp, cpuToCpuDp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ctrlTc, ctrlDp, cpuToCpuDp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCtrlQosGet function
* @endinternal
*
* @brief   Get control packets TC and DP if Control Remap QoS enabled on a port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] ctrlTcPtr                - The TC assigned to control packets forwarded to
*                                      cascading ports
* @param[out] ctrlDpPtr                - The DP assigned to CPU-to-network control traffic or
*                                      network-to-CPU traffic.
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
* @param[out] cpuToCpuDpPtr            - The DP assigned to CPU-to-CPU control traffic
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_VALUE             - on wrong ctrlDp or cpuToCpuDp level value or tc value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static GT_STATUS internal_cpssDxChCscdCtrlQosGet
(
   IN  GT_U8             devNum,
   OUT GT_U8             *ctrlTcPtr,
   OUT CPSS_DP_LEVEL_ENT *ctrlDpPtr,
   OUT CPSS_DP_LEVEL_ENT *cpuToCpuDpPtr
)
{
    GT_STATUS   status;
    GT_U32      regAddr;    /* register address                 */
    GT_U32      hwValue;    /* field value */
    GT_U32      startBit;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(ctrlTcPtr);
    CPSS_NULL_PTR_CHECK_MAC(ctrlDpPtr);
    CPSS_NULL_PTR_CHECK_MAC(cpuToCpuDpPtr);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueConfigExt;
        startBit = 6;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.distributor.generalConfig;
        startBit = 0;
    }

    /* The TC assigned to control packets forwarded to cascading ports. */
    status = prvCpssHwPpGetRegField(devNum, regAddr, startBit + 0, 3, &hwValue);
    if (status != GT_OK)
    {
        return status;
    }

    *ctrlTcPtr = (GT_U8)hwValue;

    /* The DP assigned to CPU-to-CPU control traffic */
    status = prvCpssHwPpGetRegField(devNum, regAddr, startBit + 3, 1, &hwValue);
    if (status != GT_OK)
    {
        return status;
    }

    *cpuToCpuDpPtr = (hwValue == 0) ? CPSS_DP_GREEN_E : CPSS_DP_RED_E;

    /* The DP assigned to CPU-to-network control traffic or
       network-to-CPU traffic. */
    status = prvCpssHwPpGetRegField(devNum, regAddr, startBit + 5, 1, &hwValue);
    if (status != GT_OK)
    {
        return status;
    }

    *ctrlDpPtr = (hwValue == 0) ? CPSS_DP_GREEN_E : CPSS_DP_RED_E;

    return status;
}

/**
* @internal cpssDxChCscdCtrlQosGet function
* @endinternal
*
* @brief   Get control packets TC and DP if Control Remap QoS enabled on a port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] ctrlTcPtr                - The TC assigned to control packets forwarded to
*                                      cascading ports
* @param[out] ctrlDpPtr                - The DP assigned to CPU-to-network control traffic or
*                                      network-to-CPU traffic.
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
* @param[out] cpuToCpuDpPtr            - The DP assigned to CPU-to-CPU control traffic
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_VALUE             - on wrong ctrlDp or cpuToCpuDp level value or tc value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChCscdCtrlQosGet
(
   IN  GT_U8             devNum,
   OUT GT_U8             *ctrlTcPtr,
   OUT CPSS_DP_LEVEL_ENT *ctrlDpPtr,
   OUT CPSS_DP_LEVEL_ENT *cpuToCpuDpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCtrlQosGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ctrlTcPtr, ctrlDpPtr, cpuToCpuDpPtr));

    rc = internal_cpssDxChCscdCtrlQosGet(devNum, ctrlTcPtr, ctrlDpPtr, cpuToCpuDpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ctrlTcPtr, ctrlDpPtr, cpuToCpuDpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdRemapDataQosTblSet function
* @endinternal
*
* @brief   Set table to remap Data packets QoS parameters
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] tc                       - original packet TC (APPLICABLE RANGES: 0..7)
* @param[in] dp                       - original packet DP
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
* @param[in] remapTc                  - TC assigned to data packets with DP and TC (APPLICABLE RANGES: 0..7)
* @param[in] remapDp                  - DP assigned to data packets with DP and TC
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_VALUE             - on wrong DP or dp level value or tc value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdRemapDataQosTblSet
(
   IN GT_U8              devNum,
   IN GT_U8              tc,
   IN CPSS_DP_LEVEL_ENT  dp,
   IN GT_U8              remapTc,
   IN CPSS_DP_LEVEL_ENT  remapDp
)
{
    GT_STATUS   status;
    GT_U32      regAddr;    /* register address */
    GT_U32      offset;     /* field offset */
    GT_U32      value;      /* field value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_COS_CHECK_DP_MAC(dp);
    PRV_CPSS_DXCH_COS_CHECK_DP_MAC(remapDp);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(remapTc);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        if (dp == CPSS_DP_GREEN_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.tcDp0CscdDataRemap;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.tcDp1CscdDataRemap;
        }
    }
    else
    {
        if (dp == CPSS_DP_GREEN_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.distributor.tcDpRemapping[0];
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.distributor.tcDpRemapping[1];
        }

    }

    offset = 3 * tc;
    value = remapTc;
    /* Set TC field */
    status = prvCpssHwPpSetRegField(devNum, regAddr, offset, 3, value);
    if (status != GT_OK)
    {
        return status;
    }

    offset = 24 + tc;
    value = (remapDp == CPSS_DP_GREEN_E) ? 0 : 1;
    /* Set DP field */
    status = prvCpssHwPpSetRegField(devNum, regAddr, offset, 1, value);

    return status;
}

/**
* @internal cpssDxChCscdRemapDataQosTblSet function
* @endinternal
*
* @brief   Set table to remap Data packets QoS parameters
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] tc                       - original packet TC (APPLICABLE RANGES: 0..7)
* @param[in] dp                       - original packet DP
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
* @param[in] remapTc                  - TC assigned to data packets with DP and TC (APPLICABLE RANGES: 0..7)
* @param[in] remapDp                  - DP assigned to data packets with DP and TC
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_VALUE             - on wrong DP or dp level value or tc value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdRemapDataQosTblSet
(
   IN GT_U8              devNum,
   IN GT_U8              tc,
   IN CPSS_DP_LEVEL_ENT  dp,
   IN GT_U8              remapTc,
   IN CPSS_DP_LEVEL_ENT  remapDp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdRemapDataQosTblSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tc, dp, remapTc, remapDp));

    rc = internal_cpssDxChCscdRemapDataQosTblSet(devNum, tc, dp, remapTc, remapDp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tc, dp, remapTc, remapDp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdRemapDataQosTblGet function
* @endinternal
*
* @brief   Get QoS parameters from table to remap Data packets
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] tc                       - original packet TC (APPLICABLE RANGES: 0..7)
* @param[in] dp                       - original packet DP
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
*
* @param[out] remapTcPtr               - TC assigned to data packets with DP and TC
* @param[out] remapDpPtr               - DP assigned to data packets with DP and TC
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_VALUE             - on wrong DP or dp level value or tc value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static GT_STATUS internal_cpssDxChCscdRemapDataQosTblGet
(
   IN  GT_U8              devNum,
   IN  GT_U8              tc,
   IN  CPSS_DP_LEVEL_ENT  dp,
   OUT GT_U8              *remapTcPtr,
   OUT CPSS_DP_LEVEL_ENT  *remapDpPtr
)
{
    GT_STATUS   status;
    GT_U32      regAddr;    /* register address */
    GT_U32      offset;     /* field offset */
    GT_U32      hwValue;    /* hwValue value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_COS_CHECK_DP_MAC(dp);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);
    CPSS_NULL_PTR_CHECK_MAC(remapTcPtr);
    CPSS_NULL_PTR_CHECK_MAC(remapDpPtr);


    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        if (dp == CPSS_DP_GREEN_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.tcDp0CscdDataRemap;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.tcDp1CscdDataRemap;
        }
    }
    else
    {
        if (dp == CPSS_DP_GREEN_E)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.distributor.tcDpRemapping[0];
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.distributor.tcDpRemapping[1];
        }
    }

    offset = 3 * tc;
    /* Get TC field */
    status = prvCpssHwPpGetRegField(devNum, regAddr, offset, 3, &hwValue);
    if (status != GT_OK)
    {
        return status;
    }

    *remapTcPtr = (GT_U8)hwValue;

    offset = 24 + tc;
    /* Get DP field */
    status = prvCpssHwPpGetRegField(devNum, regAddr, offset, 1, &hwValue);
    if (status != GT_OK)
    {
        return status;
    }

    *remapDpPtr = (hwValue == 0) ? CPSS_DP_GREEN_E : CPSS_DP_RED_E;

    return status;
}

/**
* @internal cpssDxChCscdRemapDataQosTblGet function
* @endinternal
*
* @brief   Get QoS parameters from table to remap Data packets
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] tc                       - original packet TC (APPLICABLE RANGES: 0..7)
* @param[in] dp                       - original packet DP
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
*
* @param[out] remapTcPtr               - TC assigned to data packets with DP and TC
* @param[out] remapDpPtr               - DP assigned to data packets with DP and TC
*                                      DxCh devices support only :
*                                      CPSS_DP_RED_E and CPSS_DP_GREEN_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_VALUE             - on wrong DP or dp level value or tc value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChCscdRemapDataQosTblGet
(
   IN  GT_U8              devNum,
   IN  GT_U8              tc,
   IN  CPSS_DP_LEVEL_ENT  dp,
   OUT GT_U8              *remapTcPtr,
   OUT CPSS_DP_LEVEL_ENT  *remapDpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdRemapDataQosTblGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tc, dp, remapTcPtr, remapDpPtr));

    rc = internal_cpssDxChCscdRemapDataQosTblGet(devNum, tc, dp, remapTcPtr, remapDpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tc, dp, remapTcPtr, remapDpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdDsaSrcDevFilterSet function
* @endinternal
*
* @brief   Enable/Disable filtering the ingress DSA tagged packets in which
*         source device (Src Dev) equals to local device number.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] enableOwnDevFltr         - enable/disable ingress DSA loop filter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdDsaSrcDevFilterSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enableOwnDevFltr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      value;      /* field value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig1;
    }
    else
    {
        /* getting register address */
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bridgeRegs.bridgeGlobalConfigRegArray[1];
    }

    /* getting the bit value to be set */
    value = (enableOwnDevFltr == GT_TRUE) ? 0 : 1;

    /* set the DSA loop filter DISABLE bit */
    return prvCpssHwPpSetRegField(devNum, regAddr, 31, 1, value);
}

/**
* @internal cpssDxChCscdDsaSrcDevFilterSet function
* @endinternal
*
* @brief   Enable/Disable filtering the ingress DSA tagged packets in which
*         source device (Src Dev) equals to local device number.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] enableOwnDevFltr         - enable/disable ingress DSA loop filter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdDsaSrcDevFilterSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enableOwnDevFltr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdDsaSrcDevFilterSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enableOwnDevFltr));

    rc = internal_cpssDxChCscdDsaSrcDevFilterSet(devNum, enableOwnDevFltr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enableOwnDevFltr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdDsaSrcDevFilterGet function
* @endinternal
*
* @brief   get value of Enable/Disable filtering the ingress DSA tagged packets in which
*         source device (Src Dev) equals to local device number.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enableOwnDevFltrPtr      - (pointer to) enable/disable ingress DSA loop filter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdDsaSrcDevFilterGet
(
    IN GT_U8        devNum,
    OUT GT_BOOL      *enableOwnDevFltrPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      value;      /* field value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enableOwnDevFltrPtr);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig1;
    }
    else
    {
        /* getting register address */
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bridgeRegs.bridgeGlobalConfigRegArray[1];
    }

    /* Get the DSA loop filter DISABLE bit */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 31, 1, &value);

    *enableOwnDevFltrPtr = (value == 1) ? GT_FALSE : GT_TRUE;

    return rc;

}

/**
* @internal cpssDxChCscdDsaSrcDevFilterGet function
* @endinternal
*
* @brief   get value of Enable/Disable filtering the ingress DSA tagged packets in which
*         source device (Src Dev) equals to local device number.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enableOwnDevFltrPtr      - (pointer to) enable/disable ingress DSA loop filter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdDsaSrcDevFilterGet
(
    IN GT_U8        devNum,
    OUT GT_BOOL      *enableOwnDevFltrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdDsaSrcDevFilterGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enableOwnDevFltrPtr));

    rc = internal_cpssDxChCscdDsaSrcDevFilterGet(devNum, enableOwnDevFltrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enableOwnDevFltrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdHyperGPortCrcModeSet function
* @endinternal
*
* @brief   Set CRC mode to be standard 4 bytes or proprietary one byte CRC mode.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - TX/RX cascade port direction
* @param[in] crcMode                  - TX/RX HyperG.link CRC Mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdHyperGPortCrcModeSet
(
    IN GT_U8                             devNum,
    IN GT_PHYSICAL_PORT_NUM              portNum,
    IN CPSS_PORT_DIRECTION_ENT           portDirection,
    IN CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT  crcMode
)
{
    GT_STATUS status;               /* return status */
    GT_U32 value;                   /* write register's value */
    GT_U32 hwCrcMode = 1;           /* CRC bit value */
    GT_U32 hwStripCrcMode = 0;      /* strip CRC bit value */
    GT_U32 hwIvalidCrcMode;         /* invalid CRC mode value */
    GT_U32 rxDmaRecalcCrc = 0;      /* RxDMA CRC recalculation configuration */
    GT_BOOL inPort = GT_FALSE;      /* ingress or egress port */
    GT_U32 regAddr;                 /* register address */
    GT_U32 mask;                    /* Mask Bits */
    GT_U32  portGroupId;            /*the port group Id - support multi-port-groups device */
    GT_U32   localPort;             /* local port - support multi-port-groups device */
    GT_U32 portMacNum;              /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType; /* MAC type */
    GT_U32 crcBytesNum;             /* HW value for CRC */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        PRV_CPSS_CSCD_PORT_CRC_SW_TO_HW_VALUE_CONVERT_MAC(crcMode, crcBytesNum);

        status = prvCpssDxChFalconPortMacCrcModeSet(devNum,portNum,portDirection,crcBytesNum);

        if (status != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(status, LOG_ERROR_NO_MSG);
        }
        /* for now the setting of rx Recalc data for SIP 6 was not proven as working */
        return status;
    }
    else if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
       PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        PRV_CPSS_CSCD_PORT_CRC_SW_TO_HW_VALUE_CONVERT_MAC(crcMode, crcBytesNum);

        status = prvCpssDxChBobcat2PortMacCrcModeSet(devNum,portNum,portDirection,crcBytesNum);
        if (status != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(status, LOG_ERROR_NO_MSG);
        }
    }

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    switch (crcMode)
    {
        case CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E:
            hwCrcMode = 0;
            hwStripCrcMode = 1;
            rxDmaRecalcCrc = 1;
            break;
        case CPSS_DXCH_CSCD_PORT_CRC_TWO_BYTES_E:
        case CPSS_DXCH_CSCD_PORT_CRC_THREE_BYTES_E:
            if(!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            hwCrcMode = 0;
            break;
        case CPSS_DXCH_CSCD_PORT_CRC_FOUR_BYTES_E:
            hwCrcMode = 1;
            hwStripCrcMode = 0;
            rxDmaRecalcCrc = 0;
            break;
        case CPSS_DXCH_CSCD_PORT_CRC_ZERO_BYTES_E:
            if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            rxDmaRecalcCrc = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (portDirection)
    {
        case CPSS_PORT_DIRECTION_RX_E:
            value = hwCrcMode << 12;
            inPort = GT_TRUE;
            break;
        case CPSS_PORT_DIRECTION_TX_E:
            value = hwCrcMode << 13;
            break;
        case CPSS_PORT_DIRECTION_BOTH_E:
            value = (hwCrcMode << 12) | (hwCrcMode << 13);
            inPort = GT_TRUE;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* In Lion2 families fields shift one bit right (>>1)*/
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        value = value>>1;
        mask = 0x1800;
    }
    else
    {
        mask = 0x3000;
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacNum);

    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);
    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,macType,&regAddr);

    if((!PRV_CPSS_SIP_5_CHECK_MAC(devNum)) &&
       (!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        status = prvCpssHwPpPortGroupWriteRegBitMask(devNum, portGroupId,regAddr, mask, value);
        if (status != GT_OK)
        {
            return status;
        }
    }

    hwIvalidCrcMode = (crcMode == CPSS_DXCH_CSCD_PORT_CRC_FOUR_BYTES_E) ? 0 : 1;
    if (inPort == GT_TRUE)
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            status = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                         portNum, /*global port*/
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                         SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INVALID_CRC_MODE_E, /* field name */
                                         PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                         hwIvalidCrcMode);
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                      haRegs.invalidCrcModeConfigReg[OFFSET_TO_WORD_MAC(localPort)];
            /* Set invalid CRC format for relevant ingress port */
            status = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr,
                                     OFFSET_TO_BIT_MAC(localPort), 1, hwIvalidCrcMode);
        }

        if(GT_OK != status)
        {
            return status;
        }

        /*
            XCAT3:
                HA will not remove 1B CRC, the one byte CRC should be removed by the port.
                So for 1B CRC strip CRC MAC feature should be enabled.
                For 4B CRC it should be disabled and HA will remove it.
        */
        if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

            /* XG/XGL configuration */
            if(macType > PRV_CPSS_PORT_GE_E)
            {
                PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,macType,&regAddr);
                status = prvCpssHwPpSetRegField(devNum, regAddr, 11, 1, hwStripCrcMode);
                if (status != GT_OK)
                {
                    return status;
                }
            }
        }

        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
            GT_U32  rxDmaNum;

            status = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
            if(GT_OK != status)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(status, LOG_ERROR_NO_MSG);
            }
            rxDmaNum = portMapShadowPtr->portMap.rxDmaNum;

            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).
                    configs.channelConfig.channelGeneralConfigs[rxDmaNum];
                /* <Recalc CRC> */
                status = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, rxDmaRecalcCrc);
            }
            else
            {
                regAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAConfigs.SCDMAConfig0[rxDmaNum];

                status = prvCpssDrvHwPpSetRegField(devNum, regAddr, 4, 1, rxDmaRecalcCrc);
            }
            if(GT_OK != status)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(status, LOG_ERROR_NO_MSG);
            }
        }

    }

    return GT_OK;
}

/**
* @internal cpssDxChCscdHyperGPortCrcModeSet function
* @endinternal
*
* @brief   Set CRC mode to be standard 4 bytes or proprietary one byte CRC mode.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - TX/RX cascade port direction
* @param[in] crcMode                  - TX/RX HyperG.link CRC Mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdHyperGPortCrcModeSet
(
    IN GT_U8                             devNum,
    IN GT_PHYSICAL_PORT_NUM              portNum,
    IN CPSS_PORT_DIRECTION_ENT           portDirection,
    IN CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT  crcMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdHyperGPortCrcModeSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portDirection, crcMode));

    rc = internal_cpssDxChCscdHyperGPortCrcModeSet(devNum, portNum, portDirection, crcMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portDirection, crcMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdHyperGPortCrcModeGet function
* @endinternal
*
* @brief   Get CRC mode (standard 4 bytes or proprietary one byte).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - TX/RX cascade port direction (ingress or egress)
*
* @param[out] crcModePtr               - TX/RX HyperG.link CRC Mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static GT_STATUS internal_cpssDxChCscdHyperGPortCrcModeGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  CPSS_PORT_DIRECTION_ENT           portDirection,
    OUT CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT  *crcModePtr
)
{
    GT_STATUS rc;                   /* return code */
    GT_U32 hwValue;                 /* read hw value */
    GT_U32 shiftValue;              /* dependent on family shift value */
    GT_U32 hwCrcMode;               /* CRC bit value */
    GT_U32 regAddr;                 /* register address */
    GT_U32 mask;                    /* Mask Bits */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(crcModePtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        GT_U32  crcBytesNum;                       /* number of bytes in CRC */

        rc = prvCpssDxChFalconPortMacCrcModeGet(devNum,portNum,portDirection,/*OUT*/&crcBytesNum);

        PRV_CPSS_CSCD_PORT_CRC_HW_TO_SW_VALUE_CONVERT_MAC(crcBytesNum,*crcModePtr);

        return rc;
    }
    else if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
      (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        GT_U32  crcBytesNum;                       /* number of bytes in CRC */

        if (portDirection != CPSS_PORT_DIRECTION_RX_E && portDirection != CPSS_PORT_DIRECTION_TX_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDxChBobcat2PortMacCrcModeGet(devNum,portNum,portDirection,/*OUT*/&crcBytesNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        PRV_CPSS_CSCD_PORT_CRC_HW_TO_SW_VALUE_CONVERT_MAC(crcBytesNum,*crcModePtr);

        return GT_OK;
    }

    /* In Lion2 families */
   if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        mask = 0x1800;
        shiftValue = 1;
    }
    else
    {
        mask = 0x3000;
        shiftValue = 0;
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    PRV_CPSS_DXCH_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum),&regAddr);

    rc = prvCpssHwPpPortGroupReadRegBitMask(devNum, portGroupId,regAddr, mask, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (portDirection)
    {
        case CPSS_PORT_DIRECTION_RX_E:
            hwCrcMode = (hwValue >> (12 - shiftValue)) & 0x1;
            break;
        case CPSS_PORT_DIRECTION_TX_E:
            hwCrcMode = (hwValue >> (13 - shiftValue)) & 0x1;
            break;
        case CPSS_PORT_DIRECTION_BOTH_E:
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *crcModePtr = (hwCrcMode == 1) ? CPSS_DXCH_CSCD_PORT_CRC_FOUR_BYTES_E : CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E;

    return rc;
}

/**
* @internal cpssDxChCscdHyperGPortCrcModeGet function
* @endinternal
*
* @brief   Get CRC mode (standard 4 bytes or proprietary one byte).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - TX/RX cascade port direction (ingress or egress)
*
* @param[out] crcModePtr               - TX/RX HyperG.link CRC Mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChCscdHyperGPortCrcModeGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  CPSS_PORT_DIRECTION_ENT           portDirection,
    OUT CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT  *crcModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdHyperGPortCrcModeGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portDirection, crcModePtr));

    rc = internal_cpssDxChCscdHyperGPortCrcModeGet(devNum, portNum, portDirection, crcModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portDirection, crcModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdFastFailoverFastStackRecoveryEnableSet function
* @endinternal
*
* @brief   Enable/Disable fast stack recovery.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  fast stack recovery
*                                      GT_FALSE: disable fast stack recovery
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdFastFailoverFastStackRecoveryEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      hwValue;    /* hw field value   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    /* getting register address */
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).DSAConfig;
    }
    else
    {
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.fastStack;
    }

    /* getting the bit value to be set */
    hwValue = BOOL2BIT_MAC(enable);

    /* set the Fast Stack Enable bit */
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, hwValue);
}

/**
* @internal cpssDxChCscdFastFailoverFastStackRecoveryEnableSet function
* @endinternal
*
* @brief   Enable/Disable fast stack recovery.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  fast stack recovery
*                                      GT_FALSE: disable fast stack recovery
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdFastFailoverFastStackRecoveryEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdFastFailoverFastStackRecoveryEnableSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChCscdFastFailoverFastStackRecoveryEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdFastFailoverFastStackRecoveryEnableGet function
* @endinternal
*
* @brief   Get the status of fast stack recovery (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) fast stack recovery status
*                                      GT_TRUE: fast stack recovery enabled
*                                      GT_FALSE: fast stack recovery disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdFastFailoverFastStackRecoveryEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *enablePtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      hwValue;    /* hw field value   */
    GT_STATUS   rc;         /* return code      */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* getting register address */
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).DSAConfig;
    }
    else
    {
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.fastStack;
    }

    /* get the Fast Stack Enable bit */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &hwValue);

    *enablePtr = BIT2BOOL_MAC(hwValue);

    return rc;
}

/**
* @internal cpssDxChCscdFastFailoverFastStackRecoveryEnableGet function
* @endinternal
*
* @brief   Get the status of fast stack recovery (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) fast stack recovery status
*                                      GT_TRUE: fast stack recovery enabled
*                                      GT_FALSE: fast stack recovery disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdFastFailoverFastStackRecoveryEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdFastFailoverFastStackRecoveryEnableGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChCscdFastFailoverFastStackRecoveryEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdFastFailoverSecondaryTargetPortMapSet function
* @endinternal
*
* @brief   Set secondary target port map.
*         If the device is the device where the ring break occured, the
*         packet is looped on the ingress port and is egressed according to
*         Secondary Target Port Map. Also "packetIsLooped" bit is
*         set in DSA tag.
*         If the device receives a packet with "packetIsLooped" bit is set
*         in DSA tag, the packet is forwarded according to Secondary Target Port
*         Map.
*         Device MAP table (cpssDxChCscdDevMapTableSet) is not used in the case.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - ingress port number
* @param[in] secondaryTargetPort      - secondary target port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - when secondaryTargetPort is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdFastFailoverSecondaryTargetPortMapSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_PHYSICAL_PORT_NUM         secondaryTargetPort
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  portGroupId;/*the port group Id of primary port - support multi-port-groups device */
    GT_U32  secondaryPortHemNum;/* the hemisphere number of secondary port  */
    GT_U32  localSecondaryPort; /* the local port number of the secondary port */
    GT_U32  numOfBits; /* number of bits to set from secondaryTargetPort value*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
        PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, secondaryTargetPort);

        return prvCpssDxChWriteTableEntry(devNum,
            CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_EFT_FAST_STACK_FAILOVER_SECONDARY_TARGET_PORT_E,
            portNum,
            &secondaryTargetPort);
    }

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* getting register address */
        if(portNum >= 64)
        {
            /* protect the DB of egrTxQConf.secondaryTargetPortMapTbl[] */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "MAC[%d] is not supported ",portNum);
        }

        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.
                                                secondaryTargetPortMapTbl[portNum];

        localSecondaryPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,secondaryTargetPort);

        if( localSecondaryPort >= BIT_5 )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* set the Fast Stack Failover secondary port */
        return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 0, 5, localSecondaryPort);
    }
    else
    {


        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_LION2_FAST_FAILOVER_WA_E))
        {
            secondaryPortHemNum = PRV_CPSS_DXCH_GLOBAL_TO_HEM_NUMBER(devNum, secondaryTargetPort);

            /* the secondary port must be in the same hemisphere as the primary one */
            if( secondaryPortHemNum != PRV_CPSS_DXCH_GLOBAL_TO_HEM_NUMBER(devNum, portNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.
            devMapTableConfig.secondaryTargetPortTable[portNum];

        secondaryTargetPort = PRV_CPSS_DXCH_GLOBAL_TO_HEM_LOCAL_PORT(devNum,secondaryTargetPort);

        if( secondaryTargetPort > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PHY_PORT_MAC(devNum) )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        numOfBits = PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).phyPort;

        /* set the Fast Stack Failover secondary port */
        return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 0, numOfBits, secondaryTargetPort);
    }
}

/**
* @internal cpssDxChCscdFastFailoverSecondaryTargetPortMapSet function
* @endinternal
*
* @brief   Set secondary target port map.
*         If the device is the device where the ring break occured, the
*         packet is looped on the ingress port and is egressed according to
*         Secondary Target Port Map. Also "packetIsLooped" bit is
*         set in DSA tag.
*         If the device receives a packet with "packetIsLooped" bit is set
*         in DSA tag, the packet is forwarded according to Secondary Target Port
*         Map.
*         Device MAP table (cpssDxChCscdDevMapTableSet) is not used in the case.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - ingress port number
* @param[in] secondaryTargetPort      - secondary target port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - when secondaryTargetPort is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdFastFailoverSecondaryTargetPortMapSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_PHYSICAL_PORT_NUM         secondaryTargetPort
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdFastFailoverSecondaryTargetPortMapSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, secondaryTargetPort));

    rc = internal_cpssDxChCscdFastFailoverSecondaryTargetPortMapSet(devNum, portNum, secondaryTargetPort);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, secondaryTargetPort));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdFastFailoverSecondaryTargetPortMapGet function
* @endinternal
*
* @brief   Get Secondary Target Port Map for given device.
*         If the device is the device where the ring break occured, the
*         packet is looped on the ingress port and is egressed according to
*         Secondary Target Port Map. Also "packetIsLooped" bit is
*         set in DSA tag.
*         If the device receives a packet with "packetIsLooped" bit is set
*         in DSA tag, the packet is forwarded according to Secondary Target Port
*         Map.
*         Device MAP table (cpssDxChCscdDevMapTableSet) is not used in the case.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - ingress port number
*
* @param[out] secondaryTargetPortPtr   - (pointer to) secondary target port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdFastFailoverSecondaryTargetPortMapGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_PHYSICAL_PORT_NUM        *secondaryTargetPortPtr
)
{
    GT_U32      regAddr;    /* register address             */
    GT_U32      hwValue;    /* value to read from register  */
    GT_STATUS   rc;         /* return code                  */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32      numOfBits;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(secondaryTargetPortPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

        return prvCpssDxChReadTableEntry(devNum,
            CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_EFT_FAST_STACK_FAILOVER_SECONDARY_TARGET_PORT_E,
            portNum,
            secondaryTargetPortPtr);
    }

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* getting register address */
        if(portNum >= 64)
        {
            /* protect the DB of egrTxQConf.secondaryTargetPortMapTbl[] */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "MAC[%d] is not supported ",portNum);
        }

        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.
                                            secondaryTargetPortMapTbl[portNum];

        /* get the Secondary target port */
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, 0, 5, &hwValue);

        *secondaryTargetPortPtr =
            PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum, portGroupId, hwValue);
    }
    else
    {
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.
            devMapTableConfig.secondaryTargetPortTable[portNum];

        numOfBits = PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).phyPort;

        /* get the Fast Stack Failover secondary port */
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 0, numOfBits, &hwValue);
        if(rc != GT_OK)
            return rc;

        *secondaryTargetPortPtr = PRV_CPSS_DXCH_HEM_LOCAL_TO_GLOBAL_PORT(devNum,portGroupId,hwValue);
    }

    return rc;
}

/**
* @internal cpssDxChCscdFastFailoverSecondaryTargetPortMapGet function
* @endinternal
*
* @brief   Get Secondary Target Port Map for given device.
*         If the device is the device where the ring break occured, the
*         packet is looped on the ingress port and is egressed according to
*         Secondary Target Port Map. Also "packetIsLooped" bit is
*         set in DSA tag.
*         If the device receives a packet with "packetIsLooped" bit is set
*         in DSA tag, the packet is forwarded according to Secondary Target Port
*         Map.
*         Device MAP table (cpssDxChCscdDevMapTableSet) is not used in the case.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - ingress port number
*
* @param[out] secondaryTargetPortPtr   - (pointer to) secondary target port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdFastFailoverSecondaryTargetPortMapGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_PHYSICAL_PORT_NUM        *secondaryTargetPortPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdFastFailoverSecondaryTargetPortMapGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, secondaryTargetPortPtr));

    rc = internal_cpssDxChCscdFastFailoverSecondaryTargetPortMapGet(devNum, portNum, secondaryTargetPortPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, secondaryTargetPortPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet function
* @endinternal
*
* @brief   Enable/Disable fast failover loopback termination
*         for single-destination packets.
*         There are two configurable options for forwarding single-destination
*         packets that are looped-back across the ring:
*         - Termination Disabled.
*         Unconditionally forward the looped-back packet to the configured
*         backup ring port (for the given ingress ring port) on all the ring
*         devices until it reaches the far-end device where it is again
*         internally looped-back on the ring port and then forward it normally.
*         - Termination Enabled.
*         The looped-back packet passes through the
*         ring until it reaches the target device where it is egressed on its
*         target port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  terminating local loopback
*                                      for fast stack recovery
*                                      GT_FALSE: disable terminating local loopback
*                                      for fast stack recovery
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_U32      regAddr;    /* register address             */
    GT_U32      hwValue;    /* value to write to register   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    hwValue = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr =  PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).egrFilterConfigs.egrFiltersEnable;

        /* <TerminateLocalLoopbackEn> */
        return prvCpssHwPpSetRegField(devNum, regAddr, 1, 1, hwValue);
    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* getting register address */
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.
                                                            txQueueDpToCfiReg;
        /* set the local loopback termination for fast stack bit */
        return prvCpssHwPpSetRegField(devNum, regAddr, 4, 1, hwValue);
    }
    else
    {
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.
            filterConfig.globalEnables;

        /* set the Enable ending of loopback on Fast Stack Failover */
        return prvCpssHwPpSetRegField(devNum, regAddr, 2, 1, hwValue);
    }
}

/**
* @internal cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet function
* @endinternal
*
* @brief   Enable/Disable fast failover loopback termination
*         for single-destination packets.
*         There are two configurable options for forwarding single-destination
*         packets that are looped-back across the ring:
*         - Termination Disabled.
*         Unconditionally forward the looped-back packet to the configured
*         backup ring port (for the given ingress ring port) on all the ring
*         devices until it reaches the far-end device where it is again
*         internally looped-back on the ring port and then forward it normally.
*         - Termination Enabled.
*         The looped-back packet passes through the
*         ring until it reaches the target device where it is egressed on its
*         target port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  terminating local loopback
*                                      for fast stack recovery
*                                      GT_FALSE: disable terminating local loopback
*                                      for fast stack recovery
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet function
* @endinternal
*
* @brief   Get the status of fast failover loopback termination
*         for single-destination packets (Enabled/Disabled).
*         There are two configurable options for forwarding single-destination
*         packets that are looped-back across the ring:
*         - Termination Disabled.
*         Unconditionally forward the looped-back packet to the configured
*         backup ring port (for the given ingress ring port) on all the ring
*         devices until it reaches the far-end device where it is again
*         internally looped-back on the ring port and then forward it normally.
*         - Termination Enabled.
*         The looped-back packet passes through the
*         ring until it reaches the target device where it is egressed on its
*         target port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE: terminating local loopback
*                                      for fast stack recovery enabled
*                                      GT_FALSE: terminating local loopback
*                                      for fast stack recovery disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *enablePtr
)
{
    GT_U32      regAddr;    /* register address             */
    GT_U32      hwValue;    /* value to write to register   */
    GT_STATUS   rc;         /* return code                  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr =  PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).egrFilterConfigs.egrFiltersEnable;

        /* <TerminateLocalLoopbackEn> */
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 1, 1, &hwValue);
    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* getting register address */
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.
                                                            txQueueDpToCfiReg;
        /* get the local loopback termination for fast stack bit */
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 4, 1, &hwValue);
    }
    else
    {
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.
            filterConfig.globalEnables;
        /* get the Enable ending of loopback on Fast Stack Failover */
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 2, 1, &hwValue);
    }

    *enablePtr = BIT2BOOL_MAC(hwValue);

    return rc;
}

/**
* @internal cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet function
* @endinternal
*
* @brief   Get the status of fast failover loopback termination
*         for single-destination packets (Enabled/Disabled).
*         There are two configurable options for forwarding single-destination
*         packets that are looped-back across the ring:
*         - Termination Disabled.
*         Unconditionally forward the looped-back packet to the configured
*         backup ring port (for the given ingress ring port) on all the ring
*         devices until it reaches the far-end device where it is again
*         internally looped-back on the ring port and then forward it normally.
*         - Termination Enabled.
*         The looped-back packet passes through the
*         ring until it reaches the target device where it is egressed on its
*         target port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE: terminating local loopback
*                                      for fast stack recovery enabled
*                                      GT_FALSE: terminating local loopback
*                                      for fast stack recovery disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdFastFailoverPortIsLoopedSet function
* @endinternal
*
* @brief   Enable/disable Fast Failover on a failed port.
*         When port is looped and get packet with DSA tag <Packet is Looped> = 0,
*         then device do next:
*         - set DSA tag <Packet is Looped> = 1
*         - bypass ingress and egress processing
*         - send packet through egress port that defined in secondary target
*         port map (cpssDxChCscdFastFailoverSecondaryTargetPortMapSet).
*         When port is looped and get packet with DSA tag <Packet is Looped> = 1,
*         then device do next:
*         - set DSA tag <Packet is Looped> = 0
*         - Apply usual ingress and egress processing
*         When port is not looped and get packet then device do next:
*         - Apply usual ingress and egress processing
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU)
* @param[in] isLooped                 -  GT_FALSE - Port is not looped.
*                                      GT_TRUE - Port is looped.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdFastFailoverPortIsLoopedSet
(
  IN GT_U8                     devNum,
  IN GT_PHYSICAL_PORT_NUM      portNum,
  IN GT_BOOL                   isLooped
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;    /* register address             */
    GT_U32      hwValue;    /* value to write to register   */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32       localPort;  /* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);


    hwValue = BOOL2BIT_MAC(isLooped);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* write to TTI-Physical-Port-Attribute table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                                         portNum,/*global port*/
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                         SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_LOOPED_E, /* field name */
                                         PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                         hwValue);
    }
    else
    {
        /* getting register address */
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.loopPortReg;

        if (localPort == CPSS_CPU_PORT_NUM_CNS)
        {
            localPort = 31;
        }

        /* set port is looped per port bit for fast stack failover */
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, localPort, 1, hwValue);
    }

    return rc;

}

/**
* @internal cpssDxChCscdFastFailoverPortIsLoopedSet function
* @endinternal
*
* @brief   Enable/disable Fast Failover on a failed port.
*         When port is looped and get packet with DSA tag <Packet is Looped> = 0,
*         then device do next:
*         - set DSA tag <Packet is Looped> = 1
*         - bypass ingress and egress processing
*         - send packet through egress port that defined in secondary target
*         port map (cpssDxChCscdFastFailoverSecondaryTargetPortMapSet).
*         When port is looped and get packet with DSA tag <Packet is Looped> = 1,
*         then device do next:
*         - set DSA tag <Packet is Looped> = 0
*         - Apply usual ingress and egress processing
*         When port is not looped and get packet then device do next:
*         - Apply usual ingress and egress processing
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU)
* @param[in] isLooped                 -  GT_FALSE - Port is not looped.
*                                      GT_TRUE - Port is looped.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdFastFailoverPortIsLoopedSet
(
  IN GT_U8                     devNum,
  IN GT_PHYSICAL_PORT_NUM      portNum,
  IN GT_BOOL                   isLooped
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdFastFailoverPortIsLoopedSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isLooped));

    rc = internal_cpssDxChCscdFastFailoverPortIsLoopedSet(devNum, portNum, isLooped);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isLooped));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdFastFailoverPortIsLoopedGet function
* @endinternal
*
* @brief   Get status (Enable/Disable) of Fast Failover on the failed port.
*         When port is looped and get packet with DSA tag <Packet is Looped> = 0,
*         then device do next:
*         - set DSA tag <Packet is Looped> = 1
*         - bypass ingress and egress processing
*         - send packet through egress port that defined in secondary target
*         port map (cpssDxChCscdFastFailoverSecondaryTargetPortMapSet).
*         When port is looped and get packet with DSA tag <Packet is Looped> = 1,
*         then device do next:
*         - set DSA tag <Packet is Looped> = 0
*         - Apply usual ingress and egress processing
*         When port is not looped and get packet then device do next:
*         - Apply usual ingress and egress processing
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU)
*
* @param[out] isLoopedPtr              - (pointer to) Is Looped
*                                      GT_FALSE - Port is not looped.
*                                      GT_TRUE - Port is looped.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdFastFailoverPortIsLoopedGet
(
  IN GT_U8                  devNum,
  IN GT_PHYSICAL_PORT_NUM   portNum,
  OUT GT_BOOL               *isLoopedPtr
)
{
    GT_U32      regAddr;    /* register address             */
    GT_U32      hwValue;    /* value to write to register   */
    GT_STATUS   rc;         /* return code                  */
    GT_U32      portGroupId;/* the port group Id - support multi-port-groups device */
    GT_U32       localPort;  /* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(isLoopedPtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* write to TTI-Physical-Port-Attribute table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                         CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                                         portNum,/*global port*/
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                         SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_LOOPED_E, /* field name */
                                         PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                         &hwValue);
    }
    else
    {
        /* getting register address */
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.loopPortReg;

        if (localPort == CPSS_CPU_PORT_NUM_CNS)
        {
            localPort = 31;
        }

        /* get port is looped per port bit for fast stack failover */
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, localPort, 1, &hwValue);

    }

    *isLoopedPtr = BIT2BOOL_MAC(hwValue);

    return rc;
}

/**
* @internal cpssDxChCscdFastFailoverPortIsLoopedGet function
* @endinternal
*
* @brief   Get status (Enable/Disable) of Fast Failover on the failed port.
*         When port is looped and get packet with DSA tag <Packet is Looped> = 0,
*         then device do next:
*         - set DSA tag <Packet is Looped> = 1
*         - bypass ingress and egress processing
*         - send packet through egress port that defined in secondary target
*         port map (cpssDxChCscdFastFailoverSecondaryTargetPortMapSet).
*         When port is looped and get packet with DSA tag <Packet is Looped> = 1,
*         then device do next:
*         - set DSA tag <Packet is Looped> = 0
*         - Apply usual ingress and egress processing
*         When port is not looped and get packet then device do next:
*         - Apply usual ingress and egress processing
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU)
*
* @param[out] isLoopedPtr              - (pointer to) Is Looped
*                                      GT_FALSE - Port is not looped.
*                                      GT_TRUE - Port is looped.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdFastFailoverPortIsLoopedGet
(
  IN GT_U8                  devNum,
  IN GT_PHYSICAL_PORT_NUM   portNum,
  OUT GT_BOOL               *isLoopedPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdFastFailoverPortIsLoopedGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isLoopedPtr));

    rc = internal_cpssDxChCscdFastFailoverPortIsLoopedGet(devNum, portNum, isLoopedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isLoopedPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/****************************** Private Functions *****************************/

/**
* @internal internal_cpssDxChCscdQosPortTcRemapEnableSet function
* @endinternal
*
* @brief   Enable/Disable Traffic Class Remapping on cascading port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - cascading port number or CPU port
* @param[in] enable                   - GT_TRUE:  Traffic Class remapping
*                                      GT_FALSE: disable Traffic Class remapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdQosPortTcRemapEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              enable
)
{
    GT_U32      regAddr;    /* register address             */
    GT_U32      hwValue;    /* value to write to register   */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    hwValue = BOOL2BIT_MAC(enable);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
        /* getting register address */
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EGF_qag.distributor.stackRemapEn[portNum/32];
        return prvCpssHwPpSetRegField(devNum, regAddr,(portNum%32), 1, hwValue);
    }

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        /* getting register address */
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.stackRemapPortEnReg;

        if (localPort == CPSS_CPU_PORT_NUM_CNS)
        {
            localPort = 31;
        }

        /* enable/disable stack tc remap per port */
        return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, localPort, 1, hwValue);
    }
    else
    {
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.
            distributor.stackRemapEn[OFFSET_TO_WORD_MAC(portNum)];

        return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr,
            OFFSET_TO_BIT_MAC(portNum), 1, hwValue);
    }

}

/**
* @internal cpssDxChCscdQosPortTcRemapEnableSet function
* @endinternal
*
* @brief   Enable/Disable Traffic Class Remapping on cascading port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - cascading port number or CPU port
* @param[in] enable                   - GT_TRUE:  Traffic Class remapping
*                                      GT_FALSE: disable Traffic Class remapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdQosPortTcRemapEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdQosPortTcRemapEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChCscdQosPortTcRemapEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdQosPortTcRemapEnableGet function
* @endinternal
*
* @brief   Get the status of Traffic Class Remapping on cascading port
*         (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - cascading port number or CPU port
*
* @param[out] enablePtr                - GT_TRUE: Traffic Class remapping enabled
*                                      GT_FALSE: Traffic Class remapping disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdQosPortTcRemapEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_U32      regAddr;    /* register address             */
    GT_U32      hwValue;    /* value to write to register   */
    GT_STATUS   rc;         /* return code                  */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
        /* getting register address */
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EGF_qag.distributor.stackRemapEn[portNum/32];
        rc = prvCpssHwPpGetRegField(devNum, regAddr, (portNum%32), 1, &hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        *enablePtr = BIT2BOOL_MAC(hwValue);
        return rc;
    }

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        /* getting register address */
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.stackRemapPortEnReg;

        if (localPort == CPSS_CPU_PORT_NUM_CNS)
        {
            localPort = 31;
        }

        /* get enable/disable stack tc remap per port */
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, localPort, 1, &hwValue);
    }
    else
    {
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.
            distributor.stackRemapEn[OFFSET_TO_WORD_MAC(portNum)];

        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr,
            OFFSET_TO_BIT_MAC(portNum), 1, &hwValue);
    }

    *enablePtr = BIT2BOOL_MAC(hwValue);

    return rc;
}

/**
* @internal cpssDxChCscdQosPortTcRemapEnableGet function
* @endinternal
*
* @brief   Get the status of Traffic Class Remapping on cascading port
*         (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - cascading port number or CPU port
*
* @param[out] enablePtr                - GT_TRUE: Traffic Class remapping enabled
*                                      GT_FALSE: Traffic Class remapping disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdQosPortTcRemapEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdQosPortTcRemapEnableGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChCscdQosPortTcRemapEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdQosTcRemapTableSet function
* @endinternal
*
* @brief   Remap Traffic Class on cascading port to new Traffic Classes,
*         for each DSA tag type and for source port type (local or cascading).
*         If the source port is enabled for Traffic Class remapping, then traffic
*         will egress with remapped Traffic Class.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tc                       - Traffic Class of the packet on the source port (APPLICABLE RANGES: 0..7)
* @param[in] tcMappingsPtr            - (pointer to )remapped traffic Classes
*                                      for ingress Traffic Class
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdQosTcRemapTableSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  tc,
    IN  CPSS_DXCH_CSCD_QOS_TC_REMAP_STC    *tcMappingsPtr
)
{
    GT_U32      regAddr;    /* register address             */
    GT_U32      data;       /* data to write to register   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);
    CPSS_NULL_PTR_CHECK_MAC(tcMappingsPtr);

    /* check validity of Traffic Classess */
    if ((tcMappingsPtr->forwardLocalTc >= BIT_3) ||
        (tcMappingsPtr->forwardStackTc >= BIT_3) ||
        (tcMappingsPtr->toAnalyzerLocalTc >= BIT_3) ||
        (tcMappingsPtr->toAnalyzerStackTc >= BIT_3) ||
        (tcMappingsPtr->toCpuLocalTc >= BIT_3) ||
        (tcMappingsPtr->toCpuStackTc >= BIT_3) ||
        (tcMappingsPtr->fromCpuLocalTc >= BIT_3) ||
        (tcMappingsPtr->fromCpuStackTc >= BIT_3))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* getting register address */
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.
                                                stackTcPerProfileRemapTbl[tc];
    }
    else
    {
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.
            distributor.stackTcRemapping[tc];
    }

    /* build data to write to register */
    data = (tcMappingsPtr->forwardLocalTc |
           (tcMappingsPtr->toAnalyzerLocalTc << 3) |
           (tcMappingsPtr->toCpuLocalTc << 6) |
           (tcMappingsPtr->fromCpuLocalTc << 9) |
           (tcMappingsPtr->forwardStackTc << 12) |
           (tcMappingsPtr->toAnalyzerStackTc << 15) |
           (tcMappingsPtr->toCpuStackTc << 18) |
           (tcMappingsPtr->fromCpuStackTc << 21));


    /* write data to Stack TC per Profile remap table */
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 24, data);
}

/**
* @internal cpssDxChCscdQosTcRemapTableSet function
* @endinternal
*
* @brief   Remap Traffic Class on cascading port to new Traffic Classes,
*         for each DSA tag type and for source port type (local or cascading).
*         If the source port is enabled for Traffic Class remapping, then traffic
*         will egress with remapped Traffic Class.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tc                       - Traffic Class of the packet on the source port (APPLICABLE RANGES: 0..7)
* @param[in] tcMappingsPtr            - (pointer to )remapped traffic Classes
*                                      for ingress Traffic Class
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdQosTcRemapTableSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  tc,
    IN  CPSS_DXCH_CSCD_QOS_TC_REMAP_STC    *tcMappingsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdQosTcRemapTableSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tc, tcMappingsPtr));

    rc = internal_cpssDxChCscdQosTcRemapTableSet(devNum, tc, tcMappingsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tc, tcMappingsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdQosTcRemapTableGet function
* @endinternal
*
* @brief   Get the remapped Traffic Classes for given Traffic Class.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tc                       - Traffic Class of the packet on the source port (APPLICABLE RANGES: 0..7)
*
* @param[out] tcMappingsPtr            - (pointer to )remapped Traffic Classes
*                                      for ingress Traffic Class
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdQosTcRemapTableGet
(
    IN   GT_U8                                  devNum,
    IN   GT_U32                                 tc,
    OUT  CPSS_DXCH_CSCD_QOS_TC_REMAP_STC   *tcMappingsPtr
)
{
    GT_U32      regAddr;    /* register address             */
    GT_U32      data;       /* data to write to register    */
    GT_STATUS   rc;         /* return code                  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tc);
    CPSS_NULL_PTR_CHECK_MAC(tcMappingsPtr);

    /* getting register address */
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.
                                                stackTcPerProfileRemapTbl[tc];
    }
    else
    {
        regAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.
            distributor.stackTcRemapping[tc];
    }

    /* read data from Stack TC per Profile remap table */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 24, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set Traffic Classes according to register data */
    tcMappingsPtr->forwardLocalTc = (data & 0x7);
    tcMappingsPtr->toAnalyzerLocalTc = ((data >> 3) & 0x7);
    tcMappingsPtr->toCpuLocalTc = ((data >> 6) & 0x7);
    tcMappingsPtr->fromCpuLocalTc = ((data >> 9) & 0x7);
    tcMappingsPtr->forwardStackTc = ((data >> 12) & 0x7);
    tcMappingsPtr->toAnalyzerStackTc = ((data >> 15) & 0x7);
    tcMappingsPtr->toCpuStackTc = ((data >> 18) & 0x7);
    tcMappingsPtr->fromCpuStackTc = ((data >> 21) & 0x7);

    return GT_OK;
}

/**
* @internal cpssDxChCscdQosTcDpRemapTableAccessModeSet function
* @endinternal
*
* @brief   Set the remap table access mode for distributor
*          general config register.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum   - device number
* @param[in] prioMode - mode of the split between source bits to target bits.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdQosTcDpRemapTableAccessModeSet
(
    IN  GT_U8                                              devNum,
    IN  CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT     prioMode
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 regAddr;
    GT_U32 regOffset;
    GT_U32 regValue;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    regAddr = PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.distributorGeneralConfigs;
    regOffset = 18;

    switch (prioMode)
    {
        case CPSS_DXCH_CSCD_QOS_2B_SRC_2B_TRG_E:
            regValue = prioMode;
            break;

        case CPSS_DXCH_CSCD_QOS_1B_SRC_3B_TRG_E:
            regValue = prioMode;
            break;

        case CPSS_DXCH_CSCD_QOS_4B_TRG_E:
            regValue = prioMode;
            break;

        default:
            rc = GT_BAD_PARAM;
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "bad enum value in cpssDxChCscdQosTcDpRemapTableAccessModeSet");
            break;
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, regOffset, 2, regValue);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "fail in cpssDxChCscdQosTcDpRemapTableAccessModeSet");
        }
    return rc;
}

/**
* @internal cpssDxChCscdQosTcDpRemapTableAccessModeSet function
* @endinternal
*
* @brief   Set the remap table access mode for distributor
*          general config register.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] prioMode - mode of the split between source bits to target bits.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdQosTcDpRemapTableAccessModeSet
(
    IN  GT_U8      devNum,
    IN  CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT     prioMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdQosTcDpRemapTableAccessModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, prioMode));

    rc = internal_cpssDxChCscdQosTcDpRemapTableAccessModeSet(devNum, prioMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, prioMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChCscdQosTcDpRemapTableAccessModeGet function
* @endinternal
*
* @brief   Get the remap table access mode for distributor
*          general config register.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum     - device number

* @param[out] prioModePtr   - mode of the split between source bits to target bits.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdQosTcDpRemapTableAccessModeGet
(
    IN  GT_U8                       devNum,
    OUT  CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT     *prioModePtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regOffset;
    GT_U32 value;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(prioModePtr);
    regAddr = PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).distributor.distributorGeneralConfigs;
    regOffset = 18;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, regOffset, 2, &value);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

  switch(value)
  {
    case 0:
        *prioModePtr = CPSS_DXCH_CSCD_QOS_2B_SRC_2B_TRG_E;
        break;
    case 1:
        *prioModePtr = CPSS_DXCH_CSCD_QOS_4B_TRG_E;
        break;
    case 2:
        *prioModePtr = CPSS_DXCH_CSCD_QOS_1B_SRC_3B_TRG_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
  }
    return rc;
}

/**
* @internal cpssDxChCscdQosTcDpRemapTableAccessModeGet function
* @endinternal
*
* @brief   Get the remap table access mode for distributor
*          general config register.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum     - device number

* @param[out] prioModePtr   - mode of the split between source bits to target bits.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdQosTcDpRemapTableAccessModeGet
(
    IN  GT_U8                                               devNum,
    OUT  CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT     *prioModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdQosTcDpRemapTableAccessModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, prioModePtr));

    rc = internal_cpssDxChCscdQosTcDpRemapTableAccessModeGet(devNum, prioModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, prioModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChCscdQosTcRemapTableGet function
* @endinternal
*
* @brief   Get the remapped Traffic Classes for given Traffic Class.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] tc                       - Traffic Class of the packet on the source port (APPLICABLE RANGES: 0..7)
*
* @param[out] tcMappingsPtr            - (pointer to )remapped Traffic Classes
*                                      for ingress Traffic Class
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdQosTcRemapTableGet
(
    IN   GT_U8                                  devNum,
    IN   GT_U32                                 tc,
    OUT  CPSS_DXCH_CSCD_QOS_TC_REMAP_STC   *tcMappingsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdQosTcRemapTableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tc, tcMappingsPtr));

    rc = internal_cpssDxChCscdQosTcRemapTableGet(devNum, tc, tcMappingsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tc, tcMappingsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCscdQosTcDpRemapTableSet function
* @endinternal
*
* @brief   Set remaping of (packet TC, packet ingress Port Type, packet DSA cmd,
*         packet drop precedence and packet multi ot single-destination) on cascading port
*         to new priority queue for enqueuing the packet, new drop precedence
*         and new priority queue for PFC assigned to this packet.
*         APPLICABLE DEVICES:
*         Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*         NOT APPLICABLE DEVICES:
*         xCat3; AC5; Lion2.
*         INPUTS:
*         devNum     - device number
*         tcDpRemappingPtr  - (pointer to )tc,dp remapping index structure.
*         newTc   - new priority queue assigned to the packet. (APPLICABLE RANGES: 0..7).
*         newDp   - new drop precedence assigned to the packet.
*         newPfcTc - new priority queue fot PFC assigned to the packet.
*         (APPLICABLE RANGES: 0..7
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] tcDpRemappingPtr         - (pointer to )tc,dp remapping index structure.
* @param[in] newTc                    - new priority queue assigned to the packet.
*                                       (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2 0..7).
*                                       (APPLICABLE RANGES: Falcon; AC5P; AC5X; Harrier; Ironman 0..15).
* @param[in] newDp                    - new drop precedence assigned to the packet.
* @param[in] newPfcTc                 - new priority queue fot PFC assigned to the packet.
*                                      (APPLICABLE RANGES: 0..7
*                                      APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
* @param[in]  mcastPriority            -defines whether a Multicast packet has guarantee delivery (e.g. IPTv,
*                                                             ARP-BC) or best effort delivery (e.g. flood).This indication has meaning only if the
*                                                             packet is Multicast.(APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman).
* @param[in]  dpForRx                 - defines whether on congestion packets will be dropped from RX context queue or
*                                       queue will back pressure the control pipe.(APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in]  preemptiveTc            -defines whether a new TC will be preemptive or express.
*                                                              Valid only if targetPortTcProfile is  CPSS_DXCH_PORT_PROFILE_NETWORK_PREEMPTIVE_E
*                                                               (APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdQosTcDpRemapTableSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC *tcDpRemappingPtr,
    IN  GT_U32                                   newTc,
    IN  CPSS_DP_LEVEL_ENT                        newDp,
    IN  GT_U32                                   newPfcTc,
    IN  CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT     mcastPriority,
    IN  CPSS_DP_FOR_RX_ENT                       dpForRx,
    IN  GT_BOOL                                  preemptiveTc
)
{
    GT_STATUS rc;
    GT_U32 newDpValue ;
    GT_U32 oldDpValue;
    GT_U32 dsaTagCmdValue;
    GT_U32 index;
    GT_U32 regAddr;
    GT_U32 fieldOffset;
    GT_U32 fieldLength;
    GT_U32 targetPortTcProfile;
    GT_U32 sourcePortTcProfile;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;

    /* check input parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(tcDpRemappingPtr);

    PRV_CPSS_DXCH_COS_DP_TO_HW_CHECK_AND_CONVERT_MAC(
        devNum, newDp, newDpValue);
    PRV_CPSS_DXCH_COS_CHECK_4BIT_TC_MAC(devNum, newTc);
    PRV_CPSS_DXCH_COS_DP_TO_HW_CHECK_AND_CONVERT_MAC(
        devNum, tcDpRemappingPtr->dp, oldDpValue);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcDpRemappingPtr->tc);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        PRV_CPSS_DXCH_COS_CHECK_TC_MAC(newPfcTc);
        PRV_CPSS_DXCH_PORT_TC_PROFILE_CONVERT_MAC(tcDpRemappingPtr->targetPortTcProfile, targetPortTcProfile);
        PRV_CPSS_DXCH_PORT_TC_PROFILE_CONVERT_MAC(tcDpRemappingPtr->isStack, sourcePortTcProfile);

        return prvCpssDxChHwEgfQagTcDpMapperByParamsSet(devNum,
                                                        tcDpRemappingPtr->tc, tcDpRemappingPtr->dp,
                                                        tcDpRemappingPtr->dsaTagCmd,
                                                        tcDpRemappingPtr->packetIsMultiDestination,
                                                        targetPortTcProfile,
                                                        sourcePortTcProfile,
                                                        newDp, newTc, newPfcTc,mcastPriority,dpForRx,preemptiveTc);
    }

    switch (tcDpRemappingPtr->dsaTagCmd)
    {
    case CPSS_DXCH_NET_DSA_CMD_TO_CPU_E:
        dsaTagCmdValue = 0;
        break;
    case CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E:
        dsaTagCmdValue = 1;
        break;
    case CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E:
        dsaTagCmdValue = 2;
        break;
    case CPSS_DXCH_NET_DSA_CMD_FORWARD_E:
        dsaTagCmdValue = 3;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    switch (tcDpRemappingPtr->isStack)
    {
    case CPSS_DXCH_PORT_PROFILE_NETWORK_E:
        sourcePortTcProfile = 0;
        break;
    case CPSS_DXCH_PORT_PROFILE_CSCD_E:
        sourcePortTcProfile = 1;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    /* build 8 bit the key value */
    index = 0;
    index = (tcDpRemappingPtr->tc << 5) | (sourcePortTcProfile << 4) |(dsaTagCmdValue << 2) | oldDpValue;
    /* set new priority queue */
    /* each register incorporates 8 priority queue values */
    regAddr = regsAddrPtr->EGF_qag.distributor.TCRemap[index/8];
    fieldLength = 3;
    fieldOffset = (index%8)*fieldLength;
    rc =  prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, newTc);
    if (rc != GT_OK)
    {
        return rc;
    }
    /*set new dp */
    regAddr = regsAddrPtr->EGF_qag.distributor.DPRemap[index/16];
    fieldLength = 2;
    fieldOffset = (index%16)*fieldLength;
    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, newDpValue);
}

/**
* @internal cpssDxChCscdQosTcDpRemapTableSet function
* @endinternal
*
* @brief   Set remaping of (packet TC, packet ingress Port Type, packet DSA cmd,
*         packet drop precedence and packet multi ot single-destination) on cascading port
*         to new priority queue for enqueuing the packet, new drop precedence
*         and new priority queue for PFC assigned to this packet.
*         APPLICABLE DEVICES:
*         Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*         NOT APPLICABLE DEVICES:
*         xCat3; AC5; Lion2.
*         INPUTS:
*         devNum     - device number
*         tcDpRemappingPtr  - (pointer to )tc,dp remapping index structure.
* @param[in] newTc                    - new priority queue assigned to the packet.
*                                       (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2 0..7).
*                                       (APPLICABLE RANGES: Falcon; AC5P; AC5X; Harrier; Ironman 0..15).
*         newDp   - new drop precedence assigned to the packet.
*         newPfcTc - new priority queue for PFC assigned to the packet.
*         (APPLICABLE RANGES: 0..7
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] tcDpRemappingPtr         - (pointer to )tc,dp remapping index structure.
* @param[in] newTc                    - new priority queue assigned to the packet. (APPLICABLE RANGES: 0..7).
* @param[in] newDp                    - new drop precedence assigned to the packet.
* @param[in] newPfcTc                 - new priority queue for PFC assigned to the packet.
*                                      (APPLICABLE RANGES: 0..7
*                                      APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
* @param[in]  mcastPriority            -defines whether a Multicast packet has guarantee delivery (e.g. IPTv,
*                                                             ARP-BC) or best effort delivery (e.g. flood).This indication has meaning only if the
*                                                             packet is Multicast.(APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman).
* @param[in]  dpForRx                 - defines whether on congestion packets will be dropped from RX context queue or
*                                       queue will back pressure the control pipe.(APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in]  preemptiveTc            -defines whether a new TC will be preemptive or express.
*                                                              Valid only if targetPortTcProfile is  CPSS_DXCH_PORT_PROFILE_NETWORK_PREEMPTIVE_E
*                                                               (APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdQosTcDpRemapTableSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC *tcDpRemappingPtr,
    IN  GT_U32                                   newTc,
    IN  CPSS_DP_LEVEL_ENT                        newDp,
    IN  GT_U32                                   newPfcTc,
    IN  CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT     mcastPriority,
    IN  CPSS_DP_FOR_RX_ENT                       dpForRx,
    IN  GT_BOOL                                  preemptiveTc
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdQosTcDpRemapTableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcDpRemappingPtr, newTc, newDp, mcastPriority, dpForRx, preemptiveTc));

    rc = internal_cpssDxChCscdQosTcDpRemapTableSet(devNum, tcDpRemappingPtr, newTc, newDp, newPfcTc, mcastPriority, dpForRx, preemptiveTc);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcDpRemappingPtr, newTc, newDp, mcastPriority, dpForRx, preemptiveTc));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCscdQosTcDpRemapTableGet function
* @endinternal
*
* @brief   Get the remapped value of priority queue, drop precedence and priority queue for PFC
*         assigned to the packet for given (packet TC, packet ingress Port Type,
*         packet DSA cmd, packet drop precedence and packet multi or single-destination) on cascading port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] tcDpRemappingPtr         - (pointer to )tc,dp remapping index structure.
*
* @param[out] remappedTcPtr            - (pointer to) priority queue assigned to the packet.
* @param[out] remappedDpPtr            - (pointer to) drop precedence assigned to the packet.
* @param[out] remappedPfcTcPtr         - (pointer to) priority queue for PFC assigned to the packet.
*                                      (APPLICABLE DEVICES Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
* @param[out]  remappedMcastPriorityPtr            -(pointer to)defines whether a Multicast packet has guarantee delivery (e.g. IPTv,
*                                                             ARP-BC) or best effort delivery (e.g. flood).This indication has meaning only if the
*                                                             packet is Multicast.(APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman).
* @param[out] remappedDpForRxPtr         -(pointer to) drop precedence to RX context queue.
* @param[in]  preemptiveTcPtr            -(pointer to)defines whether a new TC will be preemptive or express.
*                                                              Valid only if targetPortTcProfile is  CPSS_DXCH_PORT_PROFILE_NETWORK_PREEMPTIVE_E
*                                                               (APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdQosTcDpRemapTableGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC *tcDpRemappingPtr,
    OUT  GT_U32                                  *remappedTcPtr,
    OUT  CPSS_DP_LEVEL_ENT                       *remappedDpPtr,
    OUT  GT_U32                                  *remappedPfcTcPtr,
    OUT  CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT    *remappedMcastPriorityPtr,
    OUT  CPSS_DP_FOR_RX_ENT                      *remappedDpForRxPtr,
    OUT  GT_BOOL                                 *preemptiveTcPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32  oldDpValue;
    GT_U32  dsaTagCmdValue;
    GT_U32  index;
    GT_U32  regAddr;
    GT_U32  fieldOffset;
    GT_U32  fieldLength;
    GT_U32  fieldValue;
    GT_U32  targetPortTcProfile;
    GT_U32  sourcePortTcProfile;

    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    /* check input parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(tcDpRemappingPtr);

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    PRV_CPSS_DXCH_COS_DP_TO_HW_CHECK_AND_CONVERT_MAC(
        devNum, tcDpRemappingPtr->dp, oldDpValue);
    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(tcDpRemappingPtr->tc);
    CPSS_NULL_PTR_CHECK_MAC(remappedTcPtr);
    CPSS_NULL_PTR_CHECK_MAC(remappedDpPtr);
    CPSS_NULL_PTR_CHECK_MAC(remappedDpForRxPtr);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        CPSS_NULL_PTR_CHECK_MAC(remappedPfcTcPtr);
        PRV_CPSS_DXCH_PORT_TC_PROFILE_CONVERT_MAC(tcDpRemappingPtr->targetPortTcProfile, targetPortTcProfile);
        PRV_CPSS_DXCH_PORT_TC_PROFILE_CONVERT_MAC(tcDpRemappingPtr->isStack, sourcePortTcProfile);

        return prvCpssDxChHwEgfQagTcDpMapperByParamsGet(devNum,
                                                        tcDpRemappingPtr->tc, tcDpRemappingPtr->dp,
                                                        tcDpRemappingPtr->dsaTagCmd,
                                                        tcDpRemappingPtr->packetIsMultiDestination,
                                                        targetPortTcProfile,
                                                        sourcePortTcProfile,
                                                        remappedDpPtr, remappedTcPtr, remappedPfcTcPtr,remappedMcastPriorityPtr,
                                                        remappedDpForRxPtr,
                                                        preemptiveTcPtr);
    }

    switch (tcDpRemappingPtr->dsaTagCmd)
    {
    case CPSS_DXCH_NET_DSA_CMD_TO_CPU_E:
        dsaTagCmdValue = 0;
        break;
    case CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E:
        dsaTagCmdValue = 1;
        break;
    case CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E:
        dsaTagCmdValue = 2;
        break;
    case CPSS_DXCH_NET_DSA_CMD_FORWARD_E:
        dsaTagCmdValue = 3;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (tcDpRemappingPtr->isStack)
    {
    case CPSS_DXCH_PORT_PROFILE_NETWORK_E:
        sourcePortTcProfile = 0;
        break;
    case CPSS_DXCH_PORT_PROFILE_CSCD_E:
        sourcePortTcProfile = 1;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* build 8 bit the key value */
    index = 0;
    index = (tcDpRemappingPtr->tc << 5) | (sourcePortTcProfile << 4) |(dsaTagCmdValue << 2) | oldDpValue;
    /* get remapped priority queue */
    /* each register incorporates 8 priority queue values */
    regAddr = regsAddrPtr->EGF_qag.distributor.TCRemap[index/8];
    fieldLength = 3;
    fieldOffset = (index%8)*fieldLength;
    rc =  prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &fieldValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    *remappedTcPtr = fieldValue;
    /*get remapped  dp */
    regAddr = regsAddrPtr->EGF_qag.distributor.DPRemap[index/16];
    fieldLength = 2;
    fieldOffset = (index%16)*fieldLength;
    rc =  prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &fieldValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    *remappedDpPtr = fieldValue;
    return rc;
}

/**
* @internal cpssDxChCscdQosTcDpRemapTableGet function
* @endinternal
*
* @brief   Get the remapped value of priority queue, drop precedence and priority queue for PFC
*         assigned to the packet for given (packet TC, packet ingress Port Type,
*         packet DSA cmd, packet drop precedence and packet multi or single-destination) on cascading port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] tcDpRemappingPtr         - (pointer to )tc,dp remapping index structure.
*
* @param[out] remappedTcPtr            - (pointer to) priority queue assigned to the packet.
* @param[out] remappedDpPtr            - (pointer to) drop precedence assigned to the packet.
* @param[out] remappedPfcTcPtr         - (pointer to) priority queue for PFC assigned to the packet.
*                                      (APPLICABLE DEVICES Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
* @param[out]  remappedMcastPriorityPtr            -(pointer to)defines whether a Multicast packet has guarantee delivery (e.g. IPTv,
*                                                             ARP-BC) or best effort delivery (e.g. flood).This indication has meaning only if the
*                                                             packet is Multicast.(APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman).
* @param[out]  remappedDpForRxPtr         - (pointer to) drop precedence for RX context queue.
* @param[out]  preemptiveTcPtr            -(pointer to)defines whether a new TC will be preemptive or express.
*                                                              Valid only if targetPortTcProfile is  CPSS_DXCH_PORT_PROFILE_NETWORK_PREEMPTIVE_E
*                                                               (APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdQosTcDpRemapTableGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC *tcDpRemappingPtr,
    OUT  GT_U32                                  *remappedTcPtr,
    OUT  CPSS_DP_LEVEL_ENT                       *remappedDpPtr,
    OUT  GT_U32                                  *remappedPfcTcPtr,
    OUT  CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT    *remappedMcastPriorityPtr,
    OUT  CPSS_DP_FOR_RX_ENT                      *remappedDpForRxPtr,
    OUT  GT_BOOL                                 *preemptiveTcPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdQosTcDpRemapTableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcDpRemappingPtr, remappedTcPtr, remappedDpPtr,
        remappedMcastPriorityPtr, remappedDpForRxPtr, preemptiveTcPtr));

    rc = internal_cpssDxChCscdQosTcDpRemapTableGet(devNum, tcDpRemappingPtr, remappedTcPtr, remappedDpPtr,
        remappedPfcTcPtr,remappedMcastPriorityPtr, remappedDpForRxPtr, preemptiveTcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcDpRemappingPtr, remappedTcPtr, remappedDpPtr,
        remappedMcastPriorityPtr, remappedDpForRxPtr, preemptiveTcPtr));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCscdPortBridgeBypassEnableSet function
* @endinternal
*
* @brief   The function enables/disables bypass of the bridge engine per port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -  PP's device number.
* @param[in] portNum                  -  port number or CPU port
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   -  GT_TRUE  - Enable bypass of the bridge engine.
*                                      GT_FALSE - Disable bypass of the bridge engine.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If the DSA tag is not extended Forward, the bridging decision
*       is performed regardless of the setting.
*       When bypass enabled the Bridge engine still learn MAC source addresses,
*       but will not modify the packet command, attributes (or forwarding
*       decision).
*
*/
static GT_STATUS internal_cpssDxChCscdPortBridgeBypassEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    IN  GT_BOOL     enable
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      data;           /* data to write to HW */
    GT_U32      fieldOffset;    /* field offset */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* Convert boolean value to the HW format */
    data = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to pre-tti-lookup-ingress-eport table */
         rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_EXTENDED_DSA_BYPASS_BRIDGE_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            data);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        fieldOffset = 8;

        /* Update port mode in the VLAN and QoS Configuration Entry */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                               CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                               portNum,
                                               0,       /* start at word 0 */
                                               fieldOffset, /* field offset */
                                               1,       /* 1 bit */
                                               data);
    }
    return rc;

}

/**
* @internal cpssDxChCscdPortBridgeBypassEnableSet function
* @endinternal
*
* @brief   The function enables/disables bypass of the bridge engine per port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -  PP's device number.
* @param[in] portNum                  -  port number or CPU port
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   -  GT_TRUE  - Enable bypass of the bridge engine.
*                                      GT_FALSE - Disable bypass of the bridge engine.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If the DSA tag is not extended Forward, the bridging decision
*       is performed regardless of the setting.
*       When bypass enabled the Bridge engine still learn MAC source addresses,
*       but will not modify the packet command, attributes (or forwarding
*       decision).
*
*/
GT_STATUS cpssDxChCscdPortBridgeBypassEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    IN  GT_BOOL     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortBridgeBypassEnableSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChCscdPortBridgeBypassEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdPortBridgeBypassEnableGet function
* @endinternal
*
* @brief   The function gets bypass of the bridge engine per port
*         configuration status.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] portNum                  - port number or CPU port
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                -  pointer to bypass of the bridge engine per port
*                                      configuration status.
*                                      GT_TRUE  - Enable bypass of the bridge engine.
*                                      GT_FALSE - Disable bypass of the bridge engine.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdPortBridgeBypassEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_U32      data;           /* data to read from HW */
    GT_STATUS   rc;             /* return code          */
    GT_U32      fieldOffset;    /* field offset         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read from pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_EXTENDED_DSA_BYPASS_BRIDGE_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &data);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        fieldOffset = 8;

        /* Read port mode in the VLAN and QoS Configuration Entry */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                               CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                               portNum,
                                               0,           /* start at word 0 */
                                               fieldOffset, /* field offset */
                                               1,           /* 1 bit */
                                               &data);
    }

    /* Convert HW format to the boolean value */
    *enablePtr = BIT2BOOL_MAC(data);

    return rc;

}

/**
* @internal cpssDxChCscdPortBridgeBypassEnableGet function
* @endinternal
*
* @brief   The function gets bypass of the bridge engine per port
*         configuration status.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP's device number.
* @param[in] portNum                  - port number or CPU port
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                -  pointer to bypass of the bridge engine per port
*                                      configuration status.
*                                      GT_TRUE  - Enable bypass of the bridge engine.
*                                      GT_FALSE - Disable bypass of the bridge engine.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdPortBridgeBypassEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortBridgeBypassEnableGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChCscdPortBridgeBypassEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdOrigSrcPortFilterEnableSet function
* @endinternal
*
* @brief   Enable/Disable filtering the multi-destination packet that was received
*         by the local device, sent to another device, and sent back to this
*         device, from being sent back to the network port at which it was
*         initially received.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - filter and drop the packet
*                                      - GT_FALSE - don't filter the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdOrigSrcPortFilterEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_U32  regAddr;      /* register address */
    GT_U32  data;         /* reg subfield data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    data = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr =  PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).egrFilterConfigs.egrFiltersEnable;

        /* <OrigSrcPortFilterEn > */
        return prvCpssHwPpSetRegField(devNum, regAddr, 2, 1, data);
    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueConfigExt;
        return prvCpssHwPpSetRegField(devNum, regAddr, 5, 1, data);
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.filterConfig.globalEnables;
        return prvCpssHwPpSetRegField(devNum, regAddr, 3, 1, data);
    }
}

/**
* @internal cpssDxChCscdOrigSrcPortFilterEnableSet function
* @endinternal
*
* @brief   Enable/Disable filtering the multi-destination packet that was received
*         by the local device, sent to another device, and sent back to this
*         device, from being sent back to the network port at which it was
*         initially received.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - filter and drop the packet
*                                      - GT_FALSE - don't filter the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdOrigSrcPortFilterEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdOrigSrcPortFilterEnableSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChCscdOrigSrcPortFilterEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCscdOrigSrcPortFilterEnableGet function
* @endinternal
*
* @brief   Get the status of filtering the multi-destination packet that was
*         received by the local device, sent to another device, and sent back to
*         this device, from being sent back to the network port at which it was
*         initially received.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE - filter and drop the packet
*                                      - GT_FALSE - don't filter the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdOrigSrcPortFilterEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32      regAddr;    /* register address     */
    GT_U32      data;       /* reg subfield data    */
    GT_STATUS   rc;         /* return code          */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr =  PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).egrFilterConfigs.egrFiltersEnable;

        /* <OrigSrcPortFilterEn > */
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 2, 1, &data);
    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueConfigExt;

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 5, 1, &data);
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.filterConfig.globalEnables;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 3, 1, &data);
    }

    *enablePtr = BIT2BOOL_MAC(data);

    return rc;
}

/**
* @internal cpssDxChCscdOrigSrcPortFilterEnableGet function
* @endinternal
*
* @brief   Get the status of filtering the multi-destination packet that was
*         received by the local device, sent to another device, and sent back to
*         this device, from being sent back to the network port at which it was
*         initially received.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE - filter and drop the packet
*                                      - GT_FALSE - don't filter the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdOrigSrcPortFilterEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdOrigSrcPortFilterEnableGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChCscdOrigSrcPortFilterEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdDevMapLookupModeSet function
* @endinternal
*
* @brief   Set lookup mode for accessing the Device Map table.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - device Map lookup mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdDevMapLookupModeSet
(
    IN GT_U8                               devNum,
    IN CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT   mode
)
{
    GT_U32      regAddr;    /* register address     */
    GT_U32      data;       /* reg subfield data    */
    GT_U32      profile;    /* the profile to use */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);

    /* SIP4 devices provide only 9 different lookup mode configurations for
     * accessing device map table unlike SIP 5 devices and above which have total
     * of 13 different lookup configuration modes.
     */
    if(!PRV_CPSS_SIP_5_CHECK_MAC(devNum) && mode >= CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_PORT_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(mode)
    {
        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E:
            data = 0;
            break;

        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E:
            data = 1;
            break;

        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E:
            data = 3;
            break;

        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E:
            data = 2;
            break;

        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E:
            data = 8;
            break;

        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E:
            data = 7;
            break;

        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E:
            data = 4;
            break;

        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E:
            data = 5;
            break;

        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E:
            data = 6;
            break;

        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_PORT_E:
            data = 9;
            break;

        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_PHYSICAL_SRC_PORT_E:
            data = 10;
            break;

        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_LSB_HASH_E:
            data = 11;
            break;

        case CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_MSB_HASH_E:
            data = 12;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK /* need parameter to set on different 'profiles' */
        profile = 0;

        regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).deviceMapConfigs.devMapTableAddrConstructProfile[profile];
    }
    else
    {
        if (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(devNum))
        {
            /* set Dist Dev Map Table Addr Construct */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                txqVer1.dist.deviceMapTable.addrConstruct;

            rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 4, data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.egr.devMapTableConfig.addrConstruct;
    }

    /* Set lookup mode for accessing the Device Map table. */
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 4, data);
}

/**
* @internal cpssDxChCscdDevMapLookupModeSet function
* @endinternal
*
* @brief   Set lookup mode for accessing the Device Map table.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - device Map lookup mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdDevMapLookupModeSet
(
    IN GT_U8                               devNum,
    IN CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT   mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdDevMapLookupModeSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChCscdDevMapLookupModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdDevMapLookupModeGet function
* @endinternal
*
* @brief   Get lookup mode for accessing the Device Map table.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - pointer to device Map lookup mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdDevMapLookupModeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT   *modePtr
)
{
    GT_U32      regAddr;    /* register address     */
    GT_U32      data;       /* reg subfield data    */
    GT_U32      profile;    /* the profile to use */
    GT_STATUS   rc;         /* return code          */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        CPSS_TBD_BOOKMARK /* need parameter to set on different 'profiles' */
        profile = 0;

        regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).deviceMapConfigs.devMapTableAddrConstructProfile[profile];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            txqVer1.egr.devMapTableConfig.addrConstruct;
    }

    /* Get lookup mode for accessing the Device Map table. */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 4, &data);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(data)
    {
        case 0:
            *modePtr = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E;
            break;

        case 1:
            *modePtr = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E;
            break;

        case 2:
            *modePtr = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E;
            break;

        case 3:
            *modePtr = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E;
            break;

        case 4:
            *modePtr = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E;
            break;

        case 5:
            *modePtr = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E;
            break;

        case 6:
            *modePtr = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E;
            break;

        case 7:
            *modePtr = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E;
            break;

        case 8:
            *modePtr = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E;
            break;

        case 9:
            *modePtr = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_PORT_E;
            break;

        case 10:
            *modePtr = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_PHYSICAL_SRC_PORT_E;
            break;

        case 11:
            *modePtr = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_LSB_HASH_E;
            break;

        case 12:
            *modePtr = CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_MSB_HASH_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChCscdDevMapLookupModeGet function
* @endinternal
*
* @brief   Get lookup mode for accessing the Device Map table.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - pointer to device Map lookup mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - wrong hardware value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdDevMapLookupModeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT   *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdDevMapLookupModeGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChCscdDevMapLookupModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChCscdLion2EgrSet function
* @endinternal
*
* @brief   Configure EGR units according to source and target port enable bitmaps
*         of the Distributor unit. Configure non zero values only for EGR units
*         where both target and source bits are enabled.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number or portDirection
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChCscdLion2EgrSet
(
    IN GT_U8                devNum
)
{
    GT_U32  regAddr;            /* register address     */
    GT_U32  portGroupId;        /* the port group Id - support multi-port-groups device */
    GT_STATUS   rc;             /* return status                */
    GT_U32 srcPortsBmp[4];      /* source ports enable bitmap */
    GT_U32 dataToWrite[4];      /* data buffer to write */
    GT_U32 trgPortsBmp[4];      /* target ports enable bitmap */
    GT_U32 egrUnitsBmp;         /* bitmap of EGR units */
    GT_U32 srcPortGroupBmp;     /* bitmap of source port groups with enabled bits */
    GT_U32 mask;                /* bits mask */
    GT_U32 egrUnit;             /* EGR unit number */
    GT_U32 egrUnitOffset;       /* address offset for EGR unit */
    GT_U32 *trgDataPtr;         /* pointer to target bitmap data */
    GT_U32 srcDataIdx;          /* index in source bitmap array to get data */
    GT_U32 dataToWriteIdx;      /* index in source bitmap buffer to write data */

    /* read all source ports enable bitmap from DIST unit */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)-> txqVer1.dist.deviceMapTable.
                            localSrcPortMapOwnDevEn[0];
    rc = prvCpssHwPpReadRam(devNum,regAddr,4,srcPortsBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* read all target ports enable bitmap from DIST unit */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)-> txqVer1.dist.deviceMapTable.
                            localTrgPortMapOwnDevEn[0];
    rc = prvCpssHwPpReadRam(devNum,regAddr,4,trgPortsBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* calculate EGR units bitmap:
     Tagret ports define hemisphere and source ports define EGR unit
     within hemisphere:
     EGR0 - EGR7 (hemisphere #0) used for target ports 0..63
     EGR8 - EGR15 (hemisphere #1) used for target ports 64..127.
     Source port's port group define EGR unit withing hemisphere as next:
     - hemisphere #0 - EGR unit = source port group;
     - hemisphere #1 - EGR unit = source port group + 8 */
    egrUnitsBmp = 0;
    if (trgPortsBmp[0] || trgPortsBmp[1])
    {
        /* ERGs of hemisphere #0 have non zero bits */
        egrUnitsBmp |= 0xFF;
    }

    if (trgPortsBmp[2] || trgPortsBmp[3])
    {
        /* ERGs of hemisphere #1 have non zero bits */
        egrUnitsBmp |= 0xFF00;
    }

    srcPortGroupBmp = 0;
    /* calculate source groups bitmap */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
    {
        /* each word has bitmap of two port groups */
        mask = (portGroupId & 1) ? 0xFFFF0000 : 0xFFFF;
        if ((srcPortsBmp[portGroupId / 2] & mask))
        {
            /* set bits for both hemisphere */
            srcPortGroupBmp |= (1 << portGroupId) | (1 << (portGroupId + 8));
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)

    /* apply source port group mask to EGR units bitmap */
    egrUnitsBmp &= srcPortGroupBmp;

    for (egrUnit = 0; egrUnit < 16; egrUnit++)
    {
        portGroupId = egrUnit / 2;
        /* skip not active port groups */
        if(0 == (PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp & (1<<(portGroupId))))
        {
            continue;
        }

        /* calculate EGR unit offset */
        egrUnitOffset = (egrUnit & 1) ? 0x00800000 : 0;

        if (!(egrUnitsBmp & (1 << egrUnit)))
        {
            /* reset enable bits in all skipped EGR units */
            cpssOsMemSet(dataToWrite, 0, sizeof(dataToWrite));

            /* configure target ports enablers */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.devMapTableConfig.
                                   localTrgPortMapOwnDevEn[0] + egrUnitOffset;
            PRV_CPSS_MUST_DIRECT_CALL_TO_DRIVER_INDICATION_CNS
            rc = prvCpssDrvHwPpPortGroupWriteRam(devNum,portGroupId, regAddr, 2, dataToWrite);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* configure source ports enablers */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.devMapTableConfig.
                                   localSrcPortMapOwnDevEn[0] + egrUnitOffset;
            PRV_CPSS_MUST_DIRECT_CALL_TO_DRIVER_INDICATION_CNS
            rc = prvCpssDrvHwPpPortGroupWriteRam(devNum,portGroupId, regAddr, 4, dataToWrite);
            if(rc != GT_OK)
            {
                return rc;
            }

            continue;
        }

        /* configure target ports enablers */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.devMapTableConfig.
                               localTrgPortMapOwnDevEn[0] + egrUnitOffset;

        /* hemisphere#0 holds bits for ports 0..63
           hemisphere#1 holds bits for ports 64..127
         */
        trgDataPtr = (egrUnit < 8) ? trgPortsBmp : (trgPortsBmp + 2);
        PRV_CPSS_MUST_DIRECT_CALL_TO_DRIVER_INDICATION_CNS
        rc = prvCpssDrvHwPpPortGroupWriteRam(devNum,portGroupId, regAddr, 2, trgDataPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* configure source ports enablers */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.devMapTableConfig.
                               localSrcPortMapOwnDevEn[0] + egrUnitOffset;

        /* set bits only own port group */
        cpssOsMemSet(dataToWrite, 0, sizeof(dataToWrite));
        mask = (egrUnit & 1) ? 0xFFFF0000 : 0xFFFF;
        srcDataIdx = dataToWriteIdx = ((egrUnit % 8) / 2);

        if ((egrUnit >= 4) && (egrUnit <= 7))
        {
            /* EGR 4-7 use bits 0..63 for source ports 65..127 */
            dataToWriteIdx = srcDataIdx - 2;
        }
        else if ((egrUnit >= 8) && (egrUnit <= 11))
        {
            /* EGR 8-11 use bits 65..127 for source ports 0..63 */
            dataToWriteIdx = srcDataIdx + 2;
        }

        dataToWrite[dataToWriteIdx] = srcPortsBmp[srcDataIdx] & mask;
        PRV_CPSS_MUST_DIRECT_CALL_TO_DRIVER_INDICATION_CNS
        rc = prvCpssDrvHwPpPortGroupWriteRam(devNum,portGroupId, regAddr, 4, dataToWrite);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChCscdPortLocalDevMapLookupEnableSet function
* @endinternal
*
* @brief   Enable / Disable the local target port for device map lookup
*         for local device.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port
* @param[in] portDirection            - port's direction:
*                                      CPSS_DIRECTION_INGRESS_E - source port
*                                      CPSS_DIRECTION_EGRESS_E  -  target port
*                                      CPSS_DIRECTION_BOTH_E    - both source and target ports
* @param[in] enable                   - GT_TRUE  - the port is enabled for device map lookup
*                                      for local device.
*                                      - GT_FALSE - No access to Device map table for
*                                      local devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number or portDirection
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To enable access to the Device Map Table for the local target devices
*       - Enable the local source port for device map lookup
*       - Enable the local target port for device map lookup
*
*/
static GT_STATUS internal_cpssDxChCscdPortLocalDevMapLookupEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DIRECTION_ENT   portDirection,
    IN GT_BOOL              enable
)
{
    GT_U32  regAddr;            /* register address     */
    GT_U32  data;               /* reg subfield data    */
    GT_U32  portGroupId;        /* the port group Id - support multi-port-groups device */
    GT_U32  fieldOffset;        /* register field offset */
    GT_STATUS   rc;             /* return status                */
    GT_BOOL  setIngress = GT_FALSE; /* configure source port enabler */
    GT_BOOL  setEgress = GT_FALSE;  /* configure target port enabler */
    GT_BOOL  useWa;                 /* use WA for Lion2 TxQ EGR registers */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);

    data = (enable == GT_TRUE) ? 1 : 0;

    switch(portDirection)
    {
        case CPSS_DIRECTION_BOTH_E:
            setIngress = GT_TRUE;
            setEgress  = GT_TRUE;
            break;
        case CPSS_DIRECTION_INGRESS_E:
            setIngress = GT_TRUE;
            break;
        case CPSS_DIRECTION_EGRESS_E:
            setEgress  = GT_TRUE;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(setIngress)
        {
            rc = prvCpssDxChHwEgfEftFieldSet(devNum,portNum,
                PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_DEV_MAP_TABLE_LOCAL_SRC_PORT_MAP_OWN_DEV_ENABLE_E,
                data);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        if(setEgress)
        {
            rc = prvCpssDxChHwEgfEftFieldSet(devNum,portNum,
                PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_DEV_MAP_TABLE_LOCAL_TRG_PORT_MAP_OWN_DEV_ENABLE_E,
                data);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        return GT_OK;
    }

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

    if((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
        PRV_CPSS_DXCH_LION2_LOCAL_SRC_PORT_DEV_MAP_ENABLE_WA_E)) &&
       PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(devNum))
    {
        /* use WA only for Lion2 devices with two hemisphere */
        useWa = GT_TRUE;
    }
    else
    {
        useWa = GT_FALSE;
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    fieldOffset = OFFSET_TO_BIT_MAC(portNum);


    /* Enable / Disable the source port for device map lookup for local device. */
    if (setIngress == GT_TRUE)
    {
        if (useWa == GT_FALSE)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.devMapTableConfig.
                                   localSrcPortMapOwnDevEn[OFFSET_TO_WORD_MAC(portNum)];

            /* Lion2: must be global and not port group aware */
            /*        need to be set to all EGR units */
            rc =  prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 1, data);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        if (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(devNum))
        {
            /* configure Dist Unit also */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)-> txqVer1.dist.deviceMapTable.
                                localSrcPortMapOwnDevEn[OFFSET_TO_WORD_MAC(portNum)];

            rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 1, data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* Enable / Disable the local target for device map lookup for local device. */
    if (setEgress == GT_TRUE)
    {
        if (useWa == GT_FALSE)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.devMapTableConfig.
                                   localTrgPortMapOwnDevEn[OFFSET_TO_WORD_MAC(portNum)];

            /* Lion2: must be port group aware */
            /*        need to be set to only to all egr units in relevant TXQ */
            rc =  prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                       fieldOffset, 1, data);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        if (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(devNum))
        {
            /* configure Dist Unit also */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)-> txqVer1.dist.deviceMapTable.
                                localTrgPortMapOwnDevEn[OFFSET_TO_WORD_MAC(portNum)];

            rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 1, data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if (useWa == GT_TRUE)
    {
        /* configure EGR units */
        rc = prvCpssDxChCscdLion2EgrSet(devNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal cpssDxChCscdPortLocalDevMapLookupEnableSet function
* @endinternal
*
* @brief   Enable / Disable the local target port for device map lookup
*         for local device.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port
* @param[in] portDirection            - port's direction:
*                                      CPSS_DIRECTION_INGRESS_E - source port
*                                      CPSS_DIRECTION_EGRESS_E  -  target port
*                                      CPSS_DIRECTION_BOTH_E    - both source and target ports
* @param[in] enable                   - GT_TRUE  - the port is enabled for device map lookup
*                                      for local device.
*                                      - GT_FALSE - No access to Device map table for
*                                      local devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number or portDirection
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To enable access to the Device Map Table for the local target devices
*       - Enable the local source port for device map lookup
*       - Enable the local target port for device map lookup
*
*/
GT_STATUS cpssDxChCscdPortLocalDevMapLookupEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DIRECTION_ENT   portDirection,
    IN GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortLocalDevMapLookupEnableSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portDirection, enable));

    rc = internal_cpssDxChCscdPortLocalDevMapLookupEnableSet(devNum, portNum, portDirection, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portDirection, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdPortLocalDevMapLookupEnableGet function
* @endinternal
*
* @brief   Get status of enabling / disabling the local target port
*         for device map lookup for local device.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port
* @param[in] portDirection            - port's direction:
*                                      CPSS_DIRECTION_INGRESS_E - source port
*                                      CPSS_DIRECTION_EGRESS_E  -  target port
*                                      CPSS_DIRECTION_BOTH_E    - both source and target ports
*
* @param[out] enablePtr                - pointer to status of enabling / disabling the local
*                                      target port for device map lookup for local device.
*                                      - GT_TRUE  - target port is enabled for device map lookup
*                                      for local device.
*                                      - GT_FALSE - No access to Device map table for
*                                      local devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number or portDirection
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdPortLocalDevMapLookupEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DIRECTION_ENT      portDirection,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_U32  regAddr;       /* register address     */
    GT_U32  data;          /* reg subfield data    */
    GT_U32  portGroupId;   /* the port group Id - support multi-port-groups device */
    GT_U32  fieldOffset;   /* register field offset */
    GT_STATUS   rc;        /* return status                */
    GT_BOOL  useWa;        /* use WA for Lion2 TxQ EGR registers */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        switch(portDirection)
        {
            case CPSS_DIRECTION_INGRESS_E:
                rc = prvCpssDxChHwEgfEftFieldGet(devNum,portNum,
                    PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_DEV_MAP_TABLE_LOCAL_SRC_PORT_MAP_OWN_DEV_ENABLE_E,
                    &data);
                break;
            case CPSS_DIRECTION_EGRESS_E:
                rc = prvCpssDxChHwEgfEftFieldGet(devNum,portNum,
                    PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_DEV_MAP_TABLE_LOCAL_TRG_PORT_MAP_OWN_DEV_ENABLE_E,
                    &data);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        *enablePtr = BIT2BOOL_MAC(data);

        return rc;
    }

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

    if((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
        PRV_CPSS_DXCH_LION2_LOCAL_SRC_PORT_DEV_MAP_ENABLE_WA_E)) &&
       PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(devNum))
    {
        /* use WA only for Lion2 devices with two hemisphere */
        useWa = GT_TRUE;
    }
    else
    {
        useWa = GT_FALSE;
    }

    switch(portDirection)
    {
        case CPSS_DIRECTION_INGRESS_E:
            if (useWa == GT_TRUE)
            {
                /* use DIST unit */
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)-> txqVer1.dist.deviceMapTable.
                                localSrcPortMapOwnDevEn[OFFSET_TO_WORD_MAC(portNum)];
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.devMapTableConfig.
                                localSrcPortMapOwnDevEn[OFFSET_TO_WORD_MAC(portNum)];
            }
            break;
        case CPSS_DIRECTION_EGRESS_E:
            if (useWa == GT_TRUE)
            {
                /* use DIST unit */
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)-> txqVer1.dist.deviceMapTable.
                                localTrgPortMapOwnDevEn[OFFSET_TO_WORD_MAC(portNum)];
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.egr.devMapTableConfig.
                               localTrgPortMapOwnDevEn[OFFSET_TO_WORD_MAC(portNum)];
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    fieldOffset = OFFSET_TO_BIT_MAC(portNum);

    /* Get status of enabling / disabling the local target port
       for device map lookup for local device. */
    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                             fieldOffset, 1, &data);

    *enablePtr = BIT2BOOL_MAC(data);

    return GT_OK;
}

/**
* @internal cpssDxChCscdPortLocalDevMapLookupEnableGet function
* @endinternal
*
* @brief   Get status of enabling / disabling the local target port
*         for device map lookup for local device.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port
* @param[in] portDirection            - port's direction:
*                                      CPSS_DIRECTION_INGRESS_E - source port
*                                      CPSS_DIRECTION_EGRESS_E  -  target port
*                                      CPSS_DIRECTION_BOTH_E    - both source and target ports
*
* @param[out] enablePtr                - pointer to status of enabling / disabling the local
*                                      target port for device map lookup for local device.
*                                      - GT_TRUE  - target port is enabled for device map lookup
*                                      for local device.
*                                      - GT_FALSE - No access to Device map table for
*                                      local devices.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or port number or portDirection
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdPortLocalDevMapLookupEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DIRECTION_ENT      portDirection,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortLocalDevMapLookupEnableGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portDirection, enablePtr));

    rc = internal_cpssDxChCscdPortLocalDevMapLookupEnableGet(devNum, portNum, portDirection, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portDirection, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable initial local source port assignment from DSA tag, used
*         for centralized chassis.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - Centralized Chassis Mode
*                                      GT_FALSE: Ingress port is not connected to a line-card
*                                      device in a centralized chassis system
*                                      GT_TRUE:  Ingress port is connected to a line-card
*                                      device in a centralized chassis system
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisModeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /* write to TTI-Physical-Port-Attribute table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                                             portNum,/*global port*/
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                             SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CENTRALIZED_CHASSIS_PORT_ENABLE_E, /* field name */
                                             PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                             BOOL2BIT_MAC(enable));
    if(rc != GT_OK)
        return rc;

    /* write to HA-Physical-Port-Attribute table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                          CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                             portNum,/*global port*/
                                          PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                          SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_SET_SOURCE_TO_LOCAL_E, /* field name */
                                          PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                             BOOL2BIT_MAC(enable));

    return rc;

}

/**
* @internal cpssDxChCscdCentralizedChassisModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable initial local source port assignment from DSA tag, used
*         for centralized chassis.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - Centralized Chassis Mode
*                                      GT_FALSE: Ingress port is not connected to a line-card
*                                      device in a centralized chassis system
*                                      GT_TRUE:  Ingress port is connected to a line-card
*                                      device in a centralized chassis system
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisModeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisModeEnableSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChCscdCentralizedChassisModeEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisModeEnableGet function
* @endinternal
*
* @brief   Get initial local source port assignment from DSA tag, used
*         for centralized chassis.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) Centralized Chassis Mode
*                                      GT_FALSE: Ingress port is not connected to a line-card
*                                      device in a centralized chassis system
*                                      GT_TRUE:  Ingress port is connected to a line-card
*                                      device in a centralized chassis system
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisModeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS   rc;             /* function return code              */
    GT_U32      value, value1;  /* value to write                    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /* read from TTI-Physical-Port-Attribute table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                     CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                                     portNum,/*global port*/
                                     PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                     SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CENTRALIZED_CHASSIS_PORT_ENABLE_E, /* field name */
                                     PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                     &value);
    if(rc!=GT_OK)
        return rc;

    /* read from HA-Physical-Port-Attribute table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                      CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                     portNum,/*global port*/
                                      PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                      SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_SET_SOURCE_TO_LOCAL_E, /* field name */
                                      PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                     &value1);

    if(rc!=GT_OK)
        return rc;

    if(value != value1)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChCscdCentralizedChassisModeEnableGet function
* @endinternal
*
* @brief   Get initial local source port assignment from DSA tag, used
*         for centralized chassis.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) Centralized Chassis Mode
*                                      GT_FALSE: Ingress port is not connected to a line-card
*                                      device in a centralized chassis system
*                                      GT_TRUE:  Ingress port is connected to a line-card
*                                      device in a centralized chassis system
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisModeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisModeEnableGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChCscdCentralizedChassisModeEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisReservedDevNumSet function
* @endinternal
*
* @brief   Configures a value representing the local device in centralized chassis
*         and used as the source device in the outgoing FORWARD DSA tag.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] reservedHwDevNum         - reserved hw dev num
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisReservedDevNumSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_HW_DEV_NUM                       reservedHwDevNum
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /* write to HA-Physical-Port-Attribute table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                      CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                      portNum, /*global port*/
                                      PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                      SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_DEVICE_NUMBER_E, /* field name */
                                      PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                             reservedHwDevNum);

    return rc;

}

/**
* @internal cpssDxChCscdCentralizedChassisReservedDevNumSet function
* @endinternal
*
* @brief   Configures a value representing the local device in centralized chassis
*         and used as the source device in the outgoing FORWARD DSA tag.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] reservedHwDevNum         - reserved hw dev num
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisReservedDevNumSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_HW_DEV_NUM                       reservedHwDevNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisReservedDevNumSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, reservedHwDevNum));

    rc = internal_cpssDxChCscdCentralizedChassisReservedDevNumSet(devNum, portNum, reservedHwDevNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, reservedHwDevNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisReservedDevNumGet function
* @endinternal
*
* @brief   Gets a value representing the local device in centralized chassis
*         and used as the source device in the outgoing FORWARD DSA tag.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] reservedHwDevNumPtr      - (pointer to) reserved hw dev num
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisReservedDevNumGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_HW_DEV_NUM                       *reservedHwDevNumPtr
)
{
    GT_STATUS   rc;             /* function return code              */
    GT_U32      value;          /* value to write                    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(reservedHwDevNumPtr);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /* read from HA-Physical-Port-Attribute table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                      CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                     portNum,/*global port*/
                                      PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                      SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_DEVICE_NUMBER_E, /* field name */
                                      PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                     &value);

    *reservedHwDevNumPtr = value;

    return rc;
}

/**
* @internal cpssDxChCscdCentralizedChassisReservedDevNumGet function
* @endinternal
*
* @brief   Gets a value representing the local device in centralized chassis
*         and used as the source device in the outgoing FORWARD DSA tag.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] reservedHwDevNumPtr      - (pointer to) reserved hw dev num
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisReservedDevNumGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_HW_DEV_NUM                       *reservedHwDevNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisReservedDevNumGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, reservedHwDevNumPtr));

    rc = internal_cpssDxChCscdCentralizedChassisReservedDevNumGet(devNum, portNum, reservedHwDevNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, reservedHwDevNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisSrcIdSet function
* @endinternal
*
* @brief   Configured what is the source ID used by the line card directly
*         attached to the Centralized chassis port
*         Relevant only when <CC Mode Enable> = Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] srcId                    - Src Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisSrcIdSet
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  GT_U32                       srcId
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /* write to HA-Physical-Port-Attribute table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                      CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                      portNum, /*global port*/
                                      PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                      SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_SOURCE_ID_E, /* field name */
                                      PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                             srcId);

    return rc;

}

/**
* @internal cpssDxChCscdCentralizedChassisSrcIdSet function
* @endinternal
*
* @brief   Configured what is the source ID used by the line card directly
*         attached to the Centralized chassis port
*         Relevant only when <CC Mode Enable> = Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] srcId                    - Src Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisSrcIdSet
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  GT_U32                       srcId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisSrcIdSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, srcId));

    rc = internal_cpssDxChCscdCentralizedChassisSrcIdSet(devNum, portNum, srcId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, srcId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisSrcIdGet function
* @endinternal
*
* @brief   Get CC Src ID
*         Relevant only when <CC Mode Enable> = Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] srcIdPtr                 - (pointer to) Src Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisSrcIdGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_U32                              *srcIdPtr
)
{
    GT_STATUS   rc;             /* function return code              */
    GT_U32      value;          /* value to write                    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(srcIdPtr);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /* read from HA-Physical-Port-Attribute table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                      CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                     portNum, /*global port*/
                                      PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                      SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_SOURCE_ID_E, /* field name */
                                      PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                     &value);

    *srcIdPtr = value;

    return rc;
}

/**
* @internal cpssDxChCscdCentralizedChassisSrcIdGet function
* @endinternal
*
* @brief   Get CC Src ID
*         Relevant only when <CC Mode Enable> = Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] srcIdPtr                 - (pointer to) Src Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisSrcIdGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_U32                              *srcIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisSrcIdGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, srcIdPtr));

    rc = internal_cpssDxChCscdCentralizedChassisSrcIdGet(devNum, portNum, srcIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, srcIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdDbRemoteHwDevNumModeSet function
* @endinternal
*
* @brief   Set single/dual HW device number mode to remote HW device number.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hwDevNum                 - HW device number (APPLICABLE RANGES: 0..(4K-1)).
* @param[in] hwDevMode                - single/dual HW device number mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - hwDevNum is odd number and hwDevMode is
*                                       CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E
* @retval GT_OUT_OF_RANGE          - when hwDevNum is out of range
*
* @note 1. Only even device numbers allowed to be marked as "dual HW device"
*       2. "Dual HW device" mode must be configured before any other
*       configuration that uses hwDevNum.
*       3. There are no restrictions on SW devNum for dual mode devices.
*
*/
static GT_STATUS internal_cpssDxChCscdDbRemoteHwDevNumModeSet
(
    IN GT_HW_DEV_NUM                    hwDevNum,
    IN CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT hwDevMode
)
{
    if (hwDevNum >= BIT_12)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if ((hwDevNum % 2) && hwDevMode == CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E == hwDevMode) ||
        (PRV_GLOBAL_PPDRV_HWINIT(prvCpssHwDevNumModeDb[hwDevNum]) == CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E))
    {
        PRV_GLOBAL_PPDRV_HWINIT(prvCpssHwDevNumModeDb[hwDevNum & 0xFFFFFFFE]) =
            PRV_GLOBAL_PPDRV_HWINIT(prvCpssHwDevNumModeDb[(hwDevNum & 0xFFFFFFFE) + 1]) =
            hwDevMode;
    }
    else if (CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E != hwDevMode)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else
    {
        PRV_GLOBAL_PPDRV_HWINIT(prvCpssHwDevNumModeDb[hwDevNum]) = hwDevMode;
    }

    return GT_OK;
}

/**
* @internal cpssDxChCscdDbRemoteHwDevNumModeSet function
* @endinternal
*
* @brief   Set single/dual HW device number mode to remote HW device number.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hwDevNum                 - HW device number (APPLICABLE RANGES: 0..(4K-1)).
* @param[in] hwDevMode                - single/dual HW device number mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - hwDevNum is odd number and hwDevMode is
*                                       CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E
* @retval GT_OUT_OF_RANGE          - when hwDevNum is out of range
*
* @note 1. Only even device numbers allowed to be marked as "dual HW device"
*       2. "Dual HW device" mode must be configured before any other
*       configuration that uses hwDevNum.
*       3. There are no restrictions on SW devNum for dual mode devices.
*
*/
GT_STATUS cpssDxChCscdDbRemoteHwDevNumModeSet
(
    IN GT_HW_DEV_NUM                    hwDevNum,
    IN CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT hwDevMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdDbRemoteHwDevNumModeSet);
    /*Must use zero level protection , do not remove this*/
    CPSS_ZERO_LEVEL_API_LOCK_MAC;
    CPSS_LOG_API_ENTER_MAC((funcId, hwDevNum, hwDevMode));

    rc = internal_cpssDxChCscdDbRemoteHwDevNumModeSet(hwDevNum, hwDevMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, hwDevNum, hwDevMode));
    CPSS_ZERO_LEVEL_API_UNLOCK_MAC;

    return rc;
}

/**
* @internal internal_cpssDxChCscdDbRemoteHwDevNumModeGet function
* @endinternal
*
* @brief   Get single/dual HW device number mode to remote HW device number.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hwDevNum                 - HW device number (APPLICABLE RANGES: 0..(4K-1)).
*
* @param[out] hwDevModePtr             - (pointer to) single/dual HW device number mode.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - when hwDevNum is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS internal_cpssDxChCscdDbRemoteHwDevNumModeGet
(
    IN  GT_HW_DEV_NUM                    hwDevNum,
    OUT CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT *hwDevModePtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(hwDevModePtr);

    if (hwDevNum >= BIT_12)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    *hwDevModePtr =  PRV_GLOBAL_PPDRV_HWINIT(prvCpssHwDevNumModeDb[hwDevNum]);

    return GT_OK;
}

/**
* @internal cpssDxChCscdDbRemoteHwDevNumModeGet function
* @endinternal
*
* @brief   Get single/dual HW device number mode to remote HW device number.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hwDevNum                 - HW device number (APPLICABLE RANGES: 0..(4K-1)).
*
* @param[out] hwDevModePtr             - (pointer to) single/dual HW device number mode.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - when hwDevNum is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChCscdDbRemoteHwDevNumModeGet
(
    IN  GT_HW_DEV_NUM                    hwDevNum,
    OUT CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT *hwDevModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdDbRemoteHwDevNumModeGet);
    /*Must use zero level protection , do not remove this*/
    CPSS_ZERO_LEVEL_API_LOCK_MAC;
    CPSS_LOG_API_ENTER_MAC((funcId, hwDevNum, hwDevModePtr));

    rc = internal_cpssDxChCscdDbRemoteHwDevNumModeGet(hwDevNum, hwDevModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, hwDevNum, hwDevModePtr));
     CPSS_ZERO_LEVEL_API_UNLOCK_MAC;

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet function
* @endinternal
*
* @brief   Set initial default ePort mapping assignment on ingress centralized chassis
*         enabled ports, when packets are received from line-card port/trunk accordingly.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portBaseEport            - base value used for default ePort mapping on
*                                      ingress CC enabled ports, when packets are
*                                      received from line-card port.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
* @param[in] trunkBaseEport           base value used for default ePort mapping on
*                                      ingress CC enabled ports, when packets are
*                                      received from line-card trunk.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_PORT_NUM                         portBaseEport,
    IN  GT_PORT_NUM                         trunkBaseEport
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portBaseEport);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,trunkBaseEport);

    /* write to TTI-Physical-Port-Attribute table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                                             portNum,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                             SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_PORTS_DEFAULT_EPORT_BASE_E, /* field name */
                                             PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                             portBaseEport);

    if (rc != GT_OK)
    {
        return rc;
    }

    /* write to TTI-Physical-Port-Attribute table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                                             portNum,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                             SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_TRUNK_DEFAULT_EPORT_BASE_E, /* field name */
                                             PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                             trunkBaseEport);
    return rc;
}

/**
* @internal cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet function
* @endinternal
*
* @brief   Set initial default ePort mapping assignment on ingress centralized chassis
*         enabled ports, when packets are received from line-card port/trunk accordingly.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portBaseEport            - base value used for default ePort mapping on
*                                      ingress CC enabled ports, when packets are
*                                      received from line-card port.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
* @param[in] trunkBaseEport           base value used for default ePort mapping on
*                                      ingress CC enabled ports, when packets are
*                                      received from line-card trunk.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
GT_STATUS cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_PORT_NUM                         portBaseEport,
    IN  GT_PORT_NUM                         trunkBaseEport
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portBaseEport, trunkBaseEport));

    rc = internal_cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet(devNum, portNum, portBaseEport, trunkBaseEport);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portBaseEport, trunkBaseEport));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet function
* @endinternal
*
* @brief   Get initial default ePort mapping assignment on ingress centralized chassis
*         enabled ports, when packets are received from line-card port/trunk accordingly.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] portBaseEportPtr         - (pointer to) base value used for default
*                                      ePort mapping on ingress CC enabled ports,
*                                      when packets are received from line-card port.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
* @param[out] trunkBaseEportPtr        - (pointer to) base value used for default
*                                      ePort mapping on ingress CC enabled ports,
*                                      when packets are received from line-card trunk.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_PORT_NUM                         *portBaseEportPtr,
    OUT GT_PORT_NUM                         *trunkBaseEportPtr
)
{
    GT_STATUS   rc;             /* function return code              */
    GT_U32      value;          /* value to read                     */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(portBaseEportPtr);
    CPSS_NULL_PTR_CHECK_MAC(trunkBaseEportPtr);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /* read from TTI-Physical-Port-Attribute table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                     CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                                     portNum,
                                     PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                     SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_PORTS_DEFAULT_EPORT_BASE_E, /* field name */
                                     PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                     &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *portBaseEportPtr = value;

    /* read from TTI-Physical-Port-Attribute table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                     CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                                     portNum,
                                     PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                     SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_TRUNK_DEFAULT_EPORT_BASE_E, /* field name */
                                     PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                     &value);

    *trunkBaseEportPtr = value;
    return rc;
}

/**
* @internal cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet function
* @endinternal
*
* @brief   Get initial default ePort mapping assignment on ingress centralized chassis
*         enabled ports, when packets are received from line-card port/trunk accordingly.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] portBaseEportPtr         - (pointer to) base value used for default
*                                      ePort mapping on ingress CC enabled ports,
*                                      when packets are received from line-card port.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
* @param[out] trunkBaseEportPtr        - (pointer to) base value used for default
*                                      ePort mapping on ingress CC enabled ports,
*                                      when packets are received from line-card trunk.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_PORT_NUM                         *portBaseEportPtr,
    OUT GT_PORT_NUM                         *trunkBaseEportPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portBaseEportPtr, trunkBaseEportPtr));

    rc = internal_cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet(devNum, portNum, portBaseEportPtr, trunkBaseEportPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portBaseEportPtr, trunkBaseEportPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet function
* @endinternal
*
* @brief   Set the amount of least significant bits taken from DSA tag
*         for assigning a default source ePort on CC ports, for packets received
*         from line-card device trunks/physical ports accordingly.
*         Relevant only when <CC Mode Enable> = Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] srcTrunkLsbAmount        - amount of least significant bits taken from DSA<SrcTrunk>
*                                      for assigning a default source ePort on CC ports, for
*                                      packets received from line-card device trunks.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 1.
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] srcPortLsbAmount         - amount of least significant bits taken from DSA<SrcPort>
*                                      for assigning a default source ePort on CC ports, for
*                                      packets received from line-card device physical ports.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 0.
*                                      (APPLICABLE RANGES: 0..6)
* @param[in] srcDevLsbAmount          - amount of least significant bits taken from DSA<SrcDev>
*                                      for assigning a default source ePort on CC ports, for
*                                      packets received from line-card device physical ports.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 0.
*                                      (APPLICABLE RANGES: 0..5)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              srcTrunkLsbAmount,
    IN  GT_U32                              srcPortLsbAmount,
    IN  GT_U32                              srcDevLsbAmount
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;        /* address of register */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    if (srcTrunkLsbAmount > 7 || srcPortLsbAmount > 6 || srcDevLsbAmount > 5)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* getting register address */
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfigForCCPorts;

    /* CC Trunk Mapping DSA<SrcTrunk> LSB Amount */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 3, srcTrunkLsbAmount);

    if (rc != GT_OK)
    {
        return rc;
    }

    /* CC Trunk Mapping DSA<SrcPort> LSB Amount */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 3, 3, srcPortLsbAmount);

    if (rc != GT_OK)
    {
        return rc;
    }

    /* CC Trunk Mapping DSA<SrcDev> LSB Amount */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 6, 3, srcDevLsbAmount);

    return rc;

}

/**
* @internal cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet function
* @endinternal
*
* @brief   Set the amount of least significant bits taken from DSA tag
*         for assigning a default source ePort on CC ports, for packets received
*         from line-card device trunks/physical ports accordingly.
*         Relevant only when <CC Mode Enable> = Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] srcTrunkLsbAmount        - amount of least significant bits taken from DSA<SrcTrunk>
*                                      for assigning a default source ePort on CC ports, for
*                                      packets received from line-card device trunks.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 1.
*                                      (APPLICABLE RANGES: 0..7)
* @param[in] srcPortLsbAmount         - amount of least significant bits taken from DSA<SrcPort>
*                                      for assigning a default source ePort on CC ports, for
*                                      packets received from line-card device physical ports.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 0.
*                                      (APPLICABLE RANGES: 0..6)
* @param[in] srcDevLsbAmount          - amount of least significant bits taken from DSA<SrcDev>
*                                      for assigning a default source ePort on CC ports, for
*                                      packets received from line-card device physical ports.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 0.
*                                      (APPLICABLE RANGES: 0..5)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
GT_STATUS cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              srcTrunkLsbAmount,
    IN  GT_U32                              srcPortLsbAmount,
    IN  GT_U32                              srcDevLsbAmount
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, srcTrunkLsbAmount, srcPortLsbAmount, srcDevLsbAmount));

    rc = internal_cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet(devNum, srcTrunkLsbAmount, srcPortLsbAmount, srcDevLsbAmount);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srcTrunkLsbAmount, srcPortLsbAmount, srcDevLsbAmount));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet function
* @endinternal
*
* @brief   Get the amount of least significant bits taken from DSA tag
*         for assigning a default source ePort on CC ports, for packets received
*         from line-card device trunks/physical ports accordingly.
*         Relevant only when <CC Mode Enable> = Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] srcTrunkLsbAmountPtr     - (pointer to) amount of least significant bits taken
*                                      from DSA<SrcTrunk> for assigning a default source
*                                      ePort on CC ports, for packets received from
*                                      line-card device trunks.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 1.
*                                      (APPLICABLE RANGES: 0..7)
* @param[out] srcPortLsbAmountPtr      - (pointer to) amount of least significant bits taken
*                                      from DSA<SrcPort> for assigning a default source
*                                      ePort on CC ports, for packets received from
*                                      line-card device physical ports.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 0.
*                                      (APPLICABLE RANGES: 0..6)
* @param[out] srcDevLsbAmountPtr       - (pointer to) amount of least significant bits taken
*                                      from DSA<SrcDev> for assigning a default source
*                                      ePort on CC ports, for packets received from
*                                      line-card device physical ports.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 0.
*                                      (APPLICABLE RANGES: 0..5)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet
(
    IN  GT_U8                               devNum,
    OUT GT_U32                              *srcTrunkLsbAmountPtr,
    OUT GT_U32                              *srcPortLsbAmountPtr,
    OUT GT_U32                              *srcDevLsbAmountPtr
)
{
    GT_STATUS   rc;             /* function return code              */
    GT_U32      value;          /* value to read                     */
    GT_U32      regAddr;        /* address of register               */


    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(srcTrunkLsbAmountPtr);
    CPSS_NULL_PTR_CHECK_MAC(srcPortLsbAmountPtr);
    CPSS_NULL_PTR_CHECK_MAC(srcDevLsbAmountPtr);

    /* getting register address */
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfigForCCPorts;

    /* CC Trunk Mapping DSA<SrcTrunk> LSB Amount */
    rc = prvCpssHwPpGetRegField (devNum, regAddr, 0, 3, &value);
    *srcTrunkLsbAmountPtr = value;

    if (rc != GT_OK)
    {
        return rc;
    }

    /* CC Trunk Mapping DSA<SrcPort> LSB Amount */
    rc = prvCpssHwPpGetRegField (devNum, regAddr, 3, 3, &value);
    *srcPortLsbAmountPtr = value;
    if (rc != GT_OK)
    {
        return rc;
    }

    /* CC Trunk Mapping DSA<SrcDev> LSB Amount */
    rc = prvCpssHwPpGetRegField (devNum, regAddr, 6, 3, &value);
    *srcDevLsbAmountPtr = value;

    return rc;
}

/**
* @internal cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet function
* @endinternal
*
* @brief   Get the amount of least significant bits taken from DSA tag
*         for assigning a default source ePort on CC ports, for packets received
*         from line-card device trunks/physical ports accordingly.
*         Relevant only when <CC Mode Enable> = Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] srcTrunkLsbAmountPtr     - (pointer to) amount of least significant bits taken
*                                      from DSA<SrcTrunk> for assigning a default source
*                                      ePort on CC ports, for packets received from
*                                      line-card device trunks.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 1.
*                                      (APPLICABLE RANGES: 0..7)
* @param[out] srcPortLsbAmountPtr      - (pointer to) amount of least significant bits taken
*                                      from DSA<SrcPort> for assigning a default source
*                                      ePort on CC ports, for packets received from
*                                      line-card device physical ports.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 0.
*                                      (APPLICABLE RANGES: 0..6)
* @param[out] srcDevLsbAmountPtr       - (pointer to) amount of least significant bits taken
*                                      from DSA<SrcDev> for assigning a default source
*                                      ePort on CC ports, for packets received from
*                                      line-card device physical ports.
*                                      Relevant only for packet DSA<SrcIsTrunk> = 0.
*                                      (APPLICABLE RANGES: 0..5)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet
(
    IN  GT_U8                               devNum,
    OUT GT_U32                              *srcTrunkLsbAmountPtr,
    OUT GT_U32                              *srcPortLsbAmountPtr,
    OUT GT_U32                              *srcDevLsbAmountPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, srcTrunkLsbAmountPtr, srcPortLsbAmountPtr, srcDevLsbAmountPtr));

    rc = internal_cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet(devNum, srcTrunkLsbAmountPtr, srcPortLsbAmountPtr, srcDevLsbAmountPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srcTrunkLsbAmountPtr, srcPortLsbAmountPtr, srcDevLsbAmountPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChCscdPortMruCheckOnCascadeEnableSet function
* @endinternal
*
* @brief   Enable/Disable MRU Check On Cascade Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port
* @param[in] enable                   - MRU check Mode
*                                      GT_FALSE: Disable MRU Check On Cascade Port
*                                      GT_TRUE:  Enable MRU Check On Cascade Port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdPortMruCheckOnCascadeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /* write to TTI-Physical-Port-Attribute table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                                         portNum,
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                         SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENABLE_MRU_CHECK_ON_CASCADE_PORT_E, /* field name */
                                         PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                         BOOL2BIT_MAC(enable));
    return rc;

}

/**
* @internal cpssDxChCscdPortMruCheckOnCascadeEnableSet function
* @endinternal
*
* @brief   Enable/Disable MRU Check On Cascade Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port
* @param[in] enable                   - MRU check Mode
*                                      GT_FALSE: Disable MRU Check On Cascade Port
*                                      GT_TRUE:  Enable MRU Check On Cascade Port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdPortMruCheckOnCascadeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortMruCheckOnCascadeEnableSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChCscdPortMruCheckOnCascadeEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdPortMruCheckOnCascadeEnableGet function
* @endinternal
*
* @brief   Get (Enable/Disable) MRU Check On Cascade Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port
*
* @param[out] enablePtr                - (pointer to) MRU check Mode
*                                      GT_FALSE: Disable MRU Check On Cascade Port
*                                      GT_TRUE:  Enable MRU Check On Cascade Port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdPortMruCheckOnCascadeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS   rc;             /* function return code              */
    GT_U32      value;          /* value to write                    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /* read from TTI-Physical-Port-Attribute table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENABLE_MRU_CHECK_ON_CASCADE_PORT_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChCscdPortMruCheckOnCascadeEnableGet function
* @endinternal
*
* @brief   Get (Enable/Disable) MRU Check On Cascade Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port
*
* @param[out] enablePtr                - (pointer to) MRU check Mode
*                                      GT_FALSE: Disable MRU Check On Cascade Port
*                                      GT_TRUE:  Enable MRU Check On Cascade Port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdPortMruCheckOnCascadeEnableGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortMruCheckOnCascadeEnableGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChCscdPortMruCheckOnCascadeEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet function
* @endinternal
*
* @brief   Set the number of LSB bits taken from the DSA tag <source device> and
*         <source port>, that are used to map the My Physical port to a physical
*         port.
*         Relevant when <Assign Physical Port From DSA Enable> is enabled in the
*         My Physical Port Table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] srcDevLsbAmount          - the number of bits from the DSA <source device>
*                                      used for the mapping
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] srcPortLsbAmount         - the number of bits from the DSA <source port> used
*                                      for the mapping
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet
(
    IN GT_U8        devNum,
    IN GT_U32       srcDevLsbAmount,
    IN GT_U32       srcPortLsbAmount
)
{
    GT_U32      regAddr;
    GT_U32      value;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    if ((srcDevLsbAmount > 15) || (srcPortLsbAmount > 15))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).phyPortAndEPortAttributes.
        remotePhysicalPortAssignmentGlobalConfig;
    value = (srcPortLsbAmount | (srcDevLsbAmount << 4));

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 8, value);
}

/**
* @internal cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet function
* @endinternal
*
* @brief   Set the number of LSB bits taken from the DSA tag <source device> and
*         <source port>, that are used to map the My Physical port to a physical
*         port.
*         Relevant when <Assign Physical Port From DSA Enable> is enabled in the
*         My Physical Port Table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] srcDevLsbAmount          - the number of bits from the DSA <source device>
*                                      used for the mapping
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] srcPortLsbAmount         - the number of bits from the DSA <source port> used
*                                      for the mapping
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
GT_STATUS cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet
(
    IN GT_U8        devNum,
    IN GT_U32       srcDevLsbAmount,
    IN GT_U32       srcPortLsbAmount
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, srcDevLsbAmount, srcPortLsbAmount));

    rc = internal_cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet(devNum, srcDevLsbAmount, srcPortLsbAmount);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srcDevLsbAmount, srcPortLsbAmount));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet function
* @endinternal
*
* @brief   Get the number of LSB bits taken from the DSA tag <source device> and
*         <source port>, that are used to map the My Physical port to a physical
*         port.
*         Relevant when <Assign Physical Port From DSA Enable> is enabled in the
*         My Physical Port Table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] srcDevLsbAmountPtr       - (pointer to) the number of bits from the DSA
*                                      <source device> used for the mapping
* @param[out] srcPortLsbAmountPtr      - (pointer to) the number of bits from the DSA
*                                      <source port> used for the mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *srcDevLsbAmountPtr,
    OUT GT_U32      *srcPortLsbAmountPtr
)
{
    GT_U32      regAddr;
    GT_U32      value;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(srcDevLsbAmountPtr);
    CPSS_NULL_PTR_CHECK_MAC(srcPortLsbAmountPtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).phyPortAndEPortAttributes.
        remotePhysicalPortAssignmentGlobalConfig;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 8, &value);
    if (rc == GT_OK)
    {
        *srcDevLsbAmountPtr = ((value >> 4) & 0xF);
        *srcPortLsbAmountPtr = (value & 0xF);
    }
    return rc;
}

/**
* @internal cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet function
* @endinternal
*
* @brief   Get the number of LSB bits taken from the DSA tag <source device> and
*         <source port>, that are used to map the My Physical port to a physical
*         port.
*         Relevant when <Assign Physical Port From DSA Enable> is enabled in the
*         My Physical Port Table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] srcDevLsbAmountPtr       - (pointer to) the number of bits from the DSA
*                                      <source device> used for the mapping
* @param[out] srcPortLsbAmountPtr      - (pointer to) the number of bits from the DSA
*                                      <source port> used for the mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *srcDevLsbAmountPtr,
    OUT GT_U32      *srcPortLsbAmountPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, srcDevLsbAmountPtr, srcPortLsbAmountPtr));

    rc = internal_cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet(devNum, srcDevLsbAmountPtr, srcPortLsbAmountPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srcDevLsbAmountPtr, srcPortLsbAmountPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal
*           internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet
*           function
* @endinternal
*
* @brief   Set TPID value of the VLAN Tag taken from the
*           outermost VLAN Tag TPID port. Relevant when enabling
*           VLAN Tag based remote physical ports mapping, this
*           field specifies the TPID value of the VLAN Tag
*
* @note   APPLICABLE DEVICES:    Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] vlanTagTpid       -  TPID value of the VLAN Tag.
*                              (APPLICABLE RANGES:0..0xffff)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      vlanTagTpid
)
{
    GT_U32      regAddr;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    if (vlanTagTpid > 0xffff)
    {
        /* TPID max value is 0xffff*/
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).phyPortAndEPortAttributes.remotePhysicalPortAssignmentGlobalConfigExt ;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 16, 16, vlanTagTpid);
    return rc;
}


/**
* @internal
*           cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet
*           function
* @endinternal
*
* @brief   Set TPID value of the VLAN Tag taken from the
*           outermost VLAN Tag TPID port. Relevant when enabling
*           VLAN Tag based remote physical ports mapping, this
*           field specifies the TPID value of the VLAN Tag
*
* @note   APPLICABLE DEVICES:    Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] vlanTagTpid       -  TPID value of the VLAN Tag.
*                               (APPLICABLE RANGES:0..0xffff)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      vlanTagTpid
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanTagTpid));

    rc = internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet(devNum, vlanTagTpid);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanTagTpid));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal
*           internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet
*           function
* @endinternal
*
* @brief   Get TPID value of the VLAN Tag taken from the outermost VLAN Tag TPID port.
*           Relevant when enabling VLAN Tag based remote physical ports
*           mapping, this field specifies the TPID value of the VLAN Tag
*
* @note   APPLICABLE DEVICES:    Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - device number
*
* @param[out] vlanTagTpidPtr       - (pointer to) TPID value of the VLAN Tag
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *vlanTagTpidPtr
)
{
    GT_U32      regAddr;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(vlanTagTpidPtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).phyPortAndEPortAttributes.remotePhysicalPortAssignmentGlobalConfigExt ;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 16, 16, vlanTagTpidPtr);
    return rc;
}


/**
* @internal
*           cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet
*           function
* @endinternal
*
* @brief   Get TPID value of the VLAN Tag taken from the outermost VLAN Tag TPID port.
*           Relevant when enabling VLAN Tag based remote physical ports
*           mapping, this field specifies the TPID value of the VLAN Tag
*
* @note   APPLICABLE DEVICES:    Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - device number
*
* @param[out] vlanTagTpidPtr       - (pointer to) TPID value of the VLAN Tag
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet
(
  IN GT_U8        devNum,
  OUT GT_U32      *vlanTagTpidPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanTagTpidPtr));

    rc = internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet(devNum, vlanTagTpidPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanTagTpidPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal prvCpssDxChCscdMyPhysicalPortAttributesRegAddressGet function
* @endinternal
*
* @brief   Get address of myPhysicalPort Attributes register by myPhysicalPort number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] myPhysicalPortNum        - my physical port number assigned by RxDMA unit.
*                                      Assumed that physical port (used as cascade)
*                                      with such Id mapped.
*
* @param[out] regAddrPtr               - (pointer to) register address.
* @param[out] portGroupNumPtr          - (pointer to) the port group Id that the
*                                         'physical port' belong to
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChCscdMyPhysicalPortAttributesRegAddressGet
(
    IN   GT_U8                    devNum,
    IN   GT_PHYSICAL_PORT_NUM     myPhysicalPortNum,
    OUT  GT_U32                   *regAddrPtr,
    OUT  GT_U32                   *portGroupNumPtr
)
{
    GT_STATUS rc;
    GT_U32    portGroupId, myPhyPortIndex;
    GT_U32    numEntries = 256;
    GT_U32    ttiInstanceId = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, myPhysicalPortNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* according to the number of entries in
           phyPortAndEPortAttributes.myPhysicalPortAttributes[] */
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tti.TTI_myPhysicalPortAttributes_numEntries)
        {
            numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tti.TTI_myPhysicalPortAttributes_numEntries;
        }
        else
        {
            numEntries = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? 75 : 288;
        }

        /* NOTE: we are not allowing the 'trick' that we allowed in the BC3 to
           come with value 256..511

           because the TTI not hold '2^x' entries (288 is not '2^x')
        */

        if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) && myPhysicalPortNum == CPSS_CPU_PORT_NUM_CNS/*63*/&&
           numEntries <= CPSS_CPU_PORT_NUM_CNS)
        {
            /* use the last index in the table for the CPU port */
            myPhysicalPortNum = numEntries-1;
        }

        /* we not allow value higher than 287 */
        if(myPhysicalPortNum >= numEntries)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "myPhysicalPortNum[%d] must be less than [%d]",
                myPhysicalPortNum,
                numEntries);
        }
    }

    myPhyPortIndex = (myPhysicalPortNum % numEntries);

    if (0 == PRV_CPSS_IS_MULTI_PIPES_DEVICE_MAC(devNum))
    {
        *regAddrPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).
            phyPortAndEPortAttributes.myPhysicalPortAttributes[myPhyPortIndex];

        *portGroupNumPtr = 0;
        return GT_OK;
    }

    /* convert the physical port to the actual portGroup that occupy it */
    rc = prvCpssDxChHwPpPhysicalPortNumToPortGroupIdConvert(
        devNum, myPhysicalPortNum, &portGroupId);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* use address from TTI[0] and it will be manipulated to proper 'pipe'
            by the prvCpssFalconDuplicatedMultiPortGroupsGet_byDevNum(...) */
        ttiInstanceId = 0;
    }
    else/* Bobcat3 only */
    {
        ttiInstanceId = portGroupId;
    }

    if (ttiInstanceId == 0)
    {
        *regAddrPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).
            phyPortAndEPortAttributes.myPhysicalPortAttributes[myPhyPortIndex];
    }
    else
    {
        *regAddrPtr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).
            phyPortAndEPortAttributes.myPhysicalPortAttributes2[myPhyPortIndex];
    }

    *portGroupNumPtr = portGroupId;

    return GT_OK;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet function
* @endinternal
*
* @brief   Set the ingress physical port assignment mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
* @param[in] assignmentMode           - the physical port assignment mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet
(
    IN GT_U8                                                        devNum,
    IN GT_PHYSICAL_PORT_NUM                                         portNum,
    IN CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT     assignmentMode
)
{
    GT_U32      regAddr;
    GT_U32      value;
    GT_U32      portGroupId;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    switch (assignmentMode)
    {
        case CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DISABLED_E:
            value = 0;
            break;

        case CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_INTERLAKEN_E:
            value = 1;
            break;

        case CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E:
            value = 2;
            break;

        case CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_E_TAG_E:
            if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            value = 3;
            break;

        case CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_VLAN_TAG_E:
            if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            value = 4;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (prvCpssDxChCscdMyPhysicalPortAttributesRegAddressGet(
        devNum, portNum, &regAddr , &portGroupId) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId , regAddr,
        PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 11 : 9,
        PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? 3 : 2, value);
}

/**
* @internal cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet function
* @endinternal
*
* @brief   Set the ingress physical port assignment mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
* @param[in] assignmentMode           - the physical port assignment mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet
(
    IN GT_U8                                                        devNum,
    IN GT_PHYSICAL_PORT_NUM                                         portNum,
    IN CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT     assignmentMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, assignmentMode));

    rc = internal_cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet(devNum, portNum, assignmentMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, assignmentMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet function
* @endinternal
*
* @brief   Get the ingress physical port assignment mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
*
* @param[out] assignmentModePtr        - (pointer to) the physical port assignment mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    OUT CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT    *assignmentModePtr
)
{
    GT_U32      regAddr;
    GT_U32      value;
    GT_STATUS   rc;
    GT_U32      portGroupId;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(assignmentModePtr);

    if (prvCpssDxChCscdMyPhysicalPortAttributesRegAddressGet(
        devNum, portNum, &regAddr , &portGroupId) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId , regAddr,
        PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 11 : 9,
        PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? 3 : 2, &value);
    if (rc == GT_OK)
    {
        switch (value)
        {
            case 0:
                *assignmentModePtr = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DISABLED_E;
                break;

            case 1:
                *assignmentModePtr = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_INTERLAKEN_E;
                break;

            case 2:
                *assignmentModePtr = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E;
                break;
            case 3:
                *assignmentModePtr = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_E_TAG_E;
                break;
            case 4:
                *assignmentModePtr = CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_VLAN_TAG_E;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }
    return rc;
}

/**
* @internal cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet function
* @endinternal
*
* @brief   Get the ingress physical port assignment mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
*
* @param[out] assignmentModePtr        - (pointer to) the physical port assignment mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet
(
    IN  GT_U8                                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                        portNum,
    OUT CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT    *assignmentModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, assignmentModePtr));

    rc = internal_cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet(devNum, portNum, assignmentModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, assignmentModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet function
* @endinternal
*
* @brief   Set the ingress physical port base. It's used to map the My Physical
*         port to a physical port.
*         Relevant only when the ingress Physical Port assignment mode is
*         configured to Interlaken or DSA mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
* @param[in] physicalPortBase         - the physical port base (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   physicalPortBase
)
{
    GT_U32      regAddr;
    GT_U32      numOfBits;
    GT_U32      portGroupId;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    numOfBits = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 10 : 8 ;

    if (physicalPortBase >= (GT_U32)(1<<numOfBits))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (prvCpssDxChCscdMyPhysicalPortAttributesRegAddressGet(
        devNum, portNum, &regAddr , &portGroupId) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId , regAddr, 1, numOfBits,
        physicalPortBase);
}

/**
* @internal cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet function
* @endinternal
*
* @brief   Set the ingress physical port base. It's used to map the My Physical
*         port to a physical port.
*         Relevant only when the ingress Physical Port assignment mode is
*         configured to Interlaken or DSA mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
* @param[in] physicalPortBase         - the physical port base (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
GT_STATUS cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   physicalPortBase
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, physicalPortBase));

    rc = internal_cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet(devNum, portNum, physicalPortBase);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, physicalPortBase));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet function
* @endinternal
*
* @brief   Get the ingress physical port base. It's used to map the My Physical
*         port to a physical port.
*         Relevant only when the ingress Physical Port assignment mode is
*         configured to Interlaken or DSA mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
*
* @param[out] physicalPortBasePtr      - (pointer to) the physical port base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *physicalPortBasePtr
)
{
    GT_U32      regAddr;
    GT_U32      portGroupId;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    CPSS_NULL_PTR_CHECK_MAC(physicalPortBasePtr);

    if (prvCpssDxChCscdMyPhysicalPortAttributesRegAddressGet(
        devNum, portNum, &regAddr , &portGroupId) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpPortGroupGetRegField(devNum, portGroupId , regAddr, 1,
        PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 10 : 8,
        physicalPortBasePtr);
}

/**
* @internal cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet function
* @endinternal
*
* @brief   Get the ingress physical port base. It's used to map the My Physical
*         port to a physical port.
*         Relevant only when the ingress Physical Port assignment mode is
*         configured to Interlaken or DSA mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
*
* @param[out] physicalPortBasePtr      - (pointer to) the physical port base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *physicalPortBasePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, physicalPortBasePtr));

    rc = internal_cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet(devNum, portNum, physicalPortBasePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, physicalPortBasePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet function
* @endinternal
*
* @brief   Enable the assignment of the packet source device to be the local device
*         number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
* @param[in] enable                   -  the assignment
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  enable
)
{
    GT_U32      regAddr;
    GT_U32      value;
    GT_U32      portGroupId;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    if (prvCpssDxChCscdMyPhysicalPortAttributesRegAddressGet(
        devNum, portNum, &regAddr , &portGroupId) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    value = BOOL2BIT_MAC(enable);

    return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId , regAddr, 0, 1, value);
}

/**
* @internal cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet function
* @endinternal
*
* @brief   Enable the assignment of the packet source device to be the local device
*         number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
* @param[in] enable                   -  the assignment
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet function
* @endinternal
*
* @brief   Get the enabling status of the assignment of the packet source device to
*         be the local device number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
*
* @param[out] enablePtr                - (pointer to) the enabling status of the assignment
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_U32      regAddr;
    GT_U32      value;
    GT_STATUS   rc;
    GT_U32      portGroupId;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if (prvCpssDxChCscdMyPhysicalPortAttributesRegAddressGet(
        devNum, portNum, &regAddr , &portGroupId) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId , regAddr, 0, 1, &value);
    if (rc == GT_OK)
    {
        *enablePtr = BIT2BOOL_MAC(value);
    }
    return rc;
}

/**
* @internal cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet function
* @endinternal
*
* @brief   Get the enabling status of the assignment of the packet source device to
*         be the local device number
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port
*
* @param[out] enablePtr                - (pointer to) the enabling status of the assignment
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdPortStackAggregationEnableSet function
* @endinternal
*
* @brief   Enable/disable stack aggregation per port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE -  stack aggregation
*                                      GT_FALSE - disable stack aggregation
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B0
*
*/
static GT_STATUS internal_cpssDxChCscdPortStackAggregationEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              enable
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      portGroupId;    /* the port group Id */
    GT_U32       localPort;      /* local port */
    GT_U32      data;           /* register data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

    if(!PRV_CPSS_DXCH_LION2_B0_AND_ABOVE_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* convert the 'Physical port' to portGroupId,local port */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

/*
    ERSPAN Entry0 Word1

    [11:0] aggregator_port_en[11:0]
    [13:12] Reserved
    [15:14] aggregator_port_en[15:14]
    [31:16] Reserved
*/
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.erspanEntryWord1[0];

    data = BOOL2BIT_MAC(enable);

    return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, localPort, 1, data);
}

/**
* @internal cpssDxChCscdPortStackAggregationEnableSet function
* @endinternal
*
* @brief   Enable/disable stack aggregation per port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE -  stack aggregation
*                                      GT_FALSE - disable stack aggregation
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B0
*
*/
GT_STATUS cpssDxChCscdPortStackAggregationEnableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortStackAggregationEnableSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChCscdPortStackAggregationEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCscdPortStackAggregationEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of stack aggregation per port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - aggregator port number
*
* @param[out] enablePtr                - (pointer to)GT_TRUE - enable stack aggregation
*                                      GT_FALSE - disable stack aggregation
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B0
*
*/
static GT_STATUS internal_cpssDxChCscdPortStackAggregationEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      regAddr;        /* register address */
    GT_U32      portGroupId;    /* the port group Id */
    GT_U32       localPort;      /* local port */
    GT_U32      data;           /* register data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(!PRV_CPSS_DXCH_LION2_B0_AND_ABOVE_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* convert the 'Physical port' to portGroupId,local port */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.erspanEntryWord1[0];

    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, localPort, 1, &data);
    if(GT_OK != rc)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(data);

    return GT_OK;
}

/**
* @internal cpssDxChCscdPortStackAggregationEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of stack aggregation per port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - aggregator port number
*
* @param[out] enablePtr                - (pointer to)GT_TRUE - enable stack aggregation
*                                      GT_FALSE - disable stack aggregation
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B0
*
*/
GT_STATUS cpssDxChCscdPortStackAggregationEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortStackAggregationEnableGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChCscdPortStackAggregationEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCscdPortStackAggregationConfigSet function
* @endinternal
*
* @brief   Set stack aggregation configuration per egress port.
*         All Forward DSA tagged packets transmitted from cascaded port with
*         these configurable source device number, source port numbers and
*         source IDs when feature enabled by
*         cpssDxChCscdPortStackAggregationEnableSet().
*         Configuration should be applied on aggregator device only.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] aggHwDevNum              - aggregator HW device number
* @param[in] aggPortNum               - aggregator port number
* @param[in] aggSrcId                 - aggregator source ID
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portNum
* @retval GT_OUT_OF_RANGE          - on wrong aggHwDevNum, aggPortNum, aggSrcId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B0
*
*/
static GT_STATUS internal_cpssDxChCscdPortStackAggregationConfigSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_HW_DEV_NUM        aggHwDevNum,
    IN  GT_PORT_NUM          aggPortNum,
    IN  GT_U32               aggSrcId
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      regOffset;      /* register offset */
    GT_U32      portGroupId;    /* the port group Id */
    GT_U32       localPort;      /* local port */
    GT_U32      data;           /* register data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

    if(!PRV_CPSS_DXCH_LION2_B0_AND_ABOVE_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if((aggHwDevNum >= BIT_5) || (aggPortNum >= BIT_6) || (aggSrcId >= BIT_5))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* convert the 'Physical port' to portGroupId,local port */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

/*
        ERSPAN Entry<1-6> Word1
    [4:0] aggregator_dev_<(x-1)*2>[4:0]
    [10:5] aggregator_port_<(x-1)*2> [5:0]
    [15:11] aggregator_sst_id_<(x-1)*2> [4:0]
    [20:16] aggregator_dev_<[(x-1)*2]+1> [4:0]
    [26:21] aggregator_port_<[(x-1)*2]+1> [5:0]
    [31:27] aggregator_sst_id_<[(x-1)*2]+1> [4:0]

        ERSPAN Entry<7> Word1
    [4:0] aggregator_dev_14[4:0]
    [10:5] aggregator_port_14[5:0]
    [15:11] aggregator_sst_id_14[4:0]
    [20:16] aggregator_dev_15[4:0]
    [26:21] aggregator_port_15[5:0]
    [31:27] aggregator_sst_id_15[4:0]
*/

    regOffset = (localPort % 2 == 0) ? 0 : 16;

    if(localPort > 11)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.erspanEntryWord1[7];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.erspanEntryWord1[localPort/2 + 1];
    }

    data = aggHwDevNum | (aggPortNum << 5) | (aggSrcId << 11);

    return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, regOffset, 16, data);
}

/**
* @internal cpssDxChCscdPortStackAggregationConfigSet function
* @endinternal
*
* @brief   Set stack aggregation configuration per egress port.
*         All Forward DSA tagged packets transmitted from cascaded port with
*         these configurable source device number, source port numbers and
*         source IDs when feature enabled by
*         cpssDxChCscdPortStackAggregationEnableSet().
*         Configuration should be applied on aggregator device only.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] aggHwDevNum              - aggregator HW device number
* @param[in] aggPortNum               - aggregator port number
* @param[in] aggSrcId                 - aggregator source ID
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portNum
* @retval GT_OUT_OF_RANGE          - on wrong aggHwDevNum, aggPortNum, aggSrcId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B0
*
*/
GT_STATUS cpssDxChCscdPortStackAggregationConfigSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_HW_DEV_NUM        aggHwDevNum,
    IN  GT_PORT_NUM          aggPortNum,
    IN  GT_U32               aggSrcId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortStackAggregationConfigSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, aggHwDevNum, aggPortNum, aggSrcId));

    rc = internal_cpssDxChCscdPortStackAggregationConfigSet(devNum, portNum, aggHwDevNum, aggPortNum, aggSrcId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, aggHwDevNum, aggPortNum, aggSrcId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCscdPortStackAggregationConfigGet function
* @endinternal
*
* @brief   Get stack aggregation configuration per egress port.
*         All Forward DSA tagged packets transmitted from cascaded port with
*         these configurable source device number, source port numbers and
*         source IDs when feature enabled by
*         cpssDxChCscdPortStackAggregationEnableSet().
*         Configuration should be applied on aggregator device only.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] aggHwDevNumPtr           - (pointer to) aggregator HW device number
* @param[out] aggPortNumPtr            - (pointer to) aggregator port number
* @param[out] aggSrcIdPtr              - (pointer to) aggregator source ID
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B0
*
*/
static GT_STATUS internal_cpssDxChCscdPortStackAggregationConfigGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_HW_DEV_NUM        *aggHwDevNumPtr,
    OUT GT_PORT_NUM          *aggPortNumPtr,
    OUT GT_U32               *aggSrcIdPtr
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      regAddr;        /* register address */
    GT_U32      regOffset;      /* register offset */
    GT_U32      portGroupId;    /* the port group Id */
    GT_U32       localPort;      /* local port */
    GT_U32      data;           /* register data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(aggHwDevNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(aggPortNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(aggSrcIdPtr);

    if(!PRV_CPSS_DXCH_LION2_B0_AND_ABOVE_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* convert the 'Physical port' to portGroupId,local port */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

/*
        ERSPAN Entry<1-6> Word1
    [4:0] aggregator_dev_<(x-1)*2>[4:0]
    [10:5] aggregator_port_<(x-1)*2> [5:0]
    [15:11] aggregator_sst_id_<(x-1)*2> [4:0]
    [20:16] aggregator_dev_<[(x-1)*2]+1> [4:0]
    [26:21] aggregator_port_<[(x-1)*2]+1> [5:0]
    [31:27] aggregator_sst_id_<[(x-1)*2]+1> [4:0]

        ERSPAN Entry<7> Word1
    [4:0] aggregator_dev_14[4:0]
    [10:5] aggregator_port_14[5:0]
    [15:11] aggregator_sst_id_14[4:0]
    [20:16] aggregator_dev_15[4:0]
    [26:21] aggregator_port_15[5:0]
    [31:27] aggregator_sst_id_15[4:0]
*/

    regOffset = (localPort % 2 == 0) ? 0 : 16;

    if(localPort > 11)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.erspanEntryWord1[7];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.erspanEntryWord1[localPort/2 + 1];
    }

    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, regOffset, 16, &data);
    if(GT_OK != rc)
    {
        return rc;
    }

    *aggHwDevNumPtr = U32_GET_FIELD_MAC(data, 0, 5);
    *aggPortNumPtr = U32_GET_FIELD_MAC(data, 5, 6);
    *aggSrcIdPtr = U32_GET_FIELD_MAC(data, 11, 5);

    return rc;
}

/**
* @internal cpssDxChCscdPortStackAggregationConfigGet function
* @endinternal
*
* @brief   Get stack aggregation configuration per egress port.
*         All Forward DSA tagged packets transmitted from cascaded port with
*         these configurable source device number, source port numbers and
*         source IDs when feature enabled by
*         cpssDxChCscdPortStackAggregationEnableSet().
*         Configuration should be applied on aggregator device only.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] aggHwDevNumPtr           - (pointer to) aggregator HW device number
* @param[out] aggPortNumPtr            - (pointer to) aggregator port number
* @param[out] aggSrcIdPtr              - (pointer to) aggregator source ID
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 device: applicable starting from revision B0
*
*/
GT_STATUS cpssDxChCscdPortStackAggregationConfigGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_HW_DEV_NUM        *aggHwDevNumPtr,
    OUT GT_PORT_NUM          *aggPortNumPtr,
    OUT GT_U32               *aggSrcIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortStackAggregationConfigGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, aggHwDevNumPtr, aggPortNumPtr, aggSrcIdPtr));

    rc = internal_cpssDxChCscdPortStackAggregationConfigGet(devNum, portNum, aggHwDevNumPtr, aggPortNumPtr, aggSrcIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, aggHwDevNumPtr, aggPortNumPtr, aggSrcIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet function
* @endinternal
*
* @brief   Enable mapping of target physical port to a remote physical port that
*         resides over a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - target device number
* @param[in] portNum                  - target physical port
* @param[in] enable                   -  mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_U32      value;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    value = BOOL2BIT_MAC(enable);
    return prvCpssDxChWriteTableEntryField(devNum,
                                           CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                           portNum,
                                           PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                           SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_MAP_ENABLE_E,
                                           PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                           value);
}

/**
* @internal cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet function
* @endinternal
*
* @brief   Enable mapping of target physical port to a remote physical port that
*         resides over a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - target device number
* @param[in] portNum                  - target physical port
* @param[in] enable                   -  mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet function
* @endinternal
*
* @brief   Get the enabling status of the mapping of target physical port to a
*         remote physical port that resides over a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - target device number
* @param[in] portNum                  - target physical port
*
* @param[out] enablePtr                - (pointer to) the enabling status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    OUT GT_BOOL                   *enablePtr
)
{
    GT_U32      value;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_MAP_ENABLE_E,
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);
    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet function
* @endinternal
*
* @brief   Get the enabling status of the mapping of target physical port to a
*         remote physical port that resides over a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - target device number
* @param[in] portNum                  - target physical port
*
* @param[out] enablePtr                - (pointer to) the enabling status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    OUT GT_BOOL                   *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet function
* @endinternal
*
* @brief   Map a target physical port to a remote physical port that resides over
*         a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - target device number
* @param[in] portNum                  - target physical port
* @param[in] remotePhysicalHwDevNum   - the remote physical HW device
*                                      (APPLICABLE RANGES: 0..4095)
* @param[in] remotePhysicalPortNum    - the remote physical port
*                                      (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The mapping should be enabled in order to take effect. To enable the
*       mapping use cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet
*
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_HW_DEV_NUM           remotePhysicalHwDevNum,
    IN  GT_PHYSICAL_PORT_NUM    remotePhysicalPortNum
)
{
    GT_STATUS   rc;
    GT_U32      value;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /*
        do not set 2 fields together , because we not need here logic according
            to sip version.
        the proper limit of the 'size' of field is already exists in the
        'write field in entry of table'

        NOTE: in sip6 the remotePhysicalPortNum is 10 bits (not 8)
    */


    value = remotePhysicalPortNum;

    rc = prvCpssDxChWriteTableEntryField(devNum,
                                           CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                           portNum,
                                           PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                           SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_NUMBER_E,
                                           PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                           value);
    if(rc != GT_OK)
    {
        return rc;
    }

    value = remotePhysicalHwDevNum;

    rc = prvCpssDxChWriteTableEntryField(devNum,
                                           CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                           portNum,
                                           PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                           SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_DEVICE_ID_E,
                                           PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                           value);

    return rc;

}

/**
* @internal cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet function
* @endinternal
*
* @brief   Map a target physical port to a remote physical port that resides over
*         a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - target device number
* @param[in] portNum                  - target physical port
* @param[in] remotePhysicalHwDevNum   - the remote physical HW device
*                                      (APPLICABLE RANGES: 0..4095)
* @param[in] remotePhysicalPortNum    - the remote physical port
*                                      (APPLICABLE RANGES: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The mapping should be enabled in order to take effect. To enable the
*       mapping use cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet
*
*/
GT_STATUS cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_HW_DEV_NUM           remotePhysicalHwDevNum,
    IN  GT_PHYSICAL_PORT_NUM    remotePhysicalPortNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, remotePhysicalHwDevNum, remotePhysicalPortNum));

    rc = internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet(devNum, portNum, remotePhysicalHwDevNum, remotePhysicalPortNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, remotePhysicalHwDevNum, remotePhysicalPortNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet function
* @endinternal
*
* @brief   Get the mapping of a target physical port to a remote physical port that
*         resides over a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - target device number
* @param[in] portNum                  - target physical port
*
* @param[out] remotePhysicalHwDevNumPtr - (pointer to) the remote physical HW device
* @param[out] remotePhysicalPortNumPtr - (pointer to) the remote physical port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The mapping takes effect only if it's enabled. To check whether it's
*       enabled use cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet
*
*/
static GT_STATUS internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_HW_DEV_NUM           *remotePhysicalHwDevNumPtr,
    OUT GT_PHYSICAL_PORT_NUM    *remotePhysicalPortNumPtr
)
{
    GT_U32      value;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(remotePhysicalHwDevNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(remotePhysicalPortNumPtr);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /*
        do not set 2 fields together , because we not need here logic according
            to sip version.
        the prepoer limit of the 'size' of field is already exists in the
        'write field in entry of table'

        NOTE: in sip6 the remotePhysicalPortNum is 10 bits (not 8)
    */

    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_NUMBER_E,
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);
    if(rc != GT_OK)
    {
        return rc;
    }
    *remotePhysicalPortNumPtr = value;

    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_DEVICE_ID_E,
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);

    *remotePhysicalHwDevNumPtr = value;

    return rc;
}

/**
* @internal cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet function
* @endinternal
*
* @brief   Get the mapping of a target physical port to a remote physical port that
*         resides over a DSA-tagged interface
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - target device number
* @param[in] portNum                  - target physical port
*
* @param[out] remotePhysicalHwDevNumPtr - (pointer to) the remote physical HW device
* @param[out] remotePhysicalPortNumPtr - (pointer to) the remote physical port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The mapping takes effect only if it's enabled. To check whether it's
*       enabled use cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet
*
*/
GT_STATUS cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_HW_DEV_NUM           *remotePhysicalHwDevNumPtr,
    OUT GT_PHYSICAL_PORT_NUM    *remotePhysicalPortNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, remotePhysicalHwDevNumPtr, remotePhysicalPortNumPtr));

    rc = internal_cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet(devNum, portNum, remotePhysicalHwDevNumPtr, remotePhysicalPortNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, remotePhysicalHwDevNumPtr, remotePhysicalPortNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdPortQosDsaModeSet function
* @endinternal
*
* @brief   Configures DSA tag QoS trust mode for cascade port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - cascading port number.
* @param[in] portQosDsaMode           - DSA tag QoS mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port,portQosDsaTrustMode.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChCscdPortQosDsaModeSet
(
    IN GT_U8                         devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT  portQosDsaMode
)
{
    GT_STATUS   rc;
    GT_U32      value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E );

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    switch(portQosDsaMode)
    {
    case CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_GLOBAL_E:
        value = 0;
        break;
    case CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_EXTENDED_E:
        value = 1;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* configure FORWARD eDSA tag received on an ingress cascade port */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_QOS_MODE_E,
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   value);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* FORWARD eDSA tag transmitted on an egress cascade port */
    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_DSA_QOS_MODE_E,
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   value);
}

/**
* @internal cpssDxChCscdPortQosDsaModeSet function
* @endinternal
*
* @brief   Configures DSA tag QoS trust mode for cascade port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - cascading port number.
* @param[in] portQosDsaMode           - DSA tag QoS mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port,portQosDsaTrustMode.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChCscdPortQosDsaModeSet
(
    IN GT_U8                         devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT  portQosDsaMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortQosDsaModeSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portQosDsaMode));

    rc = internal_cpssDxChCscdPortQosDsaModeSet(devNum, portNum, portQosDsaMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portQosDsaMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdPortQosDsaModeGet function
* @endinternal
*
* @brief   Get DSA tag QoS trust mode for cascade port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - cascading port number.
*
* @param[out] portQosDsaTrustModePtr   - pointer to DSA tag QoS trust mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port.
* @retval GT_BAD_PTR               - portQosDsaTrustModePtr is NULL pointer.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChCscdPortQosDsaModeGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT  *portQosDsaTrustModePtr
)
{
    GT_STATUS rc;
    GT_U32   hwValue;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E );

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(portQosDsaTrustModePtr);

    /* read  QoS DSA Mode */
    rc =  prvCpssDxChReadTableEntryField(devNum,
                                         CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                                         portNum,
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                         SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_QOS_MODE_E,
                                         PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                         &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    switch(hwValue)
    {
    case 0:
        *portQosDsaTrustModePtr = CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_GLOBAL_E;
        break;
    case 1:
        *portQosDsaTrustModePtr = CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_EXTENDED_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    return rc;
}

/**
* @internal cpssDxChCscdPortQosDsaModeGet function
* @endinternal
*
* @brief   Get DSA tag QoS trust mode for cascade port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - cascading port number.
*
* @param[out] portQosDsaTrustModePtr   - pointer to DSA tag QoS trust mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum, port.
* @retval GT_BAD_PTR               - portQosDsaTrustModePtr is NULL pointer.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChCscdPortQosDsaModeGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT  *portQosDsaTrustModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortQosDsaModeGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portQosDsaTrustModePtr));

    rc = internal_cpssDxChCscdPortQosDsaModeGet(devNum, portNum, portQosDsaTrustModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portQosDsaTrustModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCscdHashInDsaEnableSet function
* @endinternal
*
* @brief   Enable a packet's hash to be extracted from(rx) and/or inserted into(tx)
*         DSA tag. Relevant for FORWARD DSA tagged packets only.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - port's  (rx/tx/both)
* @param[in] enable                   - GT_TRUE:  enable
*                                      GT_FALSE: disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdHashInDsaEnableSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN CPSS_PORT_DIRECTION_ENT direction,
    IN GT_BOOL                 enable
)
{
    GT_U32    data;
    GT_STATUS rc = GT_OK;

    /* check input parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
          CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);

    if ((GT_U32)direction > CPSS_PORT_DIRECTION_BOTH_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* write registers */
    data = (GT_TRUE == enable)? 1 : 0;

    if (direction == CPSS_PORT_DIRECTION_RX_E ||
        direction == CPSS_PORT_DIRECTION_BOTH_E)
    {
        rc = prvCpssDxChWriteTableEntryField(
            devNum,
            CPSS_DXCH_SIP5_20_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_2_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            250, 1, /* Extract Hash from FORWARD eDSA */
            data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (direction == CPSS_PORT_DIRECTION_TX_E ||
        direction == CPSS_PORT_DIRECTION_BOTH_E)
    {
        rc = prvCpssDxChWriteTableEntryField(
            devNum,
            CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INSERT_HASH_INTO_FORWARD_DSA_ENABLE_E,
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}


/**
* @internal cpssDxChCscdHashInDsaEnableSet function
* @endinternal
*
* @brief   Enable a packet's hash to be extracted from(rx) and/or inserted into(tx)
*         DSA tag. Relevant for FORWARD DSA tagged packets only.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - port's  (rx/tx/both)
* @param[in] enable                   - GT_TRUE:  enable
*                                      GT_FALSE: disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdHashInDsaEnableSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN CPSS_PORT_DIRECTION_ENT direction,
    IN GT_BOOL                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdHashInDsaEnableSet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, enable));

    rc = internal_cpssDxChCscdHashInDsaEnableSet(devNum, portNum,
                                                 direction, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdHashInDsaEnableGet function
* @endinternal
*
* @brief   Get value of flag determining whether a packet's hash should be
*         extracted from(rx)/inserted info(tx) DSA tag.
*         Relevand for FORWARD DSA tagged packets only.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - port's  (rx or tx. Not both!)
*
* @param[out] enablePtr                - (pointer to) flag value
*                                      GT_TRUE:  use DSA to extract/store hash.
*                                      GT_FALSE: don't use DSA to extract/store hash.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCscdHashInDsaEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_U32 data  = 0;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
          CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    switch (direction) {
        case CPSS_PORT_DIRECTION_RX_E:
            rc = prvCpssDxChReadTableEntryField(
                devNum,
                CPSS_DXCH_SIP5_20_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_2_E,
                portNum,
                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                250, 1, /* Extract Hash from FORWARD eDSA */
                &data);
            break;

        case CPSS_PORT_DIRECTION_TX_E:
            rc = prvCpssDxChReadTableEntryField(
                devNum,
                CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                portNum,
                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INSERT_HASH_INTO_FORWARD_DSA_ENABLE_E,
                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                &data);
            break;

        default:
            rc = GT_BAD_PARAM;
    }
    if (GT_OK == rc)
    {
        *enablePtr = (data == 0 ? GT_FALSE : GT_TRUE);
    }
    return rc;
}


/**
* @internal cpssDxChCscdHashInDsaEnableGet function
* @endinternal
*
* @brief   Get value of flag determining whether a packet's hash should be
*         extracted from(rx)/inserted info(tx) DSA tag.
*         Relevand for FORWARD DSA tagged packets only.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] direction                - port's  (rx or tx. Not both!)
*
* @param[out] enablePtr                - (pointer to) flag value
*                                      GT_TRUE:  use DSA to extract/store hash.
*                                      GT_FALSE: don't use DSA to extract/store hash.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdHashInDsaEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdHashInDsaEnableGet);

   CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, enablePtr));

    rc = internal_cpssDxChCscdHashInDsaEnableGet(devNum, portNum,
                                                 direction, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdPortTcProfiletSet function
* @endinternal
*
* @brief   Sets port TC profile on source/target port.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - port's direction.
* @param[in] portProfile              - port's profile values.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChCscdPortTcProfiletSet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    IN    CPSS_PORT_DIRECTION_ENT    portDirection,
    IN    CPSS_DXCH_PORT_PROFILE_ENT portProfile
)
{
    GT_STATUS   rc;
    GT_U32      hwValue;
    GT_U32      doSourcePort;
    GT_U32      doTargetPort;

    /* check input parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
          CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PORT_TC_PROFILE_CONVERT_MAC(portProfile, hwValue);

    rc = prvCpssDxChPortProfileValidate(devNum,portNum,portDirection,portProfile);
    if (rc != GT_OK)
    {
      return rc;
    }

    switch(portDirection)
    {
        case CPSS_PORT_DIRECTION_RX_E:
            doSourcePort = 1;
            doTargetPort = 0;
            break;
        case CPSS_PORT_DIRECTION_TX_E:
            doSourcePort = 0;
            doTargetPort = 1;
            break;
        case CPSS_PORT_DIRECTION_BOTH_E:
            doSourcePort = 1;
            doTargetPort = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (doSourcePort == 1)
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                    CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E,
                    portNum,
                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                    SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_TC_PROFILE_E, /* field name */
                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                    hwValue);

        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (doTargetPort == 1)
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                    CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E,
                    portNum,
                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                    SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ETARGET_PHYSICAL_PORT_TC_PROFILE_E, /* field name */
                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                    hwValue);

        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChCscdPortTcProfiletSet function
* @endinternal
*
* @brief   Sets port TC profile on source/target port.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - port's direction.
* @param[in] portProfile              - port's profile values.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdPortTcProfiletSet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    IN    CPSS_PORT_DIRECTION_ENT    portDirection,
    IN    CPSS_DXCH_PORT_PROFILE_ENT portProfile
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortTcProfiletSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portDirection, portProfile));

    rc = internal_cpssDxChCscdPortTcProfiletSet(devNum, portNum, portDirection, portProfile);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portDirection, portProfile));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChCscdPortTcProfiletGet function
* @endinternal
*
* @brief   Gets port TC profile on source/target port.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - port's direction.
*
* @param[out] portProfilePtr           - (pointer to) port's profile values.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChCscdPortTcProfiletGet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    IN    CPSS_PORT_DIRECTION_ENT    portDirection,
    OUT   CPSS_DXCH_PORT_PROFILE_ENT * portProfilePtr
)
{
    GT_STATUS rc;
    GT_U32 hwValue;
    GT_U32 doSourcePort;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
          CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);

    CPSS_NULL_PTR_CHECK_MAC(portProfilePtr);

    switch(portDirection)
    {
        case CPSS_PORT_DIRECTION_RX_E:
            doSourcePort = 1;
            break;
        case CPSS_PORT_DIRECTION_TX_E:
            doSourcePort = 0;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (doSourcePort)
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                    CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E,
                    portNum,
                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                    SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_TC_PROFILE_E, /* field name */
                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                    &hwValue);

        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                    CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E,
                    portNum,
                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                    SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ETARGET_PHYSICAL_PORT_TC_PROFILE_E, /* field name */
                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                    &hwValue);

        if (rc != GT_OK)
        {
            return rc;
        }
    }

    PRV_CPSS_DXCH_PORT_HW_PROFILE_CONVERT_MAC(hwValue, *portProfilePtr);

    return GT_OK;
}

/**
* @internal cpssDxChCscdPortTcProfiletGet function
* @endinternal
*
* @brief   Gets port TC profile on source/target port.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - port's direction.
*
* @param[out] portProfilePtr           - (pointer to) port's profile values.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdPortTcProfiletGet
(
    IN    GT_U8                      devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    IN    CPSS_PORT_DIRECTION_ENT    portDirection,
    OUT   CPSS_DXCH_PORT_PROFILE_ENT * portProfilePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortTcProfiletGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portDirection, portProfilePtr));

    rc = internal_cpssDxChCscdPortTcProfiletGet(devNum, portNum, portDirection, portProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portDirection, portProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet function
* @endinternal
*
* @brief   Configure port mode regarding skipping of SA lookup.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] acceptSkipSaLookup    - Port's skip SA lookup mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet
(
    IN    GT_U8                   devNum,
    IN    GT_PHYSICAL_PORT_NUM    portNum,
    IN    GT_BOOL                 acceptSkipSaLookup
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      hwValue;    /* value to write to register   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E |
         CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);

    hwValue = BOOL2BIT_MAC(acceptSkipSaLookup);

    /* write to TTI-Physical-Port-Attribute table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
            portNum,/*global port*/
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_ACCEPT_EDSA_SKIP_FDB_SA_LOOKUP_FIELD_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            hwValue);

    return rc;
}

/**
* @internal cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet function
* @endinternal
*
* @brief   Configure port mode regarding skipping of SA lookup.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] acceptSkipSaLookup    - Port's skip SA lookup mode.
*                                  - GT_TRUE:  Do not accept the <Skip FDB SA lookup> field in the eDSA tag
*                                  - GT_FALSE: Accept the <Skip FDB SA lookup> field in the eDSA tag
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet
(
    IN    GT_U8                   devNum,
    IN    GT_PHYSICAL_PORT_NUM    portNum,
    IN    GT_BOOL                 acceptSkipSaLookup
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, acceptSkipSaLookup));

    rc = internal_cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet(devNum, portNum, acceptSkipSaLookup);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, acceptSkipSaLookup));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet function
* @endinternal
*
* @brief   Gets port mode regarding skipping of SA lookup.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @param[out] acceptSkipSaLookup   - (pointer to) port's skip SA lookup mode.
*                                  - GT_TRUE:  Do not accept the <Skip FDB SA lookup> field in the eDSA tag
*                                  - GT_FALSE: Accept the <Skip FDB SA lookup> field in the eDSA tag
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet
(
    IN    GT_U8                   devNum,
    IN    GT_PHYSICAL_PORT_NUM    portNum,
    OUT   GT_BOOL                 *acceptSkipSaLookupPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      hwValue;
    CPSS_NULL_PTR_CHECK_MAC(acceptSkipSaLookupPtr);

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /* read from TTI-Physical-Port-Attribute table */
    rc = prvCpssDxChReadTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
            portNum,/*global port*/
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_ACCEPT_EDSA_SKIP_FDB_SA_LOOKUP_FIELD_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            &hwValue);

    *acceptSkipSaLookupPtr = BIT2BOOL_MAC(hwValue);
    return rc;
}

/**
* @internal cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet function
* @endinternal
*
* @brief   Gets port mode regarding skipping of SA lookup.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @param[out] acceptSkipSaLookup   - (pointer to) port's skip SA lookup mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet
(
    IN    GT_U8                   devNum,
    IN    GT_PHYSICAL_PORT_NUM    portNum,
    OUT   GT_BOOL                 *acceptSkipSaLookupPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, acceptSkipSaLookupPtr));

    rc = internal_cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet(devNum, portNum, acceptSkipSaLookupPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, acceptSkipSaLookupPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCscdPortForce4BfromCpuDsaEnableSet function
* @endinternal
*
* @brief   Enables/disables the forced 4B FROM_CPU DSA tag in FORWARD, TO_ANALYZER and FROM_CPU packets.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE  - enable forced 4B FROM_CPU DSA tag
*                                     - GT_FALSE - disable forced 4B FROM_CPU DSA tag
*
* @retval GT_OK                       - on success
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - wrong value in any of the parameter
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
*/
GT_STATUS internal_cpssDxChCscdPortForce4BfromCpuDsaEnableSet
(
    IN    GT_U8                       devNum,
    IN    GT_PHYSICAL_PORT_NUM        portNum,
    IN    GT_BOOL                     enable
)
{
    GT_STATUS   rc;
    GT_U32      value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);

    value = BOOL2BIT_MAC(enable);

    rc = prvCpssDxChWriteTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FWD_TO_4B_FROM_CPU_DSA_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        value);

    if(rc!= GT_OK)
    {
        return rc;

    }
    rc = prvCpssDxChWriteTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_TO_ANALYZER_TO_4B_FROM_CPU_DSA_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        value);

    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChWriteTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FROM_CPU_TO_4B_FROM_CPU_DSA_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        value);

    return rc;

}

/**
* @internal cpssDxChCscdPortForce4BfromCpuDsaEnableSet function
* @endinternal
*
* @brief   Enables/disables the forced 4B FROM_CPU DSA tag in FORWARD, TO_ANALYZER and FROM_CPU packets.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE  - enable forced 4B FROM_CPU DSA tag
*                                     - GT_FALSE - disable forced 4B FROM_CPU DSA tag
*
* @retval GT_OK                       - on success
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - wrong value in any of the parameter
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
*/
GT_STATUS cpssDxChCscdPortForce4BfromCpuDsaEnableSet
(
    IN    GT_U8                       devNum,
    IN    GT_PHYSICAL_PORT_NUM        portNum,
    IN    GT_BOOL                     enable
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortForce4BfromCpuDsaEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChCscdPortForce4BfromCpuDsaEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;

}


/**
* @internal internal_cpssDxChCscdPortForce4BfromCpuDsaEnableGet function
* @endinternal
*
* @brief   Get the state of the forced 4B FROM_CPU DSA tagged packets.
*
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                 - device number
* @param[in] portNum                - port number
*
* @param[out] enable                - (pointer to) state of the forced 4B FROM_CPU DSA tag
*                                     GT_TRUE  - enable forced 4B FROM_CPU DSA tag
*                                     GT_FALSE - disable forced 4B FROM_CPU DSA tag
*
* @retval GT_OK                     - on success
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_BAD_PTR                - on NULL pointer
* @retval GT_BAD_STATE              - illegal state
* @retval GT_BAD_PARAM              - wrong value in any of the parameter
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*
*/

GT_STATUS internal_cpssDxChCscdPortForce4BfromCpuDsaEnableGet
(
    IN    GT_U8                       devNum,
    IN    GT_PHYSICAL_PORT_NUM        portNum,
    OUT   GT_BOOL                    *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32      enableFwd;
    GT_U32      enableToAnalyzer;
    GT_U32      enableFromCpu;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    rc = prvCpssDxChReadTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FWD_TO_4B_FROM_CPU_DSA_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        &enableFwd);

    if(rc!= GT_OK)
    {
        return rc;

    }

    rc = prvCpssDxChReadTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_TO_ANALYZER_TO_4B_FROM_CPU_DSA_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        &enableToAnalyzer);

    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChReadTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FROM_CPU_TO_4B_FROM_CPU_DSA_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        &enableFromCpu);

    if (rc != GT_OK)
    {
        return rc;
    }

    /* Check whether all the three bits are enabled/diabled)*/
    if((enableFwd==enableToAnalyzer) && (enableFwd==enableFromCpu))
    {
        *enablePtr = BOOL2BIT_MAC(enableFwd);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChCscdPortForce4BfromCpuDsaEnableGet function
* @endinternal
*
* @brief   Get the state of the forced 4B FROM_CPU DSA tagged packets.
*
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                 - device number
* @param[in] portNum                - port number
*
* @param[out] enable                - (pointer to) state of the forced 4B FROM_CPU DSA tag
*                                     GT_TRUE  - enable forced 4B FROM_CPU DSA tag
*                                     GT_FALSE - disable forced 4B FROM_CPU DSA tag
*
* @retval GT_OK                     - on success
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_BAD_PTR                - on NULL pointer
* @retval GT_BAD_STATE              - illegal state
* @retval GT_BAD_PARAM              - wrong value in any of the parameter
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*
*/

GT_STATUS cpssDxChCscdPortForce4BfromCpuDsaEnableGet
(
    IN    GT_U8                       devNum,
    IN    GT_PHYSICAL_PORT_NUM        portNum,
    OUT   GT_BOOL                    *enablePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCscdPortForce4BfromCpuDsaEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChCscdPortForce4BfromCpuDsaEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;

}

