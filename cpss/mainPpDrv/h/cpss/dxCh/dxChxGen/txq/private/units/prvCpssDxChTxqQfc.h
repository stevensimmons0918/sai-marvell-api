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
* @file prvCpssDxChTxqQfc.h
*
* @brief CPSS SIP6 TXQ QFC low level configurations.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChTxqQfc
#define __prvCpssDxChTxqQfc

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPfc.h>

#define PRV_FALCON_QFC_HR_PORT_PER_REGISTER_NUM_CNS(_dev) (PRV_QFC_GROUP_SIZE_CNS)
#define PRV_FALCON_QFC_HR_EVENTS_PER_REGISTER_NUM_MAX_CNS    (CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC * CPSS_TC_RANGE_CNS)
#define PRV_FALCON_QFC_HR_EVENTS_PER_REGISTER_NUM_CNS(_dev)  PRV_FALCON_QFC_HR_PORT_PER_REGISTER_NUM_CNS(_dev)*CPSS_TC_RANGE_CNS



/**
* @struct PRV_CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_PFC_CFG_STC
 *
 * @brief Global PFC configuration
*/

typedef struct
{
    GT_BOOL hrCountingEnable;
     /** @brief  number of buffers in the packet buffer. used to determent the congestion level:
            Enabling IA TC PFC on/off per TC - 8 bit vector
     */
    GT_U32 ingressAgregatorTcPfcBitmap;

     /** @brief  number of buffers in the packet buffer. used to determent the congestion level:
            Threshold = Guaranteed Threshold + congestion_factor X (Available buffers - PB congestion).
     */
    GT_U32 pbAvailableBuffers ;

     /** @brief  when Enabled, if the PB occupancy is above the configured limit, PFC OFF message will be sent to all port.tc
                        0x0 = Disable; Disable; Disable PFC OFF messages for pb congestion
                        0x1 = Enable; Enable; Enable PFC OFF messages for pb congestion
     */
    GT_BOOL  pbCongestionPfcEnable;

     /** @brief  when Enabled, if the QFC receives PDX congestion indication, PFC OFF message will be sent to all port.tc
                        0x0 = Disable; Disable; Disable PFC OFF messages for pdx_congestion
                        0x1 = Enable; Enable; Enable PFC OFF messages for pdx_congestion
     */
    GT_BOOL  pdxCongestionPfcEnable;

     /** @brief  global PFC enable, when disabled, QFC will not issue PFC messages to D2D
                        0x0 = Disable; Disable; Disable PFC message generation
                        0x1 = Enable; Enable; Enable PFC message generation
     */
    GT_BOOL  globalPfcEnable;

} PRV_CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_PFC_CFG_STC;

/**
* @struct PRV_CPSS_DXCH_HR_EVENT_INFO_STC
 *
 * @brief  Headroom threshold crossed  event information
 *
 */
typedef struct
{
    /** @brief    Physical port that triggered  the event
     */
    GT_U32                localPortNum;
     /** @brief  Queue offset of the queue that triggered  the event
     */
    GT_U32                 trafficClass;
} PRV_CPSS_DXCH_HR_EVENT_INFO_STC;



/**
* @internal prvCpssFalconTxqQfcPortBufNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated per specified local port.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               - device number
* @param[in] tileNum              - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum               - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] index                - if perPort then local port number (APPLICABLE RANGES:0..8) else local queue number (APPLICABLE RANGES:0..399).
* @param[in] perPort              - if equal GT_TRUE then it is per port querry,else per queue
*
* @param[out] numPtr              - number of buffers
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcBufNumberGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileNum,
    IN  GT_U32  qfcNum,
    IN  GT_U32  index,
    IN  GT_BOOL perPort,
    OUT  GT_U32 *numPtr
);

/**
* @internal prvCpssFalconTxqQfcLocalPortToSourcePortSet function
* @endinternal
*
* @brief   Map local DP port to global dma
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                -  device number
* @param[in] tileNum               -  tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                -  QFC unit id (APPLICABLE RANGES:0..7).
* @param[in] localPort             -  local DP port (APPLICABLE RANGES:0..8).
* @param[in] sourcePort            -  Source port(0..2^9-1).
* @param[in] fcMode                -  flow control mode
* @param[in] ignoreMapping         -  Ignore mapping configuration ,configure only PFC generation
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcLocalPortToSourcePortSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      tileNum,
    IN  GT_U32                      qfcNum,
    IN  GT_U32                      localPort,
    IN  GT_U32                      sourcePort,
    IN CPSS_DXCH_PORT_FC_MODE_ENT   fcMode,
    IN GT_BOOL                      ignoreMapping
);

/**
* @internal prvCpssFalconTxqQfcLocalPortToSourcePortGet function
* @endinternal
*
* @brief   Get local DP port to global dma mapping
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                -  device number
* @param[in] tileNum               -  tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                -  QFC unit id (APPLICABLE RANGES:0..7).
* @param[in] localPort             -  local DP port (APPLICABLE RANGES:0..8).
* @param[out] sourcePortPtr        -  (Pointer to)Source port number(physical port).
* @param[out] fcModePtr            - (Pointer to) Enable PFC generation for the local port.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcLocalPortToSourcePortGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      tileNum,
    IN  GT_U32                      qfcNum,
    IN  GT_U32                      localPort,
    OUT  GT_U32                     *sourcePortPtr,
    OUT CPSS_DXCH_PORT_FC_MODE_ENT  *fcModePtr
);

/**
* @internal prvCpssFalconTxqQfcGlobalPfcCfgSet function
* @endinternal
*
* @brief   Set global PFC configuration per QFC (pbAvailableBuffers ,pbCongestionPfcEnable,pdxCongestionPfcEnable,globalPfcEnable)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] cfgPtr                - (pointer to)QFC PFC global configuration

* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalPfcCfgSet
(
     IN  GT_U8                                          devNum,
     IN  GT_U32                                         tileNum,
     IN  GT_U32                                         qfcNum,
     IN  PRV_CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_PFC_CFG_STC  * cfgPtr
);

/**
* @internal prvCpssFalconTxqQfcGlobalPfcCfgGet function
* @endinternal
*
* @brief   Get global PFC configuration per QFC (pbAvailableBuffers ,pbCongestionPfcEnable,pdxCongestionPfcEnable,globalPfcEnable)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[out] cfgPtr               - (pointer to)QFC PFC global configuration

* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalPfcCfgGet
(
     IN  GT_U8                                          devNum,
     IN  GT_U32                                         tileNum,
     IN  GT_U32                                         qfcNum,
     IN  PRV_CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_PFC_CFG_STC  *cfgPtr
);

/**
* @internal prvCpssFalconTxqQfcGlobalPbLimitSet function
* @endinternal
*
* @brief   Set global packet buffer limit
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] globalPbLimit         -  packet buffer limit

* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalPbLimitSet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN  GT_U32  globalPbLimit
);

/**
* @internal prvCpssFalconTxqQfcGlobalPbLimitGet function
* @endinternal
*
* @brief   Set global packet buffer limit
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] globalPbLimitPtr      - (pointer to)packet buffer limit

* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalPbLimitGet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     OUT  GT_U32 * globalPbLimitPtr
);

/**
* @internal prvCpssFalconTxqQfcGlobalTcPfcThresholdSet function
* @endinternal
*
* @brief   Set global TC threshold .
*    Note - Duplication to all units is done internally in prvCpssFalconDuplicatedMultiPortGroupsGet_byDevNum
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - device number
* @param[in] tc               - traffic class(APPLICABLE RANGES:0..7).
* @param[in] enable           - Global TC PFC enable option.
* @param[in] thresholdCfgPtr  - (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalTcPfcThresholdSet
(
  IN  GT_U8                             devNum,
  IN  GT_U32                            tc,
  IN  GT_BOOL                           enable,
  IN  CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
  IN  CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
);

/**
* @internal prvCpssFalconTxqQfcGlobalTcPfcThresholdGet function
* @endinternal
*
* @brief   Get global TC threshold .Note - The threshold is read from QFC0(all other DPs should contain the same value)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - device number
* @param[in] tc               - traffic class(APPLICABLE RANGES:0..7).
* @param[in] tileNum          - current tile id(APPLICABLE RANGES:0..3).
* @param[in] dp               - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[out] enablePtr       - (pointer to )Global TC PFC enable option.
* @param[out] thresholdCfgPtr - (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalTcPfcThresholdGet
(
     IN  GT_U8                             devNum,
     IN  GT_U32                            tc,
     IN  GT_U32                            tile,
     IN  GT_U32                            dp,
     OUT GT_BOOL                           *enablePtr,
     OUT CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
     OUT CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
);

/**
* @internal prvCpssFalconTxqQfcPortTcPfcThresholdSet function
* @endinternal
*
* @brief   Set  Port/TC threshold .
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number
* @param[in] tileNum         - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum          - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] localPort       - local port (APPLICABLE RANGES:0..8).
* @param[in] tc              - traffic class(APPLICABLE RANGES:0..7).
* @param[in] enable          - Port/TC PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct                -
*
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcPortTcPfcThresholdSet
(
     IN  GT_U8                       devNum,
     IN  GT_U32                      tileNum,
     IN  GT_U32                      qfcNum,
     IN  GT_U32                      localPort,
     IN  GT_U32                      tc,
     IN  GT_BOOL                     enable,
     IN CPSS_DXCH_PFC_THRESHOLD_STC  *thresholdCfgPtr
);

/**
* @internal prvCpssFalconTxqQfcPortTcPfcThresholdGet function
* @endinternal
*
* @brief   Get  Port/TC threshold .
*
* @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - device number
* @param[in] tileNum          - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum           - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] localPort        - local port (APPLICABLE RANGES:0..8).
* @param[in] tc               - traffic class(APPLICABLE RANGES:0..7).
* @param[out] enablePtr       - (pointer to )Port TC PFC enable option.
* @param[out] thresholdCfgPtr - (pointer to) threshold struct
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcPortTcPfcThresholdGet
(
     IN  GT_U8                          devNum,
     IN  GT_U32                         tileNum,
     IN  GT_U32                         qfcNum,
     IN  GT_U32                         localPort,
     IN  GT_U32                         tc,
     IN  GT_BOOL                        *enablePtr,
     OUT CPSS_DXCH_PFC_THRESHOLD_STC    *thresholdCfgPtr
);

/**
* @internal prvCpssFalconTxqQfcPortPfcThresholdSet function
* @endinternal
*
* @brief   Set  Port threshold .
*
* @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number
* @param[in] tileNum         - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum          - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] localPort       - local port (APPLICABLE RANGES:0..8).
* @param[in] thresholdCfgPtr - (pointer to) threshold struct
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcPortPfcThresholdSet
(
     IN  GT_U8                              devNum,
     IN  GT_U32                             tileNum,
     IN  GT_U32                             qfcNum,
     IN  GT_U32                             localPort,
     IN  GT_BOOL                            enable,
     IN  CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
     IN  CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
);

/**
* @internal prvCpssFalconTxqQfcPortPfcThresholdGet function
* @endinternal
*
* @brief   Get  Port threshold .
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - device number
* @param[in] tileNum          - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum           - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] localPort        - local port (APPLICABLE RANGES:0..8).
* @param[out] enablePtr       - (pointer to )Port PFC enable option.
* @param[out] thresholdCfgPtr - (pointer to) threshold struct
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcPortPfcThresholdGet
(
     IN  GT_U8                              devNum,
     IN  GT_U32                             tileNum,
     IN  GT_U32                             qfcNum,
     IN  GT_U32                             localPort,
     OUT GT_BOOL                            *enablePtr,
     OUT  CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
     OUT CPSS_DXCH_PFC_HYSTERESIS_CONF_STC  *hysteresisCfgPtr
);

/**
* @internal prvCpssFalconTxqQfcGlobalPfcCfgPbCongestionSet function
* @endinternal
*
* @brief   Set PBB PFC enable per QFC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum              - device number
* @param[in] tileNum             - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum              - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] pbCongestionEnable  - when Enabled, if the PB occupancy is above the configured limit,
*                                  PFC OFF message will be sent to all port.tc
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalPfcCfgPbCongestionSet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN GT_BOOL  pbCongestionEnable
);

/**
* @internal prvCpssFalconTxqQfcGlobalPfcCfgGlobalEnableSet function
* @endinternal
*
* @brief   Set global PFC enable per QFC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum              - device number
* @param[in] tileNum             - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum              - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] pbCongestionEnable  - when Enabled, if the PB occupancy is above the configured limit,
*                                  PFC OFF message will be sent to all port.tc
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqQfcGlobalPfcCfgGlobalEnableSet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  qfcNum,
     IN GT_BOOL  enable
);

/**
* @internal prvCpssFalconTxqQfcTcAvaileblePoolBuffersSet
* @endinternal
*
* @brief   Set amount of buffers available for dynamic allocation for PFC for specific TC.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - physical device number
* @param[in] tileNum          - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum           - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] confMode         - Define what availeble buffer value should be updated (Global TC/Port-TC/Both)
* @param[in] tc               - Traffic class [0..7]
* @param[in] availableBuffers - amount of available buffers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcTcAvaileblePoolBuffersSet
(
     IN  GT_U8                                          devNum,
     IN  GT_U32                                         tileNum,
     IN  GT_U32                                         qfcNum,
     IN  CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT  confMode,
     IN  GT_U8                                          tc,
     IN  GT_U32                                         availeblePoolBuffers
);


/**
* @internal prvCpssFalconTxqQfcTcAvaileblePoolBuffersGet
* @endinternal
*
* @brief   Get amount of buffers available for dynamic allocation for PFC for specific TC.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               - physical device number
* @param[in] tileNum              - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum               - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] confMode             - Define what availeble buffer value should be updated (Global TC/Port-TC/Both)
* @param[in] tc                   - Traffic class [0..7]
* @param[out] availableBuffersPtr - (pointer to)amount of available buffers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcTcAvaileblePoolBuffersGet
(
     IN  GT_U8                                          devNum,
     IN  GT_U32                                         tileNum,
     IN  GT_U32                                         qfcNum,
     IN  CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT  confMode,
     IN  GT_U8                                          tc,
     IN  GT_U32                                         *availeblePoolBuffersPtr
);

/**
* @internal prvCpssFalconTxqQfcUburstEnableSet
* @endinternal
*
* @brief   Enable/disable micro burst event generation per queue
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..399).
* @param[in] queueNum              - local  queue offset (APPLICABLE RANGES:0..7).
* @param[in] enable                - Enable/disable micro burst feature
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqQfcUburstEnableSet
(
     IN  GT_U8                              devNum,
     IN  GT_U32                             tileNum,
     IN  GT_U32                             qfcNum,
     IN  GT_U32                             queueNum,
     IN  GT_BOOL                            enable
);

/**
* @internal prvCpssFalconTxqQfcUburstEnableGet
* @endinternal
*
* @brief   Get enable/disable micro burst event generation per queue
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..399).
* @param[in] queueNum              - local queue offset (APPLICABLE RANGES:0..7).
* @param[in] enable                - (pointer to)Enable/disable micro burst feature
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqQfcUburstEnableGet
(
     IN  GT_U8                              devNum,
     IN  GT_U32                             tileNum,
     IN  GT_U32                             qfcNum,
     IN  GT_U32                             queueNum,
     IN  GT_BOOL                            *enablePtr
);

/**
* @internal prvCpssFalconTxqQfcUburstProfileBindSet
* @endinternal
*
* @brief  Set queue micro burst profile binding
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] queueNum              - local  queue offset (APPLICABLE RANGES:0..399).
* @param[in] profileNum            - Profile number[0..31]
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcUburstProfileBindSet
(
     IN  GT_U8                              devNum,
     IN  GT_U32                             tileNum,
     IN  GT_U32                             qfcNum,
     IN  GT_U32                             queueNum,
     IN  GT_U32                             profileNum
);

/**
* @internal prvCpssFalconTxqQfcUburstProfileBindGet
* @endinternal
*
* @brief  Get queue micro burst profile binding
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[in] queueNum              - local  queue offset (APPLICABLE RANGES:0..399).
* @param[in] profileNumPtr         - (pointer to)Profile number[0..31]
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqQfcUburstProfileBindGet
(
     IN  GT_U8                              devNum,
     IN  GT_U32                             tileNum,
     IN  GT_U32                             qfcNum,
     IN  GT_U32                             queueNum,
     IN  GT_U32                             *profileNumPtr
);
/**
* @internal prvCpssFalconTxqQfcUburstProfileSet
* @endinternal
*
* @brief  Set  micro burst profile attributes
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] profileNum            - Profile number[0..31]
* @param[in] treshold0Trigger      - Threshold 0 trigger (none/fill/drain/either)
* @param[in] treshold0Value        - Threshold 0 value [0..0xFFFFF]
* @param[in] treshold1Trigger      - Threshold 0 trigger (none/fill/drain/either)
* @param[in] treshold1Value        - Threshold 0 value [0..0xFFFFF]
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcUburstProfileSet
(
     IN  GT_U8                                  devNum,
     IN  GT_U32                                 profileNum,
     IN  CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_ENT treshold0Trigger,
     IN  GT_U32                                 treshold0Value,
     IN  CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_ENT treshold1Trigger,
     IN  GT_U32                                 treshold1Value
);

/**
* @internal prvCpssFalconTxqQfcUburstProfileGet
* @endinternal
*
* @brief  Get  micro burst profile attributes
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] profileNum            - Profile number[0..31]
* @param[out] treshold0TriggerPtr  - (pointer to)Threshold 0 trigger (none/fill/drain/either)
* @param[out] treshold0ValuePtr    - (pointer to)Threshold 0 value [0..0xFFFFF]
* @param[out] treshold1TriggerPtr  - (pointer to)Threshold 0 trigger (none/fill/drain/either)
* @param[out] treshold1ValuePtr    - (pointer to)Threshold 0 value [0..0xFFFFF]
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcUburstProfileGet
(
     IN  GT_U8                                    devNum,
     IN  GT_U32                                   profileNum,
     OUT  CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_ENT  *treshold0TriggerPtr,
     OUT  GT_U32                                  *treshold0ValuePtr,
     OUT  CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_ENT  *treshold1TriggerPtr,
     OUT  GT_U32                                  *treshold1ValuePtr
);

/**
* @internal prvCpssFalconTxqQfcUburstEventInfoGet function
* @endinternal
*
* @brief Get micro burst event from specific tile/dp
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                -  physical device number
* @param[in] tileNum               -  current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                -  QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[out] queueNumPtr          - (pointer to)local  queue offset  that had a micro burst
* @param[out] thresholdIdPtr       - (pointer to)Threshold id (0/1)
* @param[out] timestampPtr         - (pointer to)Time stamp of the event
* @param[out] triggerPtr           - (pointer to)Trigger that caused the event(FILL/DRAIN)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NO_MORE - no more new events found
*
*/
GT_STATUS prvCpssFalconTxqQfcUburstEventInfoGet
(
    IN   GT_U8                                    devNum,
    IN   GT_U32                                   tileNum,
    IN   GT_U32                                   qfcNum,
    OUT  GT_U32                                   *queueNumPtr,
    OUT  GT_U32                                   *thresholdIdPtr,
    OUT  CPSS_DXCH_UBURST_TIME_STAMP_STC          *timestampPtr,
    OUT  CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_ENT   *triggerPtr
);

/**
* @internal prvCpssFalconTxqQfcUburstEventPendingGet function
* @endinternal
*
* @brief Get the number of pending micro burst events from specific tile/dp
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - current tile id(APPLICABLE RANGES:0..3).
* @param[in] qfcNum                - QFC that being quried number (APPLICABLE RANGES:0..7).
* @param[out] eventPendingNumPtr   - (pointer to)the number of  micro burst events
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NO_MORE - no more new events found
*
*/
GT_STATUS prvCpssFalconTxqQfcUburstEventPendingGet
(
    IN   GT_U8                                    devNum,
    IN   GT_U32                                   tileNum,
    IN   GT_U32                                   qfcNum,
    IN   GT_BOOL                                  clearOnly,
    OUT  GT_U32                                   *eventPendingNumPtr
);

/**
* @internal  prvCpssFalconTxqQfcPfcStatusGet  function
* @endinternal
*
* @brief   Read QFC status from HW
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - PP's device number.
* @param[in] tileNum               - tile number.
* @param[in] qfcNum                - QFC number
*
* @param[out] globalPfcStatusBmpPtr- (Pointer to)Global PFC status
* @param[out] portPfcStatusBmpPtr  - (Pointer to)Port  PFC status
* @param[out] portTcPfcStatusBmpPtr- (Pointer to)Port TC  PFC status
* @param[out] pfcMessageCounterPtr - (Pointer to)PFC message counter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqQfcPfcStatusGet
(
    IN   GT_U8                                    devNum,
    IN   GT_U32                                   tileNum,
    IN   GT_U32                                   qfcNum,
    OUT  GT_U32                                   *globalPfcStatusBmpPtr,
    OUT  GT_U32                                   *portPfcStatusBmpPtr,
    OUT  GT_U32                                   *portTcPfcStatusBmpPtr,
    OUT  GT_U32                                   *pfcMessageCounterPtr
);

/**
* @internal  prvCpssFalconTxqQfcPfcGlobalStatusParse  function
* @endinternal
*
* @brief   Parse global Pfc Status Bitmap
*
* @note  APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - PP's device number.
* @param[in] globalPfcStatusBmpPtr     - (Pointer to)Global PFC status
* @param[out] pbLimitCrossedPtr        - (Pointer to)PB limit status
* @param[out] pdxLimitCrossedPtr       - (Pointer to) PDX  limit status
* @param[out] iaLimitCrossedPtr        - (Pointer to)IA limit status
* @param[out] globalLimitCrossedBmpPtr - (Pointer to)Global  TC limit status bitmap

* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqQfcPfcGlobalStatusParse
(
    IN   GT_U8                                    devNum,
    IN   GT_U32                                   globalPfcStatusBmp,
    OUT  GT_U32                                   *pbLimitCrossedPtr,
    OUT  GT_U32                                   *pdxLimitCrossedPtr,
    OUT  GT_U32                                   *iaLimitCrossedPtr,
    OUT  GT_U32                                   *globalLimitCrossedBmpPtr
);

/**
* @internal  prvCpssFalconTxqQfcPfcPortStatusParse  function
* @endinternal
*
* @brief  Parse  Pfc  Port Status Bitmap
*
* @note  APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - PP's device number.
* @param[in] portPfcStatusBmpPtr   - (Pointer to)Port PFC status
* @param[in] perTc                 - if equal GT_TRUE then parcing port/tc status,else parsing port status
* @param[out] portLimitCrossedPtr  - (Pointer to) port status

* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqQfcPfcPortStatusParse
(
    IN  GT_U8                                     devNum,
    IN   GT_U32                                   *portPfcStatusBmpPtr,
    IN   GT_BOOL                                   perTc,
    OUT  GT_U32                                   *portLimitCrossedPtr
);

/**
* @internal prvCpssFalconTxqQfcHeadroomCounterGet function
* @endinternal
*
* @brief  Gets the current number of headroom buffers allocated on specified local port/DP/TC
*         for specified Traffic Class queues.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum            - physical device number
* @param[in] tileNum           - tile number.
* @param[in] qfcNum            - QFC number
* @param[in]localPort          - local port (APPLICABLE RANGES:0..8).

* @param[in] tc                - trafiic class (0..7)
* @param[out] hrCountValPtr    - (pointer to)number of buffers
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssFalconTxqQfcHeadroomCounterGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      tileNum,
    IN  GT_U32      qfcNum,
    IN  GT_U32      localPort,
    IN  GT_U32      tc,
    OUT  GT_U32     *hrCountValPtr
);

/**
* @internal prvCpssFalconTxqQfcHeadroomMonitorSet
* @endinternal
*
* @brief  Set tile/dp/local port /tc for monitoring headroom peak.
*  Results are captured at prvCpssFalconTxqQfcMaxMinHeadroomCounterGet
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - tile number.
* @param[in] qfcNum                - QFC number
* @param[in]localPort              - local port (APPLICABLE RANGES:0..8).
* @param[in] tc                    - Traffic class[0..7]
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcHeadroomMonitorSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      tileNum,
    IN  GT_U32      qfcNum,
    IN  GT_U32      localPort,
    IN  GT_U32      tc
);

/**
* @internal prvCpssFalconTxqQfcMaxMinHeadroomCounterGet
* @endinternal
*
* @brief  Get maximal and minimal headroom  headroom peak size for  port/TC set in
*   cpssDxChPortPfcPortTcHeadroomPeakMonitorSet.
*  Note : Peak values are captured since last read.
*
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - tile number.
* @param[in] qfcNum                - QFC number
* @param[out] minPeakValPtr        - (pointer to)Minimal Headroom size
* @param[out] maxPeakValPtr        - (pointer to)Maximal Headroom size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcMaxMinHeadroomCounterGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      tileNum,
    IN  GT_U32      qfcNum,
    OUT  GT_U32     *minCountValPtr,
    OUT  GT_U32     *maxCountValPtr
);

/**
* @internal prvCpssFalconTxqQfcHeadroomThresholdSet
* @endinternal
*
* @brief  Set threshold for headroom size interrupt.Once one  or more of the Port. TC has
*   crossed the configured Threshold, an interrupt is asserted and the Status of the Port. TC is written
*   to the HR crossed Threshold Status.
*
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - tile number.
* @param[in] qfcNum                - QFC number
* @param[in]localPort              - local port (APPLICABLE RANGES:0..8).
* @param[in] threshold             - Headroom threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcHeadroomThresholdSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      tileNum,
    IN  GT_U32      qfcNum,
    IN  GT_U32      localPort,
    IN  GT_U32      threshold
);

/**
* @internal prvCpssFalconTxqQfcHeadroomThresholdGet
* @endinternal
*
* @brief  Get threshold for headroom size interrupt.Once one  or more of the Port. TC has
*   crossed the configured Threshold, an interrupt is asserted and the Status of the Port. TC is written
*   to the HR crossed Threshold Status.
*
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - tile number.
* @param[in] qfcNum                - QFC number
* @param[in]localPort              - local port (APPLICABLE RANGES:0..8).

* @param[out] thresholdPtr         - (pointer to)Headroom threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqQfcHeadroomThresholdGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      tileNum,
    IN  GT_U32      qfcNum,
    IN  GT_U32      localPort,
    OUT GT_U32      *thresholdPtr
);

/**
* @internal prvCpssFalconTxqQfcHeadroomInterruptStatusGet
* @endinternal
*
* @brief  Read HR interrupt cause register from specific tile/qfc
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] tileNum                       tile number.
* @param[in] qfcNum                       QFC number
* @param[out]numOfEventsPtr         (pointer to)number of headroom threshold crossed events.
* @param[out] eventsArr                   Array of headroom threshold crossed events.
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqQfcHeadroomInterruptStatusGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    IN  GT_U32                               qfcNum,
    OUT GT_U32                               *numOfEventsPtr,
    OUT PRV_CPSS_DXCH_HR_EVENT_INFO_STC      *eventsArr
);

/**
* @internal prvCpssFalconTxqQfcHeadroomInterruptEnableSet
* @endinternal
*
* @brief Enable HR threshold crossed interrupt on tile/qfc
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] tileNum                       tile number.
* @param[in] qfcNum                       QFC number
* @param[in] enable                        mask/unmask interrupts
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqQfcHeadroomInterruptEnableSet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    IN  GT_U32                               qfcNum,
    IN  GT_BOOL                              enable
);

/**
* @internal prvCpssFalconTxqQfcHeadroomIntRegisterGet
* @endinternal
*
* @brief  Read HR interrupt cause register from specific tile/qfc/register
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - tile number.
* @param[in] qfcNum                - QFC number
* @param[in] registerInd           - index of register.each register hold status for 3 ports.[0..2]
* @param[out]numOfEventsPtr        - (pointer to)number of headroom threshold crossed events.
* @param[out] eventsArr            - Array of headroom threshold crossed events.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqQfcHeadroomIntRegisterGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    IN  GT_U32                               qfcNum,
    IN  GT_U32                               registerInd,
    OUT GT_U32                               *numOfEventsPtr,
    OUT PRV_CPSS_DXCH_HR_EVENT_INFO_STC      *eventsArr
);

/**
* @internal prvCpssFalconTxqQfcValidateDpSyncronization
* @endinternal
*
* @brief  This function check that all data paths configured similar in regarding
*   to global TC threshold.
*   The function compare configurations to Tile 0 dp 0 and expect all tiles and dps to
*   hold same values.
*
*
* @note   APPLICABLE DEVICES:       Falcon; Hawk;AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[out] tileOutOfSyncPtr     - (pointer to)tile number that is not configred correctly.
* @param[out] dpOutOfSync          - (pointer to)QFC number that is not configred correctly.
* @param[out] isSyncedPtr          - (pointer to)equal GT_TRUE if all tiles are synced ,GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqQfcValidateDpSyncronization
(
    IN  GT_U8                                devNum,
    OUT GT_U32                               *tileOutOfSyncPtr,
    OUT GT_U32                               *dpOutOfSync,
    OUT GT_BOOL                              *isSyncedPtr
);
/**
* @internal prvCpssSip6TxqQfcDebugInterruptDisableSet function
* @endinternal
*
* @brief   Debug function that disable iterrupt in QFC
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] qfcNum                - data path index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqQfcDebugInterruptDisableSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 qfcNum
);

/**
* @internal prvCpssSip6TxqQfcDebugInterruptGet function
* @endinternal
*
* @brief   Get interrupt cause for QFC
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] qfcNum                - data path index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqQfcDebugInterruptGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 qfcNum,
    OUT GT_U32 *functionalPtr,
    OUT GT_U32 *debugPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTxqPsi */

