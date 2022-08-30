#ifndef _luaSaiVlan_H
#define _luaSaiVlan_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN

#undef OUT
#define OUT

/**
 * @brief Create vlan member default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t vlan_id
 * @param[in] sai_uint32_t bridge_port_id
 * @return Object_ID
 */
sai_uint32_t sai_create_vlan_member_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t vlan_id,
    IN sai_uint32_t bridge_port_id);

/**
 * @brief Create vlan member custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t vlan_id
 * @param[in] sai_uint32_t bridge_port_id
 * @param[in] sai_vlan_tagging_mode_t vlan_tagging_mode
 * @return Object_ID
 */
sai_uint32_t sai_create_vlan_member_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t vlan_id,
    IN sai_uint32_t bridge_port_id,
    IN sai_vlan_tagging_mode_t vlan_tagging_mode);

/**
 * @brief Set vlan member attributes
 *
 * @param[in] sai_uint32_t vlan_member_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_vlan_member_attribute_wrp(
    IN sai_uint32_t vlan_member_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get vlan member attributes
 *
 * @param[in] sai_uint32_t vlan_member_id
 * @param[in] sai_vlan_member_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_vlan_member_attribute_wrp(
    IN sai_uint32_t vlan_member_id,
    IN sai_vlan_member_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief Remove vlan member
 *
 * @param[in] sai_uint32_t vlan_member_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_vlan_member_wrp(
    IN sai_uint32_t vlan_member_id);

/**
 * @brief Create vlan default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint16_t vlan_id
 * @return Object_ID
 */
sai_uint32_t sai_create_vlan_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint16_t vlan_id);

/**
 * @brief Create vlan custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t max_learned_addresses
 * @param[in] sai_uint32_t unknown_unicast_flood_group
 * @param[in] sai_uint32_t ingress_acl
 * @param[in] sai_vlan_flood_control_type_t unknown_unicast_flood_control_type
 * @param[in] sai_uint32_t stp_instance
 * @param[in] sai_vlan_mcast_lookup_key_type_t ipv4_mcast_lookup_key_type
 * @param[in] uint8_t custom_igmp_snooping_enable
 * @param[in] sai_uint32_t egress_acl
 * @param[in] uint8_t learn_disable
 * @param[in] sai_uint16_t vlan_id
 * @param[in] sai_vlan_flood_control_type_t unknown_multicast_flood_control_type
 * @param[in] sai_uint32_t unknown_multicast_flood_group
 * @param[in] sai_vlan_mcast_lookup_key_type_t ipv6_mcast_lookup_key_type
 * @param[in] sai_uint32_t broadcast_flood_group
 * @param[in] sai_vlan_flood_control_type_t broadcast_flood_control_type
 * @return Object_ID
 */
sai_uint32_t sai_create_vlan_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t max_learned_addresses,
    IN sai_uint32_t unknown_unicast_flood_group,
    IN sai_uint32_t ingress_acl,
    IN sai_vlan_flood_control_type_t unknown_unicast_flood_control_type,
    IN sai_uint32_t stp_instance,
    IN sai_vlan_mcast_lookup_key_type_t ipv4_mcast_lookup_key_type,
    IN uint8_t custom_igmp_snooping_enable,
    IN sai_uint32_t egress_acl,
    IN uint8_t learn_disable,
    IN sai_uint16_t vlan_id,
    IN sai_vlan_flood_control_type_t unknown_multicast_flood_control_type,
    IN sai_uint32_t unknown_multicast_flood_group,
    IN sai_vlan_mcast_lookup_key_type_t ipv6_mcast_lookup_key_type,
    IN sai_uint32_t broadcast_flood_group,
    IN sai_vlan_flood_control_type_t broadcast_flood_control_type);

/**
 * @brief Set vlan attributes
 *
 * @param[in] sai_uint32_t vlan_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_vlan_attribute_wrp(
    IN sai_uint32_t vlan_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get vlan attributes
 *
 * @param[in] sai_uint32_t vlan_id
 * @param[in] sai_vlan_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_vlan_attribute_wrp(
    IN sai_uint32_t vlan_id,
    IN sai_vlan_attr_t attr_id,
    IN uint32_t list_count,
    IN sai_attribute_t *out_attr);

/**
 * @brief Remove vlan
 *
 * @param[in] sai_uint32_t vlan_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_vlan_wrp(
    IN sai_uint32_t vlan_id);

/**
 * @brief Get vlan stats
 *
 * @param[in] sai_uint32_t vlan_id
 * @param[in] uint32_t number_of_counters
 * @param[in] const sai_stat_id_t *counter_ids
 * @param[in] uint64_t *counters
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_vlan_stats_wrp(
    IN sai_uint32_t vlan_id,
    IN uint32_t number_of_counters,
    IN const sai_stat_id_t *counter_ids,
    IN uint64_t *counters);

/**
 * @brief Clear vlan stats
 *
 * @param[in] sai_uint32_t vlan_id
 * @param[in] uint32_t number_of_counters
 * @param[in] const sai_stat_id_t *counter_ids
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_clear_vlan_stats_wrp(
    IN sai_uint32_t vlan_id,
    IN uint32_t number_of_counters,
    IN const sai_stat_id_t *counter_ids);

#ifdef __cplusplus
}
#endif

#endif /* _luaSaiVlan_H */
