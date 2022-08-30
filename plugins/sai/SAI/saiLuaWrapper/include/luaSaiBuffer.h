#ifndef _luaSaiBuffer_H
#define _luaSaiBuffer_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN

/**
 * @brief Create ingress priority group default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] uint8_t index
 * @param[in] sai_uint32_t port
 * @return Object_ID
 */
sai_uint32_t sai_create_ingress_priority_group_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint8_t index,
    IN sai_uint32_t port);

/**
 * @brief Create ingress priority group custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] uint8_t index
 * @param[in] sai_uint32_t port
 * @param[in] uint8_t tam_count
 * @param[in] sai_uint32_t tam_ptr[]
 * @param[in] sai_uint32_t buffer_profile
 * @return Object_ID
 */
sai_uint32_t sai_create_ingress_priority_group_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint8_t index,
    IN sai_uint32_t port,
    IN uint8_t tam_count,
    IN sai_uint32_t tam_ptr[],/*arrSizeVarName=tam_count*/
    IN sai_uint32_t buffer_profile);

/**
 * @brief Set ingress priority group attributes
 *
 * @param[in] sai_uint32_t ingress_priority_group_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_ingress_priority_group_attribute_wrp(
    IN sai_uint32_t ingress_priority_group_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get ingress priority group attributes
 *
 * @param[in] sai_uint32_t ingress_priority_group_id
 * @param[in] sai_ingress_priority_group_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[out] sai_attribute_t out_attr[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_ingress_priority_group_attribute_wrp(
    IN sai_uint32_t ingress_priority_group_id,
    IN sai_ingress_priority_group_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t out_attr[]);/*arrSizeVarName=list_count*/

/**
 * @brief Remove ingress priority group
 *
 * @param[in] sai_uint32_t ingress_priority_group_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_ingress_priority_group_wrp(
    IN sai_uint32_t ingress_priority_group_id);

/**
 * @brief Get ingress priority group stats
 *
 * @param[in] sai_uint32_t ingress_priority_group_id
 * @param[in] uint32_t number_of_counters
 * @param[in] sai_stat_id_t counter_ids[]
 * @param[out] uint32_t counters[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_ingress_priority_group_stats_wrp(
    IN sai_uint32_t ingress_priority_group_id,
    IN uint32_t number_of_counters,
    IN sai_stat_id_t counter_ids[],/*arrSizeVarName=number_of_counters*/
    OUT uint32_t counters[]);/*arrSizeVarName=number_of_counters*/

/**
 * @brief Clear ingress priority group stats
 *
 * @param[in] sai_uint32_t ingress_priority_group_id
 * @param[in] uint32_t number_of_counters
 * @param[in] sai_stat_id_t counter_ids[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_clear_ingress_priority_group_stats_wrp(
    IN sai_uint32_t ingress_priority_group_id,
    IN uint32_t number_of_counters,
    IN sai_stat_id_t counter_ids[]);/*arrSizeVarName=number_of_counters*/

/**
 * @brief Create buffer pool default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t size
 * @param[in] sai_buffer_pool_threshold_mode_t threshold_mode
 * @param[in] sai_buffer_pool_type_t type
 * @return Object_ID
 */
sai_uint32_t sai_create_buffer_pool_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t size,
    IN sai_buffer_pool_threshold_mode_t threshold_mode,
    IN sai_buffer_pool_type_t type);

/**
 * @brief Create buffer pool custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t size
 * @param[in] sai_uint32_t xoff_size
 * @param[in] uint8_t tam_count
 * @param[in] sai_uint32_t tam_ptr[]
 * @param[in] sai_uint32_t wred_profile_id
 * @param[in] sai_buffer_pool_threshold_mode_t threshold_mode
 * @param[in] sai_buffer_pool_type_t type
 * @return Object_ID
 */
sai_uint32_t sai_create_buffer_pool_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t size,
    IN sai_uint32_t xoff_size,
    IN uint8_t tam_count,
    IN sai_uint32_t tam_ptr[],/*arrSizeVarName=tam_count*/
    IN sai_uint32_t wred_profile_id,
    IN sai_buffer_pool_threshold_mode_t threshold_mode,
    IN sai_buffer_pool_type_t type);

/**
 * @brief Set buffer pool attributes
 *
 * @param[in] sai_uint32_t buffer_pool_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_buffer_pool_attribute_wrp(
    IN sai_uint32_t buffer_pool_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get buffer pool group attributes
 *
 * @param[in] sai_uint32_t buffer_pool_id
 * @param[in] sai_buffer_pool_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[out] sai_attribute_t out_attr[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_buffer_pool_attribute_wrp(
    IN sai_uint32_t buffer_pool_id,
    IN sai_buffer_pool_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t out_attr[]);/*arrSizeVarName=list_count*/

/**
 * @brief Remove buffer pool id
 *
 * @param[in] sai_uint32_t buffer_pool_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_buffer_pool_wrp(
    IN sai_uint32_t buffer_pool_id);

/**
 * @brief Get buffer pool stats
 *
 * @param[in] sai_uint32_t buffer_pool_id
 * @param[in] uint32_t number_of_counters
 * @param[in] sai_stat_id_t counter_ids[]
 * @param[out] uint32_t counters[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_buffer_pool_stats_wrp(
    IN sai_uint32_t buffer_pool_id,
    IN uint32_t number_of_counters,
    IN sai_stat_id_t counter_ids[],/*arrSizeVarName=number_of_counters*/
    OUT uint32_t counters[]);/*arrSizeVarName=number_of_counters*/

/**
 * @brief Clear buffer pool stats
 *
 * @param[in] sai_uint32_t buffer_pool_id
 * @param[in] uint32_t number_of_counters
 * @param[in] sai_stat_id_t counter_ids[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_clear_buffer_pool_stats_wrp(
    IN sai_uint32_t buffer_pool_id,
    IN uint32_t number_of_counters,
    IN sai_stat_id_t counter_ids[]);/*arrSizeVarName=number_of_counters*/

/**
 * @brief Create buffer profile default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_int8_t shared_dynamic_th
 * @param[in] sai_uint32_t shared_static_th
 * @param[in] sai_uint32_t reserved_buffer_size
 * @param[in] sai_buffer_profile_threshold_mode_t threshold_mode
 * @param[in] sai_uint32_t pool_id
 * @return Object_ID
 */
sai_uint32_t sai_create_buffer_profile_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_int8_t shared_dynamic_th,
    IN sai_uint32_t shared_static_th,
    IN sai_uint32_t reserved_buffer_size,
    IN sai_buffer_profile_threshold_mode_t threshold_mode,
    IN sai_uint32_t pool_id);

/**
 * @brief Create buffer profile custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t xoff_th
 * @param[in] sai_int8_t shared_dynamic_th
 * @param[in] sai_uint32_t shared_static_th
 * @param[in] sai_uint32_t xon_th
 * @param[in] sai_uint32_t reserved_buffer_size
 * @param[in] sai_buffer_profile_threshold_mode_t threshold_mode
 * @param[in] sai_uint32_t pool_id
 * @return Object_ID
 */
sai_uint32_t sai_create_buffer_profile_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t xoff_th,
    IN sai_int8_t shared_dynamic_th,
    IN sai_uint32_t shared_static_th,
    IN sai_uint32_t xon_th,
    IN sai_uint32_t reserved_buffer_size,
    IN sai_buffer_profile_threshold_mode_t threshold_mode,
    IN sai_uint32_t pool_id);

/**
 * @brief Set buffer profile attributes
 *
 * @param[in] sai_uint32_t buffer_profile_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_buffer_profile_attribute_wrp(
    IN sai_uint32_t buffer_profile_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get buffer profile group attributes
 *
 * @param[in] sai_uint32_t buffer_profile_id
 * @param[in] sai_buffer_profile_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[out] sai_attribute_t out_attr[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_buffer_profile_attribute_wrp(
    IN sai_uint32_t buffer_profile_id,
    IN sai_buffer_profile_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t out_attr[]); /*arrSizeVarName=list_count*/

/**
 * @brief Remove buffer profile
 *
 * @param[in] sai_uint32_t buffer_profile_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_buffer_profile_wrp(
    IN sai_uint32_t buffer_profile_id);

#ifdef __cplusplus
}
#endif

#endif
