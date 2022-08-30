#ifndef _luaSaiQosmap_H
#define _luaSaiQosmap_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN
#undef OUT
#define OUT


/**
 * @brief Create qos_map default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] uint8_t qos_map_count
 * @param[in] sai_qos_map_list_t map_to_value_list_ptr[]
 * @param[in] sai_qos_map_type_t type
 * @return Object_ID
 */
sai_uint32_t sai_create_qos_map_default_wrp(
    IN sai_uint32_t switch_id,
    IN uint8_t qos_map_count,
    IN sai_qos_map_list_t map_to_value_list_ptr[],/*arrSizeVarName=qos_map_count*/
    IN sai_qos_map_type_t type);

/**
 * @brief Create qos_map custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] uint8_t qos_map_count
 * @param[in] sai_qos_map_list_t map_to_value_list_ptr[]
 * @param[in] sai_qos_map_type_t type
 * @return Object_ID
 */
sai_uint32_t sai_create_qos_map_custom_wrp(
    IN sai_uint32_t switch_id,
    IN uint8_t qos_map_count,
    IN sai_qos_map_list_t map_to_value_list_ptr[],/*arrSizeVarName=qos_map_count*/
    IN sai_qos_map_type_t type);

/**
 * @brief Set qos_map attributes
 *
 * @param[in] sai_uint32_t qos_map_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_qos_map_attribute_wrp(
    IN sai_uint32_t qos_map_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get qos_map attributes
 *
 * @param[in] sai_uint32_t qos_map_id
 * @param[in] sai_qos_map_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[out] sai_attribute_t out_attr[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_qos_map_attribute_wrp(
    IN sai_uint32_t qos_map_id,
    IN sai_qos_map_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t out_attr[]);/*arrSizeVarName=list_count*/

/**
 * @brief Remove qos_map
 *
 * @param[in] sai_uint32_t qos_map_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_qos_map_wrp(
    IN sai_uint32_t qos_map_id);

#ifdef __cplusplus
}
#endif

#endif /* _luaSaiQosmap_H */
