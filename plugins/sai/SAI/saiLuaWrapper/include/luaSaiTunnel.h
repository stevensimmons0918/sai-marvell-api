#ifndef _luaSaiAcl_H
#define _luaSaiAcl_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN

/**
 * @brief Create tunnel default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t underlay_interface
 * @param[in] sai_tunnel_decap_ecn_mode_t decap_ecn_mode
 * @param[in] sai_object_list_t *decap_mappers_ptr
 * @param[in] sai_ip_address_t *addr_encap_src_ip
 * @param[in] sai_uint32_t overlay_interface
 * @param[in] sai_tunnel_peer_mode_t peer_mode
 * @param[in] sai_ip_address_t *addr_encap_dst_ip
 * @param[in] sai_tunnel_encap_ecn_mode_t encap_ecn_mode
 * @param[in] uint8_t encap_gre_key_valid
 * @param[in] sai_tunnel_type_t type
 * @param[in] sai_object_list_t *encap_mappers_ptr
 * @return Object_ID
 */
sai_uint32_t sai_create_tunnel_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t underlay_interface,
    IN sai_tunnel_decap_ecn_mode_t decap_ecn_mode,
    IN sai_object_list_t *decap_mappers_ptr,
    IN sai_ip_address_t *addr_encap_src_ip,
    IN sai_uint32_t overlay_interface,
    IN sai_tunnel_peer_mode_t peer_mode,
    IN sai_ip_address_t *addr_encap_dst_ip,
    IN sai_tunnel_encap_ecn_mode_t encap_ecn_mode,
    IN uint8_t encap_gre_key_valid,
    IN sai_tunnel_type_t type,
    IN sai_object_list_t *encap_mappers_ptr);

/**
 * @brief Create tunnel custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t underlay_interface
 * @param[in] sai_tunnel_decap_ecn_mode_t decap_ecn_mode
 * @param[in] sai_object_list_t *decap_mappers_ptr
 * @param[in] sai_uint8_t encap_dscp_val
 * @param[in] sai_ip_address_t *addr_encap_src_ip
 * @param[in] sai_tunnel_ttl_mode_t decap_ttl_mode
 * @param[in] sai_tunnel_dscp_mode_t decap_dscp_mode
 * @param[in] sai_uint8_t encap_ttl_val
 * @param[in] sai_tunnel_vxlan_udp_sport_mode_t vxlan_udp_sport_mode
 * @param[in] sai_uint16_t vxlan_udp_sport
 * @param[in] sai_packet_action_t loopback_packet_action
 * @param[in] sai_tunnel_ttl_mode_t encap_ttl_mode
 * @param[in] sai_uint32_t overlay_interface
 * @param[in] sai_tunnel_peer_mode_t peer_mode
 * @param[in] sai_ip_address_t *addr_encap_dst_ip
 * @param[in] sai_tunnel_encap_ecn_mode_t encap_ecn_mode
 * @param[in] uint8_t encap_gre_key_valid
 * @param[in] sai_tunnel_dscp_mode_t encap_dscp_mode
 * @param[in] sai_uint32_t encap_gre_key
 * @param[in] sai_tunnel_type_t type
 * @param[in] sai_object_list_t *encap_mappers_ptr
 * @return Object_ID
 */
sai_uint32_t sai_create_tunnel_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t underlay_interface,
    IN sai_tunnel_decap_ecn_mode_t decap_ecn_mode,
    IN sai_object_list_t *decap_mappers_ptr,
    IN sai_uint8_t encap_dscp_val,
    IN sai_ip_address_t *addr_encap_src_ip,
    IN sai_tunnel_ttl_mode_t decap_ttl_mode,
    IN sai_tunnel_dscp_mode_t decap_dscp_mode,
    IN sai_uint8_t encap_ttl_val,
    IN sai_tunnel_vxlan_udp_sport_mode_t vxlan_udp_sport_mode,
    IN sai_uint16_t vxlan_udp_sport,
    IN sai_packet_action_t loopback_packet_action,
    IN sai_tunnel_ttl_mode_t encap_ttl_mode,
    IN sai_uint32_t overlay_interface,
    IN sai_tunnel_peer_mode_t peer_mode,
    IN sai_ip_address_t *addr_encap_dst_ip,
    IN sai_tunnel_encap_ecn_mode_t encap_ecn_mode,
    IN uint8_t encap_gre_key_valid,
    IN sai_tunnel_dscp_mode_t encap_dscp_mode,
    IN sai_uint32_t encap_gre_key,
    IN sai_tunnel_type_t type,
    IN sai_object_list_t *encap_mappers_ptr);

/**
 * @brief Set tunnel attributes
 *
 * @param[in] sai_uint32_t tunnel_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_tunnel_attribute_wrp(
    IN sai_uint32_t tunnel_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get tunnel attributes
 *
 * @param[in] sai_uint32_t tunnel_id
 * @param[in] sai_tunnel_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_tunnel_attribute_wrp(
    IN sai_uint32_t tunnel_id,
    IN sai_tunnel_attr_t attr_id,
    IN uint32_t list_count,
    IN sai_attribute_t *out_attr);

/**
 * @brief Remove Tunnel
 *
 * @param[in] sai_uint32_t tunnel_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_tunnel_wrp(
    IN sai_uint32_t tunnel_id);

/**
 * @brief Get tunnel stats
 *
 * @param[in] sai_uint32_t tunnel_id
 * @param[in] uint32_t number_of_counters
 * @param[in] const sai_stat_id_t *counter_ids
 * @param[in] uint32_t *counters
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_tunnel_stats_wrp(
    IN sai_uint32_t tunnel_id,
    IN uint32_t number_of_counters,
    IN const sai_stat_id_t *counter_ids,
    IN uint32_t *counters);

/**
 * @brief Clear tunnel stats
 *
 * @param[in] sai_uint32_t tunnel_id
 * @param[in] uint32_t number_of_counters
 * @param[in] const sai_stat_id_t *counter_ids
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_clear_tunnel_stats_wrp(
    IN sai_uint32_t tunnel_id,
    IN uint32_t number_of_counters,
    IN const sai_stat_id_t *counter_ids);

/**
 * @brief Create tunnel map entry default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_tunnel_map_type_t tunnel_map_type
 * @param[in] sai_uint32_t virtual_router_id_value
 * @param[in] sai_uint32_t bridge_id_key
 * @param[in] sai_uint8_t uecn_value
 * @param[in] sai_uint32_t tunnel_map
 * @param[in] sai_uint32_t bridge_id_value
 * @param[in] sai_uint32_t vni_id_value
 * @param[in] sai_uint8_t oecn_key
 * @param[in] sai_uint8_t uecn_key
 * @param[in] sai_uint16_t vlan_id_value
 * @param[in] sai_uint16_t vlan_id_key
 * @param[in] sai_uint8_t oecn_value
 * @param[in] sai_uint32_t vni_id_key
 * @param[in] sai_uint32_t virtual_router_id_key
 * @return Object_ID
 */
sai_uint32_t sai_create_tunnel_map_entry_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_tunnel_map_type_t tunnel_map_type,
    IN sai_uint32_t virtual_router_id_value,
    IN sai_uint32_t bridge_id_key,
    IN sai_uint8_t uecn_value,
    IN sai_uint32_t tunnel_map,
    IN sai_uint32_t bridge_id_value,
    IN sai_uint32_t vni_id_value,
    IN sai_uint8_t oecn_key,
    IN sai_uint8_t uecn_key,
    IN sai_uint16_t vlan_id_value,
    IN sai_uint16_t vlan_id_key,
    IN sai_uint8_t oecn_value,
    IN sai_uint32_t vni_id_key,
    IN sai_uint32_t virtual_router_id_key);

/**
 * @brief Create tunnel map entry custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_tunnel_map_type_t tunnel_map_type
 * @param[in] sai_uint32_t virtual_router_id_value
 * @param[in] sai_uint32_t bridge_id_key
 * @param[in] sai_uint8_t uecn_value
 * @param[in] sai_uint32_t tunnel_map
 * @param[in] sai_uint32_t bridge_id_value
 * @param[in] sai_uint32_t vni_id_value
 * @param[in] sai_uint8_t oecn_key
 * @param[in] sai_uint8_t uecn_key
 * @param[in] sai_uint16_t vlan_id_value
 * @param[in] sai_uint16_t vlan_id_key
 * @param[in] sai_uint8_t oecn_value
 * @param[in] sai_uint32_t vni_id_key
 * @param[in] sai_uint32_t virtual_router_id_key
 * @return Object_ID
 */
sai_uint32_t sai_create_tunnel_map_entry_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_tunnel_map_type_t tunnel_map_type,
    IN sai_uint32_t virtual_router_id_value,
    IN sai_uint32_t bridge_id_key,
    IN sai_uint8_t uecn_value,
    IN sai_uint32_t tunnel_map,
    IN sai_uint32_t bridge_id_value,
    IN sai_uint32_t vni_id_value,
    IN sai_uint8_t oecn_key,
    IN sai_uint8_t uecn_key,
    IN sai_uint16_t vlan_id_value,
    IN sai_uint16_t vlan_id_key,
    IN sai_uint8_t oecn_value,
    IN sai_uint32_t vni_id_key,
    IN sai_uint32_t virtual_router_id_key);

/**
 * @brief Set tunnel map entry attributes
 *
 * @param[in] sai_uint32_t tunnel_map_entry_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_tunnel_map_entry_attribute_wrp(
    IN sai_uint32_t tunnel_map_entry_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get tunnel map entry attributes
 *
 * @param[in] sai_uint32_t tunnel_map_entry_id
 * @param[in] sai_tunnel_map_entry_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_tunnel_map_entry_attribute_wrp(
    IN sai_uint32_t tunnel_map_entry_id,
    IN sai_tunnel_map_entry_attr_t attr_id,
    IN uint32_t list_count,
    IN sai_attribute_t *out_attr);

/**
 * @brief Remove tunnel map entry
 *
 * @param[in] sai_uint32_t tunnel_map_entry_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_tunnel_map_entry_wrp(
    IN sai_uint32_t tunnel_map_entry_id);

/**
 * @brief Create tunnel term table entry default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_ip_address_t *addr_src_ip
 * @param[in] sai_ip_address_t *addr_dst_ip_mask
 * @param[in] sai_uint32_t action_tunnel_id
 * @param[in] sai_tunnel_type_t tunnel_type
 * @param[in] sai_ip_address_t *addr_dst_ip
 * @param[in] sai_tunnel_term_table_entry_type_t type
 * @param[in] sai_uint32_t vr_id
 * @param[in] sai_ip_address_t *addr_src_ip_mask
 * @return Object_ID
 */
sai_uint32_t sai_create_tunnel_term_table_entry_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_ip_address_t *addr_src_ip,
    IN sai_ip_address_t *addr_dst_ip_mask,
    IN sai_uint32_t action_tunnel_id,
    IN sai_tunnel_type_t tunnel_type,
    IN sai_ip_address_t *addr_dst_ip,
    IN sai_tunnel_term_table_entry_type_t type,
    IN sai_uint32_t vr_id,
    IN sai_ip_address_t *addr_src_ip_mask);

/**
 * @brief Create tunnel term table entry custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_ip_address_t *addr_src_ip
 * @param[in] sai_ip_address_t *addr_dst_ip_mask
 * @param[in] sai_uint32_t action_tunnel_id
 * @param[in] sai_tunnel_type_t tunnel_type
 * @param[in] sai_ip_address_t *addr_dst_ip
 * @param[in] sai_tunnel_term_table_entry_type_t type
 * @param[in] sai_uint32_t vr_id
 * @param[in] sai_ip_address_t *addr_src_ip_mask

 * @return Object_ID
 */
sai_uint32_t sai_create_tunnel_term_table_entry_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_ip_address_t *addr_src_ip,
    IN sai_ip_address_t *addr_dst_ip_mask,
    IN sai_uint32_t action_tunnel_id,
    IN sai_tunnel_type_t tunnel_type,
    IN sai_ip_address_t *addr_dst_ip,
    IN sai_tunnel_term_table_entry_type_t type,
    IN sai_uint32_t vr_id,
    IN sai_ip_address_t *addr_src_ip_mask);

/**
 * @brief Set tunnel term table entry
 *
 * @param[in] sai_uint32_t tunnel_term_table_entry_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_tunnel_term_table_entry_attribute_wrp(
    IN sai_uint32_t tunnel_term_table_entry_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get tunnel term table entry
 *
 * @param[in] sai_uint32_t tunnel_term_table_entry_id
 * @param[in] sai_tunnel_term_table_entry_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_tunnel_term_table_entry_attribute_wrp(
    IN sai_uint32_t tunnel_term_table_entry_id,
    IN sai_tunnel_term_table_entry_attr_t attr_id,
    IN uint32_t list_count,
    IN sai_attribute_t *out_attr);

/**
 * @brief Remove tunnel term table entry
 *
 * @param[in] sai_uint32_t tunnel_term_table_entry_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_tunnel_term_table_entry_wrp(
    IN sai_uint32_t tunnel_term_table_entry_id);

/**
 * @brief Create tunnel map default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_tunnel_map_type_t type
 * @return Object_ID
 */
sai_uint32_t sai_create_tunnel_map_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_tunnel_map_type_t type);

/**
 * @brief Create tunnel map custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_tunnel_map_type_t type
 * @return Object_ID
 */
sai_uint32_t sai_create_tunnel_map_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_tunnel_map_type_t type);

/**
 * @brief Set tunnel map attributes
 *
 * @param[in] sai_uint32_t tunnel_map_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_tunnel_map_attribute_wrp(
    IN sai_uint32_t tunnel_map_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get tunnel map attributes
 *
 * @param[in] sai_uint32_t tunnel_map_id
 * @param[in] sai_tunnel_map_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_tunnel_map_attribute_wrp(
    IN sai_uint32_t tunnel_map_id,
    IN sai_tunnel_map_attr_t attr_id,
    IN uint32_t list_count,
    IN sai_attribute_t *out_attr);

/**
 * @brief Remove tunnel map entry
 *
 * @param[in] sai_uint32_t tunnel_map_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_tunnel_map_wrp(
    IN sai_uint32_t tunnel_map_id);

#ifdef __cplusplus
}
#endif

#endif /* _luaSaiAcl_H */
