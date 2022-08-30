#ifndef _luaSaiStp_h_
#define _luaSaiStp_h_

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN
#undef OUT
#define OUT

/**
 * @brief Create stp port default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t stp
 * @param[in] sai_uint32_t bridge_port
 * @param[in] sai_stp_port_state_t state
 * @return Object_ID
 */
sai_uint32_t sai_create_stp_port_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t stp,
    IN sai_uint32_t bridge_port,
    IN sai_stp_port_state_t state);

/**
 * @brief Create stp port custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t stp
 * @param[in] sai_uint32_t bridge_port
 * @param[in] sai_stp_port_state_t state
 * @return Object_ID
 */
sai_uint32_t sai_create_stp_port_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t stp,
    IN sai_uint32_t bridge_port,
    IN sai_stp_port_state_t state);

/**
 * @brief set stp port attribute
 *
 * @param[in] sai_uint32_t stp_port_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_stp_port_attribute_wrp(
    IN sai_uint32_t stp_port_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief get stp port attribute
 *
 * @param[in]  sai_uint32_t stp_port_id
 * @param[in]  sai_stp_port_attr_t attr_id
 * @param[in]  uint32_t list_count
 * @param[out] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_stp_port_attribute_wrp(
    IN sai_uint32_t stp_port_id,
    IN sai_stp_port_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief remove stp port
 *
 * @param[in] sai_uint32_t stp_port_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_stp_port_wrp(
    IN sai_uint32_t stp_port_id);

/**
 * @brief create stp default
 *
 * @param[in] sai_uint32_t switch_id
 * @return Object_ID
 */
sai_uint32_t sai_create_stp_default_wrp(
    IN sai_uint32_t switch_id);

/**
 * @brief create stp custom
 *
 * @param[in] sai_uint32_t switch_id
 * @return Object_ID
 */
sai_uint32_t sai_create_stp_custom_wrp(
    IN sai_uint32_t switch_id);

/**
 * @brief set stp attribute
 *
 * @param[in] sai_uint32_t stp_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_stp_attribute_wrp(
    IN sai_uint32_t stp_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief get stp attribute
 *
 * @param[in]  sai_uint32_t stp_id
 * @param[in]  sai_stp_attr_t attr_id
 * @param[in]  uint32_t list_count
 * @param[out] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_stp_attribute_wrp(
    IN sai_uint32_t stp_id,
    IN sai_stp_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief remove stp
 *
 * @param[in] sai_uint32_t stp_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_stp_wrp(
    IN sai_uint32_t stp_id);

#ifdef __cplusplus
}
#endif
#endif
