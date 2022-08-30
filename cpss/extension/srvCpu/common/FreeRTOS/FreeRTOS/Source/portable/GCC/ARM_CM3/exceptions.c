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
#include <FreeRTOS.h>
#include <task.h>
#include "printf.h"

extern void stackPtr(void);
extern void xPortPendSVHandler(void);
extern void vPortSVCHandler( void );
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);
extern int main( void );

#define uint32_t unsigned long

typedef void(* const ExecFuncPtr)(void);

void Reset(void);
void InterruptHandler(void);
/* The prototype is a naked function, for an assembly function. */
static void Excep_Handler( void ) __attribute__( ( naked ) );


__attribute__ ((section(".exceptions_area")))
/* Place table in separate section */
ExecFuncPtr exception_table[] = {
	/* 0 */  stackPtr,
	/* 1 */  (ExecFuncPtr)Reset, /* Initial PC, set to entry point */
	/* 2 */  Excep_Handler,  /* Entries 2-6 were set with the generic exception handler */
	/* 3 */  Excep_Handler,
	/* 4 */  Excep_Handler,
	/* 5 */  Excep_Handler,
	/* 6 */  Excep_Handler,
	/* 7 */  InterruptHandler,
	/* 8 */  InterruptHandler,
	/* 9 */  InterruptHandler,
	/* 10 */ InterruptHandler,
	/* 11 */ vPortSVCHandler,
	/* 12 */ InterruptHandler,
	/* 13 */ InterruptHandler,
	/* 14 */ xPortPendSVHandler,
	/* 15 */ xPortSysTickHandler, /* Timer Handler */
	/* 16 */ InterruptHandler,
	/* 17 */ InterruptHandler,
	/* 18 */ InterruptHandler,
	/* 19 */ InterruptHandler,
	/* 20 */ InterruptHandler,
	/* 21 */ InterruptHandler,
	/* 22 */ InterruptHandler,
	/* 23 */ InterruptHandler, /* UART 1 */
};

extern unsigned long _bss;
extern unsigned long _ebss;

void Reset(void)
{
	/* Clear bss, page table and stacks.
	   Jump to the main demo routine - main should not return */
	    __asm( "ldr	r0, =_bss\n"
			   "ldr	r1, =_ebss\n"
	           "mov	r2, #0x00000000\n"
            "clrbss:\n"
				"str	r2, [r0]\n"
				"add	r0, r0, #4\n"
				"cmp	r0, r1\n"
				"bne	clrbss\n"
				"blx	main\n"
				"infloop:\n"
			"b	infloop");
}

/* emty exception handlers */
void InterruptHandler(void)
{
	while(1);
}

/* The function fetches PC and excep number from the stack pointer passed to it */
void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress );
void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
    uint32_t pc; /* Program counter. */
    uint32_t psr_excep;/* Program status register (at exception stage) - denotes excp # */

    // lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr_excep = pulFaultStackAddress[ 8 ];

    printf("Exception !!\n"
#if ( INCLUDE_pcTaskGetTaskName == 1 )
           "  task %s\n"
#endif
           "  pc 0x%x\n"
           "  excp # %d\n",
#if ( INCLUDE_pcTaskGetTaskName == 1 )
           pcTaskGetTaskName(NULL),
#endif
           pc, psr_excep & 0xFF);

   while(1);
}

/* The fault handler implementation determines which stack was being used when the fault occurred.
   Then it passes a pointer to the stack into a C function called prvGetRegistersFromStack().
*/
static void Excep_Handler(void)
{
    __asm volatile
    (
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " mrs r2, IPSR                                              \n"
        " str r2, [r0, #32]                                         \n"
        " bl prvGetRegistersFromStack    \n"
    );
}

