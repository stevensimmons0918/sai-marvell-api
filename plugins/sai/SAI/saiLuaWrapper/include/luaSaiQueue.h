#ifndef _luaSaiQueue_H
#define _luaSaiQueue_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN
#undef OUT
#define OUT


/**
 * @brief Create queue default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint8_t index
 * @param[in] sai_uint32_t parent_scheduler_node
 * @param[in] sai_uint32_t port
 * @param[in] sai_queue_type_t type
 * @return Object_ID
 */
sai_uint32_t sai_create_queue_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint8_t index,
    IN sai_uint32_t parent_scheduler_node,
    IN sai_uint32_t port,
    IN sai_queue_type_t type);

/**
 * @brief Create queue custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint8_t index
 * @param[in] sai_uint32_t wred_profile_id
 * @param[in] sai_uint32_t parent_scheduler_node
 * @param[in] uint8_t pfc_dlr_init
 * @param[in] sai_uint32_t buffer_profile_id
 * @param[in] sai_uint32_t scheduler_profile_id
 * @param[in] sai_uint32_t port
 * @param[in] sai_queue_type_t type
 * @return Object_ID
 */
sai_uint32_t sai_create_queue_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint8_t index,
    IN sai_uint32_t wred_profile_id,
    IN sai_uint32_t parent_scheduler_node,
    IN uint8_t pfc_dlr_init,
    IN sai_uint32_t buffer_profile_id,
    IN sai_uint32_t scheduler_profile_id,
    IN sai_uint32_t port,
    IN sai_queue_type_t type);

/**
 * @brief Set queue attributes
 *
 * @param[in] sai_uint32_t queue_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_queue_attribute_wrp(
    IN sai_uint32_t queue_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get queue attributes
 *
 * @param[in] sai_uint32_t queue_id
 * @param[in] sai_queue_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[out] sai_attribute_t out_attr[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_queue_attribute_wrp(
    IN sai_uint32_t queue_id,
    IN sai_queue_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t out_attr[]);/*arrSizeVarName=list_count*/

/**
 * @brief Remove queue
 *
 * @param[in] sai_uint32_t queue_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_queue_wrp(
    IN sai_uint32_t queue_id);

/**
 * @brief Get queue stats
 *
 * @param[in] sai_uint32_t queue_id
 * @param[in] uint32_t number_of_counters
 * @param[in] sai_stat_id_t counter_ids[]
 * @param[out] uint32_t counters[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_queue_stats_wrp(
    IN sai_uint32_t queue_id,
    IN uint32_t number_of_counters,
    IN sai_stat_id_t counter_ids[],/*arrSizeVarName=number_of_counters*/
    OUT uint32_t counters[]);/*arrSizeVarName=number_of_counters*/

/**
 * @brief Clear queue stats
 *
 * @param[in] sai_uint32_t queue_id
 * @param[in] uint32_t number_of_counters
 * @param[in] sai_stat_id_t counter_ids[]
 * @param[out] uint32_t counters[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_clear_queue_stats_wrp(
    IN sai_uint32_t queue_id,
    IN uint32_t number_of_counters,
    IN sai_stat_id_t counter_ids[]);/*arrSizeVarName=number_of_counters*/


#ifdef __cplusplus
}
#endif

#endif /* _luaSaiQueue_H */
