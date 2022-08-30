#include "saiShellSwitch.h"
#include "include/luaSaiSwitch.h"
#include "include/luaSaiUtil.h"
#include "saitypes.h"


uint32_t luaSaiSwitchAttrGet()
{
    sai_get_switch_attribute(0x111111, SAI_SWITCH_ATTR_TYPE, 0, 0);
    return 0;
}

sai_uint32_t sai_get_switch_attribute_wrp(
    IN sai_uint32_t switch_id,
    IN sai_switch_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr)
{
    sai_uint64_t switch_oid_acc = sai_get64bit_oid(switch_id,
                                                   SAI_OBJECT_TYPE_SWITCH);
    sai_status_t ret;
    ret = sai_get_switch_attribute(switch_oid_acc, attr_id, list_count, out_attr);
    return ret;
}
