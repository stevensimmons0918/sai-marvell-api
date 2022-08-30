
#include "saiShellFdb.h"
#include "include/luaSaiFdb.h"
#include "include/luaSaiUtil.h"
#include "saitypes.h"

sai_status_t sai_flush_fdb_entries_wrp(
    IN sai_uint32_t switch_id,
    IN uint32_t attr_count,
    IN sai_attribute_t *attr_list)
{
    sai_uint64_t switch_oid_acc = sai_get64bit_oid(switch_id,
                                                   SAI_OBJECT_TYPE_SWITCH);
    sai_status_t ret;
    ret = sai_flush_fdb_entries(switch_oid_acc, attr_count, attr_list);
    return ret;
}
