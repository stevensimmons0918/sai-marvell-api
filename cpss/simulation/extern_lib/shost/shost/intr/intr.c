
/*!*****************************************************RND Template version 4.0
 *!                      P A C K A G E   B O D Y
 *!=============================================================================
 *$ TITLE: virtual platform.
 *!-----------------------------------------------------------------------------
 *$ FILENAME: c:\projects\simulation\shost\src\gen\intr\intr.c
 *!-----------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: ET16
 *!-----------------------------------------------------------------------------
 *$ AUTHORS: eyran,oren-v,OrenV,BoazK,LiranP
 *!-----------------------------------------------------------------------------
 *$ LATEST UPDATE: 10-Apr-2008, 8:1 AM CREATION DATE: 07-Dec-92
 *!*****************************************************************************
 *!
 *!*****************************************************************************
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ PROCESS AND ALGORITHM: (local)
 *!
 *$ PACKAGE GLOBAL SERVICES:
 *!     (A list of package global services).
 *!
 *$ PACKAGE LOCAL SERVICES:  (local)
 *!     (A list of package local services).
 *!
 *$ PACKAGE USAGE:
 *!     (How to use the package services,
 *!     routines calling order, restrictions, etc.)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ SIDE EFFECTS:
 *!
 *$ RELATED DOCUMENTS:     (local)
 *!
 *$ REMARKS:               (local)
 *!
 *!*****************************************************************************
 *!*/

/*$ OpenGate interrupts  BODY */

/*! General definitions */
#include         <inc\defs.h>

/*!*****************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT AND EXPORT)
 *!*****************************************************************************
 *!*/

#include        <host_d\exp\host_d.h>

/*!*****************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!*****************************************************************************
 *!*/

//#include        <inf\h_sainf\exp\h_sainf.h>
#include        <gen\inc\read_write_lock.mac>
/*
#include        <gen\timer\exp\timer.pks>
#include <local\h_dll\exp\h_dll.pks>
*/

/*!*****************************************************************************
 *$              PUBLIC DECLARATIONS (EXPORT)
 *!*****************************************************************************
 *!*/

#include        <intr\inc\h_intr.cns>
#include        <intr\inc\h_intr_g.stc>

/*!*****************************************************************************
 *$              PUBLIC VARIABLE DEFINITIONS (EXPORT)
 *!*****************************************************************************
 *!*/

/*!*****************************************************************************
 *$              LOCAL DECLARATIONS
 *!*****************************************************************************
 *!*/


#include        <intr\inc\h_intr.stc>

/*!*****************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!*****************************************************************************
 *!*/

#include        <intr\inc\h_intr.lcl>

/*!**************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

#include        <intr\src\h_dosusp.cc>
#include        <intr\src\h_doresu.cc>
#include        <intr\src\h_intthd.cc>

/*!*****************************************************************************
 *$              PUBLIC FUNCTION DEFINITIONS (EXPORT)
 *!*****************************************************************************
 *!*/

#include        <intr\src\h_gointr.cc>
#include        <intr\src\hrsetint.cc>
#include        <intr\src\h_iinit.cc>
#include        <intr\src\h_bindir.cc>
#include        <intr\src\bindparam.cc>
#include        <intr\src\disable.cc>
#include        <intr\src\enable.cc>
#include        <intr\src\chk_int.cc>
#include        <intr\src\h_resint.cc>
#include        <intr\src\h_susint.cc>

/*$ END OF intr */

/* AmitK stubs for CPSS */
void t_reg_task_table(DWORD win_tid, HANDLE hnd, const char *task_name){}
void t_unreg_task_table(DWORD win_tid){}
void t_preemmption(UINT_32 type,UINT_32 *old_ptr){}
void SHOSTC_flash_close (void){}

/* AmitK stubs for simulation */
void SHOSTG_psos_reg_asic_task(void){}
void SHOSTG_psos_pre_init(void){}
