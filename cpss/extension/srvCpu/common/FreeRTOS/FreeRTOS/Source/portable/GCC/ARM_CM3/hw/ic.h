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

#ifndef IC_H
#define IC_H

#define ICMAX_INT               ( 240 )

/* Interrupt enable/disable */
#define IRQ_ENABLE  ( 1 )
#define IRQ_DISABLE ( 0 )

#define IRQ_PRIO_DEFAULT    ( 0 )

/* NVIC registers */
#define ICREG_NVIC_ISER(n)          ( 0xE000E100 + (n)*4)
#define ICREG_NVIC_ICER(n)          ( 0xE000E180 + (n)*4)

/* #if defined(CONFIG_BOBK) || defined(CONFIG_ALDRIN) || defined(CONFIG_BC3) */
#ifdef CM3
/* Switching core interrupt registers */
#define GLOB_IRQ_SUM_MASK_REG_0     (0x60000030)
#define GLOB_IRQ_SUM_CAUSE_REG_0    (0x60000034)
#define GLOB_IRQ_SUM_MASK_REG_1     (0x60000614)
#define GLOB_IRQ_SUM_CAUSE_REG_1    (0x60000610)
#define GLOB_IRQ_SUM_MASK_REG_2     (0x60000674)
#define GLOB_IRQ_SUM_CAUSE_REG_2    (0x60000670)
#define GLOB_IRQ_SUM_MASK_REG_3     (0x600002e4)
#define GLOB_IRQ_SUM_CAUSE_REG_3    (0x600002e0)
#define MG_INT_IRQ_MASK_REG_0       (0x6000003c)
#define MG_INT_IRQ_CAUSE_REG_0      (0x60000038)
#define MG_INT_IRQ_MASK_REG_1       (0x6000061c)
#define MG_INT_IRQ_CAUSE_REG_1      (0x60000618)
#define MG_INT_IRQ_MASK_REG_2       (0x6000067c)
#define MG_INT_IRQ_CAUSE_REG_2      (0x60000678)
#define MG_INT_IRQ_MASK_REG_3       (0x600002ec)
#define MG_INT_IRQ_CAUSE_REG_3      (0x600002e8)
#define MG_PORTS1_IRQ_MASK_REG_3    (0x600002d4) /* For Eagle : UART0 and UART1 interrupts are connected to Ports1 interrupt register */
#define MG_PORTS1_IRQ_CAUSE_REG_3   (0x600002d0)

#define DOORBELL_1_REG              (0x00000440)

#define PORTS_IRQ_SUM_MASK_REG3     (0x600002F4)
#define PORTS_IRQ_SUM_CAUSE_REG3    (0x600002F0)

#define MG1_INT_IRQ_CAUSE_REG_3     (0x60000118)
#define MG1_INT_IRQ_MASK_REG_3      (0x6000011c)

#define CM3_TO_MSYS_DOORBELL_REG    (0x60000538)

/* GLOB_IRQ_SUM_MASK_REG_3 fields */
#define MG_INTERNAL_SUM_MASK        (1 << 7)
#define MG_PORTS1_SUM_MASK          (1 << 16) /* Eagle */
#define MG1_INTERNAL_SUM_MASK       (1 << 18)
#define PORTS_INTERRUPTS_SUM_MASK   (1 << 5)

/* MG_INT_IRQ_MASK_REG_3 fields */
#define MG_UART_INT_MASK             (1 << 6)
#define MG_UART_INT_IN_PORTS1_MASK   (1 << 7) /* Eagle */

/* PORTS_IRQ_SUM MASK_REG3 fields */
#define PORTS_COMIRA_MASK       (3 <<  9)
#define PORTS_COMIRA0_MASK      (1 << 10)
#define PORTS_COMIRA1_MASK      (1 <<  9)

/* MG1_INT_IRQ_MASK_REG_3 fields */
#define MG_DOORBELL_FROM_MSYS       (1 << 1)
/* Interrupt nvic vector mapping, e.g. NVIC_GLOBAL_IRQ_0 (16)
 * results with (16 * 4) 0x40 offset in the CM3 NVIC vector table */
#define NVIC_GLOBAL_IRQ_0           ( 16 )
#define NVIC_GLOBAL_IRQ_1           ( 17 )
#define NVIC_GLOBAL_IRQ_2           ( 18 )
#define NVIC_GLOBAL_IRQ_3           ( 19 )

/* Interrupt mapping for enabling irq in NVIC ISER/ICER */
#define NVIC_GLOBAL_IRQ_0_EN            ( 0 )
#define NVIC_GLOBAL_IRQ_1_EN            ( 1 )
#define NVIC_GLOBAL_IRQ_2_EN            ( 2 )
#define NVIC_GLOBAL_IRQ_3_EN            ( 3 )

#endif /* #ifdef CM3 */
#ifdef CONFIG_A8K
#define NVIC_UART0              ( 20 )
#define NVIC_UART1              ( 23 )
#endif

typedef void ( *nvichandler )( void );

long iICRegisterHandler(int irq_vec_offset, long interrupt, nvichandler func,
                        long enable, long config );

inline void iICSetupInt( long interrupt, long secure, long priority,
                         long edge, long enable );
long iICUnregisterHandler(int irq_vec_offset, long interrupt );
void vICInit( void );
long iICEnableInt( long interrupt);
long iICDisableInt( long interrupt );

void tfp_printf( char *fmt, ... );
void cm3_interrupt_init(void);
void cm3_to_msys_doorbell(void);
#endif
