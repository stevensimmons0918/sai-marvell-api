#ifndef _luaSaiL2mc_h_
#define _luaSaiL2mc_h_

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN
#undef OUT
#define OUT

/**
 * @brief Create l2mc entry  default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t bv_id
 * @param[in] sai_l2mc_entry_type_t entry_type
 * @param[in] sai_ip_address_t *src_ip_address
 * @param[in] sai_ip_address_t *dst_ip_address
 * @param[in] sai_packet_action_t packet_action
 * @return Object_ID
 */
sai_uint32_t sai_create_l2mc_entry_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t bv_id,
    IN sai_l2mc_entry_type_t entry_type,
    IN sai_ip_address_t *src_ip_address,
    IN sai_ip_address_t *dst_ip_address,
    IN sai_packet_action_t packet_action);

/**
 * @brief Create l2mc entry custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t bv_id
 * @param[in] sai_l2mc_entry_type_t entry_type
 * @param[in] sai_ip_address_t *src_ip_address
 * @param[in] sai_ip_address_t *dst_ip_address
 * @param[in] sai_uint32_t output_group_id
 * @param[in] sai_packet_action_t packet_action
 * @return Object_ID
 */

sai_uint32_t sai_create_l2mc_entry_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t bv_id,
    IN sai_l2mc_entry_type_t entry_type,
    IN sai_ip_address_t *src_ip_address,
    IN sai_ip_address_t *dst_ip_address,
    IN sai_uint32_t output_group_id,
    IN sai_packet_action_t packet_action);

/**
 * @brief Set l2mc entry attribute
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t bv_id
 * @param[in] sai_l2mc_entry_type_t entry_type
 * @param[in] sai_ip_address_t *src_ip_address
 * @param[in] sai_ip_address_t *dst_ip_address
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_l2mc_entry_attribute_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t bv_id,
    IN sai_l2mc_entry_type_t entry_type,
    IN sai_ip_address_t *src_ip_address,
    IN sai_ip_address_t *dst_ip_address,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get l2mc entry attribute
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t bv_id
 * @param[in] sai_l2mc_entry_type_t entry_type
 * @param[in] sai_ip_address_t *src_ip_address
 * @param[in] sai_ip_address_t *dst_ip_address
 * @param[in] sai_l2mc_entry_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[out] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_l2mc_entry_attribute_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t bv_id,
    IN sai_l2mc_entry_type_t entry_type,
    IN sai_ip_address_t *src_ip_address,
    IN sai_ip_address_t *dst_ip_address,
    IN sai_l2mc_entry_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief Remove l2mc entry
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t bv_id
 * @param[in] sai_l2mc_entry_type_t entry_type
 * @param[in] sai_ip_address_t *src_ip_address
 * @param[in] sai_ip_address_t *dst_ip_address
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_l2mc_entry_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t bv_id,
    IN sai_l2mc_entry_type_t entry_type,
    IN sai_ip_address_t *src_ip_address,
    IN sai_ip_address_t *dst_ip_address);


#ifdef __cplusplus
}
#endif
#endif
