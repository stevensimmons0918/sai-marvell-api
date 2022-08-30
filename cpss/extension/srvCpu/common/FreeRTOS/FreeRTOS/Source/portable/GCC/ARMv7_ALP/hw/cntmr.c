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
#include "common.h"
#include "cntmrregs.h"
#include "cntmr.h"

/* defines  */
#ifdef MV_DEBUG
#define DB( x )	x
#else
#define DB( x )
#endif

#define CNTMR_EVENTS_STATUS_REG_GLOBAL	( MV_CNTMR_REGS_OFFSET + 4 )

#define TIMER_GLOBAL_BIT( timer )		(( timer == MAX_GLOBAL_TIMER ) ? ( 1<<31 ) : ( 1 << ( timer * 8 )))

#if defined( MV88F78X60_Z1 )
#define CNTMR_EVENTS_STATUS_REG_PRIVATE( t )	( MV_CPUIF_REGS_OFFSET( TIMER_TO_CPU( t ) + 0x68 ))
#define TIMER_PRIVATE_BIT( timer )	( 1 << (( CPU_TIMER( timer ) * 8 )))
#elif defined( MV88F78X60 ) && !defined( MV88F78X60_Z1 )
#define CNTMR_EVENTS_STATUS_REG_PRIVATE		( MV_CPUIF_LOCAL_REGS_OFFSET + 0x68 )
#define TIMER_PRIVATE_BIT( timer )	( 1 << (( timer - FIRST_PRIVATE_TIMER ) * 8 ))
#else
#error "No device is defined!"
#endif

/*******************************************************************************
* mvCntmrLoad -
*
* DESCRIPTION:
*       Load an init Value to a given counter/timer
*
* INPUT:
*       countNum - counter number
*       value - value to be loaded
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM on bad parameters , MV_ERROR on error ,MV_OK on sucess
*******************************************************************************/
MV_STATUS mvCntmrLoad( MV_U32 countNum, MV_U32 value )
{
	if ( countNum >= MV_CNTMR_MAX_COUNTER ) {

		DB( mvOsPrintf(( "mvCntmrLoad: Err. illegal counter number \n" )));
		return MV_BAD_PARAM;

	}

	MV_REG_WRITE( CNTMR_RELOAD_REG( countNum ), value );
	MV_REG_WRITE( CNTMR_VAL_REG( countNum ), value );

	return MV_OK;
}

/*******************************************************************************
* mvCntmrRead -
*
* DESCRIPTION:
*  	Returns the value of the given Counter/Timer
*
* INPUT:
*       countNum - counter number
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_U32 counter value
*******************************************************************************/
MV_U32 mvCntmrRead( MV_U32 countNum )
{
	return MV_REG_READ( CNTMR_VAL_REG( countNum ));
}

/*******************************************************************************
* mvCntmrWrite -
*
* DESCRIPTION:
*  	Returns the value of the given Counter/Timer
*
* INPUT:
*       countNum - counter number
*		countVal - value to write
*
* OUTPUT:
*       None.
*
* RETURN:
*       None
*******************************************************************************/
void mvCntmrWrite( MV_U32 countNum, MV_U32 countVal )
{
	MV_REG_WRITE( CNTMR_VAL_REG( countNum ), countVal );
}

/*******************************************************************************
* mvCntmrCtrlSet -
*
* DESCRIPTION:
*  	Set the Control to a given counter/timer
*
* INPUT:
*       countNum - counter number
*		pCtrl - pointer to MV_CNTMR_CTRL structure
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM on bad parameters , MV_ERROR on error ,MV_OK on sucess
*******************************************************************************/
MV_STATUS mvCntmrCtrlSet( MV_U32 countNum, MV_CNTMR_CTRL *pCtrl )
{
	MV_U32 cntmrCtrl;

	if ( countNum >= MV_CNTMR_MAX_COUNTER ) {
		DB( mvOsPrintf(( "mvCntmrCtrlSet: Err. illegal counter number \n" )));
		return MV_BAD_PARAM;
	}

	/* read control register */
	cntmrCtrl = MV_REG_READ( CNTMR_CTRL_REG( countNum ));
	cntmrCtrl &= ~(( CTCR_ARM_TIMER_EN_MASK( countNum )) | ( CTCR_ARM_TIMER_AUTO_MASK( countNum )));

	if ( pCtrl->enable )	/* enable counter\timer */
		cntmrCtrl |= ( CTCR_ARM_TIMER_EN( countNum ));

	if ( pCtrl->autoEnable )	/* Auto mode */
		cntmrCtrl |= ( CTCR_ARM_TIMER_AUTO_EN( countNum ));

#ifndef MV88F78X60_Z1
	cntmrCtrl &= ~(( CTCR_ARM_TIMER_RATIO_MASK( countNum )) | ( CTCR_ARM_TIMER_25MhzFRQ_MASK( countNum )));

	cntmrCtrl |= ( pCtrl->Ratio & 0x7 ) << ( CTCR_ARM_TIMER_RATIO_OFFS( countNum ));

	if ( pCtrl->enable_25Mhz )	/* 25Mhz enable */
		cntmrCtrl |= ( CTCR_ARM_TIMER_25MhzFRQ_EN( countNum ));

#endif


	MV_REG_WRITE( CNTMR_CTRL_REG( countNum ), cntmrCtrl );

	return MV_OK;

}

/*******************************************************************************
* mvCntmrCtrlGet -
*
* DESCRIPTION:
*  	Get the Control value of a given counter/timer
*
* INPUT:
*       countNum - counter number
*		pCtrl - pointer to MV_CNTMR_CTRL structure
*
* OUTPUT:
*       Counter\Timer control value
*
* RETURN:
*       MV_BAD_PARAM on bad parameters , MV_ERROR on error ,MV_OK on sucess
*******************************************************************************/
MV_STATUS mvCntmrCtrlGet( MV_U32 countNum, MV_CNTMR_CTRL *pCtrl )
{
	MV_U32 cntmrCtrl;

	if ( countNum >= MV_CNTMR_MAX_COUNTER ) {
		DB( mvOsPrintf(( "mvCntmrCtrlGet: Err. illegal counter number \n" )));
		return MV_BAD_PARAM;
	}

	/* read control register */
	cntmrCtrl = MV_REG_READ( CNTMR_CTRL_REG( countNum ));

	/* enable counter\timer */
	if ( cntmrCtrl & ( CTCR_ARM_TIMER_EN( countNum )))
		pCtrl->enable = MV_TRUE;
	else
		pCtrl->enable = MV_FALSE;

	/* counter mode */
	if ( cntmrCtrl & ( CTCR_ARM_TIMER_AUTO_EN( countNum )))
		pCtrl->autoEnable = MV_TRUE;
	else
		pCtrl->autoEnable = MV_FALSE;


#ifndef MV88F78X60_Z1
	pCtrl->Ratio = ( cntmrCtrl & ( CTCR_ARM_TIMER_RATIO_MASK( countNum ))) / ( 1 << ( CTCR_ARM_TIMER_RATIO_OFFS( countNum )));

	pCtrl->enable_25Mhz = ( cntmrCtrl & CTCR_ARM_TIMER_25MhzFRQ_MASK( countNum )) ? MV_TRUE : MV_FALSE;
#endif

	return MV_OK;
}

/*******************************************************************************
* mvCntmrEnable -
*
* DESCRIPTION:
*  	Set the Enable-Bit to logic '1' ==> starting the counter
*
* INPUT:
*       countNum - counter number
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM on bad parameters , MV_ERROR on error ,MV_OK on sucess
*******************************************************************************/
MV_STATUS mvCntmrEnable( MV_U32 countNum )
{
	MV_U32 cntmrCtrl;

	if ( countNum >= MV_CNTMR_MAX_COUNTER ) {

		DB( mvOsPrintf(( "mvCntmrEnable: Err. illegal counter number \n" )));
		return MV_BAD_PARAM;

	}

	/* read control register */
	cntmrCtrl = MV_REG_READ( CNTMR_CTRL_REG( countNum ));

	/* enable counter\timer */
	cntmrCtrl |= ( CTCR_ARM_TIMER_EN( countNum ));

	MV_REG_WRITE( CNTMR_CTRL_REG( countNum ), cntmrCtrl );

	return MV_OK;
}

/*******************************************************************************
* mvCntmrDisable -
*
* DESCRIPTION:
*  	Stop the counter/timer running, and returns its Value
*
* INPUT:
*       countNum - counter number
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_U32 counter\timer value
*******************************************************************************/
MV_STATUS mvCntmrDisable( MV_U32 countNum )
{
	MV_U32 cntmrCtrl;

	if ( countNum >= MV_CNTMR_MAX_COUNTER ) {

		DB( mvOsPrintf(( "mvCntmrDisable: Err. illegal counter number \n" )));
		return MV_BAD_PARAM;

	}

	/* read control register */
	cntmrCtrl = MV_REG_READ( CNTMR_CTRL_REG( countNum ));

	/* disable counter\timer */
	cntmrCtrl &= ~( CTCR_ARM_TIMER_EN( countNum ));

	MV_REG_WRITE( CNTMR_CTRL_REG( countNum ), cntmrCtrl );

	return MV_OK;
}

/*******************************************************************************
* mvCntmrStart -
*
* DESCRIPTION:
*  	Combined all the sub-operations above to one function: Load,setMode,Enable
*
* INPUT:
*       countNum - counter number
*		value - value of the counter\timer to be set
*		pCtrl - pointer to MV_CNTMR_CTRL structure
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM on bad parameters , MV_ERROR on error ,MV_OK on sucess
*******************************************************************************/
MV_STATUS mvCntmrStart( MV_U32 countNum, MV_U32 value, MV_CNTMR_CTRL *pCtrl )
{

	if ( countNum >= MV_CNTMR_MAX_COUNTER ) {

		DB( mvOsPrintf(( "mvCntmrDisable: Err. illegal counter number \n" )));
		return MV_BAD_PARAM;

	}

	/* load value onto counter\timer */
	mvCntmrLoad( countNum, value );

	/* set control for timer \ cunter and enable */
	mvCntmrCtrlSet( countNum, pCtrl );

	return MV_OK;
}

/*******************************************************************************
* mvCntmrIntClear - Clear an Counter/Timer interrupt
*
* DESCRIPTION:
*       This routine clears a specified Counter/Timer termination event in
*       Counter/Timer cause register.
*       The routine will preform argument validity check.
*
* INPUT:
*       cntmrNum - Counter/Timer number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       OK    - If the bit was set
*       ERROR - In case of invalid parameters.
*
*******************************************************************************/
MV_STATUS mvCntmrIntClear( MV_U32 cntmrNum )
{
	if ( INVALID_CNTMR( cntmrNum ))
		return MV_ERROR;

	if ( cntmrNum <= MAX_GLOBAL_TIMER ) {
		/* Reset cause bit to acknowledge interrupt */
		MV_REG_WRITE( CNTMR_EVENTS_STATUS_REG_GLOBAL, ~( TIMER_GLOBAL_BIT( cntmrNum )));
	} else {
#ifdef MV88F78X60_Z1
		MV_REG_WRITE( CNTMR_EVENTS_STATUS_REG_PRIVATE( cntmrNum ), ~( TIMER_PRIVATE_BIT( cntmrNum )));
#else
		MV_REG_WRITE( CNTMR_EVENTS_STATUS_REG_PRIVATE, ~( TIMER_PRIVATE_BIT( cntmrNum )));
#endif
	}
	return MV_OK;
}
