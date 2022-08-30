/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\init\pdlinit.h.
 *
 * @brief   Declares the pdlinit class
 */

#ifndef __pdlInith
#define __pdlInith
/**
********************************************************************************
 * @file pdlInit.h   
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
 * @brief Platform driver layer - Init library
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/init/pdlInitLedStream.h>

/**
* @addtogroup Library
* @{
*/

typedef enum {
    PDL_OS_LOCK_TYPE_NONE_E,
    PDL_OS_LOCK_TYPE_I2C_E,
    PDL_OS_LOCK_TYPE_SMI_E,
    PDL_OS_LOCK_TYPE_XSMI_E,
    PDL_OS_LOCK_TYPE_GPIO_E,
    PDL_OS_LOCK_TYPE_LOGGER_E, /* Requires recursive locking mechanism */
    PDL_OS_LOCK_TYPE_LAST_E
} PDL_OS_LOCK_TYPE_ENT;

typedef PDL_STATUS pdlExtDrvInit_FUN (
    IN   void
);

typedef struct {
    pdlExtDrvInit_FUN                     * initFun;
    PDL_OS_LOCK_TYPE_ENT                    lockType;
} PDL_CALLBACK_INIT_INFO_STC;

/**
 * @typedef void ( PDL_OS_CALLBACK_PRINT_PTR) (const char *format, ...)
 *
 * @brief   @defgroup Init Init
 *          @{Init functions
 */

typedef int    ( PDL_OS_CALLBACK_PRINT_PTR)        (const char *format, ...);

/**
 * @typedef void * ( PDL_OS_CALLBACK_MEMALLOC_PTR) (SIZE_T size)
 *
 * @brief   Defines an alias representing the size
 */

typedef void *  ( PDL_OS_CALLBACK_MEMALLOC_PTR)     (SIZE_T size);

/**
 * @typedef void ( PDL_OS_CALLBACK_MEMFREE_PTR) (void * ptr)
 *
 * @brief   Defines an alias representing the pointer
 */

typedef void    ( PDL_OS_CALLBACK_MEMFREE_PTR)      (void * ptr);

/**
 * @typedef void ( PDL_OS_CALLBACK_DEBUG_PTR) (const char *func_name_PTR, const char *format, ...)
 *
 * @brief   Defines an alias representing the ...
 */

typedef void    ( PDL_OS_CALLBACK_DEBUG_PTR)        (const char *func_name_PTR, const char *format, ...);

/**
 * @struct  PDL_OS_CALLBACK_API_STCT
 *
 * @brief   A pdl operating system callback API stct.
 */

typedef void PDL_OS_LOCK_CALLBACK_PTR (void);
typedef void PDL_OS_UNLOCK_CALLBACK_PTR (void);

typedef struct {
    PDL_OS_LOCK_CALLBACK_PTR            * lockFun;
    PDL_OS_UNLOCK_CALLBACK_PTR          * unlockFun;
} PDL_OS_LOCK_FUNC_STC;

typedef enum {
    PDL_OS_INIT_TYPE_FULL_E,
    PDL_OS_INIT_TYPE_STACK_E,
    PDL_OS_INIT_TYPE_LAST_E
} PDL_OS_INIT_TYPE_ENT;

/* CPSS MUST provide these functions to PDL in order to allow access to GPIO on PP
   lion/hopper doesn't have GPIOs on PP, so portgroup isn't required as a parameter for these callbacks*/
typedef unsigned int    GT_STATUS;
typedef GT_STATUS pdlCpssRegRead_FUN (
    IN  GT_U8       devIdx,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    OUT GT_U32    * dataPtr
);

typedef GT_STATUS pdlCpssRegWrite_FUN (
    IN  GT_U8       devIdx,
    IN  GT_U32      regAddr,
    IN  GT_U32      mask,
    IN  GT_U32      data
);

typedef GT_STATUS pdlSmiRegRead_FUN (
    IN  GT_U8       devIdx,
    IN  GT_U32      portGroupsBmp,
    IN  GT_U32      smiInterface,
    IN  GT_U32      smiAddr,
	IN  GT_U8       phyPageSelReg,
	IN  GT_U8       phyPage,
    IN  GT_U32      regAddr,
    OUT GT_U16     *dataPtr
);

typedef GT_STATUS pdlSmiPpuLock_FUN (
    IN  GT_U8       devIdx,
    IN  GT_U32      port,
	IN  BOOLEAN     lock,
    OUT BOOLEAN    *prevLockStatePtr
);

typedef GT_STATUS pdlSmiRegWrite_FUN (
    IN  GT_U8       devIdx,
    IN  GT_U32      portGroupsBmp,
    IN  GT_U32      smiInterface,
    IN  GT_U32      smiAddr,
	IN  GT_U8       phyPageSelReg,
	IN  GT_U8       phyPage,
    IN  GT_U32      regAddr,
    IN  GT_U16      data
);

typedef GT_STATUS pdlXsmiRegRead_FUN (
    IN  GT_U8      devIdx,
    IN  GT_U32     xsmiInterface,
    IN  GT_U32     xsmiAddr,
    IN  GT_U32     regAddr,
    IN  GT_U32     phyDev, 
    OUT GT_U16     *dataPtr
);

typedef GT_STATUS pdlXsmiRegWrite_FUN (
    IN  GT_U8      devIdx,
    IN  GT_U32     xsmiInterface,
    IN  GT_U32     xsmiAddr,
    IN  GT_U32     regAddr,
    IN  GT_U32     phyDev, 
    IN  GT_U16     data 
);

typedef void pdlI2cResultHandler_FUN (
    IN  PDL_STATUS i2c_ret_status,
    IN  UINT_8     slave_address,
    IN  UINT_8     bus_id,
    IN  UINT_8     offset,
    IN  BOOLEAN    i2c_write
);

typedef BOOLEAN pdlXmlArchiveUncompressHandler_FUN (
    IN  char       *archiveFileNamePtr,
    OUT char       *xmlFileNamePtr,
    OUT char       *signatureFileNamePtr
);

typedef BOOLEAN pdlXmlVerificationHandler_FUN (
    IN  char       *xmlFileNamePtr,
    IN  char       *signatureFileNamePtr
);

typedef BOOLEAN pdlPpPortModeSupported_FUN (
    UINT_8                      devIdx,
    UINT_32                     mac_port,
    PDL_PORT_SPEED_ENT          speed,
    PDL_INTERFACE_MODE_ENT      interface_mode
);

/* Ledstream related callback APIs are MANDATORY in case XML LEDs\Ledstream-is-supported is turned on
   APIs should be based on the following CPSS ledstream control APIs:
   cpssDxChLedStreamPortPositionSet
   cpssDxChLedStreamPortClassPolarityInvertEnableSet
   cpssDxChLedStreamConfigSet
   cpssDxChLedStreamClassManipulationGet
   cpssDxChLedStreamClassManipulationSet
*/

typedef GT_STATUS pdlLedStreamPortPositionSet_FUN (
    IN  GT_U8                   devNum,
    IN  GT_U32                  portNum,
    IN  GT_U32                  position
);

typedef GT_STATUS pdlLedStreamPortClassPolarityInvertEnableSet_FUN (
   IN  GT_U8                           devNum,
   IN  GT_U32                          portNum,
   IN  GT_U32                          classNum,
   IN  BOOLEAN                         invertEnable
);

typedef GT_STATUS pdlLedStreamConfigSet_FUN (
    IN  GT_U8                               devNum,
    IN  GT_U32                              ledInterfaceNum,
    IN  PDL_LED_STEAM_INTERFACE_CONF_STC   *ledConfPtr
);

typedef GT_STATUS pdlLedStreamClassManipulationGet_FUN (
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  PDL_LED_PORT_TYPE_ENT           portType,
    IN  GT_U32                          classNum,
    OUT PDL_LED_CLASS_MANIPULATION_STC *classParamsPtr
);

typedef GT_STATUS pdlLedStreamClassManipulationSet_FUN (
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  PDL_LED_PORT_TYPE_ENT           portType,
    IN  GT_U32                          classNum,
    IN  PDL_LED_CLASS_MANIPULATION_STC *classParamsPtr
);

typedef struct PDL_LEDSTREAK_CALLBACK_INFO_STCT {
    pdlLedStreamPortPositionSet_FUN                         *ledStreamPortPositionSetClbk;
    pdlLedStreamPortClassPolarityInvertEnableSet_FUN        *ledStreamPortClassPolarityInvertEnableSetClbk;
    pdlLedStreamConfigSet_FUN                               *ledStreamConfigSetClbk;
    pdlLedStreamClassManipulationGet_FUN                    *ledsStreamClassManipulationGetClbk;
    pdlLedStreamClassManipulationSet_FUN                    *ledsStreamClassManipulationSetClbk;
} PDL_LEDSTREAK_CALLBACK_INFO_STC;

typedef struct PDL_OS_CALLBACK_API_STCT {
    /** @brief   The print string pointer */
    PDL_OS_CALLBACK_PRINT_PTR      *printStringPtr;
    /** @brief   The malloc pointer */
    PDL_OS_CALLBACK_MEMALLOC_PTR   *mallocPtr;
    /** @brief   The free pointer */
    PDL_OS_CALLBACK_MEMFREE_PTR    *freePtr;
    /** @brief   The debug log pointer */
    PDL_OS_CALLBACK_DEBUG_PTR      *debugLogPtr;
    /* this array will provide lock/unlock mechanisms that provide mutual exclusion for resources such as I2C bus, PP, etc. */
    PDL_OS_LOCK_FUNC_STC                                    lockFuncArr[PDL_OS_LOCK_TYPE_LAST_E];
    pdlCpssRegRead_FUN                                      *ppCmRegReadClbk;
    pdlCpssRegWrite_FUN                                     *ppCmRegWriteClbk;
    pdlSmiRegRead_FUN                                       *smiRegReadClbk;
    pdlSmiRegWrite_FUN                                      *smiRegWriteClbk;
    pdlSmiPpuLock_FUN                                       *smiPpuLockClbk;
    pdlXsmiRegRead_FUN                                      *xsmiRegReadClbk;
    pdlXsmiRegWrite_FUN                                     *xsmiRegWriteClbk;
    pdlCpssRegRead_FUN                                      *ppRegReadClbk;
    pdlCpssRegWrite_FUN                                     *ppRegWriteClbk;
    /* optional I2C handler - after I2C operations */
    pdlI2cResultHandler_FUN                                 *i2cResultHandlerClbk;
    /* optional xml archive uncompress utility */
    pdlXmlArchiveUncompressHandler_FUN                      *arXmlUncompressClbk;
    pdlXmlVerificationHandler_FUN                           *vrXmlVerifyClbk;
    pdlPpPortModeSupported_FUN                              *portModeSupportedClbk;
    PDL_LEDSTREAK_CALLBACK_INFO_STC                          ledStreamCallbackInfo;
} PDL_OS_CALLBACK_API_STC;

/**
 * @fn  PDL_STATUS pdlInitDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Init debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlInitDebugSet (
    IN  BOOLEAN             state
);

/**
 * @fn  PDL_STATUS pdlInit ( IN char * xmlFilePathPtr, IN char * xmlTagPrefixPtr, IN PDL_OS_CALLBACK_API_STC * callbacksPTR )
 *
 * @brief   initialize PDL
 *
 * @param [in]  xmlFilePathPtr  XML location.
 * @param [in]  xmlTagPrefixPtr XML tag's prefix.
 * @param [in]  callbacksPTR    application-specific implementation for os services.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlInit (
    IN char                       * xmlFilePathPtr,
    IN char                       * xmlTagPrefixPtr,
    IN PDL_OS_CALLBACK_API_STC    * callbacksPTR,
    IN PDL_OS_INIT_TYPE_ENT         initType
);

/**
 * @fn  void pdlInitDone ()
 *
 * @brief   Called by application to indicate PDL init has been completed for all XML files
 *
 */

PDL_STATUS pdlInitDone (
    void
);

/**
 * @fn  void pdlDestroy ()
 *
 * @brief   release all memory allocated by Pdl
 *
 */

PDL_STATUS pdlDestroy (
    void
);

/* @}*/
/* @}*/

#endif
