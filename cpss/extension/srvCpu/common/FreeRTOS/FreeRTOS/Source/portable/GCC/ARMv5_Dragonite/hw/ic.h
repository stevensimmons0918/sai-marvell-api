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

#define ICMAX_IRQ				( 6 )
#define ICMAX_FIQ				( 3 )
#define ICMAX_INT				( ICMAX_IRQ + ICMAX_FIQ )

/* PoE IRQ/FIQ registers */
#define ICREG_IRQ_CAUSE			( 0x0 )
#define ICREG_IRQ_MASK			( 0x4 )
#define ICREG_FIQ_CAUSE			( 0x8 )
#define ICREG_FIQ_MASK			( 0xC )

/* Interrupt mapping */
#define IRQ_TIMER0				( 0 )
#define IRQ_TIMER1				( 1 )
#define IRQ_TIMER2				( 2 )
#define IRQ_WDTIMER				( 3 )
#define IRQ_DTCM_PARITY_ERR		( 4 )
#define IRQ_ITCM_PARITY_ERR		( 5 )
#define FIQ_DOORBELL			( 6 )
#define FIQ_LANTIQ			( 7 )
#define FIQ_GPP0			( 8 )

/* Interrupt enable/disable */
#define IRQ_ENABLE				( 1 )
#define IRQ_DISABLE				( 0 )

#define IRQ_CFG_NULL			( -1 )

#define IRQ_PRIO_DEFAULT		( 0 )

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
long iICAckInt( long irq );

#endif
