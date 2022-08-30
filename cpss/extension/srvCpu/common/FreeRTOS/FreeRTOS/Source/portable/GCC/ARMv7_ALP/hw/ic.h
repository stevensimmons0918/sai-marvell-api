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

#define ICMAX_PPI				( 16 )
#define ICMAX_SGI				( 16 )
#define ICMAX_SPI				( 160 )
#define ICMAX_GIC_IRQ			( ICMAX_PPI + ICMAX_SGI + ICMAX_SPI )
#define ICMAX_SOC_PPI			( 32 )
#define ICMAX_SOC_ERROR			( 64 )
#define ICMAX_IRQ				( ICMAX_GIC_IRQ + ICMAX_SOC_PPI )

/* Per CPU registers ( n = 0..3 ), Int Vector v = 0..3 */
#define ICREG_SOC_PPI_CAUSE		( MV_CPUIF_LOCAL_REGS_OFFSET | 0x80 )
#define ICREG_SET_MASK			( MV_CPUIF_LOCAL_REGS_OFFSET | 0xB8 )
#define ICREG_CLR_MASK			( MV_CPUIF_LOCAL_REGS_OFFSET | 0xBC )

/* Interrupt mapping */
#define IRQ_IN_DOORBELL_LOW		( ICMAX_GIC_IRQ + 0 )
#define IRQ_IN_DOORBELL_HIGH	( ICMAX_GIC_IRQ + 1 )
#define IRQ_LOCAL_TIMER0		( ICMAX_GIC_IRQ + 5 )
#define IRQ_LOCAL_TIMER1		( ICMAX_GIC_IRQ + 6 )
#define IRQ_LOCAL_WDT			( ICMAX_GIC_IRQ + 7 )
#define IRQ_SGI( n )			( n )
#define IRQ_SOC_PPI_GROUP0		( 28 )
#define IRQ_SOC_PPI_GROUP1		( 31 )
#define IRQ_GLOBAL_TIMER0		( 40 )
#define IRQ_GLOBAL_TIMER1		( 41 )
#define IRQ_GLOBAL_TIMER2		( 42 )
#define IRQ_GLOBAL_TIMER3		( 43 )
#define IRQ_UART( i )			( 44 + ( i ))
#define IRQ_RTC					( 53 )
#define IRQ_GLOBAL_WDT			( 96 )
#define IRQ_SH_DOORBELL_0		( 99 )
#define IRQ_SH_DOORBELL_1		( 100 )
#define IRQ_SH_DOORBELL_2		( 101 )
#define IRQ_SH_DOORBELL( n )	( 99 + ( n ))

/* Interrupt priorities
 * Setting priority to 240 may disable interrupt on some GIC
 * Limit values to 0-119 for secure (fiq) and 120-239 for non-secure (irq)
 * On Avanta LP priority can be set in steps of 8
 */
#define IRQ_PRIORITY_MASK		( 0xff )
#define IRQ_PRIO_LOWEST			( 119 )
#define IRQ_PRIO_HIGHEST		( 0 )
#define IRQ_PRIO_TICKS			IRQ_PRIO_HIGHEST
#define IRQ_PRIO_DEFAULT		( 64 )
#define IRQ_PRIO_HIGHEST_NONSECURE	( 120 )

/* Edge or level triggered */
#define IRQ_TRIG_EDGE			( 1 << 12 )
#define IRQ_TRIG_LEVEL			( 0 )

/* Secure or Non-secure */
#define IRQ_SECURE				( 1 << 14 )
#define IRQ_NONSECURE			( 0 )
#define IRQ_FIQ					IRQ_SECURE

/* Interrupt enable/disable */
#define IRQ_ENABLE				( 1 )
#define IRQ_DISABLE				( 0 )

#define IRQ_CFG_NULL			( -1 )

typedef long ( *irqhandler )( void * );
typedef struct {
	irqhandler	Handler;
	void		*UserData;
} irqvector;

long iICRegisterHandler( long irq, irqhandler func, void *UserData,
						long enable, long config );
inline void iICSetupInt( long irq, long secure, long priority,
						 long edge, long enable );
long iICUnregisterHandler( long irq );
void vICInit( void );
long iICEnableInt( long irq );
long iICDisableInt( long irq );
void vICDispatcher( void );
void vICSendSGI( unsigned long filter, unsigned long target, unsigned long sgiid );

#endif
