#ifndef _luaSaiFdb_H
#define _luaSaiFdb_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN

#undef OUT
#define OUT
/**
 * @brief Create FDB Entry default
 *
 * @param[in] const sai_fdb_entry_t *fdb_entry
 * @param[in] uint8_t allow_mac_move
 * @param[in] sai_packet_action_t packet_action
 * @param[in] sai_ip_address_t *addr_endpoint_ip
 * @param[in] sai_uint32_t bridge_port_id
 * @param[in] sai_fdb_entry_type_t type
 * @param[in] sai_uint32_t user_trap_id
 * @return Object_ID
 */
sai_uint32_t sai_create_fdb_entry_default_wrp(
    IN const sai_fdb_entry_t *fdb_entry,
    IN uint8_t allow_mac_move,
    IN sai_packet_action_t packet_action,
    IN sai_ip_address_t *addr_endpoint_ip,
    IN sai_uint32_t bridge_port_id,
    IN sai_fdb_entry_type_t type,
    IN sai_uint32_t user_trap_id);

/**
 * @brief Create FDB Entry custom
 *
 * @param[in] const sai_fdb_entry_t *fdb_entry
 * @param[in] uint8_t allow_mac_move
 * @param[in] sai_packet_action_t packet_action
 * @param[in] sai_ip_address_t *addr_endpoint_ip
 * @param[in] sai_uint32_t bridge_port_id
 * @param[in] sai_fdb_entry_type_t type
 * @param[in] sai_uint32_t user_trap_id
 * @return Object_ID
 */
sai_uint32_t sai_create_fdb_entry_custom_wrp(
    IN const sai_fdb_entry_t *fdb_entry,
    IN uint8_t allow_mac_move,
    IN sai_packet_action_t packet_action,
    IN sai_ip_address_t *addr_endpoint_ip,
    IN sai_uint32_t bridge_port_id,
    IN sai_fdb_entry_type_t type,
    IN sai_uint32_t user_trap_id);

/**
 * @brief Set FDB attributes
 *
 * @param[in] const sai_fdb_entry_t *fdb_entry
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_fdb_entry_attribute_wrp(
    IN const sai_fdb_entry_t *fdb_entry,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);

/**
 * @brief Get FDB attributes
 *
 * @param[in] const sai_fdb_entry_t *fdb_entry
 * @param[in] sai_fdb_entry_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_fdb_entry_attribute_wrp(
    IN const sai_fdb_entry_t *fdb_entry,
    IN sai_fdb_entry_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);

/**
 * @brief Remove FDB entry
 *
 * @param[in] const sai_fdb_entry_t *fdb_entry
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_fdb_entry_wrp(
    IN const sai_fdb_entry_t *fdb_entry);

/**
 * @brief Flush FDB entires
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] uint32_t attr_count
 * @param[in] sai_attribute_t *attr_list
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_flush_fdb_entries_wrp(
    IN sai_uint32_t switch_id,
    IN uint32_t attr_count,
    IN sai_attribute_t *attr_list);

#ifdef __cplusplus
}
#endif

#endif /* _luaSaiFdb_H */
