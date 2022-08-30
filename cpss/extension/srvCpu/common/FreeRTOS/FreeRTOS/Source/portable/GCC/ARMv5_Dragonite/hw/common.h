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

#undef ENABLE_DTCM
#undef ENABLE_ITCM
#undef ENABLE_WB
#undef ENABLE_BPU

#define INTER_REGS_BASE		( 0xFFFFFF00 )
#define RUNIT_REGS_BASE		( 0x08000000 )
#define DFEV_REGS_BASE		( 0x10000000 )
#define MV_CPU_LE

#define MMU_SECTION_SHIFT	( 20 )
#define MMU_PTE_COUNT		( 4096 )
#define MMU_PTE_SIZE		( MMU_PTE_COUNT * 4 )
#define MMU_ORDERED_PT		( 0xF2000000 )
#define PTE_B				( 1 << 2 )
#define PTE_C				( 1 << 3 )
#define PTE_TEX0			( 1 << 12 )
#define PTE_INVALID			( 1 << 31 )
#define PTE_ORDERED			( 0 )
#define PTE_WB				( PTE_C | PTE_B )
#define PTE_WBWA			( PTE_WB | PTE_TEX0 )
#define PTE_MEMORY			( PTE_WBWA )
#define CPU_SVC_STACK_SIZE	( 0x300 )
#define CPU_IRQ_STACK_SIZE	( 0x300 )
#define CPU_FIQ_STACK_SIZE	( 0x300 )
#define CPU_ABT_STACK_SIZE	( 0x100 )
#define CPU_UND_STACK_SIZE	( 0x100 )
#define CPU_SUM_STACK_SIZE	( CPU_SVC_STACK_SIZE + CPU_IRQ_STACK_SIZE + \
							  CPU_ABT_STACK_SIZE + CPU_UND_STACK_SIZE + \
							  CPU_FIQ_STACK_SIZE )
#define SZ_1M				( 0x100000 )
#define SZ_1K				( 0x400 )

#ifndef __ASSEMBLY__
#define COMMON_H

#include <stdlib.h>
#include <FreeRTOS.h>
#include "errno.h"
#include "mvCommon.h"

extern unsigned long uCpuId;
extern unsigned long uBootParams;
extern unsigned long uPageTable;
extern unsigned long uKernelEnd;

#define CPUID				( uCpuId )
#define CONFIG_SYS_TCLK		( 200000000 )

#undef CONFIG_MV_AMP_ENABLE
#define CONFIG_MARVELL 1

#define NR_CPUS 4
#define MV_BOARD_REFCLK_25MHZ		( 25000000 )

/* Function wrappers */
#define whoAmI()				CPUID
#define master_cpu_id			CPUID	/* XXX only single core supported */
#define mvOsMemset( p, v, s )	memset(( void * )( p ), ( long )v, ( size_t )s )
#define udelay( us )			UDELAY( us )
#define mvOsPrintf				printf

/* Register offsets */
#define MV_MBUS_REGS_OFFSET			( 0x20000 )
#define MV_CPUIF_LOCAL_REGS_OFFSET	( MV_CPUIF_REGS_OFFSET( CPUID ))
#define MV_CPUIF_REGS_OFFSET( cpu )	( 0x21800 + ( cpu ) * 0x100 )
#define MV_CPUIF_REGS_BASE( cpu )	( MV_CPUIF_REGS_OFFSET( cpu ))
#define MV_CNTMR_REGS_OFFSET		( 0x20300 )
#define MV_L2_REGS_OFFSET			( 0x8000 )
#define MV_SCU_REGS_OFFSET			( 0xC000 )
#define MV_COHERENCY_FABRIC_OFFSET	( 0x20200 )

/* UART register base */
#define MV_UART_REGS_BASE(x)		(RUNIT_REGS_BASE + 0x12000 + (0x100 * x))

/* Common definitions */
typedef unsigned char			MV_U8;
typedef unsigned short			MV_U16;
typedef unsigned long			MV_U32;
typedef long					MV_32;
typedef void					MV_VOID;
typedef long					MV_BOOL;
typedef long					MV_STATUS;
typedef long					bool;

/* Register access macros */
#define AXPREG( reg )					\
			( *(( volatile unsigned long * )( INTER_REGS_BASE | ( reg ))))
#define AXPREG_BYTE( reg )				\
			( *(( volatile unsigned char * )( INTER_REGS_BASE | ( reg ))))

#define MV_REG_BYTE_WRITE( offset, val )		\
						( AXPREG_BYTE( offset ) = ( val ))
#define MV_REG_BYTE_READ( offset )				\
						( AXPREG_BYTE( offset ))

#define MV_REG_WRITE( offset, val )		\
						( AXPREG( offset ) = ( val ))
#define MV_REG_READ( offset )			\
						( AXPREG( offset ))

#define MV_REG_BIT_SET( reg, bit )		\
						AXPREG( reg ) = AXPREG( reg ) | ( bit )
#define MV_REG_BIT_RESET( reg, bit )	\
						AXPREG( reg ) = AXPREG( reg ) & ~( bit )


#define RUNITREG( reg )						\
			( *(( volatile unsigned long * )( RUNIT_REGS_BASE | ( reg ))))

#define RUNIT_REG_WRITE( offset, val )		\
						( RUNITREG( offset ) = ( val ))
#define RUNIT_REG_READ( offset )			\
						( RUNITREG( offset ))

#define RUNIT_REG_BIT_SET( reg, mask )		\
						RUNITREG( reg ) = RUNITREG( reg ) | ( mask )
#define RUNIT_REG_BIT_RESET( reg, mask )	\
						RUNITREG( reg ) = RUNITREG( reg ) & ~( mask )

#define DFEVREG( reg )				\
			( *(( volatile unsigned long * )( DFEV_REGS_BASE | ( reg ))))

#define DFEV_REG_WRITE( offset, val )		\
						( DFEVREG( offset ) = ( val ))
#define DFEV_REG_READ( offset )			\
						( DFEVREG( offset ))

#define DFEV_REG_BIT_SET( reg, mask )		\
						DFEVREG( reg ) = DFEVREG( reg ) | ( mask )
#define DFEV_REG_BIT_RESET( reg, mask )		\
						DFEVREG( reg ) = DFEVREG( reg ) & ~( mask )

/* Basic delay macros */
#define MDELAY( ms )	do {										\
							long j = ( ms );						\
							for ( ; j > 0; j-- )					\
								UDELAY( 1000 );						\
						} while ( 0 )

#define UDELAY( us )	do {											\
							unsigned long i;							\
							i = configCPU_CLOCK_HZ / 4000000 * us;		\
							asm volatile(	"1:		\n\t"				\
											"NOP				\n\t"	\
											"SUB	%0, %0, #1	\n\t"	\
											"CMP	%0, #0		\n\t"	\
											"BNE	1b			\n\t"	\
											: "+r" ( i ) : );			\
						} while ( 0 )

/* Common ASM functions */
unsigned long arm_read_cpsr( void );
void arm_write_cpsr( unsigned long );
void arm_flush_wb( void );

#ifdef THUMB_INTERWORK
unsigned long cpu_read_cpsr( void );
void cpu_write_cpsr( unsigned long );
void cpu_flush_wb( void );
#else
#define cpu_read_cpsr()			arm_read_cpsr()
#define cpu_write_cpsr( v )		arm_write_cpsr( v )
#define cpu_flush_wb()			arm_flush_wb()
#endif

#define CPSR_MODE_MASK			( 0x1f )
#define CPSR_MODE_USR			( 0x10 )
#define CPSR_MODE_FIQ			( 0x11 )
#define CPSR_MODE_IRQ			( 0x12 )
#define CPSR_MODE_SVC			( 0x13 )
#define CPSR_MODE_ABT			( 0x17 )
#define CPSR_MODE_UND			( 0x1b )
#define CPSR_MODE_SYS			( 0x1f )

/* DMA memory allocator */
void *pvPortMallocDMA( size_t xWantedSize, size_t xAlignment, unsigned long ulPTEFlags );

#endif
#endif
