#ifndef _luaSaiBridge_H
#define _luaSaiBridge_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN
#undef OUT
#define OUT

/**
 * @brief Create bridge port default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t rif_id
 * @param[in] sai_uint32_t port_id
 * @param[in] sai_uint32_t bridge_id
 * @param[in] sai_bridge_port_type_t type
 * @param[in] sai_uint32_t tunnel_id
 * @param[in] sai_uint16_t vlan_id
 * @return Object_ID
 */
sai_uint32_t sai_create_bridge_port_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t rif_id,
    IN sai_uint32_t port_id,
    IN sai_uint32_t bridge_id,
    IN sai_bridge_port_type_t type,
    IN sai_uint32_t tunnel_id,
    IN sai_uint16_t vlan_id);

/**
 * @brief Create bridge port custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t rif_id
 * @param[in] sai_uint32_t port_id
 * @param[in] uint8_t admin_state
 * @param[in] sai_bridge_port_tagging_mode_t tagging_mode
 * @param[in] sai_uint32_t bridge_id
 * @param[in] sai_bridge_port_type_t type
 * @param[in] sai_uint32_t max_learned_addresses
 * @param[in] sai_uint32_t tunnel_id
 * @param[in] sai_bridge_port_fdb_learning_mode_t fdb_learning_mode
 * @param[in] sai_packet_action_t fdb_learning_limit_violation_packet_action
 * @param[in] uint8_t egress_filtering
 * @param[in] uint8_t ingress_filtering
 * @param[in] sai_uint16_t vlan_id
 * @return Object_ID
 */
sai_uint32_t sai_create_bridge_port_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t rif_id,
    IN sai_uint32_t port_id,
    IN uint8_t admin_state,
    IN sai_bridge_port_tagging_mode_t tagging_mode,
    IN sai_uint32_t bridge_id,
    IN sai_bridge_port_type_t type,
    IN sai_uint32_t max_learned_addresses,
    IN sai_uint32_t tunnel_id,
    IN sai_bridge_port_fdb_learning_mode_t fdb_learning_mode,
    IN sai_packet_action_t fdb_learning_limit_violation_packet_action,
    IN uint8_t egress_filtering,
    IN uint8_t ingress_filtering,
    IN sai_uint16_t vlan_id);

/**
 * @brief Set bridge port attributes
 *
 * @param[in] sai_uint32_t bridge_port_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_bridge_port_attribute_wrp(
    IN sai_uint32_t bridge_port_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get bridge port attributes
 *
 * @param[in] sai_uint32_t bridge_port_id
 * @param[in] sai_bridge_port_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_bridge_port_attribute_wrp(
    IN sai_uint32_t bridge_port_id,
    IN sai_bridge_port_attr_t attr_id,
    IN uint32_t list_count,
    IN sai_attribute_t *out_attr);

/**
 * @brief Remove bridge port
 *
 * @param[in] sai_uint32_t bridge_port_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_bridge_port_wrp(
    IN sai_uint32_t bridge_port_id);

/**
 * @brief Get bridge port stats
 *
 * @param[in] sai_uint32_t bridge_port_id
 * @param[in] uint32_t number_of_counters
 * @param[in] const sai_stat_id_t *counter_ids
 * @param[in] uint_32_t *counters
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_bridge_port_stats_wrp(
    IN sai_uint32_t bridge_port_id,
    IN uint32_t number_of_counters,
    IN const sai_stat_id_t *counter_ids,
    IN uint32_t *counters);

/**
 * @brief Clear bridge port stats
 *
 * @param[in] sai_uint32_t bridge_port_id
 * @param[in] uint32_t number_of_counters
 * @param[in] const sai_stat_id_t *counter_ids
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_clear_bridge_port_stats_wrp(
    IN sai_uint32_t bridge_port_id,
    IN uint32_t number_of_counters,
    IN const sai_stat_id_t *counter_ids);

/**
 * @brief Create bridge port default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_bridge_type_t type
 * @return Object_ID
 */
sai_uint32_t sai_create_bridge_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_bridge_type_t type);

/**
 * @brief Create bridge port custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_bridge_type_t type
 * @param[in] sai_uint32_t max_learned_addresses
 * @param[in] sai_bridge_flood_control_type_t unknown_multicast_flood_control_type
 * @param[in] uint8_t learn_disable
 * @param[in] sai_bridge_flood_control_type_t broadcast_flood_control_type
 * @param[in] sai_uint32_t unknown_unicast_flood_group
 * @param[in] sai_uint32_t unknown_multicast_flood_group
 * @param[in] sai_bridge_flood_control_type_t unknown_unicast_flood_control_type
 * @param[in] sai_uint32_t broadcast_flood_group
 * @return Object_ID
 */
sai_uint32_t sai_create_bridge_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_bridge_type_t type,
    IN sai_uint32_t max_learned_addresses,
    IN sai_bridge_flood_control_type_t unknown_multicast_flood_control_type,
    IN uint8_t learn_disable,
    IN sai_bridge_flood_control_type_t broadcast_flood_control_type,
    IN sai_uint32_t unknown_unicast_flood_group,
    IN sai_uint32_t unknown_multicast_flood_group,
    IN sai_bridge_flood_control_type_t unknown_unicast_flood_control_type,
    IN sai_uint32_t broadcast_flood_group);

/**
 * @brief Set bridge port attributes
 *
 * @param[in] sai_uint32_t bridge_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_bridge_attribute_wrp(
    IN sai_uint32_t bridge_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get bridge port attributes
 *
 * @param[in] sai_uint32_t bridge_id
 * @param[in] sai_bridge_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_bridge_attribute_wrp(
    IN sai_uint32_t bridge_id,
    IN sai_bridge_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief Remove bridge port
 *
 * @param[in] sai_uint32_t bridge_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_bridge_wrp(
    IN sai_uint32_t bridge_id);

/**
 * @brief Remove bridge port stats
 *
 * @param[in] sai_uint32_t bridge_id
 * @param[in] uint32_t number_of_counters
 * @param[in] const sai_stat_id_t *counter_ids
 * @param[in] uint32_t *counters
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_bridge_stats_wrp(
    IN sai_uint32_t bridge_id,
    IN uint32_t number_of_counters,
    IN const sai_stat_id_t *counter_ids,
    IN uint32_t *counters);

/**
 * @brief Clear bridge port stats
 *
 * @param[in] sai_uint32_t bridge_id
 * @param[in] uint32_t number_of_counters
 * @param[in] const sai_stat_id_t *counter_ids
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_clear_bridge_stats_wrp(
    IN sai_uint32_t bridge_id,
    IN uint32_t number_of_counters,
    IN const sai_stat_id_t *counter_ids);

#ifdef __cplusplus
}
#endif

#endif /* _luaSaiBridge_H */
