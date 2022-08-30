/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* smain.h
*
* DESCRIPTION:
*       This is a external API definition for SInit module of Simulation.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 4 $
*
*******************************************************************************/
#ifndef __sinith
#define __sinith

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <os/simTypesBind.h>

/**
* @internal SASICG_SIMULATION_ROLES_ENT function
* @endinternal
*
*/
typedef enum {
    SASICG_SIMULATION_ROLE_NON_DISTRIBUTED_E,
    SASICG_SIMULATION_ROLE_DISTRIBUTED_APPLICATION_SIDE_E,
    SASICG_SIMULATION_ROLE_DISTRIBUTED_ASIC_SIDE_E,
    SASICG_SIMULATION_ROLE_DISTRIBUTED_INTERFACE_BUS_BRIDGE_E,
    SASICG_SIMULATION_ROLE_BROKER_E,
    SASICG_SIMULATION_ROLE_DISTRIBUTED_APPLICATION_SIDE_VIA_BROKER_E,
    SASICG_SIMULATION_ROLE_DISTRIBUTED_ASIC_SIDE_VIA_INTERFACE_BUS_BRIDGE_E,

    SASICG_SIMULATION_ROLE_LAST_E
}SASICG_SIMULATION_ROLES_ENT;

/* is the dist role application */
extern GT_BOOL sasicgSimulationRoleIsApplication;
/* is the dist role device(s) */
extern GT_BOOL sasicgSimulationRoleIsDevices;
/* is the dist role broker */
extern GT_BOOL sasicgSimulationRoleIsBroker;
/* is the dist role bus */
extern GT_BOOL sasicgSimulationRoleIsBus;

/* the type of running simulation */
extern SASICG_SIMULATION_ROLES_ENT sasicgSimulationRole;
/* indicate that simulation done. and system can start using it's API.
   -- this is mainly needed for terminal to be able to start while the simulation
   try to connect on distributed system.
*/
extern GT_U32  simulationInitReady;
/* number of devices in the system */
extern GT_U32  smainDevicesNumber;
/* number of applications in the CPU :
   for broker system - number of applications connected to the broker
*/
extern GT_U32  sinitNumOfApplications;

typedef struct{
    GT_U32  ownSectionId;
    GT_U32  numDevices;
    GT_U32  *devicesIdArray;/* dynamic allocated */
}SINIT_BOARD_SECTION_INFO_STC;

/* the info about own (this) section of board
   relevant to device(s) that connected to 'Interface BUS'
*/
extern SINIT_BOARD_SECTION_INFO_STC  sinitOwnBoardSection;

/* number of connections that the interface BUS have to the devices/board sections */
extern GT_U32  sinitInterfaceBusNumConnections;

/* the application ID in multi process environment */
extern GT_U32  sinitMultiProcessOwnApplicationId;

/* short names of the system , index is sasicgSimulationRole */
extern char* consoleDistStr[];

/* distributed INI file name */
extern char   sdistIniFile[SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS];

/* indication that globally all devices work with memory access via BAR0,BAR2 */
extern GT_BOOL sinit_global_usePexLogic;


#define FUNCTION_NOT_SUPPORTED(funcName)\
    skernelFatalError(" %s : function not supported \n",funcName)

/**
* @internal SASICG_init1 function
* @endinternal
*
* @brief   Init Simulation.
*/
void SASICG_init1(
    void
);

/**
* @internal SASICG_init2 function
* @endinternal
*
* @brief   Stub function for backward compatibility.
*/
void SASICG_init2(
    void
);

/**
* @internal SASICG_exit function
* @endinternal
*
* @brief   Shut down Simulation.
*/
void SASICG_exit(
    void
);

/**
* @internal simulationLibInit function
* @endinternal
*
* @brief   Init all parts of simulation Simulation.
*/
void simulationLibInit(
    void
);

/**
* @internal skernelFatalError function
* @endinternal
*
* @brief   Fatal error handler for SKernel
*
* @param[in] format                   -  for printing.
*/
void skernelFatalError
(
    IN char * format, ...
);

/**
* @internal sinitIniFileSet function
* @endinternal
*
* @brief   Sets temporary INI file name.
*
* @param[in] iniFileName              - pointer to file name
*/
void sinitIniFileSet
(
    IN char* iniFileName
);

/**
* @internal sinitIniFileRestoreMain function
* @endinternal
*
* @brief   restore the INI file that is the 'Main INI' file.
*/
void sinitIniFileRestoreMain
(
    void
);

/**
* @internal simulationPrintf function
* @endinternal
*
* @brief   function to replace printf ... in order to make is under SCIB lock
*
* @param[in] format                   -  for printing.
*                                       On success, printf returns the number of bytes output.
*                                       On error, printf returns EOF.
*/
int simulationPrintf
(
    IN char * format, ...
);

/* check if running like emulator */
GT_U32  simulationCheck_onEmulator(void);
/* check if running Aldrin emulator in FULL mode */
GT_U32  simulationCheck_onEmulator_isAldrinFull(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sinith */



