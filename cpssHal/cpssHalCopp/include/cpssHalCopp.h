/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalCopp.h
*
* @brief Internal header which defines API for helpers functions
* which are specific for XPS Copp Table .
*
* @version   01
********************************************************************************
*/
#ifndef __cpssHalCopph
#define __cpssHalCopph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtOs/gtGenTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/generic/cos/cpssCosTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>

//TODO All these macro need to be renamed with XPS_/CPSS_ Convention.
#define NUMBER_OF_RXTX_BUFFER 1
#define MAX_TX_SDMA_BUFFER_SIZE (10240+64) //For dma memory allocation
#define DMA_BUFFER_OFFSET   32
#define MIN_DMA_BUFF_PKT_LEN 60
#define PKTDUMP_MAX_BYTES       64
#define PKTDUMP_BYTES_PER_LINE  16
#define MAC_FCS_LENGTH_CNS      4
#define DEFAULT_RATE_LIMITER_WINDOW_SIZE 100

/*
 * typedef: enum stat rate limiter index
 * Description: Statistical Rate limiter index for dropping and trapping all the packets to CPU.
 */
typedef enum
{
    XPS_STAT_RATE_INDEX_FWD_ALL = 0,
    XPS_STAT_RATE_INDEX_DROP_ALL = 1,
} XPS_STAT_RATE_LIMIT_INDEX;


typedef enum cpssHalCoppAclPriority
{
    CPSS_HAL_COPP_ACL_PRIO_INTERVENTION_ARP_E = 0,
    CPSS_HAL_COPP_ACL_PRIO_ARP_REPLY_TO_ME_E = 1,

    CPSS_HAL_COPP_ACL_PRIO_ARP_INGRESS_MIRRORED_TO_ANLYZER_E = 2,
    CPSS_HAL_COPP_ACL_PRIO_ARP_EGRESS_MIRRORED_TO_ANLYZER_E = 3,
    CPSS_HAL_COPP_ACL_PRIO_ARP_STATIC_ADDR_MOVED_E = 4,

    CPSS_HAL_COPP_ACL_PRIO_CPU_CODE_DHCPv4_L2_L3 = 5,
    CPSS_HAL_COPP_ACL_PRIO_CPU_CODE_DHCPv6_L2_L3 = 6,

    CPSS_HAL_COPP_ACL_PRIO_INTERVENTION_IGMP_E = 7,
    CPSS_HAL_COPP_ACL_PRIO_IPV6_ICMP_PACKET_E = 8,

    CPSS_HAL_COPP_ACL_PRIO_IPV4_LINK_LOCAL_MC_DIP_TRP_MRR_1_E = 9,
    CPSS_HAL_COPP_ACL_PRIO_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_1_E = 10,

    CPSS_HAL_COPP_ACL_PRIO_IPV4_LINK_LOCAL_MC_DIP_TRP_MRR_3_E = 11,
    CPSS_HAL_COPP_ACL_PRIO_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_3_E = 12,

    CPSS_HAL_COPP_ACL_PRIO_IPV4_LINK_LOCAL_MC_DIP_TRP_MRR_2_E = 13,
    CPSS_HAL_COPP_ACL_PRIO_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E = 14,

    CPSS_HAL_COPP_ACL_PRIO_IPV4_UDP_BC_MIRROR_TRAP2_E = 15,
    CPSS_HAL_COPP_ACL_PRIO_IPV6_UDP_BC_MIRROR_TRAP2_E = 16,

    CPSS_HAL_COPP_ACL_PRIO_IPV4_ZERO_TTL_TRAP_E = 17,
    CPSS_HAL_COPP_ACL_PRIO_IPV6_ZERO_TTL_TRAP_E = 18,
    CPSS_HAL_COPP_ACL_PRIO_IPV6_TTL_TRAP_E = 19,

    CPSS_HAL_COPP_ACL_PRIO_IPV4_MTU_EXCEED_E = 20,
    CPSS_HAL_COPP_ACL_PRIO_IPV6_MTU_EXCEED_E = 21,

    CPSS_HAL_COPP_ACL_PRIO_CPU_CODE_BGPv4 = 22,
    CPSS_HAL_COPP_ACL_PRIO_CPU_CODE_BGPv6 = 23,

    CPSS_HAL_COPP_ACL_PRIO_IPV4_ROUTED_PACKET_FORWARD_E = 24,
    CPSS_HAL_COPP_ACL_PRIO_IPV6_ROUTED_PACKET_FORWARD_E = 25,
    CPSS_HAL_COPP_ACL_PRIO_IPV4_ROUTE_ENTRY_TRAP_E = 26,
    CPSS_HAL_COPP_ACL_PRIO_IPV6_ROUTE_ENTRY_TRAP_E = 27,
    CPSS_HAL_COPP_ACL_PRIO_IPV6_ROUTE_TRAP_E = 28,

    CPSS_HAL_COPP_ACL_PRIO_IPV4_CPU_CODE_SSH = 29,
    CPSS_HAL_COPP_ACL_PRIO_IPV6_CPU_CODE_SSH = 30,

    CPSS_HAL_COPP_ACL_PRIO_IPV4_INGRESS_MIRRORED_TO_ANLYZER_E = 31,
    CPSS_HAL_COPP_ACL_PRIO_IPV6_INGRESS_MIRRORED_TO_ANLYZER_E = 32,

    CPSS_HAL_COPP_ACL_PRIO_IPV4_EGRESS_MIRRORED_TO_ANLYZER_E = 33,
    CPSS_HAL_COPP_ACL_PRIO_IPV6_EGRESS_MIRRORED_TO_ANLYZER_E = 34,

    CPSS_HAL_COPP_ACL_PRIO_IPV4_STATIC_ADDR_MOVED_E = 35,
    CPSS_HAL_COPP_ACL_PRIO_IPV6_STATIC_ADDR_MOVED_E = 36,

    CPSS_HAL_COPP_ACL_PRIO_NONIP_INGRESS_MIRRORED_TO_ANLYZER_E = 37,
    CPSS_HAL_COPP_ACL_PRIO_NONIP_EGRESS_MIRRORED_TO_ANLYZER_E = 38,

    CPSS_HAL_COPP_ACL_PRIO_NONIP_STATIC_ADDR_MOVED_E = 39,

    CPSS_HAL_COPP_ACL_PRIO_IEEE_RSRVD_MULTICAST_ADDR_1_E = 40,
    CPSS_HAL_COPP_ACL_PRIO_CISCO_MULTICAST_MAC_RANGE_E = 41,

    CPSS_HAL_COPP_ACL_PRIO_IEEE_RSRVD_MULTICAST_ADDR_2_E = 42,
    CPSS_HAL_COPP_ACL_PRIO_IEEE_RSRVD_MULTICAST_ADDR_3_E = 43,
    CPSS_HAL_COPP_ACL_PRIO_EAPOL_ADDR_E = 44,


} cpssHalCoppAclPriority_e;


/******************************************************************************\
 *                            XPS COPP Entry APIs                         *
\******************************************************************************/


/**
 * \brief API for getting CPU code table entry for a trapId
 *
 * \param [in] devId
 * \param [in] trapId
 * \param [in] *CPU_code_table_entry
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCoppReadCpuCodeTableEntry(uint32_t devId,
                                           CPSS_NET_RX_CPU_CODE_ENT cpuCode,
                                           CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC *entry);

/**
 * \brief API for writing CPU code table entry
 *
 * \param [in] devId
 * \param [in] trapId
 * \param [in] *CPU_code_table_entry
 *
 * \return GT_STATUS
 */
GT_STATUS cpssHalCoppWriteCpuCodeTableEntry(uint32_t devId,
                                            CPSS_NET_RX_CPU_CODE_ENT cpuCode,
                                            CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC *entry);


/**
 * \brief API for getting rate limit
 *
 * \param [in] devId
 * \param [in] index
 * \param [in] *rateLimit
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCoppGetCpuRateLimit(uint32_t devId, uint32_t index,
                                     uint32_t *rateLimit);

/**
 * \brief API for setting rate limit for a cpu code
 *
 * \param [in] devId
 * \param [in] cpuCode
 * \param [in] rateLimit
 * \param [in] index
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalDefaultRateLimitSet(uint32_t devId,
                                     CPSS_NET_RX_CPU_CODE_ENT cpuCode, uint32_t rateLimit, uint32_t index);

/**
 * \brief API for setting rate limit
 *
 * \param [in] devId
 * \param [in] index
 * \param [in] rateLimit
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCoppSetCpuRateLimit(uint32_t devId, uint32_t index,
                                     uint32_t rateLimit);

/**
* @internal prvXpsCoppTrapEntrySet function
* @endinternal
*
* @brief   Create new or update existing entry in Hardware CPU code table
* @param[in] devNum        - Device Id
* @param[in] trapType              - Trap type
* @param[in] coppEntryPtr          - (pointer to) cpu code table
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - aging timeout exceeds hardware limitation.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvXpsCoppTrapEntrySet
(
    GT_U8                    devNum,
    GT_U32   trapType,
    xpCoppEntryData_t *coppEntryPtr
);

/**
* @internal prvXpsCoppTrapEntryGet function
* @endinternal
*
* @brief   Get existing entry in Hardware CPU code table
*
* @param[in] devNum        - Device Id
* @param[in] trapType              - Trap type
* @param[out] coppEntryPtr         - (pointer to) cpu code table entry
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - aging timeout exceeds hardware limitation.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvXpsCoppTrapEntryGet
(
    GT_U8                            devNum,
    GT_U32   trapType,
    OUT xpCoppEntryData_t *coppEntryPtr
);



/**
 * \brief API for setting egress acl entry for a cpu code
 *
 * \param [in] devId
 * \param [in] tableId
 * \param [in] ruleId
 * \param [in] keyType
 * \param [in] pktCmd
 * \param [in] cpuCode
 * \param [in] policerId
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCoppWriteEgressAclTableEntry
(
    uint32_t devId,
    uint32_t tableId,
    uint32_t ruleId,
    uint32_t keyType,
    CPSS_PACKET_CMD_ENT pktCmd,
    CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    uint32_t policerId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __cpssHalCopph */

