
#ifndef COMMON_H
#define COMMON_H

/* Common definitions */
typedef unsigned char           MV_U8;
typedef unsigned short          MV_U16;
/*typedef unsigned long         MV_U32;*/
typedef unsigned int            MV_U32;
typedef long                    MV_32;
typedef void                    MV_VOID;
typedef long                    MV_BOOL;
typedef long                    MV_STATUS;

#ifndef NULL
#define NULL ((void*)0)
#endif

#define MV_FALSE    0
#define MV_TRUE     (!(MV_FALSE))

#define WORD_SIZE 4

typedef unsigned int GT_STATUS;

#define GT_ERROR                 (-1)
#define GT_OK                    (0x00) /* Operation succeeded */
#define GT_FAIL                  (0x01) /* Operation failed    */

#endif
