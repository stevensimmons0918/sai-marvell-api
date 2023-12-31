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
* @file cpssDxChBrgStp.h
*
* @brief Spanning Tree protocol facility DxCh CPSS implementation.
*
* @version   11
********************************************************************************
*/
#ifndef __cpssDxChBrgStph
#define __cpssDxChBrgStph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>

/* DxCh size of STG (Spanning Tree Group) entry in words */
/* NOTE:
    devices with up to 32 ports (cpu not included) use 2 words.
    devices with more than 64 ports (Lion2 0..127) use 8 words.
    devices with more than 128 ports (Bobcat2, Caelum, Bobcat3 0..255) use 16 words.

  In general : using 2 bits per port. so 16 ports per 'GT_U32'.
*/
#define CPSS_DXCH_STG_ENTRY_SIZE_CNS    ((PRV_CPSS_MAX_PP_PORTS_NUM_CNS+15)/16)

/**
* @enum CPSS_BRG_STP_STATE_MODE_ENT
 *
 * @brief Enumeration of STP state mode
*/
typedef enum{

    /** @brief STP state for this ePort is
     *  taken from the STP group table indexed by eVLAN
     *  table<Span State Group Index>.
     *  NOTE - eVLAN STG port state is based on
     *  physical port.
     */
    CPSS_BRG_STP_E_VLAN_MODE_E = 0,

    /** @brief STP state for this ePort is
     *  taken from <ePort STP state>
     */
    CPSS_BRG_STP_E_PORT_MODE_E

} CPSS_BRG_STP_STATE_MODE_ENT;

/**
* @internal cpssDxChBrgStpInit function
* @endinternal
*
* @brief   Initialize STP library.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgStpInit
(
    IN GT_U8    dev
);

/**
* @internal cpssDxChBrgStpStateSet function
* @endinternal
*
* @brief   Sets STP state of port belonging within an STP group.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] portNum                  port number
* @param[in] stpId                    - STG (Spanning Tree Group) index
* @param[in] state                    - STP port state.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device or port or stpId or state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgStpStateSet
(
    IN GT_U8                    dev,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U16                   stpId,
    IN CPSS_STP_STATE_ENT       state
);

/**
* @internal cpssDxChBrgStpStateGet function
* @endinternal
*
* @brief   Gets STP state of port belonging within an STP group.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] portNum                  - port number
* @param[in] stpId                    - STG (Spanning Tree Group)
*
* @param[out] statePtr                 - (pointer to) STP port state.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device or port or stpId or state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgStpStateGet
(
    IN GT_U8                    dev,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U16                   stpId,
    OUT CPSS_STP_STATE_ENT      *statePtr
);

/**
* @internal cpssDxChBrgStpEntryGet function
* @endinternal
*
* @brief   Gets the RAW STP entry from HW.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] stpId                    - STG (Spanning Tree Group) index (APPLICABLE RANGES: 0..255)
*
* @param[out] stpEntryWordArr[CPSS_DXCH_STG_ENTRY_SIZE_CNS] - (Pointer to) The raw stp entry to get.
*                                      memory allocated by the caller.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device or stpId
* @retval GT_BAD_PTR               - on NULL pointer parameter
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgStpEntryGet
(
    IN  GT_U8       dev,
    IN  GT_U16      stpId,
    OUT GT_U32      stpEntryWordArr[CPSS_DXCH_STG_ENTRY_SIZE_CNS]
);

/**
* @internal cpssDxChBrgStpEntryWrite function
* @endinternal
*
* @brief   Writes a raw STP entry to HW.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dev                      - device number
* @param[in] stpId                    - STG (Spanning Tree Group) index (APPLICABLE RANGES: 0..255)
* @param[in] stpEntryWordArr[CPSS_DXCH_STG_ENTRY_SIZE_CNS] - (Pointer to) The raw stp entry to write
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device or stpId
* @retval GT_BAD_PTR               - on NULL pointer parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgStpEntryWrite
(
    IN  GT_U8       dev,
    IN  GT_U16      stpId,
    IN  GT_U32      stpEntryWordArr[CPSS_DXCH_STG_ENTRY_SIZE_CNS]
);

/**
* @internal cpssDxChBrgStpPortSpanningTreeStateSet function
* @endinternal
*
* @brief   Set Per-Port STP state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
* @param[in] state                    - STP port state.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable when <Port STP state Mode> = ePort
*
*/
GT_STATUS cpssDxChBrgStpPortSpanningTreeStateSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN CPSS_STP_STATE_ENT   state
);

/**
* @internal cpssDxChBrgStpPortSpanningTreeStateGet function
* @endinternal
*
* @brief   Get Per-Port STP state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] statePtr                 - (pointer to) STP port state.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgStpPortSpanningTreeStateGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             portNum,
    OUT CPSS_STP_STATE_ENT      *statePtr
);

/**
* @internal cpssDxChBrgStpPortSpanningTreeStateModeSet function
* @endinternal
*
* @brief   Determines whether the STP state for this port is
*         taken from the L2I ePort table or from the STP group table via the STG
*         index from eVLAN table
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stateMode                - Port STP state Mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgStpPortSpanningTreeStateModeSet
(
    IN GT_U8                        devNum,
    IN GT_PORT_NUM                  portNum,
    IN CPSS_BRG_STP_STATE_MODE_ENT  stateMode
);

/**
* @internal cpssDxChBrgStpPortSpanningTreeStateModeGet function
* @endinternal
*
* @brief   Get whether the STP state for this port is taken from the
*         L2I ePort table or from the STP group table via the STG index from eVLAN table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] stateModePtr             - (pointer to) Port STP state Mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgStpPortSpanningTreeStateModeGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_NUM                     portNum,
    OUT CPSS_BRG_STP_STATE_MODE_ENT     *stateModePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ____cpssDxChBrgStph */


