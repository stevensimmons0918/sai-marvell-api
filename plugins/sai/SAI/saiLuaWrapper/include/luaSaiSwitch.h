#ifndef _luaSaiSwitch_H
#define _luaSaiSwitch_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN
#undef OUT
#define OUT

uint32_t luaSaiSwitchAttrGet();

/**
 * @brief Create Switch Default
 *
 * @param[in] sai_switch_type_t type
 * @param[in] sai_switch_firmware_load_method_t firmware_load_method
 * @param[in] uint8_t firmware_download_broadcast
 * @param[in] sai_uint32_t switch_profile_id
 * @param[in] sai_pointer_t register_read
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_switch_hardware_access_bus_t hardware_access_bus
 * @param[in] sai_pointer_t register_write
 * @param[in] sai_s8_list_t *switch_hardware_info_ptr
 * @param[in] sai_switch_firmware_load_type_t firmware_load_type
 * @param[in] sai_uint32_t max_system_cores
 * @param[in] sai_system_port_config_list_t *system_port_config_list_ptr
 * @param[in] uint8_t init_switch
 * @param[in] sai_uint64_t platfrom_context
 * @return Object_ID
 */
sai_uint32_t sai_create_switch_default_wrp(
    IN sai_switch_type_t type,
    IN sai_switch_firmware_load_method_t firmware_load_method,
    IN uint8_t firmware_download_broadcast,
    IN sai_uint32_t switch_profile_id,
    IN sai_pointer_t register_read,
    IN sai_uint32_t switch_id,
    IN sai_switch_hardware_access_bus_t hardware_access_bus,
    IN sai_pointer_t register_write,
    IN sai_s8_list_t *switch_hardware_info_ptr,
    IN sai_switch_firmware_load_type_t firmware_load_type,
    IN sai_uint32_t max_system_cores,
    IN sai_system_port_config_list_t *system_port_config_list_ptr,
    IN uint8_t init_switch,
    IN sai_uint64_t platfrom_context);

#if 0 /* TBD - check if this function is needed */
/**
 * @brief Create Switch custom
 *
 * @param[in] sai_uint32_t acl_table_group_id
 * @return Object_ID
 */

sai_uint32_t sai_create_switch_custom_wrp()
#endif /* if 0 */

/**
 * @brief Set Switch attributes
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr);
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_switch_attribute_wrp(
    IN sai_uint32_t switch_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get Switch attributes
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_switch_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return Object_ID
 */
sai_uint32_t sai_get_switch_attribute_wrp(
    IN sai_uint32_t switch_id,
    IN sai_switch_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief Remove Switch
 *
 * @param[in] sai_uint32_t switch_id
 * @return none
 */
void sai_remove_switch_wrp(
    IN sai_uint32_t switch_id);

/**
 * @brief Get swtich stats
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] uint32_t number_of_counters
 * @param[in] const sai_stat_id_t *counter_ids
 * @param[in] uint8_t counter_count
 * @param[in] uint32_t counter_list_ptr[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_switch_stats_wrp(
    IN sai_uint32_t switch_id,
    IN uint32_t number_of_counters,
    IN const sai_stat_id_t *counter_ids,
    IN uint8_t counter_count,
    OUT uint32_t counter_list_ptr[]);

/**
 * @brief Clear swtich stats
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] uint32_t number_of_counters
 * @param[in] const sai_stat_id_t *counter_ids
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_clear_switch_stats_wrp(
    IN sai_uint32_t switch_id,
    IN uint32_t number_of_counters,
    IN const sai_stat_id_t *counter_ids);

/**
 * @brief Create switch tunnel default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_object_list_t *encap_mappers_ptr
 * @param[in] sai_tunnel_decap_ecn_mode_t tunnel_decap_ecn_mode
 * @param[in] sai_object_list_t *decap_mappers_ptr
 * @param[in] sai_tunnel_type_t tunnel_type
 * @param[in] sai_tunnel_encap_ecn_mode_t tunnel_encap_ecn_mode
 * @return Object_ID
 */
sai_uint32_t sai_create_switch_tunnel_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_object_list_t *encap_mappers_ptr,
    IN sai_tunnel_decap_ecn_mode_t tunnel_decap_ecn_mode,
    IN sai_object_list_t *decap_mappers_ptr,
    IN sai_tunnel_type_t tunnel_type,
    IN sai_tunnel_encap_ecn_mode_t tunnel_encap_ecn_mode);

#if 0
/**
 * @brief Create switch tunnel custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_object_list_t *encap_mappers_ptr
 * @param[in] sai_packet_action_t loopback_packet_action
 * @param[in] sai_tunnel_decap_ecn_mode_t tunnel_decap_ecn_mode
 * @param[in] sai_uint16_t vxlan_udp_sport
 * @param[in] sai_object_list_t *decap_mappers_ptr
 * @param[in] sai_tunnel_vxlan_udp_sport_mode_t tunnel_vxlan_udp_sport_mode
 * @param[in] sai_tunnel_type_t tunnel_type
 * @param[in] sai_tunnel_encap_ecn_mode_t tunnel_encap_ecn_mode
 * @return Object_ID
 */
sai_uint32_t sai_create_switch_tunnel_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_object_list_t *encap_mappers_ptr,
    IN sai_packet_action_t loopback_packet_action,
    IN sai_tunnel_decap_ecn_mode_t tunnel_decap_ecn_mode,
    IN sai_uint16_t vxlan_udp_sport,
    IN sai_object_list_t *decap_mappers_ptr,
    IN sai_tunnel_vxlan_udp_sport_mode_t tunnel_vxlan_udp_sport_mode,
    IN sai_tunnel_type_t tunnel_type,
    IN sai_tunnel_encap_ecn_mode_t tunnel_encap_ecn_mode);
#endif
/**
 * @brief Set switch tunnel attributes
 *
 * @param[in] sai_uint32_t switch_tunnel_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_switch_tunnel_attribute_wrp(
    IN sai_uint32_t switch_tunnel_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get switch tunnel attributes
 *
 * @param[in] sai_uint32_t switch_tunnel_id
 * @param[in] sai_switch_tunnel_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return Object_ID
 */
#if 0
sai_uint32_t sai_get_switch_tunnel_attribute_wrp(
    IN sai_uint32_t switch_tunnel_id,
    IN sai_switch_tunnel_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);
#endif
/**
 * @brief Remove switch tunnel
 *
 * @param[in] sai_uint32_t switch_tunnel_id
 * @return none
 */
void sai_remove_switch_tunnel_wrp(
    IN sai_uint32_t switch_tunnel_id);

#ifdef __cplusplus
}
#endif

#endif /* _luaSaiSwitch_H */
