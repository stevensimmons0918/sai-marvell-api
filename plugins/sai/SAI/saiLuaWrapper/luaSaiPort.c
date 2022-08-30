#include "saiShellPort.h"
#include "include/luaSaiPort.h"
#include "include/luaSaiUtil.h"
#include "saitypes.h"

uint32_t luaSaiPortAttrtGet()
{
    sai_get_port_attribute(0x111111, SAI_PORT_ATTR_ADMIN_STATE, 0, 0);
    return 0;
}

sai_uint32_t sai_create_port_default_wrp(
    IN sai_uint32_t switch_id,
    IN uint8_t hw_lane_count,
    IN sai_uint32_t hw_lane_list_ptr[],
    IN uint8_t full_duplex_mode,
    IN sai_uint32_t speed)
{
    int itr = 0;
    sai_uint64_t switch_id_acc;
    sai_uint64_t portOid;
    switch_id_acc = sai_get64bit_oid(switch_id, SAI_OBJECT_TYPE_SWITCH);
    sai_u32_list_t hw_lane_list_acc;
    uint32_t *list;
    list = (uint32_t*) calloc(hw_lane_count, sizeof(uint32_t));
    if (list == SAI_NULL_OBJECT_ID)
    {
        return 0;
    }
    for (itr = 0; itr < hw_lane_count; itr++)
    {
        list[itr] = hw_lane_list_ptr[itr];
    }
    hw_lane_list_acc.count = hw_lane_count;
    hw_lane_list_acc.list = list;

    portOid = sai_create_port_default(switch_id_acc, &hw_lane_list_acc,
                                      full_duplex_mode, speed);
    free(list);
    return (portOid&0xFFFFFFFF);
}

sai_status_t sai_get_port_attribute_wrp(
    IN sai_uint32_t port_id,
    IN sai_port_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr)
{
    sai_uint64_t port_oid_acc = sai_get64bit_oid(port_id, SAI_OBJECT_TYPE_PORT);
    sai_status_t ret;
    ret = sai_get_port_attribute(port_oid_acc, attr_id, list_count, out_attr);
    return ret;
}

sai_status_t sai_set_port_attribute_wrp(
    IN sai_uint32_t port_id,
    IN uint32_t attr_id,
    IN sai_attribute_value_t value,
    IN int ipaddr_flag,
    IN sai_ip_address_t *ip_addr)
{
    sai_uint64_t port_oid_acc = sai_get64bit_oid(port_id, SAI_OBJECT_TYPE_PORT);
    sai_status_t ret;
    ret = sai_set_port_attribute(port_oid_acc, attr_id, value, ipaddr_flag,
                                 ip_addr);
    return ret;
}
