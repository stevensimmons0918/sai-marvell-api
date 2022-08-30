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
#include "sysmap.h"

/* MG base address */
#define MG_BASE     0x60000000

/* MG offsets */
#define REG_MBUS_SYSTEM_INT_WINDOW_CONFIG   0x334

/* configuration parameters */
#define CM3_INTERNAL_WIN_SIZE   _1M
#define CM3_INTERNAL_WIN_BASE   0x0

#ifdef CONFIG_RAVEN
#define SD1_XBAR_TARGET         0x3
#define GOP0_XBAR_TARGET        0x4
#define GOP1_XBAR_TARGET        0x5
#define SERDES_XBAR_TARGET      0x6
#define DFX_XBAR_TARGET         0x7
#define CM3_WINDOW_SIZE         0xf0000
#endif
#define cm3_internal_win_conf_write( val )  \
    (*(( volatile unsigned long * )(MG_BASE | REG_MBUS_SYSTEM_INT_WINDOW_CONFIG ))) = (val)

#define MV_MEMIO32_WRITE(addr, data) ((*((volatile unsigned int*)(addr))) = ((unsigned int)(data)))

extern int is_micro_init_mode;

static void cm3_to_msys_inter_regs(unsigned long base, unsigned long size)
{
#if defined(CONFIG_BOBK) || defined(CONFIG_AC5)
    if (size < _128K)
        size = _128K;

    if (base >= _128K)
        base = _128K - 1;
#elif defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT)
  if (size < _192K)
        size = _192K;

    if (base >= _192K)
        base = _192K - 1;
#else /*Aldrin2*/
  if (size < _384K)
        size = _384K;

    if (base >= _384K)
        base = _384K - 1;
#endif

    cm3_internal_win_conf_write(size | base);
}

void cm3_sysmap_init(void)
{
#ifdef CONFIG_RAVEN
    MV_U32 val;
#endif
    cm3_to_msys_inter_regs(CM3_INTERNAL_WIN_BASE, CM3_INTERNAL_WIN_SIZE);
#ifndef CONFIG_RAVEN
    if (is_micro_init_mode) {
        /* configure illegal access default target */
        MV_MEMIO32_WRITE((MG_BASE | 0x200), 0);
        MV_MEMIO32_WRITE((MG_BASE | 0x204), 0xe1);
        MV_MEMIO32_WRITE((MG_BASE | 0x340), 0);

        /* configure window 1 to SPI flash */
        MV_MEMIO32_WRITE((MG_BASE | 0x490), 0x10001e01); /* Base addr + Target, accessed @ 0xb0100000 */
        MV_MEMIO32_WRITE((MG_BASE | 0x494), 0x00ff0000); /* Size = 16MB */
        MV_MEMIO32_WRITE((MG_BASE | 0x498), 0x00000000); /* High base addr */
        MV_MEMIO32_WRITE((MG_BASE | 0x49c), 0x0000000e); /* Remap */

        /* RAZA - TODO: next 2 lines should be removed!
         * temporary to support load from remote
         * Noramlly, this should be set by booton */
        /*MV_MEMIO32_WRITE((MSS_REGS_BASE | 0x18000), 0x00222222);  Set MPP[5:0] to SPI */
    }

    /* configure window 0 to DFX server */
        /* 0xA000,0000 (CM3) + 0x0010,0000 (External_windows) + 0x000x,xxxx (Windows offset)   */
        /* The address of the DFX from the CM3 POV - (0xA010,0000 - 0xA01F,FFFF)*/
        /* The address of the accesses in DFX is   - (0x0000,0000 - 0x000F,FFFF)*/
#ifdef AC5_DEV_SUPPORT
    MV_MEMIO32_WRITE((MG_BASE | 0x480), 0x00000009); /* Base addr + Target, accessed @ 0xa0100000 */
    MV_MEMIO32_WRITE((MG_BASE | 0x4a0), 0x0020000D); /* Base addr + Target, accessed @ 0xa0300000 */
    MV_MEMIO32_WRITE((MG_BASE | 0x4a4), 0x000f0000); /* Size = 1MB */
    MV_MEMIO32_WRITE((MG_BASE | 0x4a8), 0x00000000); /* High base addr - this window is for general switch access */

    MV_MEMIO32_WRITE((MG_BASE | 0x4c0), 0x1000000D); /* Base addr + Target, accessed @ 0xb0100000 */
    MV_MEMIO32_WRITE((MG_BASE | 0x4c4), 0x000f0000); /* Size = 1MB */
    MV_MEMIO32_WRITE((MG_BASE | 0x4c8), 0x00000000); /* High base addr - this window is for general switch access */
    MV_MEMIO32_WRITE((MG_BASE | 0x4cc), 0x8400000e); /* Low Remap address */

#else
    MV_MEMIO32_WRITE((MG_BASE | 0x480), 0x00000008); /* Base addr + Target, accessed @ 0xa0100000 */
#endif
    MV_MEMIO32_WRITE((MG_BASE | 0x484), 0x000f0000); /* Size = 1MB */
    MV_MEMIO32_WRITE((MG_BASE | 0x488), 0x00000000); /* High base addr */
    MV_MEMIO32_WRITE((MG_BASE | 0x48c), 0x0000000e); /* Remap */
#endif
#ifdef CONFIG_RAVEN
    /* the uart_int_sel bit needs to reset,  this register is to config the uart interrupt from inside or outside,
        the default mode is from outside */
    val = MV_MEMIO32_READ(INTER_REGS_BASE | 0x18210);
    MV_MEMIO32_WRITE((INTER_REGS_BASE | 0x18210), (val & ~0x10));

    /* open window for SD1 from CM3 */
    MV_MEMIO32_WRITE((MG_BASE | 0x480), SD1_XBAR_TARGET); /* Base addr + Target, accessed @ 0xa0100000 */
    MV_MEMIO32_WRITE((MG_BASE | 0x484), CM3_WINDOW_SIZE); /* Size = 1MB */
    MV_MEMIO32_WRITE((MG_BASE | 0x488), 0x00000000); /* High base addr */
    MV_MEMIO32_WRITE((MG_BASE | 0x48c), 0x0000000e); /* Remap */

    /* open window for GOP 0 from CM3 */
    MV_MEMIO32_WRITE((MG_BASE | 0x490), 0x00100000 | GOP0_XBAR_TARGET); /* Base addr + Target, accessed @ 0xa0200000 */
    MV_MEMIO32_WRITE((MG_BASE | 0x494), CM3_WINDOW_SIZE); /* Size = 1MB */
    MV_MEMIO32_WRITE((MG_BASE | 0x498), 0x00000000); /* High base addr */
    MV_MEMIO32_WRITE((MG_BASE | 0x49c), 0x0000000e); /* Remap */

    /* open window for GOP 1 from CM3 */
    MV_MEMIO32_WRITE((MG_BASE | 0x4a0), 0x00200000 | GOP1_XBAR_TARGET); /* Base addr + Target, accessed @ 0xa0300000 */
    MV_MEMIO32_WRITE((MG_BASE | 0x4a4), CM3_WINDOW_SIZE); /* Size = 1MB */
    MV_MEMIO32_WRITE((MG_BASE | 0x4a8), 0x00000000); /* High base addr */
    MV_MEMIO32_WRITE((MG_BASE | 0x4ac), 0x0000000e); /* Remap */

    /* open window for serdes from CM3 */
    MV_MEMIO32_WRITE((MG_BASE | 0x4b0), 0x00300000 | SERDES_XBAR_TARGET); /* Base addr + Target, accessed @ 0xa0400000 */
    MV_MEMIO32_WRITE((MG_BASE | 0x4b4), CM3_WINDOW_SIZE); /* Size = 1MB */
    MV_MEMIO32_WRITE((MG_BASE | 0x4b8), 0x00000000); /* High base addr */
    MV_MEMIO32_WRITE((MG_BASE | 0x4bc), 0x0000000e); /* Remap */

    /* open window for DFX from CM3 */
    MV_MEMIO32_WRITE((MG_BASE | 0x4c0), 0x00400000 | DFX_XBAR_TARGET); /* Base addr + Target, accessed @ 0xa0500000 */
    MV_MEMIO32_WRITE((MG_BASE | 0x4c4), CM3_WINDOW_SIZE); /* Size = 1MB */
    MV_MEMIO32_WRITE((MG_BASE | 0x4c8), 0x00000000); /* High base addr */
    MV_MEMIO32_WRITE((MG_BASE | 0x4cc), 0x0000000e); /* Remap */

#endif
}


