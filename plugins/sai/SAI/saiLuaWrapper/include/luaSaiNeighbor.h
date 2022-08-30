#ifndef _luaSaiNeighbor_H
#define _luaSaiNeighbor_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN
#undef OUT
#define OUT


/**
 * @brief Create Neighbor default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t rif_id
 * @param[in] sai_ip_address_t *ip_address
 * @param[in] shell_mac *dst_mac_address
 * @return Object_ID
 */

sai_uint32_t sai_create_neighbor_entry_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t rif_id,
    IN sai_ip_address_t *ip_address,
    IN shell_mac *dst_mac_address);
/**
 * @brief Create Neighbor custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t rif_id
 * @param[in] sai_ip_address_t *ip_address
 * @param[in] uint8_t is_local,
 * @param[in] sai_packet_action_t packet_action
 * @param[in] sai_uint32_t user_trap_id
 * @param[in] sai_uint32_t encap_index
 * @param[in] shell_mac *dst_mac_address
 * @param[in] uint8_t no_host_route
 * @param[in] sai_uint32_t counter_id
 * @param[in] uint8_t encap_impose_index
 * @return Object_ID
 */

sai_uint32_t sai_create_neighbor_entry_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t rif_id,
    IN sai_ip_address_t *ip_address,
    IN uint8_t is_local,
    IN sai_packet_action_t packet_action,
    IN sai_uint32_t user_trap_id,
    IN sai_uint32_t encap_index,
    IN shell_mac *dst_mac_address,
    IN uint8_t no_host_route,
    IN sai_uint32_t counter_id,
    IN uint8_t encap_impose_index);

/**
 * @brief Set Neighbor attributes
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t rif_id
 * @param[in] sai_ip_address_t *ip_address
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_neighbor_entry_attribute_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t rif_id,
    IN sai_ip_address_t *ip_address,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);
/**
 * @brief Get Neighbor attributes
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t rif_id
 * @param[in] sai_ip_address_t *ip_address
 * @param[in] sai_neighbor_entry_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */

sai_status_t sai_get_neighbor_entry_attribute_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t rif_id,
    IN sai_ip_address_t *ip_address,
    IN sai_neighbor_entry_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);
/**
 * @brief Remove Neighbor
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t rif_id
 * @param[in] sai_ip_address_t *ip_address
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */

sai_status_t sai_remove_neighbor_entry_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t rif_id,
    IN sai_ip_address_t *ip_address);

#ifdef __cplusplus
}
#endif



#endif


