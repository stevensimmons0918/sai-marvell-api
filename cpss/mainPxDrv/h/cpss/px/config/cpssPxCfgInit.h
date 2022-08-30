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
* @file cpssPxCfgInit.h
*
* @brief CPSS PX general config APIs.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssPxCfgInit_h
#define __cpssPxCfgInit_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/px/cpssHwInit/cpssPxHwInit.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/common/config/cpssGenCfg.h>

/**
* @struct CPSS_PX_CFG_DEV_INFO_STC
 *
 * @brief Px device info structure
*/
typedef struct{

    /** generic device information */
    CPSS_GEN_CFG_DEV_INFO_STC genDevInfo;

} CPSS_PX_CFG_DEV_INFO_STC;


/**
* @enum CPSS_PX_CFG_CNTR_ENT
 *
 * @brief enumerator for the MCFC counters
*/
typedef enum{

    /** @brief Counts the number of received priority flow control packets,
     *  whether they are dropped or not.
     */
    CPSS_PX_CFG_CNTR_RECEIVED_PFC_E,

    /** @brief Counts the number of received MC packets before replication
     *  which are not QCN.
     */
    CPSS_PX_CFG_CNTR_RECEIVED_MC_E,

    /** @brief Counts the number of received UC packets which are not QCN.
     */
    CPSS_PX_CFG_CNTR_RECEIVED_UC_E,

    /** @brief Counts the number of QCN received and QCN triggered packets
     *  which are not MC.
     */
    CPSS_PX_CFG_CNTR_RECEIVED_QCN_E,

    /** @brief Counts the number of Multicast QCN received packets and
     *  QCN triggered packets (before replication).
     */
    CPSS_PX_CFG_CNTR_RECEIVED_MC_QCN_E,

    /** @brief Counts the number of received packets which arrived with an
     *  empty port map
     */
    CPSS_PX_CFG_CNTR_INGRESS_DROP_E,

    /** @brief Counts the number of outgoing MC replications which are
     *  not QCN.
     */
    CPSS_PX_CFG_CNTR_OUT_MC_PACKETS_E,

    /** @brief Counts the number of outgoing QCN2 Priority Flow Control messages
     *  due to response or Trigger QCN packets.
     */
    CPSS_PX_CFG_CNTR_OUT_QCN_TO_PFC_MSG_E,

    /** @brief Counts the number of outgoing QCN Packets which are not
     *  multicast (both QCN generated in the local device and
     *  QCN which were received on the local device).
     */
    CPSS_PX_CFG_CNTR_OUT_QCN_PACKETS_E,

    /** @brief Counts the number of outgoing PFC messages.
     */
    CPSS_PX_CFG_CNTR_OUT_PFC_PACKETS_E,

    /** @brief Counts the number of outgoing UC packets which are not QCN.
     */
    CPSS_PX_CFG_CNTR_OUT_UC_PACKETS_E,

    /** @brief Counts the number of outgoing MC QCN packets
     *  (both QCN generated in the local device and QCN
     *  which were received on the device).
     */
    CPSS_PX_CFG_CNTR_OUT_MC_QCN_PACKETS_E,

    CPSS_PX_CFG_CNTR_LAST_E

} CPSS_PX_CFG_CNTR_ENT;

/**
* @internal cpssPxCfgDevRemove function
* @endinternal
*
* @brief   Remove the device from the CPSS.
*         This function we release the DB of CPSS that associated with the PP ,
*         and will "know" that the device is not longer valid.
*         This function will not do any HW actions , and therefore can be called
*         before or after the HW was actually removed.
*         After this function is called the devNum is no longer valid for any API
*         until this devNum is used in the initialization sequence for a "new" PP.
*         NOTE: 1. Application may want to disable the Traffic to the CPU prior to
*         this operation (if the device still exists).
*         2. Application may want to a reset the HW PP , and there for need
*         to call the "hw reset" function (before calling this function)
*         and after calling this function to do the initialization
*         sequence all over again for the device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to remove.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is "ISR safe".The interrupt are disabled on API entry and enabled on API exit
*
*/
GT_STATUS cpssPxCfgDevRemove
(
    IN GT_SW_DEV_NUM   devNum
);

/**
* @internal cpssPxCfgDevEnable function
* @endinternal
*
* @brief   This routine sets the device state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to set.
* @param[in] enable                   - GT_TRUE device enable, GT_FALSE disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCfgDevEnable
(
    IN GT_SW_DEV_NUM   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssPxCfgDevEnableGet function
* @endinternal
*
* @brief   This routine gets the device state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to set.
*
* @param[out] enablePtr                - GT_TRUE device enable, GT_FALSE disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssPxCfgDevEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssPxCfgNextDevGet function
* @endinternal
*
* @brief   Return the number of the next existing device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to start from. For the first one  should be 0xFF.
*
* @param[out] nextDevNumPtr            - number of next device after devNum.
*
* @retval GT_OK                    - on success.
* @retval GT_NO_MORE               - devNum is the last device. nextDevNumPtr will be set to 0xFF.
* @retval GT_BAD_PARAM             - devNum > max device number
* @retval GT_BAD_PTR               - nextDevNumPtr pointer is NULL.
*/
GT_STATUS cpssPxCfgNextDevGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT GT_SW_DEV_NUM *nextDevNumPtr
);


/**
* @internal cpssPxCfgTableNumEntriesGet function
* @endinternal
*
* @brief   the function return the number of entries of each individual table in
*         the HW
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
* @param[in] table                    - type of the specific table
*
* @param[out] numEntriesPtr            - (pointer to) number of entries
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCfgTableNumEntriesGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  CPSS_PX_TABLE_ENT           table,
    OUT GT_U32                      *numEntriesPtr
);

/**
* @internal cpssPxCfgReNumberDevNum function
* @endinternal
*
* @brief   function allow the caller to modify the DB of the cpss ,
*         so all info that was 'attached' to 'oldDevNum' will be moved to 'newDevNum'.
*         NOTE:
*         1. there are no HW operations involved in this API.
*         2. This API is "ISR safe".The interrupt are disabled on API entry and enabled on API exit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] oldDevNum                - old device number
* @param[in] newDevNum                - new device number (APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - the device oldDevNum not exist
* @retval GT_OUT_OF_RANGE          - the device newDevNum out of range
* @retval GT_ALREADY_EXIST         - the new device number is already used
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
*
* @note This API is "ISR safe".The interrupt are disabled on API entry and enabled on API exit
*
*/
GT_STATUS cpssPxCfgReNumberDevNum
(
    IN GT_SW_DEV_NUM   oldDevNum,
    IN GT_SW_DEV_NUM   newDevNum
);

/**
* @internal cpssPxCfgDevInfoGet function
* @endinternal
*
* @brief   the function returns device static information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
*
* @param[out] devInfoPtr               - (pointer to) device information
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCfgDevInfoGet
(
    IN  GT_SW_DEV_NUM               devNum,
    OUT CPSS_PX_CFG_DEV_INFO_STC   *devInfoPtr
);

/*******************************************************************************
 * typedef: GT_STATUS (*CPSS_PX_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_READ_FUNC)
 *
 *
 * DESCRIPTION:
 *    Defines table entry read callback function for Hw access.
 *
 * INPUTS:
 *       devNum               - the device number
 *       convertedPortGroupId - the port group Id after conversion if needed, to support
 *                              multi-port-group devices that need to access specific port group
 *       tableType            - the specific table name
 *       convertedEntryIndex  - index in the table after conversion if needed.
 *       inPortGroupId        - original incoming port group.
 *       stage                - stage for callback run.
 *       status               - current status of parent CPSS caller.
 *
 * OUTPUTS:
 *       entryValuePtr - (pointer to) the data read from the table
 *                       may be NULL in the case of indirect table.
 *
 *
 * RETURNS:
 *       GT_OK         - callback success.
 *       GT_FAIL       - callback failed.
 *       GT_ABORTED    - operation aborted by calback.
 *
 * COMMENTS:
 *   When stage equals to CPSS_DRV_HW_ACCESS_STAGE_PRE_E:
 *      1. If callback returns GT_OK the parrent CPSS function continue processing.
 *      2. If callback returns GT_ABORTED, this indicates to the parent CPSS function to stop
 *          and return GT_OK to its caller.
 *
****************************************************************************** */
typedef GT_STATUS (*CPSS_PX_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_READ_FUNC)
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U32                  convertedPortGroupId,
    IN  CPSS_PX_TABLE_ENT       tableType,
    IN  GT_U32                  convertedEntryIndex,
    OUT GT_U32                 *entryValuePtr,
    IN  GT_U32                  inPortGroupId,
    IN  CPSS_DRV_HW_ACCESS_STAGE_ENT    stage,
    IN  GT_STATUS                       status
);

/*******************************************************************************
 * typedef: GT_STATUS (*CPSS_PX_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_WRITE_FUNC)
 *
 *
 * DESCRIPTION:
 *    Defines table entry read callback function for Hw access.
 *
 * INPUTS:
 *       devNum               - the device number
 *       convertedPortGroupId - the port group Id after conversion if needed, to support
 *                              multi-port-group devices that need to access specific port group
 *       tableType            - the specific table name
 *       convertedEntryIndex  - index in the table after conversion if needed.
 *       entryValuePtr        - (pointer to) the data that will be written to the table
 *                               may be NULL in the case of indirect table.
 *       entryMaskPtr         - (pointer to) the mask for entryValuePtr. Only bits raised
 *                              in the entryMaskPtr will be written from entryValuePtr
 *                              into a table.
 *                              If NULL - entryValuePtr will be written as is.
 *       inPortGroupId        - original incoming port group.
 *       stage                - stage for callback run.
 *       status               - current status of parent CPSS caller.
 *
 * OUTPUTS:
 *       None.
 *
 * RETURNS:
 *       GT_OK         - callback success.
 *       GT_FAIL       - callback failed.
 *       GT_ABORTED    - operation aborted by calback.
 *
 * COMMENTS:
 *   When stage equals to CPSS_DRV_HW_ACCESS_STAGE_PRE_E:
 *      1. If callback returns GT_OK the parrent CPSS function continue processing.
 *      2. If callback returns GT_ABORTED, this indicates to the parent CPSS function to stop
 *          and return GT_OK to its caller.
 *
****************************************************************************** */
typedef GT_STATUS (*CPSS_PX_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_WRITE_FUNC)
(
    IN GT_SW_DEV_NUM                 devNum,
    IN GT_U32                        convertedPortGroupId,
    IN CPSS_PX_TABLE_ENT             tableType,
    IN GT_U32                        convertedEntryIndex,
    IN GT_U32                       *entryValuePtr,
    IN GT_U32                       *entryMaskPtr,
    IN GT_U32                        inPortGroupId,
    IN CPSS_DRV_HW_ACCESS_STAGE_ENT  stage,
    IN  GT_STATUS                    status
);


/*******************************************************************************
 * typedef: GT_STATUS (*CPSS_PX_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_FIELD_READ_FUNC)
 *
 *
 * DESCRIPTION:
 *    Defines table entry field read callback function for Hw access.
 *
 * INPUTS:
 *       devNum                - the device number
 *       convertedPortGroupId  - the port group Id after conversion if needed, to support
 *                               multi-port-group devices that need to access specific port group
 *       tableType             - the specific table name
 *       convertedEntryIndex   - index in the table after conversion if needed.
 *       fieldWordNum          - field word number
 *                               use PRV_CPSS_PX_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
 *                               if need global offset in the field of fieldOffset
 *       fieldOffset           - field offset
 *       fieldLength           - field length
 *       inPortGroupId         - original incoming port group.
 *       stage                 - stage for callback run.
 *       status                - current status of parent CPSS caller.
 *
 * OUTPUTS:
 *       entryValuePtr - (pointer to) the data read from the table
 *                       may be NULL in the case of indirect table.
 *
 *
 * RETURNS:
 *       GT_OK         - callback success.
 *       GT_FAIL       - callback failed.
 *       GT_ABORTED    - operation aborted by calback.
 *
 * COMMENTS:
 *   When stage equals to CPSS_DRV_HW_ACCESS_STAGE_PRE_E:
 *      1. If callback returns GT_OK the parrent CPSS function continue processing.
 *      2. If callback returns GT_ABORTED, this indicates to the parent CPSS function to stop
 *          and return GT_OK to its caller.
 *
****************************************************************************** */
typedef GT_STATUS (*CPSS_PX_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_FIELD_READ_FUNC)
(
    IN GT_SW_DEV_NUM           devNum,
    IN GT_U32                  convertedPortGroupId,
    IN CPSS_PX_TABLE_ENT        tableType,
    IN GT_U32                  convertedEntryIndex,
    IN GT_U32                  fieldWordNum,
    IN GT_U32                  fieldOffset,
    IN GT_U32                  fieldLength,
    OUT GT_U32                *fieldValuePtr,
    IN GT_U32                  inPortGroupId,
    IN  CPSS_DRV_HW_ACCESS_STAGE_ENT    stage,
    IN  GT_STATUS                       status
);

/*******************************************************************************
 * typedef: GT_STATUS (*CPSS_PX_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_FIELD_WRITE_FUNC)
 *
 *
 * DESCRIPTION:
 *    Defines table entry field write callback function for Hw access.
 *
 * INPUTS:
 *       devNum                - the device number
 *       convertedPortGroupId  - the port group Id after conversion if needed, to support
 *                               multi-port-group devices that need to access specific port group
 *       tableType             - the specific table name
 *       convertedEntryIndex   - index in the table after conversion if needed.
 *       fieldWordNum          - field word number
 *                               use PRV_CPSS_PX_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
 *                               if need global offset in the field of fieldOffset
 *       fieldOffset           - field offset
 *       fieldLength           - field length
 *       entryValuePtr         - (pointer to) the data that will be written to the table
 *                                may be NULL in the case of indirect table.
 *       inPortGroupId         - original incoming port group.
 *       stage                 - stage for callback run.
 *       status                - current status of parent CPSS caller.
 *
 * OUTPUTS:
 *       None.
 *
 * RETURNS:
 *       GT_OK         - callback success.
 *       GT_FAIL       - callback failed.
 *       GT_ABORTED    - operation aborted by calback.
 *
 * COMMENTS:
 *   When stage equals to CPSS_DRV_HW_ACCESS_STAGE_PRE_E:
 *      1. If callback returns GT_OK the parrent CPSS function continue processing.
 *      2. If callback returns GT_ABORTED, this indicates to the parent CPSS function to stop
 *          and return GT_OK to its caller.
 *
****************************************************************************** */
typedef GT_STATUS (*CPSS_PX_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_FIELD_WRITE_FUNC)
(
    IN GT_SW_DEV_NUM           devNum,
    IN GT_U32                  convertedPortGroupId,
    IN CPSS_PX_TABLE_ENT tableType,
    IN GT_U32                  convertedEntryIndex,
    IN GT_U32                 fieldWordNum,
    IN GT_U32                 fieldOffset,
    IN GT_U32                 fieldLength,
    IN GT_U32                 fieldValue,
    IN GT_U32                 inPortGroupId,
    IN CPSS_DRV_HW_ACCESS_STAGE_ENT    stage,
    IN GT_STATUS                       status
);


/**
* @struct CPSS_PX_CFG_HW_ACCESS_OBJ_STC
 *
 * @brief the structure holds the tables access callback routines.
*/
typedef struct{

    /** table entry read callback function */
    CPSS_PX_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_READ_FUNC hwAccessTableEntryReadFunc;

    /** table entry write callback function */
    CPSS_PX_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_WRITE_FUNC hwAccessTableEntryWriteFunc;

    /** table entry field read callback function */
    CPSS_PX_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_FIELD_READ_FUNC hwAccessTableEntryFieldReadFunc;

    /** @brief table entry field write callback function
     *  COMMENTS:
     *  ----------------------------------------------------------------------------------------
     *  CPSS Function                Hw access Callback
     *  ------------------------------------------------------------------------------------------
     *  1  prvCpssPxPortGroupReadTableEntry       hwAccessTableEntryReadFunc;
     *  2  prvCpssPxPortGroupWriteTableEntry       hwAccessTableEntryWriteFunc;
     *  3  prvCpssPxPortGroupReadTableEntryField    hwAccessTableEntryFieldReadFunc;
     *  4  prvCpssPxPortGroupWriteTableEntryField    hwAccessTableEntryFieldWriteFunc
     *  ---------------------------------------------------------------------------------------------
     */
    CPSS_PX_CFG_ACCESS_PORT_GROUP_TABLE_ENTRY_FIELD_WRITE_FUNC hwAccessTableEntryFieldWriteFunc;

} CPSS_PX_CFG_HW_ACCESS_OBJ_STC;

/**
* @internal cpssPxCfgHwAccessObjectBind function
* @endinternal
*
* @brief   The function binds/unbinds a callback routines for HW access.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] cfgAccessObjPtr          - HW access object pointer.
* @param[in] bind                     -  GT_TRUE -  callback routines.
*                                      GT_FALSE - unbind callback routines.
*
* @retval GT_OK                    - on success
*/
GT_STATUS cpssPxCfgHwAccessObjectBind
(
    IN CPSS_PX_CFG_HW_ACCESS_OBJ_STC *cfgAccessObjPtr,
    IN GT_BOOL                     bind
);

/**
* @internal cpssPxCfgHitlessWriteMethodEnableSet function
* @endinternal
*
* @brief   Enable/disable hitless write method.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  hitless write method is enable. In this case write operation
*                                      would be performed only if writing data is not equal with
*                                      affected memory contents.
*                                      GT_FALSE: hitless write method is disable. In this case write operation
*                                      is done at any rate.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCfgHitlessWriteMethodEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_BOOL       enable
);

/**
* @internal cpssPxCfgLastRegAccessTimeStampGet function
* @endinternal
*
* @brief   Gets the last read/write time stamp counter value that represent the exact
*         timestamp of the last read/write operation to the PP registers.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] timeStampValuePtr        - pointer to timestamp of the last read operation.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portGroupsBmp
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCfgLastRegAccessTimeStampGet
(
    IN  GT_SW_DEV_NUM           devNum,
    OUT GT_U32                  *timeStampValuePtr
);


/**
* @internal cpssPxCfgCntrGet function
* @endinternal
*
* @brief   Gets the MCFC counters for specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] counterType              - the specific counter type
*
* @param[out] cntPtr                   - pointer to the number of packets that were counted by MCFC unit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCfgCntrGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  CPSS_PX_CFG_CNTR_ENT  counterType, 
    OUT GT_U32  *cntPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxCfgInit_h */


