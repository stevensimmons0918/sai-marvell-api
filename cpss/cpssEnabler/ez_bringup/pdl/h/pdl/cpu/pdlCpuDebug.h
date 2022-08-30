/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\cpu\pdlcpudebug.h.
 *
 * @brief   Declares the pdlcpudebug class
 */

#ifndef __pdlCpuDebugh
#define __pdlCpuDebugh
/**
********************************************************************************
 * @file pdlCpuDebug.h   
 * @copyright
 *    (c), Copyright 2001, Marvell International Ltd.
 *    THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 *    NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 *    OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 *    DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 *    THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,
 *    IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
********************************************************************************
 * 
 * @brief Platform driver layer - CPU related debug API
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/cpu/pdlCpu.h>


/**
 * @fn  PDL_STATUS pdlCpuTypeSet ( IN PDL_CPU_TYPE_ENT cpuType )
 *
 * @brief   Set cpu type
 *
 * @param [in] cpuType  cpu type.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlCpuTypeSet (
    IN PDL_CPU_TYPE_ENT          cpuType
);


/**
 * @fn  PDL_STATUS pdlCpuDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Cpu debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlCpuDebugSet (
    IN  BOOLEAN             state
);



#endif