#ifndef _luaSaiLag_H
#define _luaSaiLag_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN

#undef OUT
#define OUT

/**
 * @brief Create Lag default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t system_port_aggregate_id
 * @return Object_ID
 */
sai_uint32_t sai_create_lag_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t system_port_aggregate_id);

/**
 * @brief Create Lag custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t egress_acl
 * @param[in] sai_uint32_t ingress_acl
 * @param[in] sai_uint16_t port_vlan_id
 * @param[in] uint8_t drop_tagged
 * @param[in] sai_uint16_t tpid
 * @param[in] sai_uint8_t default_vlan_priority
 * @param[in] shell_string *label
 * @param[in] uint8_t drop_untagged
 * @param[in] sai_uint32_t system_port_aggregate_id
 * @return Object_ID
 */
sai_uint32_t sai_create_lag_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t egress_acl,
    IN sai_uint32_t ingress_acl,
    IN sai_uint16_t port_vlan_id,
    IN uint8_t drop_tagged,
    IN sai_uint16_t tpid,
    IN sai_uint8_t default_vlan_priority,
    IN shell_string *label,
    IN uint8_t drop_untagged,
    IN sai_uint32_t system_port_aggregate_id);

/**
 * @brief Set Lag attributes
 *
 * @param[in] sai_uint32_t acl_table_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_lag_attribute_wrp(
    IN sai_uint32_t lag_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get Lag attributes
 *
 * @param[in] sai_uint32_t lag_id
 * @param[in] sai_lag_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_lag_attribute_wrp(
    IN sai_uint32_t lag_id,
    IN sai_lag_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief Remove Lag
 *
 * @param[in] sai_uint32_t lag_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_lag_wrp(
    IN sai_uint32_t lag_id);

/**
 * @brief Create Lag member default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t port_id
 * @param[in] sai_uint32_t lag_id
 * @return Object_ID
 */
sai_uint32_t sai_create_lag_member_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t port_id,
    IN sai_uint32_t lag_id);

/**
 * @brief Create Lag member custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] uint8_t egress_disable
 * @param[in] uint8_t ingress_disable
 * @param[in] sai_uint32_t port_id
 * @param[in] sai_uint32_t lag_id
 * @return Object_ID
 */
sai_uint32_t sai_create_lag_member_custom_wrp(
    IN sai_uint32_t switch_id,
    IN uint8_t egress_disable,
    IN uint8_t ingress_disable,
    IN sai_uint32_t port_id,
    IN sai_uint32_t lag_id);

/**
 * @brief Set Lag member attributes
 *
 * @param[in] sai_uint32_t lag_member_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_lag_member_attribute_wrp(
    IN sai_uint32_t lag_member_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get Lag member attributes
 *
 * @param[in] sai_uint32_t lag_member_id
 * @param[in] sai_lag_member_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_lag_member_attribute_wrp(
    IN sai_uint32_t lag_member_id,
    IN sai_lag_member_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief Remove acl table
 *
 * @param[in] sai_uint32_t lag_member_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_lag_member_wrp(
    IN sai_uint32_t lag_member_id);

#ifdef __cplusplus
}
#endif

#endif /* _luaSaiLag_H */
