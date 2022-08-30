#ifndef _luaSaiWred_H
#define _luaSaiWred_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN
#undef OUT
#define OUT


/**
 * @brief Create wred default
 *
 * @param[in] sai_uint32_t switch_id
 * @return Object_ID
 */
sai_uint32_t sai_create_wred_default_wrp(
    IN sai_uint32_t switch_id);

/**
 * @brief Create wred custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t ecn_yellow_mark_probability
 * @param[in] sai_uint32_t ecn_green_mark_probability
 * @param[in] sai_uint32_t red_max_threshold
 * @param[in] sai_uint32_t ecn_color_unaware_max_threshold
 * @param[in] sai_uint32_t ecn_color_unaware_min_threshold
 * @param[in] sai_uint32_t ecn_green_max_threshold
 * @param[in] sai_uint32_t yellow_drop_probability
 * @param[in] sai_uint32_t red_drop_probability
 * @param[in] sai_uint32_t green_drop_probability
 * @param[in] uint8_t green_enable
 * @param[in] sai_uint32_t ecn_red_mark_probability
 * @param[in] sai_uint32_t ecn_green_min_threshold
 * @param[in] sai_ecn_mark_mode_t ecn_mark_mode
 * @param[in] sai_uint32_t yellow_min_threshold
 * @param[in] sai_uint32_t ecn_red_min_threshold
 * @param[in] sai_uint32_t green_min_threshold
 * @param[in] sai_uint32_t ecn_red_max_threshold
 * @param[in] sai_uint32_t ecn_yellow_max_threshold
 * @param[in] sai_uint32_t ecn_yellow_min_threshold
 * @param[in] sai_uint32_t green_max_threshold
 * @param[in] uint8_t red_enable
 * @param[in] sai_uint32_t ecn_color_unaware_mark_probability
 * @param[in] sai_uint32_t yellow_max_threshold
 * @param[in] sai_uint8_t weight
 * @param[in] uint8_t yellow_enable
 * @param[in] sai_uint32_t red_min_threshold
 * @return Object_ID
 */
sai_uint32_t sai_create_wred_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t ecn_yellow_mark_probability,
    IN sai_uint32_t ecn_green_mark_probability,
    IN sai_uint32_t red_max_threshold,
    IN sai_uint32_t ecn_color_unaware_max_threshold,
    IN sai_uint32_t ecn_color_unaware_min_threshold,
    IN sai_uint32_t ecn_green_max_threshold,
    IN sai_uint32_t yellow_drop_probability,
    IN sai_uint32_t red_drop_probability,
    IN sai_uint32_t green_drop_probability,
    IN uint8_t green_enable,
    IN sai_uint32_t ecn_red_mark_probability,
    IN sai_uint32_t ecn_green_min_threshold,
    IN sai_ecn_mark_mode_t ecn_mark_mode,
    IN sai_uint32_t yellow_min_threshold,
    IN sai_uint32_t ecn_red_min_threshold,
    IN sai_uint32_t green_min_threshold,
    IN sai_uint32_t ecn_red_max_threshold,
    IN sai_uint32_t ecn_yellow_max_threshold,
    IN sai_uint32_t ecn_yellow_min_threshold,
    IN sai_uint32_t green_max_threshold,
    IN uint8_t red_enable,
    IN sai_uint32_t ecn_color_unaware_mark_probability,
    IN sai_uint32_t yellow_max_threshold,
    IN sai_uint8_t weight,
    IN uint8_t yellow_enable,
    IN sai_uint32_t red_min_threshold);

/**
 * @brief Set wred attributes
 *
 * @param[in] sai_uint32_t wred_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_wred_attribute_wrp(
    IN sai_uint32_t wred_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get wred attributes
 *
 * @param[in] sai_uint32_t wred_id
 * @param[in] sai_wred_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[out] sai_attribute_t out_attr[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_wred_attribute_wrp(
    IN sai_uint32_t wred_id,
    IN sai_wred_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t out_attr[]);/*arrSizeVarName=list_count*/

/**
 * @brief Remove wred
 *
 * @param[in] sai_uint32_t wred_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_wred_wrp(
    IN sai_uint32_t wred_id);

#ifdef __cplusplus
}
#endif

#endif /* _luaSaiWred_H */
