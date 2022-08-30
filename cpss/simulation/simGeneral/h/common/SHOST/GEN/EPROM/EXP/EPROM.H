/*===========================================================================*/
#ifndef exp_eprom_PKS_INCLUDED
#define exp_eprom_PKS_INCLUDED
/*!**************************************************RND Template version 4.1
*!                      P A C K A G E       S P E C I F I C A T I O N
*!==========================================================================
*$ TITLE: <Title>
*!--------------------------------------------------------------------------
*$ FILENAME: c:\temp\radlan\gen\eprom\EXP\eprom.h
*!--------------------------------------------------------------------------
*$ SYSTEM, SUBSYSTEM: ????
*!--------------------------------------------------------------------------
*$ AUTHORS: AlexV     
*!--------------------------------------------------------------------------
*$ LATEST UPDATE:                              CREATION DATE: 15-Jul-98
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
* @internal SHOSTG_eprom_get_mac function
* @endinternal
*
*/

extern void SHOSTG_eprom_get_mac (

    /*!     INPUTS:             */

    void                *mac_addr_PTR,

    UINT_32             *num_mac_addr_PTR

    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);
/*$ END OF SHOSTG_eprom_get_mac */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTC_eprom_init function
* @endinternal
*
*/

extern void SHOSTC_eprom_init (

    /*!     INPUTS:             */

    void

    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);
/*$ END OF SHOSTC_eprom_init */
/*===========================================================================*/

#endif
/*$ END OF eprom */

