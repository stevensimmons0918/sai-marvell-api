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
#include "unq.h"

/* defines  */
#undef MV_DEBUG
#ifdef MV_DEBUG
#define DB(x)	x
#else
#define DB(x)
#endif

#define DFX_REG_BASE     0xD0100000
#define SWITCH_REG_BASE  0x48000000

/*******************************************************************************
* ac3WindowConfig.
*
* DESCRIPTION:
*       Configures DFX/SW/PCIe windows on MBus.
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

static MV_VOID windowConfig(MV_VOID)
{
	/* update window0 to DFX - target = 8 attributes = 0x0 */
	MV_REG_WRITE(MV_MBUS_REGS_OFFSET     , 0xF0081);
	MV_REG_WRITE(MV_MBUS_REGS_OFFSET+0x4 , DFX_REG_BASE);
	MV_REG_WRITE(MV_MBUS_REGS_OFFSET+0x8 , 0x0);

	/* update window1 to Switch - target = 3 attributes = 0x0 */
	MV_REG_WRITE(MV_MBUS_REGS_OFFSET+0x10 , 0x3FF0031);
	MV_REG_WRITE(MV_MBUS_REGS_OFFSET+0x14 , SWITCH_REG_BASE);
	MV_REG_WRITE(MV_MBUS_REGS_OFFSET+0x18 , 0x0);

	/* Disable all other windows */
	MV_REG_WRITE(MV_MBUS_REGS_OFFSET+0x20 , 0x0);
	MV_REG_WRITE(MV_MBUS_REGS_OFFSET+0x30 , 0x0);
	MV_REG_WRITE(MV_MBUS_REGS_OFFSET+0x40 , 0x0);
	MV_REG_WRITE(MV_MBUS_REGS_OFFSET+0x50 , 0x0);
	MV_REG_WRITE(MV_MBUS_REGS_OFFSET+0x60 , 0x0);
	
}

/*******************************************************************************
* mvCtrlEnvInit - Initialize Marvell controller environment.
*
* DESCRIPTION:
*	No init required
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
extern int is_micro_init_mode;
MV_STATUS mvCtrlEnvInit(MV_VOID)
{
    if (is_micro_init_mode) {
        windowConfig();

        /* Configure MPP and GPIO related registers per the related processor architecture */
        //unq_setMppAndGpio();
    }

    return MV_OK; 
}


