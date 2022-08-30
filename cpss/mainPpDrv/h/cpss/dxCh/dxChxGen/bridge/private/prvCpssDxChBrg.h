/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file prvCpssDxChBrg.h
*
* @brief Common private bridge declarations.
*
* @version   6
********************************************************************************
*/
#ifndef __prvCpssDxChBrgh
#define __prvCpssDxChBrgh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/config/private/prvCpssGenIntDefs.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgFdbAu.h>

/* number of words in the Au (address update) message */
#define CPSS_DXCH_FDB_AU_MSG_WORDS_SIZE_CNS   8

/* number of 32 Bit words/bytes in the data of NA Au (address update) to CPU message */
#define PRV_CPSS_DXCH_FDB_AU_NA_TO_CPU_MSG_DATA_WORDS_SIZE_CNS   6
#define PRV_CPSS_DXCH_FDB_AU_NA_TO_CPU_MSG_DATA_BYTES_SIZE_CNS   (PRV_CPSS_DXCH_FDB_AU_NA_TO_CPU_MSG_DATA_WORDS_SIZE_CNS*4)

/**
* @enum HW_FDB_ENTRY_EXT_TYPE_ENT
 *
 * @brief hold the different types FDB Entries in HW format
*/
typedef enum{
    HW_FDB_ENTRY_TYPE_MAC_ADDR_E = 0,
    HW_FDB_ENTRY_TYPE_IPV4_MCAST_E = 1,
    HW_FDB_ENTRY_TYPE_IPV6_MCAST_E = 2,
    HW_FDB_ENTRY_TYPE_IPV4_UC_E    = 3,
    HW_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E = 4,
    HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E = 5
} HW_FDB_ENTRY_EXT_TYPE_ENT;

/**
* @internal prvCpssDxChBrgCpuPortBmpConvert function
* @endinternal
*
* @brief   Convert port bitmap according to physical CPU port connection.
*
* @note   APPLICABLE DEVICES:       Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   None.
*
* @param[in] devNum                   - device number
* @param[in] isWriteConversion        - direction of conversion
*                                      GT_TRUE - write conversion
*                                      GT_FALSE - read conversion
* @param[in] portBitmapPtr            - (pointer to) bmp of ports members in vlan
*                                      CPU port supported
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvCpssDxChBrgCpuPortBmpConvert
(
    IN GT_U8                devNum,
    IN GT_BOOL              isWriteConversion,
    IN CPSS_PORTS_BMP_STC   *portBitmapPtr
);

/**
* @internal prvCpssDxChFullQueueRewind function
* @endinternal
*
* @brief   Rewind Full FU or AU queue.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] queueType                - queue type : one of :
*                                      MESSAGE_QUEUE_PRIMARY_AUQ_E
*                                      MESSAGE_QUEUE_PRIMARY_FUQ_E
*                                      MESSAGE_QUEUE_CNC23_FUQ_E
* @param[in] descCtrlPtr             - pointer to Descriptor Control Structure passed by caller
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChFullQueueRewind
(
    IN  GT_U8                       devNum,
    IN  MESSAGE_QUEUE_ENT           queueType,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *descCtrlPtr
);


/**
* @internal prvCpssDxChFuqOwnerSet function
* @endinternal
*
* @brief   Set FUQ used for CNC / FDB upload.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Lion2; Falcon)
*                                      a. see general note about portGroupsBmp in FDB APIs for Falcon
* @param[in] isCnc                    - is CNC need to be used ?
*                                      GT_TRUE - CNC need to use the FUQ
*                                      GT_FALSE - FDB upload need to use the FUQ
* @param[in] fuDescCtrlPtr            - pointer to the FU block descriptor
*
**
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChFuqOwnerSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portGroupId,
    IN GT_BOOL                      isCnc,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *fuDescCtrlPtr
);

/**
* @internal prvCpssDxChBrgFdbSizeSet function
* @endinternal
*
* @brief   function to set the FDB size.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] fdbSize                  - the FDB size to set.
*                                      (each device with it's own relevant values)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad device or FDB table size
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBrgFdbSizeSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  fdbSize
);

/**
* @internal prvCpssDxChBrgFdbMhtSet function
* @endinternal
*
* @brief   function to set the FDB MHT (Number of Multiple Hash Tables).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] mht                      - the FDB MHT (Number of Multiple Hash Tables)
*                                       4/8/16
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad device or EM table size
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBrgFdbMhtSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mht
);

/**
* @internal prvCpssDxChBrgFdbHashRequestSend function
* @endinternal
*
* @brief   The function Send Hash request (HR) message to PP to generate all values
*         of hash results that relate to the 'Entry KEY'.
*         The function checks that AU messaging is ready before using it.
*         The function does not check that HR message was processed by PP.
*         For the results Application can call cpssDxChBrgFdbHashResultsGet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] entryKeyPtr              - (pointer to) entry key
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or entryKeyPtr->entryType
* @retval GT_OUT_OF_RANGE          - one of the parameters of entryKeyPtr is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - the PP is not ready to get a message from CPU.
*                                       (PP still busy with previous message)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Application should synchronize call of cpssDxChBrgFdbMacEntrySet,
*       cpssDxChBrgFdbQaSend , cpssDxChBrgFdbMacEntryDelete , cpssDxChBrgFdbHashRequestSend
*       , cpssDxChBrgFdbPortGroupMacEntrySet, cpssDxChBrgFdbPortGroupQaSend ,
*       cpssDxChBrgFdbPortGroupMacEntryDelete , cpssDxChBrgFdbMacEntryMove functions.
*       Invocations of these functions should be mutual exclusive because they
*       use same HW resources.
*       Application can check that the HR message processing has completed by
*       cpssDxChBrgFdbFromCpuAuMsgStatusGet or cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet.
*
*/
GT_STATUS prvCpssDxChBrgFdbHashRequestSend
(
    IN  GT_U8                       devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC   *entryKeyPtr
);

/**
* @internal prvCpssDxChEgfPortLinkFilterRestoreApplicationIfAllowed function
* @endinternal
*
* @brief  restore value that application asked for when called cpssDxChBrgEgrFltPortLinkEnableSet(...),
*         the CPSS internal logic set the filter as 'force link DOWN' as protection on the TXQ at some points.
*         This function is to set (Restore) value of the application (only if terms allow it !)
*         This function uses the DB of 'portEgfForceStatusBitmapPtr' as 'what application asked for.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChEgfPortLinkFilterRestoreApplicationIfAllowed
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum
);

/**
* @internal prvCpssDxChEgfPortLinkFilterDbGet function
* @endinternal
*
* @brief  get egf status from db
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_U32 prvCpssDxChEgfPortLinkFilterDbGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum
);

/**
* @internal prvCpssDxChEgfPortLinkFilterForceLinkDown function
* @endinternal
*
* @brief  'force link DOWN' as protection on the TXQ at some points.
*         ignoring value requested by the application when called cpssDxChBrgEgrFltPortLinkEnableSet(...),
*         NOTE: the 'restore' logic is in function prvCpssDxChEgfPortLinkFilterRestoreApplicationIfAllowed(...)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChEgfPortLinkFilterForceLinkDown
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum
);

/**
* @internal prvCpssDxChBrgFdbAgeBitDaRefreshEnableSet function
* @endinternal
*
* @brief   Enables/disables destination address-based aging : flag for UC and flag for MC.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] destinationUcRefreshEnable - GT_TRUE -  enable  UC refreshing
*                                         GT_FALSE - disable UC refreshing
* @param[in] destinationMcRefreshEnable - GT_TRUE -  enable  MC refreshing
*                                         GT_FALSE - disable MC refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* COMMENTS
*/
GT_STATUS prvCpssDxChBrgFdbAgeBitDaRefreshEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL destinationUcRefreshEnable,
    IN GT_BOOL destinationMcRefreshEnable
);
/**
* @internal prvCpssDxChBrgFdbAgeBitDaRefreshEnableGet function
* @endinternal
*
* @brief   Get the Enables/disables destination address-based aging : flag for UC and flag for MC.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[out] destinationUcRefreshEnablePtr - (pointer to)GT_TRUE -  enable  UC refreshing
*                                            GT_FALSE - disable UC refreshing
* @param[out] destinationMcRefreshEnablePtr - (pointer to)GT_TRUE -  enable  MC refreshing
*                                            GT_FALSE - disable MC refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* COMMENTS
*/
GT_STATUS prvCpssDxChBrgFdbAgeBitDaRefreshEnableGet
(
    IN GT_U8   devNum,
    OUT GT_BOOL *destinationUcRefreshEnablePtr,
    OUT GT_BOOL *destinationMcRefreshEnablePtr
);

/**
* @internal prvCpssDxChBrgFdbPortGroupQueueFullGet function
* @endinternal
*
* @brief   The function returns the state that the AUQ/FUQ - is full/not full
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in FDB APIs for Falcon
* @param[in] queueType                - queue type AU/FU queue
*                                      NOTE: when using shared queue for AU and FU messages, this
*                                      parameter is ignored (and the AUQ is queried)
*
* @param[out] isFullPortGroupsBmpPtr   - (pointer to) bitmap of port groups on which
*                                      'the queue is full'.
*                                      for NON multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Falcon)
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS means that
*                                      'queue is full' , otherwise 'queue is NOT full'
*                                      for multi-port groups device :
*                                      when bit is set (1) - 'queue is full' (in the
*                                      corresponding port group).
*                                      when bit is not set (0) - 'queue is NOT full'. (in the corresponding
*                                      port group)
*                                       a. see general note about portGroupsBmp in FDB APIs for Falcon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad devNum or portGroupsBmp or queueType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBrgFdbPortGroupQueueFullGet
(
    IN GT_U8    devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN CPSS_DXCH_FDB_QUEUE_TYPE_ENT  queueType,
    IN PRV_CPSS_DXCH_MG_CLIENT_ENT  mgClient,
    OUT GT_PORT_GROUPS_BMP  *isFullPortGroupsBmpPtr
);

/**
* @internal prvCpssDxChBrgL2DlbWaInit function
* @endinternal
*
* @brief  Allocates the required data structure.
*         Configures the maximum granularity for the DLB clock.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] secondaryLttBaseIndex - Base index to the secondary region of LTT table.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBrgL2DlbWaInit
(
    IN GT_U8                                devNum,
    IN GT_U32                               secondaryLttBaseIndex
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChBrgh */

