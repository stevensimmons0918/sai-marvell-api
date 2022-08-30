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
* @file cpssDxChNst.c
*
* @brief Function implementation for Network Shield Technology configuration.
*
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/nst/private/prvCpssDxChNstLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNst.h>
/* check errata */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChErrataMng.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_CPSS_DXCH_NST_MAC_ACCESS_LEVELS_MAX_NUM_CNS (8)
#define PRV_CPSS_SIP5_NST_MAC_ROUTER_ACCESS_LEVELS_MAX_NUM_CNS (64)

/* check access level validity */
#define PRV_CPSS_DXCH2_NST_CHECK_MAC_ACCESS_LEVEL_MAC(accessLevel)        \
    if((accessLevel) >= PRV_CPSS_DXCH_NST_MAC_ACCESS_LEVELS_MAX_NUM_CNS)  \
    {                                                                     \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                              \
    }

#define PRV_CPSS_SIP5_NST_ROUTER_CHECK_MAC_ACCESS_LEVEL_MAC(accessLevel)        \
    if((accessLevel) >= PRV_CPSS_SIP5_NST_MAC_ROUTER_ACCESS_LEVELS_MAX_NUM_CNS)  \
    {                                                                     \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                              \
    }

/* set default access level field offset according to the type */
#define PRV_CPSS_DXCH2_NST_DEFAULT_ACCESS_LEVEL_FIELD_OFFSET_SET_MAC(        \
    _paramType, _fieldOffset)                                                \
    switch (_paramType)                                                      \
    {                                                                        \
       case CPSS_NST_AM_SA_AUTO_LEARNED_E:                                   \
           _fieldOffset = 8;                                                 \
           break;                                                            \
       case CPSS_NST_AM_DA_AUTO_LEARNED_E:                                   \
           _fieldOffset = 12;                                                \
           break;                                                            \
       case CPSS_NST_AM_SA_UNKNOWN_E:                                        \
           _fieldOffset = 0;                                                 \
           break;                                                            \
       case CPSS_NST_AM_DA_UNKNOWN_E:                                        \
           _fieldOffset = 4;                                                 \
           break;                                                            \
       default:                                                              \
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                              \
    }

/* set sanity check field offset and index for register address setting
   according sanity check type */
#define PRV_CPSS_DXCH2_NST_SANITY_CHECK_TYPE_FIELD_OFFSET_SET_MAC(           \
    _checkType, _index, _fieldOffset)                                        \
    switch(_checkType)                                                       \
    {                                                                        \
        case CPSS_NST_CHECK_TCP_SYN_DATA_E:                                  \
            _fieldOffset = 29;                                               \
            _index = 0;                                                      \
            break;                                                           \
        case CPSS_NST_CHECK_TCP_OVER_MAC_MC_BC_E:                            \
            _fieldOffset = 17;                                               \
            _index = 0;                                                      \
            break;                                                           \
        case CPSS_NST_CHECK_TCP_FLAG_ZERO_E:                                   \
            _fieldOffset = 4;                                                \
            _index = 3;                                                      \
            break;                                                           \
        case CPSS_NST_CHECK_TCP_FLAGS_FIN_URG_PSH_E:                         \
            _fieldOffset = 3;                                                \
            _index = 3;                                                      \
            break;                                                           \
        case CPSS_NST_CHECK_TCP_FLAGS_SYN_FIN_E:                             \
            _fieldOffset = 2;                                                \
            _index = 3;                                                      \
            break;                                                           \
        case CPSS_NST_CHECK_TCP_FLAGS_SYN_RST_E:                             \
            _fieldOffset = 1;                                                \
            _index = 3;                                                      \
            break;                                                           \
        case CPSS_NST_CHECK_TCP_UDP_PORT_ZERO_E:                             \
            _fieldOffset = 0;                                                \
            _index = 3;                                                      \
            break;                                                           \
        case CPSS_NST_CHECK_FRAG_IPV4_ICMP_E:                                \
            _fieldOffset = 5;                                                \
            _index = 3;                                                      \
            break;                                                           \
        case CPSS_NST_CHECK_ARP_MAC_SA_MISMATCH_E:                           \
            _fieldOffset = 16;                                               \
            _index = 0;                                                      \
            break;                                                           \
        default:                                                             \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                             \
    }

/* set sanity check field offset and index for register address setting
   according sanity check type */
#define PRV_CPSS_SIP5_NST_SANITY_CHECK_TYPE_FIELD_OFFSET_SET_MAC(           \
    _checkType, _regAddr, _fieldOffset)                                      \
    switch(_checkType)                                                       \
    {                                                                        \
        case CPSS_NST_CHECK_TCP_SYN_DATA_E:                                  \
            _fieldOffset = 18;                                               \
            _regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).         \
                bridgeEngineConfig.bridgeCommandConfig3;                     \
            break;                                                           \
        case CPSS_NST_CHECK_TCP_OVER_MAC_MC_BC_E:                            \
            _fieldOffset = 15;                                               \
            _regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).         \
                bridgeEngineConfig.bridgeCommandConfig3;                     \
            break;                                                           \
        case CPSS_NST_CHECK_TCP_FLAG_ZERO_E:                                 \
            _fieldOffset = 12;                                               \
            _regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).         \
                bridgeEngineConfig.bridgeCommandConfig3;                     \
            break;                                                           \
        case CPSS_NST_CHECK_TCP_FLAGS_FIN_URG_PSH_E:                         \
            _fieldOffset = 9;                                                \
            _regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).         \
                bridgeEngineConfig.bridgeCommandConfig3;                     \
            break;                                                           \
        case CPSS_NST_CHECK_TCP_FLAGS_SYN_FIN_E:                             \
            _fieldOffset = 6;                                                \
            _regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).         \
                bridgeEngineConfig.bridgeCommandConfig3;                     \
            break;                                                           \
        case CPSS_NST_CHECK_TCP_FLAGS_SYN_RST_E:                             \
            _fieldOffset = 3;                                                \
            _regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).         \
                bridgeEngineConfig.bridgeCommandConfig3;                     \
            break;                                                           \
        case CPSS_NST_CHECK_TCP_UDP_PORT_ZERO_E:                             \
            _fieldOffset = 0;                                                \
            _regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).         \
                bridgeEngineConfig.bridgeCommandConfig3;                     \
            break;                                                           \
        case CPSS_NST_CHECK_FRAG_IPV4_ICMP_E:                                \
            _fieldOffset = 0;                                                \
            _regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).         \
                bridgeEngineConfig.bridgeCommandConfig1;                     \
            break;                                                           \
        case CPSS_NST_CHECK_ARP_MAC_SA_MISMATCH_E:                           \
            _fieldOffset = 21;                                               \
            _regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).         \
                bridgeEngineConfig.bridgeCommandConfig0;                     \
            break;                                                           \
        case CPSS_NST_CHECK_TCP_WITHOUT_FULL_HEADER_E:                       \
            _fieldOffset = 24;                                               \
            _regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).         \
                bridgeEngineConfig.bridgeCommandConfig3;                     \
            break;                                                           \
        case CPSS_NST_CHECK_TCP_FIN_WITHOUT_ACK_E:                           \
            _fieldOffset = 27;                                               \
            _regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).         \
                bridgeEngineConfig.bridgeCommandConfig3;                     \
            break;                                                           \
        case CPSS_NST_CHECK_SIP_IS_DIP_E:                                    \
            _fieldOffset = 6;                                                \
            _regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).         \
                bridgeEngineConfig.bridgeCommandConfig2;                     \
            break;                                                           \
        case CPSS_NST_CHECK_TCP_SPORT_IS_DPORT_E:                            \
            _fieldOffset = 21;                                               \
            _regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).         \
                bridgeEngineConfig.bridgeCommandConfig3;                     \
            break;                                                           \
        default:                                                             \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                             \
    }

/* set ingress forward filter register address according to filter type */
#define PRV_CPSS_DXCH2_NST_PORT_INGRESS_FRW_FILTER_REG_ADDRESS_SET_MAC(      \
    _devNum, _filterType, _regAddr)                                          \
   switch(_filterType)                                                       \
   {                                                                         \
       case CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E:                        \
           _regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(_devNum)->                  \
                             bridgeRegs.nstRegs.netIngrFrwFltConfReg;        \
           break;                                                            \
       case CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E:                            \
           _regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(_devNum)->                  \
                             bridgeRegs.nstRegs.cpuIngrFrwFltConfReg;        \
           break;                                                            \
       case CPSS_NST_INGRESS_FRW_FILTER_TO_ANALYZER_E:                       \
           _regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(_devNum)->                  \
                             bridgeRegs.nstRegs.analyzerIngrFrwFltConfReg;   \
           break;                                                            \
       default:                                                              \
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                              \
   }


/* set egress forward filter register address according to filter type */
#define PRV_CPSS_DXCH2_NST_PORT_EGRESS_FRW_FILTER_REG_ADDRESS_SET_MAC(         \
    _devNum, _filterType, _regAddr)                                            \
   switch(_filterType)                                                         \
   {                                                                           \
       case CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E:                             \
             _regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(_devNum)->                  \
                                     bridgeRegs.nstRegs.cpuPktsFrwFltConfReg;  \
            break;                                                             \
       case CPSS_NST_EGRESS_FRW_FILTER_BRIDGED_E:                              \
            _regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(_devNum)->                   \
                                     bridgeRegs.nstRegs.brgPktsFrwFltConfReg;  \
            break;                                                             \
       case CPSS_NST_EGRESS_FRW_FILTER_ROUTED_E:                               \
            _regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(_devNum)->                   \
                                     bridgeRegs.nstRegs.routePktsFrwFltConfReg;\
            break;                                                             \
       default:                                                                \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                               \
   }

/* for TXQ ver 1 : get egress forward filter register address and bit offset
    according to filter type , and port */
#define TXQ_DISTIBUTER_FORWARD_RESTRICTED_FILTER_REG_ADDR_AND_BIT_MAC(         \
    _devNum, _portNum,_filterType, _regAddr,_bitOffset)                        \
{ GT_U32 _regIdx = OFFSET_TO_WORD_MAC(_portNum);                               \
    if (_regIdx >= PORTS_BMP_NUM_SIP_4_CNS) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);              \
                                                                               \
   _bitOffset = OFFSET_TO_BIT_MAC(_portNum);                                   \
   switch(_filterType)                                                         \
   {                                                                           \
       case CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E:                             \
             _regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(_devNum)->                  \
                        txqVer1.queue.distributor.                             \
                        fromCpuForwardRestricted[_regIdx];                     \
            break;                                                             \
       case CPSS_NST_EGRESS_FRW_FILTER_BRIDGED_E:                              \
            _regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(_devNum)->                   \
                        txqVer1.queue.distributor.                             \
                        bridgedForwardRestricted[_regIdx];                     \
            break;                                                             \
       case CPSS_NST_EGRESS_FRW_FILTER_ROUTED_E:                               \
            _regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(_devNum)->                   \
                        txqVer1.queue.distributor.                             \
                        routedForwardRestricted[_regIdx];                      \
            break;                                                             \
       default:                                                                \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);     \
   }                                                                           \
}


/**
* @internal prvCpssDxChNstAccessMatrixCmdSet function
* @endinternal
*
* @brief   Set bridge/ip access matrix entry.
*         The packet is assigned two access levels based on its MAC SA/SIP, MAC
*         DA/DIP or FCOE S_ID/D_ID.
*         The Access Matrix Configuration tables controls which access level
*         pairs can communicate with each other. Based on the access level pair,
*         the packet is assigned a command of: FORWARD, HARD DROP, and SOFT DROP.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - physical device number
* @param[in] saAccessLevel            - Source address access level
* @param[in] daAccessLevel            - Destination address access level
* @param[in] command                  -  assigned to a packet
* @param[in] isBridge                 - whether to set the bridge or the ip access matrix.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, saAccessLevel, daAccessLevel,
*                                       command.
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChNstAccessMatrixCmdSet
(
    IN GT_U8                       devNum,
    IN GT_U32                      saAccessLevel,
    IN GT_U32                      daAccessLevel,
    IN CPSS_PACKET_CMD_ENT         command,
    IN GT_BOOL                     isBridge
)
{
    GT_U32    matrixAccessRegAddr;/* register address */
    GT_U32    hwValueOfCmd;       /* Hardware value of command  */
    GT_STATUS rc;                 /* return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    switch(command)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            hwValueOfCmd = 0;
            break;
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            hwValueOfCmd = 1;
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E:
            hwValueOfCmd = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (isBridge == GT_TRUE)
    {
        PRV_CPSS_DXCH2_NST_CHECK_MAC_ACCESS_LEVEL_MAC(saAccessLevel);
        PRV_CPSS_DXCH2_NST_CHECK_MAC_ACCESS_LEVEL_MAC(daAccessLevel);

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(hwValueOfCmd,command);

            matrixAccessRegAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                                bridgeAccessMatrix.bridgeAccessMatrixLine[saAccessLevel];

            /* Set bridge access matrix entry */
            rc =  prvCpssHwPpSetRegField(devNum,matrixAccessRegAddr,
                                      daAccessLevel * 3, 3, hwValueOfCmd);

            return rc;
        }
        else
        {
            matrixAccessRegAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                               bridgeRegs.nstRegs.brgAccessMatrix;
        }
    }
    else
    {
        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* backward compatibility:
               Write to IPv4, Ipv6, FCoE table */
            PRV_CPSS_SIP5_NST_ROUTER_CHECK_MAC_ACCESS_LEVEL_MAC(saAccessLevel);
            PRV_CPSS_SIP5_NST_ROUTER_CHECK_MAC_ACCESS_LEVEL_MAC(daAccessLevel);

            /* IPv4 */
            rc = prvCpssDxChWriteTableEntryField(devNum,
                                                 CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_ACCESS_MATRIX_E,
                                                 saAccessLevel,
                                                 PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                                 2 * daAccessLevel,
                                                 2,
                                                 hwValueOfCmd);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* IPv6 */
            rc = prvCpssDxChWriteTableEntryField(devNum,
                                                 CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_ACCESS_MATRIX_E,
                                                 64 + saAccessLevel,
                                                 PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                                 2 * daAccessLevel,
                                                 2,
                                                 hwValueOfCmd);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* FCoE */
            rc = prvCpssDxChWriteTableEntryField(devNum,
                                                 CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_ACCESS_MATRIX_E,
                                                 128 + saAccessLevel,
                                                 PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                                 2 * daAccessLevel,
                                                 2,
                                                 hwValueOfCmd);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* FCoE */
            rc = prvCpssDxChWriteTableEntryField(devNum,
                                                 CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_ACCESS_MATRIX_E,
                                                 128 + saAccessLevel,
                                                 PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                                 2 * daAccessLevel,
                                                 2,
                                                 hwValueOfCmd);
            return rc;
        }
        else
        {
            PRV_CPSS_DXCH2_NST_CHECK_MAC_ACCESS_LEVEL_MAC(saAccessLevel);
            PRV_CPSS_DXCH2_NST_CHECK_MAC_ACCESS_LEVEL_MAC(daAccessLevel);

            matrixAccessRegAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                               ipRegs.routerAccessMatrixBase;
        }
    }

    /* Set bridge access matrix entry */
    return prvCpssHwPpSetRegField(devNum,
                             (matrixAccessRegAddr + saAccessLevel * 0x4),
                              daAccessLevel * 2, 2, hwValueOfCmd);
}

/**
* @internal prvCpssDxChNstAccessMatrixCmdGet function
* @endinternal
*
* @brief   Get bridge/ip access matrix entry.
*         The packet is assigned two access levels based on its MAC SA/SIP and MAC
*         DA/DIP.
*         The Access Matrix Configuration tables controls which access level
*         pairs can communicate with each other. Based on the access level pair,
*         the packet is assigned a command of: FORWARD, HARD DROP, and SOFT DROP.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - physical device number
* @param[in] saAccessLevel            - Source address access level
* @param[in] daAccessLevel            - Destination address access level
* @param[in] isBridge                 - weather to set the bridge or the ip access matrix.
*
* @param[out] commandPtr               - command assigned to a packet
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, saAccessLevel, daAccessLevel.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvCpssDxChNstAccessMatrixCmdGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      saAccessLevel,
    IN  GT_U32                      daAccessLevel,
    OUT CPSS_PACKET_CMD_ENT         *commandPtr,
    IN  GT_BOOL                     isBridge
)
{
    GT_U32    matrixAccessRegAddr;/* register address */
    GT_U32    hwValueOfCmd;       /* Hardware value of command  */
    GT_STATUS rc;                 /* return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(commandPtr);

    matrixAccessRegAddr = 0;    /* prevent compiler warning */
    if (isBridge == GT_TRUE)
    {
        PRV_CPSS_DXCH2_NST_CHECK_MAC_ACCESS_LEVEL_MAC(saAccessLevel);
        PRV_CPSS_DXCH2_NST_CHECK_MAC_ACCESS_LEVEL_MAC(daAccessLevel);

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            matrixAccessRegAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                                bridgeAccessMatrix.bridgeAccessMatrixLine[saAccessLevel];

            /* Set bridge access matrix entry */
            rc =  prvCpssHwPpGetRegField(devNum,matrixAccessRegAddr,
                                      daAccessLevel * 3, 3, &hwValueOfCmd);
            if(rc != GT_OK)
                return rc;

            PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC((*commandPtr),hwValueOfCmd);

            return GT_OK;
        }
        else
        {
            matrixAccessRegAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                           bridgeRegs.nstRegs.brgAccessMatrix;
        }
    }
    else
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
        {
            PRV_CPSS_DXCH2_NST_CHECK_MAC_ACCESS_LEVEL_MAC(saAccessLevel);
            PRV_CPSS_DXCH2_NST_CHECK_MAC_ACCESS_LEVEL_MAC(daAccessLevel);

            matrixAccessRegAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                               ipRegs.routerAccessMatrixBase;
        }
        else
        {
           PRV_CPSS_SIP5_NST_ROUTER_CHECK_MAC_ACCESS_LEVEL_MAC(saAccessLevel);
           PRV_CPSS_SIP5_NST_ROUTER_CHECK_MAC_ACCESS_LEVEL_MAC(daAccessLevel);
        }
    }

    /* Get bridge access matrix entry */
    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        /* Get bridge access matrix entry */
        rc = prvCpssHwPpGetRegField(devNum,
                                 (matrixAccessRegAddr + saAccessLevel * 0x4),
                                  daAccessLevel * 2, 2, &hwValueOfCmd);
        if(rc != GT_OK)
            return rc;
    }
    else    /* eArch */
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_ACCESS_MATRIX_E,
                                            saAccessLevel,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            2 * daAccessLevel,
                                            2,
                                            &hwValueOfCmd);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    switch(hwValueOfCmd)
    {
        case 0:
            *commandPtr = CPSS_PACKET_CMD_FORWARD_E;
            break;
        case 1:
            *commandPtr = CPSS_PACKET_CMD_DROP_SOFT_E;
            break;
        case 2:
            *commandPtr = CPSS_PACKET_CMD_DROP_HARD_E;
            break;
        default:
            *commandPtr = hwValueOfCmd;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChNstBridgeAccessMatrixCmdSet function
* @endinternal
*
* @brief   Set bridge access matrix entry.
*         The packet is assigned two access levels based on its MAC SA/SIP, MAC
*         DA/DIP or FCOE S_ID/D_ID.
*         The device supports up to 8 SA and up to 8 DA levels.
*         The Access Matrix Configuration tables controls which access level
*         pairs can communicate with each other. Based on the access level pair,
*         the packet is assigned a command of: FORWARD, HARD DROP, and SOFT DROP.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] saAccessLevel            - Source address access level
* @param[in] daAccessLevel            - Destination address access level
* @param[in] command                  -  assigned to a packet
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, saAccessLevel, daAccessLevel.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstBridgeAccessMatrixCmdSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      saAccessLevel,
    IN  GT_U32                      daAccessLevel,
    IN  CPSS_PACKET_CMD_ENT         command
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    return prvCpssDxChNstAccessMatrixCmdSet(devNum,saAccessLevel,daAccessLevel,
                                            command,GT_TRUE);

}

/**
* @internal cpssDxChNstBridgeAccessMatrixCmdSet function
* @endinternal
*
* @brief   Set bridge access matrix entry.
*         The packet is assigned two access levels based on its MAC SA/SIP, MAC
*         DA/DIP or FCOE S_ID/D_ID.
*         The device supports up to 8 SA and up to 8 DA levels.
*         The Access Matrix Configuration tables controls which access level
*         pairs can communicate with each other. Based on the access level pair,
*         the packet is assigned a command of: FORWARD, HARD DROP, and SOFT DROP.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] saAccessLevel            - Source address access level
* @param[in] daAccessLevel            - Destination address access level
* @param[in] command                  -  assigned to a packet
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, saAccessLevel, daAccessLevel.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstBridgeAccessMatrixCmdSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      saAccessLevel,
    IN  GT_U32                      daAccessLevel,
    IN  CPSS_PACKET_CMD_ENT         command
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstBridgeAccessMatrixCmdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, saAccessLevel, daAccessLevel, command));

    rc = internal_cpssDxChNstBridgeAccessMatrixCmdSet(devNum, saAccessLevel, daAccessLevel, command);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, saAccessLevel, daAccessLevel, command));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstBridgeAccessMatrixCmdGet function
* @endinternal
*
* @brief   Get bridge access matrix entry.
*         The packet is assigned two access levels based on its MAC SA/SIP and MAC
*         DA/DIP.
*         The device supports up to 8 SA and up to 8 DA levels.
*         The Access Matrix Configuration tables controls which access level
*         pairs can communicate with each other. Based on the access level pair,
*         the packet is assigned a command of: FORWARD, HARD DROP, and SOFT DROP.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] saAccessLevel            - Source address access level
* @param[in] daAccessLevel            - Destination address access level
*
* @param[out] commandPtr               - command assigned to a packet
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, saAccessLevel, daAccessLevel.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstBridgeAccessMatrixCmdGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      saAccessLevel,
    IN  GT_U32                      daAccessLevel,
    OUT CPSS_PACKET_CMD_ENT         *commandPtr
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    return prvCpssDxChNstAccessMatrixCmdGet(devNum,saAccessLevel,daAccessLevel,
                                            commandPtr,GT_TRUE);
}

/**
* @internal cpssDxChNstBridgeAccessMatrixCmdGet function
* @endinternal
*
* @brief   Get bridge access matrix entry.
*         The packet is assigned two access levels based on its MAC SA/SIP and MAC
*         DA/DIP.
*         The device supports up to 8 SA and up to 8 DA levels.
*         The Access Matrix Configuration tables controls which access level
*         pairs can communicate with each other. Based on the access level pair,
*         the packet is assigned a command of: FORWARD, HARD DROP, and SOFT DROP.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] saAccessLevel            - Source address access level
* @param[in] daAccessLevel            - Destination address access level
*
* @param[out] commandPtr               - command assigned to a packet
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, saAccessLevel, daAccessLevel.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstBridgeAccessMatrixCmdGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      saAccessLevel,
    IN  GT_U32                      daAccessLevel,
    OUT CPSS_PACKET_CMD_ENT         *commandPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstBridgeAccessMatrixCmdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, saAccessLevel, daAccessLevel, commandPtr));

    rc = internal_cpssDxChNstBridgeAccessMatrixCmdGet(devNum, saAccessLevel, daAccessLevel, commandPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, saAccessLevel, daAccessLevel, commandPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstDefaultAccessLevelsSet function
* @endinternal
*
* @brief   Set default access levels for Bridge Access Matrix Configuration table
*         that controls which access level pairs can communicate with each other.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] paramType                -   type of bridge acess matrix global parameter to
*                                      configure default access level .
* @param[in] accessLevel              -  default security access level for parameter
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum,paramType,accessLevel
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstDefaultAccessLevelsSet
(
    IN GT_U8                    devNum,
    IN CPSS_NST_AM_PARAM_ENT    paramType,
    IN GT_U32                   accessLevel
)
{
    GT_U32 secureLevelConfRegAddr;    /* register address */
    GT_U32 fieldOffset;               /* the start bit number in register */
    GT_U32 numOfBits = 3;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH2_NST_CHECK_MAC_ACCESS_LEVEL_MAC(accessLevel);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        switch (paramType)
        {
           case CPSS_NST_AM_SA_AUTO_LEARNED_E:
               secureLevelConfRegAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.
                                FDBGlobalConfig.bridgeAccessLevelConfig;
               fieldOffset = 0;

               if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
               {
                   if(accessLevel >= BIT_1)
                   {
                        /*single bit supported*/
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                            "accessLevel for SA_AUTO_LEARNED limited to [0..1] but got value[%d]",
                                accessLevel);
                   }
                   numOfBits = 1;
               }

               break;
           case CPSS_NST_AM_DA_AUTO_LEARNED_E:
               secureLevelConfRegAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.
                                FDBGlobalConfig.bridgeAccessLevelConfig;

               if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
               {
                   if(accessLevel >= BIT_1)
                   {
                        /*single bit supported*/
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                            "accessLevel for SA_AUTO_LEARNED limited to [0..1] but got value[%d]",
                                accessLevel);
                   }
                   fieldOffset = 1;
                   numOfBits = 1;
               }
               else
               {
                   fieldOffset = 4;
               }
               break;
           case CPSS_NST_AM_SA_UNKNOWN_E:
                secureLevelConfRegAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                                bridgeAccessMatrix.bridgeAccessMatrixDefault;
               fieldOffset = 0;
               break;
           case CPSS_NST_AM_DA_UNKNOWN_E:
                secureLevelConfRegAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                                bridgeAccessMatrix.bridgeAccessMatrixDefault;
               fieldOffset = 4;
               break;
           default:
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

    }
    else
    {
        secureLevelConfRegAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                     bridgeRegs.nstRegs.brgSecureLevelConfReg;

        PRV_CPSS_DXCH2_NST_DEFAULT_ACCESS_LEVEL_FIELD_OFFSET_SET_MAC(paramType,
                                                                 fieldOffset);
    }

    return /* Set default access levels*/
        prvCpssHwPpSetRegField(devNum,secureLevelConfRegAddr,fieldOffset,
                                                               numOfBits,accessLevel);

}

/**
* @internal cpssDxChNstDefaultAccessLevelsSet function
* @endinternal
*
* @brief   Set default access levels for Bridge Access Matrix Configuration table
*         that controls which access level pairs can communicate with each other.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] paramType                -   type of bridge acess matrix global parameter to
*                                      configure default access level .
* @param[in] accessLevel              -  default security access level for parameter
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum,paramType,accessLevel
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstDefaultAccessLevelsSet
(
    IN GT_U8                    devNum,
    IN CPSS_NST_AM_PARAM_ENT    paramType,
    IN GT_U32                   accessLevel
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstDefaultAccessLevelsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, paramType, accessLevel));

    rc = internal_cpssDxChNstDefaultAccessLevelsSet(devNum, paramType, accessLevel);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, paramType, accessLevel));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstDefaultAccessLevelsGet function
* @endinternal
*
* @brief   Get default access levels for Bridge Access Matrix Configuration table
*         that controls which access level pairs can communicate with each other.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] paramType                -   type of bridge acess matrix global parameter to
*                                      configure default access level .
*
* @param[out] accessLevelPtr           - (pointer to) default security access level for parameter
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum,paramType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstDefaultAccessLevelsGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_NST_AM_PARAM_ENT    paramType,
    OUT GT_U32                   *accessLevelPtr
)
{
    GT_U32 secureLevelConfRegAddr;    /* register address */
    GT_U32 fieldOffset;               /* the start bit number in register */
    GT_U32 numOfBits = 3;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(accessLevelPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        switch (paramType)
        {
           case CPSS_NST_AM_SA_AUTO_LEARNED_E:
               secureLevelConfRegAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.
                                FDBGlobalConfig.bridgeAccessLevelConfig;
               fieldOffset = 0;
               if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
               {
                   numOfBits = 1;
               }

               break;
           case CPSS_NST_AM_DA_AUTO_LEARNED_E:
               secureLevelConfRegAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.
                                FDBGlobalConfig.bridgeAccessLevelConfig;
               if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
               {
                   fieldOffset = 1;
                   numOfBits = 1;
               }
               else
               {
                   fieldOffset = 4;
               }
               break;
           case CPSS_NST_AM_SA_UNKNOWN_E:
                secureLevelConfRegAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                                bridgeAccessMatrix.bridgeAccessMatrixDefault;
               fieldOffset = 0;
               break;
           case CPSS_NST_AM_DA_UNKNOWN_E:
                secureLevelConfRegAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                                bridgeAccessMatrix.bridgeAccessMatrixDefault;
               fieldOffset = 4;
               break;
           default:
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

    }
    else
    {
        PRV_CPSS_DXCH2_NST_DEFAULT_ACCESS_LEVEL_FIELD_OFFSET_SET_MAC(paramType,
                                                                 fieldOffset);

        secureLevelConfRegAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                     bridgeRegs.nstRegs.brgSecureLevelConfReg;
    }


    /* Get default access levels */
    return  prvCpssHwPpGetRegField(devNum,secureLevelConfRegAddr,
                                       fieldOffset,numOfBits,accessLevelPtr);

}

/**
* @internal cpssDxChNstDefaultAccessLevelsGet function
* @endinternal
*
* @brief   Get default access levels for Bridge Access Matrix Configuration table
*         that controls which access level pairs can communicate with each other.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] paramType                -   type of bridge acess matrix global parameter to
*                                      configure default access level .
*
* @param[out] accessLevelPtr           - (pointer to) default security access level for parameter
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum,paramType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstDefaultAccessLevelsGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_NST_AM_PARAM_ENT    paramType,
    OUT GT_U32                   *accessLevelPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstDefaultAccessLevelsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, paramType, accessLevelPtr));

    rc = internal_cpssDxChNstDefaultAccessLevelsGet(devNum, paramType, accessLevelPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, paramType, accessLevelPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstProtSanityCheckPacketCommandSet function
* @endinternal
*
* @brief   Set packet command for sanity checks.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] checkType                - the type of sanity check
* @param[in] packetCmd                - packet command for sanity checks
*                                       for xCat3; AC5; Lion2.
*                                       APPLICABLE VALUES:
*                                       CPSS_PACKET_CMD_FORWARD_E;
*                                       CPSS_PACKET_CMD_DROP_HARD_E.
*                                       for Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*                                       APPLICABLE VALUES:
*                                       CPSS_PACKET_CMD_FORWARD_E;
*                                       CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
*                                       CPSS_PACKET_CMD_TRAP_TO_CPU_E;
*                                       CPSS_PACKET_CMD_DROP_HARD_E;
*                                       CPSS_PACKET_CMD_DROP_SOFT_E.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum,checkType or packetCmd
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstProtSanityCheckPacketCommandSet
(
    IN GT_U8                 devNum,
    IN CPSS_NST_CHECK_ENT    checkType,
    IN CPSS_PACKET_CMD_ENT   packetCmd
)
{
    GT_STATUS rc;           /* return code */
    GT_U32 regAddr;         /* register address */
    GT_U32 value;           /* value to write into register */
    GT_U32 index;           /* array index */
    GT_U32 fieldOffset;     /* the start bit number in register */
    GT_U32 data;            /* reg subfield data */
    GT_U32 tmpValue;        /* store packetCmd value to write to register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    switch (packetCmd)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            value = 0;
            break;
            /* MIRROR_TO_CPU has value 1 for SIP_5 devices
             * and is BAD_PARAM for xCat3, AC5, Lion2.
             * HAR_DROP has value 3 for SIP_5 devices and
             * value 1 for xcat3, AC5, Lion2
             */
        case CPSS_PACKET_CMD_DROP_HARD_E:
            value = (PRV_CPSS_SIP_5_CHECK_MAC(devNum)) ? 3 : 1;
            break;
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
            {
                value = 1;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
            {
                value = (packetCmd == CPSS_PACKET_CMD_TRAP_TO_CPU_E) ? 2 : 4;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Packet command invalid for the device");
    }

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_XCAT3_TCP_SYN_WITH_DATA_E))
    {
        if((checkType == CPSS_NST_CHECK_TCP_SYN_DATA_E) && (packetCmd != CPSS_PACKET_CMD_FORWARD_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /* set all TCP Sanity checks */
        if(checkType == CPSS_NST_CHECK_TCP_ALL_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                bridgeEngineConfig.bridgeCommandConfig3;

            tmpValue = value;

            data = 0;
            /*duplicate the value to 10 fields*/
            for(index = 0 ; index < 10; index++)
            {
                if((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_XCAT3_TCP_SYN_WITH_DATA_E)) &&
                    (index == 6))
                {
                    continue;
                }

                U32_SET_FIELD_MAC(data,(index*3),3,tmpValue);
            }

            return prvCpssHwPpSetRegField(devNum,regAddr,0,30,data);
        }

        PRV_CPSS_SIP5_NST_SANITY_CHECK_TYPE_FIELD_OFFSET_SET_MAC(
            checkType, regAddr, fieldOffset);

        return /* set sanity check */
            prvCpssHwPpSetRegField(devNum,regAddr,fieldOffset,3,value);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                bridgeRegs.bridgeGlobalConfigRegArray[0];

    /* set all TCP Sanity checks */
    if(checkType == CPSS_NST_CHECK_TCP_ALL_E)
    {
        data = (value == 1) ? 0xFFFFFFFF : 0;
        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_XCAT3_TCP_SYN_WITH_DATA_E))
        {
            data &= 0xDFFFFFFF ;
        }


        rc = prvCpssHwPpWriteRegBitMask(devNum,regAddr,0x20020000,data);
        if(rc != GT_OK)
        {
            return rc;
        }

        return prvCpssHwPpWriteRegBitMask(devNum,
                                             regAddr + 0xC,0x0000001F,data);
    }

    PRV_CPSS_DXCH2_NST_SANITY_CHECK_TYPE_FIELD_OFFSET_SET_MAC(
        checkType, index, fieldOffset);

    return /* set sanity check */
        prvCpssHwPpSetRegField(devNum,regAddr + index * 4,fieldOffset,1,value);

}

/**
* @internal cpssDxChNstProtSanityCheckPacketCommandSet function
* @endinternal
*
* @brief   Set packet command for sanity checks.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] checkType                - the type of sanity check
* @param[in] packetCmd                - packet command for sanity checks
*                                       for xCat3; AC5; Lion2.
*                                       APPLICABLE VALUES:
*                                       CPSS_PACKET_CMD_FORWARD_E;
*                                       CPSS_PACKET_CMD_DROP_HARD_E.
*                                       for Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*                                       APPLICABLE VALUES:
*                                       CPSS_PACKET_CMD_FORWARD_E;
*                                       CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
*                                       CPSS_PACKET_CMD_TRAP_TO_CPU_E;
*                                       CPSS_PACKET_CMD_DROP_HARD_E;
*                                       CPSS_PACKET_CMD_DROP_SOFT_E.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum,checkType or packetCmd
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstProtSanityCheckPacketCommandSet
(
    IN GT_U8                 devNum,
    IN CPSS_NST_CHECK_ENT    checkType,
    IN CPSS_PACKET_CMD_ENT   packetCmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstProtSanityCheckPacketCommandSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, checkType, packetCmd));

    rc = internal_cpssDxChNstProtSanityCheckPacketCommandSet(devNum, checkType, packetCmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, checkType, packetCmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstProtSanityCheckPacketCommandGet function
* @endinternal
*
* @brief   Get packet command for sanity checks.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] checkType                - the type of sanity check
*
* @param[out] packetCmdPtr            - (pointer to) packet command for sanity check
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum,checkType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstProtSanityCheckPacketCommandGet
(
    IN  GT_U8                 devNum,
    IN  CPSS_NST_CHECK_ENT    checkType,
    OUT CPSS_PACKET_CMD_ENT   *packetCmdPtr
)
{
    GT_U32 regAddr;       /* register address */
    GT_U32 data;          /* reg subfield data */
    GT_U32 fieldOffset;   /* the start bit number in register */
    GT_U32 index;         /* number of bridge global configuration register */
    GT_STATUS rc;         /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(packetCmdPtr);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        if(checkType == CPSS_NST_CHECK_TCP_ALL_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                bridgeEngineConfig.bridgeCommandConfig3;

            rc = prvCpssHwPpGetRegField(devNum,regAddr,0,30,&data);
            if(rc != GT_OK)
            {
                return rc;
            }

            for(index = 0 ; index < (10-1); index++)
            {
                if((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_XCAT3_TCP_SYN_WITH_DATA_E)) &&
                    ((index == 6) || (index+1 == 6)))
                {
                    continue;
                }
                if(U32_GET_FIELD_MAC(data,(index*3),3) != U32_GET_FIELD_MAC(data,(index+1)*3,3))
                {
                    /* not all fields with the same value */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
            }

            data = U32_GET_FIELD_MAC(data,0,3);

            /* all fields with the same value */
        }
        else
        {
            PRV_CPSS_SIP5_NST_SANITY_CHECK_TYPE_FIELD_OFFSET_SET_MAC(
                checkType, regAddr, fieldOffset);

             /* get sanity check */
            rc = prvCpssHwPpGetRegField(devNum,regAddr,fieldOffset,3,&data);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(*packetCmdPtr, data);
        return GT_OK;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                     bridgeRegs.bridgeGlobalConfigRegArray[0];


    PRV_CPSS_DXCH2_NST_SANITY_CHECK_TYPE_FIELD_OFFSET_SET_MAC(
        checkType, index, fieldOffset);
     /* get sanity check */
    rc = prvCpssHwPpGetRegField(devNum,regAddr + index * 4,
                                   fieldOffset,1,&data);
    if(rc != GT_OK)
    {
        return rc;

    }
    *packetCmdPtr = (data == 1) ? CPSS_PACKET_CMD_DROP_HARD_E : CPSS_PACKET_CMD_FORWARD_E;

    return GT_OK;

}

/**
* @internal cpssDxChNstProtSanityCheckPacketCommandGet function
* @endinternal
*
* @brief   Get packet command for sanity checks.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] checkType                - the type of sanity check
*
* @param[out] packetCmdPtr            - (pointer to) packet command for sanity check
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum,checkType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstProtSanityCheckPacketCommandGet
(
    IN  GT_U8                 devNum,
    IN  CPSS_NST_CHECK_ENT    checkType,
    OUT CPSS_PACKET_CMD_ENT   *packetCmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstProtSanityCheckPacketCommandGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, checkType, packetCmdPtr));

    rc = internal_cpssDxChNstProtSanityCheckPacketCommandGet(devNum, checkType, packetCmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, checkType, packetCmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstProtSanityCheckSet function
* @endinternal
*
* @brief   Set packet sanity checks.
*         Sanity Check engine identifies "suspicious" packets and
*         provides an option for assigning them a Hard Drop packet command.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] checkType                - the type of sanity check
* @param[in] enable                   - GT_TRUE   packet's sanity check. Packet that not passed
*                                      check will be dropped and treated as security breach event.
*                                      - GT_FALSE disable packet's sanity check.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum,checkType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstProtSanityCheckSet
(
    IN GT_U8                 devNum,
    IN CPSS_NST_CHECK_ENT    checkType,
    IN GT_BOOL               enable
)
{
    CPSS_PACKET_CMD_ENT   packetCmd; /* sanity check packet command */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    packetCmd = (enable == GT_TRUE) ? CPSS_PACKET_CMD_DROP_HARD_E : CPSS_PACKET_CMD_FORWARD_E;

     /* set sanity check */
    return cpssDxChNstProtSanityCheckPacketCommandSet(devNum, checkType, packetCmd);
}

/**
* @internal cpssDxChNstProtSanityCheckSet function
* @endinternal
*
* @brief   Set packet sanity checks.
*         Sanity Check engine identifies "suspicious" packets and
*         provides an option for assigning them a Hard Drop packet command.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] checkType                - the type of sanity check
* @param[in] enable                   - GT_TRUE   packet's sanity check. Packet that not passed
*                                      check will be dropped and treated as security breach event.
*                                      - GT_FALSE disable packet's sanity check.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum,checkType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstProtSanityCheckSet
(
    IN GT_U8                 devNum,
    IN CPSS_NST_CHECK_ENT    checkType,
    IN GT_BOOL               enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstProtSanityCheckSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, checkType, enable));

    rc = internal_cpssDxChNstProtSanityCheckSet(devNum, checkType, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, checkType, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstProtSanityCheckGet function
* @endinternal
*
* @brief   Get packet sanity checks.
*         Sanity Check engine identifies "suspicious" packets and
*         provides an option for assigning them a Hard Drop packet command.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] checkType                - the type of sanity check
*
* @param[out] enablePtr                - GT_TRUE  enable packet's sanity check. Packet that not passed
*                                      check will be dropped and treated as security breach event.
*                                      - GT_FALSE disable packet's sanity check.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum,checkType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstProtSanityCheckGet
(
    IN  GT_U8                 devNum,
    IN  CPSS_NST_CHECK_ENT    checkType,
    OUT GT_BOOL               *enablePtr
)
{
    GT_STATUS rc;         /* return code */
    CPSS_PACKET_CMD_ENT   pktCommand; /* sanity check packet command */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    rc = cpssDxChNstProtSanityCheckPacketCommandGet(devNum, checkType, &pktCommand);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (pktCommand == CPSS_PACKET_CMD_DROP_HARD_E) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal cpssDxChNstProtSanityCheckGet function
* @endinternal
*
* @brief   Get packet sanity checks.
*         Sanity Check engine identifies "suspicious" packets and
*         provides an option for assigning them a Hard Drop packet command.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] checkType                - the type of sanity check
*
* @param[out] enablePtr                - GT_TRUE  enable packet's sanity check. Packet that not passed
*                                      check will be dropped and treated as security breach event.
*                                      - GT_FALSE disable packet's sanity check.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum,checkType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstProtSanityCheckGet
(
    IN  GT_U8                 devNum,
    IN  CPSS_NST_CHECK_ENT    checkType,
    OUT GT_BOOL               *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstProtSanityCheckGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, checkType, enablePtr));

    rc = internal_cpssDxChNstProtSanityCheckGet(devNum, checkType, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, checkType, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIngressFrwFilterSet function
* @endinternal
*
* @brief   Set port ingress forwarding filter.
*         For a given ingress port Enable/Disable traffic if it is destinied to:
*         CPU, ingress analyzer, network.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] filterType               - ingress filter type
* @param[in] enable                   - GT_TRUE -  ingress forwarding restiriction according
*                                      to filterType, filterred packets will be dropped.
*                                      - GT_FALSE - disable ingress forwarding restiriction
*                                      according to filterType
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, port, filterType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstPortIngressFrwFilterSet
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_NST_INGRESS_FRW_FILTER_ENT  filterType,
    IN GT_BOOL                          enable
)
{
    GT_STATUS   rc  = GT_OK;
    GT_U32      regAddr;           /* register address */
    GT_U32      data;              /* reg subfield data */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32       localPort;/* local port - support multi-port-groups device */
    GT_U32      bitOffset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    data = (enable == GT_TRUE) ? 0 : 1;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        switch(filterType)
        {
            case CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E:
                bitOffset = 1;
                break;
           case CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E:
               bitOffset = 0;
               break;
           case CPSS_NST_INGRESS_FRW_FILTER_TO_ANALYZER_E:
               bitOffset = 2;
               break;
           default:
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

         /* set EQ-ingress-eport table */
         rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            bitOffset, /* start bit */
                                            1,  /* 1 bit */
                                            data);

    }
    else
    {
        /* The device not supports 'CPU port' even if the FS may (by mistake) state so */
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        PRV_CPSS_DXCH2_NST_PORT_INGRESS_FRW_FILTER_REG_ADDRESS_SET_MAC(devNum,
                                                                   filterType,
                                                                   regAddr);
        /* Set port ingress forwarding filter */
        rc = prvCpssHwPpPortGroupSetRegField(devNum,portGroupId, regAddr, localPort, 1, data);
    }

    if(rc != GT_OK)
        return rc;

    return rc;
}

/**
* @internal cpssDxChNstPortIngressFrwFilterSet function
* @endinternal
*
* @brief   Set port ingress forwarding filter.
*         For a given ingress port Enable/Disable traffic if it is destinied to:
*         CPU, ingress analyzer, network.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] filterType               - ingress filter type
* @param[in] enable                   - GT_TRUE -  ingress forwarding restiriction according
*                                      to filterType, filterred packets will be dropped.
*                                      - GT_FALSE - disable ingress forwarding restiriction
*                                      according to filterType
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, port, filterType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortIngressFrwFilterSet
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_NST_INGRESS_FRW_FILTER_ENT  filterType,
    IN GT_BOOL                          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIngressFrwFilterSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, filterType, enable));

    rc = internal_cpssDxChNstPortIngressFrwFilterSet(devNum, portNum, filterType, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, filterType, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortIngressFrwFilterGet function
* @endinternal
*
* @brief   Get port ingress forwarding filter.
*         For a given ingress port Enable/Disable traffic if it is destinied to:
*         CPU, ingress analyzer, network.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] filterType               - ingress filter type
*
* @param[out] enablePtr                - GT_TRUE - enable ingress forwarding restiriction according
*                                      to filterType, filterred packets will be dropped.
*                                      - GT_FALSE - disable ingress forwarding restiriction
*                                      according to filterType
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, port, filterType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstPortIngressFrwFilterGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    IN  CPSS_NST_INGRESS_FRW_FILTER_ENT  filterType,
    OUT GT_BOOL                          *enablePtr
)
{
    GT_U32      regAddr;           /* register address */
    GT_STATUS   rc;                /* return code */
    GT_U32      data;              /* reg subfield data */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32       localPort;/* local port - support multi-port-groups device */
    GT_U32      bitOffset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        switch(filterType)
        {
            case CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E:
                bitOffset = 1;
                break;
           case CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E:
               bitOffset = 0;
               break;
           case CPSS_NST_INGRESS_FRW_FILTER_TO_ANALYZER_E:
               bitOffset = 2;
               break;
           default:
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

         /* get EQ-ingress-eport table */
         rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            bitOffset, /* start bit */
                                            1,  /* 1 bit */
                                            &data);

    }
    else
    {
        /* The device not supports 'CPU port' even if the FS may (by mistake) state so */
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        PRV_CPSS_DXCH2_NST_PORT_INGRESS_FRW_FILTER_REG_ADDRESS_SET_MAC(devNum,
                                                                   filterType,
                                                                   regAddr);
        /* Set port ingress forwarding filter */
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, localPort, 1, &data);
        if(rc != GT_OK)
            return rc;
    }
    *enablePtr = (data == 1) ? GT_FALSE : GT_TRUE;

    return GT_OK;

}

/**
* @internal cpssDxChNstPortIngressFrwFilterGet function
* @endinternal
*
* @brief   Get port ingress forwarding filter.
*         For a given ingress port Enable/Disable traffic if it is destinied to:
*         CPU, ingress analyzer, network.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] filterType               - ingress filter type
*
* @param[out] enablePtr                - GT_TRUE - enable ingress forwarding restiriction according
*                                      to filterType, filterred packets will be dropped.
*                                      - GT_FALSE - disable ingress forwarding restiriction
*                                      according to filterType
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, port, filterType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortIngressFrwFilterGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    IN  CPSS_NST_INGRESS_FRW_FILTER_ENT  filterType,
    OUT GT_BOOL                          *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortIngressFrwFilterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, filterType, enablePtr));

    rc = internal_cpssDxChNstPortIngressFrwFilterGet(devNum, portNum, filterType, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, filterType, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChNstPortEgressFrwFilterSet function
* @endinternal
*
* @brief   Set port egress forwarding filter.
*         For a given egress port Enable/Disable traffic if the packet was:
*         sent from CPU with FROM_CPU DSA tag, bridged or policy switched,
*         routed or policy routed.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] filterType               - egress filter type
* @param[in] enable                   - GT_TRUE -  egress forwarding restiriction according
*                                      to filterType, filterred packets will be dropped.
*                                      - GT_FALSE - disable egress forwarding restiriction
*                                      according to filterType.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, port, filterType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstPortEgressFrwFilterSet
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_NST_EGRESS_FRW_FILTER_ENT   filterType,
    IN GT_BOOL                          enable
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    data;              /* reg subfield data */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */
    GT_U32  bitOffset;/* bit offset in the register */
    PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ENT fieldType;/*field type*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    data = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        switch(filterType)
        {
            case CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E:
                fieldType = PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_FROM_CPU_FORWARD_RESTRICTED_E;
                break;
            case CPSS_NST_EGRESS_FRW_FILTER_BRIDGED_E:
                fieldType = PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_BRIDGED_FORWARD_RESTRICTED_E;
               break;
            case CPSS_NST_EGRESS_FRW_FILTER_ROUTED_E:
                fieldType = PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ROUTED_FORWARD_RESTRICTED_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        return prvCpssDxChHwEgfShtFieldSet(devNum,portNum,
                fieldType,
                GT_TRUE, /*accessPhysicalPort*/
                GT_TRUE, /*accessEPort*/
                data);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
        {
            /* Set port egress forwarding filter */
            PRV_CPSS_DXCH2_NST_PORT_EGRESS_FRW_FILTER_REG_ADDRESS_SET_MAC(devNum,
                                                                      filterType,
                                                                      regAddr);
            return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, localPort, 1, data);
        }
        else
        {
            TXQ_DISTIBUTER_FORWARD_RESTRICTED_FILTER_REG_ADDR_AND_BIT_MAC(devNum,
                                                                      portNum,
                                                                      filterType,
                                                                      regAddr,
                                                                      bitOffset);
            return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, bitOffset, 1, data);
        }
    }
}

/**
* @internal cpssDxChNstPortEgressFrwFilterSet function
* @endinternal
*
* @brief   Set port egress forwarding filter.
*         For a given egress port Enable/Disable traffic if the packet was:
*         sent from CPU with FROM_CPU DSA tag, bridged or policy switched,
*         routed or policy routed.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] filterType               - egress filter type
* @param[in] enable                   - GT_TRUE -  egress forwarding restiriction according
*                                      to filterType, filterred packets will be dropped.
*                                      - GT_FALSE - disable egress forwarding restiriction
*                                      according to filterType.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, port, filterType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortEgressFrwFilterSet
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_NST_EGRESS_FRW_FILTER_ENT   filterType,
    IN GT_BOOL                          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortEgressFrwFilterSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, filterType, enable));

    rc = internal_cpssDxChNstPortEgressFrwFilterSet(devNum, portNum, filterType, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, filterType, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortEgressFrwFilterGet function
* @endinternal
*
* @brief   Get port egress forwarding filter.
*         For a given egress port Enable/Disable traffic if the packet was:
*         sent from CPU with FROM_CPU DSA tag, bridged or policy switched,
*         routed or policy routed.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] filterType               - egress filter type
*
* @param[out] enablePtr                - GT_TRUE - enable egress forwarding restiriction according
*                                      to filterType, filterred packets will be dropped.
*                                      - GT_FALSE - disable egress forwarding restiriction
*                                      according to filterType.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, port, filterType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstPortEgressFrwFilterGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    IN  CPSS_NST_EGRESS_FRW_FILTER_ENT   filterType,
    OUT GT_BOOL                          *enablePtr
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    data;              /* reg subfield data */
    GT_STATUS rc;                /* return code */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */
    GT_U32  bitOffset;/* bit offset in the register */
    PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ENT fieldType;/*field type*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        switch(filterType)
        {
            case CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E:
                fieldType = PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_FROM_CPU_FORWARD_RESTRICTED_E;
                break;
            case CPSS_NST_EGRESS_FRW_FILTER_BRIDGED_E:
                fieldType = PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_BRIDGED_FORWARD_RESTRICTED_E;
               break;
            case CPSS_NST_EGRESS_FRW_FILTER_ROUTED_E:
                fieldType = PRV_CPSS_DXCH_HW_EGF_SHT_FIELD_ROUTED_FORWARD_RESTRICTED_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDxChHwEgfShtFieldGet(devNum,portNum,
                fieldType,
                GT_FALSE, /*accessPhysicalPort*/
                &data);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
        {
            PRV_CPSS_DXCH2_NST_PORT_EGRESS_FRW_FILTER_REG_ADDRESS_SET_MAC(devNum,
                                                                      filterType,
                                                                      regAddr);
            /* Get port egress forwarding filter */
            rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, localPort, 1, &data);
        }
        else
        {
            TXQ_DISTIBUTER_FORWARD_RESTRICTED_FILTER_REG_ADDR_AND_BIT_MAC(devNum,
                                                                      portNum,
                                                                      filterType,
                                                                      regAddr,
                                                                      bitOffset);
            rc =  prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, bitOffset, 1, &data);
        }
    }

    *enablePtr = BIT2BOOL_MAC(data);

    return rc;

}

/**
* @internal cpssDxChNstPortEgressFrwFilterGet function
* @endinternal
*
* @brief   Get port egress forwarding filter.
*         For a given egress port Enable/Disable traffic if the packet was:
*         sent from CPU with FROM_CPU DSA tag, bridged or policy switched,
*         routed or policy routed.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] filterType               - egress filter type
*
* @param[out] enablePtr                - GT_TRUE - enable egress forwarding restiriction according
*                                      to filterType, filterred packets will be dropped.
*                                      - GT_FALSE - disable egress forwarding restiriction
*                                      according to filterType.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, port, filterType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortEgressFrwFilterGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    IN  CPSS_NST_EGRESS_FRW_FILTER_ENT   filterType,
    OUT GT_BOOL                          *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortEgressFrwFilterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, filterType, enablePtr));

    rc = internal_cpssDxChNstPortEgressFrwFilterGet(devNum, portNum, filterType, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, filterType, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstIngressFrwFilterDropCntrSet function
* @endinternal
*
* @brief   Set the global ingress forwarding restriction drop packet counter.
*         This counter counts the number of packets dropped due to Ingress forward
*         restrictions.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] ingressCnt               - the counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstIngressFrwFilterDropCntrSet
(
    IN GT_U8       devNum,
    IN GT_U32      ingressCnt
)
{
    return cpssDxChNstPortGroupIngressFrwFilterDropCntrSet(devNum,
                                              CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                           ingressCnt);
}

/**
* @internal cpssDxChNstIngressFrwFilterDropCntrSet function
* @endinternal
*
* @brief   Set the global ingress forwarding restriction drop packet counter.
*         This counter counts the number of packets dropped due to Ingress forward
*         restrictions.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] ingressCnt               - the counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstIngressFrwFilterDropCntrSet
(
    IN GT_U8       devNum,
    IN GT_U32      ingressCnt
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstIngressFrwFilterDropCntrSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ingressCnt));

    rc = internal_cpssDxChNstIngressFrwFilterDropCntrSet(devNum, ingressCnt);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ingressCnt));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstIngressFrwFilterDropCntrGet function
* @endinternal
*
* @brief   Reads the global ingress forwarding restriction drop packet counter.
*         This counter counts the number of packets dropped due to Ingress forward
*         restrictions.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] ingressCntPtr            - the counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstIngressFrwFilterDropCntrGet
(
    IN GT_U8       devNum,
    OUT GT_U32     *ingressCntPtr
)
{
    return cpssDxChNstPortGroupIngressFrwFilterDropCntrGet(devNum,
                                               CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                           ingressCntPtr);
}

/**
* @internal cpssDxChNstIngressFrwFilterDropCntrGet function
* @endinternal
*
* @brief   Reads the global ingress forwarding restriction drop packet counter.
*         This counter counts the number of packets dropped due to Ingress forward
*         restrictions.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] ingressCntPtr            - the counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstIngressFrwFilterDropCntrGet
(
    IN GT_U8       devNum,
    OUT GT_U32     *ingressCntPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstIngressFrwFilterDropCntrGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ingressCntPtr));

    rc = internal_cpssDxChNstIngressFrwFilterDropCntrGet(devNum, ingressCntPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ingressCntPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstRouterAccessMatrixCmdSet function
* @endinternal
*
* @brief   Set Routers access matrix entry.
*         The packet is assigned two access levels based on its SIP and DIP.
*         The Access Matrix Configuration tables controls which access level
*         pairs can communicate with each other. Based on the access level pair,
*         the packet is assigned a command of: FORWARD, HARD DROP, and SOFT DROP.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - physical device number
* @param[in] sipAccessLevel           - the sip access level
* @param[in] dipAccessLevel           - the dip access level
* @param[in] command                  -  assigned to a packet
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, saAccessLevel, daAccessLevel.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstRouterAccessMatrixCmdSet
(
    IN  GT_U8                      devNum,
    IN  GT_U32                     sipAccessLevel,
    IN  GT_U32                     dipAccessLevel,
    IN  CPSS_PACKET_CMD_ENT        command
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* Set router access matrix entry */
    return prvCpssDxChNstAccessMatrixCmdSet(devNum,sipAccessLevel,dipAccessLevel,
                                            command,GT_FALSE);
}

/**
* @internal cpssDxChNstRouterAccessMatrixCmdSet function
* @endinternal
*
* @brief   Set Routers access matrix entry.
*         The packet is assigned two access levels based on its SIP and DIP.
*         The Access Matrix Configuration tables controls which access level
*         pairs can communicate with each other. Based on the access level pair,
*         the packet is assigned a command of: FORWARD, HARD DROP, and SOFT DROP.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - physical device number
* @param[in] sipAccessLevel           - the sip access level
*                                       APPLICABLE RANGES:
*                                       xCat3, AC5, Lion2: 0...7
*                                       Bobcat2 and Above: 0...63
* @param[in] dipAccessLevel           - the dip access level
*                                       APPLICABLE RANGES:
*                                       xCat3, AC5, Lion2: 0...7
*                                       Bobcat2 and Above: 0...63
* @param[in] command                  -  assigned to a packet
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, saAccessLevel, daAccessLevel.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstRouterAccessMatrixCmdSet
(
    IN  GT_U8                      devNum,
    IN  GT_U32                     sipAccessLevel,
    IN  GT_U32                     dipAccessLevel,
    IN  CPSS_PACKET_CMD_ENT        command
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstRouterAccessMatrixCmdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sipAccessLevel, dipAccessLevel, command));

    rc = internal_cpssDxChNstRouterAccessMatrixCmdSet(devNum, sipAccessLevel, dipAccessLevel, command);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sipAccessLevel, dipAccessLevel, command));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstRouterAccessMatrixCmdGet function
* @endinternal
*
* @brief   Get Routers access matrix entry.
*         The packet is assigned two access levels based on its SIP and DIP.
*         The Access Matrix Configuration tables controls which access level
*         pairs can communicate with each other. Based on the access level pair,
*         the packet is assigned a command of: FORWARD, HARD DROP, and SOFT DROP.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - physical device number
* @param[in] sipAccessLevel           - the sip access level
*                                       APPLICABLE RANGES:
*                                       xCat3, AC5, Lion2: 0...7
*                                       Bobcat2 and Above: 0...63
* @param[in] dipAccessLevel           - the dip access level
*                                       APPLICABLE RANGES:
*                                       xCat3, AC5, Lion2: 0...7
*                                       Bobcat2 and Above: 0...63
* @param[out] commandPtr               - command assigned to a packet
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, saAccessLevel, daAccessLevel.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstRouterAccessMatrixCmdGet
(
    IN  GT_U8                      devNum,
    IN  GT_U32                     sipAccessLevel,
    IN  GT_U32                     dipAccessLevel,
    OUT  CPSS_PACKET_CMD_ENT       *commandPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* Get router access matrix entry */
    return prvCpssDxChNstAccessMatrixCmdGet(devNum,sipAccessLevel,dipAccessLevel,
                                            commandPtr,GT_FALSE);
}

/**
* @internal cpssDxChNstRouterAccessMatrixCmdGet function
* @endinternal
*
* @brief   Get Routers access matrix entry.
*         The packet is assigned two access levels based on its SIP and DIP.
*         The Access Matrix Configuration tables controls which access level
*         pairs can communicate with each other. Based on the access level pair,
*         the packet is assigned a command of: FORWARD, HARD DROP, and SOFT DROP.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - physical device number
* @param[in] sipAccessLevel           - the sip access level
*                                       APPLICABLE RANGES:
*                                       xCat3, AC5, Lion2: 0...7
*                                       Bobcat2 and Above: 0...63
* @param[in] dipAccessLevel           - the dip access level
*                                       APPLICABLE RANGES:
*                                       xCat3, AC5, Lion2: 0...7
*                                       Bobcat2 and Above: 0...63
* @param[out] commandPtr               - command assigned to a packet
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, saAccessLevel, daAccessLevel.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstRouterAccessMatrixCmdGet
(
    IN  GT_U8                      devNum,
    IN  GT_U32                     sipAccessLevel,
    IN  GT_U32                     dipAccessLevel,
    OUT  CPSS_PACKET_CMD_ENT       *commandPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstRouterAccessMatrixCmdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sipAccessLevel, dipAccessLevel, commandPtr));

    rc = internal_cpssDxChNstRouterAccessMatrixCmdGet(devNum, sipAccessLevel, dipAccessLevel, commandPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sipAccessLevel, dipAccessLevel, commandPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortGroupIngressFrwFilterDropCntrSet function
* @endinternal
*
* @brief   Set the global ingress forwarding restriction drop packet counter.
*         This counter counts the number of packets dropped due to Ingress forward
*         restrictions.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ingressCnt               - the counter value
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstPortGroupIngressFrwFilterDropCntrSet
(
    IN GT_U8                   devNum,
    IN GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN GT_U32                  ingressCnt
)
{
    GT_U32    regAddr;           /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                      bridgeRegs.nstRegs.ingressFrwDropCounter;

    /* set ingress drop counter */
    return prvCpssPortGroupsBmpCounterSet(devNum, portGroupsBmp,
                                              regAddr,0, 32, ingressCnt);
}

/**
* @internal cpssDxChNstPortGroupIngressFrwFilterDropCntrSet function
* @endinternal
*
* @brief   Set the global ingress forwarding restriction drop packet counter.
*         This counter counts the number of packets dropped due to Ingress forward
*         restrictions.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ingressCnt               - the counter value
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortGroupIngressFrwFilterDropCntrSet
(
    IN GT_U8                   devNum,
    IN GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN GT_U32                  ingressCnt
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortGroupIngressFrwFilterDropCntrSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, ingressCnt));

    rc = internal_cpssDxChNstPortGroupIngressFrwFilterDropCntrSet(devNum, portGroupsBmp, ingressCnt);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, ingressCnt));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNstPortGroupIngressFrwFilterDropCntrGet function
* @endinternal
*
* @brief   Reads the global ingress forwarding restriction drop packet counter.
*         This counter counts the number of packets dropped due to Ingress forward
*         restrictions.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] ingressCntPtr            - the counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNstPortGroupIngressFrwFilterDropCntrGet
(
    IN GT_U8                    devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *ingressCntPtr
)
{
    GT_U32    regAddr;           /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(ingressCntPtr);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
                          ingrForwardingRestrictions.ingrForwardingRestrictionsDroppedPktsCntr;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                          bridgeRegs.nstRegs.ingressFrwDropCounter;
    }

    /* read ingress drop counter */
    return prvCpssPortGroupsBmpCounterSummary(devNum, portGroupsBmp, regAddr,
                                                  0, 32,
                                                  ingressCntPtr, NULL);
}

/**
* @internal cpssDxChNstPortGroupIngressFrwFilterDropCntrGet function
* @endinternal
*
* @brief   Reads the global ingress forwarding restriction drop packet counter.
*         This counter counts the number of packets dropped due to Ingress forward
*         restrictions.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] ingressCntPtr            - the counter value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNstPortGroupIngressFrwFilterDropCntrGet
(
    IN GT_U8                    devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *ingressCntPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNstPortGroupIngressFrwFilterDropCntrGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, ingressCntPtr));

    rc = internal_cpssDxChNstPortGroupIngressFrwFilterDropCntrGet(devNum, portGroupsBmp, ingressCntPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, ingressCntPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



