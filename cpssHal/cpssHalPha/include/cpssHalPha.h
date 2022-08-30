/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalPha.h
*
* @brief Private API declarations which can be used in XPS layer.
*
* @version   01
********************************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "xpsCommon.h"
#include <gtOs/gtGenTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>

#define CPSSHAL_ERSPAN_IPV4_PHA_THREAD_ID 6
#define CPSSHAL_ERSPAN_IPV6_PHA_THREAD_ID 7

/**
* @internal cpssHalPhaPortEnableEDSATagSet function
* @endinternal
*
* @brief   Enable eDSA tag on a target port for TX.
*
* @param[in] devNum                - device number.
* @param[in] port                  - port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index, interface type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on wrong port or device number in interfacePtr.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalPhaPortEnableEDSATagSet
(
    IN GT_U8   devId,
    IN GT_U32  analyzerPort,
    IN bool    enable
);

/**
* @internal cpssHalPhaThreadInfoSet function
* @endinternal
*
* @brief   Set the entry in the PHA Thread-Id table.
*          Set the ERSPAN entry in the PHA shared memory table.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number.
* @param[in] analyzerIndex            - of analyzer interface. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong phaThreadId number or device or extType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalPhaThreadInfoSet
(
    GT_U8                devId,
    GT_U32               analyzerIndex,
    CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC *erspanEntry
);

/**
* @internal cpssHalPhaPortThreadIdSet function
* @endinternal
*
* @brief   Per target port ,set the associated thread-Id.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - the target port number
* @param[in] enable                   - enable/disable the use of threadId for the target port.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port or phaThreadId
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssHalPhaPortThreadIdSet
(
    IN GT_U8   devId,
    IN GT_U32  ePort,
    IN GT_BOOL enable,
    IN GT_U32  analyzerId
);

GT_STATUS cpssHalPhaThreadInfoGet
(
    GT_U8                devId,
    GT_U32               analyzerIndex,
    CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC *erspanEntry
);


/**
* @internal cpssHalPhaInit function
* @endinternal
*
* @brief   Init the PHA unit in the device.
*
*   NOTEs:
*   1. GT_NOT_INITIALIZED will be return for any 'PHA' lib APIs if called before
*       cpssDxChPhaInit(...)
*       (exclude cpssDxChPhaInit(...) itself)
*   2. GT_NOT_INITIALIZED will be return for EPCL APIs trying to enable PHA processing
*       if called before cpssDxChPhaInit(...)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_ALREADY_EXIST         - the library was already initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalPhaInit
(
    IN GT_U8 devId
);

GT_STATUS cpssHalPhaTargetPortEntrySet
(
    GT_U8                devNum,
    GT_PHYSICAL_PORT_NUM portNum,
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  infoType,
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);

GT_STATUS cpssHalPhaSourcePortEntrySet
(
    GT_U8                devNum,
    GT_PHYSICAL_PORT_NUM portNum,
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  infoType,
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);

#ifdef __cplusplus
}
#endif
