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
* @file cpssDxChPortSyncEther.h
*
* @brief CPSS definition for Sync-E (Synchronous Ethernet)
*
* @version   15
********************************************************************************
*/

#ifndef __cpssDxChPortSyncEther
#define __cpssDxChPortSyncEther

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @enum CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_TYPE_ENT
 *
 * @brief Enumeration of PLL reference clock.
*/
typedef enum{

    /** Oscillator clock */
    CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_OSCILLATOR_E,

    /** Recovered clock */
    CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_RECOVERED_E

} CPSS_DXCH_PORT_SYNC_ETHER_PLL_REF_CLK_TYPE_ENT;

/**
* @enum CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT
 *
 * @brief Enumeration of physical recovery clock output pins.
*/
typedef enum{

    /** @brief clock output pin0 or
     *  clock output for port group 0.
     */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E,

    /** @brief clock output pin1 or
     *  clock output for port group 1.
     */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E,

    /** @brief clock output for port group 2.
     *  (APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
     */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK2_E,

    /** @brief clock output for port group 3.
     *  (APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
     */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK3_E,

    /** @brief clock output for port group 4.
     *  (APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
     */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK4_E,

    /** @brief clock output for port group 5.
     *  (APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
     */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK5_E,

    /** @brief clock output for port group 6.
     *  (APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
     */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK6_E,

    /** @brief clock output for port group 7.
     *  (APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
     */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK7_E

} CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT;

/**
* @enum CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT
 *
 * @brief Enumeration of recovery clock divider values.
*/
typedef enum{

    /** clock divided by 1. */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E,

    /** clock divided by 2. */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_E,

    /** clock divided by 3. */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_3_E,

    /** clock divided by 4. */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_4_E,

    /** clock divided by 5. */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_5_E,

    /** clock divided by 8. */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_8_E,

    /** clock divided by 16. */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_16_E,

    /** clock divided by 2.5. */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_5_E,

    /** @brief clock divided by 16.5.
     *  (APPLICABLE DEVICES: Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
     */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_16_5_E

} CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT;

/**
* @enum CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT
 *
 * @brief Enumeration of recovered clock select bus.
*/
typedef enum{

    /** clock0 bus */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E,

    /** clock1 bus */
    CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E

} CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT;

/**
* @internal cpssDxChPortSyncEtherRecoveryClkConfigSet function
* @endinternal
*
* @brief   Function configures the recovery clock enable/disable state and sets
*         its source portNum.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type.
* @param[in] enable                   - enable/disable recovered clock1/clock2
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
* @param[in] portNum                  - port number.
*                                      For xCat3 valid ports for configuration:
*                                      Giga Ports: 0,4,8,12,16,20
*                                      Stacking Ports: 24,25,26,27
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, recoveryClkType, portNum
*                                       or laneNum.
* @retval GT_BAD_STATE             - another SERDES already enabled
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortSyncEtherRecoveryClkConfigSet
(
    IN  GT_U8   devNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    IN  GT_BOOL enable,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32  laneNum
);

/**
* @internal cpssDxChPortSyncEtherRecoveryClkConfigGet function
* @endinternal
*
* @brief   Function gets the recovery clock enable/disable state and its source
*         portNum.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type
*
* @param[out] enablePtr                - (pointer to) port state as reference.
*                                      GT_TRUE -  enabled
*                                      GT_FALSE - disbled
* @param[out] portNumPtr               - (pointer to) port number.
* @param[out] laneNumPtr               - (pointer to) selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, recoveryClkType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on bad state of register
*/
GT_STATUS cpssDxChPortSyncEtherRecoveryClkConfigGet
(
    IN  GT_U8    devNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    OUT GT_BOOL  *enablePtr,
    OUT GT_PHYSICAL_PORT_NUM    *portNumPtr,
    OUT GT_U32   *laneNumPtr
);

/**
* @internal cpssDxChPortSyncEtherRecoveryClkDividerValueSet function
* @endinternal
*
* @brief   Function sets recovery clock divider value.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*                                      For xCat2 valid ports for configuration:
*                                      Giga Ports: 0,4,8,12,16,20
*                                      Stacking Ports: 24,25,26,27
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] value                    - recovery clock divider  to set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum, lane number or clockSelect
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port type
*/
GT_STATUS cpssDxChPortSyncEtherRecoveryClkDividerValueSet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  value
);

/**
* @internal cpssDxChPortSyncEtherRecoveryClkDividerValueGet function
* @endinternal
*
* @brief   Function gets recovery clock divider value.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*                                      For xCat2 valid ports for configuration:
*                                      Giga Ports: 0,4,8,12,16,20
*                                      Stacking Ports: 24,25,26,27
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] valuePtr                 - (pointer to) recovery clock divider value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum, lane number or clockSelect
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported port type
*/
GT_STATUS cpssDxChPortSyncEtherRecoveryClkDividerValueGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    OUT CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  *valuePtr
);

/**
* @internal cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet function
* @endinternal
*
* @brief   Function sets Recovered Clock Automatic Masking enabling.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*                                      For xCat2 valid ports for configuration:
*                                      Giga Ports: 0,4,8,12,16,20
*                                      Stacking Ports: 24,25,26,27
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
* @param[in] enable                   - enable/disable Recovered Clock Automatic Masking
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum, lane number or clockSelect
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port type
*/
GT_STATUS cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32  laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    IN  GT_BOOL enable
);

/**
* @internal cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet function
* @endinternal
*
* @brief   Function gets Recovered Clock Automatic Masking status.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*                                      For xCat2 valid ports for configuration:
*                                      Giga Ports: 0,4,8,12,16,20
*                                      Stacking Ports: 24,25,26,27
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) Recovered Clock Automatic Masking state
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum, lane number or clockSelect
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported port type
*/
GT_STATUS cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32  laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    OUT GT_BOOL *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortSyncEther */


