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

#define ENABLE_DCACHE		( 1 )
#define ENABLE_ICACHE		( 1 )
#define ENABLE_L2			( 1 )
#define ENABLE_IOCC			( 1 )

#define INTER_REGS_BASE		( 0xF1000000 )
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

extern unsigned long uCpuId;
extern unsigned long uBootParams;
extern unsigned long uPageTable;
extern unsigned long uKernelEnd;
extern unsigned long ulCpuClk;
extern unsigned long ulTClk;
extern unsigned long ulL2Clk;

#define CONFIG_MV_AMP_ENABLE
#define MV88F78X60

#define CPUID					( uCpuId )
#define NR_CPUS					( 4 )
#define MV_BOARD_REFCLK_25MHZ	( 25000000 )

/* Function wrappers */
#define whoAmI()				CPUID
#define master_cpu_id			CPUID	/* XXX only single core supported */
#define mvOsMemset( p, v, s )	memset(( void * )( p ), ( long )v, ( size_t )s )
#define udelay( us )			UDELAY( us )
#define mvOsPrintf				printf

#ifdef CONFIG_SHEEVA_ERRATA_ARM_CPU_6075
#define dmb()					__asm__ __volatile__ ( "dsb" : : : "memory" )
#else
#define dmb()					__asm__ __volatile__ ( "dmb" : : : "memory" )
#endif
#define dsb()					__asm__ __volatile__ ( "dsb" : : : "memory" )

/* Register offsets */
#define MV_MBUS_REGS_OFFSET			( 0x20000 )
#define MV_CPUIF_LOCAL_REGS_OFFSET	( MV_CPUIF_REGS_OFFSET( CPUID ))
#define MV_CPUIF_REGS_OFFSET( cpu )	( 0x21800 + ( cpu ) * 0x100 )
#define MV_CPUIF_REGS_BASE( cpu )	( MV_CPUIF_REGS_OFFSET( cpu ))
#define MV_CNTMR_REGS_OFFSET		( 0x20300 )
#define MV_L2_REGS_OFFSET			( 0x8000 )
#define MV_SCU_REGS_OFFSET			( 0xC000 )
#define MV_COHERENCY_FABRIC_OFFSET	( 0x20200 )
#define MPP_SAMPLE_AT_RESET( id )	( 0xE8200 + (( id ) * 0x4 ))

/* Common definitions */
#define BIT0					( 0x01 )
#define BIT1					( 0x02 )
#define BIT2					( 0x04 )
#define BIT3					( 0x08 )
#define BIT4					( 0x10 )
#define BIT5					( 0x20 )
#define BIT6					( 0x40 )
#define BIT7					( 0x80 )
#define MV_ERROR				( -1 )
#define MV_OK					( 0 )
#define MV_FAIL			    	( 1 )	
#define MV_TIMEOUT				( 0x0E )
#define MV_BAD_PARAM			( 4 )
#define MV_NO_MORE          	( 12 )
#define MV_NOT_STARTED      	( 29 )
#define MV_FALSE				( 0 )
#define MV_TRUE					( !( MV_FALSE ))
typedef unsigned char			MV_U8;
typedef unsigned long			MV_U32;
typedef long					MV_32;
typedef void					MV_VOID;
typedef long					MV_BOOL;
typedef long					MV_STATUS;
typedef long					bool;
#define FALSE					( 0 )
#define TRUE					( !FALSE )

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
											"NOP				\n\t"	\
											"SUBS	%0, %0, #1	\n\t"	\
											"BNE	1b			\n\t"	\
											: "+r" ( i ) : );			\
						} while ( 0 )

/* ASM functions to read and write CPSR */
#define arm_read_cpsr( x )		asm volatile ( "MRS	%0, CPSR	\n\t" : "=r" ( x ) : );
#define arm_write_cpsr( x )		asm volatile ( "MSR	CPSR_c, %0	\n\t" :: "r" ( x ));

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

/* PTE handling functions (always built in ARM mode) */
void pte_map_section( unsigned long base_phys, unsigned long base_virt,
		unsigned long size, unsigned long pte_flags );

#define vPTEAddSection( base_virt, base_phys, size, flags )	\
						vPTEMapSection( base_phys, base_virt, size, flags )
#define vPTEDelSection( base_virt, size )		\
						vPTEMapSection( base_virt, base_virt, size, PTE_INVALID )
#define vPTEGetSection( base_virt )				\
		( * (( unsigned long * )( uPageTable + (( base_virt ) >> MMU_SECTION_SHIFT ) * 4 )))
#define vPTESetSection( base_virt, pte_value )	\
		vPTEGetSection( base_virt ) = pte_value

#ifdef THUMB_INTERWORK
void vPTEMapSection( unsigned long base_phys, unsigned long base_virt,
	unsigned long size, unsigned long pte_flags );
#else
#define vPTEMapSection				pte_map_section
#endif

/* Cache maintenance functions (always built in ARM mode) */
unsigned long wbinv_disable_idcache( void );
void enable_idcache( unsigned long mask );
void armv7_dcache_wbinv_range( unsigned long base, unsigned long size);
void armv7_idcache_wbinv_all( void );

#ifdef THUMB_INTERWORK
unsigned long cpu_disable_idcache( void );
void cpu_enable_idcache( unsigned long mask );
void cpu_dcache_wbinv_range( unsigned long base, unsigned long size);
void cpu_idcache_wbinv_all( void );
#else
#define cpu_disable_idcache()		wbinv_disable_idcache()
#define cpu_enable_idcache( mask )	enable_idcache( mask )
#define cpu_dcache_wbinv_range( base, size )	armv7_dcache_wbinv_range( base, size )
#define cpu_idcache_wbinv_all()		armv7_idcache_wbinv_all()
#endif

#endif
#endif
