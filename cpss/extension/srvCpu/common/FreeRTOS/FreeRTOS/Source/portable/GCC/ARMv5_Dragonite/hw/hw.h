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

#ifndef HW_H
#define HW_H

#include "common.h"
#include "ic.h"
#include "uart.h"
#include "cntmr.h"

#define DRAGONITE_DTCM_OFFSET		( 0x04000000 )
#define DRAGONITE_DTCM_SIZE		( 32 * 1024 )

/* GPP Control */

#define GPP_CONTROL			( 0x10 )
#define GPP_DIRECT2CHIP_BIT		( 1 << 8 )

/* MPP Control */
#define MPPREG_BASE			( 0x18000 )
#define MPPREG_CONTROL( i )		( MPPREG_BASE + ((( i ) & ~0x07 ) >> 1 ))
#define MPPMASK_PIN( i )		( 0x0F << ((( i ) & 0x07) << 2))

/* GPIO control */
#define GPIOREG_BASE			( 0x18100 )
#define GPIOREG_DATA_OUT( i )		( GPIOREG_BASE + ((( i ) & ~0x1F ) << 1 ))
#define GPIOBIT_OUT( p )		( 1 << (( p ) & 0x1F ) )

/* TDM Control */
#define TDM_CTRL_REG			( 0x18200 )
#define TDM_CTRL_SSI_INT_RATE_MASK	( 1 << 28 )
#define TDM_CTRL_SSI_INT_RATE_2KHZ	( 1 << 28 )
#define TMD_CTRL_SSI_INT_RATE_8KHZ	( 0 << 28 )

/* SSI Interrupt Management */
#define SSI_INT_CAUSE_REG		( 0x18260 )
#define SSI_INT_MASK_REG		( 0x18264 )
#define SSI_INT_PERIODIC		( 1 << 7 )

#endif
