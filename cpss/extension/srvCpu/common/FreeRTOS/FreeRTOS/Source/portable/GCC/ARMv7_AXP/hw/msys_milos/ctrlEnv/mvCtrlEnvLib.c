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

/* includes */
#include "common.h"
#include "mvCtrlEnvLib.h"
/*#include "boardEnv/mvBoardEnvLib.h"*/

/* defines  */
#undef MV_DEBUG
#ifdef MV_DEBUG
#define DB(x)	x
#else
#define DB(x)
#endif

#define DFX_REG_BASE                        0x80000000

/*******************************************************************************
* dfxWinConfig.
*
* DESCRIPTION:
*       Configures DFX window on MBus.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
void dfxWinConfig()
{
	/* update window0 to DFX - target = 8 attributes = 0x0 */
	MV_REG_WRITE(MV_MBUS_REGS_OFFSET,	 0xF0081);

	MV_REG_WRITE(MV_MBUS_REGS_OFFSET+0x10, 0xF0051 );    /* Enable window b0, BAR1 b1, target=5 b7:4, Attr=0 b15:8, size=1MB b31:16 */  
	MV_REG_WRITE(MV_MBUS_REGS_OFFSET+0x18, 0xFA000001 ); /* Enable remap, remap to address 0xFA000000 */
}

/*******************************************************************************
* mvCtrlEnvInit - Initialize Marvell controller environment.
*
* DESCRIPTION:
*       This function get environment information and initialize controller
*       internal/external environment. For example
*       1) MPP settings according to board MPP macros.
*		NOTE: It is the user responsibility to shut down all DMA channels
*		in device and disable controller sub units interrupts during
*		boot process.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_STATUS mvCtrlEnvInit(MV_VOID)
{
	dfxWinConfig();

	return MV_OK;
}



/*******************************************************************************
* mvServerRegRead.
*
* DESCRIPTION:
*       Read register from DFX 
*
* INPUT:
*       addr
*
* OUTPUT:
*       data
*
* RETURN:
*       None.
*
*******************************************************************************/
void mvServerRegRead(unsigned int addr, unsigned int *data)
{
	addr |= DFX_REG_BASE;
	*data = *(( volatile unsigned * )( DFX_REG_BASE | addr));
}


/*******************************************************************************
* mvServerRegWrite.
*
* DESCRIPTION:
*       Write register from DFX 
*
* INPUT:
*       addr
*		data
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
void mvServerRegWrite(unsigned int addr, unsigned int data)
{
	addr |= DFX_REG_BASE;
	*(( volatile unsigned * )( DFX_REG_BASE | addr)) = data;
}


