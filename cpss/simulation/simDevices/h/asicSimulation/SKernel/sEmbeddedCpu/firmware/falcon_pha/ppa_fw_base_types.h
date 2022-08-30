/*
 *  ppa_fw_base_types.h
 *
 *  PPA fw type definitions
 *
 */

#ifndef __ppa_fw_base_types_h
#define __ppa_fw_base_types_h


typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef short              int16_t;
typedef unsigned short     uint16_t;
typedef int                int32_t;
typedef unsigned int       uint32_t;
typedef unsigned long      u32;
/* only stores 0/1 */
#define bool _Bool
#ifndef NULL
#define NULL (0)
#endif

#endif /*__ppa_fw_base_types_h*/
