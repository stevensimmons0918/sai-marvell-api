#include "saiShellVlan.h"
#include "include/luaSaiVlan.h"
#include "include/luaSaiUtil.h"
#include "saitypes.h"

sai_uint32_t sai_create_vlan_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint16_t vlan_id)
{
    sai_uint64_t switch_oid_acc = sai_get64bit_oid(switch_id,
                                                   SAI_OBJECT_TYPE_SWITCH);
    sai_status_t ret;
    ret = sai_create_vlan_default(switch_oid_acc, vlan_id);
    return ret;
}

sai_uint32_t sai_create_vlan_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t ingress_acl,
    IN sai_vlan_mcast_lookup_key_type_t ipv4_mcast_lookup_key_type,
    IN uint8_t custom_igmp_snooping_enable,
    IN sai_uint32_t egress_acl,
    IN sai_uint16_t vlan_id,
    IN sai_vlan_flood_control_type_t unknown_multicast_flood_control_type,
    IN sai_uint32_t unknown_multicast_flood_group,
    IN sai_uint32_t broadcast_flood_group,
    IN sai_vlan_flood_control_type_t broadcast_flood_control_type)
{
    sai_uint64_t switch_oid_acc = sai_get64bit_oid(switch_id,
                                                   SAI_OBJECT_TYPE_SWITCH);
    sai_status_t ret;
    ret = sai_create_vlan_custom(switch_oid_acc, ingress_acl,
                                 ipv4_mcast_lookup_key_type,
                                 custom_igmp_snooping_enable, egress_acl, vlan_id,
                                 unknown_multicast_flood_control_type, unknown_multicast_flood_group,
                                 broadcast_flood_group, broadcast_flood_control_type);
    return ret;
}

sai_uint32_t sai_create_vlan_member_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t vlan_id,
    IN sai_uint32_t bridge_port_id)
{
    sai_uint64_t switch_oid_acc = sai_get64bit_oid(switch_id,
                                                   SAI_OBJECT_TYPE_SWITCH);
    sai_uint64_t vlan_oid_acc = sai_get64bit_oid(vlan_id, SAI_OBJECT_TYPE_VLAN);
    sai_uint64_t bridge_oid_acc = sai_get64bit_oid(bridge_port_id,
                                                   SAI_OBJECT_TYPE_BRIDGE_PORT);
    sai_uint64_t ret;
    ret = sai_create_vlan_member_default(switch_oid_acc, vlan_oid_acc,
                                         bridge_oid_acc);
    return ret&0xFFFFFFFF;
}

sai_status_t sai_remove_vlan_member_wrp(
    IN sai_uint32_t vlan_member_id)
{
    sai_uint64_t vlan_oid_acc = sai_get64bit_oid(vlan_member_id,
                                                 SAI_OBJECT_TYPE_VLAN_MEMBER);
    sai_status_t ret;
    ret = sai_remove_vlan_member(vlan_oid_acc);
    return ret;
}

sai_status_t sai_remove_vlan_wrp(
    IN sai_uint32_t vlan_id)
{
    sai_uint64_t vlan_oid_acc = sai_get64bit_oid(vlan_id, SAI_OBJECT_TYPE_VLAN);
    sai_status_t ret;
    ret = sai_remove_vlan(vlan_oid_acc);
    return ret;


}
