#ifndef _luaSaiPolicer_H
#define _luaSaiPolicer_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN
#undef OUT
#define OUT


/**
 * @brief Create policer default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_meter_type_t meter_type
 * @param[in] sai_policer_mode_t mode
 * @param[in] sai_policer_color_source_t color_source
 * @return Object_ID
 */
sai_uint32_t sai_create_policer_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_meter_type_t meter_type,
    IN sai_policer_mode_t mode,
    IN sai_policer_color_source_t color_source);

/**
 * @brief Create policer custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_meter_type_t meter_type
 * @param[in] sai_uint32_t cir
 * @param[in] sai_packet_action_t yellow_packet_action
 * @param[in] sai_policer_mode_t mode
 * @param[in] uint8_t pkt_action_count
 * @param[in] sai_s32_list_t enable_counter_packet_action_list_ptr[]
 * @param[in] sai_policer_color_source_t color_source
 * @param[in] sai_packet_action_t green_packet_action
 * @param[in] sai_packet_action_t red_packet_action
 * @param[in] sai_uint32_t cbs
 * @param[in] sai_uint32_t pbs
 * @param[in] sai_uint32_t pir
 * @return Object_ID
 */
sai_uint32_t sai_create_policer_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_meter_type_t meter_type,
    IN sai_uint32_t cir,
    IN sai_packet_action_t yellow_packet_action,
    IN sai_policer_mode_t mode,
    IN uint8_t pkt_action_count,
    IN sai_s32_list_t
    enable_counter_packet_action_list_ptr[],/*arrSizeVarName=tam_count*/
    IN sai_policer_color_source_t color_source,
    IN sai_packet_action_t green_packet_action,
    IN sai_packet_action_t red_packet_action,
    IN sai_uint32_t cbs,
    IN sai_uint32_t pbs,
    IN sai_uint32_t pir);

/**
 * @brief Set policer attributes
 *
 * @param[in] sai_uint32_t policer_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_policer_attribute_wrp(
    IN sai_uint32_t policer_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get policer attributes
 *
 * @param[in] sai_uint32_t policer_id
 * @param[in] sai_policer_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[out] sai_attribute_t out_attr[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_policer_attribute_wrp(
    IN sai_uint32_t policer_id,
    IN sai_policer_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t out_attr[]);/*arrSizeVarName=list_count*/

/**
 * @brief Remove policer
 *
 * @param[in] sai_uint32_t policer_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_policer_wrp(
    IN sai_uint32_t policer_id);

/**
 * @brief Get policer stats
 *
 * @param[in] sai_uint32_t policer_id
 * @param[in] uint32_t number_of_counters
 * @param[in] sai_stat_id_t counter_ids[]
 * @param[out] uint32_t counters[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_policer_stats_wrp(
    IN sai_uint32_t policer_id,
    IN uint32_t number_of_counters,
    IN sai_stat_id_t counter_ids[],/*arrSizeVarName=number_of_counters*/
    OUT uint32_t counters[]);/*arrSizeVarName=number_of_counters*/

/**
 * @brief Clear policer stats
 *
 * @param[in] sai_uint32_t policer_id
 * @param[in] uint32_t number_of_counters
 * @param[in] sai_stat_id_t counter_ids[]
 * @param[out] uint32_t counters[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_clear_policer_stats_wrp(
    IN sai_uint32_t policer_id,
    IN uint32_t number_of_counters,
    IN sai_stat_id_t counter_ids[]);/*arrSizeVarName=number_of_counters*/

#ifdef __cplusplus
}
#endif

#endif /* _luaSaiPolicer_H */
