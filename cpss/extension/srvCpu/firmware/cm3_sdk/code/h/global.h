

#ifndef GLOBAL_H
#define GLOBAL_H

#include <common.h>
#include <cpss/generic/ipc/mvShmIpc.h>

/* MG base address */
#define MG_BASE             0x60000000

/* MG offsets */
#define REG_MBUS_SYSTEM_INT_WINDOW_CONFIG   0x334

#define CM3_SRAM_SIZE       0x20000

#define SC_REMAP_DEF_VAL    0xE

#define CM3_WIN(i) (MG_BASE | (0x480 + i*0x10))

/* CM3 external timer registers */
#define CM3_EXT_TIMER_RATIO   0x34c
#define CM3_EXT_TIMER_VAL0    0x360
#define CM3_EXT_TIMER_VAL1    0x364
#define CM3_EXT_TIMER_START   0x370
#define CM3_EXT_TIMER_CONTROL 0x374


#define DFX_UNIT_ID    8

/* Define single bit masks.             */
#define BIT_0           (0x1)
#define BIT_1           (0x2)
#define BIT_2           (0x4)
#define BIT_3           (0x8)
#define BIT_4           (0x10)
#define BIT_5           (0x20)
#define BIT_6           (0x40)
#define BIT_7           (0x80)
#define BIT_8           (0x100)
#define BIT_9           (0x200)
#define BIT_10          (0x400)
#define BIT_11          (0x800)
#define BIT_12          (0x1000)
#define BIT_13          (0x2000)
#define BIT_14          (0x4000)
#define BIT_15          (0x8000)
#define BIT_16          (0x10000)
#define BIT_17          (0x20000)
#define BIT_18          (0x40000)
#define BIT_19          (0x80000)
#define BIT_20          (0x100000)
#define BIT_21          (0x200000)
#define BIT_22          (0x400000)
#define BIT_23          (0x800000)
#define BIT_24          (0x1000000)
#define BIT_25          (0x2000000)
#define BIT_26          (0x4000000)
#define BIT_27          (0x8000000)
#define BIT_28          (0x10000000)
#define BIT_29          (0x20000000)
#define BIT_30          (0x40000000)
#define BIT_31          (0x80000000)

/* check NULL pointer */
#define NULL_PTR_CHECK_MAC(ptr)    if(ptr == NULL) { \
    printf("%s: %s is NULL pointer\n", __FUNCTION__, #ptr); \
    return MV_BAD_PTR; \
}


#define MV_MEMIO16_WRITE(addr, data)   \
    (*((volatile unsigned short*)(addr)) = (unsigned short)(data) )

#define MV_MEMIO16_READ(addr)          \
    ((*((volatile unsigned short*)(addr))))

#define MV_MEMIO8_WRITE(addr, data)    \
    ((*((volatile unsigned char*)(addr))) = (unsigned char)(data) )

#define MV_MEMIO8_READ(addr)           \
    ((*((volatile unsigned char*)(addr))))


extern IPC_SHM_STC directShm;
void srvCpuIpcInit(void* ipcBase); /* include IPC to main, this function is used to intial IPC */

#endif


