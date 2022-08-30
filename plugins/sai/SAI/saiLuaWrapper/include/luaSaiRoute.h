#ifndef _luaSaiRoute_H
#define _luaSaiRoute_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN

/**
 * @brief Create Route entry default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t vr_id
 * @param[in] uint32_t ip_addr_family
 * @param[in] shell_ip6 *ipaddr
 * @param[in] shell_ip6 *mask
 * @return Object_ID
 */
sai_uint32_t sai_create_route_entry_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t vr_id,
    IN uint32_t ip_addr_family,
    IN shell_ip6 *ipaddr,
    IN shell_ip6 *mask);

/**
 * @brief Create Route entry custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t vr_id
 * @param[in] uint32_t ip_addr_family
 * @param[in] shell_ip6 *ipaddr
 * @param[in] shell_ip6 *mask
 * @param[in] sai_uint32_t next_hop_id
 * @param[in] sai_uint32_t user_trap_id
 * @param[in] sai_packet_action_t packet_action
 * @param[in] sai_uint32_t counter_id
 * @return Object_ID
 */
sai_uint32_t sai_create_route_entry_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t vr_id,
    IN uint32_t ip_addr_family,
    IN shell_ip6 *ipaddr,
    IN shell_ip6 *mask,
    IN sai_uint32_t next_hop_id,
    IN sai_uint32_t user_trap_id,
    IN sai_packet_action_t packet_action,
    IN sai_uint32_t counter_id);

/**
 * @brief Set route entry attributes
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t vr_id
 * @param[in] uint32_t ip_addr_family
 * @param[in] shell_ip6 *ipaddr
 * @param[in] shell_ip6 *mask
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_route_entry_attribute_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t vr_id,
    IN uint32_t ip_addr_family,
    IN shell_ip6 *ipaddr,
    IN shell_ip6 *mask,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get route entry attributes
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t vr_id
 * @param[in] uint32_t ip_addr_family
 * @param[in] shell_ip6 *ipaddr
 * @param[in] shell_ip6 *mask
 * @param[in] sai_route_entry_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_route_entry_attribute_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t vr_id,
    IN uint32_t ip_addr_family,
    IN shell_ip6 *ipaddr,
    IN shell_ip6 *mask,
    IN sai_route_entry_attr_t attr_id,
    IN uint32_t list_count,
    IN sai_attribute_t *out_attr);

/**
 * @brief Remove route entry
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t vr_id
 * @param[in] uint32_t ip_addr_family
 * @param[in] shell_ip6 *ipaddr
 * @param[in] shell_ip6 *mask
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_route_entry_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t vr_id,
    IN uint32_t ip_addr_family,
    IN shell_ip6 *ipaddr,
    IN shell_ip6 *mask);

#ifdef __cplusplus
}
#endif

#endif /* _luaSaiRoute_H */
