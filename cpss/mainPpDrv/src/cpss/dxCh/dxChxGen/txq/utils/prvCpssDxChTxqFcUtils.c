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
* @file prvCpssDxChTxqFcUtils.c
*
* @brief CPSS SIP6 TXQ  flow control  functions
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqSdq.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqFcGopUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPfcc.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqQfc.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqTailDropUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#define PRV_CPSS_DXCH_FALCON_TXQ_MAX_QUANTA_MAC 0xFFFF
#define PRV_CPSS_DXCH_FALCON_TXQ_DEFAULT_THRESHOLD_MAC 0xF000

#define PRV_CPSS_FC_PB_SIZE_PER_TILE_MAC(_devNum)  (prvCpssTxqUtilsPbSizeInBuffersGet(_devNum))


/**
 * @internal prvCpssFalconTxqUtilsPfcEnable function
 * @endinternal
 *
 * @brief   Set PFC  enable status for both generation and responce  at
 *               TxQ and MAC
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -         `                          physical device number
 * @param[in] portNum                                                         physical port number
 * @param[out] pfcRespBitMapPtr                                         (pointer to)current PFC responce mode
 * @param[out] pfcGenerationModePtr                      -        (pointer to)current PFC generation mode
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsPfcEnable
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32                               *pfcRespBitMapPtr,
    IN CPSS_DXCH_PORT_FC_MODE_ENT           *pfcGenerationModePtr
)
{
    GT_STATUS           rc;
    GT_U32              tileNum, dpNum, localdpPortNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
    CPSS_NULL_PTR_CHECK_MAC(pfcRespBitMapPtr);
    CPSS_NULL_PTR_CHECK_MAC(pfcGenerationModePtr);
    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localdpPortNum, &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }
    /*1.Write PFC responce bitmap*/
    rc = prvCpssFalconTxqSdqPfcControlEnableBitmapSet(devNum, tileNum, dpNum, localdpPortNum, *pfcRespBitMapPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqPfcControlEnableBitmapSet  failed for localdpPortNum  %d  ", localdpPortNum);
    }
    /*2.Write PFC generation mode*/
    rc = prvCpssFalconTxqQfcLocalPortToSourcePortSet(devNum, tileNum, dpNum, localdpPortNum, 0, *pfcGenerationModePtr, GT_TRUE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcLocalPortToSourcePortSet failed for localdpPortNum %d", localdpPortNum);
    }
    /*4.Enable PFC responce at MAC*/
    rc = prvCpssDxChPortSip6PfcPauseIgnoreEnableSet(devNum, portNum, GT_FALSE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcLinkPauseIgnoreEnableSet  failed for portNum  %d  ", portNum);
    }

    /*5.Enable PFC generation  at MAC. Handle xoff_gen bus towards MAC*/
    rc = prvCpssDxChPortSip6XoffOverrideEnableSet(devNum, portNum, GT_FALSE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcLinkPauseIgnoreEnableSet  failed for portNum  %d  ", portNum);
    }
    return GT_OK;
}

/**
 * @internal prvCpssFalconTxqUtilsFlowControlInitAvailebleBufferSize function
 * @endinternal
 *
 * @brief  Initialize available PB size used for DBA. Initialize to maximal value.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -         `                          physical device number
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
static GT_STATUS prvCpssFalconTxqUtilsFlowControlInitAvailebleBufferSize
(
    IN GT_U8 devNum
)
{
    GT_U32      numberOfTiles;
    GT_STATUS   rc;
    GT_U8       trafficClass;
    switch (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
    case 0:
    case 1:
        numberOfTiles = 1;
        break;
    case 2:
    case 4:
        numberOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ", PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
        break;
    }
    for (trafficClass = 0; trafficClass < TC_NUM_CNS; trafficClass++)
    {
        rc = cpssDxChPortPfcDbaTcAvailableBuffersSet(devNum, CPSS_DATA_PATH_UNAWARE_MODE_CNS,
                                 CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_GLOBAL_AND_PORT_TC_E,
                                 trafficClass, numberOfTiles * PRV_CPSS_FC_PB_SIZE_PER_TILE_MAC(devNum));
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxMcastAvailableBuffersSet failed ");
        }
    }
    rc = cpssDxChPortPfcDbaAvailableBuffSet(devNum, CPSS_DATA_PATH_UNAWARE_MODE_CNS,
                        numberOfTiles * PRV_CPSS_FC_PB_SIZE_PER_TILE_MAC(devNum));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortPfcDbaAvailableBuffSet failed ");
    }
    return GT_OK;
}

/**
 * @internal prvCpssFalconTxqUtilsFlowControlInit function
 * @endinternal
 *
 * @brief   This function  initialize PFC quanta and PFC threshold for all mapped ports
 *
 * @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                         PP's device number.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsFlowControlInit
(
    IN GT_U8 devNum
)
{
    GT_U32                  i, j;
    GT_STATUS               rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *   portMapShadowPtr;
    GT_U32                  masterTile = PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC;
    PRV_CPSS_PFCC_TILE_INIT_STC     initDb;
    for (i = 0; i < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; i++)
    {
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, i, /*OUT*/ &portMapShadowPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        if (portMapShadowPtr->valid == GT_TRUE)
        {
            if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                for (j = 0; j < 8; j++)
                {
                    rc = prvCpssDxChPortSip6PfcPauseQuantaSet(devNum, i, j, PRV_CPSS_DXCH_FALCON_TXQ_MAX_QUANTA_MAC);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortSip6PfcPauseQuantaSet failed for port %d tc % d\n", i, j);
                    }
                    rc = prvCpssDxChPortSip6PfcQuantaThreshSet(devNum, i, j, PRV_CPSS_DXCH_FALCON_TXQ_DEFAULT_THRESHOLD_MAC);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortSip6PfcQuantaThreshSet failed for port %d tc % d\n", i, j);
                    }
                }
            }
        }
    }
    rc = prvCpssFalconTxqUtilsFlowControlInitAvailebleBufferSize(devNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsFlowControlInitBufferSize failed.\n");
    }
    /*Now after availeble buffers are set , enable the chain*/
    rc = prvCpssFalconTxqPfccUnitCfgGet(devNum, masterTile, &initDb);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccUnitCfgGet  failed for tile  %d  ", masterTile);
    }
    initDb.pfccEnable   = GT_TRUE;
    rc          = prvCpssFalconTxqPfccUnitCfgSet(devNum, masterTile, &initDb);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccUnitCfgSet  failed for tile  %d  ", masterTile);
    }
    return GT_OK;
}


/**
 * @internal prvCpssFalconTxqUtilsGlobalFlowControlEnableSet  function
 * @endinternal
 *
 * @brief   Enable/disble flow control globally.
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 * @param[in] fcEnable                   - GT_TRUE,  enable flow control globally,
 *                                                         GT_FALSE, disable flow control globally
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device number, profile set or
 *                                       traffic class
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters is NULL pointer
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_BAD_STATE - in case of enabling low latency mode on port  that already
 *                                                contain latency queue.
 */
GT_STATUS prvCpssFalconTxqUtilsGlobalFlowControlEnableSet
(
    IN GT_U8 devNum,
    IN GT_BOOL fcEnable
)
{
    GT_U32                  i;
    GT_STATUS               rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *   portMapShadowPtr;
    for (i = 0; i < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; i++)
    {
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, i, /*OUT*/ &portMapShadowPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        if (portMapShadowPtr->valid == GT_TRUE)
        {
            if ((portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
                && (portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E))
            {
                rc = prvCpssDxChPortSip6XoffOverrideEnableSet(devNum, i, !fcEnable);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortSip6PfcPauseQuantaSet failed for port %d \n", i);
                }
            }
        }
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsGlobalFlowControlEnableGet  function
 * @endinternal
 *
 * @brief   Get enable/disble flow control globally.
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 * @param[out] fcEnablePtr                   -(pointer to) GT_TRUE,  enable flow control globally,
 *                                                         GT_FALSE, disable flow control globally
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device number, profile set or
 *                                       traffic class
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters is NULL pointer
 * @retval GT_OUT_OF_RANGE          - on out of range value
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_BAD_STATE - in case of enabling low latency mode on port  that already
 *                                                contain latency queue.
 */
GT_STATUS prvCpssFalconTxqUtilsGlobalFlowControlEnableGet
(
    IN GT_U8 devNum,
    OUT GT_BOOL *fcEnablePtr
)
{
    GT_U32                  i;
    GT_STATUS               rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *   portMapShadowPtr;
    GT_U32                  value;
    *fcEnablePtr = GT_TRUE;
    for (i = 0; i < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; i++)
    {
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, i, /*OUT*/ &portMapShadowPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        if (portMapShadowPtr->valid == GT_TRUE)
        {
            if ((portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
                && (portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E))
            {
                rc = prvCpssDxChPortSip6XoffOverrideEnableGet(devNum, i, &value);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortSip6PfcPauseQuantaSet failed for port %d \n", i);
                }
                if (value != 0x0)
                {
                    *fcEnablePtr = GT_FALSE;
                    break;
                }
            }
        }
    }
    return GT_OK;
}

/**
 * @internal prvCpssFalconTxqUtilsPfcDisable function
 * @endinternal
 *
 * @brief   Read PFC  enable status for both generation and responce and then disable PFC on RX/TX at
 *               TxQ and MAC
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -         `                          physical device number
 * @param[in] portNum                                                         physical port number
 * @param[out] pfcRespBitMapPtr                                         (pointer to)current PFC responce mode
 * @param[out] pfcGenerationModePtr                      -        (pointer to)current PFC generation mode
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsPfcDisable
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32                               *pfcRespBitMapPtr,
    IN CPSS_DXCH_PORT_FC_MODE_ENT           *pfcGenerationModePtr
)
{
    GT_STATUS           rc;
    GT_U32              tileNum, dpNum, localdpPortNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
    CPSS_NULL_PTR_CHECK_MAC(pfcRespBitMapPtr);
    CPSS_NULL_PTR_CHECK_MAC(pfcGenerationModePtr);
    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localdpPortNum, &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }
    /*1.Read PFC responce bitmap*/
    rc = prvCpssFalconTxqSdqPfcControlEnableBitmapGet(devNum, tileNum, dpNum, localdpPortNum, pfcRespBitMapPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqPfcControlEnableBitmapGet  failed for localdpPortNum  %d  ", localdpPortNum);
    }
    /*2.Read PFC generation mode*/
    rc = prvCpssFalconTxqQfcLocalPortToSourcePortGet(devNum, tileNum, dpNum, localdpPortNum, NULL, pfcGenerationModePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcLocalPortToSourcePortGet failed for localdpPortNum %d", localdpPortNum);
    }
    /*3.Disable PFC responce at SDQ*/
    rc = prvCpssFalconTxqSdqPfcControlEnableBitmapSet(devNum, tileNum, dpNum, localdpPortNum, 0);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqPfcControlEnableBitmapGet  failed for localdpPortNum  %d  ", localdpPortNum);
    }
    /*4.Disable PFC responce at MAC*/
    rc = prvCpssDxChPortSip6PfcPauseIgnoreEnableSet(devNum, portNum, GT_TRUE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcLinkPauseIgnoreEnableSet  failed for portNum  %d  ", portNum);
    }

    /*5.Disable PFC generation  at MAC. Handle xoff_gen bus towards MAC*/
    rc = prvCpssDxChPortSip6XoffOverrideEnableSet(devNum, portNum, GT_TRUE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcLinkPauseIgnoreEnableSet  failed for portNum  %d  ", portNum);
    }

    /*6.Disable PFC generation at QFC*/
    rc = prvCpssFalconTxqQfcLocalPortToSourcePortSet(devNum, tileNum, dpNum, localdpPortNum, 0, CPSS_DXCH_PORT_FC_MODE_DISABLE_E, GT_TRUE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcLocalPortToSourcePortGet failed for localdpPortNum %d", localdpPortNum);
    }
    return GT_OK;
}

/**
 * @internal prvCpssFalconTxqPfcValidGet function
 * @endinternal
 *
 * @brief   This function check if PFC is configured per port or port/TC
 *
 * @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                          PP's device number.
 * @param[in] tileNum                           tile number
 * @param[in] dpNum         -                 data path[0..7]
 * @param[in] localPort                         local port number[0..8]
 * @param[in] tcBmpPtr                           (pointer to) traffic class that is set for flow control,0x0 mean no flow control is set,0xFF mean flow control on port
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqPfcValidGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 dpNum,
    IN GT_U32 localPort,
    OUT GT_U32                 *tcBmpPtr
)
{
    GT_STATUS           rc;
    CPSS_DXCH_PFC_THRESHOLD_STC thresholdCfg;
    GT_BOOL             enable;
    GT_U32              i;
    *tcBmpPtr   = 0;
    rc      = prvCpssFalconTxqQfcPortPfcThresholdGet(devNum, tileNum, dpNum, localPort, &enable, &thresholdCfg, NULL);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcPortPfcThresholdGet  failed for localPort  %d  ", localPort);
    }
    if (enable == GT_TRUE)
    {
        *tcBmpPtr = 0xFF;
        return GT_OK;
    }
    /*Check port/TC*/
    for (i = 0; i < 8; i++)
    {
        rc = prvCpssFalconTxqQfcPortTcPfcThresholdGet(devNum, tileNum, dpNum, localPort, i, &enable, &thresholdCfg);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcPortTcPfcThresholdGet  failed for localPort  %d  ", localPort);
        }
        if (enable == GT_TRUE)
        {
            *tcBmpPtr |= 1 << i;
        }
    }
    return GT_OK;
}

