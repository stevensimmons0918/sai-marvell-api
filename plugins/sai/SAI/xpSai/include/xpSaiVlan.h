// xpSaiVlan.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiVlan_h_
#define _xpSaiVlan_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _xpSaiVlanAttributesT
{
    sai_attribute_value_t vlanId;
    sai_attribute_value_t maxLearnedAddresses;
    sai_attribute_value_t stpInstance;
    sai_attribute_value_t learnDisable;
    sai_attribute_value_t countMode;
    sai_attribute_value_t ingressAcl;
    sai_attribute_value_t egressAcl;
    sai_attribute_value_t unknownUcFloodControlType;
    sai_attribute_value_t unknownUcFloodGroup;
    sai_attribute_value_t unknownMcFloodControlType;
    sai_attribute_value_t unknownMcFloodGroup;
    sai_attribute_value_t broadcastFloodControlType;
    sai_attribute_value_t broadcastFloodGroup;
    sai_attribute_value_t ipv4McastLookupKeyType;
    sai_attribute_value_t ipv6McastLookupKeyType;
    sai_attribute_value_t igmpSnoopingEnable;
} xpSaiVlanAttributesT;


/**
 * @brief Datastructure for vlan count mode
 */
typedef enum _sai_vlan_count_mode_t
{
    /** Count only number of packets*/
    SAI_VLAN_COUNT_MODE_PACKET_ONLY,

    /** Count number of packets and octets both*/
    SAI_VLAN_COUNT_MODE_PACKET_OCTET_BOTH,

    /** Max count mode */
    SAI_VLAN_COUNT_MODE_MAX,

} sai_vlan_count_mode_t;

/**
 * @brief Attribute Id in sai_set_vlan_attribute() and
 * sai_get_vlan_attribute() calls
 */
typedef enum _sai_vlan_custom_attr_t
{
    /** Custom range base value */
    //SAI_VLAN_ATTR_CUSTOM_RANGE_START = 0x10000000,

    /**
    * @brief VLAN count mode
    * @type sai_vlan_count_mode_t
    * @flags CREATE_AND_SET
    */
    SAI_VLAN_ATTR_COUNT_MODE = 0x10000000,

    /** End of custom range base */
    //SAI_VLAN_ATTR_CUSTOM_RANGE_END

} sai_vlan_custom_attr_t;


/*
16 bits - obj type
8  bits - dev id
40 bits - obj value ~ Reffering to XDK_SAI_OBJID_VALUE_BITS
    Divided:
        20 bits - VLAN ID
        20 bits - PORT ID/LAG ID
*/
#define SAI_VLAN_MEMBER_SUB_OBJID_VALUE_BITS 20
#define SAI_VLAN_MEMBER_OBJ_VALUE_MASK 0x00000000000FFFFFULL

sai_uint64_t xpSaiVlanMemberVlanIdValueGet(sai_object_id_t sai_object_id);
sai_uint64_t xpSaiVlanMemberPortIdValueGet(sai_object_id_t sai_object_id);
sai_status_t xpSaiVlanMemberObjIdCreate(sai_uint32_t dev_id,
                                        sai_uint64_t vlan_id, sai_uint64_t port_id, sai_object_id_t *sai_object_id);

sai_status_t xpSaiExcludeIntfAllVlans(sai_object_id_t portOid);
sai_status_t xpSaiVlanCreateDefaultVlanState(xpsDevice_t xpSaiDevId,
                                             sai_vlan_id_t vlanid);
sai_status_t xpSaiVlanCreateVlanMembersState(xpsDevice_t xpSaiDevId,
                                             sai_vlan_id_t vlanid);

sai_status_t xpSaiDeleteDefaultVlan(xpsDevice_t xpSaiDevId);

XP_STATUS xpSaiVlanInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiVlanDeInit(xpsDevice_t xpSaiDevId);

XP_STATUS xpSaiVlanApiInit(uint64_t flag,
                           const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiVlanApiDeinit();

sai_status_t xpSaiMaxCountVlanAttribute(uint32_t *count);
sai_status_t xpSaiMaxCountVlanMemberAttribute(uint32_t *count);

sai_status_t xpSaiCountVlanObjects(uint32_t *count);
sai_status_t xpSaiCountVlanMemberObjects(uint32_t *count);

sai_status_t xpSaiGetVlanObjectList(uint32_t *object_count,
                                    sai_object_key_t *object_list);
sai_status_t xpSaiGetVlanMemberObjectList(uint32_t *object_count,
                                          sai_object_key_t *object_list);

sai_status_t xpSaiBulkGetVlanAttributes(sai_object_id_t id,
                                        uint32_t *attr_count, sai_attribute_t *attr_list);
sai_status_t xpSaiBulkGetVlanMemberAttributes(sai_object_id_t id,
                                              uint32_t *attr_count, sai_attribute_t *attr_list);

sai_status_t xpSaiConvertSaiFloodControlType2xps(sai_vlan_flood_control_type_t
                                                 sai_packet_action, xpsPktCmd_e* pXpsPktCmd);
sai_status_t xpSaiConvertSaiMcastLookupKeyType2xps(
    sai_vlan_mcast_lookup_key_type_t mcast_lookup_key_type,
    xpsVlanBridgeMcMode_e *pXpsBrMcMode);
sai_status_t xpSaiConvertXpsMcastLookupKeyType2sai(xpsVlanBridgeMcMode_e
                                                   xpsBrMcMode, sai_vlan_mcast_lookup_key_type_t *mcast_lookup_key_type);

sai_status_t xpSaiSetVlanAttrUnknownUnicastFloodControlType(xpsVlan_t xpsVlanId,
                                                            sai_int32_t value, bool cpuFloodEnable);
sai_status_t xpSaiGetVlanAttrUnknownUnicastFloodControlType(xpsVlan_t xpsVlanId,
                                                            sai_int32_t* value);
sai_status_t xpSaiSetVlanAttrUnknownMulticastFloodControlType(
    xpsVlan_t xpsVlanId, sai_int32_t value, bool cpuFloodEnable);
sai_status_t xpSaiGetVlanAttrUnknownMulticastFloodControlType(
    xpsVlan_t xpsVlanId,  sai_int32_t* value);
sai_status_t xpSaiSetVlanAttrBroadcastFloodControlType(xpsVlan_t xpsVlanId,
                                                       sai_int32_t value, bool cpuFloodEnable);
sai_status_t xpSaiGetVlanAttrBroadcastFloodControlType(xpsVlan_t xpsVlanId,
                                                       sai_int32_t* value);

sai_status_t xpSaiSetVlanBroadcastFloodControlGroup(sai_object_id_t vlanIdOid,
                                                    sai_object_id_t listOid);
sai_status_t xpSaiResetVlanBroadcastFloodControlGroup(sai_object_id_t vlanIdObj,
                                                      int32_t floodType);
sai_status_t xpSaiSetVlanUnregMulicastFloodControlGroup(
    sai_object_id_t vlanIdOid, sai_object_id_t listOid);
sai_status_t xpSaiResetVlanUnregMulicastFloodControlGroup(
    sai_object_id_t vlanIdObj, int32_t floodType);

sai_status_t xpSaiBridgePortIsVlanMember(sai_object_id_t saiVlanOid,
                                         sai_object_id_t saiBrPortOid);
sai_status_t xpSaiSetVlanAttrIgmpSnoopingEnable(xpsVlan_t xpsVlanId,
                                                sai_attribute_value_t value);
sai_status_t xpSaiGetVlanAttrIgmpSnoopingEnable(xpsVlan_t xpsVlanId,
                                                sai_attribute_value_t *value);

void xpSaiVlanUpdatePktCmd(bool cpuFloodEnable, xpsPktCmd_e *pktCmd);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiVlan_h_
