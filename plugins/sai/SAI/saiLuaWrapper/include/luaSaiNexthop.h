#ifndef _luaSaiNexthop_H
#define _luaSaiNexthop_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN
#undef OUT
#define OUT

/**
 * @brief Create Nexthop default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t router_interface_id
 * @param[in] sai_next_hop_type_t type,
 * @param[in] sai_ip_address_t *addr_ip
 * @return Object_ID
 */

sai_uint32_t sai_create_next_hop_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t router_interface_id,
    IN sai_next_hop_type_t type,
    IN sai_ip_address_t *addr_ip);
/**
 * @brief Create Nexthop custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint8_t outseg_ttl_value,
 * @param[in] sai_outseg_exp_mode_t outseg_exp_mode
 * @param[in] sai_uint32_t counter_id
 * @param[in] sai_uint32_t router_interface_id
 * @param[in] sai_uint32_t qos_tc_and_color_to_mpls_exp_map
 * @param[in] sai_next_hop_type_t type
 * @param[in] uint8_t disable_decrement_ttl
 * @param[in] sai_uint32_t tunnel_vni
 * @param[in] sai_ip_address_t *addr_ip
 * @param[in] sai_outseg_ttl_mode_t outseg_ttl_mode
 * @param[in] shell_mac *tunnel_mac
 * @param[in] sai_outseg_type_t outseg_type
 * @param[in] sai_uint8_t outseg_exp_value
 * @return Object_ID
 */
sai_uint32_t sai_create_next_hop_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint8_t outseg_ttl_value,
    IN sai_outseg_exp_mode_t outseg_exp_mode,
    IN sai_uint32_t counter_id,
    IN sai_uint32_t router_interface_id,
    IN sai_uint32_t qos_tc_and_color_to_mpls_exp_map,
    IN sai_next_hop_type_t type,
    IN uint8_t disable_decrement_ttl,
    IN sai_uint32_t tunnel_vni,
    IN sai_ip_address_t *addr_ip,
    IN sai_outseg_ttl_mode_t outseg_ttl_mode,
    IN shell_mac *tunnel_mac,
    IN sai_outseg_type_t outseg_type,
    IN sai_uint8_t outseg_exp_value);
/**
 * @brief Set Nexthop attributes
 *
 * @param[in] sai_uint32_t next_hop_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */

sai_status_t sai_set_next_hop_attribute_wrp(
    IN sai_uint32_t next_hop_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);
/**
 * @brief Get Nexthop attributes
 *
 * @param[in] sai_uint32_t next_hop_id
 * @param[in] sai_next_hop_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */

sai_status_t sai_get_next_hop_attribute_wrp(
    IN sai_uint32_t next_hop_id,
    IN sai_next_hop_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);
/**
 * @brief Remove Nexthop
 *
 * @param[in] sai_uint32_t next_hop_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */

sai_status_t sai_remove_next_hop_wrp(
    IN sai_uint32_t next_hop_id);


#ifdef __cplusplus
}
#endif



#endif


