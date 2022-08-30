/*******************************************************************************
   Copyright (C) Marvell International Ltd. and its affiliates
********************************************************************************
Marvell GNU General Public License FreeRTOS Exception

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the Lesser
General Public License Version 2.1 plus the following FreeRTOS exception.
An independent module is a module which is not derived from or based on
FreeRTOS.
Clause 1:
Linking FreeRTOS statically or dynamically with other modules is making a
combined work based on FreeRTOS. Thus, the terms and conditions of the GNU
General Public License cover the whole combination.
As a special exception, the copyright holder of FreeRTOS gives you permission
to link FreeRTOS with independent modules that communicate with FreeRTOS solely
through the FreeRTOS API interface, regardless of the license terms of these
independent modules, and to copy and distribute the resulting combined work
under terms of your choice, provided that:
1. Every copy of the combined work is accompanied by a written statement that
details to the recipient the version of FreeRTOS used and an offer by yourself
to provide the FreeRTOS source code (including any modifications you may have
made) should the recipient request it.
2. The combined work is not itself an RTOS, scheduler, kernel or related
product.
3. The independent modules add significant and primary functionality to
FreeRTOS and do not merely extend the existing functionality already present in
FreeRTOS.
Clause 2:
FreeRTOS may not be used for any competitive or comparative purpose, including
the publication of any form of run time or compile time metric, without the
express permission of Real Time Engineers Ltd. (this is the norm within the
industry and is intended to ensure information accuracy).
*******************************************************************************/

#ifndef COMMON_H

#define MV_CPU_LE

#define DEFAULT_DEV0_NUM    ( 0 )
#define DEFAULT_DEV1_NUM    ( 1 )

#define SZ_1M               ( 0x100000 )
#define SZ_1K               ( 0x400 )

#ifndef __ASSEMBLY__
#define COMMON_H

#include <stdlib.h>
#include <FreeRTOS.h>
#include "errno.h"
#include "mvCommon.h"

/* "Mbus clock" in terms of SoC department */
#if defined(CONFIG_BOBK)
#define CONFIG_SYS_TCLK     ( 200000000 )
#elif (defined (CONFIG_ALDRIN) || defined (CONFIG_BC3) || defined (CONFIG_PIPE)|| defined (CONFIG_ALDRIN2) || defined (CONFIG_FALCON))
#define CONFIG_SYS_TCLK     ( 250000000 )
#elif defined (CONFIG_RAVEN)
#define CONFIG_SYS_TCLK     ( 167000000 )
#elif defined (CONFIG_AC5)
#define CONFIG_SYS_TCLK     ( 328000000 )
#endif

/* TBD - must be fixed. Can be different in bobK, BC3, Aldrin, pipe */
#define MV_BOARD_REFCLK_25MHZ       ( 25000000 )

/* Function wrappers */
#define CPUID                   4
#define whoAmI()                CPUID
#define master_cpu_id           CPUID   /* XXX only single core supported */
#define mvOsMemset( p, v, s )   memset(( void * )( p ), ( long )v, ( size_t )s )
#define udelay( us )            UDELAY( us )
#define mvOsPrintf              printf
#define dmb()               __asm__ __volatile__ ("dmb" : : : "memory")

/* Register offsets */

/* Common definitions */
typedef signed char             int8_t;

typedef unsigned char           MV_U8;
typedef unsigned short          MV_U16;
typedef unsigned long           MV_U32;
typedef long                    MV_32;
typedef void                    MV_VOID;
/*typedef long                    MV_BOOL;*/
typedef unsigned char           MV_BOOL;

typedef long                    MV_STATUS;
typedef unsigned long           MV_UINTPTR_T;
/* The STM32 libraries already defines the bool type */
#if !defined(CONFIG_STM32) && !defined(CONFIG_MV_IPC_FREERTOS_DRIVER)
typedef long                    bool;
#endif

typedef enum { BC2_E, AC3_E, BOBK_E, ALDRIN_E} dev_type_ent;

#define MV_BOARD_TCLK           250000000

#define IN
#define OUT

#define WRITE_FLASH_BUFF_SIZE   0x100           //256 bytes


/* MG base address */
#define MG_BASE         0x60000000


/* Register access macros */
#define MSS_REGS_BASE 0xa0000000

#ifdef CONFIG_A8K
#define MSS_REGS_BASE 0x40500000
#endif

#define INTER_REGS_BASE MSS_REGS_BASE

#ifdef CONFIG_RAVEN
#define SD1_REGS_BASE           (INTER_REGS_BASE  + 0x00100000)
#define GOP0_REGS_BASE          (SD1_REGS_BASE    + 0x00100000)
#define GOP1_REGS_BASE          (GOP0_REGS_BASE   + 0x00100000)
#define SERDES_REGS_BASE        (GOP1_REGS_BASE   + 0x00100000)
#define DFX_REGS_BASE           (SERDES_REGS_BASE + 0x00100000)
#endif


#define MSSREG( reg )                       \
            ( *(( volatile unsigned long * )( MSS_REGS_BASE | ( reg ))))

#define MSS_REG_WRITE( offset, val )        \
                        ( MSSREG( offset ) = ( val ))
#define MSS_REG_READ( offset )          \
                        ( MSSREG( offset ))

#define MSS_REG_BIT_SET( reg, mask )        \
                        MSSREG( reg ) = MSSREG( reg ) | ( mask )
#define MSS_REG_BIT_RESET( reg, mask )  \
                        MSSREG( reg ) = MSSREG( reg ) & ~( mask )


#define MSS_REG_BYTE( reg )             \
    ( *(( volatile unsigned char * )( MSS_REGS_BASE | ( reg ))))

#define MV_REG_BYTE_WRITE( offset, val )        \
                        ( MSS_REG_BYTE( offset ) = ( val ))
#define MV_REG_BYTE_READ( offset )              \
                        ( MSS_REG_BYTE( offset ))

/* Basic delay macros */
#define MDELAY( ms )    do {                                        \
                            long j = ( ms );                        \
                            for ( ; j > 0; j-- )                    \
                                UDELAY( 1000 );                     \
                        } while ( 0 )

#define UDELAY( us )    do {                                            \
                            unsigned long i;                            \
                            i = configCPU_CLOCK_HZ / 4000000 * us;      \
                            asm volatile(   "1:     \n\t"               \
                                            "NOP                \n\t"   \
                                            "SUB    %0, %0, #1  \n\t"   \
                                            "CMP    %0, #0      \n\t"   \
                                            "BNE    1b          \n\t"   \
                                            : "+r" ( i ) : );           \
                        } while ( 0 )


#endif
#endif
