#ifndef _luaSaiPort_H
#define _luaSaiPort_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN
#undef OUT
#define OUT


/**
 * @brief Create Port default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_u32_list_t *hw_lane_list_ptr
 * @param[in] uint8_t full_duplex_mode
 * @param[in] sai_uint64_t reference_clock
 * @param[in] sai_uint32_t speed
 * @return Object_ID
 */
sai_uint32_t sai_create_port_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_u32_list_t *hw_lane_list_ptr,
    IN uint8_t full_duplex_mode,
    IN sai_uint64_t reference_clock,
    IN sai_uint32_t speed);

#if 0 /* TBD - check if this function is needed */
/**
 * @brief Create Port custom
 *
 * @return Object_ID
 */
sai_uint32_t sai_create_port_custom_wrp()
#endif /* if 0 */

/**
 * @brief Set Port attributes
 *
 * @param[in] sai_uint32_t port_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_port_attribute_wrp(
    IN sai_uint32_t port_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get Port attributes
 *
 * @param[in] sai_uint32_t port_id
 * @param[in] sai_port_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_port_attribute_wrp(
    IN sai_uint32_t port_id,
    IN sai_port_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief Remove Port
 *
 * @param[in] sai_uint32_t port_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_port_wrp(
    IN sai_uint32_t port_id);

/**
 * @brief Get Port stats
 *
 * @param[in] sai_uint32_t port_id
 * @param[in] uint32_t number_of_counters
 * @param[in] const sai_stat_id_t *counter_ids
 * @param[in] uint32_t *counters
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_port_stats_wrp(
    IN sai_uint32_t port_id,
    IN uint32_t number_of_counters,
    IN const sai_stat_id_t *counter_ids,
    OUT uint32_t *counters);

/**
 * @brief Clear Port stats
 *
 * @param[in] sai_uint32_t port_id
 * @param[in] uint32_t number_of_counters
 * @param[in] const sai_stat_id_t *counter_ids
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_clear_port_stats_wrp(
    IN sai_uint32_t port_id,
    IN uint32_t number_of_counters,
    IN const sai_stat_id_t *counter_ids);

/**
 * @brief Clear Port all stats
 *
 * @param[in] sai_uint32_t port_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_clear_port_all_stats_wrp(
    IN sai_uint32_t port_id);

/**
 * @brief Create Port pool default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t buffer_pool_id
 * @param[in] sai_uint32_t port_id
 * @return Object_ID
 */
sai_uint32_t sai_create_port_pool_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t buffer_pool_id,
    IN sai_uint32_t port_id);

/**
 * @brief Create Port pool custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t buffer_pool_id
 * @param[in] sai_uint32_t port_id
 * @param[in] sai_uint32_t qos_wred_profile_id
 * @return Object_ID
 */
sai_uint32_t sai_create_port_pool_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t buffer_pool_id,
    IN sai_uint32_t port_id,
    IN sai_uint32_t qos_wred_profile_id);

/**
 * @brief Set Port pool attributes
 *
 * @param[in] sai_uint32_t port_pool_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_port_pool_attribute_wrp(
    IN sai_uint32_t port_pool_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get Port pool attributes
 *
 * @param[in] sai_uint32_t port_pool_id
 * @param[in] sai_port_pool_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_port_pool_attribute_wrp(
    IN sai_uint32_t port_pool_id,
    IN sai_port_pool_attr_t attr_id,
    IN uint32_t list_count,
    IN sai_attribute_t *out_attr);

/**
 * @brief Remove Port pool
 *
 * @param[in] sai_uint32_t port_pool_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_port_pool_wrp(
    IN sai_uint32_t port_pool_id);

/**
 * @brief Get Port pool stats
 *
 * @param[in] sai_uint32_t port_pool_id
 * @param[in] uint32_t number_of_counters
 * @param[in] const sai_stat_id_t *counter_ids
 * @param[in] uint64_t *counters
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_port_pool_stats_wrp(
    IN sai_uint32_t port_pool_id,
    IN uint32_t number_of_counters,
    IN const sai_stat_id_t *counter_ids,
    IN uint64_t *counters);

/**
 * @brief Clear Port pool stats
 *
 * @param[in] sai_uint32_t port_pool_id
 * @param[in] uint32_t number_of_counters
 * @param[in] const sai_stat_id_t *counter_ids
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_clear_port_pool_stats_wrp(
    IN sai_uint32_t port_pool_id,
    IN uint32_t number_of_counters,
    IN const sai_stat_id_t *counter_ids);

#ifdef __cplusplus
}
#endif

#endif /* _luaSaiPort_H */
