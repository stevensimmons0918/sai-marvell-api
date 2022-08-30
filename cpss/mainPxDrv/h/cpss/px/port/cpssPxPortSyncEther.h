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
* @file cpssPxPortSyncEther.h
*
* @brief CPSS definition for Sync-E in Pipe devices (Synchronous Ethernet)
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxPortSyncEther
#define __cpssPxPortSyncEther

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @enum CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT
 *
 * @brief Enumeration of physical recovery clock output pins.
*/
typedef enum{

    /** @brief clock output pin0 or
     *  clock output for port group 0.
     */
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK0_E,

    /** @brief clock output pin1 or
     *  clock output for port group 1.
     */
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK1_E

} CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT;

/**
* @enum CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT
 *
 * @brief Enumeration of recovery clock divider values.
*/
typedef enum{

    /** clock divided by 1. */
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E,

    /** clock divided by 2. */
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_E,

    /** clock divided by 3. */
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_3_E,

    /** clock divided by 4. */
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_4_E,

    /** clock divided by 5. */
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_5_E,

    /** clock divided by 8. */
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_8_E,

    /** clock divided by 16. */
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_16_E,

    /** clock divided by 2.5. */
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_5_E

} CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT;

/**
* @enum CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT
 *
 * @brief Enumeration of recovered clock select bus.
*/
typedef enum{

    /** clock0 bus */
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E,

    /** clock1 bus */
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E

} CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT;

/**
* @internal cpssPxPortSyncEtherRecoveryClkConfigSet function
* @endinternal
*
* @brief   Function configures the recovery clock enable/disable state and sets
*         its source portNum.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type.
* @param[in] enable                   - enable/disable recovered clock1/clock2:
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
* @param[in] portNum                  - port number.
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..3
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_STATE             - on bad state
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortSyncEtherRecoveryClkConfigSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    IN  GT_BOOL                 enable,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneNum
);

/**
* @internal cpssPxPortSyncEtherRecoveryClkConfigGet function
* @endinternal
*
* @brief   Function gets the recovery clock enable/disable state and its source
*         portNum.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type.
*
* @param[out] enablePtr                - (pointer to) port state as reference.
*                                      GT_TRUE -  enabled
*                                      GT_FALSE - disbled
* @param[out] portNumPtr               - (pointer to) port number.
* @param[out] laneNumPtr               - (pointer to) selected SERDES lane number within port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on bad state of register
*/
GT_STATUS cpssPxPortSyncEtherRecoveryClkConfigGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    OUT GT_BOOL  *              enablePtr,
    OUT GT_PHYSICAL_PORT_NUM    *portNumPtr,
    OUT GT_U32                  *laneNumPtr
);

/**
* @internal cpssPxPortSyncEtherRecoveryClkDividerValueSet function
* @endinternal
*
* @brief   Function sets recovery clock divider value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..3
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
* @param[in] value                    - recovery clock divider  to set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortSyncEtherRecoveryClkDividerValueSet
(
    IN  GT_SW_DEV_NUM                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT   clockSelect,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT    value
);

/**
* @internal cpssPxPortSyncEtherRecoveryClkDividerValueGet function
* @endinternal
*
* @brief   Function gets recovery clock divider value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..3
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
*
* @param[out] valuePtr                 - (pointer to) recovery clock divider value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssPxPortSyncEtherRecoveryClkDividerValueGet
(
    IN  GT_SW_DEV_NUM                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT   clockSelect,
    OUT CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT    *valuePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxPortSyncEther */


