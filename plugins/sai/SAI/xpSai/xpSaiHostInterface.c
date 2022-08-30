// xpSaiHostInterface.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include <signal.h>

#include "xpSaiHostInterface.h"
#include "xpSaiSwitch.h"
#include "xpSaiDev.h"
#include "xpSaiCopp.h"
#include "xpSaiUtil.h"
#include "xpSaiValidationArrays.h"
#include "xpsCopp.h"
#include "xpSaiPolicer.h"

sai_mac_t gcustConrolMacEntryList[] =
{
    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x00},
    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x02},
    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x03},
    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x04},
    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x05},
    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x06},
    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x07},
    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x08},
    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x09},
    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0A},
    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0B},
    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0C},
    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0D},
    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0E},
    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0F},
    {0x01, 0x00, 0x0C, 0xCC, 0xCC, 0xCD},
    {0x01, 0x00, 0x5E, 0x00, 0x00, 0x22},
    {0x01, 0x01, 0xE8, 0x00, 0xDE, 0x11},
};

#define gcustNumNonAclReasonCode (sizeof (gcustNonAclReasonCodeList) / sizeof (uint32_t))
#define gcustNumOfConrolMacEntry (sizeof (gcustConrolMacEntryList) / sizeof (sai_mac_t))

#define XP_SAI_MAX_PKT_TX_ATTR 2
#define XP_SAI_DEFAULT_DUMP_PKT_COUNT 10

typedef struct _xpSaiSoftFwdInfo_t
{
    bool            isValid;
    sai_mac_t       dstMac;
    sai_object_id_t outIntf;
} xpSaiSoftFwdInfo_t;

static xpSaiSoftFwdInfo_t gXpSaiSoftFwdInfo = {false, {0, 0, 0, 0, 0, 0}, 0};
static bool               gXpSaiPktDumpEnable = false;
static uint32_t           gXpSaiDumpAllowedPktCount =
    XP_SAI_DEFAULT_DUMP_PKT_COUNT;
static uint32_t           gXpSaiDumpedPktCount = 0;

XP_SAI_LOG_REGISTER_API(SAI_API_HOSTIF);

#define SAI_HOSTIF_TRAP_TYPE_SWITCH_CUSTOM(offset)      ((sai_hostif_trap_type_t)(SAI_HOSTIF_TRAP_TYPE_SWITCH_CUSTOM_RANGE_BASE + (offset)))
#define SAI_HOSTIF_TRAP_TYPE_ROUTER_CUSTOM(offset)      ((sai_hostif_trap_type_t)(SAI_HOSTIF_TRAP_TYPE_ROUTER_CUSTOM_RANGE_BASE + (offset)))
#define SAI_HOSTIF_TRAP_TYPE_LOCAL_IP_CUSTOM(offset)    ((sai_hostif_trap_type_t)(SAI_HOSTIF_TRAP_TYPE_LOCAL_IP_CUSTOM_RANGE_BASE + (offset)))
#define SAI_HOSTIF_TRAP_TYPE_CUSTOM_EXCEPTION(offset)   ((sai_hostif_trap_type_t)(SAI_HOSTIF_TRAP_TYPE_CUSTOM_EXCEPTION_RANGE_BASE + (offset)))
#define XP_SAI_TRAP_RC_LLDP                             850
#define XP_SAI_TRAP_RC_LACP                             851
#define XP_SAI_TRAP_RC_STP_BPDU                         852
#define XP_SAI_TRAP_RC_PVRST_BPDU                       853
#define XP_SAI_TRAP_RC_EAPOL                            854

#define XP_HOSTIF_DMA_QUEUE_NUM                         64

#define XP_SAI_HOST_INTF_MAX_IDS                 1024
#define XP_SAI_HOST_INTF_RANGE_START             1
#define XP_SAI_HOST_INTF_GRP_MAX_IDS             256
#define XP_SAI_HOST_INTF_GRP_RANGE_START         1
#define XP_SAI_HOST_INTF_TABLE_ENTRY_MAX_IDS     1024
#define XP_SAI_HOST_INTF_TABLE_ENTRY_RANGE_START 1


#define XP_SAI_TRAP_RC_CUST_ISCSI           150
#define SAI_HOSTIF_TRAP_TYPE_CUST_ISCSI     (sai_hostif_trap_type_t)XP_SAI_TRAP_RC_CUST_ISCSI

/**
 * \brief Packet buffer for CB channel
 *
 */
uint8_t packetBuf[XP_MAX_PACKET_SIZE];

/**
 * \brief Channel State Database Handle
 *
 */
static xpsDbHandle_t xpSaiHostIntfChannelDbHandle =
    XPSAI_HOST_INTF_CHANNEL_DB_HNDL;

/**
 * \brief Trap Group State Database Handle
 *
 */
static xpsDbHandle_t xpSaiHostIntfGroupDbHandle = XPSAI_HOST_INTF_GROUP_DB_HNDL;

/**
 * \brief Trap State Database Handle
 *
 */
static xpsDbHandle_t xpSaiHostIntfTrapDbHandle = XPSAI_HOST_INTF_TRAP_DB_HNDL;

/**
 * \brief Host Table State Database Handle
 *
 */
static xpsDbHandle_t xpSaiHostIntfTableDbHandle = XPSAI_HOST_INTF_TABLE_DB_HNDL;
#ifdef __cplusplus
extern "C" {
#endif
/**
 * \brief Policer State Database Handle
 *
 */
static xpsDbHandle_t xpSaiHostInterfacePolicerDbHandle =
    XPSAI_HOST_INTF_POLICER_DB_HNDL;

/**
 * \brief Pointer to Host Interface thread.
 *
 */
pthread_t gSaiHostIntfThread = (pthread_t)NULL;

/**
 * \brief Pointer to Host Interface thread.
 *
 */
pthread_t gSaiHostIntfTapThread = (pthread_t)NULL;
#ifdef __cplusplus
}
#endif



/**
 * \brief Trap default table
 *
 */
static xpSaiHostInterfaceTrapDefaultT xpSaiHostIntfDefaultTraps[] =
{
    /* Trap id                                            Action                     Mac address                           Reason                                  Enable */

    /** Switch trap **/
    {SAI_HOSTIF_TRAP_TYPE_STP,                        SAI_PACKET_ACTION_DROP,    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x00}, 1, {XP_SAI_TRAP_RC_STP_BPDU},           true},
    {SAI_HOSTIF_TRAP_TYPE_LACP,                       SAI_PACKET_ACTION_DROP,    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x02}, 1, {XP_SAI_TRAP_RC_LACP},               true},
    {SAI_HOSTIF_TRAP_TYPE_LLDP,                       SAI_PACKET_ACTION_DROP,    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0E}, 1, {XP_SAI_TRAP_RC_LLDP},               true},
    {SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_QUERY,            SAI_PACKET_ACTION_FORWARD, {0x01, 0x00, 0x5E, 0x00, 0x00, 0x22}, 1, {XP_BRIDGE_RC_IVIF_IGMP_CMD},        true},
    {SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_LEAVE,            SAI_PACKET_ACTION_FORWARD, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {XP_BRIDGE_RC_IVIF_IGMP_CMD},        true},
    {SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V1_REPORT,        SAI_PACKET_ACTION_FORWARD, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {XP_BRIDGE_RC_IVIF_IGMP_CMD},        true},
    {SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V2_REPORT,        SAI_PACKET_ACTION_FORWARD, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {XP_BRIDGE_RC_IVIF_IGMP_CMD},        true},
    {SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V3_REPORT,        SAI_PACKET_ACTION_FORWARD, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {XP_BRIDGE_RC_IVIF_IGMP_CMD},        true},

    /** Router traps **/
    {SAI_HOSTIF_TRAP_TYPE_ARP_REQUEST,                SAI_PACKET_ACTION_FORWARD,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0}, true},
    {SAI_HOSTIF_TRAP_TYPE_ARP_RESPONSE,               SAI_PACKET_ACTION_FORWARD,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},        true},
    {SAI_HOSTIF_TRAP_TYPE_DHCP_L2,                    SAI_PACKET_ACTION_FORWARD, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_DHCP,                       SAI_PACKET_ACTION_FORWARD, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_OSPF,                       SAI_PACKET_ACTION_FORWARD, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_PIM,                        SAI_PACKET_ACTION_FORWARD, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_VRRP,                       SAI_PACKET_ACTION_FORWARD, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_DHCPV6_L2,                  SAI_PACKET_ACTION_FORWARD, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_DHCPV6,                     SAI_PACKET_ACTION_FORWARD, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_OSPFV6,                     SAI_PACKET_ACTION_FORWARD, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_IPV6_NEIGHBOR_DISCOVERY,    SAI_PACKET_ACTION_FORWARD,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},       true},
    {SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_V2,             SAI_PACKET_ACTION_FORWARD,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_REPORT,         SAI_PACKET_ACTION_FORWARD,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_DONE,           SAI_PACKET_ACTION_FORWARD,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_MLD_V2_REPORT,              SAI_PACKET_ACTION_FORWARD,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_UNKNOWN_L3_MULTICAST,       SAI_PACKET_ACTION_DROP,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 false},

    {SAI_HOSTIF_TRAP_TYPE_SNMP,                       SAI_PACKET_ACTION_DROP,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_TTL_ERROR,                  SAI_PACKET_ACTION_DROP,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},     true},
    {SAI_HOSTIF_TRAP_TYPE_L3_MTU_ERROR,               SAI_PACKET_ACTION_DROP,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},       true},

    {SAI_HOSTIF_TRAP_TYPE_BGPV6,                      SAI_PACKET_ACTION_DROP,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_IP2ME,                      SAI_PACKET_ACTION_DROP,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},

    {SAI_HOSTIF_TRAP_TYPE_EAPOL,                      SAI_PACKET_ACTION_DROP,    {0x01, 0x80, 0xC2, 0x00, 0x00, 0x03}, 1, {XP_SAI_TRAP_RC_EAPOL},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_PVRST,                      SAI_PACKET_ACTION_DROP,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_SAMPLEPACKET,               SAI_PACKET_ACTION_DROP,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_BGP,                        SAI_PACKET_ACTION_DROP,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_VRRPV6,                     SAI_PACKET_ACTION_DROP,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_SSH,                        SAI_PACKET_ACTION_DROP,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_UDLD,                       SAI_PACKET_ACTION_DROP,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
    {SAI_HOSTIF_TRAP_TYPE_STATIC_FDB_MOVE,            SAI_PACKET_ACTION_DROP,    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 1, {0},                                 true},
};

static sai_hostif_api_t* _xpSaiHostInterfaceApi;

static sai_status_t xpSaiHostInterfaceTrapDefaultApply(xpsDevice_t devId,
                                                       sai_hostif_trap_type_t trapType);



static sai_status_t xpSaiCreateHostInterfaceTrapGroup(sai_object_id_t
                                                      *hostif_trap_group_id, sai_object_id_t switch_id,
                                                      uint32_t attr_count, const sai_attribute_t *attr_list);

static sai_status_t xpSaiHostInterfaceQueueApply(sai_object_id_t
                                                 hostif_trap_group_id, sai_uint32_t oldQueue);

static sai_status_t xpSaiHostInterfacePolicerApply(sai_object_id_t
                                                   hostif_trap_group_id,
                                                   sai_object_id_t oldPolicerOid);

static sai_status_t xpSaiHostInterfacePolicerEntryAdd(sai_object_id_t
                                                      policerOid);

static sai_status_t xpSaiHostInterfacePolicerEntryRemove(
    sai_object_id_t policerOid);

//Info: Internal api
//Func: xpSaiHostInterfaceTrapValidPortsInit
sai_status_t xpSaiHostInterfaceTrapValidPortsInit(xpsDevice_t devId,
                                                  xpSaiHostInterfaceTrapDbEntryT *pTrapEntry);

//Func: xpSaiHostInterfaceTrapValidPortsDeinit
sai_status_t xpSaiHostInterfaceTrapValidPortsDeinit(
    xpSaiHostInterfaceTrapDbEntryT *pTrapEntry);



//Func: xpSaiHostInterfaceGetXpsCoppCtrlPktType
sai_status_t xpSaiHostInterfaceGetXpsCoppCtrlPktType(
    sai_hostif_trap_type_t saiTrap, xpsCoppCtrlPkt_e* pXpsTrap)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    *pXpsTrap = XPS_COPP_CTRL_PKT_MAX;
    switch (saiTrap)
    {
        case SAI_HOSTIF_TRAP_TYPE_ARP_REQUEST:
            *pXpsTrap = XPS_COPP_CTRL_PKT_ARP_REQUEST;
            break;
        case SAI_HOSTIF_TRAP_TYPE_ARP_RESPONSE:
            *pXpsTrap = XPS_COPP_CTRL_PKT_ARP_RESPONSE;
            break;
        case SAI_HOSTIF_TRAP_TYPE_BGP:
            *pXpsTrap = XPS_COPP_CTRL_PKT_BGP;
            break;
        case SAI_HOSTIF_TRAP_TYPE_BGPV6:
            *pXpsTrap = XPS_COPP_CTRL_PKT_BGPV6;
            break;
        case SAI_HOSTIF_TRAP_TYPE_DHCP_L2:
            *pXpsTrap = XPS_COPP_CTRL_PKT_DHCP_L2;
            break;
        case SAI_HOSTIF_TRAP_TYPE_DHCP:
            *pXpsTrap = XPS_COPP_CTRL_PKT_DHCP;
            break;
        case SAI_HOSTIF_TRAP_TYPE_DHCPV6_L2:
            *pXpsTrap = XPS_COPP_CTRL_PKT_DHCPV6_L2;
            break;
        case SAI_HOSTIF_TRAP_TYPE_DHCPV6:
            *pXpsTrap = XPS_COPP_CTRL_PKT_DHCPV6;
            break;
        case SAI_HOSTIF_TRAP_TYPE_EAPOL:
            *pXpsTrap = XPS_COPP_CTRL_PKT_EAPOL;
            break;
        case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_LEAVE:
            *pXpsTrap = XPS_COPP_CTRL_PKT_IGMP_TYPE_LEAVE;
            break;
        case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_QUERY:
            *pXpsTrap = XPS_COPP_CTRL_PKT_IGMP_TYPE_QUERY;
            break;
        case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V1_REPORT:
            *pXpsTrap = XPS_COPP_CTRL_PKT_IGMP_TYPE_V1_REPORT;
            break;
        case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V2_REPORT:
            *pXpsTrap = XPS_COPP_CTRL_PKT_IGMP_TYPE_V2_REPORT;
            break;
        case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V3_REPORT:
            *pXpsTrap = XPS_COPP_CTRL_PKT_IGMP_TYPE_V3_REPORT;
            break;
        case SAI_HOSTIF_TRAP_TYPE_IP2ME:
            *pXpsTrap = XPS_COPP_CTRL_PKT_IP2ME;
            break;
        case SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_DONE:
            *pXpsTrap = XPS_COPP_CTRL_PKT_IPV6_MLD_V1_DONE;
            break;
        case SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_REPORT:
            *pXpsTrap = XPS_COPP_CTRL_PKT_IPV6_MLD_V1_REPORT;
            break;
        case SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_V2:
            *pXpsTrap = XPS_COPP_CTRL_PKT_IPV6_MLD_V1_V2;
            break;
        case SAI_HOSTIF_TRAP_TYPE_IPV6_NEIGHBOR_DISCOVERY:
            *pXpsTrap = XPS_COPP_CTRL_PKT_IPV6_NEIGHBOR_DISCOVERY;
            break;
        case SAI_HOSTIF_TRAP_TYPE_L3_MTU_ERROR:
            *pXpsTrap = XPS_COPP_CTRL_PKT_L3_MTU_ERROR;
            break;
        case SAI_HOSTIF_TRAP_TYPE_LACP:
            *pXpsTrap = XPS_COPP_CTRL_PKT_LACP;
            break;
        case SAI_HOSTIF_TRAP_TYPE_LLDP:
            *pXpsTrap = XPS_COPP_CTRL_PKT_LLDP;
            break;
        case SAI_HOSTIF_TRAP_TYPE_MLD_V2_REPORT:
            *pXpsTrap = XPS_COPP_CTRL_PKT_MLD_V2_REPORT;
            break;
        case SAI_HOSTIF_TRAP_TYPE_OSPF:
            *pXpsTrap = XPS_COPP_CTRL_PKT_OSPF;
            break;
        case SAI_HOSTIF_TRAP_TYPE_OSPFV6:
            *pXpsTrap = XPS_COPP_CTRL_PKT_OSPFV6;
            break;
        case SAI_HOSTIF_TRAP_TYPE_PIM:
            *pXpsTrap = XPS_COPP_CTRL_PKT_PIM;
            break;
        case SAI_HOSTIF_TRAP_TYPE_PVRST:
            *pXpsTrap = XPS_COPP_CTRL_PKT_PVRST;
            break;
        case SAI_HOSTIF_TRAP_TYPE_SAMPLEPACKET:
            *pXpsTrap = XPS_COPP_CTRL_PKT_SAMPLEPACKET;
            break;
        case SAI_HOSTIF_TRAP_TYPE_SNMP:
            *pXpsTrap = XPS_COPP_CTRL_PKT_SNMP;
            break;
        case SAI_HOSTIF_TRAP_TYPE_SSH:
            *pXpsTrap = XPS_COPP_CTRL_PKT_SSH;
            break;
        case SAI_HOSTIF_TRAP_TYPE_STP:
            *pXpsTrap = XPS_COPP_CTRL_PKT_STP;
            break;
        case SAI_HOSTIF_TRAP_TYPE_TTL_ERROR:
            *pXpsTrap = XPS_COPP_CTRL_PKT_TTL_ERROR;
            break;
        case SAI_HOSTIF_TRAP_TYPE_UDLD:
            *pXpsTrap = XPS_COPP_CTRL_PKT_UDLD;
            break;
        case SAI_HOSTIF_TRAP_TYPE_VRRP:
            *pXpsTrap = XPS_COPP_CTRL_PKT_VRRP;
            break;
        case SAI_HOSTIF_TRAP_TYPE_VRRPV6:
            *pXpsTrap = XPS_COPP_CTRL_PKT_VRRPV6;
            break;
        case SAI_HOSTIF_TRAP_TYPE_STATIC_FDB_MOVE :
            *pXpsTrap = XPS_COPP_CTRL_PKT_STATIC_FDB_MOVE;
            break;
        default:
            XP_SAI_LOG_ERR("Host Interface Trap %d is not supported\n", saiTrap);
            return SAI_STATUS_NOT_SUPPORTED;
    }
    return saiStatus;
}

//Func: xpSaiHostInterfaceUserTrapIdRangeGet

sai_status_t xpSaiHostInterfaceUserTrapIdRangeGet(uint32_t *trapIdMin,
                                                  uint32_t *trapIdMax)
{
    if (trapIdMin == NULL || trapIdMax == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *trapIdMin = XP_SAI_USER_DEFINED_TRAP_RC_MIN;
    *trapIdMax = XP_SAI_USER_DEFINED_TRAP_RC_MAX;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostInterfaceUserTrapReasonCodeByOid

sai_status_t xpSaiHostInterfaceUserTrapReasonCodeByOid(sai_object_id_t
                                                       userTrapOid, uint32_t *reasonCode)
{
    if (reasonCode == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *reasonCode = (uint32_t)xpSaiObjIdValueGet(userTrapOid);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiConvertTrapGroupOid

sai_status_t xpSaiConvertTrapGroupOid(sai_object_id_t trap_group_id,
                                      uint32_t* pId)
{
    if (pId == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(trap_group_id, SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(trap_group_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *pId = (uint32_t)xpSaiObjIdValueGet(trap_group_id);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiConvertHostIntfOid

sai_status_t xpSaiConvertHostIntfOid(sai_object_id_t hif_id, uint32_t *pIntf)
{
    if (pIntf == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(hif_id, SAI_OBJECT_TYPE_HOSTIF))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(hif_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *pIntf = (uint32_t)xpSaiObjIdValueGet(hif_id);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiConvertTrapOid

sai_status_t xpSaiConvertTrapOid(sai_object_id_t trapOid, uint32_t *pTrapType)
{
    if (pTrapType == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(trapOid, SAI_OBJECT_TYPE_HOSTIF_TRAP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(trapOid));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *pTrapType = (uint32_t)xpSaiObjIdValueGet(trapOid);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiConvertTableEntryOid

sai_status_t xpSaiConvertTableEntryOid(sai_object_id_t table_entry_id,
                                       uint32_t *pId)
{
    if (pId == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(table_entry_id,
                                  SAI_OBJECT_TYPE_HOSTIF_TABLE_ENTRY))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(table_entry_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *pId = (uint32_t)xpSaiObjIdValueGet(table_entry_id);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostIntfChannelKeyCompare

static int32_t xpSaiHostIntfChannelKeyCompare(void *key1, void *key2)
{
    return ((int32_t)(((xpSaiHostInterfaceChannelDbEntryT*)key1)->keyId) -
            (int32_t)(((xpSaiHostInterfaceChannelDbEntryT*)key2)->keyId));
}

//Func: xpSaiHostIntfTrapKeyCompare

static int32_t xpSaiHostIntfTrapKeyCompare(void *key1, void *key2)
{
    if (((xpSaiHostInterfaceTrapDbEntryT *)key1)->trapOid < ((
                                                                 xpSaiHostInterfaceTrapDbEntryT *)key2)->trapOid)
    {
        return -1;
    }
    else if (((xpSaiHostInterfaceTrapDbEntryT *)key1)->trapOid > ((
                                                                      xpSaiHostInterfaceTrapDbEntryT *)key2)->trapOid)
    {
        return 1;
    }
    return 0;
}

//Func: xpSaiHostIntfGroupKeyCompare

static int32_t xpSaiHostIntfGroupKeyCompare(void *key1, void *key2)
{
    return ((int32_t)(((xpSaiHostInterfaceGroupDbEntryT*)key1)->keyId) -
            (int32_t)(((xpSaiHostInterfaceGroupDbEntryT*)key2)->keyId));
}

//Func: xpSaiHostIntfTableKeyCompare

static int32_t xpSaiHostIntfTableKeyCompare(void *key1, void *key2)
{
    return ((int32_t)(((xpSaiHostInterfaceTableDbEntryT*)key1)->keyId) -
            (int32_t)(((xpSaiHostInterfaceTableDbEntryT*)key2)->keyId));
}

//Func: xpSaiHostInterfacePolicerDbEntryComp

static int32_t xpSaiHostInterfacePolicerDbEntryComp(void *key1, void *key2)
{
    const xpSaiHostInterfacePolicerDbEntry_t *entry1 =
        (xpSaiHostInterfacePolicerDbEntry_t*)key1;
    const xpSaiHostInterfacePolicerDbEntry_t *entry2 =
        (xpSaiHostInterfacePolicerDbEntry_t*)key2;

    return entry1->keyOid - entry2->keyOid;
}

//Func: xpSaiHostIntfDbInit

sai_status_t xpSaiHostIntfDbInit(xpsDevice_t xpsDevId)
{
    XP_STATUS       retVal     = XP_NO_ERR;
    xpsScope_t      scope      = xpSaiScopeFromDevGet(xpsDevId);

    retVal = xpsStateRegisterDb(scope, "HOST INTF CHANNEL DB", XPS_GLOBAL,
                                &xpSaiHostIntfChannelKeyCompare, xpSaiHostIntfChannelDbHandle);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not create Host Interface Channel DB");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateRegisterDb(scope, "HOST INTF TRAP DB", XPS_GLOBAL,
                                &xpSaiHostIntfTrapKeyCompare, xpSaiHostIntfTrapDbHandle);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not create Host Interface Trap DB");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateRegisterDb(scope, "HOST INTF GROUP DB", XPS_GLOBAL,
                                &xpSaiHostIntfGroupKeyCompare, xpSaiHostIntfGroupDbHandle);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not create Host Interface Group DB");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateRegisterDb(scope, "HOST INTF TABLE DB", XPS_GLOBAL,
                                &xpSaiHostIntfTableKeyCompare, xpSaiHostIntfTableDbHandle);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not create Host Interface Table DB");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "SAI Host Interface Policer DB",
                                XPS_GLOBAL, &xpSaiHostInterfacePolicerDbEntryComp,
                                xpSaiHostInterfacePolicerDbHandle);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateRegisterDb failed: status=%i.\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostIntfDbDeInit

sai_status_t xpSaiHostIntfDbDeInit(xpsDevice_t xpsDevId)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    xpsScope_t      scope       = xpSaiScopeFromDevGet(xpsDevId);

    retVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                  &xpSaiHostInterfacePolicerDbHandle);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateDeRegisterDb failed: status=%i.\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateDeRegisterDb(scope, &xpSaiHostIntfTrapDbHandle);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge Host Interface Trap DB");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateDeRegisterDb(scope, &xpSaiHostIntfGroupDbHandle);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge Host Interface Group DB");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateDeRegisterDb(scope, &xpSaiHostIntfChannelDbHandle);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge Host Interface Channel DB");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateDeRegisterDb(scope, &xpSaiHostIntfTableDbHandle);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge Host Interface Table DB");
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostIntfChannelDbInfoGet

sai_status_t xpSaiHostIntfChannelDbInfoGet(sai_object_id_t interface_id,
                                           xpSaiHostInterfaceChannelDbEntryT **ppChannelEntry)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_FAILURE;
    uint32_t        id          = 0;

    xpSaiHostInterfaceChannelDbEntryT interfaceKey;

    memset(&interfaceKey, 0, sizeof(interfaceKey));

    saiRetVal = xpSaiConvertHostIntfOid(interface_id, &id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertHostIntfOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    interfaceKey.keyId = id;

    retVal = xpsStateSearchData(xpSaiObjIdScopeGet(interface_id),
                                xpSaiHostIntfChannelDbHandle, &interfaceKey, (void**)ppChannelEntry);

    if (*ppChannelEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find Host Interface channel entry in DB");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiHostIntfGroupDbInfoGet

sai_status_t xpSaiHostIntfGroupDbInfoGet(sai_object_id_t trap_group_id,
                                         xpSaiHostInterfaceGroupDbEntryT **ppGrpEntry)
{
    XP_STATUS     retVal     = XP_NO_ERR;
    sai_status_t  saiRetVal  = SAI_STATUS_FAILURE;
    uint32_t      grpId      = 0;

    xpSaiHostInterfaceGroupDbEntryT grpKey;

    memset(&grpKey, 0, sizeof(grpKey));

    saiRetVal = xpSaiConvertTrapGroupOid(trap_group_id, &grpId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertTrapGroupOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    grpKey.keyId = grpId;

    retVal = xpsStateSearchData(xpSaiObjIdScopeGet(trap_group_id),
                                xpSaiHostIntfGroupDbHandle, &grpKey, (void**)ppGrpEntry);

    if (*ppGrpEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find Host Interface group entry in DB");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiHostIntfTrapDbInfoGet

sai_status_t xpSaiHostIntfTrapDbInfoGet(sai_object_id_t trapOid,
                                        xpSaiHostInterfaceTrapDbEntryT **ppTrapEntry)
{
    XP_STATUS     retVal     = XP_NO_ERR;

    xpSaiHostInterfaceTrapDbEntryT trapKey;

    memset(&trapKey, 0, sizeof(trapKey));
    trapKey.trapOid = trapOid;

    retVal = xpsStateSearchData(xpSaiObjIdScopeGet(trapOid),
                                xpSaiHostIntfTrapDbHandle, &trapKey, (void**)ppTrapEntry);

    if (*ppTrapEntry == NULL)
    {
        XP_SAI_LOG_NOTICE("Could not find Host Interface trap entry in DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiHostIntfTrapDbInfoGetNext

static sai_status_t xpSaiHostIntfTrapDbInfoGetNext(
    xpSaiHostInterfaceTrapDbEntryT *pTrapEntry,
    xpSaiHostInterfaceTrapDbEntryT **ppTrapEntry)
{
    XP_STATUS     retVal     = XP_NO_ERR;

    retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, xpSaiHostIntfTrapDbHandle,
                                 pTrapEntry, (void**)ppTrapEntry);

    if (*ppTrapEntry == NULL)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiHostIntfTrapDbInfoGet

sai_status_t xpSaiHostIntfTableDbInfoGet(sai_object_id_t table_entry_id,
                                         xpSaiHostInterfaceTableDbEntryT **ppTableEntry)
{
    XP_STATUS     retVal        = XP_NO_ERR;
    sai_status_t  saiRetVal     = SAI_STATUS_FAILURE;
    uint32_t      tableEntryId  = 0;

    xpSaiHostInterfaceTableDbEntryT tableEntryKey;

    memset(&tableEntryKey, 0, sizeof(tableEntryKey));

    saiRetVal = xpSaiConvertTableEntryOid(table_entry_id, &tableEntryId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertTableEntryOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    tableEntryKey.keyId = tableEntryId;

    retVal = xpsStateSearchData(xpSaiObjIdScopeGet(table_entry_id),
                                xpSaiHostIntfTableDbHandle, &tableEntryKey, (void**)ppTableEntry);

    if (*ppTableEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find Host Interface table entry in DB");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiHostIntfTableDbInfoGetNext

sai_status_t xpSaiHostIntfTableDbInfoGetNext(xpSaiHostInterfaceTableDbEntryT
                                             *pTableEntry, xpSaiHostInterfaceTableDbEntryT **ppTableEntry)
{
    XP_STATUS retVal = XP_NO_ERR;

    retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, xpSaiHostIntfTableDbHandle,
                                 pTableEntry, (void **)ppTableEntry);

    if (*ppTableEntry == NULL)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiHostInterfacePolicerDbEntryGet

static sai_status_t xpSaiHostInterfacePolicerDbEntryGet(
    sai_object_id_t policerOid, xpSaiHostInterfacePolicerDbEntry_t **entry)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpSaiHostInterfacePolicerDbEntry_t keyEntry;

    if (entry == NULL)
    {
        XP_SAI_LOG_ERR("NULL pointer: status=%i.\n", SAI_STATUS_INVALID_PARAMETER);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(&keyEntry, 0, sizeof(keyEntry));

    keyEntry.keyOid = policerOid;

    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT,
                                  xpSaiHostInterfacePolicerDbHandle, &keyEntry, (void**)entry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed: status=%i.\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (*entry == NULL)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostInterfacePolicerDbEntryInsert

static sai_status_t xpSaiHostInterfacePolicerDbEntryInsert(
    const xpSaiHostInterfacePolicerDbEntry_t *entry)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpSaiHostInterfacePolicerDbEntry_t *dataEntry = NULL, keyEntry;

    if (entry == NULL)
    {
        XP_SAI_LOG_ERR("NULL pointer: status=%i.\n", SAI_STATUS_INVALID_PARAMETER);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(&keyEntry, 0, sizeof(keyEntry));

    keyEntry.keyOid = entry->keyOid;

    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT,
                                  xpSaiHostInterfacePolicerDbHandle, &keyEntry, (void**)&dataEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed: status=%i.\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (dataEntry != NULL)
    {
        XP_SAI_LOG_ERR("Failed to insert Host Interface Policer DB entry: status=%i.\n",
                       SAI_STATUS_ITEM_ALREADY_EXISTS);
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    xpStatus = xpsStateHeapMalloc(sizeof(xpSaiHostInterfacePolicerDbEntry_t),
                                  (void**)&dataEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapMalloc failed: status=%i.\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    dataEntry->keyOid = entry->keyOid;
    dataEntry->data.acmIndex = entry->data.acmIndex;
    dataEntry->data.refCount = entry->data.refCount;

    xpStatus = xpsStateInsertData(XP_SCOPE_DEFAULT,
                                  xpSaiHostInterfacePolicerDbHandle, dataEntry);
    if (xpStatus != XP_NO_ERR)
    {
        xpStatus = xpsStateHeapFree(dataEntry);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsStateHeapFree failed: status=%i.\n", xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }

        XP_SAI_LOG_ERR("xpsStateInsertData failed: status=%i.\n", SAI_STATUS_FAILURE);
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostInterfacePolicerDbEntryRemove

static sai_status_t xpSaiHostInterfacePolicerDbEntryRemove(
    sai_object_id_t policerOid)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpSaiHostInterfacePolicerDbEntry_t *dataEntry = NULL, keyEntry;

    memset(&keyEntry, 0, sizeof(keyEntry));

    keyEntry.keyOid = policerOid;

    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT,
                                  xpSaiHostInterfacePolicerDbHandle, &keyEntry, (void**)&dataEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed: status=%i.\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (dataEntry == NULL)
    {
        return SAI_STATUS_SUCCESS;
    }

    xpStatus = xpsStateDeleteData(XP_SCOPE_DEFAULT,
                                  xpSaiHostInterfacePolicerDbHandle, &keyEntry, (void**)&dataEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateDeleteData failed: status=%i.\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsStateHeapFree(dataEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapFree failed: status=%i.\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostInterfaceCoppEntryAdd

static sai_status_t xpSaiHostInterfaceCoppEntryAdd(xpsDevice_t devId,
                                                   xpSaiHostInterfaceTrapDbEntryT *pTrapEntry,
                                                   xpsPktCmd_e pktCmd, xpsPktCmd_e defaultPktCmd)
{
    sai_status_t        saiRetVal       = SAI_STATUS_FAILURE;
    uint32_t            policerId       = XPSAI_INVALID_POLICER_ID;

    xpSaiHostInterfaceGroupDbEntryT     *pGrpEntry      = NULL;
    xpSaiHostInterfacePolicerDbEntry_t  *policerEntry   = NULL;
    sai_hostif_trap_type_t saiTrap;
    xpsCoppCtrlPkt_e xpsTrap = XPS_COPP_CTRL_PKT_MAX;
    /* Retrieve Trap Group from DB */
    saiRetVal = xpSaiHostIntfGroupDbInfoGet(pTrapEntry->groupId, &pGrpEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find a trap group in DB, error %d\n", saiRetVal);
        return saiRetVal;
    }

    /* Map reason code to CPU queue */
    saiTrap = (sai_hostif_trap_type_t)xpSaiObjIdValueGet(pTrapEntry->trapOid);
    saiRetVal = xpSaiHostInterfaceGetXpsCoppCtrlPktType(saiTrap, &xpsTrap);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get xpsTrap for saiTrap %d error %d\n", saiTrap,
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiCoppQueueMappingSet(devId, pGrpEntry->queue,
                                         (uint32_t)xpsTrap);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiCoppQueueMappingSet() failed for reason code %d with error code: %d\n",
                       saiTrap, saiRetVal);
        return saiRetVal;
    }

    if (pGrpEntry->policerOid != SAI_NULL_OBJECT_ID)
    {
        saiRetVal = xpSaiHostInterfacePolicerDbEntryGet(pGrpEntry->policerOid,
                                                        &policerEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiHostInterfacePolicerDbEntryGet failed: policerOid=%lu, status=%i.\n",
                           pGrpEntry->policerOid, saiRetVal);
            return saiRetVal;
        }

        policerId = policerEntry->data.acmIndex;
    }
    /* Create/Update CoPP Table Entry */
    saiRetVal = xpSaiCoppEntryAdd(devId, pTrapEntry->validPort, policerId, pktCmd,
                                  (uint32_t)xpsTrap, defaultPktCmd);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not add copp entry, reason code %d, error %d\n", saiTrap,
                       saiRetVal);
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostInterfaceUserTrapHwApply

static sai_status_t xpSaiHostInterfaceUserTrapHwApply(xpsDevice_t devId,
                                                      xpSaiHostInterfaceTrapDbEntryT *pTrapEntry)
{
    /* Create CoPP table entry for the user-defined trap.
     * Preserve packet action for the user-defined trap */
    return xpSaiHostInterfaceCoppEntryAdd(devId, pTrapEntry, XP_PKTCMD_MAX,
                                          XP_PKTCMD_MAX);
}

//Func: xpSaiHostInterfaceTrapHwApply

static sai_status_t xpSaiHostInterfaceTrapHwApply(xpsDevice_t devId,
                                                  xpSaiHostInterfaceTrapDbEntryT* pTrapEntry)
{
    sai_status_t        saiRetVal       = SAI_STATUS_FAILURE;
    xpsPktCmd_e         pktCmd          = XP_PKTCMD_DROP;
    xpsPktCmd_e         defaultPktCmd   = XP_PKTCMD_DROP;

    XP_STATUS           retVal          = XP_NO_ERR;
    //sai_mac_t           controlMac;
    //uint32_t            reasonCode      = 0;
    //xpsHashIndexList_t  indexList       = { 0 };
    xpSaiHostInterfaceTableDbEntryT *pTableEntry = NULL;
    xpSaiHostInterfaceTrapDefaultT  *pDefTrapEntry  = NULL;
    sai_hostif_trap_type_t          trapType        = SAI_HOSTIF_TRAP_TYPE_END;
    sai_hostif_trap_type_t saiTrap;
    xpsCoppCtrlPkt_e xpsTrap = XPS_COPP_CTRL_PKT_MAX;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    trapType = (sai_hostif_trap_type_t)xpSaiObjIdValueGet(pTrapEntry->trapOid);

    pDefTrapEntry = xpSaiHostIntfDefaultTrapGet(trapType);
    if (pDefTrapEntry == NULL)
    {
        XP_SAI_LOG_ERR("Invalid Host Interface Trap %d\n", trapType);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiConvertSaiPacketAction2xps(pTrapEntry->action, &pktCmd);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to convert trap action, error %d\n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiConvertSaiPacketAction2xps(pDefTrapEntry->action,
                                                &defaultPktCmd);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to convert trap default action, error %d\n", saiRetVal);
        return saiRetVal;
    }

    while (xpSaiHostIntfTableDbInfoGetNext(pTableEntry,
                                           &pTableEntry) == SAI_STATUS_SUCCESS)
    {
        if (pTableEntry->type == SAI_HOSTIF_TABLE_ENTRY_TYPE_WILDCARD &&
            pTableEntry->channelType ==
            SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_NETDEV_PHYSICAL_PORT)
        {
            //Install and enable global trap
            saiTrap = (sai_hostif_trap_type_t)xpSaiObjIdValueGet(pTrapEntry->trapOid);
            saiRetVal = xpSaiHostInterfaceGetXpsCoppCtrlPktType(saiTrap, &xpsTrap);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to get xpsTrap for saiTrap %d error %d\n", saiTrap,
                               saiRetVal);
                return saiRetVal;
            }
            retVal = xpsCoppEnableCtrlPktTrapOnSwitch(devId, xpsTrap, pktCmd);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("failed to add xpsTrap %u error %d\n", xpsTrap, retVal);
                return xpsStatus2SaiStatus(retVal);
            }
        }
    }
    /* Add CoPP table entry */
    saiRetVal = xpSaiHostInterfaceCoppEntryAdd(devId, pTrapEntry, pktCmd,
                                               defaultPktCmd);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not add copp entry, error %d\n", saiRetVal);
        return saiRetVal;
    }
    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiHostIntfDefaultTrapGet

xpSaiHostInterfaceTrapDefaultT *xpSaiHostIntfDefaultTrapGet(
    sai_hostif_trap_type_t trapType)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(xpSaiHostIntfDefaultTraps); i++)
    {
        if ((xpSaiHostIntfDefaultTraps[i].trapType == trapType) &&
            xpSaiHostIntfDefaultTraps[i].enable)
        {
            return &xpSaiHostIntfDefaultTraps[i];
        }
    }

    return NULL;
}

//Func: xpSaiHostInterfaceTrapDefaultApply

static sai_status_t xpSaiHostInterfaceTrapDefaultApply(xpsDevice_t devId,
                                                       sai_hostif_trap_type_t trapType)
{
    sai_status_t saiRetVal = SAI_STATUS_FAILURE;

    xpSaiHostInterfaceTrapDefaultT  *pDefTrapEntry  = NULL;
    xpSaiHostInterfaceTrapDbEntryT  trapEntry;

    pDefTrapEntry = xpSaiHostIntfDefaultTrapGet(trapType);
    if (pDefTrapEntry == NULL)
    {
        XP_SAI_LOG_ERR("Host Interface Trap %d is not supported\n", trapType);
        return SAI_STATUS_NOT_SUPPORTED;
    }

    memset(&trapEntry, 0, sizeof(xpSaiHostInterfaceTrapDbEntryT));

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_HOSTIF_TRAP, devId, trapType,
                                 &trapEntry.trapOid);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, retVal %d\n", saiRetVal);
        return saiRetVal;
    }

    trapEntry.action        = pDefTrapEntry->action;
    trapEntry.priority      = 0;

    xpSaiSwitchDefaultTrapGroupGet(&trapEntry.groupId);

    trapEntry.reasonCodeNum = pDefTrapEntry->reasonCodeNum;
    memcpy(trapEntry.reasonCode, pDefTrapEntry->reasonCode,
           sizeof(pDefTrapEntry->reasonCode));

    saiRetVal = xpSaiHostInterfaceTrapHwApply(devId, &trapEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHostInterfaceTrapHwApply() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetDefaultHostInterfaceAttributeVals

void xpSaiSetDefaultHostInterfaceAttributeVals(xpSaiHostInterfaceAttributesT*
                                               attributes)
{
    XP_SAI_LOG_DBG("Calling xpSaiSetDefaultHostInterfaceAttributeVals\n");
}

//Func: xpSaiUpdateHostInterfaceAttributeVals

void xpSaiUpdateHostInterfaceAttributeVals(const uint32_t attr_count,
                                           const sai_attribute_t* attr_list, xpSaiHostInterfaceAttributesT* attributes)
{
    XP_SAI_LOG_DBG("Calling xpSaiUpdateHostInterfaceAttributeVals\n");

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_HOSTIF_ATTR_TYPE:
                {
                    attributes->type = attr_list[count].value;
                    break;
                }
            case SAI_HOSTIF_ATTR_OBJ_ID:
                {
                    attributes->ifId = attr_list[count].value;
                    break;
                }
            case SAI_HOSTIF_ATTR_NAME:
                {
                    attributes->name = attr_list[count].value;
                    break;
                }
            case SAI_HOSTIF_ATTR_GENETLINK_MCGRP_NAME:
                {
                    attributes->mcgrpName = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Attr %d Not supported\n", attr_list[count].id);
                }
        }

    }

}

//Func: xpSaiCreateHostInterface
sai_status_t xpSaiCreateHostInterface(sai_object_id_t *hif_id,
                                      sai_object_id_t switch_id,
                                      uint32_t attr_count, const sai_attribute_t *attr_list)
{
    XP_STATUS           retVal      = XP_NO_ERR;
    sai_status_t        saiRetVal   = SAI_STATUS_FAILURE;
    uint32_t            intf        = 0;
    uint32_t            id          = 0;
    xpsDevice_t         devId       = xpSaiObjIdValueGet(switch_id);
    uint16_t            familyId    = 0;
    uint32_t            mcgrpId     = 0;

    xpSaiHostInterfaceChannelDbEntryT   *pChannelEntry = NULL;
    xpSaiHostInterfaceAttributesT       attributes;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               HOSTIF_VALIDATION_ARRAY_SIZE, hostif_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    xpSaiSetDefaultHostInterfaceAttributeVals(&attributes);
    xpSaiUpdateHostInterfaceAttributeVals(attr_count, attr_list, &attributes);

    retVal = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_HOST_INTF, &id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate Host Interface id, retVal %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_HOSTIF, devId, (sai_uint64_t)id,
                                 hif_id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, retVal %d", saiRetVal);
        retVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_HOST_INTF, id);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not release Host Interface id, retVal %d\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }
        return saiRetVal;
    }
    XP_SAI_LOG_DBG("SAI_OBJECT_TYPE_HOSTIF ObjId success id %d\n", hif_id);
    switch (attributes.type.s32)
    {
        case SAI_HOSTIF_TYPE_NETDEV:

            if (!(XDK_SAI_OBJID_TYPE_CHECK(attributes.ifId.oid, SAI_OBJECT_TYPE_PORT) ||
                  XDK_SAI_OBJID_TYPE_CHECK(attributes.ifId.oid, SAI_OBJECT_TYPE_LAG)  ||
                  XDK_SAI_OBJID_TYPE_CHECK(attributes.ifId.oid,
                                           SAI_OBJECT_TYPE_ROUTER_INTERFACE)))
            {
                XP_SAI_LOG_ERR("Wrong object type received(%u).",
                               xpSaiObjIdTypeGet(attributes.ifId.oid));
                return SAI_STATUS_INVALID_PARAMETER;
            }
            else if ((XDK_SAI_OBJID_TYPE_CHECK(attributes.ifId.oid, SAI_OBJECT_TYPE_LAG) ||
                      XDK_SAI_OBJID_TYPE_CHECK(attributes.ifId.oid,
                                               SAI_OBJECT_TYPE_ROUTER_INTERFACE)))
            {
                /*Sonic specific error , need to be moved to not supported array*/
                XP_SAI_LOG_ERR("Sonic not supported object type received(%u).",
                               xpSaiObjIdTypeGet(attributes.ifId.oid));
                return SAI_STATUS_INVALID_PARAMETER;
            }
            intf = (xpsInterfaceId_t)xpSaiObjIdValueGet(attributes.ifId.oid);

            XP_SAI_LOG_DBG("Creating netdev for intf id = %d, interface name = %s\n", intf,
                           (uint8_t*)attributes.name.chardata);
            saiRetVal = xpSaiHostIfNetdevCreate(devId, intf,
                                                (uint8_t *)attributes.name.chardata);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not create netdev interface, retVal %d", saiRetVal);
                retVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_HOST_INTF, id);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Could not release Host Interface id, retVal %d\n", retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                *hif_id = SAI_NULL_OBJECT_ID;
                return saiRetVal;
            }
            break;

        case SAI_HOSTIF_TYPE_FD:
            XP_SAI_LOG_ERR("Host Interface type FD not supported \n");
            retVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_HOST_INTF, id);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not release Host Interface id, retVal %d\n", retVal);
                return xpsStatus2SaiStatus(retVal);
            }
            *hif_id = SAI_NULL_OBJECT_ID;
            break;

        case SAI_HOSTIF_TYPE_GENETLINK:
            saiRetVal = xpSaiGenetlinkObjCreate((uint8_t *)attributes.name.chardata,
                                                (uint8_t *)attributes.mcgrpName.chardata, &intf, &familyId, &mcgrpId);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not create genetlink interface, retVal %d", saiRetVal);
                retVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_HOST_INTF, id);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Could not release Host Interface id, retVal %d\n", retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                *hif_id = SAI_NULL_OBJECT_ID;
                return saiRetVal;
            }
            break;
        default:
            XP_SAI_LOG_ERR("Attr type not available\n");
            return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateHeapMalloc(sizeof(xpSaiHostInterfaceChannelDbEntryT),
                                (void**)&pChannelEntry);
    if (retVal != XP_NO_ERR)
    {
        retVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_HOST_INTF, id);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not release Host Interface id, retVal %d\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }
        XP_SAI_LOG_ERR("Could not allocate channel structure\n");
        *hif_id = SAI_NULL_OBJECT_ID;
        return xpsStatus2SaiStatus(retVal);
    }

    memset(pChannelEntry, 0, sizeof(xpSaiHostInterfaceChannelDbEntryT));

    pChannelEntry->keyId = id;
    memcpy(pChannelEntry->name, attributes.name.chardata,
           sizeof(pChannelEntry->name));
    pChannelEntry->type = (sai_hostif_type_t)attributes.type.s32;
    pChannelEntry->vlanTagType = SAI_HOSTIF_VLAN_TAG_STRIP;
    pChannelEntry->intfFd = intf;
    memcpy(pChannelEntry->mcgrpName, attributes.mcgrpName.chardata,
           sizeof(pChannelEntry->mcgrpName));
    pChannelEntry->genetlinkFamilyId = familyId;
    pChannelEntry->mcgrpId = mcgrpId;

    retVal = xpsStateInsertData(XP_SCOPE_DEFAULT, xpSaiHostIntfChannelDbHandle,
                                (void*)pChannelEntry);
    if (retVal != XP_NO_ERR)
    {
        retVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_HOST_INTF, id);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not release Host Interface id, retVal %d\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }
        XP_SAI_LOG_ERR("Could not insert a channel structure into DB, error %d\n",
                       retVal);
        *hif_id = SAI_NULL_OBJECT_ID;
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveHostInterface

sai_status_t xpSaiRemoveHostInterface(sai_object_id_t hif_id)
{
    XP_STATUS           retVal    = XP_NO_ERR;
    sai_status_t        saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t         devId     = xpSaiObjIdSwitchGet(hif_id);
    uint32_t            id        = 0;
    xpSaiHostInterfaceChannelDbEntryT* pChannelEntry = NULL;
    xpSaiHostInterfaceChannelDbEntryT channelKey;

    memset(&channelKey, 0, sizeof(channelKey));

    saiRetVal = xpSaiHostIntfChannelDbInfoGet(hif_id, &pChannelEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find a channel %d in DB\n", id);
        return saiRetVal;
    }

    switch (pChannelEntry->type)
    {
        case SAI_HOSTIF_TYPE_NETDEV:
            {
                saiRetVal = xpSaiHostIfNetdevDelete(devId, pChannelEntry->intfFd);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Could not delete netdev, retVal %d", saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_HOSTIF_TYPE_FD:
            {
                XP_SAI_LOG_ERR("Host Interface type FD not supported \n");
                break;
            }
        case SAI_HOSTIF_TYPE_GENETLINK:
            {
                saiRetVal = xpSaiGenetlinkObjDelete(pChannelEntry->intfFd);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Could not delete genetlink, retVal %d", saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Invalid Host Interface type %d\n", pChannelEntry->type);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    retVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_HOST_INTF,
                                   pChannelEntry->keyId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not release Host Interface id, retVal %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    channelKey.keyId = pChannelEntry->keyId;

    retVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, xpSaiHostIntfChannelDbHandle,
                                &channelKey, (void**)&pChannelEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not delete channel structure from DB, id %d\n",
                       channelKey.keyId);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateHeapFree(pChannelEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not deallocate channel structure, id %d\n",
                       channelKey.keyId);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetHostInterfaceAttribute

static sai_status_t xpSaiSetHostInterfaceAttribute(sai_object_id_t hif_id,
                                                   const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS    status = XP_NO_ERR;
    xpsDevice_t  devId     = xpSaiObjIdSwitchGet(hif_id);
    retVal = xpSaiAttrCheck(1, attr,
                            HOSTIF_VALIDATION_ARRAY_SIZE, hostif_attribs,
                            SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }
    xpSaiHostInterfaceChannelDbEntryT* pChannelEntry = NULL;
    xpSaiHostInterfaceChannelDbEntryT channelKey;

    memset(&channelKey, 0, sizeof(channelKey));
    retVal = xpSaiHostIntfChannelDbInfoGet(hif_id, &pChannelEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find a channel %d in DB\n", hif_id);
        return retVal;
    }
    switch (attr->id)
    {
        case SAI_HOSTIF_ATTR_OPER_STATUS:
            {
                status = xpSaiHostIfLinkStatusSet(devId, pChannelEntry->intfFd,
                                                  attr->value.booldata);
                if (status != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiHostIfLinkStatusSet failed, id %d\n",
                                   pChannelEntry->keyId);
                    return xpsStatus2SaiStatus(status);
                }
                XP_SAI_LOG_DBG("xpSaiHostIfLinkStatusSet success retVal %d\n", status);
                break;

            }
        case SAI_HOSTIF_ATTR_VLAN_TAG:
            {
                status = xpSaiHostIfVlanTagTypeSet(devId, pChannelEntry->intfFd,
                                                   (sai_hostif_vlan_tag_t)attr->value.s32);
                if (status != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiHostIfVlanTagTypeSet failed, id %d\n",
                                   pChannelEntry->intfFd);
                    return xpsStatus2SaiStatus(status);
                }
                pChannelEntry->vlanTagType = (sai_hostif_vlan_tag_t)attr->value.s32;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("[skip] Failed to set %d\n", attr->id);
                return SAI_STATUS_SUCCESS;
            }
    }
    return  retVal;
}

//Func: xpSaiGetHostInterfaceAttrName

sai_status_t xpSaiGetHostInterfaceAttrName(sai_object_id_t hif_id,
                                           sai_attribute_value_t* value)
{
    sai_status_t        saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t            id        = 0;
    xpSaiHostInterfaceChannelDbEntryT* pChannelEntry = NULL;

    saiRetVal = xpSaiConvertHostIntfOid(hif_id, &id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertHostIntfOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiHostIntfChannelDbInfoGet(hif_id, &pChannelEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find a channel %d in DB\n", id);
        return saiRetVal;
    }

    strncpy(value->chardata, pChannelEntry->name, SAI_HOSTIF_NAME_SIZE);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetHostInterfaceAttrType

sai_status_t xpSaiGetHostInterfaceAttrType(sai_object_id_t hif_id,
                                           sai_attribute_value_t* value)
{
    sai_status_t        saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t            id        = 0;
    xpSaiHostInterfaceChannelDbEntryT* pChannelEntry = NULL;

    saiRetVal = xpSaiConvertHostIntfOid(hif_id, &id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertHostIntfOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiHostIntfChannelDbInfoGet(hif_id, &pChannelEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find a channel %d in DB\n", id);
        return saiRetVal;
    }

    value->s32 = pChannelEntry->type;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetHostInterfaceAttrTagType

sai_status_t xpSaiGetHostInterfaceAttrTagType(sai_object_id_t hif_id,
                                              sai_attribute_value_t* value)
{
    sai_status_t        saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t            id        = 0;
    xpSaiHostInterfaceChannelDbEntryT* pChannelEntry = NULL;

    saiRetVal = xpSaiConvertHostIntfOid(hif_id, &id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertHostIntfOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiHostIntfChannelDbInfoGet(hif_id, &pChannelEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find a channel %d in DB\n", id);
        return saiRetVal;
    }

    value->s32 = pChannelEntry->vlanTagType;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetHostInterfaceAttrGenetlinkMcgrpName

sai_status_t xpSaiGetHostInterfaceAttrGenetlinkMcgrpName(sai_object_id_t hif_id,
                                                         sai_attribute_value_t* value)
{
    sai_status_t        saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t            id        = 0;
    xpSaiHostInterfaceChannelDbEntryT* pChannelEntry = NULL;

    saiRetVal = xpSaiConvertHostIntfOid(hif_id, &id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertHostIntfOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiHostIntfChannelDbInfoGet(hif_id, &pChannelEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find a channel %d in DB\n", id);
        return saiRetVal;
    }

    strncpy(value->chardata, pChannelEntry->mcgrpName,
            SAI_HOSTIF_GENETLINK_MCGRP_NAME_SIZE);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetHostInterfaceAttrIfId

sai_status_t xpSaiGetHostInterfaceAttrIfId(sai_object_id_t hif_id,
                                           sai_attribute_value_t* value)
{
    XP_STATUS                          retVal    = XP_NO_ERR;
    sai_status_t                       saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t                           id        = 0;
    xpSaiHostInterfaceChannelDbEntryT *pChannelEntry = NULL;
    xpsInterfaceType_e                 intfType        = XPS_PORT;
    xpsDevice_t                        devId = xpSaiGetDevId();

    saiRetVal = xpSaiConvertHostIntfOid(hif_id, &id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertHostIntfOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiHostIntfChannelDbInfoGet(hif_id, &pChannelEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find a channel %d in DB\n", id);
        return saiRetVal;
    }

    retVal = xpsInterfaceGetType(pChannelEntry->intfFd, &intfType);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Interface Get type for interface %d failed, error %d\n",
                       pChannelEntry->intfFd, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (intfType == XPS_PORT)
    {
        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                     (sai_uint64_t) pChannelEntry->intfFd, &value->oid);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI port object could not be created, error %d\n", saiRetVal);
            return saiRetVal;
        }
    }
    else if (intfType == XPS_LAG)
    {
        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_LAG, devId,
                                     (sai_uint64_t) pChannelEntry->intfFd, &value->oid);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI LAG object could not be created, error %d\n", saiRetVal);
            return saiRetVal;
        }
    }
    else if (intfType == XPS_VLAN_ROUTER)
    {
        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_ROUTER_INTERFACE, devId,
                                     (sai_uint64_t) pChannelEntry->intfFd, &value->oid);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI LAG object could not be created, error %d\n", saiRetVal);
            return saiRetVal;
        }
    }
    else
    {
        XP_SAI_LOG_ERR("Unsupported interface type, intf %d, type %d\n",
                       pChannelEntry->intfFd, intfType);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiGetHostInterfaceAttribute

static sai_status_t xpSaiGetHostInterfaceAttribute(sai_object_id_t hif_id,
                                                   sai_attribute_t *attr, uint32_t attr_index)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiGetHostInterfaceAttribute\n");

    switch (attr->id)
    {
        case SAI_HOSTIF_ATTR_TYPE:
            {
                saiRetVal = xpSaiGetHostInterfaceAttrType(hif_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_HOSTIF_ATTR_TYPE)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_HOSTIF_ATTR_OBJ_ID:
            {
                saiRetVal = xpSaiGetHostInterfaceAttrIfId(hif_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_HOSTIF_ATTR_OBJ_ID)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_HOSTIF_ATTR_NAME:
            {
                saiRetVal = xpSaiGetHostInterfaceAttrName(hif_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_HOSTIF_ATTR_NAME)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_HOSTIF_ATTR_VLAN_TAG:
            {
                saiRetVal = xpSaiGetHostInterfaceAttrTagType(hif_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_HOSTIF_VLAN_TAG)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_HOSTIF_ATTR_OPER_STATUS:
            {
                XP_SAI_LOG_ERR("Attribute %d is not implemented\n", attr->id);
                return SAI_STATUS_ATTR_NOT_IMPLEMENTED_0 + SAI_STATUS_CODE(attr_index);
            }
        case SAI_HOSTIF_ATTR_GENETLINK_MCGRP_NAME:
            {
                saiRetVal = xpSaiGetHostInterfaceAttrGenetlinkMcgrpName(hif_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_HOSTIF_ATTR_GENETLINK_MCGRP_NAME)\n");
                    return saiRetVal;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Attribute %d is unknown\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(attr_index);
            }
    }
    return saiRetVal;
}

//Func: xpSaiGetHostInterfaceAttributes

static sai_status_t xpSaiGetHostInterfaceAttributes(sai_object_id_t hif_id,
                                                    uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               HOSTIF_VALIDATION_ARRAY_SIZE, hostif_attribs,
                               SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetHostInterfaceAttribute(hif_id, &attr_list[count], count);
        if (SAI_STATUS_SUCCESS != saiRetVal)
        {
            XP_SAI_LOG_ERR("failed for hostInterfaceId %" PRIu64 "\n", hif_id);
            return saiRetVal;
        }
    }

    return saiRetVal;
}

//Func: xpSaiBulkGetHostInterfaceAttributes

sai_status_t xpSaiBulkGetHostInterfaceAttributes(sai_object_id_t id,
                                                 uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_HOSTIF))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountHostInterfaceAttribute(&maxcount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get max attribute count!\n");
        return SAI_STATUS_FAILURE;
    }

    if (*attr_count < maxcount)
    {
        *attr_count = maxcount;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    for (uint32_t count = 0; count < maxcount; count++)
    {
        attr_list[idx].id = SAI_HOSTIF_ATTR_START + count;
        saiRetVal = xpSaiGetHostInterfaceAttribute(id, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
        else if (saiRetVal == SAI_STATUS_ITEM_NOT_FOUND)
        {
            /* Host interface object not found in DB */
            XP_SAI_LOG_ERR("failed for object ID %lu\n", id);
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

//Func: void xpSaiSetDefaultHostInterfaceTableAttributeVals

static void xpSaiSetDefaultHostInterfaceTableAttributeVals(
    xpSaiHostInterfaceTableAttributesT *attributes)
{
    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    memset(attributes, 0, sizeof(xpSaiHostInterfaceTableAttributesT));
}

//Func: xpSaiUpdateHostInterfaceAttributeVals

static sai_status_t xpSaiUpdateHostInterfaceTableAttributeVals(
    const uint32_t attr_count, const sai_attribute_t *attr_list,
    xpSaiHostInterfaceTableAttributesT *attributes)
{
    const sai_attribute_t           *attr       = NULL;
    sai_hostif_table_entry_type_t   entryType   = SAI_HOSTIF_TABLE_ENTRY_TYPE_PORT;
    uint32_t                        attrIdx     = 0;
    sai_object_type_t               objType     = SAI_OBJECT_TYPE_NULL;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    attr = xpSaiFindAttrById(SAI_HOSTIF_TABLE_ENTRY_ATTR_TYPE, attr_count,
                             attr_list, NULL);
    if (attr == NULL)
    {
        XP_SAI_LOG_ERR("SAI_HOSTIF_TABLE_ENTRY_ATTR_TYPE Not Found");
        return SAI_STATUS_FAILURE;
    }

    attributes->type = attr->value;
    entryType = (sai_hostif_table_entry_type_t)attributes->type.s32;

    if (entryType != SAI_HOSTIF_TABLE_ENTRY_TYPE_WILDCARD)
    {
        attr = xpSaiFindAttrById(SAI_HOSTIF_TABLE_ENTRY_ATTR_TRAP_ID, attr_count,
                                 attr_list, &attrIdx);
        if (attr == NULL)
        {
            XP_SAI_LOG_ERR("SAI_HOSTIF_TABLE_ENTRY_ATTR_TRAP_ID Not Found");
            return SAI_STATUS_FAILURE;
        }

        if (xpSaiObjIdTypeGet(attr->value.oid) != SAI_OBJECT_TYPE_HOSTIF_TRAP &&
            xpSaiObjIdTypeGet(attr->value.oid) != SAI_OBJECT_TYPE_HOSTIF_USER_DEFINED_TRAP)
        {
            XP_SAI_LOG_ERR("Invalid Host Interface trap ID\n");
            return SAI_STATUS_INVALID_ATTR_VALUE_0 + attrIdx;
        }
        attributes->trapId = attr->value;

        /* SAI_HOSTIF_TABLE_ENTRY_TYPE_PORT | SAI_HOSTIF_TABLE_ENTRY_TYPE_LAG | SAI_HOSTIF_TABLE_ENTRY_TYPE_VLAN */
        if (entryType != SAI_HOSTIF_TABLE_ENTRY_TYPE_TRAP_ID)
        {
            attr = xpSaiFindAttrById(SAI_HOSTIF_TABLE_ENTRY_ATTR_OBJ_ID, attr_count,
                                     attr_list, &attrIdx);

            if (attr == NULL)
            {
                XP_SAI_LOG_ERR("SAI_HOSTIF_TABLE_ENTRY_ATTR_OBJ_ID Not Found");
                return SAI_STATUS_FAILURE;
            }
            objType = xpSaiObjIdTypeGet(attr->value.oid);

            if ((entryType == SAI_HOSTIF_TABLE_ENTRY_TYPE_PORT &&
                 objType != SAI_OBJECT_TYPE_PORT) ||
                (entryType == SAI_HOSTIF_TABLE_ENTRY_TYPE_LAG &&
                 objType != SAI_OBJECT_TYPE_LAG) ||
                (entryType == SAI_HOSTIF_TABLE_ENTRY_TYPE_VLAN &&
                 objType != SAI_OBJECT_TYPE_ROUTER_INTERFACE))
            {
                XP_SAI_LOG_ERR("Host Interface Table Entry type and object ID mismatch");
                return SAI_STATUS_INVALID_ATTR_VALUE_0 + attrIdx;
            }

            attributes->objId = attr->value;
        }
    }

    attr = xpSaiFindAttrById(SAI_HOSTIF_TABLE_ENTRY_ATTR_CHANNEL_TYPE, attr_count,
                             attr_list, NULL);
    if (attr == NULL)
    {
        XP_SAI_LOG_ERR("SAI_HOSTIF_TABLE_ENTRY_ATTR_CHANNEL_TYPE Not Found");
        return SAI_STATUS_FAILURE;
    }

    attributes->channelType = attr->value;

    if ((attributes->channelType.s32 == SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_FD) ||
        (attributes->channelType.s32 == SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_GENETLINK))
    {
        attr = xpSaiFindAttrById(SAI_HOSTIF_TABLE_ENTRY_ATTR_HOST_IF, attr_count,
                                 attr_list, &attrIdx);
        if (attr == NULL)
        {
            XP_SAI_LOG_ERR("SAI_HOSTIF_TABLE_ENTRY_ATTR_HOST_IF Not Found");
            return SAI_STATUS_FAILURE;
        }

        if (xpSaiObjIdTypeGet(attr->value.oid) != SAI_OBJECT_TYPE_HOSTIF)
        {
            XP_SAI_LOG_ERR("Invalid Host Interface FD object\n");
            return SAI_STATUS_INVALID_ATTR_VALUE_0 + attrIdx;
        }

        attributes->hostIf = attr->value;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCreateHostInterfaceTableEntry

static sai_status_t xpSaiCreateHostInterfaceTableEntry(sai_object_id_t
                                                       *hif_table_entry_id, sai_object_id_t switch_id,
                                                       uint32_t attr_count, const sai_attribute_t *attr_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        id          = 0;
    xpsDevice_t     devId       = xpSaiObjIdValueGet(switch_id);
    xpsScope_t      scope       = xpSaiScopeFromDevGet(devId);
    sai_hostif_trap_type_t saiTrap;
    xpsCoppCtrlPkt_e xpsTrap = XPS_COPP_CTRL_PKT_MAX;
    xpsPktCmd_e     pktCmd          = XP_PKTCMD_DROP;

    xpSaiHostInterfaceTableDbEntryT     *pTableEntry    = NULL;
    xpSaiHostInterfaceTableAttributesT  attributes;
    xpSaiHostInterfaceTrapDbEntryT      *pTrapEntry     = NULL;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               HOSTIF_TABLE_ENTRY_VALIDATION_ARRAY_SIZE, hostif_table_entry_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    xpSaiSetDefaultHostInterfaceTableAttributeVals(&attributes);

    saiRetVal = xpSaiUpdateHostInterfaceTableAttributeVals(attr_count, attr_list,
                                                           &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to process attribute list, retVal %d\n", saiRetVal);
        return saiRetVal;
    }

    retVal = xpsStateHeapMalloc(sizeof(xpSaiHostInterfaceTableDbEntryT),
                                (void**)&pTableEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate Table Entry structure\n");
        return xpsStatus2SaiStatus(retVal);
    }

    memset(pTableEntry, 0, sizeof(xpSaiHostInterfaceTableDbEntryT));

    pTableEntry->type = (sai_hostif_table_entry_type_t)attributes.type.s32;
    pTableEntry->interfaceObjId = attributes.objId.oid;
    pTableEntry->trapId = attributes.trapId.oid;
    pTableEntry->channelType = (sai_hostif_table_entry_channel_type_t)
                               attributes.channelType.s32;
    pTableEntry->hostFd = attributes.hostIf.oid;

    /* Allocate Host Table Entry IDs */
    if (pTableEntry->type == SAI_HOSTIF_TABLE_ENTRY_TYPE_WILDCARD)
    {
        retVal = xpsAllocatorAllocateId(scope, XP_SAI_ALLOC_HOST_INTF_TABLE_ENTRY, &id);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not allocate Host Interface Table Entry id, retVal %d\n",
                           retVal);
            xpsStateHeapFree(pTableEntry);
            return xpsStatus2SaiStatus(retVal);
        }
        pTableEntry->tableEntryNum = 1;
        pTableEntry->tableEntry[0].reasonCode = 0;
        pTableEntry->tableEntry[0].tableEntryId = id;
        if (pTableEntry->channelType ==
            SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_NETDEV_PHYSICAL_PORT)
        {
            while (xpSaiHostIntfTrapDbInfoGetNext(pTrapEntry,
                                                  &pTrapEntry) == SAI_STATUS_SUCCESS)
            {
                //Install global trap
                saiTrap = (sai_hostif_trap_type_t)xpSaiObjIdValueGet(pTrapEntry->trapOid);
                saiRetVal = xpSaiHostInterfaceGetXpsCoppCtrlPktType(saiTrap, &xpsTrap);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to get xpsTrap for saiTrap %d error %d\n", saiTrap,
                                   saiRetVal);
                    xpsStateHeapFree(pTableEntry);
                    return saiRetVal;
                }
                saiRetVal = xpSaiConvertSaiPacketAction2xps(pTrapEntry->action, &pktCmd);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to convert trap action, error %d\n", saiRetVal);
                    xpsStateHeapFree(pTableEntry);
                    return saiRetVal;
                }
                retVal = xpsCoppEnableCtrlPktTrapOnSwitch(devId, xpsTrap, pktCmd);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("failed to add xpsTrap %u error %d\n", xpsTrap, retVal);
                    xpsStateHeapFree(pTableEntry);
                    return xpsStatus2SaiStatus(retVal);
                }
            }
        }
    }
    else
    {
        if (pTableEntry->channelType == SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_GENETLINK)
        {
            retVal = xpsAllocatorAllocateId(scope, XP_SAI_ALLOC_HOST_INTF_TABLE_ENTRY, &id);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not allocate Host Interface Table Entry id, retVal %d\n",
                               retVal);
                xpsStateHeapFree(pTableEntry);
                return xpsStatus2SaiStatus(retVal);
            }
            pTableEntry->tableEntryNum = 1;
            pTableEntry->tableEntry[0].reasonCode = 0;
            pTableEntry->tableEntry[0].tableEntryId = id;

            saiRetVal = xpSaiHostIntfTrapDbInfoGet(pTableEntry->trapId, &pTrapEntry);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not find Host Interface Trap %" PRIu64 " in DB\n",
                               xpSaiObjIdValueGet(pTableEntry->trapId));
                xpsStateHeapFree(pTableEntry);
                return saiRetVal;
            }
            saiTrap = (sai_hostif_trap_type_t)xpSaiObjIdValueGet(pTrapEntry->trapOid);
            saiRetVal = xpSaiHostInterfaceGetXpsCoppCtrlPktType(saiTrap, &xpsTrap);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to get xpsTrap for saiTrap %d error %d\n", saiTrap,
                               saiRetVal);
                xpsStateHeapFree(pTableEntry);
                return saiRetVal;
            }

            /* Get hostif info */
            xpSaiHostInterfaceChannelDbEntryT* pChannelEntry = NULL;
            saiRetVal = xpSaiHostIntfChannelDbInfoGet(pTableEntry->hostFd, &pChannelEntry);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not find a channel %d in DB\n", pTableEntry->hostFd);
                return saiRetVal;
            }
            /* Create hostIf table entry for genetlink */
            retVal = xpsHostIfTableGenetlinkCreate(devId,
                                                   xpsTrap,
                                                   pChannelEntry->intfFd,
                                                   pChannelEntry->genetlinkFamilyId,
                                                   pChannelEntry->mcgrpId);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not create a Table Entry for genetlink, error %d\n",
                               retVal);
                return xpsStatus2SaiStatus(retVal);
            }
        }
        else
        {
            saiRetVal = xpSaiHostIntfTrapDbInfoGet(pTableEntry->trapId, &pTrapEntry);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not find Host Interface Trap %" PRIu64 " in DB\n",
                               xpSaiObjIdValueGet(pTableEntry->trapId));
                xpsStateHeapFree(pTableEntry);
                return saiRetVal;
            }

            /* Initialize trap table entries */
            pTableEntry->tableEntryNum = pTrapEntry->reasonCodeNum;
            for (uint32_t i = 0; i < pTrapEntry->reasonCodeNum; i++)
            {
                /* Multiple XDK reason codes can be mapped into single SAI trap ID.
                * The trap table - located in the driver - uses unique key/ID
                * for each entry created for a specific reason code. This unique key
                * is allocated by XDK allocator manager through
                * allocateHostIntfTableEntry(). */
                retVal = xpsAllocatorAllocateId(scope, XP_SAI_ALLOC_HOST_INTF_TABLE_ENTRY, &id);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Could not allocate Host Interface Table Entry id, retVal %d\n",
                                   retVal);
                    xpsStateHeapFree(pTableEntry);
                    return xpsStatus2SaiStatus(retVal);
                }

                pTableEntry->tableEntry[i].reasonCode = pTrapEntry->reasonCode[i];
                pTableEntry->tableEntry[i].tableEntryId = id;
            }
        }
    }
    pTableEntry->keyId = pTableEntry->tableEntry[0].tableEntryId;
    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_HOSTIF_TABLE_ENTRY, devId,
                                 (sai_uint64_t)pTableEntry->keyId, hif_table_entry_id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, retVal %d\n", saiRetVal);
        xpsStateHeapFree(pTableEntry);
        return saiRetVal;
    }

    retVal = xpsStateInsertData(scope, xpSaiHostIntfTableDbHandle,
                                (void *)pTableEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not insert a Table Entry structure into DB, error %d\n",
                       retVal);
        xpsStateHeapFree(pTableEntry);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveHostInterfaceTableEntry

static sai_status_t xpSaiRemoveHostInterfaceTableEntry(sai_object_id_t
                                                       hif_table_entry)
{
    XP_STATUS       retVal          = XP_NO_ERR;
    sai_status_t    saiRetVal       = SAI_STATUS_FAILURE;
    uint32_t        tableEntryId    = 0;
    sai_hostif_trap_type_t saiTrap;
    xpsCoppCtrlPkt_e xpsTrap = XPS_COPP_CTRL_PKT_MAX;
    xpsPktCmd_e     pktCmd          = XP_PKTCMD_DROP;
    xpsDevice_t     devId           = xpSaiObjIdSwitchGet(hif_table_entry);
    xpsScope_t      scope           = xpSaiObjIdScopeGet(hif_table_entry);
    xpSaiHostInterfaceTrapDefaultT  *pDefTrapEntry  = NULL;
    xpSaiHostInterfaceTableDbEntryT *pTableEntry = NULL;
    xpSaiHostInterfaceTrapDbEntryT  *pTrapEntry  = NULL;
    xpSaiHostInterfaceTableDbEntryT tableEntryKey;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    memset(&tableEntryKey, 0, sizeof(xpSaiHostInterfaceTableDbEntryT));

    saiRetVal = xpSaiConvertTableEntryOid(hif_table_entry, &tableEntryId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertTableEntryOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    XP_SAI_LOG_DBG("Table Entry %d is being removed\n", tableEntryId);

    tableEntryKey.keyId = tableEntryId;

    retVal = xpsStateDeleteData(scope, xpSaiHostIntfTableDbHandle, &tableEntryKey,
                                (void**)&pTableEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not delete Table Entry %d from DB\n", tableEntryId);
        return xpsStatus2SaiStatus(retVal);
    }
    if (pTableEntry == NULL)
    {
        XP_SAI_LOG_ERR("Table Entry %d Not Found\n", tableEntryId);
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);
    }

    if (pTableEntry->type == SAI_HOSTIF_TABLE_ENTRY_TYPE_WILDCARD)
    {
        if (pTableEntry->channelType ==
            SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_NETDEV_PHYSICAL_PORT)
        {
            while (xpSaiHostIntfTrapDbInfoGetNext(pTrapEntry,
                                                  &pTrapEntry) == SAI_STATUS_SUCCESS)
            {
                //Install default values for trap types
                saiTrap = (sai_hostif_trap_type_t)xpSaiObjIdValueGet(pTrapEntry->trapOid);
                pDefTrapEntry = xpSaiHostIntfDefaultTrapGet(saiTrap);
                if (pDefTrapEntry == NULL)
                {
                    XP_SAI_LOG_ERR("Invalid Host Interface Trap %d\n", saiTrap);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
                saiRetVal = xpSaiHostInterfaceGetXpsCoppCtrlPktType(saiTrap, &xpsTrap);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to get xpsTrap for saiTrap %d error %d\n", saiTrap,
                                   saiRetVal);
                    return saiRetVal;
                }
                saiRetVal = xpSaiConvertSaiPacketAction2xps(pDefTrapEntry->action, &pktCmd);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to convert trap action, error %d\n", saiRetVal);
                    return saiRetVal;
                }
                retVal = xpsCoppEnableCtrlPktTrapOnSwitch(devId, xpsTrap, pktCmd);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("failed to add xpsTrap %u error %d\n", xpsTrap, retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
            }
        }
    }
    /* Release allocated IDs */
    for (uint32_t i = 0; i < pTableEntry->tableEntryNum; i++)
    {
        retVal = xpsAllocatorReleaseId(scope, XP_SAI_ALLOC_HOST_INTF_TABLE_ENTRY,
                                       pTableEntry->tableEntry[i].tableEntryId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not release Host Interface Table Entry ID #%u, retVal %d\n",
                           pTableEntry->tableEntry[i].tableEntryId, retVal);
        }
        /* Release hostIf table entry for genetlink */
        if (pTableEntry->channelType == SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_GENETLINK)
        {
            if (pTableEntry->tableEntry[i].reasonCode > 0)
            {
                retVal = xpsHostIfTableGenetlinkDelete(devId,
                                                       pTableEntry->tableEntry[i].reasonCode);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Could not release Host Interface Table Entry ID #%u for genetlink, retVal %d\n",
                                   pTableEntry->tableEntry[i].tableEntryId, retVal);
                }
            }
        }
    }
    retVal = xpsStateHeapFree(pTableEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not de-allocate Table Entry %d\n", tableEntryId);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetHostInterfaceTableEntryAttribute

static sai_status_t xpSaiSetHostInterfaceTableEntryAttribute(
    sai_object_id_t hif_table_entry, const sai_attribute_t *attr)
{
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;

    saiRetVal = xpSaiAttrCheck(1, attr,
                               HOSTIF_TABLE_ENTRY_VALIDATION_ARRAY_SIZE, hostif_table_entry_attribs,
                               SAI_COMMON_API_SET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    /* All mandatory attributes have CREATE_ONLY flag */
    XP_SAI_LOG_ERR("Set Host Table Entry attribute operation is not supported\n");
    return SAI_STATUS_NOT_SUPPORTED;
}

//Func: xpSaiGetHostInterfaceTableEntryAttribute

static sai_status_t xpSaiGetHostInterfaceTableEntryAttribute(
    sai_object_id_t hif_table_entry, sai_attribute_t *attr, uint32_t attr_index)
{
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;

    xpSaiHostInterfaceTableDbEntryT  *pEntry    = NULL;

    if (attr == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiHostIntfTableDbInfoGet(hif_table_entry, &pEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find a Host Interface %" PRIu64 " in DB\n",
                       xpSaiObjIdValueGet(hif_table_entry));
        return saiRetVal;
    }

    switch (attr->id)
    {
        case SAI_HOSTIF_TABLE_ENTRY_ATTR_TYPE:
            {
                attr->value.s32 = pEntry->type;
                break;
            }
        case SAI_HOSTIF_TABLE_ENTRY_ATTR_OBJ_ID:
            {
                attr->value.oid = pEntry->interfaceObjId;
                break;
            }
        case SAI_HOSTIF_TABLE_ENTRY_ATTR_TRAP_ID:
            {
                attr->value.oid = pEntry->trapId;
                break;
            }
        case SAI_HOSTIF_TABLE_ENTRY_ATTR_CHANNEL_TYPE:
            {
                attr->value.s32 = pEntry->channelType;
                break;
            }
        case SAI_HOSTIF_TABLE_ENTRY_ATTR_HOST_IF:
            {
                attr->value.oid = pEntry->hostFd;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to get %d\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(attr_index);
            }
    }
    return saiRetVal;
}

//Func: xpSaiGetHostInterfaceTableEntryAttributes

static sai_status_t xpSaiGetHostInterfaceTableEntryAttributes(
    sai_object_id_t hif_table_entry, uint32_t attr_count,
    sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               HOSTIF_TABLE_ENTRY_VALIDATION_ARRAY_SIZE, hostif_table_entry_attribs,
                               SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetHostInterfaceTableEntryAttribute(hif_table_entry,
                                                             &attr_list[count], count);
        if (SAI_STATUS_SUCCESS != saiRetVal)
        {
            XP_SAI_LOG_ERR("failed for attribute %d\n", attr_list[count].id);
            return saiRetVal;
        }
    }

    return saiRetVal;
}

//Func: xpSaiBulkGetHostInterfaceTableEntryAttributes

sai_status_t xpSaiBulkGetHostInterfaceTableEntryAttributes(sai_object_id_t id,
                                                           uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_HOSTIF_TABLE_ENTRY))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountHostInterfaceTableEntryAttribute(&maxcount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get max attribute count!\n");
        return SAI_STATUS_FAILURE;
    }

    if (*attr_count < maxcount)
    {
        *attr_count = maxcount;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    for (uint32_t count = 0; count < maxcount; count++)
    {
        attr_list[idx].id = SAI_HOSTIF_TABLE_ENTRY_ATTR_START + count;
        saiRetVal = xpSaiGetHostInterfaceTableEntryAttribute(id, &attr_list[idx],
                                                             count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
        else if (saiRetVal == SAI_STATUS_ITEM_NOT_FOUND)
        {
            /* Host interface object not found in DB */
            XP_SAI_LOG_ERR("failed for object ID %lu\n", id);
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief Disable trap for a list of SAI ports
 *
 * This API updates @validPort list for a particular trap entry. The configuration
 * is expected to be applied on HW by subsequent xpSaiHostInterfaceTrapHwApply() call.
 *
 * \param[in] xpsDevice_t devId
 * \param[out] xpSaiHostInterfaceTrapDbEntryT *pTrapEntry
 * \param[in] sai_attribute_value_t value
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSetHostInterfaceTrapAttributeExcludePortList(
    xpsDevice_t devId, xpSaiHostInterfaceTrapDbEntryT *pTrapEntry,
    sai_attribute_value_t value)
{
    sai_status_t        saiRetVal   = SAI_STATUS_SUCCESS;
    XP_STATUS           retVal      = XP_NO_ERR;
    xpsInterfaceId_t    intfId      = XPS_INTF_INVALID_ID;
    xpsDevice_t         intfDevId   = 0;
    uint32_t            portId      = 0;
    uint32_t     maxPortNum  = 0;
    uint32_t i = 0;

    retVal = xpsPortGetMaxNum(devId, &maxPortNum);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get maximum ports number, retVal %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    XPS_GLOBAL_PORT_ITER(i, maxPortNum)
    {
        pTrapEntry->validPort[i] = true;
    }

    for (uint32_t i = 0; i < value.objlist.count; i++)
    {
        saiRetVal = xpSaiConvertPortOid(value.objlist.list[i], &intfId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiConvertPortOid() failed with error code: %d\n", saiRetVal);
            return saiRetVal;
        }

        retVal = xpsPortGetDevAndPortNumFromIntf(intfId, &intfDevId, &portId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to retrieve port by interface ID, error %d\n", retVal);
            return saiRetVal;
        }

        if (devId != intfDevId)
        {
            XP_SAI_LOG_ERR("Invalid port number\n");
            return SAI_STATUS_INVALID_PORT_NUMBER;
        }
        if (xpsLinkManagerIsPortNumValid(devId, portId) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Invalid Port ID %u \n", portId);
            continue;
        }
        pTrapEntry->validPort[portId] = false;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetDefaultHostInterfaceTrapAttributeVals

void xpSaiSetDefaultHostInterfaceTrapAttributeVals(
    xpSaiHostInterfaceTrapAttributesT *attributes)
{
    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    memset(attributes, 0, sizeof(xpSaiHostInterfaceTrapAttributesT));

    xpSaiSwitchDefaultTrapGroupGet(&attributes->trapGroup.oid);
}

//Func: xpSaiUpdateHostInterfaceAttributeVals

static sai_status_t xpSaiUpdateHostInterfaceTrapAttributeVals(
    const uint32_t attr_count,
    const sai_attribute_t *attr_list,
    xpSaiHostInterfaceTrapAttributesT *attributes)
{
    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_HOSTIF_TRAP_ATTR_TRAP_TYPE:
                {
                    attributes->trapType = attr_list[count].value;
                    break;
                }
            case SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION:
                {
                    attributes->packetAction = attr_list[count].value;
                    break;
                }
            case SAI_HOSTIF_TRAP_ATTR_TRAP_PRIORITY:
                {
                    attributes->trapPriority = attr_list[count].value;
                    break;
                }
            case SAI_HOSTIF_TRAP_ATTR_EXCLUDE_PORT_LIST:
                {
                    attributes->excludePortList = attr_list[count].value;
                    break;
                }
            case SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP:
                {
                    attributes->trapGroup = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                    return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(count);
                }
        }

    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCreateHostInterfaceTrap

static sai_status_t xpSaiCreateHostInterfaceTrap(sai_object_id_t
                                                 *hostif_trap_id, sai_object_id_t switch_id,
                                                 uint32_t attr_count, const sai_attribute_t *attr_list)
{
    XP_STATUS               retVal          = XP_NO_ERR;
    sai_status_t            saiRetVal       = SAI_STATUS_SUCCESS;
    sai_object_id_t         trapOid         = SAI_NULL_OBJECT_ID;
    const sai_attribute_t   *attr           = NULL;
    xpsDevice_t             devId           = (xpsDevice_t) xpSaiObjIdValueGet(
                                                  switch_id);
    uint32_t                attrIdx         = 0;

    xpSaiHostInterfaceTrapDbEntryT      *pTrapEntry     = NULL;
    xpSaiHostInterfaceTrapDefaultT      *pDefTrapEntry  = NULL;
    xpSaiHostInterfaceTrapAttributesT   attributes;
    sai_hostif_trap_type_t              trapType        = SAI_HOSTIF_TRAP_TYPE_END;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               HOSTIF_TRAP_VALIDATION_ARRAY_SIZE, hostif_trap_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    xpSaiSetDefaultHostInterfaceTrapAttributeVals(&attributes);

    saiRetVal = xpSaiUpdateHostInterfaceTrapAttributeVals(attr_count, attr_list,
                                                          &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to process attribute list, retVal %d\n", saiRetVal);
        return saiRetVal;
    }

    /* Get Trap type */
    attr = xpSaiFindAttrById(SAI_HOSTIF_TRAP_ATTR_TRAP_TYPE, attr_count, attr_list,
                             &attrIdx);
    if (attr == NULL)
    {
        XP_SAI_LOG_ERR("SAI_HOSTIF_TRAP_ATTR_TRAP_TYPE Not Found");
        return SAI_STATUS_FAILURE;
    }

    pDefTrapEntry = xpSaiHostIntfDefaultTrapGet((sai_hostif_trap_type_t)
                                                attr->value.s32);
    if (pDefTrapEntry == NULL)
    {
        XP_SAI_LOG_ERR("Host Interface Trap %d is not supported\n", attr->value.s32);
        return SAI_STATUS_SUCCESS;
    }

    /* Create Trap object ID */
    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_HOSTIF_TRAP, devId,
                                 (sai_uint64_t)attr->value.s32, &trapOid);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, retVal %d\n", saiRetVal);
        return saiRetVal;
    }

    retVal = xpsStateHeapMalloc(sizeof(xpSaiHostInterfaceTrapDbEntryT),
                                (void**)&pTrapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate trap structure, trap %d\n", attr->value.s32);
        return xpsStatus2SaiStatus(retVal);
    }

    memset(pTrapEntry, 0, sizeof(xpSaiHostInterfaceTrapDbEntryT));

    saiRetVal = xpSaiHostInterfaceTrapValidPortsInit(devId, pTrapEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not initialize validPort list, saiRetVal %d\n",
                       saiRetVal);
        xpsStateHeapFree(pTrapEntry);
        return saiRetVal;
    }

    pTrapEntry->trapOid = trapOid;
    pTrapEntry->action = (sai_packet_action_t)attributes.packetAction.s32;
    pTrapEntry->priority = attributes.trapPriority.u32;
    pTrapEntry->groupId = attributes.trapGroup.oid;
    pTrapEntry->reasonCodeNum = pDefTrapEntry->reasonCodeNum;

    trapType = (sai_hostif_trap_type_t)xpSaiObjIdValueGet(pTrapEntry->trapOid);
    if (trapType == SAI_HOSTIF_TRAP_TYPE_STATIC_FDB_MOVE)
    {
        /* Other packet action not supported */
        if ((pTrapEntry->action != SAI_PACKET_ACTION_DROP) &&
            (pTrapEntry->action != SAI_PACKET_ACTION_FORWARD) &&
            (pTrapEntry->action != SAI_PACKET_ACTION_TRAP))
        {
            XP_SAI_LOG_ERR("Invalid Host Interface Trap action %d\n",
                           pDefTrapEntry->action);
            xpsStateHeapFree(pTrapEntry);
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    for (uint32_t i = 0; i < pDefTrapEntry->reasonCodeNum; i++)
    {
        pTrapEntry->reasonCode[i] = pDefTrapEntry->reasonCode[i];
    }

    saiRetVal = xpSaiSetHostInterfaceTrapAttributeExcludePortList(devId, pTrapEntry,
                                                                  attributes.excludePortList);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiSetHostInterfaceTrapAttributeExcludePortList() failed with error code: %d\n",
                       saiRetVal);
        xpsStateHeapFree(pTrapEntry);
        return saiRetVal;
    }

    saiRetVal = xpSaiHostInterfaceTrapHwApply(devId, pTrapEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHostInterfaceTrapHwApply() failed with error code: %d\n",
                       saiRetVal);
        xpsStateHeapFree(pTrapEntry);
        return saiRetVal;
    }

    retVal = xpsStateInsertData(XP_SCOPE_DEFAULT, xpSaiHostIntfTrapDbHandle,
                                (void*)pTrapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not insert trap structure into DB, trap_id %d, error %d\n",
                       attributes.trapType.s32, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    *hostif_trap_id = trapOid;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveHostInterfaceTrap

static sai_status_t xpSaiRemoveHostInterfaceTrap(sai_object_id_t hostif_trap_id)
{
    sai_status_t            saiRetVal   = SAI_STATUS_SUCCESS;
    XP_STATUS               retVal      = XP_NO_ERR;
    sai_hostif_trap_type_t  trapType    = (sai_hostif_trap_type_t)
                                          xpSaiObjIdValueGet(hostif_trap_id);
    xpsDevice_t             devId       = xpSaiObjIdSwitchGet(hostif_trap_id);

    xpSaiHostInterfaceTrapDbEntryT  *pTrapEntry     = NULL;
    xpSaiHostInterfaceTrapDefaultT  *pDefTrapEntry  = NULL;
    xpsPktCmd_e                     defaultPktCmd   = XP_PKTCMD_DROP;
    xpSaiHostInterfaceTrapDbEntryT  trapKey;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    memset(&trapKey, 0, sizeof(trapKey));

    saiRetVal = xpSaiHostIntfTrapDbInfoGet(hostif_trap_id, &pTrapEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find the trap 0x%08X in DB\n", (uint32_t)trapType);
        return saiRetVal;
    }
    /* H/W to be programmed in case of update and removal not for trap creation
       During trap creation, h/w is configured while enabling SAI_FDB_ENTRY_ATTR_ALLOW_MAC_MOVE */
    if (trapType == SAI_HOSTIF_TRAP_TYPE_STATIC_FDB_MOVE)
    {
        pDefTrapEntry = xpSaiHostIntfDefaultTrapGet(trapType);
        if (pDefTrapEntry == NULL)
        {
            XP_SAI_LOG_ERR("Invalid Host Interface Trap %d\n", trapType);
            return SAI_STATUS_INVALID_PARAMETER;
        }

        /* Other packet action not supported */
        if ((pDefTrapEntry->action != SAI_PACKET_ACTION_DROP) &&
            (pDefTrapEntry->action != SAI_PACKET_ACTION_FORWARD) &&
            (pDefTrapEntry->action != SAI_PACKET_ACTION_TRAP))
        {
            XP_SAI_LOG_ERR("Invalid Host Interface Trap action %d\n",
                           pDefTrapEntry->action);
            return SAI_STATUS_INVALID_PARAMETER;
        }

        saiRetVal = xpSaiConvertSaiPacketAction2xps(pDefTrapEntry->action,
                                                    &defaultPktCmd);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to convert trap default action, error %d\n", saiRetVal);
            return saiRetVal;
        }

        retVal = xpsFdbFdbMgrBrgSecurBreachCommandSet(devId, defaultPktCmd);

        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsFdbSetAttribute failed retVal : %d \n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    trapKey.trapOid = hostif_trap_id;

    retVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, xpSaiHostIntfTrapDbHandle,
                                &trapKey, (void**)&pTrapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not delete trap structure from DB, id %d\n", trapType);
        return xpsStatus2SaiStatus(retVal);
    }

    saiRetVal = xpSaiHostInterfaceTrapValidPortsDeinit(pTrapEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not proceed deinit ValidPorts list, saiRetVal %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    retVal = xpsStateHeapFree(pTrapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not deallocate trap structure, id %d\n", trapType);
        return xpsStatus2SaiStatus(retVal);
    }

    /* Set Trap Table to the default state for this Trap */
    saiRetVal = xpSaiHostInterfaceTrapDefaultApply(devId, trapType);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to set Trap 0x%08X to the default value, error %d\n",
                       (uint32_t)trapType, saiRetVal);
        return saiRetVal;
    }

    return saiRetVal;
}

//Func: xpSaiSetHostInterfaceTrapAttribute

sai_status_t xpSaiSetHostInterfaceTrapAttribute(sai_object_id_t hostif_trapid,
                                                const sai_attribute_t *attr)
{
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    xpsDevice_t     devId       = xpSaiObjIdSwitchGet(hostif_trapid);

    xpSaiHostInterfaceTrapDbEntryT  *pTrapEntry     = NULL;
    sai_hostif_trap_type_t          trapType        = SAI_HOSTIF_TRAP_TYPE_END;
    xpsPktCmd_e                     pktCmd          = XP_PKTCMD_DROP;
    XP_STATUS                       retVal          = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiAttrCheck(1, attr,
                               HOSTIF_TRAP_VALIDATION_ARRAY_SIZE, hostif_trap_attribs,
                               SAI_COMMON_API_SET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiHostIntfTrapDbInfoGet(hostif_trapid, &pTrapEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find the trap 0x%08X in DB\n",
                       (uint32_t)xpSaiObjIdValueGet(hostif_trapid));
        return saiRetVal;
    }

    switch (attr->id)
    {
        case SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION:
            {
                pTrapEntry->action = (sai_packet_action_t)attr->value.s32;
                break;
            }
        case SAI_HOSTIF_TRAP_ATTR_TRAP_PRIORITY:
            {
                pTrapEntry->priority = attr->value.u32;
                break;
            }
        case SAI_HOSTIF_TRAP_ATTR_EXCLUDE_PORT_LIST:
            {
                saiRetVal = xpSaiSetHostInterfaceTrapAttributeExcludePortList(devId, pTrapEntry,
                                                                              attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("failed to set (SAI_HOSTIF_TRAP_ATTR_PORT_LIST)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP:
            {
                pTrapEntry->groupId = attr->value.oid;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Attribute %d is unknown\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            }
    }

    trapType = (sai_hostif_trap_type_t)xpSaiObjIdValueGet(pTrapEntry->trapOid);
    /* H/W to be programmed in case of update and removal not for trap creation
       During trap creation, h/w is configured while enabling SAI_FDB_ENTRY_ATTR_ALLOW_MAC_MOVE */
    if (trapType == SAI_HOSTIF_TRAP_TYPE_STATIC_FDB_MOVE &&
        attr->id == SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION)
    {
        /* Other packet action not supported */
        if ((pTrapEntry->action != SAI_PACKET_ACTION_DROP) &&
            (pTrapEntry->action != SAI_PACKET_ACTION_FORWARD) &&
            (pTrapEntry->action != SAI_PACKET_ACTION_TRAP))
        {
            XP_SAI_LOG_ERR("Invalid Host Interface Trap action %d\n", pTrapEntry->action);
            return SAI_STATUS_INVALID_PARAMETER;
        }

        saiRetVal = xpSaiConvertSaiPacketAction2xps(pTrapEntry->action,
                                                    &pktCmd);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to convert trap action, error %d\n", saiRetVal);
            return saiRetVal;
        }

        retVal = xpsFdbFdbMgrBrgSecurBreachCommandSet(devId, pktCmd);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xps update interface trap attribute failed retVal : %d \n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    saiRetVal = xpSaiHostInterfaceTrapHwApply(devId, pTrapEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHostInterfaceTrapHwApply() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetHostInterfaceTrapAttributePortList(xpsDevice_t devId,
                                                        const xpSaiHostInterfaceTrapDbEntryT *pTrapEntry, sai_attribute_value_t *value)
{
    sai_status_t        saiRetVal   = SAI_STATUS_SUCCESS;
    XP_STATUS           retVal      = XP_NO_ERR;
    xpsInterfaceId_t    intfId      = XPS_INTF_INVALID_ID;
    uint32_t            portId      = 0;
    uint32_t            portNum     = 0;
    uint32_t             maxPortNum = 0;

    retVal = xpsPortGetMaxNum(devId, &maxPortNum);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get maximum ports number, retVal %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    XPS_GLOBAL_PORT_ITER(portId, maxPortNum)
    {
        if (!pTrapEntry->validPort[portId])
        {
            ++portNum;
        }
    }

    if (portNum > value->objlist.count)
    {
        XP_SAI_LOG_ERR("Enable to fit %u OIDs into the buffer with size %u\n", portNum,
                       value->objlist.count);
        value->objlist.count = portNum;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    portNum = 0;

    XPS_GLOBAL_PORT_ITER(portId, maxPortNum)
    {
        if (pTrapEntry->validPort[portId])
        {
            continue;
        }

        retVal = xpsPortGetPortIntfId(devId, portId, &intfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_DBG("Failed to get interface ID by port number. Error #%d\n",
                           retVal);
            continue;
        }

        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId, (sai_uint64_t)intfId,
                                     &value->objlist.list[portNum]);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error : SAI port object could not be created.\n");
            return saiRetVal;
        }
        ++portNum;
    }

    value->objlist.count = portNum;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetHostInterfaceTrapAttribute

static sai_status_t xpSaiGetHostInterfaceTrapAttribute(sai_object_id_t
                                                       hostif_trapid, sai_attribute_t *attr, uint32_t attr_index)
{
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    xpsDevice_t     devId       = xpSaiObjIdSwitchGet(hostif_trapid);

    xpSaiHostInterfaceTrapDbEntryT  *pTrapEntry     = NULL;

    saiRetVal = xpSaiHostIntfTrapDbInfoGet(hostif_trapid, &pTrapEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find the trap 0x%08X in DB\n",
                       (uint32_t)xpSaiObjIdValueGet(hostif_trapid));
        return saiRetVal;
    }

    switch (attr->id)
    {
        case SAI_HOSTIF_TRAP_ATTR_TRAP_TYPE:
            {
                attr->value.s32 = (sai_int32_t)xpSaiObjIdValueGet(hostif_trapid);
                break;
            }
        case SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION:
            {
                attr->value.s32 = pTrapEntry->action;
                break;
            }
        case SAI_HOSTIF_TRAP_ATTR_TRAP_PRIORITY:
            {
                attr->value.u32 = pTrapEntry->priority;
                break;
            }
        case SAI_HOSTIF_TRAP_ATTR_EXCLUDE_PORT_LIST:
            {
                saiRetVal = xpSaiGetHostInterfaceTrapAttributePortList(devId, pTrapEntry,
                                                                       &attr->value);
                {
                    if (SAI_STATUS_SUCCESS != saiRetVal)
                    {
                        XP_SAI_LOG_ERR("failed to get (SAI_HOSTIF_TRAP_ATTR_EXCLUDE_PORT_LIST)\n");
                        return saiRetVal;
                    }
                }
                break;
            }
        case SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP:
            {
                attr->value.oid = pTrapEntry->groupId;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Attribute %d is unknown\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(attr_index);
            }
    }

    return SAI_STATUS_SUCCESS;
}



//Func: xpSaiGetHostInterfaceTrapAttribute

static sai_status_t xpSaiGetHostInterfaceTrapAttributes(
    sai_object_id_t hostif_trapid, uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               HOSTIF_TRAP_VALIDATION_ARRAY_SIZE, hostif_trap_attribs,
                               SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetHostInterfaceTrapAttribute(hostif_trapid, &attr_list[count],
                                                       count);
        if (SAI_STATUS_SUCCESS != saiRetVal)
        {
            XP_SAI_LOG_ERR("failed for trapId %d\n", (sai_hostif_trap_type_t)hostif_trapid);
            return saiRetVal;
        }
    }

    return saiRetVal;
}

//Func: xpSaiBulkGetHostInterfaceTrapAttributes

sai_status_t xpSaiBulkGetHostInterfaceTrapAttributes(sai_object_id_t id,
                                                     uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_HOSTIF_TRAP))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountHostInterfaceTrapAttribute(&maxcount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get max attribute count!\n");
        return SAI_STATUS_FAILURE;
    }

    if (*attr_count < maxcount)
    {
        *attr_count = maxcount;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    for (uint32_t count = 0; count < maxcount; count++)
    {
        attr_list[idx].id = SAI_HOSTIF_TRAP_ATTR_START + count;
        saiRetVal = xpSaiGetHostInterfaceTrapAttribute(id, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
        else if (saiRetVal == SAI_STATUS_ITEM_NOT_FOUND)
        {
            /* Host interface object not found in DB */
            XP_SAI_LOG_ERR("failed for object ID %lu\n", id);
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetDefaultHostInterfaceUseDefinedTrapAttributeVals

static void xpSaiSetDefaultHostInterfaceUseDefinedTrapAttributeVals(
    xpSaiHostInterfaceTrapAttributesT *attributes)
{
    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    memset(attributes, 0, sizeof(xpSaiHostInterfaceTrapAttributesT));

    xpSaiSwitchDefaultTrapGroupGet(&attributes->trapGroup.oid);
}

//Func: xpSaiUpdateHostInterfaceAttributeVals

static sai_status_t xpSaiUpdateHostInterfaceUseDefinedTrapAttributeVals(
    const uint32_t attr_count,
    const sai_attribute_t *attr_list,
    xpSaiHostInterfaceTrapAttributesT *attributes)
{
    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_TYPE:
                {
                    attributes->trapType = attr_list[count].value;
                    break;
                }
            case SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_TRAP_PRIORITY:
                {
                    attributes->trapPriority = attr_list[count].value;
                    break;
                }
            case SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_TRAP_GROUP:
                {
                    attributes->trapGroup = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                    return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(count);
                }
        }

    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostInterfaceUserTrapObjIdCreate

static sai_status_t xpSaiHostInterfaceUserTrapObjIdCreate(xpsDevice_t devId,
                                                          sai_object_id_t *sai_object_id)
{
    sai_status_t        saiRetVal       = SAI_STATUS_SUCCESS;
    sai_object_id_t     oid             = SAI_NULL_OBJECT_ID;

    xpSaiHostInterfaceTrapDbEntryT  *pTrapEntry     = NULL;

    if (sai_object_id == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t trapId = XP_SAI_USER_DEFINED_TRAP_RC_MIN;
         trapId <= XP_SAI_USER_DEFINED_TRAP_RC_MAX; trapId++)
    {
        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_HOSTIF_USER_DEFINED_TRAP, devId,
                                     trapId, &oid);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI object could not be created, retVal %d\n", saiRetVal);
            return saiRetVal;
        }

        pTrapEntry = NULL;
        while ((saiRetVal = xpSaiHostIntfTrapDbInfoGetNext(pTrapEntry,
                                                           &pTrapEntry)) == SAI_STATUS_SUCCESS)
        {
            if (pTrapEntry->trapOid == oid)
            {
                /* User-defined trap ID is already in use */
                break;
            }
        }

        if (saiRetVal == SAI_STATUS_ITEM_NOT_FOUND)
        {
            *sai_object_id = oid;
            return SAI_STATUS_SUCCESS;
        }
    }

    XP_SAI_LOG_ERR("Failed to allocate user-defined trap OID. Insufficient system resources exist.\n");
    return SAI_STATUS_INSUFFICIENT_RESOURCES;
}

//Func: xpSaiCreateHostInterfaceUserDefinedTrap

static sai_status_t xpSaiCreateHostInterfaceUserDefinedTrap(
    sai_object_id_t *hostif_user_defined_trap_id, sai_object_id_t switch_id,
    uint32_t attr_count, const sai_attribute_t *attr_list)
{
    XP_STATUS               retVal          = XP_NO_ERR;
    sai_status_t            saiRetVal       = SAI_STATUS_SUCCESS;
    const sai_attribute_t   *attr           = NULL;
    xpsDevice_t             devId           = (xpsDevice_t) xpSaiObjIdValueGet(
                                                  switch_id);
    xpsScope_t              scope           = xpSaiScopeFromDevGet(devId);
    uint32_t                attrIdx         = 0;

    xpSaiHostInterfaceTrapDbEntryT      *pTrapEntry     = NULL;
    xpSaiHostInterfaceTrapAttributesT   attributes;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               HOSTIF_USER_DEFINED_TRAP_VALIDATION_ARRAY_SIZE,
                               hostif_user_defined_trap_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    xpSaiSetDefaultHostInterfaceUseDefinedTrapAttributeVals(&attributes);

    saiRetVal = xpSaiUpdateHostInterfaceUseDefinedTrapAttributeVals(attr_count,
                                                                    attr_list, &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to process attribute list, retVal %d\n", saiRetVal);
        return saiRetVal;
    }

    /* Get trap type */
    attr = xpSaiFindAttrById(SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_TYPE, attr_count,
                             attr_list, &attrIdx);
    if (attr == NULL)
    {
        XP_SAI_LOG_ERR("SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_TYPE Not Found");
        return SAI_STATUS_FAILURE;
    }

    if ((sai_hostif_user_defined_trap_type_t)attr->value.s32 !=
        SAI_HOSTIF_USER_DEFINED_TRAP_TYPE_ACL)
    {
        XP_SAI_LOG_ERR("Unsupported user defined trap type %d\n", attr->value.s32);
        return SAI_STATUS_INVALID_ATTR_VALUE_0 + attrIdx;
    }

    retVal = xpsStateHeapMalloc(sizeof(xpSaiHostInterfaceTrapDbEntryT),
                                (void**)&pTrapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate trap structure, trap %d\n", attr->value.s32);
        return xpsStatus2SaiStatus(retVal);
    }

    memset(pTrapEntry, 0, sizeof(xpSaiHostInterfaceTrapDbEntryT));

    saiRetVal = xpSaiHostInterfaceTrapValidPortsInit(devId, pTrapEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not initialize validPort list, saiRetVal %d\n",
                       saiRetVal);
        xpsStateHeapFree(pTrapEntry);
        return saiRetVal;
    }

    saiRetVal = xpSaiHostInterfaceUserTrapObjIdCreate(devId, &pTrapEntry->trapOid);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, retVal %d\n", saiRetVal);
        xpsStateHeapFree(pTrapEntry);
        return saiRetVal;
    }

    pTrapEntry->priority = attributes.trapPriority.u32;
    pTrapEntry->groupId = attributes.trapGroup.oid;
    pTrapEntry->reasonCodeNum = 1;
    pTrapEntry->reasonCode[0] = (uint32_t)xpSaiObjIdValueGet(pTrapEntry->trapOid);

    saiRetVal = xpSaiHostInterfaceUserTrapHwApply(devId, pTrapEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHostInterfaceUserDefinedTrapHwApply() failed with error code: %d\n",
                       saiRetVal);
        xpsStateHeapFree(pTrapEntry);
        return saiRetVal;
    }

    retVal = xpsStateInsertData(scope, xpSaiHostIntfTrapDbHandle,
                                (void*)pTrapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not insert trap into DB, trap_id %" PRIu64 ", error %d\n",
                       xpSaiObjIdValueGet(pTrapEntry->trapOid), retVal);
        xpsStateHeapFree(pTrapEntry);
        return xpsStatus2SaiStatus(retVal);
    }

    *hostif_user_defined_trap_id = pTrapEntry->trapOid;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveHostInterfaceUserDefinedTrap

static sai_status_t xpSaiRemoveHostInterfaceUserDefinedTrap(
    sai_object_id_t hostif_user_defined_trap_id)
{
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    XP_STATUS       retVal      = XP_NO_ERR;
    xpsDevice_t     devId       = xpSaiObjIdSwitchGet(hostif_user_defined_trap_id);
    sai_uint64_t    trapId      = xpSaiObjIdValueGet(hostif_user_defined_trap_id);
    xpsScope_t      scope       = xpSaiObjIdScopeGet(hostif_user_defined_trap_id);
    sai_hostif_trap_type_t saiTrap;
    xpsCoppCtrlPkt_e xpsTrap = XPS_COPP_CTRL_PKT_MAX;

    xpSaiHostInterfaceTrapDbEntryT  *pTrapEntry  = NULL;
    xpSaiHostInterfaceTrapDbEntryT  trapKey;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    memset(&trapKey, 0, sizeof(trapKey));

    saiRetVal = xpSaiHostIntfTrapDbInfoGet(hostif_user_defined_trap_id,
                                           &pTrapEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find user defined trap %" PRIu64 " in DB\n", trapId);
        return saiRetVal;
    }

    trapKey.trapOid = hostif_user_defined_trap_id;

    saiTrap = (sai_hostif_trap_type_t)xpSaiObjIdValueGet(pTrapEntry->trapOid);
    saiRetVal = xpSaiHostInterfaceGetXpsCoppCtrlPktType(saiTrap, &xpsTrap);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get xpsTrap for saiTrap %d error %d\n", saiTrap,
                       saiRetVal);
        return saiRetVal;
    }
    saiRetVal = xpSaiCoppEntryRemove(devId, (uint32_t)xpsTrap);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not delete user defined trap %" PRIu64 " from HW\n",
                       trapId);
        return saiRetVal;
    }

    retVal = xpsStateDeleteData(scope, xpSaiHostIntfTrapDbHandle, &trapKey,
                                (void**)&pTrapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not delete user defined trap %" PRIu64 " from DB\n",
                       trapId);
        return xpsStatus2SaiStatus(retVal);
    }

    saiRetVal = xpSaiHostInterfaceTrapValidPortsDeinit(pTrapEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not proceed deinit ValidPorts list, saiRetVal %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    retVal = xpsStateHeapFree(pTrapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not deallocate user defined trap structure, id %" PRIu64
                       "\n", trapId);
        return xpsStatus2SaiStatus(retVal);
    }

    return saiRetVal;
}

//Func: xpSaiSetHostInterfaceUserDefinedTrapAttribute

static sai_status_t xpSaiSetHostInterfaceUserDefinedTrapAttribute(
    sai_object_id_t hostif_user_defined_trap_id, const sai_attribute_t *attr)
{
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    xpsDevice_t     devId       = xpSaiObjIdSwitchGet(hostif_user_defined_trap_id);

    xpSaiHostInterfaceTrapDbEntryT  *pTrapEntry     = NULL;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiAttrCheck(1, attr,
                               HOSTIF_USER_DEFINED_TRAP_VALIDATION_ARRAY_SIZE,
                               hostif_user_defined_trap_attribs,
                               SAI_COMMON_API_SET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiHostIntfTrapDbInfoGet(hostif_user_defined_trap_id,
                                           &pTrapEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find user defined trap #%" PRIu64 " in DB\n",
                       xpSaiObjIdValueGet(hostif_user_defined_trap_id));
        return saiRetVal;
    }

    switch (attr->id)
    {
        case SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_TRAP_PRIORITY:
            {
                pTrapEntry->priority = attr->value.u32;
                break;
            }
        case SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_TRAP_GROUP:
            {
                pTrapEntry->groupId = attr->value.oid;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Attribute %d is unknown\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            }
    }

    saiRetVal = xpSaiHostInterfaceUserTrapHwApply(devId, pTrapEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHostInterfaceUserTrapHwApply() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetHostInterfaceUserDefinedTrapAttribute

static sai_status_t xpSaiGetHostInterfaceUserDefinedTrapAttribute(
    sai_object_id_t hostif_user_defined_trap_id, sai_attribute_t *attr,
    uint32_t attr_index)
{
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;

    xpSaiHostInterfaceTrapDbEntryT  *pTrapEntry     = NULL;

    saiRetVal = xpSaiHostIntfTrapDbInfoGet(hostif_user_defined_trap_id,
                                           &pTrapEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find the trap 0x%08X in DB\n",
                       (uint32_t)xpSaiObjIdValueGet(hostif_user_defined_trap_id));
        return saiRetVal;
    }

    switch (attr->id)
    {
        case SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_TYPE:
            {
                attr->value.s32 = (sai_int32_t)xpSaiObjIdValueGet(hostif_user_defined_trap_id);
                break;
            }
        case SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_TRAP_PRIORITY:
            {
                attr->value.u32 = pTrapEntry->priority;
                break;
            }
        case SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_TRAP_GROUP:
            {
                attr->value.oid = pTrapEntry->groupId;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Attribute %d is unknown\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(attr_index);
            }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiGetHostInterfaceUserDefinedTrapAttributes(
    sai_object_id_t hostif_user_defined_trap_id, uint32_t attr_count,
    sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               HOSTIF_USER_DEFINED_TRAP_VALIDATION_ARRAY_SIZE,
                               hostif_user_defined_trap_attribs,
                               SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetHostInterfaceUserDefinedTrapAttribute(
                        hostif_user_defined_trap_id, &attr_list[count], count);
        if (SAI_STATUS_SUCCESS != saiRetVal)
        {
            XP_SAI_LOG_ERR("failed for user trap Id %lu\n",
                           (long unsigned int) hostif_user_defined_trap_id);
            return saiRetVal;
        }
    }
    return saiRetVal;
}

//Func: xpSaiBulkGetHostInterfaceUserTrapAttributes

sai_status_t xpSaiBulkGetHostInterfaceUserTrapAttributes(sai_object_id_t id,
                                                         uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_HOSTIF_USER_DEFINED_TRAP))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountHostInterfaceUserTrapAttribute(&maxcount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get max attribute count!\n");
        return SAI_STATUS_FAILURE;
    }

    if (*attr_count < maxcount)
    {
        *attr_count = maxcount;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    for (uint32_t count = 0; count < maxcount; count++)
    {
        attr_list[idx].id = SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_START + count;
        saiRetVal = xpSaiGetHostInterfaceUserDefinedTrapAttribute(id, &attr_list[idx],
                                                                  count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
        else if (saiRetVal == SAI_STATUS_ITEM_NOT_FOUND)
        {
            /* Host interface object not found in DB */
            XP_SAI_LOG_ERR("failed for object ID %lu\n", id);
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetDefaultHostInterfaceTrapGroupAttributeVals

static void xpSaiSetDefaultHostInterfaceTrapGroupAttributeVals(
    xpSaiHostInterfaceTrapGroupAttributesT *attributes)
{
    XP_SAI_LOG_DBG("Calling xpSaiSetDefaultHostInterfaceTrapGroupAttributeVals\n");

    attributes->adminState.booldata = true;
    attributes->queue.u32 = 0;
    attributes->policer.oid = SAI_NULL_OBJECT_ID;
}

//Func: xpSaiUpdateHostInterfaceTrapGroupAttributeVals

static void xpSaiUpdateHostInterfaceTrapGroupAttributeVals(
    const uint32_t attr_count, const sai_attribute_t *attr_list,
    xpSaiHostInterfaceTrapGroupAttributesT *attributes)
{
    XP_SAI_LOG_DBG("Calling xpSaiUpdateHostInterfaceTrapGroupAttributeVals\n");

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE:
                {
                    attributes->adminState = attr_list[count].value;
                    break;
                }
            case SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE:
                {
                    attributes->queue = attr_list[count].value;
                    break;
                }
            case SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER:
                {
                    attributes->policer = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }

    }

}

//Func: xpSaiCreateHostInterfaceTrapGroup

static sai_status_t xpSaiCreateHostInterfaceTrapGroup(sai_object_id_t
                                                      *hostif_trap_group_id,
                                                      sai_object_id_t switch_id,
                                                      uint32_t attr_count,
                                                      const sai_attribute_t *attr_list)
{
    XP_STATUS       retVal     = XP_NO_ERR;
    XP_STATUS       retValFinal= XP_NO_ERR;
    sai_status_t    saiRetVal  = SAI_STATUS_FAILURE;
    uint32_t        grpId      = 0;
    xpsDevice_t     devId      = xpSaiGetDevId();

    xpSaiHostInterfaceGroupDbEntryT         *pGrpEntry  = NULL;
    xpSaiHostInterfaceTrapGroupAttributesT  attributes;

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               HOSTIF_TRAP_GROUP_VALIDATION_ARRAY_SIZE, hostif_trap_group_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    xpSaiSetDefaultHostInterfaceTrapGroupAttributeVals(&attributes);

    xpSaiUpdateHostInterfaceTrapGroupAttributeVals(attr_count, attr_list,
                                                   &attributes);

    retVal = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_HOST_INTF_GRP,
                                    &grpId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate Host Interface Group id, retVal %d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    XP_SAI_LOG_DBG("Group %d is being created\n", grpId);

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP, devId,
                                 (sai_uint64_t)grpId, hostif_trap_group_id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created, saiRetVal %d", saiRetVal);
        return saiRetVal;
    }

    retVal = xpsStateHeapMalloc(sizeof(xpSaiHostInterfaceGroupDbEntryT),
                                (void**)&pGrpEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate group structure, grpId %d\n", grpId);
        return xpsStatus2SaiStatus(retVal);
    }

    pGrpEntry->keyId = grpId;
    pGrpEntry->adminState = attributes.adminState.booldata;
    pGrpEntry->policerOid = attributes.policer.oid;
    pGrpEntry->queue = attributes.queue.u32;

    if (pGrpEntry->policerOid != SAI_NULL_OBJECT_ID)
    {
        saiRetVal = xpSaiHostInterfacePolicerEntryAdd(pGrpEntry->policerOid);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiHostInterfacePolicerEntryAdd failed: policerOid=%lu, status=%i.\n",
                           pGrpEntry->policerOid, saiRetVal);
            retVal = xpsStateHeapFree((void *)pGrpEntry);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not deallocate group structure, grpId %d\n", grpId);
                return xpsStatus2SaiStatus(retVal);
            }
            return saiRetVal;
        }
    }

    retVal = xpsStateInsertData(XP_SCOPE_DEFAULT, xpSaiHostIntfGroupDbHandle,
                                (void*)pGrpEntry);
    if (retVal != XP_NO_ERR)
    {
        retValFinal = retVal;

        XP_SAI_LOG_ERR("Could not insert group structure into DB, grpId %d\n", grpId);
        retVal = xpsStateHeapFree((void *)pGrpEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not deallocate group structure, grpId %d\n", grpId);
            return xpsStatus2SaiStatus(retVal);
        }
        return xpsStatus2SaiStatus(retValFinal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveHostInterfaceTrapGroup

sai_status_t xpSaiRemoveHostInterfaceTrapGroup(sai_object_id_t
                                               hostif_trap_group_id)
{
    XP_STATUS       retVal     = XP_NO_ERR;
    sai_status_t    saiRetVal  = SAI_STATUS_FAILURE;
    uint32_t        grpId      = 0;

    xpSaiHostInterfaceGroupDbEntryT     *pGrpEntry  = NULL;
    xpSaiHostInterfaceTrapDbEntryT      *pTrapEntry = NULL;
    xpSaiHostInterfaceGroupDbEntryT     grpKey;

    while (xpSaiHostIntfTrapDbInfoGetNext(pTrapEntry,
                                          &pTrapEntry) == SAI_STATUS_SUCCESS)
    {
        if (pTrapEntry->groupId == hostif_trap_group_id)
        {
            XP_SAI_LOG_ERR("group 0x%016" PRIX64 " is in use\n", hostif_trap_group_id);
            return SAI_STATUS_OBJECT_IN_USE;
        }
    }

    memset(&grpKey, 0, sizeof(xpSaiHostInterfaceGroupDbEntryT));

    saiRetVal = xpSaiConvertTrapGroupOid(hostif_trap_group_id, &grpId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertTrapGroupOid() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    XP_SAI_LOG_DBG("Group %d is being removed\n", grpId);

    retVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_HOST_INTF_GRP,
                                   grpId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not release Host Interface Group id, retVal %d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    grpKey.keyId = grpId;

    retVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, xpSaiHostIntfGroupDbHandle,
                                &grpKey, (void**)&pGrpEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not delete group structure from DB, grpId %d\n", grpId);
        return xpsStatus2SaiStatus(retVal);
    }

    if ((pGrpEntry) && (pGrpEntry->policerOid != SAI_NULL_OBJECT_ID))
    {
        saiRetVal = xpSaiHostInterfacePolicerEntryRemove(pGrpEntry->policerOid);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiHostInterfacePolicerEntryRemove failed: policerOid=%lu, status=%i.\n",
                           pGrpEntry->policerOid, saiRetVal);
            return saiRetVal;
        }
    }

    retVal = xpsStateHeapFree(pGrpEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not deallocate group structure, grpId %d\n", grpId);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//xpSaiSetHostInterfaceTrapGroupAttrAdminState

sai_status_t xpSaiSetHostInterfaceTrapGroupAttrAdminState(
    sai_object_id_t hostif_trap_group_id, sai_attribute_value_t value)
{
    sai_status_t    saiRetVal   = SAI_STATUS_FAILURE;
    XP_STATUS       retVal      = XP_NO_ERR;
    uint32_t         cpuPortNum  = 0;
    xpsDevice_t     xpsDevId    = xpSaiObjIdSwitchGet(hostif_trap_group_id);

    xpSaiHostInterfaceGroupDbEntryT *pGrpEntry = NULL;

    saiRetVal = xpSaiHostIntfGroupDbInfoGet(hostif_trap_group_id, &pGrpEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find a trap group in DB\n");
        return saiRetVal;
    }

    retVal = xpsGlobalSwitchControlGetCpuPhysicalPortNum(xpsDevId, &cpuPortNum);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get CPU physical port number, error %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsQosAqmSetEnqueueEnable(xpsDevId, cpuPortNum, pGrpEntry->queue,
                                       value.booldata);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to set queue %u admin state, error %d\n",
                       pGrpEntry->queue, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    pGrpEntry->adminState = value.booldata;

    return SAI_STATUS_SUCCESS;
}

//xpSaiSetHostInterfaceTrapGroupAttrQueue

sai_status_t xpSaiSetHostInterfaceTrapGroupAttrQueue(sai_object_id_t
                                                     hostif_trap_group_id, sai_attribute_value_t value)
{
    sai_status_t                    saiRetVal  = SAI_STATUS_FAILURE;
    xpSaiHostInterfaceGroupDbEntryT *pGrpEntry = NULL;
    sai_uint32_t                    oldQueue   = 0;

    saiRetVal = xpSaiHostIntfGroupDbInfoGet(hostif_trap_group_id, &pGrpEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find a trap group in DB\n");
        return saiRetVal;
    }

    oldQueue = pGrpEntry->queue;
    pGrpEntry->queue = value.u32;

    saiRetVal = xpSaiHostInterfaceQueueApply(hostif_trap_group_id, oldQueue);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not apply queue\n");
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

//xpSaiHostInterfaceQueueApply

static sai_status_t xpSaiHostInterfaceQueueApply(sai_object_id_t
                                                 hostif_trap_group_id, sai_uint32_t oldQueue)
{
    sai_status_t    saiRetVal   = SAI_STATUS_FAILURE;
    xpsDevice_t     xpsDevId    = xpSaiGetDevId();

    xpSaiHostInterfaceTrapDbEntryT  *pTrapEntry     = NULL;
    xpSaiHostInterfaceGroupDbEntryT *pGrpEntry      = NULL;
    xpSaiHostInterfaceTrapDefaultT  *pDefTrapEntry  = NULL;
    sai_hostif_trap_type_t          trapType        = SAI_HOSTIF_TRAP_TYPE_END;
    xpsCoppCtrlPkt_e xpsTrap = XPS_COPP_CTRL_PKT_MAX;

    saiRetVal = xpSaiHostIntfGroupDbInfoGet(hostif_trap_group_id, &pGrpEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find a trap group in DB, error %d\n", saiRetVal);
        return saiRetVal;
    }

    if (pGrpEntry->queue == oldQueue)
    {
        return SAI_STATUS_SUCCESS;
    }

    while (xpSaiHostIntfTrapDbInfoGetNext(pTrapEntry,
                                          &pTrapEntry) == SAI_STATUS_SUCCESS)
    {
        if (pTrapEntry->groupId != hostif_trap_group_id)
        {
            continue;
        }

        trapType = (sai_hostif_trap_type_t)xpSaiObjIdValueGet(pTrapEntry->trapOid);
        saiRetVal = xpSaiHostInterfaceGetXpsCoppCtrlPktType(trapType, &xpsTrap);
        pDefTrapEntry = xpSaiHostIntfDefaultTrapGet(trapType);
        if (pDefTrapEntry == NULL)
        {
            XP_SAI_LOG_ERR("Invalid Host Interface Trap %d\n", trapType);
            return SAI_STATUS_INVALID_PARAMETER;
        }

        for (uint32_t i = 0; i < pDefTrapEntry->reasonCodeNum; i++)
        {
            saiRetVal = xpSaiCoppQueueMappingSet(xpsDevId, pGrpEntry->queue,
                                                 (uint32_t)xpsTrap);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not add copp entry, reason code %d, error %d\n",
                               pDefTrapEntry->reasonCode[i], saiRetVal);
                return saiRetVal;
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostInterfacePolicerEntryHwAdd

static sai_status_t xpSaiHostInterfacePolicerEntryHwAdd(
    sai_object_id_t policerOid,
    sai_uint32_t index)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS xpStatus = XP_NO_ERR;
    xpsDevice_t devId = xpSaiObjIdSwitchGet(policerOid);
    xpsPolicerEntry_t policerEntry;
    memset(&policerEntry, 0, sizeof(policerEntry));

    saiStatus = xpSaiPolicerGetXpsEntry(policerOid, &policerEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiPolicerGetXpsEntry failed: status=%i.\n", saiStatus);
        return saiStatus;
    }

    xpStatus = xpsPolicerAddEntry(devId, XP_EGRESS_FLOW_POLICER, index, index,
                                  &policerEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsPolicerAddEntry failed: status=%i.\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpSaiPolicerAddIdxToIdxList(policerOid, XP_EGRESS_FLOW_POLICER,
                                           index);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiPolicerAddIdxToIdxList failed: status=%i.\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Clear policer counter entry */
    if ((xpStatus = xpsPolicerClearPolicerCounterEntry(devId,
                                                       XP_EGRESS_FLOW_POLICER, index)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to clear policer counter |Error: %d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostInterfacePolicerEntryAdd

static sai_status_t xpSaiHostInterfacePolicerEntryAdd(sai_object_id_t
                                                      policerOid)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    sai_uint32_t acmIndex = 0;
    XP_STATUS    status = XP_NO_ERR;
    xpSaiHostInterfacePolicerDbEntry_t *dataEntry = NULL, keyEntry;

    saiStatus = xpSaiHostInterfacePolicerDbEntryGet(policerOid, &dataEntry);
    if (saiStatus == SAI_STATUS_SUCCESS)
    {
        dataEntry->data.refCount++;
        return SAI_STATUS_SUCCESS;
    }

    status = xpsPolicerIndexAllocate(XP_EGRESS_FLOW_POLICER, &acmIndex);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Allocate COPP policer id failed: status=%i.\n", status);
        return xpsStatus2SaiStatus(status);
    }

    keyEntry.keyOid = policerOid;
    keyEntry.data.acmIndex = acmIndex;
    keyEntry.data.refCount = 1;

    saiStatus = xpSaiHostInterfacePolicerDbEntryInsert(&keyEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHostInterfacePolicerDbEntryInsert failed: status=%i.\n",
                       saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiHostInterfacePolicerEntryHwAdd(policerOid, acmIndex);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHostInterfacePolicerEntryHwAdd failed: status=%i.\n",
                       saiStatus);
        return saiStatus;
    }

    XP_SAI_LOG_DBG("Add policer entry: policerOid=%lu, acmIndex=%u, refCount=%u.\n",
                   policerOid, keyEntry.data.acmIndex, keyEntry.data.refCount);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostInterfacePolicerEntryHwRemove

static sai_status_t xpSaiHostInterfacePolicerEntryHwRemove(
    sai_object_id_t policerOid,
    sai_uint32_t index)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpsDevice_t devId = xpSaiGetDevId();

    xpStatus = xpsPolicerRemoveEntry(devId, XP_EGRESS_FLOW_POLICER, index);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsPolicerRemoveEntry failed: status=%i.\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpSaiPolicerRemoveIdxFromIdxList(policerOid, XP_EGRESS_FLOW_POLICER,
                                                index);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiPolicerRemoveIdxFromIdxList failed: status=%i.\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostInterfacePolicerEntryRemove

static sai_status_t xpSaiHostInterfacePolicerEntryRemove(
    sai_object_id_t policerOid)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS    status = XP_NO_ERR;
    xpSaiHostInterfacePolicerDbEntry_t *dataEntry = NULL;

    saiStatus = xpSaiHostInterfacePolicerDbEntryGet(policerOid, &dataEntry);
    if (saiStatus == SAI_STATUS_SUCCESS)
    {
        if (--dataEntry->data.refCount == 0)
        {
            saiStatus = xpSaiHostInterfacePolicerEntryHwRemove(policerOid,
                                                               dataEntry->data.acmIndex);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiHostInterfacePolicerEntryHwRemove failed: status=%i.\n",
                               saiStatus);
                return saiStatus;
            }

            status = xpsPolicerIndexRelease(XP_EGRESS_FLOW_POLICER,
                                            dataEntry->data.acmIndex);
            if (status != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Allocate COPP policer id failed: status=%i.\n", status);
                return xpsStatus2SaiStatus(status);
            }

            saiStatus = xpSaiHostInterfacePolicerDbEntryRemove(policerOid);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiHostInterfacePolicerDbEntryRemove failed: status=%i.\n",
                               saiStatus);
                return saiStatus;
            }

            XP_SAI_LOG_DBG("Remove policer entry: policerOid=%lu\n", policerOid);
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostInterfacePolicerApply

static sai_status_t xpSaiHostInterfacePolicerApply(sai_object_id_t groupId,
                                                   sai_object_id_t oldPolicerOid)
{
    sai_status_t    saiRetVal       = SAI_STATUS_FAILURE;
    xpsPktCmd_e     pktCmd          = XP_PKTCMD_DROP;
    xpsPktCmd_e     defaultPktCmd   = XP_PKTCMD_DROP;
    uint32_t        policerId       = XPSAI_INVALID_POLICER_ID;
    xpsDevice_t     xpsDevId        = xpSaiObjIdSwitchGet(groupId);
    xpsCoppCtrlPkt_e xpsTrap = XPS_COPP_CTRL_PKT_MAX;

    xpSaiHostInterfaceTrapDbEntryT     *pTrapEntry    = NULL;
    xpSaiHostInterfaceGroupDbEntryT    *pGrpEntry     = NULL;
    xpSaiHostInterfaceTrapDefaultT     *pDefTrapEntry = NULL;
    xpSaiHostInterfacePolicerDbEntry_t *policerEntry  = NULL;

    sai_hostif_trap_type_t trapType = SAI_HOSTIF_TRAP_TYPE_END;

    saiRetVal = xpSaiHostIntfGroupDbInfoGet(groupId, &pGrpEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find a trap group in DB, error %d\n", saiRetVal);
        return saiRetVal;
    }

    if (pGrpEntry->policerOid != SAI_NULL_OBJECT_ID)
    {
        saiRetVal = xpSaiHostInterfacePolicerDbEntryGet(pGrpEntry->policerOid,
                                                        &policerEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiHostInterfacePolicerDbEntryGet failed: policerOid=%lu, status=%i.\n",
                           pGrpEntry->policerOid, saiRetVal);
            return saiRetVal;
        }

        policerId = policerEntry->data.acmIndex;
    }

    while (xpSaiHostIntfTrapDbInfoGetNext(pTrapEntry,
                                          &pTrapEntry) == SAI_STATUS_SUCCESS)
    {
        if (pTrapEntry->groupId != groupId)
        {
            continue;
        }

        trapType = (sai_hostif_trap_type_t)xpSaiObjIdValueGet(pTrapEntry->trapOid);
        saiRetVal = xpSaiHostInterfaceGetXpsCoppCtrlPktType(trapType, &xpsTrap);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get xpsTrap for saiTrap %d error %d\n", trapType,
                           saiRetVal);
            return saiRetVal;
        }

        pDefTrapEntry = xpSaiHostIntfDefaultTrapGet(trapType);
        if (pDefTrapEntry == NULL)
        {
            XP_SAI_LOG_ERR("Invalid Host Interface Trap %d\n", trapType);
            return SAI_STATUS_INVALID_PARAMETER;
        }

        saiRetVal = xpSaiConvertSaiPacketAction2xps(pTrapEntry->action, &pktCmd);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to convert trap action, error %d\n", saiRetVal);
            return saiRetVal;
        }

        saiRetVal = xpSaiConvertSaiPacketAction2xps(pDefTrapEntry->action,
                                                    &defaultPktCmd);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to convert trap default action, error %d\n", saiRetVal);
            return saiRetVal;
        }

        saiRetVal = xpSaiCoppEntryAdd(xpsDevId, pTrapEntry->validPort, policerId,
                                      pktCmd, (uint32_t)xpsTrap, defaultPktCmd);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not add copp entry, reason code %d, error %d\n",
                           (uint32_t)xpsTrap, saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetHostInterfaceTrapGroupAttrPolicer

sai_status_t xpSaiSetHostInterfaceTrapGroupAttrPolicer(sai_object_id_t
                                                       trapGroupOid, sai_attribute_value_t atrValue)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    sai_object_id_t oldPolicerOid = SAI_NULL_OBJECT_ID;
    xpSaiHostInterfaceGroupDbEntryT *trapGroupEntry = NULL;

    XP_SAI_LOG_DBG("Called.\n");

    if (trapGroupOid == SAI_NULL_OBJECT_ID)
    {
        XP_SAI_LOG_ERR("Invalid object id: status=%i.\n", SAI_STATUS_INVALID_OBJECT_ID);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(trapGroupOid, SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP))
    {
        XP_SAI_LOG_ERR("Invalid object type: trapGroupOid=%lu, objIdType=i%, status=%i.\n",
                       trapGroupOid, xpSaiObjIdTypeGet(trapGroupOid), SAI_STATUS_INVALID_OBJECT_TYPE);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    saiStatus = xpSaiHostIntfGroupDbInfoGet(trapGroupOid, &trapGroupEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHostIntfGroupDbInfoGet failed: trapGroupOid=%lu, status=%i.\n",
                       trapGroupOid, saiStatus);
        return saiStatus;
    }

    if ((atrValue.oid != SAI_NULL_OBJECT_ID) &&
        (!XDK_SAI_OBJID_TYPE_CHECK(atrValue.oid, SAI_OBJECT_TYPE_POLICER)))
    {
        XP_SAI_LOG_ERR("Invalid object type: policerOid=%lu, objIdType=i%, status=%i.\n",
                       atrValue.oid, xpSaiObjIdTypeGet(atrValue.oid), SAI_STATUS_INVALID_OBJECT_TYPE);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    if (trapGroupEntry->policerOid == atrValue.oid)
    {
        return SAI_STATUS_SUCCESS;
    }

    oldPolicerOid = trapGroupEntry->policerOid;
    trapGroupEntry->policerOid = atrValue.oid;

    if (oldPolicerOid != SAI_NULL_OBJECT_ID)
    {
        saiStatus = xpSaiHostInterfacePolicerEntryRemove(oldPolicerOid);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiHostInterfacePolicerEntryRemove failed: policerOid=%lu, status=%i.\n",
                           oldPolicerOid, saiStatus);
            return saiStatus;
        }
    }

    if (trapGroupEntry->policerOid != SAI_NULL_OBJECT_ID)
    {
        saiStatus = xpSaiHostInterfacePolicerEntryAdd(trapGroupEntry->policerOid);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiHostInterfacePolicerEntryAdd failed: policerOid=%lu, status=%i.\n",
                           trapGroupEntry->policerOid, saiStatus);
            return saiStatus;
        }
    }

    saiStatus = xpSaiHostInterfacePolicerApply(trapGroupOid, oldPolicerOid);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHostInterfacePolicerApply failed: "
                       "trapGroupOid=%lu, oldPolicerOid=%lu, status=%i.\n",
                       trapGroupOid, oldPolicerOid, saiStatus);
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

//xpSaiSetHostInterfaceTrapGroupAttribute
//
sai_status_t xpSaiSetHostInterfaceTrapGroupAttribute(sai_object_id_t
                                                     hostif_trap_group_id, const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    retVal = xpSaiAttrCheck(1, attr,
                            HOSTIF_TRAP_GROUP_VALIDATION_ARRAY_SIZE, hostif_trap_group_attribs,
                            SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    switch (attr->id)
    {
        case SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE:
            {
                retVal = xpSaiSetHostInterfaceTrapGroupAttrAdminState(hostif_trap_group_id,
                                                                      attr->value);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("failed to set SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE, for trapGroupId %lu\n",
                                   (unsigned long int)hostif_trap_group_id);
                    return  retVal;
                }
                break;
            }
        case SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE:
            {
                retVal = xpSaiSetHostInterfaceTrapGroupAttrQueue(hostif_trap_group_id,
                                                                 attr->value);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("failed to set SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE, for trapGroupId %lu\n",
                                   (unsigned long int)hostif_trap_group_id);
                    return SAI_STATUS_SUCCESS;
                }
                break;
            }

        case SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER:
            {
                retVal = xpSaiSetHostInterfaceTrapGroupAttrPolicer(hostif_trap_group_id,
                                                                   attr->value);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("[skip] failed to set SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER, for trapGroupId %lu\n",
                                   (unsigned long int)hostif_trap_group_id);
                    return SAI_STATUS_SUCCESS;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Attribute %d is unknown\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            }
    }
    return  retVal;
}

//xpSaiGetHostInterfaceTrapGroupAttrAdminState

sai_status_t xpSaiGetHostInterfaceTrapGroupAttrAdminState(
    sai_object_id_t hostif_trap_group_id, sai_attribute_value_t* value)
{
    sai_status_t                    saiRetVal  = SAI_STATUS_FAILURE;
    xpSaiHostInterfaceGroupDbEntryT *pGrpEntry = NULL;

    saiRetVal = xpSaiHostIntfGroupDbInfoGet(hostif_trap_group_id, &pGrpEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find a trap group in DB\n");
        return saiRetVal;
    }

    value->booldata = pGrpEntry->adminState;

    return SAI_STATUS_SUCCESS;
}

//xpSaiGetHostInterfaceTrapGroupAttrQueue

sai_status_t xpSaiGetHostInterfaceTrapGroupAttrQueue(sai_object_id_t
                                                     hostif_trap_group_id, sai_attribute_value_t* value)
{
    sai_status_t                    saiRetVal  = SAI_STATUS_FAILURE;
    xpSaiHostInterfaceGroupDbEntryT *pGrpEntry = NULL;

    saiRetVal = xpSaiHostIntfGroupDbInfoGet(hostif_trap_group_id, &pGrpEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find a trap group in DB\n");
        return saiRetVal;
    }

    value->u32 = pGrpEntry->queue;

    return SAI_STATUS_SUCCESS;
}

//xpSaiGetHostInterfaceTrapGroupAttrPolicer

sai_status_t xpSaiGetHostInterfaceTrapGroupAttrPolicer(sai_object_id_t
                                                       hostif_trap_group_id, sai_attribute_value_t* value)
{
    sai_status_t                    saiRetVal  = SAI_STATUS_FAILURE;
    xpSaiHostInterfaceGroupDbEntryT *pGrpEntry = NULL;

    saiRetVal = xpSaiHostIntfGroupDbInfoGet(hostif_trap_group_id, &pGrpEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find a trap group in DB\n");
        return saiRetVal;
    }

    value->oid = pGrpEntry->policerOid;

    return SAI_STATUS_SUCCESS;
}

//xpSaiGetHostInterfaceTrapGroupAttribute
//
static sai_status_t xpSaiGetHostInterfaceTrapGroupAttribute(
    sai_object_id_t hostif_trap_group_id, sai_attribute_t *attr,
    uint32_t attr_index)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    switch (attr->id)
    {
        case SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE:
            {
                saiRetVal = xpSaiGetHostInterfaceTrapGroupAttrAdminState(hostif_trap_group_id,
                                                                         &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("failed to get SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE, for trapGroupId %lu\n",
                                   (unsigned long int) hostif_trap_group_id);
                    return saiRetVal;
                }
                break;
            }
        case SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE:
            {
                saiRetVal = xpSaiGetHostInterfaceTrapGroupAttrQueue(hostif_trap_group_id,
                                                                    &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("failed to get SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE, for trapGroupId %lu\n",
                                   (unsigned long int) hostif_trap_group_id);
                    return saiRetVal;
                }
                break;
            }
        case SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER:
            {
                saiRetVal = xpSaiGetHostInterfaceTrapGroupAttrPolicer(hostif_trap_group_id,
                                                                      &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("failed to get SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER, for trapGroupId %lu\n",
                                   (unsigned long int) hostif_trap_group_id);
                    return saiRetVal;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Attribute %d is unknown\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(attr_index);
            }
    }
    return saiRetVal;
}

//Func: xpSaiGetHostInterfaceTrapGroupAttributes

static sai_status_t xpSaiGetHostInterfaceTrapGroupAttributes(
    sai_object_id_t hostif_trap_group_id, uint32_t attr_count,
    sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               HOSTIF_TRAP_GROUP_VALIDATION_ARRAY_SIZE, hostif_trap_group_attribs,
                               SAI_COMMON_API_GET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetHostInterfaceTrapGroupAttribute(hostif_trap_group_id,
                                                            &attr_list[count], count);
        if (SAI_STATUS_SUCCESS != saiRetVal)
        {
            XP_SAI_LOG_ERR("failed for trapGroupId %lu\n",
                           (long unsigned int) hostif_trap_group_id);
            return saiRetVal;
        }
    }
    return saiRetVal;
}

//Func: xpSaiBulkGetHostInterfaceTrapGroupAttributes

sai_status_t xpSaiBulkGetHostInterfaceTrapGroupAttributes(sai_object_id_t id,
                                                          uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountHostInterfaceTrapGroupAttribute(&maxcount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get max attribute count!\n");
        return SAI_STATUS_FAILURE;
    }

    if (*attr_count < maxcount)
    {
        *attr_count = maxcount;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    for (uint32_t count = 0; count < maxcount; count++)
    {
        attr_list[idx].id = SAI_HOSTIF_TRAP_GROUP_ATTR_START + count;
        saiRetVal = xpSaiGetHostInterfaceTrapGroupAttribute(id, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
        else if (saiRetVal == SAI_STATUS_ITEM_NOT_FOUND)
        {
            /* Host interface object not found in DB */
            XP_SAI_LOG_ERR("failed for object ID %lu\n", id);
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostInterfaceApiInit

XP_STATUS xpSaiHostInterfaceApiInit(uint64_t flag,
                                    const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    _xpSaiHostInterfaceApi = (sai_hostif_api_t *)xpMalloc(sizeof(sai_hostif_api_t));
    if (NULL == _xpSaiHostInterfaceApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiHostInterfaceApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    memset(_xpSaiHostInterfaceApi, 0, sizeof(sai_hostif_api_t));

    _xpSaiHostInterfaceApi->create_hostif = xpSaiCreateHostInterface;
    _xpSaiHostInterfaceApi->remove_hostif = xpSaiRemoveHostInterface;
    _xpSaiHostInterfaceApi->set_hostif_attribute = xpSaiSetHostInterfaceAttribute;
    _xpSaiHostInterfaceApi->get_hostif_attribute = xpSaiGetHostInterfaceAttributes;

    _xpSaiHostInterfaceApi->create_hostif_table_entry =
        xpSaiCreateHostInterfaceTableEntry;
    _xpSaiHostInterfaceApi->remove_hostif_table_entry =
        xpSaiRemoveHostInterfaceTableEntry;
    _xpSaiHostInterfaceApi->set_hostif_table_entry_attribute =
        xpSaiSetHostInterfaceTableEntryAttribute;
    _xpSaiHostInterfaceApi->get_hostif_table_entry_attribute =
        xpSaiGetHostInterfaceTableEntryAttributes;

    _xpSaiHostInterfaceApi->create_hostif_trap_group =
        xpSaiCreateHostInterfaceTrapGroup;
    _xpSaiHostInterfaceApi->remove_hostif_trap_group =
        xpSaiRemoveHostInterfaceTrapGroup;
    _xpSaiHostInterfaceApi->set_hostif_trap_group_attribute =
        xpSaiSetHostInterfaceTrapGroupAttribute;
    _xpSaiHostInterfaceApi->get_hostif_trap_group_attribute =
        xpSaiGetHostInterfaceTrapGroupAttributes;

    _xpSaiHostInterfaceApi->create_hostif_trap = xpSaiCreateHostInterfaceTrap;
    _xpSaiHostInterfaceApi->remove_hostif_trap = xpSaiRemoveHostInterfaceTrap;
    _xpSaiHostInterfaceApi->set_hostif_trap_attribute =
        xpSaiSetHostInterfaceTrapAttribute;
    _xpSaiHostInterfaceApi->get_hostif_trap_attribute =
        xpSaiGetHostInterfaceTrapAttributes;

    _xpSaiHostInterfaceApi->create_hostif_user_defined_trap =
        xpSaiCreateHostInterfaceUserDefinedTrap;
    _xpSaiHostInterfaceApi->remove_hostif_user_defined_trap =
        xpSaiRemoveHostInterfaceUserDefinedTrap;
    _xpSaiHostInterfaceApi->set_hostif_user_defined_trap_attribute =
        xpSaiSetHostInterfaceUserDefinedTrapAttribute;
    _xpSaiHostInterfaceApi->get_hostif_user_defined_trap_attribute =
        xpSaiGetHostInterfaceUserDefinedTrapAttributes;

#ifndef MAC
    _xpSaiHostInterfaceApi->recv_hostif_packet = xpSaiFdPacketRecv;
    _xpSaiHostInterfaceApi->send_hostif_packet = xpSaiFdPacketSend;
#else
    _xpSaiHostInterfaceApi->recv_hostif_packet = NULL;
    _xpSaiHostInterfaceApi->send_hostif_packet = NULL;
#endif

    saiRetVal = xpSaiApiRegister(SAI_API_HOSTIF, (void*)_xpSaiHostInterfaceApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register hostif API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return  retVal;
}

//Func: xpSaiHostInterfaceApiDeinit

XP_STATUS xpSaiHostInterfaceApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiHostInterfaceApiDeinit\n");

    xpFree(_xpSaiHostInterfaceApi);
    _xpSaiHostInterfaceApi = NULL;

    return  retVal;
}


//Func: xpSaiHostInterfaceInit

sai_status_t xpSaiHostInterfaceInit(xpsDevice_t xpsDevId)
{
    sai_status_t        saiRetVal       = SAI_STATUS_SUCCESS;
    XP_STATUS           retVal          = XP_NO_ERR;
    sai_object_id_t     trapGroupOid    = SAI_NULL_OBJECT_ID;
    xpsScope_t          scope           = xpSaiScopeFromDevGet(xpsDevId);
    xpsHashIndexList_t  indexList;
    xpCoppEntryData_t   coppEntry;
    memset(&indexList, 0x0, sizeof(xpsHashIndexList_t));
    memset(&coppEntry, 0x0, sizeof(xpCoppEntryData_t));


    retVal = xpsAllocatorInitIdAllocator(scope, XP_SAI_ALLOC_HOST_INTF,
                                         XP_SAI_HOST_INTF_MAX_IDS,
                                         XP_SAI_HOST_INTF_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI Host Interface ID allocator\n");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsAllocatorInitIdAllocator(scope, XP_SAI_ALLOC_HOST_INTF_GRP,
                                         XP_SAI_HOST_INTF_GRP_MAX_IDS,
                                         XP_SAI_HOST_INTF_GRP_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI Host Interface Group ID allocator\n");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsAllocatorInitIdAllocator(scope, XP_SAI_ALLOC_HOST_INTF_TABLE_ENTRY,
                                         XP_SAI_HOST_INTF_TABLE_ENTRY_MAX_IDS,
                                         XP_SAI_HOST_INTF_TABLE_ENTRY_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI Host Interface Table Entry ID allocator\n");
        return xpsStatus2SaiStatus(retVal);
    }

    saiRetVal = xpSaiHostIntfDbInit(xpsDevId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHostIntfDbInit failed");
        return saiRetVal;
    }
    /* QueueInit Code crashing ,fix it */
    saiRetVal = xpSaiCreateHostInterfaceTrapGroup(&trapGroupOid, xpsDevId, 0, NULL);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create a default trap group, error %d\n", saiRetVal);
        return saiRetVal;
    }

    xpSaiSwitchDefaultTrapGroupSet(trapGroupOid);
    for (uint8_t entry=0; entry < gcustNumOfConrolMacEntry; entry++)
    {
        sai_mac_t           controlMac;
        memcpy(controlMac, gcustConrolMacEntryList[entry], sizeof(sai_mac_t));
        xpSaiMacReverse(controlMac);

        saiRetVal = xpsVlanSetGlobalControlMac(xpsDevId, controlMac);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to init control mac entries");
        }
    }

    // Creating 1K Special Copp Rule (Source Port being 128) for deny used for EACL + COPP
    coppEntry.updatePktCmd = 0x1;
    for (uint32_t entry=0; entry < 1024; entry++)
    {
        memset(&indexList, 0x0, sizeof(xpsHashIndexList_t));
    }


    /* Initialize Trap Table to the default state */
    for (uint32_t i = 0; i < ARRAY_SIZE(xpSaiHostIntfDefaultTraps); i++)
    {
        if (!xpSaiHostIntfDefaultTraps[i].enable)
        {
            continue;
        }

        saiRetVal = xpSaiHostInterfaceTrapDefaultApply(xpsDevId,
                                                       xpSaiHostIntfDefaultTraps[i].trapType);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to create a trap, trap_id %d, error %d\n",
                           xpSaiHostIntfDefaultTraps[i].trapType, saiRetVal);
            return saiRetVal;
        }
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostInterfaceDeInit

sai_status_t xpSaiHostInterfaceDeInit(xpsDevice_t xpsDevId)
{
    sai_status_t      saiRetVal   = SAI_STATUS_SUCCESS;

    saiRetVal = xpSaiHostIntfDbDeInit(xpsDevId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHostIntfDbDeInit failed");
        return saiRetVal;
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostInterfaceTrapIdParse


sai_status_t xpSaiHostInterfaceTrapIdParse(xpsDevice_t devId,
                                           uint32_t reasonCode, sai_attribute_t *attr)
{
    attr->id = SAI_HOSTIF_PACKET_ATTR_HOSTIF_TRAP_ID;
    attr->value.oid = 0;

    for (uint32_t i = 0; i < ARRAY_SIZE(xpSaiHostIntfDefaultTraps); i++)
    {
        for (uint32_t j = 0; j < xpSaiHostIntfDefaultTraps[i].reasonCodeNum; j++)
        {
            if (xpSaiHostIntfDefaultTraps[i].reasonCode[j] == reasonCode &&
                xpSaiHostIntfDefaultTraps[i].enable)
            {
                attr->value.oid = (sai_object_id_t)xpSaiHostIntfDefaultTraps[i].trapType;
                return SAI_STATUS_SUCCESS;
            }
        }
    }

    attr->value.oid = (sai_object_id_t)reasonCode;
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostInterfacePortTypeParse

sai_status_t xpSaiHostInterfacePortTypeParse(xpsDevice_t devId,
                                             xpsInterfaceType_e intfType,
                                             xpsInterfaceId_t intfId, sai_attribute_t *attr)
{
    sai_status_t    saiStatus   = SAI_STATUS_SUCCESS;

    if (intfType == XPS_PORT)
    {
        attr->id = SAI_HOSTIF_PACKET_ATTR_INGRESS_PORT;

        saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId, (sai_uint64_t)intfId,
                                     &attr->value.oid);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI port object could not be created, error %d\n", saiStatus);
            return saiStatus;
        }
    }
    else if (intfType == XPS_LAG)
    {
        attr->id = SAI_HOSTIF_PACKET_ATTR_INGRESS_LAG;

        saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_LAG, devId, (sai_uint64_t)intfId,
                                     &attr->value.oid);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI LAG object could not be created, error %d\n", saiStatus);
            return saiStatus;
        }
    }
    else
    {
        XP_SAI_LOG_ERR("Unsupported interface type, type %d\n", intfType);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}
void xpSaiSetSoftFwdInfo(sai_mac_t dstMac, sai_object_id_t outIntf)
{
    XP_SAI_LOG_DBG("dstMac: %02x:%02x:%02x:%02x:%02x:%02x, outIntf: 0x%" PRIx64
                   "\n",
                   dstMac[0], dstMac[1], dstMac[2], dstMac[3], dstMac[4], dstMac[5], outIntf);

    memcpy(gXpSaiSoftFwdInfo.dstMac, dstMac, sizeof(sai_mac_t));
    gXpSaiSoftFwdInfo.outIntf = outIntf;
    gXpSaiSoftFwdInfo.isValid = true;
}

void xpSaiResetSoftFwdInfo()
{
    uint8_t *dstMac;

    dstMac = gXpSaiSoftFwdInfo.dstMac;

    XP_SAI_LOG_DBG("Saved Info. softFwd %s, dstMac: %02x:%02x:%02x:%02x:%02x:%02x, "
                   "outIntf: 0x%" PRIx64 "\n",
                   (gXpSaiSoftFwdInfo.isValid) ? "Enabled" : "Disabled",
                   dstMac[0], dstMac[1], dstMac[2], dstMac[3], dstMac[4], dstMac[5],
                   gXpSaiSoftFwdInfo.outIntf);

    gXpSaiSoftFwdInfo.isValid = false;
}

void xpSaiDumpSoftFwdInfo()
{
    uint8_t *dstMac;

    dstMac = gXpSaiSoftFwdInfo.dstMac;

    XP_SAI_LOG_DBG("softFwd %s, dstMac: %02x:%02x:%02x:%02x:%02x:%02x, "
                   "outIntf: 0x%" PRIx64 "\n",
                   (gXpSaiSoftFwdInfo.isValid) ? "Enabled" : "Disabled",
                   dstMac[0], dstMac[1], dstMac[2], dstMac[3], dstMac[4], dstMac[5],
                   gXpSaiSoftFwdInfo.outIntf);
}

void xpSaiEnablePktDump(uint32_t maxCount)
{
    gXpSaiPktDumpEnable       = true;
    gXpSaiDumpedPktCount      = 0;
    gXpSaiDumpAllowedPktCount = maxCount;
}

void xpSaiDisablePktDump()
{
    gXpSaiPktDumpEnable = false;
    gXpSaiDumpedPktCount = 0;
    gXpSaiDumpAllowedPktCount = XP_SAI_DEFAULT_DUMP_PKT_COUNT;
}


void xpSaiDumpPkt(void *pkt, uint16_t len)
{
    uint32_t     i = 0, j = 0, str_index = 0;
    char         str [128] = "";
    sai_uint8_t *pData;

    gXpSaiDumpedPktCount++;

    XP_SAI_LOG_DBG("Pkt %d, Len: %d\n", gXpSaiDumpedPktCount, len);
    pData = (sai_uint8_t *) pkt;
    while (i < len)
    {
        for (j = 0; j < 16; j++)
        {
            if (i >= len)
            {
                break;
            }

            str_index += sprintf(&str [str_index], "%02x%s", *pData,
                                 ((j % 4) == 3) ? " " : "");
            pData++;
            i++;
        }

        XP_SAI_LOG_DBG("%s\n", str);
        str_index = 0;
    }
}

#define XP_SAI_VLAN_ETHER_TYPE              0x8100
#define XP_SAI_ARP_ETHER_TYPE               0x0806
#define XP_SAI_IPV4_ETHER_TYPE              0x0800
#define XP_SAI_IPV6_ETHER_TYPE              0x86DD
#define XP_SAI_VLAN_TAG_SIZE                     4
#define XP_SAI_ETHER_TYPE_SIZE                   2
#define XP_SAI_VLAN_ID_SIZE                      2
#define XP_SAI_OUTER_ETHER_TYPE_OFFSET          12
#define XP_SAI_TTL_OFFSET_IN_IPV4_HDR            8
#define XP_SAI_HOP_LIMIT_OFFSET_IN_IPV6_HDR      7

void xpSaiGetPktDetails(void *inPkt, uint16_t len, uint8_t *pOutTtl,
                        bool *pOutIsArp, bool *pOutIsVlanUntagged)
{
    uint32_t  offSet;
    uint32_t  ipHdrOffSet;
    uint16_t  etherType;
    uint8_t  *pkt;

    *pOutTtl            = 0;
    *pOutIsArp          = false;
    *pOutIsVlanUntagged = true;

    pkt = (uint8_t *) inPkt;

    offSet = XP_SAI_OUTER_ETHER_TYPE_OFFSET;
    memcpy(&etherType, pkt + offSet, XP_SAI_ETHER_TYPE_SIZE);
    etherType = ntohs(etherType);

    if (etherType == XP_SAI_VLAN_ETHER_TYPE)
    {
        *pOutIsVlanUntagged = false;

        offSet += XP_SAI_VLAN_TAG_SIZE;
        memcpy(&etherType, pkt + offSet, XP_SAI_ETHER_TYPE_SIZE);

        etherType = ntohs(etherType);
    }

    ipHdrOffSet = offSet + XP_SAI_ETHER_TYPE_SIZE;

    if (etherType == XP_SAI_IPV4_ETHER_TYPE)
    {
        /* IPv4 */
        *pOutTtl = pkt [ipHdrOffSet + XP_SAI_TTL_OFFSET_IN_IPV4_HDR];
    }
    else if (etherType == XP_SAI_IPV6_ETHER_TYPE)
    {
        /* IPv6 */
        *pOutTtl = pkt [ipHdrOffSet + XP_SAI_HOP_LIMIT_OFFSET_IN_IPV6_HDR];
    }
    else if (etherType == XP_SAI_ARP_ETHER_TYPE)
    {
        *pOutIsArp = true;
    }
}




//Func: xpSaiHostInterfaceCbThread

void* xpSaiHostInterfaceCbThread(void *pArg)
{
    xphRxHdr                rxHdata;
    struct xpPacketInfo     rxPacket;
    struct xpPacketInfo*    packets[1];
    xpPacketInterface       packetIntfType  = XP_NETDEV_DMA;
    XP_STATUS               retVal          = XP_NO_ERR;
    bool done = false;

    memset(&rxHdata, 0, sizeof(rxHdata));
    memset(&rxPacket, 0, sizeof(rxPacket));
    memset(packets, 0, sizeof(packets));

    XP_SAI_LOG_DBG("SAI Host interface thread is started\n");
    /*Prepare thread enviroment.*/
    struct sched_param param;
    int ret;
    memset(&param, 0, sizeof(struct sched_param));
    //Set packet driver thread's higher policy/priority as compared to SCHED_OTHER
    //See man 7 sched for detailed info for threading policy.
    param.sched_priority = 99;
    ret = pthread_setschedparam(pthread_self(), SCHED_RR, &param);
    if (0 != ret)
    {
        XP_SAI_LOG_ERR("SAI Host interface thread policy/priority changes API failed\n");
    }
    pArg = pArg;

    packets[0] = &rxPacket;
    packets[0]->buf = packetBuf;
    while (!done)
    {

        if (xpSaiGetExitStatus())
        {
            break;
        }
        packets[0]->bufSize = XP_MAX_PACKET_SIZE;
        // delay while polling in DMA mode
        if ((xpGetSalType() != XP_SAL_HW_TYPE) ||
            (xpGetSalType() == XP_SAL_KERN_TYPE) || (packetIntfType == XP_DMA_TAP))
        {
            if (retVal == XP_ERR_PKT_NOT_AVAILABLE)
            {
                xpSaiSleepMsec(5);
            }
        }
    }

    XP_SAI_LOG_DBG("SAI Host interface thread is finished\n");

    return NULL;
}


//Func: xpSaiHostInterfaceTapThread

void* xpSaiHostInterfaceTapThread(void *pArg)
{
    bool done = false;
    XP_SAI_LOG_DBG("SAI Host interface (TAP update) thread is started\n");
    /*Prepare thread enviroment.*/
    while (!done)
    {

        if (xpSaiGetExitStatus())
        {
            break;
        }
    }

    XP_SAI_LOG_DBG("SAI Host interface (TAP update) thread is finished\n");
    return NULL;
}

sai_status_t xpSaiMaxCountHostInterfaceAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_HOSTIF_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountHostInterfaceObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCount(XP_SCOPE_DEFAULT, xpSaiHostIntfChannelDbHandle,
                              count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetHostInterfaceObjectList(uint32_t *object_count,
                                             sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;
    xpsDevice_t     devId = xpSaiGetDevId();

    xpSaiHostInterfaceChannelDbEntryT   *pHostifNext    = NULL;

    saiRetVal = xpSaiCountHostInterfaceObjects(&objCount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get object count!\n");
        return saiRetVal;
    }

    if (objCount > *object_count)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Buffer overflow occured\n");
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if (object_list == NULL)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *object_count = objCount;

    for (uint32_t i = 0; i < *object_count; i++)
    {
        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, xpSaiHostIntfChannelDbHandle,
                                     pHostifNext, (void **)&pHostifNext);
        if (retVal != XP_NO_ERR || pHostifNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve hostif object, error %d\n", retVal);
            return SAI_STATUS_FAILURE;
        }

        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_HOSTIF, devId,
                                     (sai_uint64_t)pHostifNext->keyId, &object_list[i].key.object_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiMaxCountHostInterfaceTrapAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_HOSTIF_TRAP_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiHostInterfaceTrapCountGet(sai_object_type_t objType,
                                                   uint32_t *count)
{
    XP_STATUS   retVal      = XP_NO_ERR;
    uint32_t    trapCount   = 0;
    xpsScope_t  scopeId     = XP_SCOPE_DEFAULT;

    xpSaiHostInterfaceTrapDbEntryT  *pTrapNext  = NULL;

    if (count == NULL)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetNextData(scopeId, xpSaiHostIntfTrapDbHandle, NULL,
                                 (void **)&pTrapNext);

    while (pTrapNext && (retVal == XP_NO_ERR))
    {
        if (xpSaiObjIdTypeGet(pTrapNext->trapOid) == objType)
        {
            trapCount++;
        }

        retVal = xpsStateGetNextData(scopeId, xpSaiHostIntfTrapDbHandle, pTrapNext,
                                     (void **)&pTrapNext);
    }

    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve hostif trap object, error %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    *count = trapCount;
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountHostInterfaceTrapObjects(uint32_t *count)
{
    return xpSaiHostInterfaceTrapCountGet(SAI_OBJECT_TYPE_HOSTIF_TRAP, count);
}

sai_status_t xpSaiGetHostInterfaceTrapObjectList(uint32_t *object_count,
                                                 sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;
    xpsScope_t      scopeId     = XP_SCOPE_DEFAULT;

    xpSaiHostInterfaceTrapDbEntryT  *pTrapNext    = NULL;

    saiRetVal = xpSaiCountHostInterfaceTrapObjects(&objCount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get object count!\n");
        return saiRetVal;
    }

    if (objCount > *object_count)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Buffer overflow occured\n");
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if (object_list == NULL)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    objCount = 0;
    retVal = xpsStateGetNextData(scopeId, xpSaiHostIntfTrapDbHandle, NULL,
                                 (void **)&pTrapNext);

    while ((pTrapNext != NULL) && (retVal == XP_NO_ERR) &&
           (objCount < *object_count))
    {
        if (xpSaiObjIdTypeGet(pTrapNext->trapOid) == SAI_OBJECT_TYPE_HOSTIF_TRAP)
        {
            object_list[objCount++].key.object_id = pTrapNext->trapOid;
        }

        retVal = xpsStateGetNextData(scopeId, xpSaiHostIntfTrapDbHandle, pTrapNext,
                                     (void **)&pTrapNext);
    }

    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve hostif trap object, error %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    *object_count = objCount;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiMaxCountHostInterfaceUserTrapAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Failed to get MaxCount. Error : %d\n", XP_ERR_NULL_POINTER);
        return SAI_STATUS_FAILURE;
    }
    *count = SAI_HOSTIF_USER_DEFINED_TRAP_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountHostInterfaceUserTrapObjects(uint32_t *count)
{
    return xpSaiHostInterfaceTrapCountGet(SAI_OBJECT_TYPE_HOSTIF_USER_DEFINED_TRAP,
                                          count);
}

sai_status_t xpSaiGetHostInterfaceUserTrapObjectList(uint32_t *object_count,
                                                     sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;
    xpsScope_t      scopeId     = XP_SCOPE_DEFAULT;

    xpSaiHostInterfaceTrapDbEntryT  *pTrapNext    = NULL;

    saiRetVal = xpSaiCountHostInterfaceUserTrapObjects(&objCount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get object count!\n");
        return saiRetVal;
    }

    if (objCount > *object_count)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Buffer overflow occured\n");
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if (object_list == NULL)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    objCount = 0;
    retVal = xpsStateGetNextData(scopeId, xpSaiHostIntfTrapDbHandle, NULL,
                                 (void **)&pTrapNext);

    while ((pTrapNext != NULL) && (retVal == XP_NO_ERR) &&
           (objCount < *object_count))
    {
        if (xpSaiObjIdTypeGet(pTrapNext->trapOid) ==
            SAI_OBJECT_TYPE_HOSTIF_USER_DEFINED_TRAP)
        {
            object_list[objCount++].key.object_id = pTrapNext->trapOid;
        }

        retVal = xpsStateGetNextData(scopeId, xpSaiHostIntfTrapDbHandle, pTrapNext,
                                     (void **)&pTrapNext);
    }

    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve hostif trap object, error %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    *object_count = objCount;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiMaxCountHostInterfaceTrapGroupAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_HOSTIF_TRAP_GROUP_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountHostInterfaceTrapGroupObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCount(XP_SCOPE_DEFAULT, xpSaiHostIntfGroupDbHandle, count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetHostInterfaceTrapGroupObjectList(uint32_t *object_count,
                                                      sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;
    xpsDevice_t     devId = xpSaiGetDevId();

    xpSaiHostInterfaceGroupDbEntryT     *pGroupNext     = NULL;

    saiRetVal = xpSaiCountHostInterfaceTrapGroupObjects(&objCount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get object count!\n");
        return saiRetVal;
    }

    if (objCount > *object_count)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Buffer overflow occured\n");
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if (object_list == NULL)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *object_count = objCount;

    for (uint32_t i = 0; i < *object_count; i++)
    {
        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, xpSaiHostIntfGroupDbHandle,
                                     pGroupNext, (void **)&pGroupNext);
        if (retVal != XP_NO_ERR || pGroupNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve hostif group object, error %d\n", retVal);
            return SAI_STATUS_FAILURE;
        }

        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP, devId,
                                     (sai_uint64_t)pGroupNext->keyId, &object_list[i].key.object_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiMaxCountHostInterfaceTableEntryAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_HOSTIF_TABLE_ENTRY_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountHostInterfaceTableEntryObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCount(XP_SCOPE_DEFAULT, xpSaiHostIntfTableDbHandle, count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetHostInterfaceTableEntryObjectList(uint32_t *object_count,
                                                       sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;
    xpsDevice_t     devId = xpSaiGetDevId();

    xpSaiHostInterfaceTableDbEntryT     *pTableEntryNext    = NULL;

    saiRetVal = xpSaiCountHostInterfaceTableEntryObjects(&objCount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get object count!\n");
        return saiRetVal;
    }

    if (objCount > *object_count)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Buffer overflow occured\n");
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if (object_list == NULL)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *object_count = objCount;

    for (uint32_t i = 0; i < *object_count; i++)
    {
        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, xpSaiHostIntfTableDbHandle,
                                     pTableEntryNext, (void **)&pTableEntryNext);
        if (retVal != XP_NO_ERR || pTableEntryNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve hostif table entry object, error %d\n",
                           retVal);
            return SAI_STATUS_FAILURE;
        }

        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_HOSTIF_TABLE_ENTRY, devId,
                                     (sai_uint64_t)pTableEntryNext->keyId, &object_list[i].key.object_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiHostInterfaceTrapValidPortsInit(xpsDevice_t devId,
                                                  xpSaiHostInterfaceTrapDbEntryT *pTrapEntry)
{
    XP_STATUS   retVal      = XP_NO_ERR;
    uint32_t     maxPortNum  = 0;
    uint32_t i = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (pTrapEntry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return xpsStatus2SaiStatus(XP_ERR_NULL_POINTER);
    }

    retVal = xpsPortGetMaxNum(devId, &maxPortNum);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get maximum ports number, retVal %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateHeapMalloc(XP_SAI_HOST_INTF_MAX_IDS,
                                (void**)&pTrapEntry->validPort);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate validPort in trapEntry!\n");
        return xpsStatus2SaiStatus(retVal);
    }
    memset(pTrapEntry->validPort, 0, XP_SAI_HOST_INTF_MAX_IDS*sizeof(bool));

    XPS_GLOBAL_PORT_ITER(i, maxPortNum)
    {
        pTrapEntry->validPort[i] = true;
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiHostInterfaceTrapValidPortsDeinit(
    xpSaiHostInterfaceTrapDbEntryT *pTrapEntry)
{
    XP_STATUS   retVal      = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (pTrapEntry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return xpsStatus2SaiStatus(XP_ERR_NULL_POINTER);
    }

    retVal = xpsStateHeapFree(pTrapEntry->validPort);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not deallocate memory for valid ports list, retVal %d\n",
                       retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiHostInterfaceTrapQueueDropCounter(uint32_t queue,
                                                    uint64_t *DropPkts)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;
    xpSaiHostInterfaceGroupDbEntryT     *pGroupNext     = NULL;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!DropPkts)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return xpsStatus2SaiStatus(XP_ERR_NULL_POINTER);
    }

    saiRetVal = xpSaiCountHostInterfaceTrapGroupObjects(&objCount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get object count!\n");
        return saiRetVal;
    }

    for (uint32_t i = 0; i < objCount; i++)
    {
        xpSaiPolicerAttributes_t            PolicerAttrInfo = {};
        sai_stat_id_t   counter_ids[3] = {SAI_POLICER_STAT_GREEN_PACKETS, SAI_POLICER_STAT_YELLOW_PACKETS, SAI_POLICER_STAT_RED_PACKETS};
        uint64_t        counters[3] = {};
        uint32_t        number_of_counters = 3;

        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, xpSaiHostIntfGroupDbHandle,
                                     pGroupNext, (void **)&pGroupNext);
        if (retVal != XP_NO_ERR || pGroupNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve hostif group object, error %d\n", retVal);
            return SAI_STATUS_FAILURE;
        }

        if ((pGroupNext->queue != queue) ||
            (pGroupNext->policerOid == SAI_NULL_OBJECT_ID))
        {
            continue;
        }
        retVal = xpSaiGetPolicerAttrInfo(pGroupNext->policerOid, &PolicerAttrInfo);
        if (retVal != XP_NO_ERR)
        {
            continue;
        }
        saiRetVal = xpSaiGetPolicerStats(pGroupNext->policerOid, number_of_counters,
                                         counter_ids, counters);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            continue;
        }
        if (PolicerAttrInfo.greenPacketAction.s32 == SAI_PACKET_ACTION_DROP)
        {
            *DropPkts += counters[0];
        }
        if (PolicerAttrInfo.yellowPacketAction.s32 == SAI_PACKET_ACTION_DROP)
        {
            *DropPkts += counters[1];
        }
        if (PolicerAttrInfo.redPacketAction.s32 == SAI_PACKET_ACTION_DROP)
        {
            *DropPkts += counters[2];
        }
    }

    return SAI_STATUS_SUCCESS;
}
