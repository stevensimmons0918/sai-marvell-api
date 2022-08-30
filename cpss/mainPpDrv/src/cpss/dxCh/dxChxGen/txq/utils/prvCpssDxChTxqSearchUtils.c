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
* @file prvCpssDxChTxqSearchUtils.c
*
* @brief CPSS SIP6 TXQ  search software data base  operation functions
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
 * @internal prvCpssSip6TxqAnodeToPnodeIndexGet function
 * @endinternal
 *
 * @brief   Find P node by A node.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] tileNum                  - tile number to look for.(APPLICABLE RANGES:0..3)
 * @param[in] aNodeIndex                  -index of A node
 * @param[out] pNodePtr                  -(pointer to)index of A node
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong tile number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssSip6TxqAnodeToPnodeIndexGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 aNodeIndex,
    OUT GT_U32 *pNodePtr
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;

    CPSS_NULL_PTR_CHECK_MAC(pNodePtr);
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    tileConfigsPtr  = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[tileNum]);

    if(aNodeIndex>=tileConfigsPtr->mapping.searchTable.aNodeIndexToPnodeIndexSize)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid A node index %d",aNodeIndex);
    }

    *pNodePtr  = tileConfigsPtr->mapping.searchTable.aNodeIndexToPnodeIndex[aNodeIndex];

    return GT_OK;
}


/**
 * @internal prvCpssSip6TxqQnodeToAnodeIndexGet function
 * @endinternal
 *
 * @brief   Find A node by Q node.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] tileNum                  - tile number to look for.(APPLICABLE RANGES:0..3)
 * @param[in] aNodeIndex                  -index of A node
 * @param[out] pNodePtr                  -(pointer to)index of A node
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong tile number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssSip6TxqQnodeToAnodeIndexGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 qNodeIndex,
    OUT GT_U32 *aNodePtr
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;

    CPSS_NULL_PTR_CHECK_MAC(aNodePtr);
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    tileConfigsPtr  = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[tileNum]);

    if(qNodeIndex>=tileConfigsPtr->mapping.searchTable.qNodeIndexToAnodeIndexSize)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid Q node index %d",qNodeIndex);
    }

    *aNodePtr  = tileConfigsPtr->mapping.searchTable.qNodeIndexToAnodeIndex[qNodeIndex];

    return GT_OK;
}



/**
 * @internal prvCpssSip6TxqGoQToPhysicalPortGet function
 * @endinternal
 *
 * @brief   Find physical port  by GoQ  index.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] tileNum                  - tile number to look for.(APPLICABLE RANGES:0..3)
 * @param[in] aNodeIndex                  -index of A node
 * @param[out] pNodePtr                  -(pointer to)index of A node
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong tile number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssSip6TxqGoQToPhysicalPortGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 goQIndex,
    OUT GT_U32 *physicalPortPtr
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;

    CPSS_NULL_PTR_CHECK_MAC(physicalPortPtr);
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    tileConfigsPtr  = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[tileNum]);

    if(goQIndex>=tileConfigsPtr->mapping.searchTable.goqToPhyPortSize)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid GoQ  index %d",goQIndex);
    }

    *physicalPortPtr  = tileConfigsPtr->mapping.searchTable.goqToPhyPortIndex[goQIndex];

    return GT_OK;
}

/*******************************************************************************
* prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet
*
* DESCRIPTION:
*      Read entry from physical port to Anode index data base
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* INPUTS:
*       devNum        -  device number
*       dmaNum       - Global DMA number(0..263).
*       tileNum         - traffic class queue on this device (0..7).
*
* OUTPUTS:
*      aNodeNumPtr   - Index of A node
*      tileNumPtr   - Index of tile
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - wrong sdq number.
*       GT_HW_ERROR              - on writing to HW error.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNumber,
    OUT GT_U32   * tileNumPtr,
    OUT GT_U32   * aNodeNumPtr
)
{
    GT_STATUS               rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *   portMapShadowPtr;


    if (physicalPortNumber >= PRV_CPSS_MAX_PP_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, physicalPortNumber, /*OUT*/ &portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[physicalPortNumber].tileInd  == CPSS_SIP6_TXQ_INVAL_DATA_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "Fail in prvCpssFalconTxqUtilsSetDmaToPnodeMapping.Unexpected tile number");
    }
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[physicalPortNumber].aNodeInd == CPSS_SIP6_TXQ_INVAL_DATA_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "Fail in prvCpssFalconTxqUtilsSetDmaToPnodeMapping.Unexpected tile number");
    }
    *aNodeNumPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[physicalPortNumber].aNodeInd;
    if (tileNumPtr)
    {
        *tileNumPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.physicalPortToAnode[physicalPortNumber].tileInd;
    }
    return GT_OK;
}


/**
 * @internal prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber function
 * @endinternal
 *
 * @brief   Find A node by physical port number
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] tileNum                  - tile number to look for.(APPLICABLE RANGES:0..3)
 * @param[in] portNum                  - physical port number
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong tile number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE ** outPtr
)
{
    GT_U32                  pNodeIndex, aNodeIndex, i;
    GT_STATUS               rc;
    GT_U32                  tileNum;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNodePtr = NULL;;
    rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, portNum, &tileNum, &aNodeIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc  = prvCpssSip6TxqAnodeToPnodeIndexGet(devNum,tileNum,aNodeIndex,&pNodeIndex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqAnodeToPnodeIndexGet failed for index %d",aNodeIndex);
    }

    PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pNodePtr,devNum,tileNum,pNodeIndex);

    for (i = 0; i < pNodePtr->aNodeListSize; i++)
    {
        if (pNodePtr->aNodelist[i].aNodeIndex == aNodeIndex)
        {
            *outPtr = &(pNodePtr->aNodelist[i]);
            return GT_OK;
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}


