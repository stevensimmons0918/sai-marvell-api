/*===========================================================================*/
#ifndef exp_host_d_PKS_INCLUDED
#define exp_host_d_PKS_INCLUDED
/*!**************************************************RND Template version 4.1
*!                      P A C K A G E       S P E C I F I C A T I O N
*!==========================================================================
*$ TITLE: host dll pks
*!--------------------------------------------------------------------------
*$ FILENAME: c:\temp\radlan\host_d\EXP\host_d.h
*!--------------------------------------------------------------------------
*$ SYSTEM, SUBSYSTEM: ET16
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
#if (defined WINDOWS_COMPATIBLE)
    #include <conio.h>
#endif /*(defined WINDOWS_COMPATIBLE)*/

#include <GEN/UART/EXP/UART.H>
#include <GEN/FLASH/EXP/FLASH.H>
#include <GEN/INTR/EXP/INTR.H>
#include <GEN/TIMER/EXP/TIMER.H>
#include <LOCAL/H_DATA/EXP/H_DATA.H>
#include <GEN/PSOS/EXP/PSOS.H>
#include <LOCAL/H_CONF/EXP/H_CONF.H>
#include <GEN/NVRAM/EXP/NVRAM.H>
#include <GEN/DRAM/EXP/DRAM.H>
#include <GEN/EPROM/EXP/EPROM.H>
/*#include <h_center\exp\h_center.pks>*/
/*!**************************************************************************
*$              PUBLIC DECLARATIONS (EXPORT)
*!**************************************************************************
*!*/
/*!**************************************************************************
*$              PUBLIC VARIABLE DEFINITIONS (EXPORT)
*!**************************************************************************
*!*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!        P U B L I C   V A R I A B L E   D E C L A R A T I O N S
 *!==========================================================================
 *$ TITLE: <Title>
 *!--------------------------------------------------------------------------
 *$ FILENAME: u:\next\simulat\shost\host_d\inc\host_d.pub
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: ????
 *!--------------------------------------------------------------------------
 *$ AUTHORS: AlexV
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 12-Jul-98  14:01:04    CREATION DATE: 12-Jul-98
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ INCLUDE REQUIRED:
 *!
 *$ REMARKS:
 *!
 *!**************************************************************************
 *!*/

 /*
  * Who initializes:
  * Who reads (uses):
  * Who updates:
  * Protecting mechanism:
 */

extern volatile UINT_32 SHOSTC_d_enable_debug ;
extern volatile UINT_8   *SHOSTC_d_RS_section ;


/*===========================================================================*/
/*!**************************************************************************
*$              PUBLIC FUNCTION DEFINITIONS (EXPORT)
*!**************************************************************************
*!*/
/*===========================================================================*/
/**
* @internal Perror function
* @endinternal
*
*/

extern void Perror(

    /*!     INPUTS:             */

    LPTSTR msg

    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);
/*$ END OF Perror */
/*$ END OF <FuncName> */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTG_halt function
* @endinternal
*
*/

extern void SHOSTG_halt( void )
;
/*$ END OF SHOSTG_halt */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTC_exit function
* @endinternal
*
*/

extern  void SHOSTC_exit(UINT_32  status)
;
/*$ END OF SHOSTC_exit */
/*$ END OF Host_exit */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTG_abort function
* @endinternal
*
*/

extern  void SHOSTG_abort(void)
;
/*$ END OF SHOSTG_abort */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal STARTG_ctrl_handler function
* @endinternal
*
*/

extern BOOL STARTG_ctrl_handler(DWORD type);
/*$ END OF STARTG_ctrl_handler */

/*===========================================================================*/
/*===========================================================================*/
/**
* @internal STARTG_error function
* @endinternal
*
*/

extern  void  STARTG_error(
    /*!     INPUTS:             */
         char   *mes
);
/*$ END OF STARTG_error */
/*$ END OF <FuncName> */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTC_dll_abort function
* @endinternal
*
*/

extern void SHOSTC_dll_abort(
    /* INPUT  */
    UINT_8* msg
);
/*$ END OF SHOSTC_dll_abort */
/*$ END OF SHOSTP_dll_abort */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTG_reset function
* @endinternal
*
*/

extern void SHOSTG_reset (

    /*!     INPUTS:             */

    unsigned int status

);
/*$ END OF SHOSTG_reset */
/*===========================================================================*/

#endif
/*$ END OF host_d */

