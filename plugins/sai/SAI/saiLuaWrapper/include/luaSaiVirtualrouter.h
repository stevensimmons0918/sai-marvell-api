#ifndef _luaSaiVirtualrouter_H
#define _luaSaiVirtualrouter_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN
#undef OUT
#define OUT


/**
 * @brief Create Virtual Router default
 *
 * @param[in] sai_uint32_t switch_id
 * @return Object_ID
 */

sai_uint32_t sai_create_virtual_router_default_wrp(IN sai_uint32_t switch_id);

/**
 * @brief Create Virtual Router custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t switch_id
 * @param[in] uint8_t admin_v4_state
 * @param[in] uint8_t admin_v6_state
 * @param[in] shell_string *label
 * @param[in] shell_mac *src_mac_address
 * @param[in] sai_packet_action_t violation_ip_options_packet_action
 * @param[in] sai_packet_action_t violation_ttl1_packet_action
 * @return Object_ID
 */

sai_uint32_t sai_create_virtual_router_custom_wrp(
    IN sai_uint32_t switch_id,
    IN uint8_t admin_v4_state,
    IN uint8_t admin_v6_state,
    IN shell_string *label,
    IN shell_mac *src_mac_address,
    IN sai_packet_action_t violation_ip_options_packet_action,
    IN sai_packet_action_t violation_ttl1_packet_action);
/**
 * @brief Set Virtual Router attributes
 *
 * @param[in] sai_uint32_t virtual_router_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */

sai_status_t sai_set_virtual_router_attribute_wrp(
    IN sai_uint32_t virtual_router_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);
/**
 * @brief Get Virtual Router attributes
 *
 * @param[in] sai_uint32_t virtual_router_id
 * @param[in] sai_virtual_router_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_virtual_router_attribute_wrp(
    IN sai_uint32_t virtual_router_id,
    IN sai_virtual_router_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);
/**
 * @brief Remove Virtual Router
 *
 * @param[in] sai_uint32_t virtual_router_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_virtual_router_wrp(
    IN sai_uint32_t virtual_router_id);


#ifdef __cplusplus
}
#endif

#endif


