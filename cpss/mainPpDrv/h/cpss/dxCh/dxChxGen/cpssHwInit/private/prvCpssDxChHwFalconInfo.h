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
* @file prvCpssDxChHwFalconInfo.h
*
* @brief Private definition for the Falcon devices.
*
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChHwFalconInfoh
#define __prvCpssDxChHwFalconInfoh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>

/**
* @internal prvDxChHwRegAddrFalconDbInit function
* @endinternal
*
* @brief   init the base address manager of the Falcon device.
*         prvDxChFalconUnitsIdUnitBaseAddrArr[]
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvDxChHwRegAddrFalconDbInit(GT_VOID);

/**
* @internal prvCpssFalconGopGlobalMacPortNumToLocalMacPortInPipeConvert function
* @endinternal
*
* @brief   Falcon : convert the global GOP MAC port number in device to local GOP MAC port
*         in the pipe , and the pipeId.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalMacPortNum         - the MAC global port number.
*
* @param[out] pipeIndexPtr             - (pointer to) the pipe Index of the MAC port
* @param[out] localMacPortNumPtr       - (pointer to) the MAC local port number
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconGopGlobalMacPortNumToLocalMacPortInPipeConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalMacPortNum,
    OUT GT_U32  *pipeIndexPtr,
    OUT GT_U32  *localMacPortNumPtr
);

/**
* @internal prvCpssFalconGlobalDpToTileAndLocalDp function
* @endinternal
*
* @brief   Falcon : convert the global DP number in device to local DP number and tile
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalDpIndex             - the DP global number.
*
* @param[out] localDpIndexPtr               - (pointer to) the local Data Path (DP) Index
* @param[out] tileIndexPtr           - (pointer to) the  tile number
*                                       GT_OK on success
*/

GT_STATUS prvCpssFalconGlobalDpToTileAndLocalDp
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalDpIndex,
    OUT  GT_U32 * localDpIndexPtr,
    OUT  GT_U32  * tileIndexPtr
);

/**
* @internal prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert function
* @endinternal
*
* @brief   Falcon : convert the global DMA number in device to TileId and to local
*         DMA number in the Local DataPath (DP index in tile).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] globalDmaNum             - the DMA global number.
*
* @param[out] tileIdPtr                - (pointer to) the tile Id
*                                      dpIndexPtr          - (pointer to) the Data Path (DP) Index (local DP in the tile !!!)
*                                      localDmaNumPtr      - (pointer to) the DMA local number (local DMA in the DP !!!)
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconDmaGlobalNumToTileLocalDpLocalDmaNumConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  globalDmaNum,
    OUT GT_U32  *tileIdPtr,
    OUT GT_U32  *localDpIndexInTilePtr,
    OUT GT_U32  *localDmaNumInDpPtr
);

/**
* @internal prvCpssFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert function
* @endinternal
*
* @brief   Falcon : convert the local DMA number in the Local DataPath (DP index in tile),
*         to global DMA number in the device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] tileId                   - The tile Id
* @param[in] localDpIndexInTile       - the Data Path (DP) Index (local DP in the tile !!!)
* @param[in] localDmaNumInDp          - the DMA local number (local DMA in the DP !!!)
*
* @param[out] globalDmaNumPtr          - (pointer to) the DMA global number in the device.
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvert
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileId,
    IN  GT_U32  localDpIndexInTile,
    IN  GT_U32  localDmaNumInDp,
    OUT GT_U32  *globalDmaNumPtr
);


/**
* @internal prvCpssFalconInitParamsSet function
* @endinternal
*
* @brief   Falcon : init the very first settings in the DB of the device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
*                                       GT_OK on success
*/
GT_STATUS prvCpssFalconInitParamsSet
(
    IN  GT_U8   devNum
);

/**
* @internal prvCpssDxChFalconPortMacDeviceMapGet function
* @endinternal
*
* @brief   Returns tile and raven number mapped to current port MAC
*
* @param[in] devNum                   - device Id
* @param[in] portMacNum               - global MAC number
*
* @param[out] ravenNumPtr             - (pointer to) raven number
* @param[out] localRavenPtr           - (pointer to) local raven in tile
* @param[out] tileNumPtr              - (pointer to) tile number
*
* */
GT_VOID prvCpssDxChFalconPortMacDeviceMapGet
(
    IN GT_U8    devNum,
    IN GT_U32   portMacNum,
    OUT GT_U32  *ravenNumPtr,
    OUT GT_U32  *localRavenPtr,
    OUT GT_U32  *tileNumPtr
);



/**
* @internal prvCpssFalconDmaGlobalDmaMuxed_getNext function
* @endinternal
*
* @brief   Falcon : support 'mux' of SDMA CPU port or network CPU port.
*          the function return 'next' global DMA port number that may share the
*          same local dma number in the same DP.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - The PP's device number.
* @param[out] globalDmaNumPtr          - (pointer to) the 'current' DMA global number.
*
* @param[out] globalDmaNumPtr          - (pointer to) the 'next' DMA global number.
*  GT_OK      on success
*  GT_NO_MORE on success but no more such global DMA port
*  GT_BAD_PARAM on bad param
*  GT_NOT_FOUND on non exists global DMA port
*/
GT_STATUS prvCpssFalconDmaGlobalDmaMuxed_getNext
(
    IN  GT_U8   devNum,
    INOUT GT_U32  *globalDmaNumPtr
);


/**
* @internal prvCpssFalconRavenMemoryAccessCheck function
* @endinternal
*
* @brief   Check memory access to Ravens in Falcon device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum     - the device number
* @param[in] tileId     - tile ID
* @param[in] unitIndex  - unit index
*
* @param[out] skipUnitPtr  - (pointer to) skip status to Raven device
*/
GT_VOID prvCpssFalconRavenMemoryAccessCheck
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileId,
    IN  PRV_CPSS_DXCH_UNIT_ENT  unitIndex,
    OUT GT_BOOL                *skipUnitPtr
);

/**
* @internal prvCpssFalconRavenMemoryAddressSkipCheck function
* @endinternal
*
* @brief   Check skipping of non-valid Raven memory addresses in Falcon device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum         - the device number
* @param[in] address        - memory address in Raven device to be sampled
* @param[in] skipUnitPtr    - pointer to) memory address status
*                               GT_TRUE     - the address should be skipped
*                               GT_FALSE    - the address should NOT be skipped
*/
GT_VOID prvCpssFalconRavenMemoryAddressSkipCheck
(
    IN GT_U8                   devNum,
    IN GT_U32                  address,
    IN GT_BOOL                 *skipUnitPtr
);

GT_VOID prvCpssFalconNonSharedHwInfoFuncPointersSet
(
    IN  GT_U8   devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChHwFalconInfoh */

