#ifndef RS_INC_PKS_INCLUDED
#define RS_INC_PKS_INCLUDED

/*!**************************************************RND Template version 4.1
 *!                  P A C K A G E   S P E C I F I C A T I O N
 *!==========================================================================
 *$ TITLE: General definitions
 *!--------------------------------------------------------------------------
 *$ FILENAME: routers\src\inc\defs.h
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: Router Server, General definitions
 *!--------------------------------------------------------------------------
 *$ AUTHORS: Michael,RazA
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 26-Dec-2011                   CREATION DATE: 29-Dec-94
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ PACKAGE GLOBAL SERVICES:
 *!
 *$ PACKAGE USAGE:
 *!
 *$ ASSUMPTIONS:
 *!
 *$ SIDE EFFECTS:
 *!
 *!**************************************************************************
 *!*/

/*!**************************************************************************
 *$            EXTERNAL DECLARATIONS (IMPORT AND EXPORT)
 *!**************************************************************************
 *!*/


/*!**************************************************************************
 *$            PUBLIC DECLARATIONS (EXPORT)
 *!**************************************************************************
 *!*/


#ifdef PSS_PPC
 #include <inc/psos/bsp.h>
 #include <inc/psos/psos.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include <string.h>
#include <stddef.h>
#include <stdarg.h>

#if defined (WORKBENCH_31) || defined (WORKBENCH_33)
 #define WORKBENCH
#endif

#include <inc/defs1.dtd>

#ifdef VXARM
 #undef I960
 #include <inc/private/vxarm.dtd>
#endif

#ifdef VXMIPS
 #undef I960
 #include <inc/private/vxmips.dtd>
#endif

#ifdef VXPPC
 #undef I960
 #include <inc/private/vxppc.dtd>
#endif

#ifdef PSS_MIPS
 #include <inc/private/idtmips.dtd>
#endif

#ifdef PSS_PPC
 #include <inc/private/psppc.dtd>
#endif

#ifdef I960
 #include <inc/private/i960.dtd>
#endif

#ifdef M68K
 #include <inc/private/m68k.dtd>
#endif

#ifdef _WIN32
    #ifdef _VISUALC
        #include <memory.h>
    #else
        #include <mem.h>
    #endif
 #include <windows.h>
    #if defined(_VISUALC) && defined(_VC_VER_10)
        #include <inc/private/_vc.dtd>
    #else
        #include <inc/private/_win32.dtd>
    #endif
#endif

#ifdef LINUX_PPC
 #include <inc/private/linux_ppc.dtd>
#endif

#ifdef LINUX_ARM
 #include <inc/private/linux_arm.dtd>
#endif

#if (defined ASIC_SIMULATION) && (defined LINUX)
 #include <inc/private/linux_sim.dtd>
#endif

#ifdef LINUX_MIPS
 #include <inc/private/linux_mips.dtd>
#endif

#include <inc/defs2.dtd>

/*!**************************************************************************
 *$            PUBLIC VARIABLE DEFINITIONS (EXPORT)
 *!**************************************************************************
 *!*/

/*!**************************************************************************
 *$            PUBLIC FUNCTION DEFINITIONS (EXPORT)
 *!**************************************************************************
 *!*/

/*$ END OF RS_INC */

#endif

