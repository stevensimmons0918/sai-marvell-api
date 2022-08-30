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
#include "gic.h"
#include <portmacro.h>
#include "mvSemaphore.h"

irqvector ICVectors[ ICMAX_IRQ ];

static inline unsigned long uICRegRead( long reg )
{
	return MV_REG_READ( reg );
}

static inline void vICRegWrite( long reg, unsigned long value )
{
	MV_REG_WRITE( reg, value );
}

long iICRegisterHandler( long irq, irqhandler func, void *UserData,
						long enable, long config )
{
	long ret;
	static long soc_ppi_installed = 0;

	if (( irq < 0 ) || ( irq >= ICMAX_IRQ ))
		return -1;
	if ( func == NULL )
		return -2;

	ret = iICDisableInt( irq );

	ICVectors[ irq ].Handler = func;
	ICVectors[ irq ].UserData = UserData;

	mvSemaLock( MV_SEMA_IRQ );
	if ( irq >= ICMAX_GIC_IRQ ) {
		if ( soc_ppi_installed == 0 ) {
			/* Enable SOC PPI interrupts */
			arm_setup_irq( IRQ_SOC_PPI_GROUP0, IRQ_NONSECURE,
					IRQ_PRIO_DEFAULT, IRQ_TRIG_LEVEL, IRQ_ENABLE );
			arm_setup_irq( IRQ_SOC_PPI_GROUP1, IRQ_NONSECURE,
					IRQ_PRIO_DEFAULT, IRQ_TRIG_LEVEL, IRQ_ENABLE );
			soc_ppi_installed = 1;
		}
	} else if ( config != IRQ_CFG_NULL ) {
		arm_setup_irq( irq, config & IRQ_SECURE, config & IRQ_PRIORITY_MASK,
				config & IRQ_TRIG_EDGE, IRQ_CFG_NULL );
	}
	mvSemaUnlock( MV_SEMA_IRQ );

	if ( enable )
		ret = iICEnableInt( irq );

	return ret;
}

inline void iICSetupInt( long irq, long secure, long priority,
						 long edge, long enable )
{
	if (( irq < 0 ) || ( irq >= ICMAX_GIC_IRQ ))
		return;

	mvSemaLock( MV_SEMA_IRQ );
	arm_setup_irq( irq, secure, priority, edge, enable );
	mvSemaUnlock( MV_SEMA_IRQ );
}

long iICUnregisterHandler( long irq )
{
	long ret;

	ret = iICDisableInt( irq );
	ICVectors[ irq ].Handler = NULL;

	return ret;
}

long iICEnableInt( long irq )
{
	if (( irq < 0 ) || ( irq >= ICMAX_IRQ ))
		return -1;
	if ( ICVectors[ irq ].Handler == NULL )
		return -2;

	mvSemaLock( MV_SEMA_IRQ );
	if ( irq < ICMAX_GIC_IRQ )
		arm_unmask_irq( irq );
	else
		vICRegWrite( ICREG_CLR_MASK, irq - ICMAX_GIC_IRQ );
	mvSemaUnlock( MV_SEMA_IRQ );

	return 0;
}

long iICDisableInt( long irq )
{
	if (( irq < 0 ) || ( irq >= ICMAX_IRQ ))
		return -1;

	mvSemaLock( MV_SEMA_IRQ );
	if ( irq < ICMAX_GIC_IRQ )
		arm_mask_irq( irq );
	else
		vICRegWrite( ICREG_SET_MASK, irq - ICMAX_GIC_IRQ );
	mvSemaUnlock( MV_SEMA_IRQ );

	return 0;
}

void vICDispatcher( void )
{
	long irq, gic_irq, ppi;
	long ret, fiq;

	/* Disable AckCtl to handle only FIQ interrupts */
	arm_read_cpsr( fiq );
	if (( fiq & CPSR_MODE_MASK ) == CPSR_MODE_FIQ ) {
		fiq = 1;
		ret = arm_gic_read_reg( A9_MPCORE_GIC_CPU + GICC_CTLR );
		ret &= ~GICC_CTLR_ACKCTL;
		arm_gic_write_reg( A9_MPCORE_GIC_CPU + GICC_CTLR, ret );
	} else
		fiq = 0;

	for ( ;; ) {
		gic_irq = arm_get_next_irq();
		irq = gic_irq & 0x3FF;
		if ( irq >= ICMAX_GIC_IRQ )
			break;

		if (( irq == 28 ) || ( irq == 31 )) {
			ppi = uICRegRead( ICREG_SOC_PPI_CAUSE );
			irq = ICMAX_GIC_IRQ;
			for ( ; ppi ; ppi >>= 1, irq++ ) {
				if (( ppi & 1 ) == 0 )
					continue;

				ret = -1;
				if ( ICVectors[ irq ].Handler != NULL )
					ret = ICVectors[ irq ].Handler( ICVectors[ irq ].UserData );

				if ( ret < 0 )
					iICDisableInt( irq );
			}
		} else {
			ret = -1;
			if ( ICVectors[ irq ].Handler != NULL )
				ret = ICVectors[ irq ].Handler( ICVectors[ irq ].UserData );

			if ( ret < 0 )
				iICDisableInt( irq );
		}
		arm_post_filter( gic_irq );
	}

	/* Reenable AckCtl after FIQ */
	if ( fiq ) {
		ret = arm_gic_read_reg( A9_MPCORE_GIC_CPU + GICC_CTLR );
		ret |= GICC_CTLR_ACKCTL;
		arm_gic_write_reg( A9_MPCORE_GIC_CPU + GICC_CTLR, ret );
	}
}

void vICInit( void )
{
	long irq;

	mvSemaLock( MV_SEMA_IRQ );

	arm_gic_init();

	/* Disable SOC PPI on current CPU */
	for ( irq = 0; irq < ICMAX_SOC_PPI; irq++ )
		vICRegWrite( ICREG_SET_MASK, irq );

	mvSemaUnlock( MV_SEMA_IRQ );
}

inline void vICSendSGI( unsigned long filter, unsigned long target,
		unsigned long sgiid )
{
	arm_send_sgi( filter, target, sgiid );
}

