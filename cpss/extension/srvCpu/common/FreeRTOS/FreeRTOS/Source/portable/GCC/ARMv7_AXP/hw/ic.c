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
#include "mvSemaphore.h"

irqvector ICVectors[ ICMAX_IRQ ];

static inline unsigned long uICRegRead( long reg )
{
	return MV_REG_READ( ICREG_BASE | reg );
}

static inline void vICRegWrite( long reg, unsigned long value )
{
	MV_REG_WRITE( ICREG_BASE | reg, value );
}

long iICRegisterHandler( long irq, irqhandler func, void *UserData,
						long enable, long priority )
{
	unsigned long value;

	if (( irq < 0 ) || ( irq >= ICMAX_IRQ ))
		return -1;

	iICDisableInt( irq );

	ICVectors[ irq ].Handler = func;
	ICVectors[ irq ].UserData = UserData;

	mvSemaLock( MV_SEMA_IRQ );
	value = uICRegRead( ICREG_SRC_CTRL( irq ));

#ifdef IC_USE_PRIO_MODE
	if ( priority < IRQ_PRIO_LOWEST )
		priority = IRQ_PRIO_LOWEST;
	if ( priority > IRQ_PRIO_HIGHEST )
		priority = IRQ_PRIO_HIGHEST;
	priority <<= ICBIT_SRC_CTRL_PRIORITY;
	value &= ~ICMASK_SRC_CTRL_PRIORITY;
	value |= priority;
#endif

	value |= ( 1 << ICBIT_SRC_CTRL_INT_EN );
	vICRegWrite( ICREG_SRC_CTRL( irq ), value );
	/* Semaphor will be unlocked in Enable/Disable functions */

	if ( enable )
		iICEnableInt( irq );
	else
		iICDisableInt( irq );

	return 0;
}

long iICUnregisterHandler( long irq )
{
	if (( irq < 0 ) || ( irq >= ICMAX_IRQ ))
		return -1;

	iICDisableInt( irq );
	ICVectors[ irq ].Handler = NULL;

	return 0;
}

long iICEnableInt( long irq )
{
	if (( irq < 0 ) || ( irq >= ICMAX_IRQ ))
		return -1;
	if ( ICVectors[ irq ].Handler == NULL )
		return -2;

	mvSemaLock( MV_SEMA_IRQ );
	vICRegWrite( ICREG_CLR_MASK, irq );
	mvSemaUnlock( MV_SEMA_IRQ );

	return 0;
}

long iICDisableInt( long irq )
{
	if (( irq < 0 ) || ( irq >= ICMAX_IRQ ))
		return -1;

	mvSemaLock( MV_SEMA_IRQ );
	vICRegWrite( ICREG_SET_MASK, irq );
	mvSemaUnlock( MV_SEMA_IRQ );

	return 0;
}

void vICDispatcher( void )
{
	unsigned long irq;
	long ret;

	for ( ;; ) {
#ifdef IC_USE_PRIO_MODE
		irq = uICRegRead( ICREG_IRQ_INT_ACK );
		irq &= ICMASK_IRQ_INT_ACK_ID;

		if ( irq == ICMASK_IRQ_INT_ACK_ID )
			break;
#else
		irq = uICRegRead( ICREG_IRQ_SEL_CAUSE );
		if (( irq & ICMASK_IRQ_SEL_CAUSE_STAT ) == 0 )
			break;

		ret = irq & ICMASK_IRQ_SEL_CAUSE_VECSEL;
		ret = ( ret >> ICBIT_IRQ_SEL_CAUSE_VECSEL ) * ICMAX_IRQ_PER_REG;

		irq &= ICMASK_IRQ_SEL_CAUSE_CAUSE;
		if ( irq == 0 )
			continue;

		for ( ; ( irq & 1 ) == 0; irq >>= 1, ret++ );
		irq = ret;
#endif

		ret = -1;
		if ( ICVectors[ irq ].Handler != NULL )
			ret = ICVectors[ irq ].Handler( ICVectors[ irq ].UserData );

		if ( ret < 0 )
			iICDisableInt( irq );
	}
}

void vICInit( void )
{
	unsigned long val;
	long irq;

	mvSemaLock( MV_SEMA_IRQ );

	val = uICRegRead( ICREG_CONTROL );
#ifdef IC_USE_PRIO_MODE
	val |= ICMASK_CONTROL_PRIO_EN;
#else
	val &= ~ICMASK_CONTROL_PRIO_EN;
#endif
	vICRegWrite( ICREG_CONTROL, val );
	vICRegWrite( ICREG_CUR_TASK_PRIO, 0 );

	for ( irq = 0; irq < ICMAX_IRQ; irq++ ) {
		/* Disable IRQ, FIQ and EndPoint Selector on current CPU */
		val = uICRegRead( ICREG_SRC_CTRL( irq ));
		val &= ~((( 1 << ICBIT_SRC_CTRL_IRQ_MASK ) |
				  ( 1 << ICBIT_SRC_CTRL_FIQ_MASK ) |
				  ( 1 << ICBIT_SRC_CTRL_EP_SEL )) << CPUID );
		vICRegWrite( ICREG_SRC_CTRL( irq ), val );
	}

	mvSemaUnlock( MV_SEMA_IRQ );
}

