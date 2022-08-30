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
* mvGw16If.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __mvGw16If_H
#define __mvGw16If_H

/* General H Files */
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/generic/hwDriver/cpssHwDriverAPI.h>

#ifdef __cplusplus
extern "C" {
#endif


/************************* Globals *******************************************************/
#define HWS_D2D_PHY_BASE_ADDR               0x33000000
#define HWS_D2D_OFFSET                      0x01000000
#define HWS_D2D_PCS_BASE_ADDR               0x33000000

#define PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, regOffset) (regOffset + HWS_D2D_PCS_BASE_ADDR + d2dIndex * HWS_D2D_OFFSET)

#define PRV_HWS_PHY_REG_ADDR_RELATIVE_CALC_MAC(d2dIndex, regOffset) (regOffset + d2dIndex * HWS_D2D_OFFSET)
#define PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, regOffset) (HWS_D2D_PHY_BASE_ADDR + PRV_HWS_PHY_REG_ADDR_RELATIVE_CALC_MAC(d2dIndex, regOffset))

#define HWS_D2D_PMA_BASE_ADDR               0x33000000
#define PRV_HWS_PMA_REG_ADDR_RELATIVE_CALC_MAC(d2dIndex, regOffset) (regOffset + d2dIndex * HWS_D2D_OFFSET)
#define PRV_HWS_PMA_REG_ADDR_CALC_MAC(d2dIndex, regOffset) (HWS_D2D_PMA_BASE_ADDR + PRV_HWS_PMA_REG_ADDR_RELATIVE_CALC_MAC(d2dIndex, regOffset))



/**
* @internal mvHwsD2dPmaLaneTestGenStatus function
* @endinternal
*
* @brief   Read the tested pattern receive error counters and status.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] d2dNum                   - D2D number (0..63)
* @param[in] d2dLaneNum               - D2D lane number (0..3)
* @param[in] txPattern                - pattern to transmit
* @param[in] counterAccumulateMode    - counter Accumulate Mode.
*
* @param[out] status                  - Defines serdes test
*       generator results
*/
GT_STATUS mvHwsD2dPmaLaneTestGenStatus
(
    IN GT_U8                     devNum,
    IN GT_U32                    d2dNum,
    IN GT_U32                    d2dLaneNum,
    IN MV_HWS_SERDES_TX_PATTERN  txPattern,
    IN GT_BOOL                   counterAccumulateMode,
    OUT MV_HWS_SERDES_TEST_GEN_STATUS *status
);

/**
* @internal mvHwsGw16TileTestGen function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - GW16 channel index (APPLICABLE RANGES: 0..3)
* @param[in] txPattern                - pattern to transmit
* @param[in] mode                     - test mode or normal
*/
GT_STATUS mvHwsGw16TileTestGen
(
    IN GT_U8                       devNum,
    IN GT_UOPT                     portGroup,  /* D2D index */
    IN GT_UOPT                     serdesNum,  /* Channel index */
    IN MV_HWS_SERDES_TX_PATTERN    txPattern,
    IN MV_HWS_SERDES_TEST_GEN_MODE mode
);

/**
* @internal mvHwsGw16TileTestGenGet function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - GW16 channel index (APPLICABLE RANGES: 0..3)
* @param[out] txPattern               - (pointer to) pattern to transmit
* @param[out] mode                    - (pointer to) test mode or normal
*/
GT_STATUS mvHwsGw16TileTestGenGet
(
    IN GT_U8                       devNum,
    IN GT_UOPT                     portGroup,
    IN GT_UOPT                     serdesNum,
    OUT MV_HWS_SERDES_TX_PATTERN    *txPatternPtr,
    OUT MV_HWS_SERDES_TEST_GEN_MODE *modePtr
);

/**
* @internal mvHwsGw16TileTestGenStatus function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - GW16 channel index (APPLICABLE RANGES: 0..3)
* @param[in] txPattern                - pattern to transmit
* @param[in] counterAccumulateMode    - counter Accumulate Mode.
*
* @param[out] status                  - Defines serdes test
*       generator results
*/
GT_STATUS mvHwsGw16TileTestGenStatus
(
    IN GT_U8                     devNum,
    IN GT_U32                    portGroup,
    IN GT_U32                    serdesNum,
    IN MV_HWS_SERDES_TX_PATTERN  txPattern,
    IN GT_BOOL                   counterAccumulateMode,
    OUT MV_HWS_SERDES_TEST_GEN_STATUS *status
);

/**
* @internal mvHwsD2dPmaLaneTestGeneneratorConfig function
* @endinternal
*
* @brief   Activates the D2D PMA Lane test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] d2dNum                   - D2D number (0..63)
* @param[in] d2dLaneNum               - D2D lane number (0..3)
* @param[in] configPtr                - (pointer to) PMA test generator configuration structure
*/
GT_STATUS mvHwsD2dPmaLaneTestGeneneratorConfig
(
    IN GT_U8                           devNum,
    IN GT_U32                          d2dNum,
    IN GT_U32                          d2dLaneNum,  /* Lane index 0..3*/
    IN MV_HWS_D2D_PRBS_CONFIG_STC      *configPtr
);


/**
* @internal mvHwsD2dPmaLaneTestCheckerConfig function
* @endinternal
*
* @brief   Activates the D2D PMA Lane test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] d2dNum                   - D2D number (0..63)
* @param[in] d2dLaneNum               - D2D lane number (0..3)
* @param[in] configPtr                - (pointer to) PMA test checker configuration structure
*/
GT_STATUS mvHwsD2dPmaLaneTestCheckerConfig
(
    IN GT_U8                           devNum,
    IN GT_U32                          d2dNum,
    IN GT_U32                          d2dLaneNum,  /* Lane index 0..3*/
    IN MV_HWS_D2D_PRBS_CONFIG_STC      *configPtr
);

/**
* @internal mvHwsD2dPmaLaneTestLoopbackSet function
* @endinternal
*
* @brief   Activates loopback between lane checker and lane generator.
*          To enable loopback XBAR configuration should be done + FIFO reset/unreset
*
* @param[in] devNum                   - The Pp's device number
* @param[in] d2dNum                   - D2D number (0..63)
* @param[in] lbType                   - loopback type
*/
GT_STATUS mvHwsD2dPmaLaneTestLoopbackSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  d2dNum,
    IN MV_HWS_PORT_LB_TYPE     lbType
);

/**
* @internal mvHwsD2dPmaLaneTestGenInjectErrorEnable function
* @endinternal
*
* @brief   Activates the D2D PMA error injection.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] d2dNum                   - D2D number (0..63)
* @param[in] d2dLaneNum               - physical serdes number
* @param[in] serdesDirection          - SERDES direction
*/
GT_STATUS mvHwsD2dPmaLaneTestGenInjectErrorEnable
(
    IN GT_U8                       devNum,
    IN GT_U32                      d2dNum,
    IN GT_UOPT                     d2dLaneNum,  /* Lane index 0..3*/
    IN MV_HWS_SERDES_DIRECTION     serdesDirection
);

/**
* @internal mvHwsD2dPhyIfInit function
* @endinternal
*
* @brief   Init GW16 Serdes IF functions.
*
* @param[in] devNum                   - The Pp's device number
*/
GT_STATUS mvHwsD2dPhyIfInit
(
    GT_U8  devNum,
    MV_HWS_SERDES_FUNC_PTRS **funcPtrArray
);

/**
* @internal mvHwsGW16SerdesAutoStartInit function
* @endinternal
*
* @param[in] devNum                   - The Pp's device number
*/
GT_STATUS mvHwsGW16SerdesAutoStartInit
(
    CPSS_HW_DRIVER_STC *driverPtr,
    GT_U8 devNum,
    GT_U32 d2dNum
);

/**
* @internal mvHwsGW16_Z2_SerdesAutoStartInit function
* @endinternal
*
* @param[in] devNum                   - The Pp's device number
* @param[in] ravenBmp                 - bitmap of requested Ravens
*/
GT_STATUS mvHwsGW16_Z2_SerdesAutoStartInit
(
    GT_U8 devNum,
    GT_U32 ravenBmp
);

GT_STATUS mvHwsGW16AutoStartStatusGet
(
    CPSS_HW_DRIVER_STC *driverPtr,
    GT_U8 devNum,
    GT_U32 d2dNum
);

/**
* @internal mvHwsGW16SerdesAutoStartInitStatusGet function
* @endinternal
*
* @param[in] devNum                   - The Pp's device number
*/
GT_STATUS mvHwsGW16SerdesAutoStartInitStatusGet
(
    GT_U8 devNum,
    GT_U32 d2dNum
);


/**
* @internal mvHwsGw16IfClose function
* @endinternal
*
* @brief   Release all system resources allocated by Serdes IF functions.
*
* @param[in] devNum                   - The Pp's device number
*/
void mvHwsGw16IfClose
(
    IN GT_U8      devNum
);

#ifdef __cplusplus
}
#endif

#endif /* __mvAvagoIf_H */

