#ifndef _luaSaiScheduler_H
#define _luaSaiScheduler_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN
#undef OUT
#define OUT


/**
 * @brief Create scheduler default
 *
 * @param[in] sai_uint32_t switch_id
 * @return Object_ID
 */
sai_uint32_t sai_create_scheduler_default_wrp(
    IN sai_uint32_t switch_id);

/**
 * @brief Create scheduler custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t min_bandwidth_burst_rate
 * @param[in] sai_scheduling_type_t scheduling_type
 * @param[in] sai_uint32_t max_bandwidth_rate
 * @param[in] sai_uint32_t min_bandwidth_rate
 * @param[in] sai_uint32_t max_bandwidth_burst_rate
 * @param[in] sai_meter_type_t meter_type
 * @param[in] sai_uint8_t scheduling_weight
 * @return Object_ID
 */
sai_uint32_t sai_create_scheduler_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t min_bandwidth_burst_rate,
    IN sai_scheduling_type_t scheduling_type,
    IN sai_uint32_t max_bandwidth_rate,
    IN sai_uint32_t min_bandwidth_rate,
    IN sai_uint32_t max_bandwidth_burst_rate,
    IN sai_meter_type_t meter_type,
    IN sai_uint8_t scheduling_weight);

/**
 * @brief Set scheduler attributes
 *
 * @param[in] sai_uint32_t scheduler_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_scheduler_attribute_wrp(
    IN sai_uint32_t scheduler_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get scheduler attributes
 *
 * @param[in] sai_uint32_t scheduler_id
 * @param[in] sai_scheduler_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[out] sai_attribute_t out_attr[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_scheduler_attribute_wrp(
    IN sai_uint32_t scheduler_id,
    IN sai_scheduler_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t out_attr[]);/*arrSizeVarName=list_count*/

/**
 * @brief Remove scheduler
 *
 * @param[in] sai_uint32_t scheduler_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_scheduler_wrp(
    IN sai_uint32_t scheduler_id);


#ifdef __cplusplus
}
#endif

#endif /* _luaSaiScheduler_H */
