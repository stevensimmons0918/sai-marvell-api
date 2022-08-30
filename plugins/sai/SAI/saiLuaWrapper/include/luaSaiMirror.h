#ifndef _luaSaiMirror_H
#define _luaSaiMirror_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN

#undef OUT
#define OUT

/**
 * @brief Create mirror session default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_ip_address_t *addr_src_ip_address
 * @param[in] sai_ip_address_t *addr_dst_ip_address
 * @param[in] uint8_t monitor_portlist_valid
 * @param[in] sai_erspan_encapsulation_type_t erspan_encapsulation_type
 * @param[in] sai_uint16_t udp_src_port
 * @param[in] sai_mirror_session_type_t type
 * @param[in] sai_uint32_t monitor_port
 * @param[in] shell_mac *dst_mac_address
 * @param[in] shell_mac *src_mac_address
 * @param[in] sai_uint16_t gre_protocol_type
 * @param[in] sai_uint8_t iphdr_version
 * @param[in] sai_uint8_t tos
 * @param[in] sai_uint16_t udp_dst_port)
 * @return Object_ID
 */
sai_uint32_t sai_create_mirror_session_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_ip_address_t *addr_src_ip_address,
    IN sai_ip_address_t *addr_dst_ip_address,
    IN uint8_t monitor_portlist_valid,
    IN sai_erspan_encapsulation_type_t erspan_encapsulation_type,
    IN sai_uint16_t udp_src_port,
    IN sai_mirror_session_type_t type,
    IN sai_uint32_t monitor_port,
    IN shell_mac *dst_mac_address,
    IN shell_mac *src_mac_address,
    IN sai_uint16_t gre_protocol_type,
    IN sai_uint8_t iphdr_version,
    IN sai_uint8_t tos,
    IN sai_uint16_t udp_dst_port);

/**
 * @brief Create mirror session custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_ip_address_t *addr_src_ip_address
 * @param[in] sai_ip_address_t *addr_dst_ip_address
 * @param[in] uint8_t monitor_portlist_valid
 * @param[in] sai_uint16_t truncate_size
 * @param[in] sai_erspan_encapsulation_type_t erspan_encapsulation_type
 * @param[in] sai_uint16_t udp_src_port
 * @param[in] uint8_t vlan_header_valid
 * @param[in] sai_mirror_session_type_t type
 * @param[in] sai_uint32_t monitor_port
 * @param[in] sai_uint32_t sample_rate
 * @param[in] sai_uint8_t ttl
 * @param[in] shell_mac *dst_mac_address
 * @param[in] shell_mac *src_mac_address
 * @param[in] sai_uint16_t gre_protocol_type
 * @param[in] sai_uint8_t tc
 * @param[in] sai_uint16_t vlan_id
 * @param[in] sai_uint8_t iphdr_version
 * @param[in] sai_uint8_t tos
 * @param[in] sai_uint16_t udp_dst_port
 * @param[in] sai_uint32_t policer
 * @return Object_ID
 */
sai_uint32_t sai_create_mirror_session_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_ip_address_t *addr_src_ip_address,
    IN sai_ip_address_t *addr_dst_ip_address,
    IN uint8_t monitor_portlist_valid,
    IN sai_uint16_t truncate_size,
    IN sai_erspan_encapsulation_type_t erspan_encapsulation_type,
    IN sai_uint16_t udp_src_port,
    IN uint8_t vlan_header_valid,
    IN sai_mirror_session_type_t type,
    IN sai_uint32_t monitor_port,
    IN sai_uint32_t sample_rate,
    IN sai_uint8_t ttl,
    IN shell_mac *dst_mac_address,
    IN shell_mac *src_mac_address,
    IN sai_uint16_t gre_protocol_type,
    IN sai_uint8_t tc,
    IN sai_uint16_t vlan_id,
    IN sai_uint8_t iphdr_version,
    IN sai_uint8_t tos,
    IN sai_uint16_t udp_dst_port,
    IN sai_uint32_t policer);

/**
 * @brief Set mirror session attributes
 *
 * @param[in] sai_uint32_t mirror_session_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_mirror_session_attribute_wrp(
    IN sai_uint32_t mirror_session_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get mirror session attributes
 *
 * @param[in] sai_uint32_t mirror_session_id
 * @param[in] sai_mirror_session_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_mirror_session_attribute_wrp(
    IN sai_uint32_t mirror_session_id,
    IN sai_mirror_session_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief Remove mirror session
 *
 * @param[in] sai_uint32_t mirror_session_id);
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_mirror_session_wrp(
    IN sai_uint32_t mirror_session_id);

#ifdef __cplusplus
}
#endif

#endif /* _luaSaiMirror_H */
