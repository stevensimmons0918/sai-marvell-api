#ifndef _luaSaiRouterinterfac_H
#define _luaSaiRouterinterfac_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN

/**
 * @brief Create routerinterface default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t vlan_id
 * @param[in] sai_router_interface_type_t type
 * @param[in] sai_uint32_t port_id
 * @param[in] sai_uint32_t virtual_router_id
 * @param[in] uint8_t is_virtual
 * @return Object_ID
 */
sai_uint32_t sai_create_router_interface_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t vlan_id,
    IN sai_router_interface_type_t type,
    IN sai_uint32_t port_id,
    IN sai_uint32_t virtual_router_id,
    IN uint8_t is_virtual);

/**
 * @brief Create routerinterface custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] shell_mac *src_mac_address
 * @param[in] uint8_t admin_v4_state
 * @param[in] uint8_t v4_mcast_enable
 * @param[in] uint8_t admin_mpls_state
 * @param[in] sai_uint32_t vlan_id
 * @param[in] sai_uint32_t ingress_acl
 * @param[in] sai_router_interface_type_t type
 * @param[in] sai_uint8_t nat_zone_id
 * @param[in] uint8_t disable_decrement_ttl
 * @param[in] uint8_t admin_v6_state
 * @param[in] sai_uint32_t egress_acl
 * @param[in] sai_packet_action_t neighbor_miss_packet_action
 * @param[in] sai_uint32_t mtu
 * @param[in] uint8_t v6_mcast_enable
 * @param[in] sai_packet_action_t loopback_packet_action
 * @param[in] sai_uint32_t port_id
 * @param[in] sai_uint32_t virtual_router_id
 * @param[in] uint8_t is_virtual
 * @return Object_ID
 */
sai_uint32_t sai_create_router_interface_custom_wrp(
    IN sai_uint32_t switch_id,
    IN shell_mac *src_mac_address,
    IN uint8_t admin_v4_state,
    IN uint8_t v4_mcast_enable,
    IN uint8_t admin_mpls_state,
    IN sai_uint32_t vlan_id,
    IN sai_uint32_t ingress_acl,
    IN sai_router_interface_type_t type,
    IN sai_uint8_t nat_zone_id,
    IN uint8_t disable_decrement_ttl,
    IN uint8_t admin_v6_state,
    IN sai_uint32_t egress_acl,
    IN sai_packet_action_t neighbor_miss_packet_action,
    IN sai_uint32_t mtu,
    IN uint8_t v6_mcast_enable,
    IN sai_packet_action_t loopback_packet_action,
    IN sai_uint32_t port_id,
    IN sai_uint32_t virtual_router_id,
    IN uint8_t is_virtual);

/**
 * @brief Set routerinterface attributes
 *
 * @param[in] sai_uint32_t router_interface_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_router_interface_attribute_wrp(
    IN sai_uint32_t router_interface_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get routerinterface attributes
 *
 * @param[in] sai_uint32_t router_interface_id
 * @param[in] sai_router_interface_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_router_interface_attribute_wrp(
    IN sai_uint32_t router_interface_id,
    IN sai_router_interface_attr_t attr_id,
    IN uint32_t list_count,
    IN sai_attribute_t *out_attr);

/**
 * @brief Remove routerinterface
 *
 * @param[in] sai_uint32_t router_interface_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_router_interface_wrp(
    IN sai_uint32_t router_interface_id);

/**
 * @brief Get routerinterface stats
 *
 * @param[in] sai_uint32_t router_interface_id
 * @param[in] uint32_t number_of_counters
 * @param[in] const sai_stat_id_t *counter_ids
 * @param[in] uint32_t *counters
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_router_interface_stats_wrp(
    IN sai_uint32_t router_interface_id,
    IN uint32_t number_of_counters,
    IN const sai_stat_id_t *counter_ids,
    IN uint32_t *counters);
/**
 * @brief Clear routerinterface stats
 *
 * @param[in] sai_uint32_t router_interface_id
 * @param[in] uint32_t number_of_counters
 * @param[in] const sai_stat_id_t *counter_ids
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_clear_router_interface_stats_wrp(
    IN sai_uint32_t router_interface_id,
    IN uint32_t number_of_counters,
    IN const sai_stat_id_t *counter_ids);

#ifdef __cplusplus
}
#endif

#endif /* _luaSaiRouterinterfac_H */
