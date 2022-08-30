#ifndef _luaSaiL2mcgroup_h_
#define _luaSaiL2mcgroup_h_

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN
#undef OUT
#define OUT

/**
 * @brief Create l2mc group default
 *
 * @param[in] sai_uint32_t switch_id
 * @return Object_ID
 */
sai_uint32_t sai_create_l2mc_group_default_wrp(
    IN sai_uint32_t switch_id);

/**
 * @brief Create l2mc group custom
 *
 * @param[in] sai_uint32_t switch_id
 * @return Object_ID
 */
sai_uint32_t sai_create_l2mc_group_custom_wrp(
    IN sai_uint32_t switch_id);

/**
 * @brief Set l2mc group attribute
 *
 * @param[in] sai_uint32_t l2mc_group_id
 * @param[in] uint32_t  attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] uint8_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_l2mc_group_attribute_wrp(
    IN sai_uint32_t l2mc_group_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN uint8_t *ip_addr);

/**
 * @brief Get l2mc group attribute
 *
 * @param[in] sai_uint32_t l2mc_group_id
 * @param[in] sai_l2mc_group_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[out] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_l2mc_group_attribute_wrp(
    IN sai_uint32_t l2mc_group_id,
    IN sai_l2mc_group_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief Remove l2mc group attribute
 *
 * @param[in] sai_uint32_t l2mc_group_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_l2mc_group_wrp(
    IN sai_uint32_t l2mc_group_id);

/**
 * @brief Create l2mc group member default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] uint8_t *addr_l2mc_endpoint_ip
 * @param[in] sai_uint32_t l2mc_group_id
 * @param[in] sai_uint32_t l2mc_output_id
 * @return Object_ID
 */
sai_uint32_t sai_create_l2mc_group_member_default_wrp(
    IN sai_uint32_t switch_id,
    IN uint8_t *addr_l2mc_endpoint_ip,
    IN sai_uint32_t l2mc_group_id,
    IN sai_uint32_t l2mc_output_id);

/**
 * @brief Create l2mc group member custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] uint8_t *addr_l2mc_endpoint_ip
 * @param[in] sai_uint32_t l2mc_group_id
 * @param[in] sai_uint32_t l2mc_output_id
 * @return Object_ID
 */
sai_uint32_t sai_create_l2mc_group_member_custom_wrp(
    IN sai_uint32_t switch_id,
    IN uint8_t *addr_l2mc_endpoint_ip,
    IN sai_uint32_t l2mc_group_id,
    IN sai_uint32_t l2mc_output_id);

/**
 * @brief Set l2mc group member attribute
 *
 * @param[in] sai_uint32_t l2mc_group_member_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] uint8_t  *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_l2mc_group_member_attribute_wrp(
    IN sai_uint32_t l2mc_group_member_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN uint8_t *ip_addr);

/**
 * @brief Get l2mc group member attribute
 *
 * @param[in] sai_uint32_t l2mc_group_member_id
 * @param[in] sai_l2mc_group_member_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[out] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_l2mc_group_member_attribute_wrp(
    IN sai_uint32_t l2mc_group_member_id,
    IN sai_l2mc_group_member_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief Remove l2mc group member
 *
 * @param[in] sai_uint32_t l2mc_group_member_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_l2mc_group_member_wrp(
    IN sai_uint32_t l2mc_group_member_id);

#ifdef __cplusplus
}
#endif
#endif
