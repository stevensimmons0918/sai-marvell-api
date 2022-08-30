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
.globl arm_read_cpsr
.globl arm_write_cpsr
.globl arm_flush_wb

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
_prefetch_abort:	.word prefetch_abort
_data_abort:		.word data_abort
_not_used:		.word not_used
_irq:			.word vIRQInterrupt
_fiq:			.word vIRQInterrupt

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

	/* set the cpu to SVC32 mode and disable interrupts */
	mrs	r0, cpsr
	bic	r0, r0, #0x1f
	orr	r0, r0, #0xd3
	msr	cpsr, r0

	/* Configure ITCM and DTCM */
	mov	r0, #0					@ set up for MCR
	mcr	p15, 0, r0, c7, c10, 4	@ drain write buffer

	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002000		@ clear bits 13 - VBar=0
	bic	r0, r0, #0x00000002		@ clear bits 1 - disable Alignment Check
#ifdef ENABLE_BPU
#if ENABLE_BPU
	orr	r0, r0, #0x00000800		@ set bit 11 Enable BPU
#else
	bic	r0, r0, #0x00000800
#endif
#endif
#ifdef ENABLE_WB
#if ENABLE_WB
	orr	r0, r0, #0x00000008		@ set bit 3 Enable Write Buffer
#else
	bic	r0, r0, #0x00000008
#endif
#endif
#ifdef ENABLE_ITCM
#if ENABLE_ITCM
	orr	r0, r0, #0x00001000		@ set bit 12 Enable ITCM
#else
	bic	r0, r0, #0x00001000
#endif
#endif
#ifdef ENABLE_DTCM
#if ENABLE_DTCM
	orr	r0, r0, #0x00000004		@ set bit 2 Enable DTCM
#else
	bic	r0, r0, #0x00000004
#endif
#endif
	mcr	p15, 0, r0, c1, c0, 0

	/* Copy data segment */
	ldr	r0, =_sdata
	ldr	r1, =_edata
	ldr	r2, =_sidata

copydata:
	ldr	r3, [r2], #4
	str	r3, [r0], #4
	cmp	r0, r1
	bne	copydata

	/* Clear bss, page table and stacks */
	ldr	r0, =_sbss
	ldr	r1, =_ebss
	mov	r3, r1				@ locate stacks after _ebss
	add	r1, r3, #( CPU_SUM_STACK_SIZE )	@ end of kernel memory

	mov	r2, #0x00000000
clrbss:
	str	r2, [r0], #4
	cmp	r0, r1
	bne	clrbss

	/* Set stack pointer for SVC mode */
	bic	r3, r3, #7			@ 8-byte alignment for ABI compliance
	add	r3, r3, #( CPU_SVC_STACK_SIZE )
	mov	sp, r3

	/* Set the SP for IRQ, FIQ, abort and undefined modes. */
	mrs	r2, cpsr

	bic	r0, r2, #0x1f
	orr	r0, r0, #0x12
	msr	cpsr, r0			@ switch to IRQ mode
	add	r3, r3, #( CPU_IRQ_STACK_SIZE )
	mov	sp, r3				@ set SP

	bic	r0, r2, #0x1f
	orr	r0, r0, #0x11
	msr	cpsr, r0			@ switch to FIQ mode
	add	r3, r3, #( CPU_FIQ_STACK_SIZE )
	mov	sp, r3				@ set SP

	bic	r0, r2, #0x1f
	orr	r0, r0, #0x17
	msr	cpsr, r0			@ switch to Abort mode
	add	r3, r3, #( CPU_ABT_STACK_SIZE )
	mov	sp, r3				@ set SP

	bic	r0, r2, #0x1f
	orr	r0, r0, #0x1b
	msr	cpsr, r0			@ swtich to Undefined mode
	add	r3, r3, #( CPU_UND_STACK_SIZE )
	mov	sp, r3				@ set SP
	msr	cpsr, r2			@ switch back to SVC mode

	mov	r4, r1
	str	r1, uKernelEnd

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
prefetch_abort:				/* Prefetch Abort exception */
	stmdb	sp!,{r0-r3,r12,lr}
	/* TODO */
1:	b	1b
	ldmia	sp!,{r0-r3,r12,lr}
	subs	pc, lr, #4

.align	5
data_abort:				/* Data Abort exception */
	stmdb	sp!,{r0-r3,r12,lr}
	/* TODO */
1:	b	1b
	ldmia	sp!,{r0-r3,r12,lr}
	subs	pc, lr, #4

uDataAbortWA:	.word 0x1

.align	5
not_used:
	b	not_used


uCpuId:			.word 0x5
uBootParams:		.word 0x100
uPageTable:		.word 0
uKernelEnd:		.word 0

arm_read_cpsr:
	mrs	r0, CPSR
	bx	lr

arm_write_cpsr:
	msr	CPSR_c, r0
	bx	lr

arm_flush_wb:
	mov	r0, #0
	mcr	p15, 0, r0, c7, c10, 4	@ drain write buffer
	bx	lr

.end
