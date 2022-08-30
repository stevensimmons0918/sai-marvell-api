/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\cpu\private\prvpdlcpu.h.
 *
 * @brief   Declares the prvpdlcpu class
 */

#ifndef __prvPdlCpuh

#define __prvPdlCpuh
/**
********************************************************************************
 * @file prvPdlCpu.h   
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
 * @brief Platform driver layer - CPU private declarations and APIs
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/cpu/pdlCpu.h>
#include <pdl/xml/xmlParser.h>

/**
* @addtogroup CPU
* @{
*/

 /** 
 * @defgroup CPU_private
 * @{CPU private definitions and declarations including:
*/

extern PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC prvPdlCpuFamilyStrToEnum;

/**
 * @fn  PDL_STATUS prvPdlCpuInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlRootId );
 *
 * @brief   Init cpu module Create cpu DB and initialize
 *
 * @param [in]  xmlRootId   Xml id.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlCpuInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlRootId
);
/* @}*/
/* @}*/

#endif
