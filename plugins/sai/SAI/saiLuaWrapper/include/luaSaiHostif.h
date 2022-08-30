#ifndef _luaSaiHostif_H
#define _luaSaiHostif_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN

#undef OUT
#define OUT
/**
 * @brief Create hostif default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_hostif_type_t type
 * @param[in] shell_string *name
 * @param[in] sai_uint32_t obj_id
 * @param[in] shell_string *genetlink_mcgrp_name
 * @return Object_ID
 */
sai_uint32_t sai_create_hostif_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_hostif_type_t type,
    IN shell_string *name,
    IN sai_uint32_t obj_id,
    IN shell_string *genetlink_mcgrp_name);

/**
 * @brief Create hostif custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_hostif_vlan_tag_t vlan_tag
 * @param[in] sai_hostif_type_t type
 * @param[in] shell_string *name
 * @param[in] sai_uint32_t obj_id
 * @param[in] sai_uint32_t queue
 * @param[in] uint8_t oper_status
 * @param[in] shell_string *genetlink_mcgrp_name
 * @return Object_ID
 */
sai_uint32_t sai_create_hostif_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_hostif_vlan_tag_t vlan_tag,
    IN sai_hostif_type_t type,
    IN shell_string *name,
    IN sai_uint32_t obj_id,
    IN sai_uint32_t queue,
    IN uint8_t oper_status,
    IN shell_string *genetlink_mcgrp_name);

/**
 * @brief Set hostif attributes
 *
 * @param[in] sai_uint32_t hostif_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_hostif_attribute_wrp(
    IN sai_uint32_t hostif_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);
/**
 * @brief Get hostif attributes
 *
 * @param[in] sai_uint32_t hostif_id
 * @param[in] sai_hostif_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_hostif_attribute_wrp(
    IN sai_uint32_t hostif_id,
    IN sai_hostif_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief Remove hostif
 *
 * @param[in] sai_uint32_t hostif_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_hostif_wrp(
    IN sai_uint32_t hostif_id);

/**
 * @brief Create hostif trap group default
 *
 * @param[in] sai_uint32_t switch_id
 * @return Object_ID
 */
sai_uint32_t sai_create_hostif_trap_group_default_wrp(
    IN sai_uint32_t switch_id);

/**
 * @brief Create hostif trap group custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t queue
 * @param[in] sai_uint32_t policer
 * @param[in] uint8_t admin_state
 * @return Object_ID
 */
sai_uint32_t sai_create_hostif_trap_group_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t queue,
    IN sai_uint32_t policer,
    IN uint8_t admin_state);

/**
 * @brief Set hostif trap group attributes
 *
 * @param[in] sai_uint32_t hostif_trap_group_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_hostif_trap_group_attribute_wrp(
    IN sai_uint32_t hostif_trap_group_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    int ipaddr_flag,
    sai_ip_address_t *ip_addr);

/**
 * @brief Get hostif trap group attributes
 *
 * @param[in] sai_uint32_t hostif_trap_group_id
 * @param[in] sai_hostif_trap_group_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_hostif_trap_group_attribute_wrp(
    IN sai_uint32_t hostif_trap_group_id,
    IN sai_hostif_trap_group_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief Remove hostif trap group
 *
 * @param[in] sai_uint32_t hostif_trap_group_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_hostif_trap_group_wrp(
    IN sai_uint32_t hostif_trap_group_id);

/**
 * @brief Create hostif user defined trap default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_hostif_user_defined_trap_type_t type
 * @return Object_ID
 */
sai_uint32_t sai_create_hostif_user_defined_trap_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_hostif_user_defined_trap_type_t type);

/**
 * @brief Create hostif user defined trap custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t trap_group
 * @param[in] sai_uint32_t trap_priority
 * @param[in] sai_hostif_user_defined_trap_type_t type
 * @return Object_ID
 */
sai_uint32_t sai_create_hostif_user_defined_trap_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t trap_group,
    IN sai_uint32_t trap_priority,
    IN sai_hostif_user_defined_trap_type_t type);

/**
 * @brief Set hostif user defined trap attributes
 *
 * @param[in] sai_uint32_t hostif_user_defined_trap_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_hostif_user_defined_trap_attribute_wrp(
    IN sai_uint32_t hostif_user_defined_trap_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get hostif user defined trap attributes
 *
 * @param[in] sai_uint32_t hostif_user_defined_trap_id
 * @param[in] sai_hostif_user_defined_trap_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_hostif_user_defined_trap_attribute_wrp(
    IN sai_uint32_t hostif_user_defined_trap_id,
    IN sai_hostif_user_defined_trap_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief Remove hostif user defined trap
 *
 * @param[in] sai_uint32_t hostif_user_defined_trap_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_hostif_user_defined_trap_wrp(
    IN sai_uint32_t hostif_user_defined_trap_id);

/**
 * @brief Create hostif trap default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_hostif_trap_type_t trap_type
 * @param[in] sai_packet_action_t packet_action
 * @return Object_ID
 */
sai_uint32_t sai_create_hostif_trap_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_hostif_trap_type_t trap_type,
    IN sai_packet_action_t packet_action);

/**
 * @brief Create hostif trap custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_hostif_trap_type_t trap_type
 * @param[in] sai_uint32_t trap_priority
 * @param[in] sai_uint32_t counter_id
 * @param[in] sai_packet_action_t packet_action
 * @param[in] sai_uint32_t trap_group
 * @return Object_ID
 */
sai_uint32_t sai_create_hostif_trap_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_hostif_trap_type_t trap_type,
    IN sai_uint32_t trap_priority,
    IN sai_uint32_t counter_id,
    IN sai_packet_action_t packet_action,
    IN sai_uint32_t trap_group);

/**
 * @brief Get hostif trap attribute
 *
 * @param[in] sai_uint32_t hostif_trap_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_hostif_trap_attribute_wrp(
    IN sai_uint32_t hostif_trap_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get hostif trap attribute
 *
 * @param[in] sai_uint32_t hostif_trap_id
 * @param[in] sai_hostif_trap_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_hostif_trap_attribute_wrp(
    IN sai_uint32_t hostif_trap_id,
    IN sai_hostif_trap_attr_t attr_id,
    uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief Remove hostif trap
 *
 * @param[in] sai_uint32_t hostif_trap_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_hostif_trap_wrp(
    IN sai_uint32_t hostif_trap_id);

/**
 * @brief Create hostif table entry default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t obj_id
 * @param[in] sai_hostif_table_entry_type_t type
 * @param[in] sai_uint32_t trap_id
 * @param[in] sai_hostif_table_entry_channel_type_t channel_type
 * @param[in] sai_uint32_t host_if
 * @return Object_ID
 */
sai_uint32_t sai_create_hostif_table_entry_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t obj_id,
    IN sai_hostif_table_entry_type_t type,
    IN sai_uint32_t trap_id,
    IN sai_hostif_table_entry_channel_type_t channel_type,
    IN sai_uint32_t host_if);

/**
 * @brief Create hostif table entry custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t obj_id
 * @param[in] sai_hostif_table_entry_type_t type
 * @param[in] sai_uint32_t trap_id
 * @param[in] sai_hostif_table_entry_channel_type_t channel_type
 * @param[in] sai_uint32_t host_if
 * @return Object_ID
 */
sai_uint32_t sai_create_hostif_table_entry_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t obj_id,
    IN sai_hostif_table_entry_type_t type,
    IN sai_uint32_t trap_id,
    IN sai_hostif_table_entry_channel_type_t channel_type,
    IN sai_uint32_t host_if);

/**
 * @brief Set hostif table entry
 *
 * @param[in] sai_uint32_t hostif_table_entry_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_hostif_table_entry_attribute_wrp(
    IN sai_uint32_t hostif_table_entry_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get hostif table entry
 *
 * @param[in] sai_uint32_t hostif_table_entry_id
 * @param[in] sai_hostif_table_entry_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_hostif_table_entry_attribute_wrp(
    IN sai_uint32_t hostif_table_entry_id,
    IN sai_hostif_table_entry_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief Remove hostif table entry
 *
 * @param[in] sai_uint32_t hostif_table_entry_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_hostif_table_entry_wrp(
    IN sai_uint32_t hostif_table_entry_id);

#ifdef __cplusplus
}
#endif

#endif /* _luaSaiHostif_H */
