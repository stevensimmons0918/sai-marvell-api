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

#include "FreeRTOS.h"
#include "hw.h"
#include "ic.h"
#include <portmacro.h>
#include <printf.h>
#include <cli.h>

extern void InterruptHandler(void);
extern void mcdSquelchRxClockImplement(unsigned short);
#define REG_WRITE(reg, val) \
            ( *(( volatile unsigned long * )( reg ))) = (val)

#define REG_READ(reg) \
            ( *(( volatile unsigned long * )( reg )))

#define NVICREG( reg )                      \
            ( *(( volatile unsigned long * )( reg )))

static inline void vICRegWrite( long reg, long irq_mask )
{
    NVICREG(reg) = irq_mask;
}

static inline void vICSetHandler( long interrupt, nvichandler func )
{
    /* set excption table - start at address 0 */
    ( *(( volatile unsigned long * ) ( interrupt * 4 ))) = ( unsigned long )func;
}


long iICRegisterHandler(int irq_vec_offset, long interrupt, nvichandler func,
                        long enable, long config )
{
    long ret;

    if (( interrupt < 0 ) || ( interrupt >= ICMAX_INT ))
        return -1;
    if ( func == NULL )
        return -2;

    ret = iICDisableInt( interrupt );

    /* In STM32 the NVIC vector table is placed in ROM and marked as a
     * read-only. So the interrupt handler can't be registered on the
     * runtime. Instead, all interrupt handlers used by STM32 are declared
     * in fixed 'exception_table'
     */
#ifndef CONFIG_STM32
    vICSetHandler(irq_vec_offset, func);
#endif

    if ( enable )
        ret = iICEnableInt( interrupt );

    return ret;
}

inline void iICSetupInt( long interrupt, long secure, long priority,
                         long edge, long enable )
{
    if (( interrupt < 0 ) || ( interrupt >= ICMAX_INT ))
        return;

    if ( enable )
        iICEnableInt( interrupt );
    else
        iICDisableInt( interrupt );
}

long iICUnregisterHandler(int irq_vec_offset, long interrupt )
{
    long ret;

    ret = iICDisableInt( interrupt );
    vICSetHandler(irq_vec_offset, InterruptHandler);

    return ret;
}

long iICEnableInt( long interrupt )
{
    int reg_of;

    if (interrupt == 0)
        reg_of = 0;
    else
        reg_of = interrupt / 32;

    if (( interrupt < 0 ) || ( interrupt >= ICMAX_INT ))
        return -1;

    vICRegWrite(ICREG_NVIC_ISER(reg_of), ( 1 << (interrupt % 32)));

    return 0;
}

long iICDisableInt( long interrupt )
{
    int reg_of;

    if (( interrupt < 0 ) || ( interrupt >= ICMAX_INT ))
        return -1;

    if (interrupt == 0)
        reg_of = 0;
    else
        reg_of = interrupt / 32;

    vICRegWrite(ICREG_NVIC_ICER(reg_of), ( 1 << (interrupt % 32)));

    return 0;
}

void vICInit( void )
{
}

/* #if defined(CONFIG_BOBK) || defined(CONFIG_ALDRIN) || defined(CONFIG_BC3) */
#ifdef CM3
static void cm3_mask_irq(void)
{
    /* maks all MG interrupts for CM3 */
    REG_WRITE(GLOB_IRQ_SUM_MASK_REG_3, 0x0);
#ifdef FALCON_DEV_SUPPORT
    REG_WRITE(MG_PORTS1_IRQ_MASK_REG_3, 0x180);
#endif
    /* mask all MG and MG1 interrupts for CM3 */
    REG_WRITE(MG_INT_IRQ_MASK_REG_3, 0x0);
    REG_WRITE(MG1_INT_IRQ_MASK_REG_3, 0x0);
}

static void cm3_unmask_irq(void)
{
    int reg;

    reg = REG_READ(GLOB_IRQ_SUM_MASK_REG_3);
    /* unmask mg_internal_int_sum and mg1_internal_int_sum */
#ifndef FALCON_DEV_SUPPORT
    REG_WRITE(GLOB_IRQ_SUM_MASK_REG_3,
        reg | MG_INTERNAL_SUM_MASK | MG1_INTERNAL_SUM_MASK);
#else
    REG_WRITE(GLOB_IRQ_SUM_MASK_REG_3,
        reg | MG_PORTS1_SUM_MASK | MG1_INTERNAL_SUM_MASK);
#endif


#ifndef DISABLE_CLI
#ifndef FALCON_DEV_SUPPORT
    /* unmask uart interrupt */
    REG_WRITE(MG_INT_IRQ_MASK_REG_3, MG_UART_INT_MASK);
#else
    REG_WRITE(MG_INT_IRQ_MASK_REG_3, MG_UART_INT_IN_PORTS1_MASK);
#endif
#endif

    /* unmask doorbell interrupt */
    REG_WRITE(MG1_INT_IRQ_MASK_REG_3, MG_DOORBELL_FROM_MSYS);

}

static void doorbell_from_msys(void)
{
    /* TODO: only for demo purpose - hook your interrupt handler here */
    tfp_printf("doorbell from MSYS\n");
}

static void cm3_mg1_internal_irq_dispatcher3(void)
{
    int reg;

    reg = REG_READ(MG1_INT_IRQ_CAUSE_REG_3);

    if (reg & MG_DOORBELL_FROM_MSYS) {
        doorbell_from_msys();
    }
}

static void cm3_mg_internal_irq_dispatcher3(void)
{
#ifndef DISABLE_CLI
    int reg;
#ifndef FALCON_DEV_SUPPORT
    reg = REG_READ(MG_INT_IRQ_CAUSE_REG_3);
    if (reg & MG_UART_INT_MASK) {
#else
    reg = REG_READ(MG_PORTS1_IRQ_CAUSE_REG_3);
    if (reg & MG_UART_INT_IN_PORTS1_MASK ) {
#endif
        iCLIHandler();
    }
#endif /* DISABLE_CLI */
}

static void cm3_irq_dispatcher3(void)
{
    int reg;

    reg = REG_READ(GLOB_IRQ_SUM_CAUSE_REG_3);

#ifndef FALCON_DEV_SUPPORT
    if (reg & MG_INTERNAL_SUM_MASK)
#else
    if (reg & MG_PORTS1_SUM_MASK)
#endif
        cm3_mg_internal_irq_dispatcher3();

    if (reg & MG1_INTERNAL_SUM_MASK)
        cm3_mg1_internal_irq_dispatcher3();
}

void cm3_interrupt_init(void)
{
    /* maks all interrupts */
    cm3_mask_irq();

    iICRegisterHandler( NVIC_GLOBAL_IRQ_3, NVIC_GLOBAL_IRQ_3_EN,
        cm3_irq_dispatcher3, IRQ_ENABLE, IRQ_PRIO_DEFAULT );

    /* Unmask used cm3 interrupts - enabling them via NVIC ISER is not
     * enough since they are also mapped to switching core, where they are
     * aggregated in global vector interrupt. Thats why all msys interrupts
     * connected to CM3 have to be configured (masked/unmasked) for the cm3
     * via Global interrupt 3 and MG internal interrupt register - please
     * refer to BobK FS chapter 'Switching Core Interrupts'
     */
    cm3_unmask_irq();
}
void cm3_to_msys_doorbell(void)
{
    REG_WRITE(CM3_TO_MSYS_DOORBELL_REG, 0x1);
}
#endif /* #ifdef CM3 */

