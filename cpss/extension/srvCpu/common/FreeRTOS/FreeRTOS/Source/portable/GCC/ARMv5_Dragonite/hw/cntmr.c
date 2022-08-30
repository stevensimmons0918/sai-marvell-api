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
#include "cntmr.h"

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
		return MV_BAD_PARAM;;
	}

	MV_REG_WRITE( TMREG_RELOAD( countNum ), value );
	MV_REG_WRITE( TMREG_VALUE( countNum ), value );

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
	return MV_REG_READ( TMREG_VALUE( countNum ));
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
	MV_REG_WRITE( TMREG_VALUE( countNum ), countVal );
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
		return MV_BAD_PARAM;
	}

	/* read control register */
	cntmrCtrl = MV_REG_READ( TMREG_CONTROL );

	/* enable counter\timer */
	cntmrCtrl |= TMBIT_CTRL_EN( countNum );

	MV_REG_WRITE( TMREG_CONTROL, cntmrCtrl );

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
		return MV_BAD_PARAM;
	}

	/* read control register */
	cntmrCtrl = MV_REG_READ( TMREG_CONTROL );

	/* enable counter\timer */
	cntmrCtrl &= ~TMBIT_CTRL_EN( countNum );

	MV_REG_WRITE( TMREG_CONTROL, cntmrCtrl );

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
MV_STATUS mvCntmrSetup( MV_U32 countNum, MV_U32 value,
		MV_U32 enable, MV_U32 autoReload, MV_U32 wdtReset )
{
	MV_U32 ctrl;

	if ( countNum >= MV_CNTMR_MAX_COUNTER )
		return MV_BAD_PARAM;

	/* load value onto timer */
	mvCntmrLoad( countNum, value );

	/* read control register */
	ctrl = MV_REG_READ( TMREG_CONTROL );

	/* configure timer */
	if ( enable )
		ctrl |= TMBIT_CTRL_EN( countNum );
	else
		ctrl &= ~TMBIT_CTRL_EN( countNum );
	if ( autoReload )
		ctrl |= TMBIT_CTRL_AUTO( countNum );
	else
		ctrl &= ~TMBIT_CTRL_AUTO( countNum );
	if ( countNum == TIMER3 ) {
		if ( wdtReset )
			ctrl |= TMBIT_CTRL_WDRST;
		else
			ctrl &= ~TMBIT_CTRL_WDRST;
	}

	MV_REG_WRITE( TMREG_CONTROL, ctrl );

	return MV_OK;
}

