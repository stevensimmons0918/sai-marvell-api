/*!**************************************************************************
 *!                  P A C K A G E    B O D Y
 *!==========================================================================
 *$ TITLE: I/O utils for 80960
 *!--------------------------------------------------------------------------
 *$ FILENAME: l:\av01\local\h_conf\h_conf.pkb
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: ET16
 *!--------------------------------------------------------------------------
 *$ AUTHORS: avi_c
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 14-Jan-97  17:36:55          CREATION DATE: 20-Sep-92
 *!--------------------------------------------------------------------------
 *$ HISTORY:   $LOG$
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
*/
/*$ IO BODY */

/*!     General definitions                 */
#include    <inc\defs.h>
#include    <ctype.h>

/*!**************************************************************************
 *$            EXTERNAL DECLARATIONS (IMPORT AND EXPORT)
 *!**************************************************************************
 *!*/

    #include <host_d\exp\host_d.h>

/*!**************************************************************************
 *$            EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/

/*!**************************************************************************
 *$            PUBLIC DECLARATIONS  (EXPORT)
 *!**************************************************************************
 *!*/

/*!**************************************************************************
 *$            PUBLIC VARIABLE DEFINITIONS (EXPORT)
 *!**************************************************************************
 *!*/


/*!****************************************************************************
 *$            LOCAL  DECLARATIONS
 *!*****************************************************************************
 *!*/

#include  <h_conf\inc\config.pub>

/*!****************************************************************************
 *$            LOCAL VARIABLE DEFINITIONS
 *!*****************************************************************************
 *!*/

/*!**************************************************************************
 *$            LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*!**************************************************************************
 *$            PUBLIC FUNCTION DEFINITIONS (EXPORT)
 *!**************************************************************************
 *!*/

#include <h_conf\src\chck_str.cc>
#include <h_conf\src\get_val.cc>
#include <h_conf\src\set_val.cc>
#include <h_conf\src\set_file.cc>
#include <h_conf\src\backup.cc>
/*$ end of h_conf */
