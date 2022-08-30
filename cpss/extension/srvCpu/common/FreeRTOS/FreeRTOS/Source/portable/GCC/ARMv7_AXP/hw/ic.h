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

#define ICREG_BASE				( 0x20000 )
#define ICREG_CPU_BASE( p )		( 0x1800 + 0x100 * ( p ))

#define ICMAX_IRQ				( 116 )
#define ICMAX_IRQ_PER_REG		( ICMAX_IRQ / 4 )
#define ICMAX_PRIVATE_IRQ		( 29 )

/* IC registers */
#define ICREG_CONTROL			( 0xA00 )
#define ICREG_SOFT_TRIG_INT		( 0xA04 )
#define ICREG_SOC_ERR_CAUSE		( 0xA20 )
#define ICREG_SET_ENABLE		( 0xA30 )
#define ICREG_CLR_ENABLE		( 0xA34 )
#define ICREG_SRC_CTRL( i )		( 0xB00 + 4 * ( i ))
/* Per CPU registers ( n = 0..3 ), Int Vector v = 0..3 */
/* These are definitions for AXP... 
for MSYS AC3 the register address is different */
#ifdef AC3_DEV_SUPPORT
#define ICREG_MAIN_INT_CAUSE( v )	( 0x1880 + 4 * ( v ))
#define ICREG_IRQ_SEL_CAUSE		( 0x18A0 )
#define ICREG_FIQ_SEL_CAUSE		( 0x18A4 )
#define ICREG_CUR_TASK_PRIO		( 0x18B0 )
#define ICREG_IRQ_INT_ACK		( 0x18B4 )
#define ICREG_SET_MASK			( 0x18B8 )
#define ICREG_CLR_MASK			( 0x18BC )
#define ICREG_SOC_ERR_MASK		( 0x18C0 )
#define ICREG_COH_FAB_LOCAL_MASK ( 0x18C4 )
#else
#define ICREG_MAIN_INT_CAUSE( v )	( 0x1080 + 4 * ( v ))
#define ICREG_IRQ_SEL_CAUSE		( 0x10A0 )
#define ICREG_FIQ_SEL_CAUSE		( 0x10A4 )
#define ICREG_CUR_TASK_PRIO		( 0x10B0 )
#define ICREG_IRQ_INT_ACK		( 0x10B4 )
#define ICREG_SET_MASK			( 0x10B8 )
#define ICREG_CLR_MASK			( 0x10BC )
#define ICREG_SOC_ERR_MASK		( 0x10C0 )
#define ICREG_COH_FAB_LOCAL_MASK ( 0x10C4 )
#endif /* AC3_DEV_SUPPORT */

/* IC register bit map */
#define ICBIT_CONTROL_PRIO_EN	( 0 )
#define ICBIT_CONTROL_NUM_INT	( 2 )
#define ICMASK_CONTROL_PRIO_EN	( 0x1 )
#define ICMASK_CONTROL_NUM_INT	( 0x1ffc )

#define ICBIT_SRC_CTRL_IRQ_MASK	( 0 )
#define ICBIT_SRC_CTRL_FIQ_MASK	( 8 )
#define ICBIT_SRC_CTRL_EP_SEL	( 16 )
#define ICBIT_SRC_CTRL_PRIORITY	( 24 )
#define ICBIT_SRC_CTRL_INT_EN	( 28 )
#define ICBIT_SRC_CTRL_EP_MASK	( 31 )
#define ICMASK_SRC_CTRL_PRIORITY ( 0xf << ICBIT_SRC_CTRL_PRIORITY )

#define ICBIT_IRQ_SEL_CAUSE_VECSEL	( 29 )
#define ICBIT_IRQ_SEL_CAUSE_STAT	( 31 )
#define ICMASK_IRQ_SEL_CAUSE_CAUSE	( 0x1fffffff )
#define ICMASK_IRQ_SEL_CAUSE_VECSEL	( 0x3 << ICBIT_IRQ_SEL_CAUSE_VECSEL )
#define ICMASK_IRQ_SEL_CAUSE_STAT	( 1 << ICBIT_IRQ_SEL_CAUSE_STAT )

#define ICBIT_CUR_TASK_PRIO_PRIO ( 28 )

#define ICMASK_IRQ_INT_ACK_ID	( 0x3ff )

/* Interrupt mapping */
#define IRQ_IN_DOORBELL_LOW		( 0 )
#define IRQ_IN_DOORBELL_HIGH	( 1 )
#define IRQ_OUT_DOORBELL_SUM	( 2 )
#define IRQ_LOCAL_TIMER0		( 5 )
#define IRQ_LOCAL_TIMER1		( 6 )
#define IRQ_LOCAL_WDT			( 7 )
#define IRQ_GLOBAL_TIMER0		( 37 )
#define IRQ_GLOBAL_TIMER1		( 38 )
#define IRQ_GLOBAL_TIMER2		( 39 )
#define IRQ_GLOBAL_TIMER3		( 40 )
#define IRQ_UART( i )			( 41 + ( i ))
#define IRQ_RTC					( 50 )
#define IRQ_GLOBAL_WDT			( 93 )

/* Interrupt priorities */
#define IRQ_PRIO_LOWEST			( 1 )
#define IRQ_PRIO_HIGHEST		( 15 )
#define IRQ_PRIO_TICKS			IRQ_PRIO_HIGHEST
#define IRQ_PRIO_DEFAULT		( 8 )

/* Interrupt enable/disable */
#define IRQ_ENABLE				( 1 )
#define IRQ_DISABLE				( 0 )

typedef long ( *irqhandler )( void * );
typedef struct {
	irqhandler	Handler;
	void		*UserData;
} irqvector;

long iICRegisterHandler( long irq, irqhandler func, void *UserData,
						long enable, long priority );
long iICUnregisterHandler( long irq );
void vICInit( void );
long iICEnableInt( long irq );
long iICDisableInt( long irq );
void vICDispatcher( void );

#endif
