/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file mvPortModeElementsPrv.h
*
* @brief Ln2CpuPortunit registers and tables classes declaration.
*
* @version   9
********************************************************************************
*/

#ifndef __mvPortModeElementsPrv_H
#define __mvPortModeElementsPrv_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

#define PRINT_ALL_DEVS  255

typedef struct
{
#ifndef MV_HWS_REDUCED_BUILD
    const MV_HWS_PORT_INIT_PARAMS   *supModesCatalog;
    MV_HWS_PORT_INIT_PARAMS         *legacySupModesCatalog;
    MV_HWS_PORT_FEC_MODE            *perModeFecList;
#endif
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
}MV_HWS_SINGLE_PORT_STC;

typedef MV_HWS_SINGLE_PORT_STC* MV_HWS_DEVICE_PORTS_ELEMENTS;

#ifdef MV_HWS_REDUCED_BUILD
extern MV_HWS_DEVICE_PORTS_ELEMENTS hwsDevicesPortsElementsArray[HWS_MAX_DEVICE_NUM];
#endif
extern const MV_HWS_PORT_INIT_PARAMS hwsSingleEntryInitParams;

/**
* @internal hwsExtendedPortNumGet function
* @endinternal
*
* @brief   it port in extended mode - returns the extended port
*          number, otherwise return the same port num.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[out] isExtended              - true if is extended
* @param[in/out] portNum              - port num
*
* @retval 0                        - on success
* @retval 1                        - on error
**/

GT_STATUS hwsExtendedPortNumGet
(
   IN    GT_U8    devNum,
   IN    GT_U32   portGroup,
   OUT   GT_BOOL  *isExtended,
   INOUT GT_U32   *portNum
);

/**
* @internal hwsPortsParamsArrayGet function
* @endinternal
*
* @brief   port params array get.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                - port num
*
* @retval MV_HWS_PORT_INIT_PARAMS* - pointer to the
*                                    *ORIGINAL* entry in the
*                                    elements database
* @retval NULL -                     on error/not supported mode
*/
MV_HWS_PORT_INIT_PARAMS *hwsPortsParamsArrayGet
(
    IN GT_U8            devNum,
    IN GT_U32           portGroup,
    IN GT_U32           portNum
);

/**
* @internal hwsPortModeParamsGet function
* @endinternal
*
* @brief   USING THIS FUNCTION MAY CORRUPT THE ELEMENTS
*          DATABASES. use hwsPortModeParamsGetToBuffer instead
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                - port num
* @param[in] portMode                - Different port standard
*       metrics
*
* @retval MV_HWS_PORT_INIT_PARAMS* - pointer to the
*                                    *ORIGINAL* entry in the
*                                    elements database
* @retval NULL -                     on error/not supported mode
*/
MV_HWS_PORT_INIT_PARAMS *hwsPortModeParamsGet
(
   IN GT_U8                    devNum,
   IN GT_U32                   portGroup,
   IN GT_U32                   portNum,
   IN MV_HWS_PORT_STANDARD     portMode,
   OUT GT_U32                  *portModeIndexPtr
);

/**
* @internal hwsPortModeParamsSet function
* @endinternal
*
* @brief   port mode params set .
*
* @param[in] devNum                  - system device number
* @param[in] portGroup               - port group (core) number
* @param[in] portNum                 - port num
* @param[in] portParams              - Port Init parameters
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortModeParamsSet
(
   IN GT_U8                    devNum,
   IN GT_U32                   portGroup,
   IN GT_U32                   portNum,
   IN MV_HWS_PORT_INIT_PARAMS  *portParams
);

/**
* @internal hwsPortsParamsCfg function
* @endinternal
*
* @brief   port  params config .
*
* @param[in] devNum                  - system device number
* @param[in] portGroup               - port group (core) number
* @param[in] portNum                 - port num
* @param[in] portMode                - port standard metric
* @param[in] macNum                  - MAC number
* @param[in] pcsNum                  - physical PCS number
* @param[in] sdVecSize               - SD vector size
* @param[in] sdVector                - SD vector
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortsParamsCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN GT_U32                  macNum,
    IN GT_U32                  pcsNum,
    IN GT_U8                   sdVecSize,
    IN GT_U8                   *sdVector
);

/**
* @internal hwsPortsElementsClose function
* @endinternal
*
* @brief   port  elements close .
*
* @param[in] devNum                  - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortsElementsClose
(
    IN GT_U8                   devNum
);

/**
* @internal hwsInitPortsModesParam function
* @endinternal
*
* @brief   Registers silicon ports modes parameters in the system.
*         Calls ones per active silicon type.
* @param[in] devType                  - silicon type.
* @param[in] portModes                - pointer to array of ports modes parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsInitPortsModesParam
(
    IN GT_U8                   devNum,
    IN MV_HWS_PORT_INIT_PARAMS *portModes[]
);

/**
* @internal hwsPortModeParamsSetMode function
* @endinternal
*
* @brief   port mode params set.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                - port num
* @param[in] portMode                - Different port standard
*       metrics
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortModeParamsSetMode
(
   GT_U8                    devNum,
   GT_U32                   portGroup,
   GT_U32                   portNum,
   MV_HWS_PORT_STANDARD     portMode
);

/**
* @internal hwsPortModeParamsSetFec function
* @endinternal
*
* @brief   port mode params set function.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                - port num
* @param[in] portMode                - Different port standard
*       metrics
* @param[in] fecMode                - FEC status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortModeParamsSetFec
(
   IN GT_U8                    devNum,
   IN GT_U32                   portGroup,
   IN GT_U32                   portNum,
   IN MV_HWS_PORT_STANDARD     portMode,
   IN MV_HWS_PORT_FEC_MODE     fecMode
);
GT_STATUS hwsFalconPortParamsGetLanes
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      portNum,
    OUT MV_HWS_PORT_INIT_PARAMS*    portParams
);
/**
* @internal hwsPortParamsGetLanes function
* @endinternal
*
* @brief   port  params get active lanes .
*
* @param[in] devNum                  - system device number
* @param[in] portGroup               - port group (core) number
* @param[in] portNum                 - port num
* @param[in,out] portParams          -  (pointer to) Port Init
*       parameters
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortParamsGetLanes
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      portNum,
    INOUT MV_HWS_PORT_INIT_PARAMS*    portParams
);

/**
* @internal hwsIsQuadCgMacSupported function
* @endinternal
*
* @brief   is port mode supported
*
* @param[in] devNum          - system device number
* @param[in] portNum         - port num
*
* @retval GT_TRUE            - if is quad CG mac mode supported
* @retval GT_FALSE           - not supported
*/
GT_BOOL hwsIsQuadCgMacSupported
(
    IN GT_U8                   devNum,
    IN GT_U32                  portNum
);

/*============================== NOT DEFINED: MV_HWS_REDUCED_BUILD ==============================*/
#ifndef MV_HWS_REDUCED_BUILD

/**
* @internal hwsDevicePortsElementsCfg function
* @endinternal
*
* @brief   Init the ports' struct and initialize
*          portModesElements.
*
* @param[in] devNum          - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsDevicePortsElementsCfg
(
    IN GT_U8       devNum
);

/**
* @internal hwsPortsElementsFindMode function
* @endinternal
*
* @brief   find the port mode line in the catlog supplied in
*          hwsPortSupModes.
*
* @param[in] hwsPortSupModes          - catalog of the supported
*       modes
* @param[in] portMode                - port mode to find
* @param[out] portModeIndex           - the line number of the
*       requested port mode in the portModeIndex catalog.
*
* @retval MV_HWS_PORT_INIT_PARAMS* - pointer to the
*                                    *ORIGINAL* entry in the
*                                    portModeIndex catlog
* @retval NULL -                     on error/not supported mode
*
* @note If portMode=NON_SUP_MODE, this value is the number of
*       supported modes.not mandatory can be NULL.
*/
const MV_HWS_PORT_INIT_PARAMS *hwsPortsElementsFindMode
(
    IN const MV_HWS_PORT_INIT_PARAMS    hwsPortSupModes[],
    IN MV_HWS_PORT_STANDARD             portMode,
    OUT GT_U32                          *portModeIndex
);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __mvPortModeElementsPrv_H */


