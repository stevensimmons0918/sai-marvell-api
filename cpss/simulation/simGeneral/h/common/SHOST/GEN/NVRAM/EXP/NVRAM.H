/*===========================================================================*/
#ifndef exp_nvram_PKS_INCLUDED
#define exp_nvram_PKS_INCLUDED
/*!**************************************************RND Template version 4.1
*!                      P A C K A G E       S P E C I F I C A T I O N
*!==========================================================================
*$ TITLE: NVRAM pkb 
*!--------------------------------------------------------------------------
*$ FILENAME: c:\temp\radlan\gen\nvram\EXP\nvram.h
*!--------------------------------------------------------------------------
*$ SYSTEM, SUBSYSTEM: HOST Dll
*!--------------------------------------------------------------------------
*$ AUTHORS: eyran 
*!--------------------------------------------------------------------------
*$ LATEST UPDATE:                              CREATION DATE: 16-Jan-94
*!**************************************************************************
*!
*!**************************************************************************
*!
*$ GENERAL DESCRIPTION:
*!  
*! PROCESS AND ALGORITHM: (local)
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
*!**************************************************************************
*!*/
/*!**************************************************************************
*$              EXTERNAL DECLARATIONS (IMPORT AND EXPORT)
*!**************************************************************************
*!*/
#include <HOST_D/EXP/HOST_D.H>
/*!**************************************************************************
*$              PUBLIC DECLARATIONS (EXPORT)
*!**************************************************************************
*!*/
/*!**************************************************************************
*$              PUBLIC VARIABLE DEFINITIONS (EXPORT)
*!**************************************************************************
*!*/
/*!**************************************************************************
*$              PUBLIC FUNCTION DEFINITIONS (EXPORT)
*!**************************************************************************
*!*/
/*===========================================================================*/
/**
* @internal SHOSTC_nvram_close function
* @endinternal
*
*/

extern void SHOSTC_nvram_close (
    void

);
/*$ END OF SHOSTC_nvram_close */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTG_nvram_init function
* @endinternal
*
*/

extern void  SHOSTG_nvram_init (
             void **nvram_base_addr_ptr_PTR,
             UINT_32 *nvram_size_PTR
);
/*$ END OF SHOSTG_nvram_init */

/*===========================================================================*/

#endif
/*$ END OF nvram */

