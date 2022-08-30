#include "saiShellBridge.h"
#include "include/luaSaiBridge.h"
#include "include/luaSaiUtil.h"
#include "saitypes.h"

sai_status_t sai_get_bridge_attribute_wrp(
    IN sai_uint32_t bridge_id,
    IN sai_bridge_attr_t attr_id,
    IN uint32_t list_count,
    OUT sai_attribute_t *out_attr)
{
    sai_uint64_t bridge_oid_acc = sai_get64bit_oid(bridge_id,
                                                   SAI_OBJECT_TYPE_BRIDGE);
    sai_status_t ret;
    ret = sai_get_bridge_attribute(bridge_oid_acc, attr_id, list_count, out_attr);
    return ret;
}
