#ifndef _luaSaiUtil_H
#define _luaSaiUtil_H

#ifdef __cplusplus
extern "C" {
#endif
#undef IN
#define IN

#undef OUT
#define OUT
uint32_t luaSaiPortAttrtGet();

/**
 * @brief Create port
 * convert 32 bit oid to 64 bit
*/


sai_uint64_t sai_get64bit_oid(sai_object_id_t local_32bit_id,
                              int sai_object_type);



#ifdef __cplusplus
}
#endif



#endif


