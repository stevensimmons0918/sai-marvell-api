/*!**************************************************RND Template version 4.1
 *!                      P A C K A G E   B O D Y
 *!==========================================================================
 *$ TITLE: host dll pks 
 *!--------------------------------------------------------------------------
 *$ FILENAME: m:\sw_gen\simulat\shost\host_d\host_d.pkb
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: ET16
 *!--------------------------------------------------------------------------
 *$ AUTHORS: eyran 
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 01-Feb-:0  16:38:09          CREATION DATE: 16-Jan-94
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

/*$ host dll BODY */

/*! General definitions */
#include         <inc\defs.h>

/*!**************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT AND EXPORT)
 *!**************************************************************************
 *!*/
#include <conio.h>
#include        <host_d\exp\host_d.h>

//#include <gen\uart\exp\uart.h>
//#include <gen\flash\exp\flash.h>
//#include <gen\intr\exp\intr.h>
//#include <gen\timer\exp\timer.h>
//#include <local\h_data\exp\h_data.h>
//#include <gen\psos\exp\psos.h>
//#include <local\h_conf\exp\h_conf.h>
//#include <gen\nvram\exp\nvram.h>
//#include <gen\dram\exp\dram.h>
//#include <gen\eprom\exp\eprom.h>
//#include <h_center\exp\h_center.pks>


/*!**************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/

//#include <inf\h_sainf\exp\h_sainf.h>

/*!**************************************************************************
 *$              PUBLIC DECLARATIONS (EXPORT)
 *!**************************************************************************
 *!*/


/*!**************************************************************************
 *$              PUBLIC VARIABLE DEFINITIONS (EXPORT)
 *!**************************************************************************
 *!*/

#include    <host_d\inc\host_d.pub>

/*!**************************************************************************
 *$              LOCAL DECLARATIONS
 *!**************************************************************************
 *!*/


/*!**************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/


/*!**************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS 
 *!**************************************************************************
 *!*/



/*!**************************************************************************
 *$              PUBLIC FUNCTION DEFINITIONS (EXPORT)
 *!**************************************************************************
 *!*/

#include    <host_d\src\perror.cc>
#include    <host_d\src\halt.cc>
#include    <host_d\src\exit.cc>
#include    <host_d\src\abort.cc>
#include    <host_d\src\signal.cc>
#include    <host_d\src\error.cc>
#include    <host_d\src\dll_abrt.cc>
#include    <host_d\src\reset.cc>

/*$ END OF host dll */
