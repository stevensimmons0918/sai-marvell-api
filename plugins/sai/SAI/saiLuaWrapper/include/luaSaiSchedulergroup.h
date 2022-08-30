#ifndef _luaSaiSchedulergroup_H
#define _luaSaiSchedulergroup_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN
#undef OUT
#define OUT


/**
 * @brief Create scheduler_group default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t port_id
 * @param[in] sai_uint8_t level
 * @param[in] sai_uint8_t max_childs
 * @param[in] sai_uint32_t parent_node
 * @return Object_ID
 */
sai_uint32_t sai_create_scheduler_group_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t port_id,
    IN sai_uint8_t level,
    IN sai_uint8_t max_childs,
    IN sai_uint32_t parent_node);

/**
 * @brief Create scheduler_group custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t port_id
 * @param[in] sai_uint8_t level
 * @param[in] sai_uint32_t scheduler_profile_id
 * @param[in] sai_uint8_t max_childs
 * @param[in] sai_uint32_t parent_node
 * @return Object_ID
 */
sai_uint32_t sai_create_scheduler_group_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t port_id,
    IN sai_uint8_t level,
    IN sai_uint32_t scheduler_profile_id,
    IN sai_uint8_t max_childs,
    IN sai_uint32_t parent_node);

/**
 * @brief Set scheduler_group attributes
 *
 * @param[in] sai_uint32_t scheduler_group_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_scheduler_group_attribute_wrp(
    IN sai_uint32_t scheduler_group_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get scheduler_group attributes
 *
 * @param[in] sai_uint32_t scheduler_group_id
 * @param[in] sai_scheduler_group_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[out] sai_attribute_t out_attr[]
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_scheduler_group_attribute_wrp(
    IN sai_uint32_t scheduler_group_id,
    IN sai_scheduler_group_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t out_attr[]);/*arrSizeVarName=list_count*/

/**
 * @brief Remove scheduler_group
 *
 * @param[in] sai_uint32_t scheduler_group_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_scheduler_group_wrp(
    IN sai_uint32_t scheduler_group_id);

#ifdef __cplusplus
}
#endif

#endif /* _luaSaiSchedulergroup_H */
