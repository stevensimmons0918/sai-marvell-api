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
* @file mvPortModeElements.h
*
* @brief Ln2CpuPortunit registers and tables classes declaration.
*
* @version   9
********************************************************************************
*/

#ifndef __mvPortModeElements_H
#define __mvPortModeElements_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElementsPrv.h>

/**
* @internal hwsPortModeParamsSetFec function
* @endinternal
*
* @brief   port mode params get to buffer.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                - port num
* @param[in] portMode                - Different port standard
*       metrics
*
* @param[out] portParamsBuffer      - Port Init parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortModeParamsGetToBuffer
(
   IN GT_U8                    devNum,
   IN GT_U32                   portGroup,
   IN GT_U32                   portNum,
   IN MV_HWS_PORT_STANDARD     portMode,
   OUT MV_HWS_PORT_INIT_PARAMS  *portParamsBuffer
);

/**
* @internal hwsIsPortModeSupported function
* @endinternal
*
* @brief   is port mode supported
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                - port num
* @param[in] portMode                - Different port standard
*       metrics
*
* @retval GT_TRUE                    - if port mode supported
* @retval GT_FALSE                   - not supported mode
*/
GT_BOOL hwsIsPortModeSupported
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portNum,
    IN MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal hwsIsFecModeSupported function
* @endinternal
*
* @brief   is FEC mode supported
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                - port num
* @param[in] portMode                - Different port standard
*       metrics
* @param[in] fecMode                - FEC status
*
* @retval GT_TRUE                    - if FEC mode supported
* @retval GT_FALSE                   - not supported mode
*/
GT_BOOL hwsIsFecModeSupported
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_FEC_MODE    fecMode
);

/**
* @internal hwsPortModeParamsPrintCurrent function
* @endinternal
*
* @brief   print the elements device.
*
* @param[in] devNum          - system device number- use
*                              PRINT_ALL_DEVS (0xff) to print
*                              all devices
* @param[in] portNum         - port num
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPortModeParamsPrintCurrent
(
    IN GT_U8                   devNum,
    IN GT_U32                  portNum
);

#ifdef __cplusplus
}
#endif

#endif /* __mvPortModeElements_H */


