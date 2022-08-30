#ifndef _luaSaiAcl_H
#define _luaSaiAcl_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN
#undef OUT
#define OUT
/**
 * @brief Create acl table group member default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t acl_table_id
 * @param[in] sai_uint32_t acl_table_group_id
 * @param[in] sai_uint32_t priority
 * @return Object_ID
 */
sai_uint32_t sai_create_acl_table_group_member_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t acl_table_id,
    IN sai_uint32_t acl_table_group_id,
    IN sai_uint32_t priority);

/**
 * @brief Create acl table group member custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t acl_table_id
 * @param[in] sai_uint32_t acl_table_group_id
 * @param[in] sai_uint32_t priority
 * @return Object_ID
 */
sai_uint32_t sai_create_acl_table_group_member_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t acl_table_id,
    IN sai_uint32_t acl_table_group_id,
    IN sai_uint32_t priority);

/**
 * @brief Set acl table group member attribute
 *
 * @param[in] sai_uint32_t acl_table_group_member_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_acl_table_attr_t value
 * @param[in] sai_ip_address_t *ip_addr
 * @return Object_ID
 */
sai_uint32_t sai_set_acl_table_group_member_attribute_wrp(
    IN sai_uint32_t acl_table_group_member_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);
/**
 * @brief Get acl table group member attribute
 *
 * @param[in] sai_uint32_t acl_table_group_member_id
 * @param[in] sai_acl_table_group_member_attr_t attr_id
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_acl_table_group_member_attribute_wrp(
    IN sai_uint32_t acl_table_group_member_id,
    IN sai_acl_table_group_member_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);
/**
 * @brief Get acl table group member remove
 *
 * @param[in] sai_uint32_t acl_table_group_member_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_acl_table_group_member_wrp(
    IN sai_uint32_t acl_table_group_member_id);

/**
 * @brief Create acl range default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_acl_range_type_t type
 * @param[in] sai_u32_range_t *limit
 * @return Object_ID
 */
sai_uint32_t sai_create_acl_range_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_acl_range_type_t type,
    IN sai_u32_range_t *limit);
/**
 * @brief Create acl range custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_acl_range_type_t type
 * @param[in] sai_u32_range_t *limit
 * @return Object_ID
 */
sai_uint32_t sai_create_acl_range_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_acl_range_type_t type,
    IN sai_u32_range_t *limit);

/**
 * @brief Set acl range attribute
 *
 * @param[in] sai_uint32_t acl_range_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_acl_range_attribute_wrp(
    IN sai_uint32_t acl_range_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);
/**
 * @brief Get acl range attribute
 *
 * @param[in] sai_uint32_t acl_range_id
 * @param[in] sai_acl_range_attr_t value
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_acl_range_attribute_wrp(
    IN sai_uint32_t acl_range_id,
    IN sai_acl_range_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);
/**
 * @brief acl range remove
 *
 * @param[in] sai_uint32_t acl_range_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_acl_range_wrp(
    IN sai_uint32_t acl_range_id);

/**
 * @brief Create acl table group default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_acl_stage_t acl_stage
 * @param[in] sai_s32_list_t acl_bind_point_type_list_ptr
 * @param[in] sai_acl_table_group_type_t type
 * @return Object_ID
 */
sai_uint32_t sai_create_acl_table_group_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_acl_stage_t acl_stage,
    IN sai_s32_list_t *acl_bind_point_type_list_ptr,
    IN sai_acl_table_group_type_t type);
/**
 * @brief Create acl table group custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_acl_stage_t acl_stage
 * @param[in] sai_s32_list_t acl_bind_point_type_list_ptr
 * @param[in] sai_acl_table_group_type_t type
 * @return Object_ID
 */
sai_uint32_t sai_create_acl_table_group_custom_wrp(
    IN sai_uint32_t switch_id,
    IN sai_acl_stage_t acl_stage,
    IN sai_s32_list_t *acl_bind_point_type_list_ptr,
    IN sai_acl_table_group_type_t type);

/**
 * @brief Set acl table group attribute
 *
 * @param[in] sai_uint32_t acl_table_group_id
 * @param[in] uint32_t attr_id
 * @param[in] sai_attribute_value_t value
 * @param[in] int ipaddr_flag
 * @param[in] sai_ip_address_t *ip_addr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_set_acl_table_group_attribute_wrp(
    IN sai_uint32_t acl_table_group_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr);
/**
 * @brief Get acl table group attribute
 *
 * @param[in] sai_uint32_t acl_table_group_id
 * @param[in] sai_acl_table_group_attr_t value
 * @param[in] uint32_t list_count
 * @param[in] sai_attribute_t *out_attr
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_get_acl_table_group_attribute_wrp(
    IN sai_uint32_t acl_table_group_id,
    IN sai_acl_table_group_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr);
/**
 * @brief Remove acl table group
 *
 * @param[in] sai_uint32_t acl_table_group_id
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_remove_acl_table_group_wrp(
    IN sai_uint32_t acl_table_group_id);

/**
 * @brief Create acl entry default
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t table_id
 * @param[in] sai_uint32_t priority
 * @return Object_ID
 */
sai_uint32_t sai_create_acl_entry_default_wrp(
    IN sai_uint32_t switch_id,
    IN sai_uint32_t table_id,
    IN sai_uint32_t priority);

#if 0 /* TBD - check if this function is needed */
/**
 * @brief Create acl entry custom
 *
 * @param[in] sai_uint32_t switch_id
 * @param[in] sai_uint32_t table_id
 * @param[in] sai_uint32_t priority
 * @return Object_ID
 */

sai_uint32_t sai_create_acl_entry_custom_wrp(
#endif /* if 0 */

    /**
     * @brief Set acl entry attribute
     *
     * @param[in] sai_uint32_t acl_entry_id
     * @param[in] uint32_t attr_id
     * @param[in] sai_attribute_value_t value
     * @param[in] int aclactionflag
     * @param[in] shell_acl_field_t *aclfield
     * @param[in] int aclactionflag
     * @param[in] shell_acl_action_t *aclaction
     * @return #SAI_STATUS_SUCCESS on success, failure status code on error
     */
    sai_status_t sai_set_acl_entry_attribute_wrp(
        IN sai_uint32_t acl_entry_id,
        IN uint32_t attr_id,
        IN sai_attribute_value_t value,
        IN int aclfieldflag,
        IN shell_acl_field_t *aclfield,
        IN int aclactionflag,
        IN shell_acl_action_t *aclaction);

    /**
     * @brief Get acl entry attribute
     *
     * @param[in] sai_uint32_t acl_entry_id
     * @param[in] sai_acl_entry_attr_t attr_id
     * @param[in] uint32_t list_count
     * @param[in] sai_attribute_t *out_attr
     * @return #SAI_STATUS_SUCCESS on success, failure status code on error
     */
    sai_status_t sai_get_acl_entry_attribute_wrp(
        IN sai_uint32_t acl_entry_id,
        IN sai_acl_entry_attr_t attr_id,
        IN uint32_t list_count,
        OUT sai_attribute_t *out_attr);
    /**
     * @brief Remove acl entry
     *
     * @param[in] sai_uint32_t acl_entry_id
     * @return #SAI_STATUS_SUCCESS on success, failure status code on error
     */
    sai_status_t sai_remove_acl_entry_wrp(
        IN sai_uint32_t acl_entry_id);

    /**
     * @brief Create acl counter default
     *
     * @param[in] sai_uint32_t switch_id
     * @param[in] sai_uint32_t table_id
     * @param[in] uint8_t enable_byte_count
     * @param[in] uint8_t enable_packet_count
     * @return Object_ID
     */
    sai_uint32_t sai_create_acl_counter_default_wrp(
        IN sai_uint32_t switch_id,
        IN sai_uint32_t table_id,
        IN uint8_t enable_byte_count,
        IN uint8_t enable_packet_count);

    /**
     * @brief Create acl counter custom
     *
     * @param[in] sai_uint32_t switch_id
     * @param[in] sai_uint32_t table_id
     * @param[in] uint8_t packet_count
     * @param[in] uint32_t packet_list_ptr[]
     * @param[in] uint8_t byte_count
     * @param[in] uint32_t byte_list_ptr[]
     * @param[in] uint8_t enable_byte_count
     * @param[in] uint8_t enable_packet_count
     * @return Object_ID
     */
    sai_uint32_t sai_create_acl_counter_custom_wrp(
        IN sai_uint32_t switch_id,
        IN sai_uint32_t table_id,
        IN uint8_t packet_count,
        IN uint32_t packet_list_ptr[]
        IN uint8_t byte_count,
        IN uint32_t byte_list_ptr[]
        IN uint8_t enable_byte_count,
        IN uint8_t enable_packet_count);
    /**
     * @brief Set acl counter
     *
     * @param[in] sai_uint32_t acl_counter_id
     * @param[in] uint8_t attr_id
     * @param[in] sai_attribute_value_t value
     * @param[in] int ipaddr_flag
     * @param[in] sai_ip_address_t *ip_addr
     * @return #SAI_STATUS_SUCCESS on success, failure status code on error
     */
    sai_status_t sai_set_acl_counter_attribute_wrp(
        IN sai_uint32_t acl_counter_id,
        IN uint32_t attr_id,
        IN sai_attribute_value_t value,
        IN int ipaddr_flag,
        IN sai_ip_address_t *ip_addr);
    /**
     * @brief Get acl counter
     *
     * @param[in] sai_uint32_t acl_counter_id
     * @param[in] sai_acl_counter_attr_t attr_id
     * @param[in] uint32_t list_count
     * @param[in] sai_attribute_t *out_attr
     * @return #SAI_STATUS_SUCCESS on success, failure status code on error
     */
    sai_status_t sai_get_acl_counter_attribute_wrp(
        IN sai_uint32_t acl_counter_id,
        IN sai_acl_counter_attr_t attr_id,
        IN uint32_t list_count,
        OUT sai_attribute_t *out_attr);
    /**
     * @brief Remove acl counter
     *
     * @param[in] sai_uint32_t acl_counter_id
     * @return #SAI_STATUS_SUCCESS on success, failure status code on error
     */
    sai_status_t sai_remove_acl_counter_wrp(
        IN sai_uint32_t acl_counter_id);

    /**
     * @brief Create ACL table default
     *
     * @param[in] sai_uint32_t switch_id
     * @param[in] uint8_t field_out_port
     * @param[in] uint8_t field_inner_dst_ipv6
     * @param[in] uint8_t field_dst_ipv6
     * @param[in] sai_s32_list_t *field_acl_range_type_ptr
     * @param[in] uint8_t field_inner_src_ipv6
     * @param[in] uint8_t field_tos
     * @param[in] uint8_t field_ether_type
     * @param[in] sai_acl_stage_t acl_stage
     * @param[in] uint8_t field_src_ipv6_word0
     * @param[in] uint8_t field_ttl
     * @param[in] uint8_t field_acl_ip_type
     * @param[in] sai_s32_list_t *acl_action_type_list_ptr
     * @param[in] uint8_t field_tcp_flags
     * @param[in] uint8_t field_in_port
     * @param[in] uint8_t field_dscp
     * @param[in] uint8_t field_src_mac
     * @param[in] uint8_t field_in_ports
     * @param[in] uint8_t field_packet_vlan
     * @param[in] uint8_t field_ecn
     * @param[in] uint8_t field_dst_ip
     * @param[in] uint8_t field_gre_key
     * @param[in] uint8_t field_src_ipv6_word3
     * @param[in] uint8_t field_src_ipv6_word2
     * @param[in] uint8_t field_src_ipv6_word1
     * @param[in] uint8_t field_l4_dst_port
     * @param[in] uint8_t field_acl_ip_frag
     * @param[in] sai_uint32_t size
     * @param[in] uint8_t field_has_vlan_tag
     * @param[in] uint8_t field_dst_ipv6_word3
     * @param[in] uint8_t field_dst_ipv6_word0
     * @param[in] uint8_t field_dst_ipv6_word1
     * @param[in] uint8_t field_src_ipv6
     * @param[in] uint8_t field_dst_mac
     * @param[in] uint8_t field_tc
     * @param[in] uint8_t field_src_port
     * @param[in] uint8_t field_src_ip
     * @param[in] uint8_t field_ip_protocol
     * @param[in] sai_s32_list_t *acl_bind_point_type_list_ptr
     * @param[in] uint8_t field_l4_src_port
     * @param[in] uint8_t field_icmp_type
     * @param[in] uint8_t field_icmp_code
     * @param[in] uint8_t field_dst_ipv6_word2
     * @return Object_ID
     */
    sai_uint32_t sai_create_acl_table_default_wrp(
        IN sai_uint32_t switch_id,
        IN uint8_t field_out_port, xi
        IN uint8_t field_inner_dst_ipv6,
        IN uint8_t field_dst_ipv6,
        IN sai_s32_list_t *field_acl_range_type_ptr,
        IN uint8_t field_inner_src_ipv6,
        IN uint8_t field_tos,
        IN uint8_t field_ether_type,
        IN sai_acl_stage_t acl_stage,
        IN uint8_t field_src_ipv6_word0,
        IN uint8_t field_ttl,
        IN uint8_t field_acl_ip_type,
        IN sai_s32_list_t *acl_action_type_list_ptr,
        IN uint8_t field_tcp_flags,
        IN uint8_t field_in_port,
        IN uint8_t field_dscp,
        IN uint8_t field_src_mac,
        IN uint8_t field_in_ports,
        IN uint8_t field_packet_vlan,
        IN uint8_t field_ecn,
        IN uint8_t field_dst_ip,
        IN uint8_t field_gre_key,
        IN uint8_t field_src_ipv6_word3,
        IN uint8_t field_src_ipv6_word2,
        IN uint8_t field_src_ipv6_word1,
        IN uint8_t field_l4_dst_port,
        IN uint8_t field_acl_ip_frag,
        IN sai_uint32_t size,
        IN uint8_t field_has_vlan_tag,
        IN uint8_t field_dst_ipv6_word3,
        IN uint8_t field_dst_ipv6_word0,
        IN uint8_t field_dst_ipv6_word1,
        IN uint8_t field_src_ipv6,
        IN uint8_t field_dst_mac,
        IN uint8_t field_tc,
        IN uint8_t field_src_port,
        IN uint8_t field_src_ip,
        IN uint8_t field_ip_protocol,
        IN sai_s32_list_t *acl_bind_point_type_list_ptr,
        IN uint8_t field_l4_src_port,
        IN uint8_t field_icmp_type,
        IN uint8_t field_icmp_code,
        IN uint8_t field_dst_ipv6_word2);

    /**
     * @brief Create ACL table custom
     *
     * @param[in] sai_uint32_t switch_id
     * @param[in] uint8_t field_out_port
     * @param[in] uint8_t field_inner_dst_ipv6
     * @param[in] uint8_t field_dst_ipv6
     * @param[in] sai_s32_list_t *field_acl_range_type_ptr
     * @param[in] uint8_t field_inner_src_ipv6
     * @param[in] uint8_t field_tos
     * @param[in] uint8_t field_ether_type
     * @param[in] sai_acl_stage_t acl_stage
     * @param[in] uint8_t field_src_ipv6_word0
     * @param[in] uint8_t field_ttl
     * @param[in] uint8_t field_acl_ip_type
     * @param[in] sai_s32_list_t *acl_action_type_list_ptr
     * @param[in] uint8_t field_tcp_flags
     * @param[in] uint8_t field_in_port
     * @param[in] uint8_t field_dscp
     * @param[in] uint8_t field_src_mac
     * @param[in] uint8_t field_in_ports
     * @param[in] uint8_t field_packet_vlan
     * @param[in] uint8_t field_ecn
     * @param[in] uint8_t field_dst_ip
     * @param[in] uint8_t field_gre_key
     * @param[in] uint8_t field_src_ipv6_word3
     * @param[in] uint8_t field_src_ipv6_word2
     * @param[in] uint8_t field_src_ipv6_word1
     * @param[in] uint8_t field_l4_dst_port
     * @param[in] uint8_t field_acl_ip_frag
     * @param[in] sai_uint32_t size
     * @param[in] uint8_t field_has_vlan_tag
     * @param[in] uint8_t field_dst_ipv6_word3
     * @param[in] uint8_t field_dst_ipv6_word0
     * @param[in] uint8_t field_dst_ipv6_word1
     * @param[in] uint8_t field_src_ipv6
     * @param[in] uint8_t field_dst_mac
     * @param[in] uint8_t field_tc
     * @param[in] uint8_t field_src_port
     * @param[in] uint8_t field_src_ip
     * @param[in] uint8_t field_ip_protocol
     * @param[in] sai_s32_list_t *acl_bind_point_type_list_ptr
     * @param[in] uint8_t field_l4_src_port
     * @param[in] uint8_t field_icmp_type
     * @param[in] uint8_t field_icmp_code
     * @param[in] uint8_t field_dst_ipv6_word2
     * @return Object_ID
     */
    sai_uint32_t sai_create_acl_table_custom_wrp(
        IN sai_uint32_t switch_id,
        IN uint8_t field_out_port,
        IN uint8_t field_inner_dst_ipv6,
        IN uint8_t field_dst_ipv6,
        IN sai_s32_list_t *field_acl_range_type_ptr, i
        IN uint8_t field_inner_src_ipv6,
        IN uint8_t field_tos,
        IN uint8_t field_ether_type,
        IN sai_acl_stage_t acl_stage,
        IN uint8_t field_src_ipv6_word0,
        IN uint8_t field_ttl,
        IN uint8_t field_acl_ip_type,
        IN sai_s32_list_t *acl_action_type_list_ptr,
        IN uint8_t field_tcp_flags,
        IN uint8_t field_in_port,
        IN uint8_t field_dscp,
        IN uint8_t field_src_mac,
        IN uint8_t field_in_ports,
        IN uint8_t field_packet_vlan,
        IN uint8_t field_ecn,
        IN uint8_t field_dst_ip,
        IN uint8_t field_gre_key,
        IN uint8_t field_src_ipv6_word3,
        IN uint8_t field_src_ipv6_word2,
        IN uint8_t field_src_ipv6_word1,
        IN uint8_t field_l4_dst_port,
        IN uint8_t field_acl_ip_frag,
        IN sai_uint32_t size,
        IN uint8_t field_has_vlan_tag,
        IN uint8_t field_dst_ipv6_word3,
        IN uint8_t field_dst_ipv6_word0,
        IN uint8_t field_dst_ipv6_word1,
        IN uint8_t field_src_ipv6,
        IN uint8_t field_dst_mac,
        IN uint8_t field_tc,
        IN uint8_t field_src_port,
        IN uint8_t field_src_ip,
        IN uint8_t field_ip_protocol,
        IN sai_s32_list_t *acl_bind_point_type_list_ptr,
        IN uint8_t field_l4_src_port,
        IN uint8_t field_icmp_type,
        IN uint8_t field_icmp_code,
        IN uint8_t field_dst_ipv6_word2);

    /**
     * @brief Set acl table attributes
     *
     * @param[in] sai_uint32_t acl_table_id
     * @param[in] uint32_t attr_id
     * @param[in] sai_acl_table_attr_t value
     * @param[in] sai_ip_address_t *ip_addr
     * @return #SAI_STATUS_SUCCESS on success, failure status code on error
     */
    sai_status_t sai_set_acl_table_attribute_wrp(
        IN sai_uint32_t acl_table_id,
        IN uint32_t attr_id,
        IN sai_attribute_value_t value,
        IN int ipaddr_flag,
        IN sai_ip_address_t *ip_addr);

    /**
     * @brief Get acl table attributes
     *
     * @param[in] sai_uint32_t acl_table_id
     * @param[in] sai_acl_table_attr_t attr_id
     * @param[in] uint32_t list_count
     * @param[in] sai_attribute_t *out_attr
     * @return #SAI_STATUS_SUCCESS on success, failure status code on error
     */
    sai_status_t sai_get_acl_table_attribute_wrp(
        IN sai_uint32_t acl_table_id,
        IN sai_acl_table_attr_t attr_id,
        IN uint32_t list_count,
        OUT sai_attribute_t *out_attr);

    /**
     * @brief Remove acl table
     *
     * @param[in] sai_uint32_t acl_table_id
     * @return #SAI_STATUS_SUCCESS on success, failure status code on error
     */
    sai_status_t sai_remove_acl_table_wrp(
        IN sai_uint32_t acl_table_id);

#ifdef __cplusplus
}
#endif

#endif /* _luaSaiAcl_H */
