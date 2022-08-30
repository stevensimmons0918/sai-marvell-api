/*===========================================================================*/
#ifndef exp_dram_PKS_INCLUDED
#define exp_dram_PKS_INCLUDED
/*!**************************************************RND Template version 4.1
*!                      P A C K A G E       S P E C I F I C A T I O N
*!==========================================================================
*$ TITLE:
*!--------------------------------------------------------------------------
*$ FILENAME: c:\temp\radlan\gen\dram\EXP\dram.h
*!--------------------------------------------------------------------------
*$ SYSTEM, SUBSYSTEM: ????
*!--------------------------------------------------------------------------
*$ AUTHORS: AlexV
*!--------------------------------------------------------------------------
*$ LATEST UPDATE:                              CREATION DATE: 01-Jul-98
*!**************************************************************************
*!
*!**************************************************************************
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
*!**************************************************************************
*!*/
/*!**************************************************************************
*$              EXTERNAL DECLARATIONS (IMPORT AND EXPORT)
*!**************************************************************************
*!*/
#if (defined WINDOWS_COMPATIBLE)
    #include <conio.h>
#endif /*(defined WINDOWS_COMPATIBLE)*/

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
* @internal SHOSTG_dram_init function
* @endinternal
*
*/

extern void SHOSTG_dram_init(

    /*!     INPUTS:             */
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */

    void            **dram_base_addr_ptr_PTR,

    UINT_32         *dram_size_PTR
);
/*$ END OF SHOSTG_dram_init */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTC_dram_close function
* @endinternal
*
*/

extern void SHOSTC_dram_close (

    /*!     INPUTS:             */

    void

    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);
/*$ END OF SHOSTC_dram_close */
/*===========================================================================*/

#endif
/*$ END OF dram */

