#include "saiShellPort.h"
#include "include/luaSaiUtil.h"

#define XDK_SAI_OBJID_VALUE_BITS 48
#define XDK_SAI_OBJID_SWITCH_BITS 40

sai_uint64_t sai_get64bit_oid(sai_object_id_t local_32bit_id,
                              int sai_object_type)
{
    sai_uint64_t type_tmp = SAI_NULL_OBJECT_ID;
    sai_uint64_t switch_tmp = SAI_NULL_OBJECT_ID;
    sai_uint64_t sai_object_id;
    sai_uint64_t dev_id =0;
    type_tmp = (sai_uint64_t) sai_object_type << XDK_SAI_OBJID_VALUE_BITS;
    switch_tmp = (sai_uint64_t) dev_id << XDK_SAI_OBJID_SWITCH_BITS;
    sai_object_id = type_tmp | switch_tmp | local_32bit_id;
    return sai_object_id;
}
