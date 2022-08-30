#ifndef _luaSaiNexthopgroupgroup_H
#define _luaSaiNexthopgroupgroup_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN
#undef OUT
#define OUT


/**
 * @brief Create Nexthopgroup default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_next_hop_group_type_t type
 * @param[in] sai_uint32_t configured_size
 * @return Object_ID
 */

sai_uint32_t sai_create_next_hop_group_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_next_hop_group_type_t type,
    IN sai_uint32_t configured_size);
/**
 * @brief Set Nexthop attributes
 *
 * @param[in] sai_uint32_t next_hop_group_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */

sai_status_t sai_set_next_hop_group_attribute_wrp(
    IN sai_uint32_t next_hop_group_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);
/**
 * @brief Get Nexthop attributes
 *
 * @param[in] sai_uint32_t next_hop_group_id
 * @param[in] sai_next_hop_group_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_next_hop_group_attribute_wrp(
    IN sai_uint32_t next_hop_group_id,
    IN sai_next_hop_group_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);
/**
 * @brief Remove Nexthop
 *
 * @param[in] sai_uint32_t next_hop_group_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */

sai_status_t sai_remove_next_hop_group_wrp(
    IN sai_uint32_t next_hop_group_id);

/**
 * @brief Create Nexthopgroup member default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t index
 * @param[in] sai_uint32_t next_hop_id
 * @param[in] sai_uint32_t next_hop_group_id
 * @param[in] sai_uint32_t sequence_id
 * @return Object_ID
 */

sai_uint32_t sai_create_next_hop_group_member_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t index,
    IN sai_uint32_t next_hop_id,
    IN sai_uint32_t next_hop_group_id,
    IN sai_uint32_t sequence_id);
/**
 * @brief Create Nexthopgroup member custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t counter_id
 * @param[in] sai_uint32_t index
 * @param[in] sai_uint32_t next_hop_id
 * @param[in] sai_uint32_t next_hop_group_id
 * @param[in] sai_uint32_t sequence_id
 * @return Object_ID
 */

sai_uint32_t sai_create_next_hop_group_member_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t counter_id,
    IN sai_uint32_t index,
    IN sai_uint32_t next_hop_id,
    IN sai_uint32_t next_hop_group_id,
    IN sai_uint32_t sequence_id);
/**
 * @brief Set Nexthopgroup attributes
 *
 * @param[in] sai_uint32_t next_hop_group_member_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */

sai_status_t sai_set_next_hop_group_member_attribute_wrp(
    IN sai_uint32_t next_hop_group_member_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);
/**
 * @brief Get Nexthopgroup member attributes
 *
 * @param[in] sai_uint32_t next_hop_group_member_id
 * @param[in] sai_next_hop_group_member_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */

sai_status_t sai_get_next_hop_group_member_attribute_wrp(
    IN sai_uint32_t next_hop_group_member_id,
    IN sai_next_hop_group_member_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);
/**
 * @brief Remove Nexthopgroup member
 *
 * @param[in] sai_uint32_t next_hop_group_member_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */

sai_status_t sai_remove_next_hop_group_member_wrp(
    IN sai_uint32_t next_hop_group_member_id);


#ifdef __cplusplus
}
#endif



#endif


