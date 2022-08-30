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

#define __ASSEMBLY__
#include "hw/common.h"

.org 0
.text

.globl Reset
.globl uCpuId
.globl uBootParams
.globl uPageTable
.globl uKernelEnd
.globl uKernelStart
.globl pte_map_section
.globl wbinv_disable_idcache
.globl enable_idcache

.section ".vectors","ax"
.code 32
VectorTable:
	b	Reset
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq

_undefined_instruction:	.word undefined_instruction
_software_interrupt:	.word vPortYieldProcessor
_prefetch_abort:		.word prefetch_abort
_data_abort:			.word data_abort
_not_used:				.word not_used
_irq:					.word vIRQInterrupt
_fiq:					.word fiq

.align 6
/* Entry Point */
Reset:
	/* Test location of boot parameters and save the pointer */
	mov	r0, r2
	mov	r1, #0xff00
	add	r1, r1, #0xff
	and	r0, r0, r1
	mov	r1, #0x100
	cmp	r0, r1
	beq	1f
	mov	r2, #0
1:	str	r2, uBootParams

	/* Read current CPU ID */
	mrc	p15, 0, r0, c0, c0, 5
	and	r0, r0, #0xf
	str	r0, uCpuId

	/* set the cpu to SVC32 mode and disable interrupts */
	mrs	r0, cpsr
	bic	r0, r0, #0x1f
	orr	r0, r0, #0xd3
	msr	cpsr, r0

	/* Set vector address in CP15 VBAR register */
	ldr	r0, =VectorTable
	mcr	p15, 0, r0, c12, c0, 0	@ set VBAR

	/* Invalidate and enable ICache, disable DCache and MMU etc. */
	mov	r0, #0					@ set up for MCR
	mcr	p15, 0, r0, c8, c7, 0	@ invalidate TLBs
	mcr	p15, 0, r0, c7, c5, 0	@ invalidate icache
	mcr	p15, 0, r0, c7, c5, 6	@ invalidate BP array
	mcr	p15, 0, r0, c7, c10, 4	@ DSB
	mcr	p15, 0, r0, c7, c5, 4	@ ISB

	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002000		@ clear bits 13 (--V-) Set VBar = 0
	bic	r0, r0, #0x00000007		@ clear bits 2:0 (-CAM) Disable MMU, Alignment check and DCache
	orr	r0, r0, #0x00000800		@ set bit 11 (Z---) Enable Branch Prediction Unit
#if ENABLE_ICACHE
	orr	r0, r0, #0x00001000		@ set bit 12 (I) Enable ICache
#else
	bic	r0, r0, #0x00001000		@ clear bit 12 (I) Disable ICache
#endif
	mcr	p15, 0, r0, c1, c0, 0

	/* Clear bss, page table and stacks */
	ldr	r0, =_bss
	ldr	r1, =_ebss
	mov	r3, #( MMU_PTE_SIZE )	@ align Page Table to its size
	add	r2, r1, r3
	sub	r3, r3, #0x1
	mvn	r3, r3
	and	r2, r2, r3
	str	r2, uPageTable			@ save location of Page Table
	add	r3, r1, #( CPU_SUM_STACK_SIZE )
	add	r1, r2, #( MMU_PTE_SIZE )
	cmp	r2, r3
	subhs	r3, r2, #( CPU_SUM_STACK_SIZE )	@ locate stacks before PT
	addlo	r3, r2, #( MMU_PTE_SIZE )		@ locate stacks after PT
	addlo	r1, r1, #( CPU_SUM_STACK_SIZE )
	mov	r2, #0x00000000
clrbss:
	str	r2, [r0]
	add	r0, r0, #4
	cmp	r0, r1
	bne	clrbss

	/* Set stack pointer for SVC mode */
	bic	r3, r3, #7				@ 8-byte alignment for ABI compliance
	add	r3, r3, #( CPU_SVC_STACK_SIZE )
	mov	sp, r3

	/* Set the SP for IRQ, abort and undefined modes. */
	mrs	r2, cpsr
	bic	r0, r2, #0x1f
	orr	r0, r0, #0x12
	msr	cpsr, r0				@ switch to IRQ mode
	add	r3, r3, #( CPU_IRQ_STACK_SIZE )
	mov	sp, r3					@ set SP

	bic	r0, r2, #0x1f
	orr	r0, r0, #0x17
	msr	cpsr, r0				@ switch to Abort mode
	add	r3, r3, #( CPU_ABT_STACK_SIZE )
	mov	sp, r3					@ set SP

	bic	r0, r2, #0x1f
	orr	r0, r0, #0x1b
	msr	cpsr, r0				@ swtich to Undefined mode
	add	r3, r3, #( CPU_UND_STACK_SIZE )
	mov	sp, r3					@ set SP
	msr	cpsr, r2				@ switch back to SVC mode

	/* Setup initial Page Table ( r1 = end of page table and stacks ) */
	mov	r4, r1
	sub	r1, r1, #0x1
	str	r1, uKernelEnd
	bl	armv7_dcache_inv_all
	ldr	r0, =VectorTable		@ memory occupied by kernel
	str	r0, uKernelStart
	mov	r1, r0
	sub	r2, r4, r0
	ldr	r3, =PTE_MEMORY
	bl	pte_map_section			@ ( base, size, CB )
	ldr	r0, uBootParams			@ U-Boot boot parameters
	sub	r0, r0, #0x100
	mov	r1, r0
	mov	r2, #0x100000
	ldr	r3, =PTE_MEMORY
	bl	pte_map_section

	/* Setup Translation Table Base and Domain Access registers */
	ldr	r0, uPageTable
	mcr	p15, 0, r0, c2, c0, 0	@ setup Translation Table Base 0 Register
	mov	r1, #0x3				@ use only Domain 0 (Manager)
	mcr	p15, 0, r1, c3, c0, 0	@ setup Domain Access Control Register

	/* Enable DCache and MMU (1:1 mapping) */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 4	@ ISB
	mrc	p15, 0, r0, c1, c0, 0
#if ENABLE_DCACHE
	orr	r0, r0, #0x00000005		@ set bits 2,0 (C-M) Enable DCache and MMU
#else
	orr	r0, r0, #0x00000001		@ set bit 0 (M) Enable MMU
#endif
	mcr	p15, 0, r0, c1, c0, 0
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 4	@ ISB

	/* Jump to the main demo routine */
	blx	main

	/* Should not be reached */
infloop:
	b	infloop


/* Exception Handlers */
.align	5
undefined_instruction:			/* Undefined instruction exception */
	stmdb	sp!,{r0-r3,r12,lr}
	/* TODO */
1:	b	1b
	ldmia	sp!,{r0-r3,r12,lr}
	movs	pc, lr

.align	5
prefetch_abort:					/* Prefetch Abort exception */
	stmdb	sp!,{r0-r3,r12,lr}
	/* TODO */
1:	b	1b
	ldmia	sp!,{r0-r3,r12,lr}
	subs	pc, lr, #4

.align	5
data_abort:						/* Data Abort exception */
	stmdb	sp!,{r0-r3,r12,lr}
	/* TODO */
1:	b	1b
	ldmia	sp!,{r0-r3,r12,lr}
	subs	pc, lr, #4

.align	5
not_used:
	b	not_used

.align	5
fiq:							/* FIQ vector handler */
	stmdb	sp!,{r0-r3,r12,lr}
	/* TODO */
1:	b	1b
	ldmia	sp!,{r0-r3,r12,lr}
	subs	pc, lr, #4


uCpuId:			.word 0
uBootParams:	.word 0x100
uPageTable:		.word 0
uKernelEnd:		.word 0
uKernelStart:		.word 0


pte_map_section:				@ ( base_phys, base_virt, size, CB )
	stmdb	sp!, {r4, sl, lr}
	cmp	r2, #0x0
	beq	2f
	stmdb	sp!, {r0-r3}
	bl	armv7_idcache_wbinv_all
	ldmia	sp!, {r0-r3}
	/* Calculate position of first PT Entry */
	add	r2, r2, r1
	sub	r2, r2, #0x1
	lsr	r2, r2, #( MMU_SECTION_SHIFT )	@ last virt section
	cmp	r2, #( MMU_PTE_COUNT )
	bhs	2f
	ldr	r4, uPageTable
	orr	r2, r2, r4, lsr #0x2
	lsr	r1, r1, #( MMU_SECTION_SHIFT )
	add	r4, r4, r1, lsl #0x2			@ first PTE
	/* Prepare entry (r3 == PTE_INVALID removes the entry) */
	tst	r3, #( PTE_INVALID )
	bne	1f
	lsr	r0, r0, #( MMU_SECTION_SHIFT )
	lsl	r0, r0, #( MMU_SECTION_SHIFT )
	ldr	r1, =0x000ff00c
	and	r3, r3, r1
	orr	r3, r0, r3				@ C,B bits
	orr	r3, r3, #0xc00			@ AP=0x11
	orr	r3, r3, #0x02			@ Domain=0x0, 1MB Section
	/* Write necessary entries */
1:	cmp	r2, r4, lsr #0x2
	str	r3, [r4], #4
	add	r3, r3, #( 1 << MMU_SECTION_SHIFT )
	bhi	1b
	/* Flush Dcache and clean TLB */
	bl	armv7_dcache_wbinv_all
	bl	armv7_tlb_flushID
2:
	ldmia	sp!, {r4, sl, lr}
	bx	lr

wbinv_disable_idcache:			@ ( void )
	stmdb	sp!, {r4, sl, lr}
	bl	armv7_dcache_wbinv_all
	mrc	p15, 0, r4, c1, c0, 0
	bic	r1, r4, #0x00001000		@ clear bit 12 (I) Disable ICache
	bic	r1, r1, #0x00000004		@ clear bit 2 (C) Disable DCache
	mcr	p15, 0, r1, c1, c0, 0
	bl	armv7_idcache_wbinv_all
	mov	r0, r4
	ldmia	sp!, {r4, sl, lr}
	bx	lr

enable_idcache:					@ ( mask )
	mrc	p15, 0, r0, c1, c0, 0
	orr	r1, r0, #0x00001000		@ set bit 12 (I) Enable ICache
	orr	r1, r1, #0x00000004		@ set bit 3 (C) Enable DCache
	mcr	p15, 0, r1, c1, c0, 0
	bx	lr

.end
