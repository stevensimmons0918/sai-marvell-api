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
* @file prvCpssDxChBrgFdbManager_hw.c
*
* @brief FDB manager support - manipulations of HW device(s) : read/write to device(s).
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_utils.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_db.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_hw.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_debug.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#if 0
static GT_STATUS prvCpssDxChBrgFdbManagerConvertDbEntryFormatFromHw
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC    *fdbManagerPtr,
    IN GT_U32                                            *hwDataArr
)
{
    GT_U32          hwValue, hwValue1, hwValue2, hwValue3;
    GT_U8           macAddr[6];

    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
        hwValue);

    if(hwValue != PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E)
    {
        cpssOsPrintf("Only MAC format supported\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* valid */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_VALID_E, hwValue);
    cpssOsPrintf("Field Name - [Valid], Field Value -[%d]\n", hwValue);

    /* skip */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_SKIP_E, hwValue);
    cpssOsPrintf("Field Name - [Skip], Field Value -[%d]\n", hwValue);

    /* age */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_AGE_E, hwValue);
    cpssOsPrintf("Field Name - [AGE], Field Value -[%d]\n", hwValue);

    /* FID */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_FID_E, hwValue);
    cpssOsPrintf("Field Name - [FID], Field Value -[%d]\n", hwValue);

    /* MAC address */
    SIP6_FDB_HW_ENTRY_FIELD_MAC_ADDR_GET_MAC(hwDataArr, macAddr);
    cpssOsPrintf("Field Name - [mac], Field Value -[%d:%d:%d:%d:%d:%d]\n",
            macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

    /* SA cmd */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_SA_CMD_E, hwValue);
    cpssOsPrintf("Field Name - [SA CMD], Field Value -[%d]\n", hwValue);

    /* static */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_IS_STATIC_E, hwValue);
    cpssOsPrintf("Field Name - [IS static], Field Value -[%d]\n", hwValue);

    /* DA cmd */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_DA_CMD_E, hwValue);
    cpssOsPrintf("Field Name - [DA CMD], Field Value -[%d]\n", hwValue);

    /* Device ID */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_DEV_ID_E, hwValue);
    cpssOsPrintf("Field Name - [Device ID], Field Value -[%d]\n", hwValue);

    /* SA Access Level */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_SA_ACCESS_LEVEL_E, hwValue);
    cpssOsPrintf("Field Name - [SA Access], Field Value -[%d]\n", hwValue);

    /* DA Access Level */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL_E, hwValue);
    cpssOsPrintf("Field Name - [DA Access], Field Value -[%d]\n", hwValue);

    /* VID1 */
    if(fdbManagerPtr->entryAttrInfo.macEntryMuxingMode == CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E)
    {
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_0_E, hwValue);
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_6_1_E, hwValue1);
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_11_7_E, hwValue2);
        cpssOsPrintf("Field Name - [VID1], Field Value -[%d:%d:%d]\n", hwValue, hwValue1, hwValue2);
    }
    else /* SRC ID */
    {
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_0_E, hwValue);
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1_E, hwValue1);
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_6_3_E, hwValue2);
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_11_7_E, hwValue3);
        cpssOsPrintf("Field Name - [SRC_ID], Field Value -[%d:%d:%d:%d]\n", hwValue, hwValue1, hwValue2, hwValue3);
    }

    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_10_7_E, hwValue);
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_6_5_E, hwValue1);
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_4_1_E, hwValue2);
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_0_E, hwValue3);
    cpssOsPrintf("Field Name - [USER Defined], Field Value -[%d:%d:%d:%d]\n", hwValue, hwValue1, hwValue2, hwValue3);

    /* SPUNKNOWN */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E, hwValue);
    cpssOsPrintf("Field Name - [SPUnknown], Field Value -[%d]\n", hwValue);

    /* VIDX */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_VIDX_E, hwValue);
    cpssOsPrintf("Field Name - [vidx], Field Value -[%d]\n", hwValue);

    /* VIDX */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_IS_TRUNK_E, hwValue);
    cpssOsPrintf("Field Name - [IsTrunk], Field Value -[%d]\n", hwValue);
    if(hwValue)
    {
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_TRUNK_NUM_E, hwValue);
        cpssOsPrintf("Field Name - [Trunk NO], Field Value -[%d]\n", hwValue);
    }
    else
    {
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_EPORT_NUM_E, hwValue);
        cpssOsPrintf("Field Name - [Eport No], Field Value -[%d]\n", hwValue);
    }

    return GT_OK;
}
#endif

/**
* @internal prvCpssDxChFdbManagerHwWriteEntry function
* @endinternal
*
* @brief   Write a whole FDB entry to the HW in all port groups.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] hwIndex                  - index in the table
* @param[in] wordsArr                 - (pointer to) the 4 words data.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of wordsArr == NULL and indirect table the function
*       just send write entry command to device.
*       And entry is taken from the data registers of a indirect table.
*
*/
GT_STATUS prvCpssDxChFdbManagerHwWriteEntry
(
    IN GT_U8                    devNum,
    IN GT_U32                   hwIndex,
    IN GT_U32                   wordsArr[]
)
{
    const PRV_CPSS_DXCH_TABLES_INFO_STC           *tableInfoPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,CPSS_DXCH_TABLE_FDB_E);

    /* do 'shortcut' instead of calling :
        rc = prvCpssDxChPortGroupWriteTableEntry(devNum,
            0xFFFFFFFF,
            CPSS_DXCH_TABLE_FDB_E,
            hwIndex,
            &wordsArr[0]);
    */

    /* Write a whole entry to table by indirect access method */
    return prvCpssDxChWriteTableEntry_indirect(devNum, 0xFFFFFFFF,
             CPSS_DXCH_TABLE_FDB_E, tableInfoPtr, hwIndex,wordsArr,NULL);

}

/**
* @internal prvCpssDxChFdbManagerAacHwAgeBitCheck function
* @endinternal
*
* @brief   Checks FDB age bits from the HW using AAC method.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - the device number
* @param[in] hwIndex               - index in the table 
* @param[out] agedOutPtr           - (pointer to) the aged-out status.
*                                    GT_TRUE  - Entry is aged-out
*                                    GT_FALSE - Entry is not aged-out
* @param[out] hwDataPtr            - (pointer to) the HW entry with the age bit set
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_HW_ERROR              - on hardware error
*
*/
static GT_STATUS prvCpssDxChFdbManagerAacHwAgeBitCheck
(
    IN GT_U8                    devNum,
    IN GT_U32                   hwIndex,
    OUT GT_BOOL                 *agedOutPtr,
    OUT GT_U32                  *hwDataPtr
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      regAddr;            /* register address */
    GT_U32      regData;            /* register data */
    GT_U32      tileId;             /* tile ID */
    GT_U32      portGroupId;        /* port group ID */
    GT_U32      hwData;             /* hardare data */

    /* Check that access done by busy-wait polling before trigger next access. */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEnginesStatus;
    rc = prvCpssPortGroupBusyWait(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr,
            PRV_CPSS_DXCH_AAC_CHANNEL_FDB_MANAGER_PRIMARY_E, GT_FALSE/*busyWait*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* AAC Engine <<%n>> Data for channel 2 */
    regAddr  = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineData[PRV_CPSS_DXCH_AAC_CHANNEL_FDB_MANAGER_PRIMARY_E];
    /* Read access */
    regData = hwIndex << 2 | 1;

    /* Write MAC command, HW index and trigger bit */
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* AAC Engine <<%n>> Address for channel 2 */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineAddress[PRV_CPSS_DXCH_AAC_CHANNEL_FDB_MANAGER_PRIMARY_E];
    regData = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBIndirectAccess.FDBIndirectAccessCtrl;

    /* Write address of FDB indirect access control register */
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Start reading FDB indirect access data registers */
    regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBIndirectAccess.FDBIndirectAccessData[0];

    /* loop on all the tiles in the device */
    for(tileId = 0; tileId < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles; tileId++)
    {
        portGroupId = tileId * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &hwData);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* Age bit is set */
        if(U32_GET_FIELD_MAC(hwData, 2, 1))
        {
            /* Read current FDB entry */
            rc = prvCpssDrvHwPpPortGroupReadRam(devNum, portGroupId, regAddr, 4, hwDataPtr);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* Reset age bit */
            U32_SET_FIELD_MAC(hwDataPtr[0], 2, 1, 0);

            /* Entry is not aged-out */
            *agedOutPtr = GT_FALSE;

            return GT_OK;
        }
    }
    
    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerAacHwWriteEntry function
* @endinternal
*
* @brief   Write a whole FDB entry to the HW using AAC method.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] hwIndex                  - index in the table
* @param[in] wordsArr                 - (pointer to) the 4 words data.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_HW_ERROR              - on hardware error
*
*/
GT_STATUS prvCpssDxChFdbManagerAacHwWriteEntry
(
    IN GT_U8                    devNum,
    IN GT_U32                   hwIndex,
    IN GT_U32                   wordsArr[]
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      numOfWords, currWord;
    GT_U32      regData;

    /* Check that previous access done by busy-wait polling before trigger next access. */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEnginesStatus;
    rc = prvCpssPortGroupBusyWait(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr,
            PRV_CPSS_DXCH_AAC_CHANNEL_FDB_MANAGER_PRIMARY_E, GT_FALSE/*busyWait*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    numOfWords = (wordsArr[0] == 0) ? 1 : 4;   /* write MAC data to 4 words for add/update or just 1 for delete */

    /* AAC Engine <<%n>> Data for channel 1 */
    regAddr  = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineData[PRV_CPSS_DXCH_AAC_CHANNEL_FDB_MANAGER_PRIMARY_E];

    for (currWord = 0; currWord < numOfWords; currWord++)
    {
        regData = wordsArr[currWord];
        /* Write words of MAC data */
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* AAC Engine <<%n>> Address for channel 1 */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineAddress[PRV_CPSS_DXCH_AAC_CHANNEL_FDB_MANAGER_PRIMARY_E];
    regData = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBIndirectAccess.FDBIndirectAccessData[0];

    /* Write address of FDB indirect access data register */
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* AAC Engine <<%n>> Data for channel 2 */
    regAddr  = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineData[PRV_CPSS_DXCH_AAC_CHANNEL_FDB_MANAGER_SECONDARY_E];
    regData = hwIndex << 2 | 3;

    /* Write MAC command, HW index and trigger bit */
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* AAC Engine <<%n>> Address for channel 2 */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineAddress[PRV_CPSS_DXCH_AAC_CHANNEL_FDB_MANAGER_SECONDARY_E];
    regData = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBIndirectAccess.FDBIndirectAccessCtrl;

    /* Write address of FDB indirect access control register */
    return prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
}

/**
* @enum UPDATE_HW_BANK_COUNTER_ACTION_ENT
 *
 * @brief types of actions that need to do on the HW bank counter.
 *
*/
typedef enum{
    /** no need to update the counter of the bank */
    UPDATE_HW_BANK_COUNTER_ACTION_NONE_E,
    /** need to increment the counter of the bank */
    UPDATE_HW_BANK_COUNTER_ACTION_INC_E,
    /** need to decrement the counter of the bank */
    UPDATE_HW_BANK_COUNTER_ACTION_DEC_E
}UPDATE_HW_BANK_COUNTER_ACTION_ENT;

/**
* @internal internal_writeFdbTableEntry function
* @endinternal
*
* @brief   Write a whole FDB entry to the HW in all port groups.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] hwIndex                  - index in the table
* @param[in] wordsArr                 - (pointer to) the 4 words data.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of wordsArr == NULL and indirect table the function
*       just send write entry command to device.
*       And entry is taken from the data registers of a indirect table.
*
*/
static GT_STATUS internal_writeFdbTableEntry
(
    IN GT_U8                    devNum,
    IN GT_U32                   hwIndex,
    IN GT_U32                   wordsArr[],
    IN GT_U32                   bankId, /* to inc/dec/none the 'HW counter' */
    IN UPDATE_HW_BANK_COUNTER_ACTION_ENT updateHwBankCounter
)
{
    GT_STATUS rc;
    GT_U32    portGroupId;
    CPSS_DXCH_TABLE_ENT updatedTableType;

#ifndef PRV_CPSS_DXCH_BRG_FDB_MANAGER_ALLOW_HW_BANK_COUNTER_UPDATE_CNS
    GT_UNUSED_PARAM(bankId);
#endif

    LOCK_DEV_NUM(devNum);

    /* convert the HW index if needed */
    prvCpssDxChIndexAsPortNumConvert(devNum, CPSS_DXCH_TABLE_FDB_E, hwIndex,
                                     &portGroupId , &hwIndex, &updatedTableType);

    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles > 1)
    {
        rc = prvCpssDxChFdbManagerAacHwWriteEntry(devNum, hwIndex, wordsArr);
    }
    else
    {
        rc = prvCpssDxChFdbManagerHwWriteEntry(devNum, hwIndex, wordsArr);
    }

    UNLOCK_DEV_NUM(devNum);

    if(rc != GT_OK)
    {
        return rc;
    }

    if(updateHwBankCounter == UPDATE_HW_BANK_COUNTER_ACTION_NONE_E)
    {
        /* no action on the counter needed */
        return GT_OK;
    }

#ifdef PRV_CPSS_DXCH_BRG_FDB_MANAGER_ALLOW_HW_BANK_COUNTER_UPDATE_CNS
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        return cpssDxChBrgFdbBankCounterUpdate(devNum,bankId,
                updateHwBankCounter == UPDATE_HW_BANK_COUNTER_ACTION_INC_E ?
                GT_TRUE :/* increment the counter */
                GT_FALSE /* decrement the counter */);
    }
    else
    {
        return GT_OK;
    }
#endif
    return GT_OK;
}

/*
* @internal prvCpssDxChFdbManagerHwFlushByHwIndex function
* @endinternal
*
* @brief   This function Flush HW of entry hwIndex from all registered devices in the manager.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] hwIndex                - HW index.
* @param[in] bankId                 - the bankId in case we need to decrement the HW counter.
* @param[in] updateHwBankCounter    - indication to decrement the HW counter.
*
* @retval GT_OK                    - on success
* @retval other                    - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChFdbManagerHwFlushByHwIndex
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN GT_U32                                           hwIndex,
    IN GT_U32                                           bankId,
    IN GT_BOOL                                          updateHwBankCounter
)
{
    GT_STATUS   rc;
    GT_BOOL     isFirst = GT_TRUE;
    GT_U8       devNum;

    rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr,isFirst,&devNum);
    isFirst = GT_FALSE;
    while(rc == GT_OK)
    {
        /* per device : flush entry from the HW */
        rc = internal_writeFdbTableEntry(devNum,hwIndex,
            fdbManagerPtr->hwEntryForFlush,
            bankId,
            (updateHwBankCounter == GT_TRUE) ?
            UPDATE_HW_BANK_COUNTER_ACTION_DEC_E :/* decrement the counter */
            UPDATE_HW_BANK_COUNTER_ACTION_NONE_E /* not-updating the counter */);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr,isFirst,&devNum);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerDeviceHwFormatGet function
* @endinternal
*
* @brief   This function gets the FDB entry HW format type.
*
* @param[in] fdbManagerPtr   - (pointer to) the FDB Manager.
* @param[in] devNum          - device number
* @param[out] hwFormatType   - (pointer to) Type of device
*
* @retval GT_OK              - on success
* @retval GT_BAD_PARAM       - on wrong parameters
*
*/
GT_STATUS prvCpssDxChFdbManagerDeviceHwFormatGet
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN  GT_U8                                             devNum,
    OUT PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT *hwFormatType
)
{
    switch(fdbManagerPtr->entryAttrInfo.shadowType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E:
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E:
            LOCK_DEV_NUM(devNum);
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                *hwFormatType = PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP6_E;
            }
            else if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                *hwFormatType = PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_20_E;
            }
            else
            {
                *hwFormatType = PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_E;
            }
            UNLOCK_DEV_NUM(devNum);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E:
            *hwFormatType = PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP6_E;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E:
            *hwFormatType = PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP4_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(fdbManagerPtr->entryAttrInfo.shadowType);
    }
    return GT_OK;
}

/**
* @internal buildHwEntry_prvFdbEntryMacAddrFormat_mux_fields function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords
*         for the fdbEntryMacAddrFormat : the 'mux' part
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildHwEntry_prvFdbEntryMacAddrFormat_mux_fields(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC  *dbEntryPtr
)
{
    GT_U32      hwValue,udbHw;
    GT_U32      *hwDataArr;

    hwDataArr =  fdbManagerPtr->tempInfo.sip6_hwFormatWords;

    /* the field is NOT muxed */
    hwValue = dbEntryPtr->saAccessLevel;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_SA_ACCESS_LEVEL_E,
        hwValue);

    switch (fdbManagerPtr->entryAttrInfo.macEntryMuxingMode)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E:
            hwValue = U32_GET_FIELD_MAC(dbEntryPtr->vid1,0,1);
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_0_E,
                hwValue);
            hwValue = U32_GET_FIELD_MAC(dbEntryPtr->vid1,1,6);
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_6_1_E,
                hwValue);
            hwValue = U32_GET_FIELD_MAC(dbEntryPtr->vid1,7,5);
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_11_7_E,
                hwValue);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E:
            hwValue = U32_GET_FIELD_MAC(dbEntryPtr->srcId,0,1);
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_0_E,
                hwValue);
            hwValue = U32_GET_FIELD_MAC(dbEntryPtr->srcId,1,2);
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1_E,
                hwValue);
            hwValue = U32_GET_FIELD_MAC(dbEntryPtr->srcId,3,4);
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_6_3_E,
                hwValue);
            hwValue = U32_GET_FIELD_MAC(dbEntryPtr->srcId,7,5);
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_11_7_E,
                hwValue);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_UDB_10_SRC_ID_1_DA_ACCESS_LEVEL_E:
        case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_UDB_8_SRC_ID_3_DA_ACCESS_LEVEL_E:

            /* the CPSS hide bit 0 for internal use */
            udbHw = dbEntryPtr->userDefined << 1;

            if(fdbManagerPtr->entryAttrInfo.macEntryMuxingMode ==
                CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_UDB_10_SRC_ID_1_DA_ACCESS_LEVEL_E)
            {
                hwValue = U32_GET_FIELD_MAC(udbHw,5,2);
                SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_6_5_E,
                    hwValue);

                hwValue = U32_GET_FIELD_MAC(udbHw,7,4);
                SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_10_7_E,
                    hwValue);
            }
            else
            {
                hwValue = U32_GET_FIELD_MAC(dbEntryPtr->srcId,1,2);
                SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1_E,
                    hwValue);

                hwValue = U32_GET_FIELD_MAC(udbHw,5,4);         /* UNLIKLE the CPSS : the value is continues. (no 2 bits hole) */
                SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_10_7_E,
                    hwValue);
            }

            hwValue = U32_GET_FIELD_MAC(udbHw,1,4);
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_4_1_E,
                hwValue);

            hwValue = U32_GET_FIELD_MAC(dbEntryPtr->srcId,0,1);
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_0_E,
                hwValue);

            hwValue = U32_GET_FIELD_MAC(dbEntryPtr->daAccessLevel,0,1);
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL_E,
                hwValue);
            break;
        default:
            break;
    }

    return GT_OK;
}

/**
* @internal buildHwEntry_prvFdbEntryIpMcFormat_mux_fields function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords
*         for the fdbEntryIpv4McFormat and fdbEntryIpv6McFormat : the 'mux' part
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildHwEntry_prvFdbEntryIpMcFormat_mux_fields
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC           *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC *dbEntryPtr
)
{
    GT_U32      hwValue,udbHw;
    GT_U32      *hwDataArr;

    hwDataArr =  fdbManagerPtr->tempInfo.sip6_hwFormatWords;

    hwValue = U32_GET_FIELD_MAC(dbEntryPtr->srcId,0,1);
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_0_E,
        hwValue);

    switch (fdbManagerPtr->entryAttrInfo.ipmcEntryMuxingMode)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E:
            hwValue = U32_GET_FIELD_MAC(dbEntryPtr->srcId,1,2);
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1_E,
                hwValue);
            hwValue = U32_GET_FIELD_MAC(dbEntryPtr->srcId,3,4);
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_6_3_E,
                hwValue);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_4_SRC_ID_3_E:
            hwValue = U32_GET_FIELD_MAC(dbEntryPtr->srcId,1,2);
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1_E,
                hwValue);
            /* the CPSS hides bit 0 for internal use */
            udbHw = dbEntryPtr->userDefined << 1;

            hwValue = U32_GET_FIELD_MAC(udbHw,1,4);
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_4_1_E,
                hwValue);

            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_E:

            /* the CPSS hides bit 0 for internal use */
            udbHw = dbEntryPtr->userDefined << 1;

            hwValue = U32_GET_FIELD_MAC(udbHw,1,4);
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_4_1_E,
                hwValue);

            hwValue = U32_GET_FIELD_MAC(udbHw,5,2);
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_6_5_E,
                hwValue);

            break;
        default:
            break;
    }

    hwValue = U32_GET_FIELD_MAC(dbEntryPtr->daAccessLevel,0,1);
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL_E,
            hwValue);

    return GT_OK;
}

/**
* @internal buildHwEntry_dstInterface_macOrIpmc function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords
*         for the mac Or Ipmc : the 'dstInterface' part
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager
* @param[in] isMacEntry             - MAC or IP MC FDB entry type:
*                                           GT_TRUE  - MAC entry
*                                           GT_FALSE - IP MC entry
* @param[in] dstInterface_type      - the type of dstInterface
* @param[in] dstInterfacePtr        - (pointer to) the manager dstInterface format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   buildHwEntry_dstInterface_macOrIpmc
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC      *fdbManagerPtr,
    IN GT_BOOL                                             isMacEntry,
    IN CPSS_INTERFACE_TYPE_ENT                             dstInterface_type,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INTERFACE_INFO_UNT     *dstInterfacePtr
)
{
    GT_U32      hwValue;
    GT_U32      *hwDataArr;
    GT_U32      devTmp,multiple,isTrunk,portTrunk,vidx;

    devTmp      = 0;
    multiple    = 0;
    isTrunk     = 0;
    portTrunk   = 0;
    vidx        = 0;

    switch(dstInterface_type)
    {
        case CPSS_INTERFACE_PORT_E:
            portTrunk = dstInterfacePtr->devPort.portNum;
            devTmp = dstInterfacePtr->devPort.hwDevNum;
            break;
        case CPSS_INTERFACE_TRUNK_E:
            isTrunk = 1;
            portTrunk = dstInterfacePtr->trunkId;
            break;
        case CPSS_INTERFACE_VIDX_E:
            multiple = 1;
            vidx = dstInterfacePtr->vidx;
            break;
        case CPSS_INTERFACE_VID_E:
            multiple = 1;
            vidx = 0xFFF;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dstInterface_type);
    }

    hwDataArr =  fdbManagerPtr->tempInfo.sip6_hwFormatWords;

    if (isMacEntry)
    {
        /* multiple */
        hwValue = multiple;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_MULTIPLE_E,
                hwValue);

        if (multiple == 0)
        {/* bit40MacAddr=0 and entry is PORT or TRUNK */

            /* devNum ID */
            hwValue = devTmp;
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_DEV_ID_E,
                hwValue);

            /* is Trunk bit */
            hwValue = isTrunk;
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_IS_TRUNK_E,
                hwValue);

            /* PortNum/TrunkNum */
            if(isTrunk)
            {
                hwValue = portTrunk;
                SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_TRUNK_NUM_E,
                    hwValue);
            }
            else
            {
                hwValue = portTrunk;
                SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_EPORT_NUM_E,
                    hwValue);
            }
        }
    }

    /* MACEntryType = "IPv4" or MACEntryType = "IPv6" or
       (MACEntryType = "MAC" and (MACAddrBit40 = "1" or Multiple = "Multicast")) */
    if (multiple == 1)
    {
        /* VIDX */
        hwValue = vidx;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_VIDX_E,
            hwValue);
    }

    return GT_OK;
}

/**
* @internal buildHwEntry_dstInterface_macOrIpmc_sip4 function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords
*         for the mac Or Ipmc : the 'dstInterface' part
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager
* @param[in] isMacEntry             - MAC or IP MC FDB entry type:
*                                           GT_TRUE  - MAC entry
*                                           GT_FALSE - IP MC entry
* @param[in] dstInterface_type      - the type of dstInterface
* @param[in] dstInterfacePtr        - (pointer to) the manager dstInterface format (manager format)
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   buildHwEntry_dstInterface_macOrIpmc_sip4
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC      *fdbManagerPtr,
    IN GT_BOOL                                             isMacEntry,
    IN CPSS_INTERFACE_TYPE_ENT                             dstInterface_type,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INTERFACE_INFO_UNT     *dstInterfacePtr,
    IN GT_U32                                              userDefined
)
{
    GT_U32      *hwDataArr;
    GT_U32      devTmp, multiple, isTrunk, portTrunk, vidx;

    hwDataArr   = fdbManagerPtr->tempInfo.sip6_hwFormatWords;
    devTmp      = 0;
    multiple    = 0;
    isTrunk     = 0;
    portTrunk   = 0;
    vidx        = 0;

    /* check interface destination type */
    switch(dstInterface_type)
    {
        case CPSS_INTERFACE_PORT_E:
            PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(
                    dstInterfacePtr->devPort.hwDevNum,
                    dstInterfacePtr->devPort.portNum);

            portTrunk = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(
                    dstInterfacePtr->devPort.hwDevNum,
                    dstInterfacePtr->devPort.portNum);

            devTmp = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(
                    dstInterfacePtr->devPort.hwDevNum,
                    dstInterfacePtr->devPort.portNum);
            /* Validation not required while using DB to HW */
            break;

        case CPSS_INTERFACE_TRUNK_E:
            portTrunk = dstInterfacePtr->trunkId;
            if (portTrunk == 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "trunkId must not be ZERO (0)");
            }
            isTrunk  = 1;
            devTmp   = dstInterfacePtr->devPort.hwDevNum;
            /* Validation not required while using DB to HW */
            break;

        case CPSS_INTERFACE_VIDX_E:
        case CPSS_INTERFACE_VID_E:
            devTmp   = dstInterfacePtr->devPort.hwDevNum;
            vidx     = (dstInterface_type == CPSS_INTERFACE_VID_E)?0xFFF:dstInterfacePtr->vidx;
            multiple = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dstInterface_type);
    }

    /* check if bit40MacAddr==0 or entry is VIDX or VID */
    if(multiple == 1)
    {
        /* VIDX - 12 bits*/
        hwDataArr[2] |= ((vidx & 0xFFF) << 13);
    }
    if(isMacEntry)
    {/* bit40MacAddr=0 and entry is PORT or TRUNK */

        if(multiple == 0)
        {
            /* devNum ID 5 bits */
            hwDataArr[2] |= ((devTmp & 0x1F) << 1);

            /* is Trunk bit */
            hwDataArr[2] |= (isTrunk << 13);

            /* PortNum/TrunkNum */
            hwDataArr[2] |= ((portTrunk & 0x7f) << 14);

            /* UserDefined 4 bits */
            hwDataArr[2] |= ((userDefined & 0xF) << 21);
        }
        /* multiple */
        hwDataArr[2] |= (multiple << 26);
    }
    return GT_OK;
}

/**
* @internal buildHwEntry_prvFdbEntryIpMcAddrFormat_sip4 function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords
*         for the fdbEntryIpv4McFormat and fdbEntryIpv6McFormat
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
* @param[in] fdbEntryType           - FDB entry type
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildHwEntry_prvFdbEntryIpMcAddrFormat_sip4
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC           *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC *dbEntryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_ENT          fdbEntryType
)
{
    GT_STATUS   rc;
    GT_U32      hwValue;
    GT_U32      *hwDataArr;

    hwDataArr =  fdbManagerPtr->tempInfo.sip6_hwFormatWords;

    /* valid bit */
    hwDataArr[0] = 1;

    /* skip */
    hwDataArr[0] |= (0 << 1);

    /* age */
    hwDataArr[0] |= (dbEntryPtr->age << 2);

    /* VID */
    hwDataArr[0] |= ((dbEntryPtr->fid & 0xFFF) << 5);

    /* static */
    hwDataArr[2] |= (dbEntryPtr->isStatic << 25);

    /* SaQosProfileIndex */
    hwDataArr[3] |= (dbEntryPtr->saQosIndex << 3);

    /* DaQosProfileIndex */
    hwDataArr[3] |= (dbEntryPtr->daQosIndex << 6);

    /* MirrorToAnalyzerPort*/
    hwDataArr[3] |= (dbEntryPtr->saLookupIngressMirrorToAnalyzerPort << 9);

    /* AppSpecific CPU Code */
    hwDataArr[3] |= (dbEntryPtr->appSpecificCpuCode << 10);

    /* DA Access Level */
    hwDataArr[3] |= (dbEntryPtr->daAccessLevel << 11);

    /* spUnknown - NA storm prevent entry */
    hwDataArr[3] |= (0 << 2);

    /* DA Route */
    hwDataArr[3] |= (dbEntryPtr->daRoute << 1);

    /* DA cmd */
    hwDataArr[2] |= (dbEntryPtr->daCommand << 27);

    /* entry type */
    hwValue = fdbEntryType;
    hwDataArr[0] |= (hwValue << 3);

    /* DIP[14..0] */
    /* Word0, bits 17-31 */
    hwDataArr[0] |= ((dbEntryPtr->dipAddr & 0x7FFF) << 17);

    /* DIP[31..15] */
    /* Word1, bits 0-16 */
    hwDataArr[1] |= (dbEntryPtr->dipAddr >> 15);

    /* SIP[14..0] */
    /* Word1, bits 17-31 */
    hwDataArr[1] |= ((dbEntryPtr->sipAddr & 0x7FFF) << 17);

    /* SIP[27..15] */
    /* Word2, bits 0-12 */
    hwDataArr[2] |= (((dbEntryPtr->sipAddr >> 15) & 0xFFF));

    /* SIP[28] */
    /* Word2, bit 26 */
    hwDataArr[2] |= (((dbEntryPtr->sipAddr >> 28) & 0x1) << 26);

    /* SIP[30..29] */
    /* Word2, bits 30-31 */
    hwDataArr[2] |= (((dbEntryPtr->sipAddr >> 29) & 0x7) << 30);

    /* SIP[31] */
    /* Word0, bit 31 */
    hwDataArr[3] |= ((dbEntryPtr->sipAddr << 31) & 0x1);

    rc = buildHwEntry_dstInterface_macOrIpmc_sip4(fdbManagerPtr, GT_FALSE /* MAC entry */,
                                             dbEntryPtr->dstInterface_type,
                                             &dbEntryPtr->dstInterface, 0/* user Defined */);
    return rc;
}

/**
* @internal buildHwEntry_prvFdbEntryMacAddrFormat_sip4 function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords
*         for the fdbEntryMacAddrFormat
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildHwEntry_prvFdbEntryMacAddrFormat_sip4
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC  *dbEntryPtr
)
{
    GT_STATUS   rc;
    GT_U32      hwValue;
    GT_U32      *hwDataArr;

    hwDataArr =  fdbManagerPtr->tempInfo.sip6_hwFormatWords;

    /* valid bit */
    hwDataArr[0] = 1;

    /* skip */
    hwDataArr[0] |= (0 << 1);

    /* age */
    hwDataArr[0] |= (dbEntryPtr->age << 2);

    /* VID */
    hwDataArr[0] |= ((dbEntryPtr->fid & 0xFFF) << 5);

    /* static */
    hwDataArr[2] |= (dbEntryPtr->isStatic << 25);

    /* SaQosProfileIndex */
    hwDataArr[3] |= (dbEntryPtr->saQosIndex << 3);

    /* DaQosProfileIndex */
    hwDataArr[3] |= (dbEntryPtr->daQosIndex << 6);

    /* MirrorToAnalyzerPort*/
    hwDataArr[3] |= (dbEntryPtr->saLookupIngressMirrorToAnalyzerPort << 9);

    /* AppSpecific CPU Code */
    hwDataArr[3] |= (dbEntryPtr->appSpecificCpuCode << 10);

    /* DA Access Level */
    hwDataArr[3] |= (dbEntryPtr->daAccessLevel << 11);

    /* SA Access Level */
    hwDataArr[3] |= (dbEntryPtr->saAccessLevel << 14);

    /* spUnknown - NA storm prevent entry */
    hwDataArr[3] |= (0 << 2);

    /* DA Route */
    hwDataArr[3] |= (dbEntryPtr->daRoute << 1);

    /* DA cmd */
    hwDataArr[2] |= (dbEntryPtr->daCommand << 27);

    /* entry type */
    hwValue = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E;
    hwDataArr[0] |= (hwValue << 3);

    /* MacAddr[14..0] */
    hwValue = dbEntryPtr->macAddr_low_32 & 0x7FFF;
    hwDataArr[0] |= (hwValue << 17); /* Word0, bits 17-31 */

    /* MacAddr[15..46] */
    hwDataArr[1] = (dbEntryPtr->macAddr_low_32 >> 15) |
                    ((dbEntryPtr->macAddr_high_16 & 0x7FFF) << 17);

    /* MacAddr[47] */
    hwDataArr[2] |= ((dbEntryPtr->macAddr_high_16 >> 15) & 0x1); /* the highest bit for Mac Address */

    /* Src ID */
    hwDataArr[2] |= ((dbEntryPtr->srcId & 0x1f) << 6);

    /* SA cmd[1..0] */
    hwValue = dbEntryPtr->saCommand;
    hwDataArr[2] |= ((hwValue & 0x3) << 30);

    /* SA cmd[2] */
    hwDataArr[3] |= ((hwValue >> 2) & 0x1);

    rc = buildHwEntry_dstInterface_macOrIpmc_sip4(fdbManagerPtr,GT_TRUE /* MAC entry */,
                                             dbEntryPtr->dstInterface_type,
                                             &dbEntryPtr->dstInterface,
                                             dbEntryPtr->userDefined);
    return rc;
}

/**
* @internal prvCpssDxChFdbManagerHwBuildHwFormatFromDbEntry_sip4 function
* @endinternal
*
* @brief  function to build the HW format for SIP4 device at : fdbManagerPtr->tempInfo.sip6_hwFormatWords
*         for the fdbEntryMacAddrFormat
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS prvCpssDxChFdbManagerHwBuildHwFormatFromDbEntry_sip4
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC           *dbEntryPtr
)
{
    cpssOsMemSet(fdbManagerPtr->tempInfo.sip6_hwFormatWords, 0, sizeof(fdbManagerPtr->tempInfo.sip6_hwFormatWords));
    switch(dbEntryPtr->hwFdbEntryType)
    {
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E:
            return buildHwEntry_prvFdbEntryMacAddrFormat_sip4(fdbManagerPtr,
                    &dbEntryPtr->specificFormat.prvMacEntryFormat);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            return buildHwEntry_prvFdbEntryIpMcAddrFormat_sip4(fdbManagerPtr,
                    &dbEntryPtr->specificFormat.prvIpv4McEntryFormat,
                    PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            return buildHwEntry_prvFdbEntryIpMcAddrFormat_sip4(fdbManagerPtr,
                    &dbEntryPtr->specificFormat.prvIpv6McEntryFormat,
                    PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E);
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryPtr->hwFdbEntryType);
    }
}

/**
* @internal buildHwEntry_dstInterface_IpvxUc_sip5 function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip5_hwFormatWords
*         for the IPvx UC routing : the 'dstInterface' part
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] dstInterface_type      - the type of dstInterface
* @param[in] isSip5_20              - Indicates the device is SIP5 or SIP5_20.
* @param[in] dstInterfacePtr        - (pointer to) the manager dstInterface format (manager format)
* @param[in] hwDataArr              - (pointer to) hw data.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman; AC5;.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   buildHwEntry_dstInterface_IpvxUc_sip5
(
    IN CPSS_INTERFACE_TYPE_ENT                             dstInterface_type,
    IN GT_BOOL                                             isSip5_20,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INTERFACE_INFO_UNT     *dstInterfacePtr,
    OUT GT_U32                                             *hwDataArr
)
{
    GT_U32      devTmp,multiple,isTrunk,portTrunk,vidx;
    GT_U32      hwValue;

    devTmp      = 0;
    multiple    = 0;
    isTrunk     = 0;
    portTrunk   = 0;
    vidx        = 0;

    switch(dstInterface_type)
    {
        case CPSS_INTERFACE_PORT_E:
            portTrunk = dstInterfacePtr->devPort.portNum;
            devTmp = dstInterfacePtr->devPort.hwDevNum;
            break;
        case CPSS_INTERFACE_TRUNK_E:
            isTrunk = 1;
            portTrunk = dstInterfacePtr->trunkId;
            break;
        case CPSS_INTERFACE_VIDX_E:
            multiple = 1;
            vidx = dstInterfacePtr->vidx;
            break;
        case CPSS_INTERFACE_VID_E:
            multiple = 1;
            vidx = 0xFFF;
            /* no param needed !!! */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dstInterface_type);
    }


    /* multiple */
    hwValue = multiple;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_USE_VIDX_E,
        hwValue);

    if (multiple == 0)
    {
        /* is Trunk bit */
        hwValue = isTrunk;
        SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_IS_TRUNK_E,
            hwValue);

        /* PortNum/TrunkNum */
        if(isTrunk)
        {
            hwValue = portTrunk;
            SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_TRUNK_NUM_E,
                hwValue);
        }
        else
        {
            hwValue = portTrunk;
            if(isSip5_20)
            {
                SIP5_20_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                        SIP5_FDB_FDB_TABLE_FIELDS_EPORT_NUM_E,
                        hwValue);
                /* devNum ID */
                SIP5_20_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                        SIP5_FDB_FDB_TABLE_FIELDS_TARGET_DEVICE_E,
                        devTmp);
            }
            else
            {
                SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                        SIP5_FDB_FDB_TABLE_FIELDS_EPORT_NUM_E,
                        hwValue);
                /* devNum ID */
                SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                        SIP5_FDB_FDB_TABLE_FIELDS_TARGET_DEVICE_E,
                        devTmp);
            }
        }
    }
    else
    {
        /* VIDX */
        hwValue = vidx;
        SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_VIDX_E,
            hwValue);
    }

    return GT_OK;
}

/**
* @internal buildHwEntry_prvIpVxUcEntryFormat_commonFields_sip5 function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip5_hwFormatWords
*         for the prvIpv4UcEntryFormat
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
*         based on buildFdbUcRouteEntryHwFormatSip6
*
* @param[in] ipUcCommonInfoPtr      - (pointer to) the manager entry ipvx common info (manager format)
* @param[in] isSip5_20              - Indicates the device is SIP5 or SIP5_20.
* @param[in] hwDataArr              - (pointer to) hwdata
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman; AC5;.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildHwEntry_prvIpVxUcEntryFormat_commonFields_sip5
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV_UC_COMMON_FORMAT_STC  *ipUcCommonInfoPtr,
    IN GT_BOOL                                                       isSip5_20,
    OUT GT_U32                                                      *hwDataArr
)
{
    GT_U32      hwValue;

    /* VRF-ID */
    hwValue = ipUcCommonInfoPtr->vrfId;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_VRF_ID_E,
            hwValue);

    /* ttl/hop decrement enable */
    hwValue = ipUcCommonInfoPtr->ttlHopLimitDecEnable;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DEC_TTL_OR_HOP_COUNT_E,
            hwValue);

    /* Bypass TTL Options Or Hop Extension */
    hwValue = ipUcCommonInfoPtr->ttlHopLimDecOptionsExtChkByPass;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E,
            hwValue);

    /* ingress mirror to analyzer index */
    hwValue = ipUcCommonInfoPtr->ingressMirrorToAnalyzerIndex;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_INGRESS_MIRROR_TO_ANALYZER_INDEX_E,
            hwValue);

    hwValue = ipUcCommonInfoPtr->qosProfileMarkingEn;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_QOS_PROFILE_MARKING_EN_E,
            hwValue);

    hwValue = ipUcCommonInfoPtr->qosProfileIndex;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_QOS_PROFILE_INDEX_E,
            hwValue);

    hwValue = ipUcCommonInfoPtr->qosProfilePrecedence;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_QOS_PROFILE_PRECEDENCE_E,
            hwValue);

    /* DB holds validated HW value, no need to validate */
    hwValue = ipUcCommonInfoPtr->modifyUp;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_MODIFY_UP_E,
            hwValue);

    /* DB holds validated HW value, no need to validate */
    hwValue = ipUcCommonInfoPtr->modifyDscp;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_MODIFY_DSCP_E,
            hwValue);

    /* DB holds validated HW value, no need to validate */
    hwValue = ipUcCommonInfoPtr->countSet;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_COUNTER_SET_INDEX_E,
            hwValue);

    hwValue = ipUcCommonInfoPtr->arpBcTrapMirrorEn;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_ARP_BC_TRAP_MIRROR_EN_E,
            hwValue);

    /* dip access level */
    hwValue = ipUcCommonInfoPtr->dipAccessLevel;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DIP_ACCESS_LEVEL_E,
            hwValue);

    hwValue = ipUcCommonInfoPtr->ICMPRedirectEnable;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_ICMP_REDIRECT_EXCEP_MIRROR_EN_E,
            hwValue);

    /* mtu profile index */
    hwValue = ipUcCommonInfoPtr->mtuProfileIndex;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_MTU_INDEX_E,
            hwValue);

    if(isSip5_20)
    {
        /* set vlan associated with this entry */
        hwValue = ipUcCommonInfoPtr->nextHopVlanId;
        SIP5_20_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_NEXT_HOP_EVLAN_E,
                hwValue);

        if (ipUcCommonInfoPtr->ucRouteExtType ==
                CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E)
        {
            /* set tunnel type */
            hwValue = 1; /* the only so far supported tunneltype is IPv4 */
            SIP5_20_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_TYPE_E,
                    hwValue);

            /* set tunnel ptr */
            hwValue = ipUcCommonInfoPtr->pointerInfo;
            SIP5_20_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_PTR_E,
                    hwValue);

            /* set that the entry is start of tunnel */
            hwValue = 1;
        }
        else
        {
            /* set that the entry is not a tunnel start */
            hwValue = 0;
        }
        SIP5_20_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_START_E,
                hwValue);

        /* arp ptr*/
        if (hwValue == 0)
        {
            hwValue = ipUcCommonInfoPtr->pointerInfo;
            SIP5_20_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_ARP_PTR_E,
                    hwValue);
        }
    }
    else
    {
        /* set vlan associated with this entry */
        hwValue = ipUcCommonInfoPtr->nextHopVlanId;
        SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_NEXT_HOP_EVLAN_E,
                hwValue);

        if (ipUcCommonInfoPtr->ucRouteExtType ==
                CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E)
        {
            /* set tunnel type */
            hwValue = 1; /* the only so far supported tunneltype is IPv4 */
            SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_TYPE_E,
                    hwValue);

            /* set tunnel ptr */
            hwValue = ipUcCommonInfoPtr->pointerInfo;
            SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_PTR_E,
                    hwValue);

            /* set that the entry is start of tunnel */
            hwValue = 1;
        }
        else
        {
            /* set that the entry is not a tunnel start */
            hwValue = 0;
        }
        SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_START_E,
                hwValue);

        /* arp ptr*/
        if (hwValue == 0)
        {
            hwValue = ipUcCommonInfoPtr->pointerInfo;
            SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_ARP_PTR_E,
                    hwValue);
        }

    }

    return buildHwEntry_dstInterface_IpvxUc_sip5(ipUcCommonInfoPtr->dstInterface_type,
            isSip5_20,
            &ipUcCommonInfoPtr->dstInterface,
            hwDataArr);
}

/**
* @internal buildHwEntry_prvIpv6UcEntryFormat_sip5 function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip5_hwFormatWords,
*                                              fdbManagerPtr->tempInfo.sip5_ipv6Ext_hwFormatWords
*         for the prvIpv6UcDataEntryFormat and prvIpv6UcKeyEntryFormat
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
*         based on buildFdbUcRouteEntryHwFormatSip6
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] isSip5_20              - Indicates the device is SIP5 or SIP5_20.
* @param[in] dbEntryDataPtr         - (pointer to) the manager entry format(data) (manager format)
* @param[in] dbEntryKeyPtr          - (pointer to) the manager entry format(key) (manager format)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman; AC5;.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildHwEntry_prvIpv6UcEntryFormat_sip5
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC               *fdbManagerPtr,
    IN GT_BOOL                                                       isSip5_20,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_DATA_FORMAT_STC   *dbEntryDataPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_KEY_FORMAT_STC    *dbEntryKeyPtr
)
{
    GT_STATUS   rc;
    GT_U32      hwValue;
    GT_U32      *hwDataArr;
    GT_U32      *hwKeyArr;

    if(isSip5_20)
    {
        cpssOsMemSet(fdbManagerPtr->tempInfo.sip5_20_hwFormatWords, 0, sizeof(fdbManagerPtr->tempInfo.sip5_20_hwFormatWords));
        cpssOsMemSet(fdbManagerPtr->tempInfo.sip5_20_ipv6Ext_hwFormatWords, 0, sizeof(fdbManagerPtr->tempInfo.sip5_20_ipv6Ext_hwFormatWords));
        hwKeyArr    = fdbManagerPtr->tempInfo.sip5_20_hwFormatWords;
        hwDataArr   = fdbManagerPtr->tempInfo.sip5_20_ipv6Ext_hwFormatWords;
    }
    else
    {
        cpssOsMemSet(fdbManagerPtr->tempInfo.sip5_hwFormatWords, 0, sizeof(fdbManagerPtr->tempInfo.sip5_hwFormatWords));
        cpssOsMemSet(fdbManagerPtr->tempInfo.sip5_ipv6Ext_hwFormatWords, 0, sizeof(fdbManagerPtr->tempInfo.sip5_ipv6Ext_hwFormatWords));
        hwKeyArr    = fdbManagerPtr->tempInfo.sip5_hwFormatWords;
        hwDataArr   = fdbManagerPtr->tempInfo.sip5_ipv6Ext_hwFormatWords;
    }

    /* valid */
    hwValue = 1;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_VALID_E,
        hwValue);

    /* skip */
    hwValue = 0;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_SKIP_E,
        hwValue);

    /* age */
    hwValue = dbEntryDataPtr->ipUcCommonInfo.age;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_AGE_E,
        hwValue);

    /* entry type */
    hwValue = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
        hwValue);

    /* VRF-ID */
    hwValue = dbEntryDataPtr->ipUcCommonInfo.vrfId;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_VRF_ID_E,
        hwValue);

    hwValue = dbEntryDataPtr->ipv6DestSiteId;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DST_SITE_ID_E,
        hwValue);

    hwValue = dbEntryDataPtr->ipv6ScopeCheck;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_IPV6_SCOPE_CHECK_E,
        hwValue);

    rc = buildHwEntry_prvIpVxUcEntryFormat_commonFields_sip5(
            &dbEntryDataPtr->ipUcCommonInfo,
            isSip5_20,
            hwDataArr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* start key data */
    /* valid */
    hwValue = 1;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwKeyArr,
        SIP5_FDB_FDB_TABLE_FIELDS_VALID_E,
        hwValue);

    /* skip */
    hwValue = 0;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwKeyArr,
        SIP5_FDB_FDB_TABLE_FIELDS_SKIP_E,
        hwValue);

    /* age */
    hwValue = dbEntryKeyPtr->age;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwKeyArr,
        SIP5_FDB_FDB_TABLE_FIELDS_AGE_E,
        hwValue);

    /* entry type */
    hwValue = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwKeyArr,
        SIP5_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
        hwValue);

    hwValue = dbEntryKeyPtr->ipAddr_31_0;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC( hwKeyArr,
            SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DIP_0_E,
            hwValue);
    hwValue = dbEntryKeyPtr->ipAddr_63_32;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC( hwKeyArr,
            SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DIP_1_E,
            hwValue);
    hwValue = dbEntryKeyPtr->ipAddr_95_64;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC( hwKeyArr,
            SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DIP_2_E,
            hwValue);
    hwValue = dbEntryKeyPtr->ipAddr_105_96;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC( hwKeyArr,
            SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DIP_3_E,
            hwValue);

    return GT_OK;
}

/**
* @internal buildHwEntry_prvIpv4UcEntryFormat_sip5 function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip5_hwFormatWords
*         for the prvIpv4UcEntryFormat
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
*         based on buildFdbUcRouteEntryHwFormatSip6
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] isSip5_20              - Indicates the device is SIP5 or SIP5_20.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman; AC5;.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildHwEntry_prvIpv4UcEntryFormat_sip5
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC         *fdbManagerPtr,
    IN GT_BOOL                                                 isSip5_20,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV4_UC_FORMAT_STC  *dbEntryPtr
)
{
    GT_U32      hwValue;
    GT_U32      *hwDataArr;

    if(isSip5_20)
    {
        cpssOsMemSet(fdbManagerPtr->tempInfo.sip5_20_hwFormatWords, 0, sizeof(fdbManagerPtr->tempInfo.sip5_20_hwFormatWords));
        hwDataArr = fdbManagerPtr->tempInfo.sip5_20_hwFormatWords;
    }
    else
    {
        cpssOsMemSet(fdbManagerPtr->tempInfo.sip5_hwFormatWords, 0, sizeof(fdbManagerPtr->tempInfo.sip5_hwFormatWords));
        hwDataArr = fdbManagerPtr->tempInfo.sip5_hwFormatWords;
    }

    /* valid */
    hwValue = 1;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_VALID_E,
        hwValue);

    /* skip */
    hwValue = 0;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_SKIP_E,
        hwValue);

    /* age */
    hwValue = dbEntryPtr->ipUcCommonInfo.age;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_AGE_E,
        hwValue);

    /* entry type */
    hwValue = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
        hwValue);

    hwValue = dbEntryPtr->ipAddr;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_IPV4_DIP_E,
        hwValue);

    buildHwEntry_prvIpVxUcEntryFormat_commonFields_sip5(
            &dbEntryPtr->ipUcCommonInfo,
            isSip5_20,
            hwDataArr);
    return GT_OK;
}

/**
* @internal buildHwEntry_dstInterface_macOrIpmc_sip5 function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip5_hwFormatWords
*         for the mac Or Ipmc : the 'dstInterface' part
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] isMacEntry             - MAC or IP MC FDB entry type:
*                                           GT_TRUE  - MAC entry
*                                           GT_FALSE - IP MC entry
* @param[in] hwDataArr              - HW data format
* @param[in] dstInterface_type      - the type of dstInterface
* @param[in] dstInterfacePtr        - (pointer to) the manager dstInterface format (manager format)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman; AC5;.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   buildHwEntry_dstInterface_macOrIpmc_sip5
(
    IN GT_BOOL                                             isMacEntry,
    IN GT_U32                                              *hwDataArr,
    IN CPSS_INTERFACE_TYPE_ENT                             dstInterface_type,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INTERFACE_INFO_UNT     *dstInterfacePtr
)
{
    GT_U32      hwValue;
    GT_U32      devTmp,multiple,isTrunk,portTrunk,vidx;

    devTmp      = 0;
    multiple    = 0;
    isTrunk     = 0;
    portTrunk   = 0;
    vidx        = 0;

    switch(dstInterface_type)
    {
        case CPSS_INTERFACE_PORT_E:
            portTrunk = dstInterfacePtr->devPort.portNum;
            devTmp = dstInterfacePtr->devPort.hwDevNum;
            break;
        case CPSS_INTERFACE_TRUNK_E:
            isTrunk = 1;
            portTrunk = dstInterfacePtr->trunkId;
            break;
        case CPSS_INTERFACE_VIDX_E:
            multiple = 1;
            vidx = dstInterfacePtr->vidx;
            break;
        case CPSS_INTERFACE_VID_E:
            multiple = 1;
            vidx = 0xFFF;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dstInterface_type);
    }

    if (isMacEntry)
    {
        /* multiple */
        hwValue = multiple;
        SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_MULTIPLE_E,
                hwValue);

        if (multiple == 0)
        {/* bit40MacAddr=0 and entry is PORT or TRUNK */

            /* devNum ID */
            hwValue = devTmp;
            SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_DEV_ID_E,
                hwValue);

            /* is Trunk bit */
            hwValue = isTrunk;
            SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_IS_TRUNK_E,
                hwValue);

            /* PortNum/TrunkNum */
            if(isTrunk)
            {
                hwValue = portTrunk;
                SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_TRUNK_NUM_E,
                    hwValue);
            }
            else
            {
                hwValue = portTrunk;
                SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_EPORT_NUM_E,
                    hwValue);
            }
        }
    }

    /* MACEntryType = "IPv4" or MACEntryType = "IPv6" or
       (MACEntryType = "MAC" and (MACAddrBit40 = "1" or Multiple = "Multicast")) */
    if (multiple == 1)
    {
        /* VIDX */
        hwValue = vidx;
        SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_VIDX_E,
            hwValue);
    }

    return GT_OK;
}

/**
* @internal buildHwEntry_prvFdbEntryIpMcAddrFormat_sip5 function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip5_hwFormatWords
*         for the fdbEntryIpv4McFormat and fdbEntryIpv6McFormat
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] isSip5_20              - Indicates the device is SIP5 or SIP5_20.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
* @param[in] fdbEntryType           - FDB entry type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman; AC5;.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildHwEntry_prvFdbEntryIpMcAddrFormat_sip5
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC           *fdbManagerPtr,
    IN GT_BOOL                                                   isSip5_20,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC *dbEntryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_ENT           fdbEntryType
)
{
    GT_U32      hwValue;
    GT_U32      *hwDataArr;
    GT_BOOL     tag1VidFdbEn = GT_FALSE;
    GT_U32      maxSourceId, maxUdb;

    if(isSip5_20)
    {
        cpssOsMemSet(fdbManagerPtr->tempInfo.sip5_20_hwFormatWords, 0, sizeof(fdbManagerPtr->tempInfo.sip5_20_hwFormatWords));
        hwDataArr = fdbManagerPtr->tempInfo.sip5_20_hwFormatWords;
    }
    else
    {
        cpssOsMemSet(fdbManagerPtr->tempInfo.sip5_hwFormatWords, 0, sizeof(fdbManagerPtr->tempInfo.sip5_hwFormatWords));
        hwDataArr = fdbManagerPtr->tempInfo.sip5_hwFormatWords;
    }

    /* valid */
    hwValue = 1;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_VALID_E,
        hwValue);

    /* skip */
    hwValue = 0;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_SKIP_E,
        hwValue);

    /* age */
    hwValue = BOOL2BIT_MAC(dbEntryPtr->age);
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_AGE_E,
        hwValue);

    /* entry type */
    hwValue = fdbEntryType;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
        hwValue);

    /* FID */
    hwValue = dbEntryPtr->fid;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_FID_E,
        hwValue);

    /*SIP*/
    hwValue = dbEntryPtr->sipAddr;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SIP_E,
            hwValue);

    /*DIP*/
    hwValue = dbEntryPtr->dipAddr;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DIP_E,
            hwValue);

    /* static */
    hwValue = BIT2BOOL_MAC(dbEntryPtr->isStatic);
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_IS_STATIC_E,
        hwValue);

    /* DA cmd */
    hwValue = dbEntryPtr->daCommand;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_DA_CMD_E,
        hwValue);

    /* DA Route */
    hwValue = BIT2BOOL_MAC(dbEntryPtr->daRoute);
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_DA_ROUTE_E,
        hwValue);

    /* spUnknown - NA storm prevent entry */
    hwValue = 0;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E,
        hwValue);

    /* SaQosProfileIndex */
    hwValue = dbEntryPtr->saQosIndex;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SA_QOS_PARAM_SET_IDX_E,
            hwValue);

    /* DaQosProfileIndex */
    hwValue = dbEntryPtr->daQosIndex;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DA_QOS_PARAM_SET_IDX_E,
            hwValue);

    /* SA Lookup Ingress Mirror to Analyzer Enable */
    hwValue = dbEntryPtr->saLookupIngressMirrorToAnalyzerPort;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E,
            hwValue);

    /* DA Lookup Ingress Mirror to Analyzer Enable */
    hwValue = dbEntryPtr->daLookupIngressMirrorToAnalyzerPort;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E,
            hwValue);

    /* AppSpecific CPU Code */
    hwValue = dbEntryPtr->appSpecificCpuCode;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E,
        hwValue);

    switch (fdbManagerPtr->entryAttrInfo.entryMuxingMode)
    {
        default:
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E:
            maxSourceId = (1 << 9);
            maxUdb      = (1 << 8);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E:
            maxSourceId = (1 << 12);
            maxUdb      = (1 << 5);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E:
            maxSourceId = (1 << 6);
            maxUdb      = (1 << 8);
            tag1VidFdbEn= GT_TRUE;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E:
            maxSourceId = (1 << 6);
            maxUdb      = (1 << 5);
            tag1VidFdbEn= GT_TRUE;
            break;
    }

    if((dbEntryPtr->srcId >= maxSourceId) || (dbEntryPtr->userDefined >= maxUdb))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Src D */
    hwValue = U32_GET_FIELD_MAC(dbEntryPtr->srcId,0,6);
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_5_0_E,
            hwValue);

    if(maxSourceId > 0x1FF)
    {
        hwValue = U32_GET_FIELD_MAC(dbEntryPtr->srcId,6,3);
        SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_8_6_E,
                hwValue);
    }

    /* NOTE: setting the 8 bits of UDB must come before setting of SOURCE_ID_11_9
       to allow SOURCE_ID_11_9 to override the 3 bits ! */
    hwValue = dbEntryPtr->userDefined;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_USER_DEFINED_E,
            hwValue);
    if(maxSourceId > 0xFFF)
    {
        hwValue = U32_GET_FIELD_MAC(dbEntryPtr->srcId,9,3);
        SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_11_9_E,
                hwValue);
    }

    if (tag1VidFdbEn != GT_FALSE)
    {
        hwValue = dbEntryPtr->vid1;
        SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_ORIG_VID1_E,
                hwValue);
    }
    else
    {
        hwValue = dbEntryPtr->daAccessLevel;
        SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL_E,
                hwValue);
    }

    return buildHwEntry_dstInterface_macOrIpmc_sip5(GT_FALSE /* IP MC entry */,
            hwDataArr,
            dbEntryPtr->dstInterface_type,
            &dbEntryPtr->dstInterface);
}


/**
* @internal buildHwEntry_prvFdbEntryMacAddrFormat_sip5 function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip5_hwFormatWords
*         for the fdbEntryMacAddrFormat
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] isSip5_20              - Indicates the device is SIP5 or SIP5_20.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman; AC5;.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildHwEntry_prvFdbEntryMacAddrFormat_sip5
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN GT_BOOL                                                  isSip5_20,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC  *dbEntryPtr
)
{
    GT_STATUS   rc;
    GT_U32      hwValue;
    GT_U32      *hwDataArr;
    GT_BOOL     tag1VidFdbEn = GT_FALSE;
    GT_U32      maxSourceId, maxUdb;

    if(isSip5_20)
    {
        cpssOsMemSet(fdbManagerPtr->tempInfo.sip5_20_hwFormatWords, 0, sizeof(fdbManagerPtr->tempInfo.sip5_20_hwFormatWords));
        hwDataArr = fdbManagerPtr->tempInfo.sip5_20_hwFormatWords;
    }
    else
    {
        cpssOsMemSet(fdbManagerPtr->tempInfo.sip5_hwFormatWords, 0, sizeof(fdbManagerPtr->tempInfo.sip5_hwFormatWords));
        hwDataArr = fdbManagerPtr->tempInfo.sip5_hwFormatWords;
    }

    /* valid */
    hwValue = 1;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_VALID_E,
            hwValue);

    /* skip */
    hwValue = 0;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SKIP_E,
            hwValue);

    /* age */
    hwValue = BOOL2BIT_MAC(dbEntryPtr->age);
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_AGE_E,
            hwValue);

    /* entry type */
    hwValue = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
            hwValue);

    /* FID */
    hwValue = dbEntryPtr->fid;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_FID_E,
            hwValue);

    /* Muxed fields set */
    switch (fdbManagerPtr->entryAttrInfo.entryMuxingMode)
    {
        default:
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E:
            maxSourceId = (1 << 9);
            maxUdb      = (1 << 8);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E:
            maxSourceId = (1 << 12);
            maxUdb      = (1 << 5);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E:
            maxSourceId = (1 << 6);
            maxUdb      = (1 << 8);
            tag1VidFdbEn= GT_TRUE;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E:
            maxSourceId = (1 << 6);
            maxUdb      = (1 << 5);
            tag1VidFdbEn= GT_TRUE;
            break;
    }

    if((dbEntryPtr->srcId >= maxSourceId) || (dbEntryPtr->userDefined >= maxUdb))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Src D */
    hwValue = U32_GET_FIELD_MAC(dbEntryPtr->srcId,0,6);
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_5_0_E,
            hwValue);

    if(maxSourceId > 0x1FF)
    {
        hwValue = U32_GET_FIELD_MAC(dbEntryPtr->srcId,6,3);
        SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_8_6_E,
                hwValue);
    }

    /* NOTE: setting the 8 bits of UDB must come before setting of SOURCE_ID_11_9
       to allow SOURCE_ID_11_9 to override the 3 bits ! */
    hwValue = dbEntryPtr->userDefined;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_USER_DEFINED_E,
            hwValue);
    if(maxSourceId > 0xFFF)
    {
        hwValue = U32_GET_FIELD_MAC(dbEntryPtr->srcId,9,3);
        SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_11_9_E,
                hwValue);
    }

    if (tag1VidFdbEn != GT_FALSE)
    {
        hwValue = dbEntryPtr->vid1;
        SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_ORIG_VID1_E,
                hwValue);
    }
    else
    {
        hwValue = dbEntryPtr->daAccessLevel;
        SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL_E,
                hwValue);

        hwValue = dbEntryPtr->saAccessLevel;
        SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_SA_ACCESS_LEVEL_E,
                hwValue);
    }

    /* MAC entry */
    SIP5_FDB_HW_ENTRY_FIELD_MAC_ADDR_SET_MAC(hwDataArr,
        dbEntryPtr->macAddr_low_32,
        dbEntryPtr->macAddr_high_16);

    /* SA cmd */
    hwValue = dbEntryPtr->saCommand;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SA_CMD_E,
            hwValue);

    /* DA cmd */
    hwValue = dbEntryPtr->daCommand;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DA_CMD_E,
            hwValue);

    /* static */
    hwValue = BIT2BOOL_MAC(dbEntryPtr->isStatic);
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_IS_STATIC_E,
            hwValue);

    /* DA Route */
    hwValue = BIT2BOOL_MAC(dbEntryPtr->daRoute);
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DA_ROUTE_E,
            hwValue);

    /* spUnknown - NA storm prevent entry */
    hwValue = BIT2BOOL_MAC(dbEntryPtr->spUnknown);
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E,
            hwValue);

    /* SaQosProfileIndex */
    hwValue = dbEntryPtr->saQosIndex;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SA_QOS_PARAM_SET_IDX_E,
            hwValue);

    /* DaQosProfileIndex */
    hwValue = dbEntryPtr->daQosIndex;
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DA_QOS_PARAM_SET_IDX_E,
            hwValue);

    /* SA Lookup Ingress Mirror to Analyzer Enable */
    hwValue = BIT2BOOL_MAC(dbEntryPtr->saLookupIngressMirrorToAnalyzerPort);
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E,
            hwValue);

    /* DA Lookup Ingress Mirror to Analyzer Enable */
    hwValue = BIT2BOOL_MAC(dbEntryPtr->daLookupIngressMirrorToAnalyzerPort);
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E,
            hwValue);

    /* AppSpecific CPU Code */
    hwValue = BIT2BOOL_MAC(dbEntryPtr->appSpecificCpuCode);
    SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E,
            hwValue);

    rc = buildHwEntry_dstInterface_macOrIpmc_sip5(GT_TRUE /* MAC entry */,
            hwDataArr,
            dbEntryPtr->dstInterface_type,
            &dbEntryPtr->dstInterface);
    return rc;
}

/**
 * @internal prvCpssDxChFdbManagerHwBuildHwFormatFromDbEntry_sip5 function
 * @endinternal
 *
* @brief  function to build the HW format for SIP4 device at : fdbManagerPtr->tempInfo.sip5_hwFormatWords
*         for the fdbEntryMacAddrFormat
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] isSip5_20              - Indicates the device is SIP5 or SIP5_20.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman; AC5;.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS prvCpssDxChFdbManagerHwBuildHwFormatFromDbEntry_sip5
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN GT_BOOL                                                  isSip5_20,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC           *dbEntryPtr
)
{
    switch(dbEntryPtr->hwFdbEntryType)
    {
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E:
            return buildHwEntry_prvFdbEntryMacAddrFormat_sip5(fdbManagerPtr,
                    isSip5_20,
                    &dbEntryPtr->specificFormat.prvMacEntryFormat);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E:
            return buildHwEntry_prvIpv4UcEntryFormat_sip5(fdbManagerPtr,
                    isSip5_20,
                    &dbEntryPtr->specificFormat.prvIpv4UcEntryFormat);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E:
            cpssOsMemSet(fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords, 0,
                    sizeof(fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords));
            return buildHwEntry_prvIpv6UcEntryFormat_sip5(fdbManagerPtr,
                    isSip5_20,
                    &dbEntryPtr->specificFormat.prvIpv6UcDataEntryFormat,
                    &dbEntryPtr->specificFormat.prvIpv6UcKeyEntryFormat);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            return buildHwEntry_prvFdbEntryIpMcAddrFormat_sip5(fdbManagerPtr,
                    isSip5_20,
                    &dbEntryPtr->specificFormat.prvIpv4McEntryFormat,
                    PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            return buildHwEntry_prvFdbEntryIpMcAddrFormat_sip5(fdbManagerPtr,
                    isSip5_20,
                    &dbEntryPtr->specificFormat.prvIpv6McEntryFormat,
                    PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E);
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryPtr->hwFdbEntryType);
    }
}

/**
* @internal buildHwEntry_prvFdbEntryMacAddrFormat function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords
*         for the fdbEntryMacAddrFormat
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildHwEntry_prvFdbEntryMacAddrFormat
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC  *dbEntryPtr
)
{
    GT_STATUS   rc;
    GT_U32      hwValue;
    GT_U32      *hwDataArr;

    hwDataArr =  fdbManagerPtr->tempInfo.sip6_hwFormatWords;

    /* valid */
    hwValue = 1;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_VALID_E,
        hwValue);

    /* skip */
    hwValue = 0;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_SKIP_E,
        hwValue);

    /* age */
    hwValue = BOOL2BIT_MAC(dbEntryPtr->age);
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_AGE_E,
        hwValue);

    /* entry type */
    hwValue = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
        hwValue);

    /* FID */
    hwValue = dbEntryPtr->fid;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_FID_E,
        hwValue);

    /* MAC address */
    SIP6_FDB_HW_ENTRY_FIELD_MAC_ADDR_SET_MAC(hwDataArr,
        dbEntryPtr->macAddr_low_32,
        dbEntryPtr->macAddr_high_16);

    /* SA cmd */
    hwValue = dbEntryPtr->saCommand;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_SA_CMD_E,
        hwValue);

    /* static */
    hwValue = BIT2BOOL_MAC(dbEntryPtr->isStatic);
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_IS_STATIC_E,
        hwValue);

    /* DA cmd */
    hwValue = dbEntryPtr->daCommand;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_DA_CMD_E,
        hwValue);

    /* DA Route */
    hwValue = BIT2BOOL_MAC(dbEntryPtr->daRoute);
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_DA_ROUTE_E,
        hwValue);

    /* spUnknown - NA storm prevent entry */
    hwValue = 0;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E,
        hwValue);

    /* AppSpecific CPU Code */
    hwValue = BIT2BOOL_MAC(dbEntryPtr->appSpecificCpuCode);
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E,
        hwValue);

    rc = buildHwEntry_prvFdbEntryMacAddrFormat_mux_fields(fdbManagerPtr,dbEntryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = buildHwEntry_dstInterface_macOrIpmc(fdbManagerPtr,GT_TRUE /* MAC entry */,
                                             dbEntryPtr->dstInterface_type,
                                             &dbEntryPtr->dstInterface);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal buildHwEntry_prvFdbEntryIpMcAddrFormat function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords
*         for the fdbEntryIpv4McFormat and fdbEntryIpv6McFormat
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
* @param[in] fdbEntryType           - FDB entry type
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildHwEntry_prvFdbEntryIpMcAddrFormat
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC           *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC *dbEntryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_ENT          fdbEntryType
)
{
    GT_STATUS   rc;
    GT_U32      hwValue;
    GT_U32      *hwDataArr;

    hwDataArr =  fdbManagerPtr->tempInfo.sip6_hwFormatWords;

    /* valid */
    hwValue = 1;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_VALID_E,
        hwValue);

    /* skip */
    hwValue = 0;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_SKIP_E,
        hwValue);

    /* age */
    hwValue = BOOL2BIT_MAC(dbEntryPtr->age);
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_AGE_E,
        hwValue);

    /* entry type */
    hwValue = fdbEntryType;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
        hwValue);

    /* FID */
    hwValue = dbEntryPtr->fid;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_FID_E,
        hwValue);

    /*SIP*/
    hwValue = dbEntryPtr->sipAddr;
    SIP6_FDB_HW_ENTRY_FIELD_SIP_ADDR_SET_MAC(hwDataArr,
        hwValue);

    /*DIP*/
    hwValue = dbEntryPtr->dipAddr;
    SIP6_FDB_HW_ENTRY_FIELD_DIP_ADDR_SET_MAC(hwDataArr,
        hwValue);

    /* static */
    hwValue = BIT2BOOL_MAC(dbEntryPtr->isStatic);
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_IS_STATIC_E,
        hwValue);

    /* DA cmd */
    hwValue = dbEntryPtr->daCommand;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_DA_CMD_E,
        hwValue);

    /* DA Route */
    hwValue = BIT2BOOL_MAC(dbEntryPtr->daRoute);
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_DA_ROUTE_E,
        hwValue);

    /* spUnknown - NA storm prevent entry */
    hwValue = 0;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E,
        hwValue);

    /* AppSpecific CPU Code */
    hwValue = BIT2BOOL_MAC(dbEntryPtr->appSpecificCpuCode);
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E,
        hwValue);

    rc = buildHwEntry_prvFdbEntryIpMcFormat_mux_fields(fdbManagerPtr,dbEntryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

        rc = buildHwEntry_dstInterface_macOrIpmc(fdbManagerPtr, GT_FALSE /* IP MC entry */,
                                             dbEntryPtr->dstInterface_type,
                                             &dbEntryPtr->dstInterface);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal buildHwEntry_dstInterface_IpvxUc function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords
*         for the IPvx UC routing : the 'dstInterface' part
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dstInterface_type      - the type of dstInterface
* @param[in] dstInterfacePtr        - (pointer to) the manager dstInterface format (manager format)
* @param[in] isIpv6Format           - when ipv6 format the data need to be set inside 'sip6_ipv6Ext_hwFormatWords'
*                                     instead of sip6_hwFormatWords
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   buildHwEntry_dstInterface_IpvxUc
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC      *fdbManagerPtr,
    IN CPSS_INTERFACE_TYPE_ENT                             dstInterface_type,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INTERFACE_INFO_UNT     *dstInterfacePtr,
    IN GT_BOOL                                             isIpv6Format
)
{
    GT_U32      devTmp,multiple,isTrunk,portTrunk,vidx;
    GT_U32      hwValue;
    GT_U32      *hwDataArr;

    devTmp      = 0;
    multiple    = 0;
    isTrunk     = 0;
    portTrunk   = 0;
    vidx        = 0;

    switch(dstInterface_type)
    {
        case CPSS_INTERFACE_PORT_E:
            portTrunk = dstInterfacePtr->devPort.portNum;
            devTmp = dstInterfacePtr->devPort.hwDevNum;
            break;
        case CPSS_INTERFACE_TRUNK_E:
            isTrunk = 1;
            portTrunk = dstInterfacePtr->trunkId;
            break;
        case CPSS_INTERFACE_VIDX_E:
            multiple = 1;
            vidx = dstInterfacePtr->vidx;
            break;
        case CPSS_INTERFACE_VID_E:
            multiple = 1;
            vidx = 0xFFF;
            /* no param needed !!! */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dstInterface_type);
    }

    if(isIpv6Format == GT_FALSE)/*no ipv6*/
    {
        hwDataArr =  fdbManagerPtr->tempInfo.sip6_hwFormatWords;
    }
    else
    {
        hwDataArr =  fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords;
    }

    /* devNum ID */
    hwValue = devTmp;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_DEV_E,
        hwValue);

    /* multiple */
    hwValue = multiple;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_USE_VIDX_E,
        hwValue);

    if (multiple == 0)
    {/* bit40MacAddr=0 and entry is PORT or TRUNK */

        /* is Trunk bit */
        hwValue = isTrunk;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_IS_TRUNK_E,
            hwValue);

        /* PortNum/TrunkNum */
        if(isTrunk)
        {
            hwValue = portTrunk;
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_TRUNK_ID_E,
                hwValue);
        }
        else
        {
            hwValue = portTrunk;
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_EPORT_E,
                hwValue);
        }
    }
    else
    {
        /* VIDX */
        hwValue = vidx;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_EVIDX_E,
            hwValue);
    }

    return GT_OK;
}

/**
* @internal buildHwEntry_prvIpv4UcEntryFormat function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords
*         for the prvIpv4UcEntryFormat
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
*         based on buildFdbUcRouteEntryHwFormatSip6
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildHwEntry_prvIpv4UcEntryFormat
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC         *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV4_UC_FORMAT_STC  *dbEntryPtr
)
{
    GT_STATUS   rc;
    GT_U32      hwValue;
    GT_U32      *hwDataArr;

    hwDataArr =  fdbManagerPtr->tempInfo.sip6_hwFormatWords;

    /* valid */
    hwValue = 1;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_VALID_E,
        hwValue);

    /* skip */
    hwValue = 0;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_SKIP_E,
        hwValue);

    /* age */
    hwValue = BOOL2BIT_MAC(dbEntryPtr->ipUcCommonInfo.age);
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_AGE_E,
        hwValue);

    /* entry type */
    hwValue = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
        hwValue);

    hwValue = dbEntryPtr->ipAddr;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV4_DIP_E,
        hwValue);

    /* VRF-ID */
    hwValue = dbEntryPtr->ipUcCommonInfo.vrfId;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_VRF_ID_E,
        hwValue);


    if(dbEntryPtr->ipUcCommonInfo.ucRouteExtType <=
       CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E)
    {
        hwValue = 0;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE_E,
            hwValue);
        /*
            case of UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E --> tunnelStartPointer
            case of UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E          --> natPointer
            case of UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E          --> arpPointer
        */
        /* ttl/hop decrement enable */
        hwValue = dbEntryPtr->ipUcCommonInfo.ttlHopLimitDecEnable;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT_E,
            hwValue);

        /* Bypass TTL Options Or Hop Extension */
        hwValue = dbEntryPtr->ipUcCommonInfo.ttlHopLimDecOptionsExtChkByPass;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E,
            hwValue);

        hwValue = dbEntryPtr->ipUcCommonInfo.countSet;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX_E,
            hwValue);

        hwValue = dbEntryPtr->ipUcCommonInfo.ICMPRedirectEnable;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN_E,
            hwValue);

        /* mtu profile index */
        hwValue = dbEntryPtr->ipUcCommonInfo.mtuProfileIndex;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_MTU_INDEX_E,
            hwValue);

        /* set vlan associated with this entry */
        hwValue = dbEntryPtr->ipUcCommonInfo.nextHopVlanId;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN_E,
            hwValue);

        if (dbEntryPtr->ipUcCommonInfo.ucRouteExtType ==
            CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E ||
            dbEntryPtr->ipUcCommonInfo.ucRouteExtType ==
            CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E )
        {
            /* set tunnel type */
            hwValue = 1; /* the only so far supported tunneltype is IPv4 */
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE_E,
                hwValue);
            /* set tunnel ptr */
            hwValue = dbEntryPtr->ipUcCommonInfo.pointerInfo;
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR_E,
                hwValue);

            if(dbEntryPtr->ipUcCommonInfo.ucRouteExtType ==
                CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E)
            {
                hwValue = 1;
                SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT_E,
                    hwValue);
            }

            /* set that the entry is start of tunnel */
            hwValue = 1;
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
               SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL_E,
               hwValue);
         }
         else  /* ARP */
         {
            /* arp ptr*/
            hwValue = dbEntryPtr->ipUcCommonInfo.pointerInfo;
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ARP_PTR_E,
                hwValue);
         }

        rc = buildHwEntry_dstInterface_IpvxUc(fdbManagerPtr,
            dbEntryPtr->ipUcCommonInfo.dstInterface_type,
            &dbEntryPtr->ipUcCommonInfo.dstInterface,
            GT_FALSE/* non-ipv6*/);
        if(rc != GT_OK)
        {
            return rc;
        }

    }
    else /* UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E /
            UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E        */
    {
        hwValue = 1;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE_E,
            hwValue);
        hwValue = dbEntryPtr->ipUcCommonInfo.pointerInfo;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ECMP_OR_NHE_POINTER_E,
            hwValue);
        if(dbEntryPtr->ipUcCommonInfo.ucRouteExtType ==
           CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E)
        {
            hwValue = 1;/* 'trick' use the 'is_NAT' places in same bit as the 'pointer_type' */
        }
        else
        {
            hwValue = 0;/* 'trick' use the 'is_NAT' places in same bit as the 'pointer_type' */
        }

        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT_E,
            hwValue);
    }

    return GT_OK;
}

/**
* @internal buildHwEntry_prvIpv6UcEntryFormat function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords,
*                                              fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords
*         for the prvIpv6UcDataEntryFormat and prvIpv6UcKeyEntryFormat
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
*         based on buildFdbUcRouteEntryHwFormatSip6
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryDataPtr         - (pointer to) the manager entry format(data) (manager format)
* @param[in] dbEntryKeyPtr          - (pointer to) the manager entry format(key) (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildHwEntry_prvIpv6UcEntryFormat
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC               *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_DATA_FORMAT_STC   *dbEntryDataPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_KEY_FORMAT_STC    *dbEntryKeyPtr
)
{
    GT_STATUS   rc;
    GT_U32      hwValue;
    GT_U32      *hwDataArr;
    GT_U32      *hwKeyArr;

    hwKeyArr    = fdbManagerPtr->tempInfo.sip6_hwFormatWords;
    hwDataArr   = fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords;
    /* valid */
    hwValue = 1;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_VALID_E,
        hwValue);

    /* skip */
    hwValue = 0;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_SKIP_E,
        hwValue);

    /* age */
    hwValue = BOOL2BIT_MAC(dbEntryDataPtr->ipUcCommonInfo.age);
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_AGE_E,
        hwValue);

    /* entry type */
    hwValue = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
        hwValue);

    /* set ipv6 dip */
    hwValue = dbEntryDataPtr->ipAddr_127_106;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_127_106_E,
        hwValue);

    /* VRF-ID */
    hwValue = dbEntryDataPtr->ipUcCommonInfo.vrfId;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_VRF_ID_E,
        hwValue);

    hwValue = dbEntryDataPtr->ipv6DestSiteId;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID_E,
        hwValue);

    hwValue = dbEntryDataPtr->ipv6ScopeCheck;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK_E,
        hwValue);

    if(dbEntryDataPtr->ipUcCommonInfo.ucRouteExtType <=
       CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E)
    {
        hwValue = 0;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE_E,
            hwValue);
        /*
            case of UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E --> tunnelStartPointer
            case of UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E          --> natPointer
            case of UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E          --> arpPointer
        */
        /* ttl/hop decrement enable */
        hwValue = dbEntryDataPtr->ipUcCommonInfo.ttlHopLimitDecEnable;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT_E,
            hwValue);

        /* Bypass TTL Options Or Hop Extension */
        hwValue = dbEntryDataPtr->ipUcCommonInfo.ttlHopLimDecOptionsExtChkByPass;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E,
            hwValue);

        hwValue = dbEntryDataPtr->ipUcCommonInfo.countSet;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX_E,
            hwValue);

        hwValue = dbEntryDataPtr->ipUcCommonInfo.ICMPRedirectEnable;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN_E,
            hwValue);

        /* mtu profile index */
        hwValue = dbEntryDataPtr->ipUcCommonInfo.mtuProfileIndex;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_MTU_INDEX_E,
            hwValue);

        /* set vlan associated with this entry */
        hwValue = dbEntryDataPtr->ipUcCommonInfo.nextHopVlanId;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN_E,
            hwValue);

        if (dbEntryDataPtr->ipUcCommonInfo.ucRouteExtType ==
            CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E ||
            dbEntryDataPtr->ipUcCommonInfo.ucRouteExtType ==
            CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E )
        {
            /* set tunnel type */
            hwValue = 1; /* the only so far supported tunneltype is IPv4 */
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE_E,
                hwValue);
            /* set tunnel ptr */
            hwValue = dbEntryDataPtr->ipUcCommonInfo.pointerInfo;
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR_E,
                hwValue);

            if(dbEntryDataPtr->ipUcCommonInfo.ucRouteExtType ==
                CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E)
            {
                hwValue = 1;
                SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT_E,
                    hwValue);
            }

            /* set that the entry is start of tunnel */
            hwValue = 1;
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
               SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL_E,
               hwValue);
         }
         else  /* ARP */
         {
            /* arp ptr*/
            hwValue = dbEntryDataPtr->ipUcCommonInfo.pointerInfo;
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ARP_PTR_E,
                hwValue);
         }

        rc = buildHwEntry_dstInterface_IpvxUc(fdbManagerPtr,
            dbEntryDataPtr->ipUcCommonInfo.dstInterface_type,
            &dbEntryDataPtr->ipUcCommonInfo.dstInterface,
            GT_TRUE/* ipv6*/);
        if(rc != GT_OK)
        {
            return rc;
        }

    }
    else /* UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E /
            UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E        */
    {
        hwValue = 1;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE_E,
            hwValue);
        hwValue = dbEntryDataPtr->ipUcCommonInfo.pointerInfo;
        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ECMP_OR_NHE_POINTER_E,
            hwValue);
        if(dbEntryDataPtr->ipUcCommonInfo.ucRouteExtType ==
           CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E)
        {
            hwValue = 1;/* 'trick' use the 'is_NAT' places in same bit as the 'pointer_type' */
        }
        else
        {
            hwValue = 0;/* 'trick' use the 'is_NAT' places in same bit as the 'pointer_type' */
        }

        SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT_E,
            hwValue);
    }

    /* start key data */

    /* valid */
    hwValue = 1;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwKeyArr,
        SIP6_FDB_FDB_TABLE_FIELDS_VALID_E,
        hwValue);

    /* skip */
    hwValue = 0;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwKeyArr,
        SIP6_FDB_FDB_TABLE_FIELDS_SKIP_E,
        hwValue);

    /* age */
    hwValue = BOOL2BIT_MAC(dbEntryKeyPtr->age);
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwKeyArr,
        SIP6_FDB_FDB_TABLE_FIELDS_AGE_E,
        hwValue);

    /* entry type */
    hwValue = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwKeyArr,
        SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
        hwValue);

    hwValue = dbEntryKeyPtr->ipAddr_31_0;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwKeyArr,
                                    SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_31_0_E,
                                    hwValue);
    hwValue = dbEntryKeyPtr->ipAddr_63_32;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwKeyArr,
                                    SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_63_32_E,
                                    hwValue);
    hwValue = dbEntryKeyPtr->ipAddr_95_64;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwKeyArr,
                                    SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_95_64_E,
                                    hwValue);
    hwValue = dbEntryKeyPtr->ipAddr_105_96;
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwKeyArr,
                                    SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_105_96_E,
                                    hwValue);

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerHwBuildHwFormatFromDbEntry_sip6 function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   prvCpssDxChFdbManagerHwBuildHwFormatFromDbEntry_sip6
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr
)
{
    cpssOsMemSet(fdbManagerPtr->tempInfo.sip6_hwFormatWords, 0, sizeof(fdbManagerPtr->tempInfo.sip6_hwFormatWords));
    switch(dbEntryPtr->hwFdbEntryType)
    {
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E:
            return buildHwEntry_prvFdbEntryMacAddrFormat(fdbManagerPtr,
                                                    &dbEntryPtr->specificFormat.prvMacEntryFormat);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E:
            return buildHwEntry_prvIpv4UcEntryFormat(fdbManagerPtr,
                                                    &dbEntryPtr->specificFormat.prvIpv4UcEntryFormat);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E:
            cpssOsMemSet(fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords, 0, sizeof(fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords));
            return buildHwEntry_prvIpv6UcEntryFormat(fdbManagerPtr,
                                                    &dbEntryPtr->specificFormat.prvIpv6UcDataEntryFormat,
                                                    &dbEntryPtr->specificFormat.prvIpv6UcKeyEntryFormat);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            return buildHwEntry_prvFdbEntryIpMcAddrFormat(fdbManagerPtr,
                                                    &dbEntryPtr->specificFormat.prvIpv4McEntryFormat,
                                                    PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            return buildHwEntry_prvFdbEntryIpMcAddrFormat(fdbManagerPtr,
                                                    &dbEntryPtr->specificFormat.prvIpv6McEntryFormat,
                                                    PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E);
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryPtr->hwFdbEntryType);
    }
}

/**
* @internal prvCpssDxChFdbManagerHwBuildHwFormatFromDbEntry function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   prvCpssDxChFdbManagerHwBuildHwFormatFromDbEntry
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr
)
{
    GT_STATUS rc;

    switch(fdbManagerPtr->entryAttrInfo.shadowType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E:
            return prvCpssDxChFdbManagerHwBuildHwFormatFromDbEntry_sip4(fdbManagerPtr, dbEntryPtr);
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E:
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E:
            if(fdbManagerPtr->hwDeviceTypesBitmap & PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP6_E)
            {
                rc = prvCpssDxChFdbManagerHwBuildHwFormatFromDbEntry_sip6(fdbManagerPtr, dbEntryPtr);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            if(fdbManagerPtr->hwDeviceTypesBitmap & PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_E)
            {
                rc = prvCpssDxChFdbManagerHwBuildHwFormatFromDbEntry_sip5(fdbManagerPtr,
                        GT_FALSE,   /* isSip5_20 */
                        dbEntryPtr);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            if(fdbManagerPtr->hwDeviceTypesBitmap & PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_20_E)
            {
                rc = prvCpssDxChFdbManagerHwBuildHwFormatFromDbEntry_sip5(fdbManagerPtr,
                        GT_TRUE,   /* isSip5_20 */
                        dbEntryPtr);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            return GT_OK;
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E:
            return prvCpssDxChFdbManagerHwBuildHwFormatFromDbEntry_sip6(fdbManagerPtr, dbEntryPtr);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(fdbManagerPtr->entryAttrInfo.shadowType);
    }
}

/**
* @internal prvCpssDxChFdbManagerHwBuildIpV6UcHwFormatFromDbEntry function
* @endinternal
*
* @brief  function to build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords for IPv6 UC entry
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager IPv6 UC key entry format (manager format)
* @param[in] dbEntryExtPtr          - (pointer to) the manager IPv6 UC data entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   prvCpssDxChFdbManagerHwBuildIpV6UcHwFormatFromDbEntry
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryExtPtr
)
{
    GT_STATUS rc;

    switch(fdbManagerPtr->entryAttrInfo.shadowType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E:
            cpssOsMemSet(fdbManagerPtr->tempInfo.sip6_hwFormatWords, 0, sizeof(fdbManagerPtr->tempInfo.sip6_hwFormatWords));
            cpssOsMemSet(fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords, 0, sizeof(fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords));
            return buildHwEntry_prvIpv6UcEntryFormat(fdbManagerPtr,
                    &dbEntryExtPtr->specificFormat.prvIpv6UcDataEntryFormat,
                    &dbEntryPtr->specificFormat.prvIpv6UcKeyEntryFormat);
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E:
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E:
            if(fdbManagerPtr->hwDeviceTypesBitmap & PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP6_E)
            {
                cpssOsMemSet(fdbManagerPtr->tempInfo.sip6_hwFormatWords, 0, sizeof(fdbManagerPtr->tempInfo.sip6_hwFormatWords));
                cpssOsMemSet(fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords, 0, sizeof(fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords));
                rc = buildHwEntry_prvIpv6UcEntryFormat(fdbManagerPtr,
                        &dbEntryExtPtr->specificFormat.prvIpv6UcDataEntryFormat,
                        &dbEntryPtr->specificFormat.prvIpv6UcKeyEntryFormat);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            if(fdbManagerPtr->hwDeviceTypesBitmap & PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_E)
            {
                rc = buildHwEntry_prvIpv6UcEntryFormat_sip5(fdbManagerPtr,
                        GT_FALSE, /* isSip5_20 */
                        &dbEntryExtPtr->specificFormat.prvIpv6UcDataEntryFormat,
                        &dbEntryPtr->specificFormat.prvIpv6UcKeyEntryFormat);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            if(fdbManagerPtr->hwDeviceTypesBitmap & PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_20_E)
            {
                rc = buildHwEntry_prvIpv6UcEntryFormat_sip5(fdbManagerPtr,
                        GT_TRUE, /* isSip5_20 */
                        &dbEntryExtPtr->specificFormat.prvIpv6UcDataEntryFormat,
                        &dbEntryPtr->specificFormat.prvIpv6UcKeyEntryFormat);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            return GT_OK;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(fdbManagerPtr->entryAttrInfo.shadowType);
    }
}

/**
* @internal updateOwnHwDevNumOnMacUcInterfacePortFormat function
* @endinternal
*
* @brief  function to update the build the hwFormatWords for the 'hwDevNum'
*       called only if : the entry is of type ''mac entry' and interface is 'port'
*
* @param[in]  fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in]  devNum                - sw devNum (to which we write the entry)
*                                     that is converted to hwDevNum.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   updateOwnHwDevNumOnMacUcInterfacePortFormat
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN GT_U8                                            devNum
)
{
    GT_HW_DEV_NUM                                       hwDevNum;
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT    hwFormatType;
    GT_U32                                             *hwDataArr;
    GT_STATUS                                           rc;

    /* we lock the CPSS DB ... to be able to access 'PRV_CPSS_PP_MAC(devNum)' */

    rc = prvCpssDxChFdbManagerDeviceHwFormatGet(fdbManagerPtr, devNum, &hwFormatType);
    if(rc != GT_OK)
    {
        return rc;
    }
    LOCK_DEV_NUM(devNum)
    hwDevNum  = PRV_CPSS_HW_DEV_NUM_MAC(devNum);
    UNLOCK_DEV_NUM(devNum);/* we are done with this device internal DB */

    switch(hwFormatType)
    {
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP4_E:
            hwDataArr = fdbManagerPtr->tempInfo.sip6_hwFormatWords;
            /* devNum ID 5 bits */
            hwDataArr[2] &= ~ ((           0x1F) << 1);
            hwDataArr[2] |=   ((hwDevNum & 0x1F) << 1);
            break;
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP6_E:
            hwDataArr = fdbManagerPtr->tempInfo.sip6_hwFormatWords;
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_DEV_ID_E,
                    hwDevNum);
            break;
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_20_E:
            hwDataArr = fdbManagerPtr->tempInfo.sip5_20_hwFormatWords;
            SIP5_20_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_DEV_ID_E,
                    hwDevNum);
            break;
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_E:
            hwDataArr = fdbManagerPtr->tempInfo.sip5_20_hwFormatWords;
            SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_DEV_ID_E,
                    hwDevNum);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(hwFormatType);
    }

    return GT_OK;
}

/**
* @internal updateOwnHwDevNumOnIpvxUcInterfacePortFormat function
* @endinternal
*
* @brief  function to update the build the hwFormatWords for the 'hwDevNum'
*       called only if : the entry is of type 'ipv4/6 UC' and interface is 'port'
*
* @param[in]  fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in]  devNum                - sw devNum (to which we write the entry)
*                                     that is converted to hwDevNum.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   updateOwnHwDevNumOnIpvxUcInterfacePortFormat
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr,
    IN GT_U8                                            devNum
)
{
    GT_HW_DEV_NUM                                       hwDevNum;
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT    hwFormatType;
    GT_U32                                             *hwDataArr;
    GT_STATUS                                           rc;

    /* we lock the CPSS DB ... to be able to access 'PRV_CPSS_PP_MAC(devNum)' */
    rc = prvCpssDxChFdbManagerDeviceHwFormatGet(fdbManagerPtr, devNum, &hwFormatType);
    if(rc != GT_OK)
    {
        return rc;
    }

    LOCK_DEV_NUM(devNum);
    hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);
    UNLOCK_DEV_NUM(devNum);/* we are done with this device internal DB */

    switch(hwFormatType)
    {
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP6_E:
            hwDataArr = fdbManagerPtr->tempInfo.sip6_hwFormatWords;
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_DEV_E,
                    hwDevNum);
            break;
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_20_E:
            hwDataArr = fdbManagerPtr->tempInfo.sip5_20_hwFormatWords;
            SIP5_20_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_TARGET_DEVICE_E,
                    hwDevNum);
            break;
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_E:
            hwDataArr = fdbManagerPtr->tempInfo.sip5_hwFormatWords;
            SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_TARGET_DEVICE_E,
                    hwDevNum);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(hwFormatType);
    }

    return GT_OK;
}

/**
* @internal checkForOwnHwDevUpdate function
* @endinternal
*
* @brief  function check if need to update the hwDevNum to be 'ownDevNum'
*         currently the logic is that 'global eports' need to have 'ownDevNum'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] devNum                 - the device number
* @param[in] calcInfoPtr            - using calcInfoPtr->dbEntryPtr
* @param[in] isIpv6                 - indication that the info is for ipv6 UC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS checkForOwnHwDevUpdate(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC       *fdbManagerPtr,
    IN GT_U8                                                devNum,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC        *dbEntryPtr,
    IN GT_BOOL                                              isIpv6Uc
)
{
    /* support global eports , that the 'hwDevNum' in each device should be according to 'own hwDevNum' */
    if(fdbManagerPtr->entryAttrInfo.globalEportInfo.enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_DISABLE_E ||
       fdbManagerPtr->entryAttrInfo.shadowType             == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E)
    {
        /* no global eport support */
        return GT_OK;
    }

    if(isIpv6Uc == GT_TRUE)
    {
        if(dbEntryPtr->specificFormat.prvIpv6UcDataEntryFormat.ipUcCommonInfo.ucRouteExtType <=
                   CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E &&
           dbEntryPtr->specificFormat.prvIpv6UcDataEntryFormat.ipUcCommonInfo.dstInterface_type == CPSS_INTERFACE_PORT_E)
        {
            if(GT_TRUE == prvCpssDxChFdbManagerDbIsGlobalEport(fdbManagerPtr,
                dbEntryPtr->specificFormat.prvIpv6UcDataEntryFormat.ipUcCommonInfo.dstInterface.devPort.portNum))
            {
                /* we update the entry before we write to HW , and we NOT restore to orig value ,
                   because if this sip6_hwFormatWords is used on next device too , then
                   the 'own hwDevNum' of next device will also update the entry
                */
                updateOwnHwDevNumOnIpvxUcInterfacePortFormat(fdbManagerPtr, devNum);
            }
        }

        return GT_OK;
    }

    if(dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E &&
       dbEntryPtr->specificFormat.prvMacEntryFormat.dstInterface_type == CPSS_INTERFACE_PORT_E)
    {
        if(GT_TRUE == prvCpssDxChFdbManagerDbIsGlobalEport(fdbManagerPtr,
            dbEntryPtr->specificFormat.prvMacEntryFormat.dstInterface.devPort.portNum))
        {
            /* we update the entry before we write to HW , and we NOT restore to orig value ,
               because if this sip6_hwFormatWords is used on next device too , then
               the 'own hwDevNum' of next device will also update the entry
            */
            updateOwnHwDevNumOnMacUcInterfacePortFormat(fdbManagerPtr, devNum);
        }
    }
    else
    if(dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E &&
       dbEntryPtr->specificFormat.prvIpv4UcEntryFormat.ipUcCommonInfo.ucRouteExtType <=
           CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E &&
        dbEntryPtr->specificFormat.prvIpv4UcEntryFormat.ipUcCommonInfo.dstInterface_type == CPSS_INTERFACE_PORT_E)
    {
        if(GT_TRUE == prvCpssDxChFdbManagerDbIsGlobalEport(fdbManagerPtr,
            dbEntryPtr->specificFormat.prvIpv4UcEntryFormat.ipUcCommonInfo.dstInterface.devPort.portNum))
        {
            /* we update the entry before we write to HW , and we NOT restore to orig value ,
               because if this sip6_hwFormatWords is used on next device too , then
               the 'own hwDevNum' of next device will also update the entry
            */
            updateOwnHwDevNumOnIpvxUcInterfacePortFormat(fdbManagerPtr, devNum);
        }
    }

    return GT_OK;
}

/*
* @internal prvCpssDxChFdbManagerHwWriteByHwIndex_perDevNum function
* @endinternal
*
* @brief   This function write HW entry of specific device at hwIndex.
*          the HW format is at : fdbManagerPtr->tempInfo.sip6_hwFormatWords
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] devNum                 - the device number
* @param[in] calcInfoPtr            - using calcInfoPtr->dbEntryPtr
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - convert of manager format to HW format failed.
*
* @note
*   NONE
*
*/
static GT_STATUS   prvCpssDxChFdbManagerHwWriteByHwIndex_perDevNum
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC       *fdbManagerPtr,
    IN GT_U8                                                devNum,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC     *calcInfoPtr
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC        *dbEntryPtr    = calcInfoPtr->dbEntryPtr;
    UPDATE_HW_BANK_COUNTER_ACTION_ENT                     updateHwBankCounterAction; /* action for bank counter */
    GT_U32                                               *hwDataPtr;

    /* do not update bank counters for update entry operation */
    updateHwBankCounterAction = (calcInfoPtr->calcEntryType == PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E) ?
                                 UPDATE_HW_BANK_COUNTER_ACTION_NONE_E : UPDATE_HW_BANK_COUNTER_ACTION_INC_E;

    /*check if need to update the hwDevNum to be 'ownDevNum'*/
    checkForOwnHwDevUpdate(fdbManagerPtr,devNum,calcInfoPtr->dbEntryPtr,GT_FALSE/*non-ipv6*/);

    if((fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E) ||
            (fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E))
    {
        LOCK_DEV_NUM(devNum)
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            hwDataPtr       = fdbManagerPtr->tempInfo.sip6_hwFormatWords;
        }
        else if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            hwDataPtr       = fdbManagerPtr->tempInfo.sip5_20_hwFormatWords;
        }
        else
        {
            hwDataPtr       = fdbManagerPtr->tempInfo.sip5_hwFormatWords;
        }
        UNLOCK_DEV_NUM(devNum)
    }
    else
    {
        hwDataPtr = fdbManagerPtr->tempInfo.sip6_hwFormatWords;
    }

    /* write the entry to the HW */
    return internal_writeFdbTableEntry(devNum, dbEntryPtr->hwIndex, hwDataPtr,
                                       calcInfoPtr->selectedBankId, updateHwBankCounterAction);
}

/*
* @internal prvCpssDxChFdbManagerHwWriteIpV6UcByHwIndex_perDevNum function
* @endinternal
*
* @brief   This function write IpV6 UC HW entry of specific device at hwIndex.
*          the HW format is at : fdbManagerPtr->tempInfo.sip6_hwFormatWords
                                 fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] devNum                 - the device number
* @param[in] calcInfoPtr            - using calcInfoPtr->dbEntryPtr,
*                                           calcInfoPtr->dbEntryExtPtr
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - convert of manager format to HW format failed.
*
* @note
*   NONE
*
*/
static GT_STATUS   prvCpssDxChFdbManagerHwWriteIpV6UcByHwIndex_perDevNum
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC       *fdbManagerPtr,
    IN GT_U8                                                devNum,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC     *calcInfoPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC        *dbEntryPtr    = calcInfoPtr->dbEntryPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC        *dbEntryExtPtr = calcInfoPtr->dbEntryExtPtr;
    UPDATE_HW_BANK_COUNTER_ACTION_ENT                     updateHwBankCounterAction; /* action for bank counter */
    GT_U32                                               *hwDataExtPtr;
    GT_U32                                               *hwDataPtr;

    /* do not update bank counters for update entry operation */
    updateHwBankCounterAction = (calcInfoPtr->calcEntryType == PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E) ?
                                 UPDATE_HW_BANK_COUNTER_ACTION_NONE_E : UPDATE_HW_BANK_COUNTER_ACTION_INC_E;

    /*check if need to update the hwDevNum to be 'ownDevNum'*/
    checkForOwnHwDevUpdate(fdbManagerPtr,devNum,calcInfoPtr->dbEntryExtPtr,GT_TRUE/*ipv6*/);

    if((fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E) ||
            (fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E))
    {
        LOCK_DEV_NUM(devNum)
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            hwDataPtr       = &fdbManagerPtr->tempInfo.sip6_hwFormatWords[0];
            hwDataExtPtr    = &fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords[0];
        }
        else if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            hwDataPtr       = &fdbManagerPtr->tempInfo.sip5_20_hwFormatWords[0];
            hwDataExtPtr    = &fdbManagerPtr->tempInfo.sip5_20_ipv6Ext_hwFormatWords[0];
        }
        else
        {
            hwDataPtr       = &fdbManagerPtr->tempInfo.sip5_hwFormatWords[0];
            hwDataExtPtr    = &fdbManagerPtr->tempInfo.sip5_ipv6Ext_hwFormatWords[0];
        }
        UNLOCK_DEV_NUM(devNum)
    }
    else
    {
        hwDataPtr       = &fdbManagerPtr->tempInfo.sip6_hwFormatWords[0];
        hwDataExtPtr    = &fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords[0];
    }

    /* write the entry to the HW */
    rc = internal_writeFdbTableEntry(devNum, dbEntryPtr->hwIndex, hwDataPtr,
            calcInfoPtr->selectedBankId, updateHwBankCounterAction);
    if (rc != GT_OK)
    {
        return rc;
    }

    return internal_writeFdbTableEntry(devNum, dbEntryExtPtr->hwIndex, hwDataExtPtr,
                                       calcInfoPtr->selectedBankId + 1, updateHwBankCounterAction);
}

/**
* @internal prvCpssDxChFdbManagerHwWriteByHwIndex function
* @endinternal
*
* @brief   This function write HW entry at hwIndex from all registered devices in the manager.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.

* @param[in] calcInfoPtr            - using calcInfoPtr->dbEntryPtr,
*                                           calcInfoPtr->dbEntryExtPtr
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - convert of manager format to HW format failed.
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChFdbManagerHwWriteByHwIndex
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC       *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC     *calcInfoPtr
)
{
    GT_STATUS   rc;
    GT_U8       devNum;

    rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr,GT_TRUE,&devNum);
    if(rc != GT_OK)
    {
        /* no devices to work with */
        return GT_OK;
    }

    if (calcInfoPtr->bankStep > 1)
    {
        /* build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords - ipv6 UC */
        rc = prvCpssDxChFdbManagerHwBuildIpV6UcHwFormatFromDbEntry(fdbManagerPtr,
                calcInfoPtr->dbEntryPtr,
                calcInfoPtr->dbEntryExtPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords */
        rc = prvCpssDxChFdbManagerHwBuildHwFormatFromDbEntry(fdbManagerPtr, calcInfoPtr->dbEntryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    while(rc == GT_OK)
    {
        if (calcInfoPtr->bankStep > 1)
        {
            /** IpV6 UC entry comprises of two types of entries - key and data and resides subsequently in DB and HW,
             *  so the order of writing/reading to/from DB/HW should be subsequent as well.
             */
            rc = prvCpssDxChFdbManagerHwWriteIpV6UcByHwIndex_perDevNum(fdbManagerPtr, devNum, calcInfoPtr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            rc = prvCpssDxChFdbManagerHwWriteByHwIndex_perDevNum(fdbManagerPtr, devNum, calcInfoPtr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr,GT_FALSE,&devNum);
    }

    return GT_OK;
}

/*
* @internal prvCpssDxChFdbManagerLearningThrottlingSet function
* @endinternal
*
* @brief   This function sets the learning throttling related parameters to the HW, according to the threshold level.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                 - the device number (cpss devNum)
* @param[in] thresholdType           - the current threshold type
*
* @retval GT_OK                    - on success
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChFdbManagerLearningThrottlingSet
(
    IN GT_U8                                            devNum,
    IN PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_ENT currentThresholdType
)
{
    GT_BOOL                 hashTooLongEn;
    GT_BOOL                 rateLimitEn;
    GT_U32                  auMsgRate;
    GT_STATUS               rc;

    switch(currentThresholdType)
    {
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_A_E:
            auMsgRate       = PRV_FDB_MANAGER_AU_MSG_RATE_STAGE_A;
            hashTooLongEn   = GT_TRUE;
            rateLimitEn     = GT_FALSE;
            break;
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_B_E:
            auMsgRate       = PRV_FDB_MANAGER_AU_MSG_RATE_STAGE_B;
            hashTooLongEn   = GT_TRUE;
            rateLimitEn     = GT_TRUE;
            break;
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_C_E:
            auMsgRate       = PRV_FDB_MANAGER_AU_MSG_RATE_STAGE_B;
            hashTooLongEn   = GT_FALSE;
            rateLimitEn     = GT_FALSE;
            break;
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_NOT_VALID_E:
            auMsgRate       = 1;
            hashTooLongEn   = GT_FALSE;
            rateLimitEn     = GT_FALSE;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(currentThresholdType);
    }

    /* Update Hash too long status in hardware */
    rc = cpssDxChBrgFdbNaMsgOnChainTooLongSet(devNum, hashTooLongEn);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Set the Au msg rate for the devices */
    return cpssDxChBrgFdbAuMsgRateLimitSet(devNum, auMsgRate, rateLimitEn);
}

/*
* @internal prvCpssDxChFdbManagerFdbAccParamSet function
* @endinternal
*
* @brief   This function sets the ACC related parameters to the HW.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                 - the device number (cpss devNum)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChFdbManagerFdbAccParamSet
(
    IN GT_U8                                            devNum
)
{
    GT_STATUS                                   rc;
    GT_U32                                      regAddr;
    GT_U32                                      regVal;
    PRV_CPSS_DXCH_AAC_CHANNELS_ENT              channel;

    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 0)
    {
        /* AAC not used for devices with single tile */
        return GT_OK;
    }

    /* AAC global control */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACControl;
    /* AAC Enable */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, 1);
    if(rc != GT_OK)
    {
        return rc;
    }

    for (channel = PRV_CPSS_DXCH_AAC_CHANNEL_FDB_MANAGER_PRIMARY_E;
         channel <= PRV_CPSS_DXCH_AAC_CHANNEL_FDB_MANAGER_SECONDARY_E; channel++)
    {
        /*  AAC Engine <<%n>> Control for channel 1,2 */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineControl[channel];
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &regVal);
        if(rc != GT_OK)
        {
            return rc;
        }
        /* AAC address mode - direct */
        U32_SET_FIELD_MAC(regVal, 0, 1, 0);
        /* AAC multicast enable */
        U32_SET_FIELD_MAC(regVal, 2, 1, 1);

        rc = prvCpssHwPpWriteRegister(devNum, regAddr, regVal);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* AAC Engine <<%n>> MC External Offset for channel 1,2 */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineMCExternalOffset[channel];
        rc = prvCpssHwPpWriteRegister(devNum, regAddr, FALCON_TILE_OFFSET_CNS);
        if(rc != GT_OK)
        {
            return rc;
        }

        /*  AAC Engine <<%n>> MC Control for channel 1,2 */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineMCControl[channel];
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 10, PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/*
* @internal prvCpssDxChFdbManagerHwInitNewDevice function
* @endinternal
*
* @brief   This function do HW initialization from new device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] devNum                 - the device number (cpss devNum)
* @param[in] numOfEports            - number of eports to initialize (according
*                                     to number of eports of the device)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChFdbManagerHwInitNewDevice(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr,
    IN GT_U8                                devNum,
    IN GT_U32                               numOfEports
)
{
    GT_STATUS   rc;
    GT_U32  portNum;
    GT_BOOL tag1VidEnable;
    GT_BOOL srcIdEnable;

    /*
       parameter         |   value                  |   details
     ===========================================================================
     Ports' Learning     |  Disable (on all ePorts) | state Disable automatic learning
     state               |                          | Setting Control learning
     ===========================================================================
     Ports’ SP state     |  Enable (on all ePorts)  | Activating SP mechanism
     ===========================================================================
     AA and TA messages  |  Disable (Global)        | Aged-out and transplant AU messages to CPU.
                         |                          | FDB Mngr. perform Aging not based on AU messages
     ===========================================================================
     FDB CRC multi Hash  |   CRC multi Hash (Global)|
        Mode             |                          |
     ===========================================================================
     FDB FID 16bit       |  Enable (Global)         | Using 16-bits of FID in MAC+FID.
     Hashing             |                          | The 12 bit is relevant to legacy devices –
                         |                          | FDB Manager doesn’t support mix of devices on same instance.
     ===========================================================================
     FDB Actions enable  |  Disable (Global)        | Disable all HW operations on FDB
     ===========================================================================
     FDB Triggering Mode | Triggered (Global)       | Disable Automatic operations on FDB
     ===========================================================================
     VID1 assign in NA   | Enable/Disable internally | Enable or disable VID1 in NA
     messages            | according to entry format |
                         | (w/o VID1) HW global mode |
                         | (disabled by default)     |
     ===========================================================================
     AU messages Rate    | Enabled only when parameter| Enable AU messages rate limiter &
                         | "MAC No-space updates" is  | defining the maximal number of messages
                         | enabled.                   | in time window.
                         | In HW its disabled by      | Note: see throttling learning mechanism
                         | default.                   | section for details
     ===========================================================================
     AAC Channels        | Enable and set the 2 FDB | Note: for accelerated update mode – see entries manipulation section for details .
                         |                          |  these setting will be done prior usage.
                         | multicast channels with  | - Global AAC: enable
                         | predefined settings.     | - AAC iteration: 4(equal number of tiles)
                                                    | - AAC offset: addresses offsets between tiles
                                                    | - AAC multicast: enable o AAC address mode: direct
     ===========================================================================
*/


    /************************/
    /* set implicit values  */
    /************************/
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbInitDone == GT_FALSE)
    {
        rc = cpssDxChBrgFdbInit(devNum);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /*hw write in case of FDB isn't under high availability */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbUnderHa == GT_FALSE)
    {
        for(portNum = 0 ; portNum < numOfEports ; portNum++)
        {
            /* disable the auto learning */
            rc = cpssDxChBrgFdbPortLearnStatusSet(devNum, portNum, GT_FALSE, CPSS_LOCK_FRWRD_E);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* Enable new address message Storm Prevention */
            rc =  cpssDxChBrgFdbNaStormPreventSet(devNum,portNum,GT_TRUE);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        rc = cpssDxChBrgFdbAAandTAToCpuSet(devNum, GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChBrgFdbActionsEnableSet(devNum,GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChBrgFdbMacTriggerModeSet(devNum,CPSS_ACT_TRIG_E);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /*******************************************************/
    /* set explicit values , given during 'manager create' */
    /*******************************************************/
    rc = prvCpssDxChBrgFdbSizeSet(devNum,
         fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(IS_FDB_MANAGER_FOR_SIP_4(fdbManagerPtr))
    {
        rc = cpssDxChBrgFdbHashModeSet(devNum,
                fdbManagerPtr->capacityInfo.hwCapacity.mode);
        if(rc != GT_OK)
        {
            return rc;
        }

    }
    else
    {
        rc = cpssDxChBrgFdb16BitFidHashEnableSet(devNum,
                fdbManagerPtr->cpssHashParams.fid16BitHashEn);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChBrgFdbHashModeSet(devNum,
                CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
        {
            rc = cpssDxChBrgFdbMacEntryMuxingModeSet(devNum,
                    (GT_U32)fdbManagerPtr->entryAttrInfo.macEntryMuxingMode);
            if(rc != GT_OK)
            {
                return rc;
            }

            rc = cpssDxChBrgFdbIpmcEntryMuxingModeSet(devNum,
                    (GT_U32)fdbManagerPtr->entryAttrInfo.ipmcEntryMuxingMode);
            if(rc != GT_OK)
            {
                return rc;
            }

            rc = cpssDxChBrgFdbSaDaCmdDropModeSet(devNum,
                    fdbManagerPtr->entryAttrInfo.saDropCommand == CPSS_PACKET_CMD_DROP_HARD_E ?
                    CPSS_MAC_TABLE_DROP_E : CPSS_MAC_TABLE_SOFT_DROP_E,
                    fdbManagerPtr->entryAttrInfo.daDropCommand == CPSS_PACKET_CMD_DROP_HARD_E ?
                    CPSS_MAC_TABLE_DROP_E : CPSS_MAC_TABLE_SOFT_DROP_E);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            /* for SIP5, SIP5_20 devices in hybrid model, the behavior will be similar as below
            fdbManagerPtr->entryAttrInfo.macEntryMuxingMode  = CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E;
            fdbManagerPtr->entryAttrInfo.ipmcEntryMuxingMode = CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E; */
        }

        /*write in case of FDB is't under high availability */
        if (PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbUnderHa == GT_FALSE)
        {
            rc = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(devNum,
                    fdbManagerPtr->entryAttrInfo.ipNhPacketcommand);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
            {
                rc = cpssDxChIpFdbRoutePrefixLenSet(devNum,
                        CPSS_IP_PROTOCOL_IPV4_E,
                        fdbManagerPtr->lookupInfo.ipv4PrefixLength);
                if(rc != GT_OK)
                {
                    return rc;
                }

                rc = cpssDxChIpFdbRoutePrefixLenSet(devNum,
                        CPSS_IP_PROTOCOL_IPV6_E,
                        fdbManagerPtr->lookupInfo.ipv6PrefixLength);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }

            rc = prvCpssDxChBrgFdbAgeBitDaRefreshEnableSet(devNum,
                    fdbManagerPtr->agingInfo.destinationUcRefreshEnable,
                    fdbManagerPtr->agingInfo.destinationMcRefreshEnable);
            if(rc != GT_OK)
            {
                return rc;
            }

            rc = cpssDxChBrgFdbRoutingUcRefreshEnableSet(devNum,
                    fdbManagerPtr->agingInfo.ipUcRefreshEnable);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        rc = cpssDxChBrgFdbCrcHashUpperBitsModeSet(devNum,
                (GT_U32)fdbManagerPtr->lookupInfo.crcHashUpperBitsMode);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /*hw write in case of FDB is'nt under high availability */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbUnderHa == GT_FALSE)
    {
        rc = cpssDxChBrgFdbRoutedLearningEnableSet(devNum,
            fdbManagerPtr->learningInfo.macRoutedLearningEnable);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* function only applicable for sip5 devices */
        if(fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)
        {
            switch(fdbManagerPtr->entryAttrInfo.entryMuxingMode)
            {
                default:
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E:
                    tag1VidEnable = GT_FALSE;
                    srcIdEnable   = GT_FALSE;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E:
                    tag1VidEnable = GT_FALSE;
                    srcIdEnable   = GT_TRUE;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E:
                    tag1VidEnable = GT_TRUE;
                    srcIdEnable   = GT_FALSE;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E:
                    tag1VidEnable = GT_TRUE;
                    srcIdEnable   = GT_TRUE;
                    break;
            }
            rc = cpssDxChBrgFdbVid1AssignmentEnableSet(devNum, tag1VidEnable);
            if(rc != GT_OK)
            {
                return rc;
            }
            rc = cpssDxChBrgFdbMaxLengthSrcIdEnableSet(devNum, srcIdEnable);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        else if(fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E)
        {
            /* SIP6 Device in Hybrid shadow table */
            fdbManagerPtr->entryAttrInfo.entryMuxingMode = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E;
        }
    }

    rc = cpssDxChBrgFdbMacVlanLookupModeSet(devNum,
        (fdbManagerPtr->learningInfo.macVlanLookupMode ==
        CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_ONLY_E) ?
            CPSS_SVL_E : CPSS_IVL_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* enable/disable AU rate limit , chain too long */
    fdbManagerPtr->currentThresholdType =
        fdbManagerPtr->learningInfo.macNoSpaceUpdatesEnable ?
            PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_A_E :
            PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_NOT_VALID_E;
    /*hw write in case of FDB isn't under high availability */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbUnderHa == GT_FALSE)
    {
        rc = prvCpssDxChFdbManagerLearningThrottlingSet(devNum, fdbManagerPtr->currentThresholdType);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /*skip hw write in case of FDB under high availability */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbUnderHa == GT_TRUE)
    {
        return GT_OK;
    }

    return prvCpssDxChFdbManagerFdbAccParamSet(devNum);
}

/**
* @internal prvCpssDxChFdbManagerHwWriteEntriesFromDbToHwOfNewDevices function
* @endinternal
*
* @brief   This function start to write entries from manager to HW of the new devices
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] devListArr[]          - the array of device ids to add to the FDB Manager.
* @param[in] numOfDevs             - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChFdbManagerHwWriteEntriesFromDbToHwOfNewDevices(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr,
    IN GT_U8  devListArr[], /*arrSizeVarName=numOfDevs*/
    IN GT_U32 numOfDevs
)
{
    GT_STATUS                                               rc;
    GT_U32                                                  ii;
    GT_U8                                                   devNum;
    GT_U32                                                  hwIndex;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC          *dbEntryPtr;
    GT_BOOL                                                 getFirst = GT_TRUE;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC        calcInfo;
    GT_HW_DEV_NUM                                           hwDevNum;

    cpssOsMemSet(&calcInfo, 0, sizeof(calcInfo));

    /*skip hw write in case of FDB under high availability */
    if (PRV_CPSS_DXCH_PP_MAC(devListArr[0])->bridge.fdbUnderHa == GT_TRUE)
    {
        return GT_OK;
    }

    /* loop on all entries in the FDB manager */
    while(GT_OK ==
          prvCpssDxChFdbManagerDbFdbEntryIteratorGetNext(fdbManagerPtr,getFirst,&dbEntryPtr))
    {
        getFirst = GT_FALSE;/* get next from here on ... */

        calcInfo.dbEntryPtr = dbEntryPtr;
        if(dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E)
        {
            /** IpV6 UC entry comprises of two types of entries - key and data and resides subsequently in DB and HW,
             *  so the order of writing/reading to/from DB/HW should be subsequent as well.
             */
            rc = prvCpssDxChFdbManagerDbFdbEntryIteratorGetNext(fdbManagerPtr, getFirst, &dbEntryPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            calcInfo.dbEntryExtPtr = dbEntryPtr;
            rc = prvCpssDxChFdbManagerHwBuildIpV6UcHwFormatFromDbEntry(fdbManagerPtr, calcInfo.dbEntryPtr, calcInfo.dbEntryExtPtr);
        }
        else
        {
            /* build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords */
            rc = prvCpssDxChFdbManagerHwBuildHwFormatFromDbEntry(fdbManagerPtr, dbEntryPtr);
        }
        if(rc != GT_OK)
        {
            return rc;
        }

        hwIndex = calcInfo.dbEntryPtr->hwIndex;
        calcInfo.selectedBankId  = hwIndex % fdbManagerPtr->numOfBanks;
        for(ii = 0 ; ii < numOfDevs ; ii++)
        {
            /* write the same format to all devices */
            devNum = devListArr[ii];

            if (calcInfo.dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E)
            {
                /* per device : write to HW */
                rc = prvCpssDxChFdbManagerHwWriteIpV6UcByHwIndex_perDevNum(fdbManagerPtr, devNum, &calcInfo);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            else
            {
                if(calcInfo.dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E)
                {
                    /* we lock the CPSS DB ... to be able to access 'PRV_CPSS_PP_MAC(devNum)' */
                    LOCK_DEV_NUM(devNum);
                    hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);
                    UNLOCK_DEV_NUM(devNum);/* we are done with this device internal DB */

                    /* check if the entry registered on 'own hwDevNum' and increment
                       it's current counter */
                    rc = prvCpssDxChFdbManagerDbLimitPerHwDevNumPerPort(fdbManagerPtr,
                        LIMIT_OPER_ADD_NEW_E,hwDevNum,
                        &calcInfo.dbEntryPtr->specificFormat.prvMacEntryFormat);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }

                /* per device : write to HW */
                rc = prvCpssDxChFdbManagerHwWriteByHwIndex_perDevNum(fdbManagerPtr, devNum, &calcInfo);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}

/*
* @internal prvCpssDxChFdbManagerHwFlushAll function
* @endinternal
*
* @brief   This function Flush all entries from the HW from all registered devices in the manager.
*          NOTE this function not update the DB itself ,the Caller responsible for it.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
*
* @retval GT_OK                    - on success
* @retval other                    - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChFdbManagerHwFlushAll
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr;
    GT_BOOL     getFirst = GT_TRUE;

    /* check if there are registered devices on the manager */
    if(!prvCpssDxChFdbManagerDbIsAnyDeviceRegistred(fdbManagerPtr))
    {
        /* no device to flush from */
        return GT_OK;
    }

    /* loop on all entries in the FDB manager */
    while(GT_OK ==
          prvCpssDxChFdbManagerDbFdbEntryIteratorGetNext(fdbManagerPtr,getFirst,&dbEntryPtr))
    {
        getFirst = GT_FALSE;/* get next from here on ... */

        /********************************************************************/
        /* flush the entry from HW of all registered devices                */
        /********************************************************************/
        rc = prvCpssDxChFdbManagerHwFlushByHwIndex(fdbManagerPtr,
            dbEntryPtr->hwIndex/*hwIndex*/,
            dbEntryPtr->hwIndex % fdbManagerPtr->numOfBanks,/* bankId */
            GT_TRUE);/* we update the bank counters in HW */
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/*
* @internal prvCpssDxChFdbManagerHwAgedOutVerify function
* @endinternal
*
* @brief   The function verifies the DB entry is aged-out not not & update the status in agedOut Out param.
*             Not aged-out - Reset the age-bit of input entry in all the devices as part of aging process.
*             Aged-out     - No updates are required.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[in] dbEntryPtr            - (pointer to) the DB entry
* @param[out] agedOutPtr           - (pointer to) the aged-out status.
*                                    GT_TRUE  - Entry is aged-out
*                                    GT_FALSE - Entry is not aged-out
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChFdbManagerHwAgedOutVerify
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC           *dbEntryPtr,
    OUT GT_BOOL                                                 *agedOutPtr
)
{
    GT_STATUS                                        rc;
    GT_BOOL                                          isFirst = GT_TRUE;
    GT_U32                                           hwValue;
    GT_U8                                            devNum;
    GT_U32                                           portGroupId; /* the port group Id */
    GT_U32                                           tileId;
    GT_U32                                           numOfTiles;
    GT_U32                                           hwData[FDB_ENTRY_WORDS_SIZE_CNS]; /* use SIP_5 size for common code */

    /* check if there are registered devices on the manager */
    *agedOutPtr = GT_TRUE;
    rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, isFirst, &devNum);
    isFirst = GT_FALSE;

    while(rc == GT_OK)
    {
        LOCK_DEV_NUM(devNum);

        /* Use AAC method to access data from HW */
        if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles > 1)
        {
            /* Keep checking age-bit refresh in all device - Till the first device detected */
            if(*agedOutPtr == GT_TRUE)
            {
                rc = prvCpssDxChFdbManagerAacHwAgeBitCheck(devNum, dbEntryPtr->hwIndex, agedOutPtr, &hwData[0]);
                if(rc != GT_OK)
                {
                    UNLOCK_DEV_NUM(devNum);
                    return rc;
                }
            }

            /* clear the age-bit for all devices - from the age-bit refresh detected device till end */
            if(*agedOutPtr == GT_FALSE)
            {
                rc = prvCpssDxChFdbManagerAacHwWriteEntry(devNum, dbEntryPtr->hwIndex, &hwData[0]);
                if(rc != GT_OK)
                {
                    UNLOCK_DEV_NUM(devNum);
                    return rc;
                }
            }
        }
        else
        {
            numOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles?PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles : 1;
            /* loop on all the tiles in the device */
            for(tileId = 0; tileId < numOfTiles; tileId++)
            {
                portGroupId = tileId * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;

                /* Keep checking age-bit refresh in all device - Till the first device detected */
                if(*agedOutPtr == GT_TRUE)
                {
                    /* per device : Check age bit */
                    rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                            portGroupId,
                            CPSS_DXCH_TABLE_FDB_E,
                            dbEntryPtr->hwIndex,
                            &hwData[0]);
                    if(rc != GT_OK)
                    {
                        UNLOCK_DEV_NUM(devNum);
                        return rc;
                    }
                    /* Get Age bit value */
                    hwValue = U32_GET_FIELD_MAC(hwData[0], 2, 1);
                    if(hwValue == 1)
                    {
                        *agedOutPtr = GT_FALSE;
                        /* Reset age bit */
                        U32_SET_FIELD_MAC(hwData[0], 2, 1, 0);
                    }
                }

                /* clear the age-bit for all devices - from the age-bit refresh detected device till end */
                if(*agedOutPtr == GT_FALSE)
                {
                    rc = prvCpssDxChPortGroupWriteTableEntry(devNum,
                            portGroupId,
                            CPSS_DXCH_TABLE_FDB_E,
                            dbEntryPtr->hwIndex,
                            &hwData[0]);
                    if(rc != GT_OK)
                    {
                        UNLOCK_DEV_NUM(devNum);
                        return rc;
                    }
                }
            }
        }

        UNLOCK_DEV_NUM(devNum);
        rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr,isFirst,&devNum);
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChFdbManagerMatchHwEntryWithDbEntry function
* @endinternal
*
* @brief   The function build the HW entry format from sw entry(dbEntryPtr) and compares with input hwdata.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[in] hwData                - HW Entry
* @param[in] hwDataExt             - HW Entry (IPv6 Data part)
* @param[in] dbEntryPtr            - (pointer to) the DB entry
* @param[in] dbEntryExtPtr         - (pointer to) the Data part of DB entry in case of IPv6
* @param[in] hwFormatType          - FDB Entry hw format type
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note
*   NONE
*/
static GT_STATUS prvCpssDxChFdbManagerMatchHwEntryWithDbEntry
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC    *fdbManagerPtr,
    IN GT_U8                                              devNum,
    IN GT_U32                                             hwData[],
    IN GT_U32                                             hwDataExt[],
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC     *dbEntryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC     *dbEntryExtPtr,
    IN PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT   hwFormatType
)
{
    GT_STATUS                                       rc;
    GT_U32                                          hwValue_dummy = 0;
    GT_U32                                          *hwDataArr;
    GT_U32                                          *hwDataExtArr;
    GT_U32                                           compareSize;

    /* build the HW format at : fdbManagerPtr->tempInfo.sip6_hwFormatWords (SW entry in HW format)
     * hwData[] - contains HW entry */
    if(dbEntryExtPtr != NULL)
    {
        rc = prvCpssDxChFdbManagerHwBuildIpV6UcHwFormatFromDbEntry(fdbManagerPtr, dbEntryPtr, dbEntryExtPtr);
    }
    else
    {
        rc = prvCpssDxChFdbManagerHwBuildHwFormatFromDbEntry(fdbManagerPtr, dbEntryPtr);
    }


    if(rc != GT_OK)
    {
        return rc;
    }

    switch(hwFormatType)
    {
        default:
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP6_E:
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP4_E:
            hwDataArr    = &fdbManagerPtr->tempInfo.sip6_hwFormatWords[0];
            hwDataExtArr = &fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords[0];
            compareSize  = SIP6_FDB_ENTRY_WORDS_SIZE_CNS*4;
            break;
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_20_E:
            hwDataArr    = &fdbManagerPtr->tempInfo.sip5_20_hwFormatWords[0];
            hwDataExtArr = &fdbManagerPtr->tempInfo.sip5_20_ipv6Ext_hwFormatWords[0];
            compareSize  = FDB_ENTRY_WORDS_SIZE_CNS*4;
            break;
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_E:
            hwDataArr    = &fdbManagerPtr->tempInfo.sip5_hwFormatWords[0];
            hwDataExtArr = &fdbManagerPtr->tempInfo.sip5_ipv6Ext_hwFormatWords[0];
            compareSize  = FDB_ENTRY_WORDS_SIZE_CNS*4;
            break;
    }

    /* Ignore the Age Bit, before comparing the entry */
    U32_SET_FIELD_MAC(hwDataArr[0], 2, 1, hwValue_dummy);
    U32_SET_FIELD_MAC(hwData[0], 2, 1, hwValue_dummy);

    if(dbEntryExtPtr != NULL)
    {
        U32_SET_FIELD_MAC(hwDataExtArr[0], 2, 1, hwValue_dummy);
        U32_SET_FIELD_MAC(hwDataExt[0], 2, 1, hwValue_dummy);

        /*check if need to update the hwDevNum to be 'ownDevNum'*/
        /* this to allow comparing the HW entry that hold 'ownDevNum' for global eports */
        rc = checkForOwnHwDevUpdate(fdbManagerPtr,devNum,dbEntryExtPtr,GT_TRUE/*ipv6*/);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(cpssOsMemCmp(hwDataExtArr, hwDataExt, compareSize) != 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

    }
    else
    {
        /*check if need to update the hwDevNum to be 'ownDevNum'*/
        /* this to allow comparing the HW entry that hold 'ownDevNum' for global eports */
        rc = checkForOwnHwDevUpdate(fdbManagerPtr,devNum,dbEntryPtr,GT_FALSE/*non-ipv6*/);
        if(rc != GT_OK)
        {
            return rc;
        }
    }


    if(dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E)
    {
        /* Ignore the SP Bit, before comparing the entry */
        switch(hwFormatType)
        {
            case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP4_E:
                /* SP Bit for AC5 */
                U32_SET_FIELD_MAC(hwDataArr[3], 2, 1, hwValue_dummy);
                U32_SET_FIELD_MAC(hwData[3], 2, 1, hwValue_dummy);
                break;
            case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_E:
            case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_20_E:
                SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                        SIP5_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E, hwValue_dummy);
                SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwData,
                        SIP5_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E, hwValue_dummy);
                break;
            case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP6_E:
                SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                        SIP6_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E, hwValue_dummy);
                SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwData,
                        SIP6_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E, hwValue_dummy);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(fdbManagerPtr->entryAttrInfo.shadowType);
        }
    }

    if((cpssOsMemCmp(hwDataArr, hwData, compareSize)) != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChBrgFdbManagerHwEntryMatchWithSwEntry_allDevice function
* @endinternal
*
* @brief   The function gets the HW entry from all device for dbEntryPtr and matches the content.
*          depending on the listType.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[in] hwIndex               - HW index.
* @param[in] dbEntryPtr            - (pointer to) the DB entry
* @param[in] listType              - DB check list type need to be validated
* @param[out] resultPtr            - (pointer to) test result.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*/
GT_STATUS prvCpssDxChBrgFdbManagerHwEntryMatchWithSwEntry_allDevice
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC    *fdbManagerPtr,
    IN GT_U32                                             hwIndex,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC     *dbEntryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_CHECK_LIST_TYPE_ENT   listType,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT   *resultPtr
)
{
    GT_STATUS                                               rc;
    GT_U32                                                  tileId;
    GT_U32                                                  numOfTiles;
    GT_U32                                                  hwData[FDB_ENTRY_WORDS_SIZE_CNS]; /* use SIP_5 size for common code */
    GT_U32                                                  hwDataExt[FDB_ENTRY_WORDS_SIZE_CNS]; /* use SIP_5 size for common code */
    GT_U8                                                   devNum;
    GT_U32                                                  portGroupId;
    GT_U32                                                  hwValue;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC          *dbEntryExtPtr = NULL;
    GT_U32                                                  dbIndexExt;
    GT_U32                                                  hwValue_spUnknown;
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT        hwFormatType;

    cpssOsMemSet(hwData, 0, sizeof(hwData));

    if((dbEntryPtr != NULL) && (dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E))
    {
        dbIndexExt      = fdbManagerPtr->indexArr[hwIndex+1].entryPointer;
        dbEntryExtPtr   = &fdbManagerPtr->entryPoolPtr[dbIndexExt];
    }
    rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_TRUE, &devNum);
    while(rc == GT_OK)
    {
        rc = prvCpssDxChFdbManagerDeviceHwFormatGet(fdbManagerPtr, devNum, &hwFormatType);
        if(rc != GT_OK)
        {
            return rc;
        }

        LOCK_DEV_NUM(devNum);
        numOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles?PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles : 1;
        /* loop on all the tiles in the device */
        for(tileId = 0; tileId < numOfTiles; tileId++)
        {
            portGroupId = tileId * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;

            /* per device : get the entry */
            rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                    portGroupId,
                    CPSS_DXCH_TABLE_FDB_E,
                    hwIndex,
                    &hwData[0]);
            if(rc != GT_OK)
            {
                UNLOCK_DEV_NUM(devNum);
                return rc;
            }

            /* IPv6 Entry - Get the data part */
            if(dbEntryExtPtr != NULL)
            {
                /* per device : get the entry */
                rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                        portGroupId,
                        CPSS_DXCH_TABLE_FDB_E,
                        hwIndex+1,
                        &hwDataExt[0]);
                if(rc != GT_OK)
                {
                    UNLOCK_DEV_NUM(devNum);
                    return rc;
                }
            }

            if(listType == PRV_CPSS_DXCH_FDB_MANAGER_DB_CHECK_USED_LIST_E)
            {
                if(dbEntryPtr == NULL)
                {
                    UNLOCK_DEV_NUM(devNum);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                rc = prvCpssDxChFdbManagerMatchHwEntryWithDbEntry(fdbManagerPtr, devNum , hwData,
                        hwDataExt, dbEntryPtr, dbEntryExtPtr, hwFormatType);
                if(rc != GT_OK)
                {
                    *resultPtr = CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E;
                    UNLOCK_DEV_NUM(devNum);
                    return GT_OK;
                }
            }
            else
            {
                /* A valid entry(may be SP) exist, in place of Free - Error */
                switch(hwFormatType)
                {
                    case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP4_E:
                        /* check 2 bits : valid(bit0)+skip(bit1) .
                           if value == 0 --> not valid              --> not used entry
                           if value == 1 --> valid (not skipped)    --> used entry
                           if value == 2 --> not valid              --> not used entry
                           if value == 3 --> valid but skipped      --> not used entry

                           so only value of '1' means that the entry considered 'used'
                           */
                        hwValue           = U32_GET_FIELD_MAC(hwData[0], 0, 2);
                        hwValue_spUnknown = U32_GET_FIELD_MAC(hwData[3], 2, 1);
                        break;
                    case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_E:
                    case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_20_E:
                        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwData, SIP5_FDB_FDB_TABLE_FIELDS_VALID_E, hwValue);
                        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwData, SIP5_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E, hwValue_spUnknown);
                        break;
                    case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP6_E:
                        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwData, SIP6_FDB_FDB_TABLE_FIELDS_VALID_E, hwValue);
                        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwData, SIP6_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E, hwValue_spUnknown);
                        break;
                    default:
                        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(fdbManagerPtr->entryAttrInfo.shadowType);
                }

                if(hwValue == 1) /* used Entry */
                {
                    if(hwValue_spUnknown == 0) /* Non-Sp entry */
                    { /* Non-Sp entry */
                        *resultPtr = CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E;
                        UNLOCK_DEV_NUM(devNum);
                        return GT_OK;
                    }
                    else if((hwValue_spUnknown == 1) && (listType == PRV_CPSS_DXCH_FDB_MANAGER_DB_CHECK_FREE_LIST_SP_AS_USED_E))
                    { /* SP Entry, and SP entry is considered as a problematic */
                        *resultPtr = CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E;
                        UNLOCK_DEV_NUM(devNum);
                        return GT_OK;
                    }
                }
            }
        }
        UNLOCK_DEV_NUM(devNum);
        rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_FALSE, &devNum);
    }
    if(rc == GT_NO_MORE)
    {
        rc = GT_OK;
    }
    return rc;
}

/*
* @internal prvCpssDxChFdbManagerDebugSelfTest_SpEntry function
* @endinternal
*
* @brief   The function Injects/corrects error relates to HW SP entry.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[in] errorEnable           - Status value indicates error/correct state
*                                    GT_TRUE  - Error need to injected(Create a SP entry in place of free index)
*                                    GT_FALSE - Error need to be corrected(Replace the SP entry with in valid entry)
* @param[in,out] hwIndexPtr        - (pointer to) the hwIndex
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*/
GT_STATUS prvCpssDxChFdbManagerDebugSelfTest_SpEntry
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC    *fdbManagerPtr,
    IN GT_BOOL                                            errorEnable,
    INOUT GT_U32                                         *hwIndexPtr
)
{
    GT_STATUS                                           rc;
    GT_U32                                              tmpCnt;
    GT_U32                                              entryCnt = 0;
    GT_BOOL                                             entryFound = GT_FALSE;
    GT_U32                                              dbIndex;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC      *dbEntryPtr;
    GT_U8                                               devNum;
    GT_U32                                              hwIndex;
    GT_U32                                             *hwDataArr;
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT    hwFormatType;

    if(!errorEnable)
    {
        hwIndex = *hwIndexPtr;
        return prvCpssDxChFdbManagerHwFlushByHwIndex(fdbManagerPtr,
                hwIndex,
                hwIndex % fdbManagerPtr->numOfBanks,
                GT_FALSE/* Do not update counter */);
    }

    /* Get a sample entry from used list
     * Change entry (SP flag Set)
     * Write to HW in place of a free entry
     */
    dbIndex     = fdbManagerPtr->headOfUsedList.entryPointer;
    dbEntryPtr  = &fdbManagerPtr->entryPoolPtr[dbIndex];
    for(tmpCnt = 0; fdbManagerPtr->capacityInfo.maxTotalEntries; tmpCnt++)
    {
        dbEntryPtr  = &fdbManagerPtr->entryPoolPtr[dbIndex];
        GET_nextEntryPointer_MAC(dbEntryPtr, dbIndex);
        /* break on 2nd mac entry */
        if(dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E)
        {
            entryCnt+=1;
            if(entryCnt == 2)
            {
                entryFound = GT_TRUE;
                break;
            }
        }
    }
    if(!entryFound)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Build HW format from the sample entry */
    rc = prvCpssDxChFdbManagerHwBuildHwFormatFromDbEntry(fdbManagerPtr, dbEntryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get the first device to write the entry */
    rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_TRUE, &devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChFdbManagerDeviceHwFormatGet(fdbManagerPtr, devNum, &hwFormatType);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(hwFormatType)
    {
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP4_E:
            hwDataArr = fdbManagerPtr->tempInfo.sip6_hwFormatWords;
            /* AC5 - Set spUnknown */
            U32_SET_FIELD_MAC(hwDataArr[3], 2, 1, 1);
            break;
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_E:
            hwDataArr = fdbManagerPtr->tempInfo.sip5_hwFormatWords;
            SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E, 1);
            break;
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_20_E:
            hwDataArr = fdbManagerPtr->tempInfo.sip5_20_hwFormatWords;
            SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E, 1);
            break;
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP6_E:
            hwDataArr = fdbManagerPtr->tempInfo.sip6_hwFormatWords;
            /* Set spUnknown - NA storm prevent entry */
            SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E, 1);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(fdbManagerPtr->entryAttrInfo.shadowType);
    }

    /* Get hwIndex of first free entry */
    for(hwIndex = 0 ; hwIndex < fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes; hwIndex++)
    {
        dbIndex = fdbManagerPtr->indexArr[hwIndex].entryPointer;
        if(!fdbManagerPtr->indexArr[hwIndex].isValid_entryPointer)
        {
            *hwIndexPtr = hwIndex;  /* Update for restore call */
            break;
        }
    }

    /* protect access to array out of range */
    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex,fdbManagerPtr->capacityInfo.maxTotalEntries);

    /* Write the entry(SP/Invalid) to a free index */
    return internal_writeFdbTableEntry(devNum,
            hwIndex,
            hwDataArr,
            hwIndex % fdbManagerPtr->numOfBanks,
            UPDATE_HW_BANK_COUNTER_ACTION_NONE_E);
}


/*
* @internal internal_prvCpssDxChBrgFdbManagerPortGroupBankCounterValueGet function
* @endinternal
*
* @brief   The function Get the value of counter of the specific FDB table bank and portGroup.
*
* @param[in] devNum                - device number
* @param[in] portGroupId           - the portGroupId. to support multi-port-groups device-port-groups device
* @param[in] bankIndex             - the index of the bank
*                                    (APPLICABLE RANGES: 0..15)
* @param[out] valuePtr             - (pointer to)the value of the counter.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_prvCpssDxChBrgFdbManagerPortGroupBankCounterValueGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          bankIndex,
    OUT GT_U32          *valuePtr
)
{
    GT_U32      regAddr;

    /* Input Validation */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);
    PRV_CPSS_DXCH_FDB_BANK_INDEX_BAD_PARAM_CHECK_MAC(devNum, bankIndex);
    if(portGroupId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PORT_GROUP_ID_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        FDB.FDBCore.FDBBankCntrs.FDBBankCntr[bankIndex];

    /* Get counter value */
    return prvCpssHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, valuePtr);
}

/*
* @internal prvCpssDxChBrgFdbManagerPortGroupBankCounterValueGet function
* @endinternal
*
* @brief   The function Get the value of counter of the specific FDB table bank and portGroup.
*
* @param[in] devNum                - device number
* @param[in] portGroupId           - the portGroupId. to support multi-port-groups device-port-groups device
* @param[in] bankIndex             - the index of the bank
*                                    (APPLICABLE RANGES: 0..15)
* @param[out] valuePtr             - (pointer to)the value of the counter.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChBrgFdbManagerPortGroupBankCounterValueGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          bankIndex,
    OUT GT_U32          *valuePtr
)
{
    GT_STATUS   rc;

    LOCK_DEV_NUM(devNum);

    rc = internal_prvCpssDxChBrgFdbManagerPortGroupBankCounterValueGet(devNum, portGroupId, bankIndex, valuePtr);

    UNLOCK_DEV_NUM(devNum);
    return rc;
}


/*
* @internal internal_prvCpssDxChBrgFdbManagerPortGroupBankCounterUpdate function
* @endinternal
*
* @brief  This function Update (increment/decrement) the counter of the specific bank.
*         The function will fail (GT_BAD_STATE) if the PP is busy with previous update.
*
*         the application can determine if PP is ready by calling
*         cpssDxChBrgFdbBankCounterUpdateStatusGet.
*         relevant to CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E
*           NOTE: this function is needed to be called after calling one of the next:
*         'write by index'      - cpssDxChBrgFdbMacEntryWrite ,
*               cpssDxChBrgFdbPortGroupMacEntryWrite
*         'invalidate by index' - cpssDxChBrgFdbMacEntryInvalidate ,
*               cpssDxChBrgFdbPortGroupMacEntryInvalidate
*
*         The application logic should be:
*             if last action was 'write by index' then :
*               - if the previous entry (in the index) was valid      -- do no call this function.
*               - if the previous entry (in the index) was not valid  -- do 'increment'.
*             if last action was 'invalidate by index' then :
*               - if the previous entry (in the index) was valid      -- do 'decrement'.
*               - if the previous entry (in the index) was not valid  -- do no call this function.
*
* @param[in] devNum                - device number
* @param[in] portGroupId           - the portGroupId. to support multi-port-groups device-port-groups device
* @param[in] bankIndex             - the index of the bank
*                                    (APPLICABLE RANGES: 0..15)
* @param[in] incOrDec              - increment or decrement the counter by one.
*                                    GT_TRUE - increment the counter
*                                    GT_FALSE - decrement the counter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - bankIndex > 15
* @retval GT_BAD_STATE             - the PP is not ready to get a new update from CPU.
*                                       (PP still busy with previous update)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_prvCpssDxChBrgFdbManagerPortGroupBankCounterUpdate
(
    IN GT_U8            devNum,
    IN GT_U32           portGroupId,
    IN GT_U32           bankIndex,
    IN GT_BOOL          incOrDec
)
{
    GT_STATUS           rc;
    GT_U32              hwValue;
    GT_U32              regAddr;

    /* Input Validation */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_FDB_BANK_INDEX_BAD_PARAM_CHECK_MAC(devNum, bankIndex);
    if(portGroupId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PORT_GROUP_ID_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* check that the device is ready */
    regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBCntrsUpdate.FDBCntrsUpdateCtrl;
    rc = prvCpssPortGroupBusyWait(devNum, portGroupId, regAddr, 0,
            GT_FALSE);/* using 'busy wait' */
    if (rc != GT_OK)
    {
        return rc;
    }

    /* FDB Counters Update Trigger */
    hwValue = 1;
    if(incOrDec == GT_TRUE)
    {
        /* FDB Counters Inc */
        hwValue |= BIT_1;
    }

    /* FDB Counters Bank Update */
    hwValue |= bankIndex << 3;

    return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 0, 7, hwValue);
}

/*
* @internal prvCpssDxChBrgFdbManagerPortGroupBankCounterUpdate function
* @endinternal
*
* @brief  This function Update (increment/decrement) the counter of the specific bank.
*         The function will fail (GT_BAD_STATE) if the PP is busy with previous update.
*
*         the application can determine if PP is ready by calling
*         cpssDxChBrgFdbBankCounterUpdateStatusGet.
*         relevant to CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E
*           NOTE: this function is needed to be called after calling one of the next:
*         'write by index'      - cpssDxChBrgFdbMacEntryWrite ,
*               cpssDxChBrgFdbPortGroupMacEntryWrite
*         'invalidate by index' - cpssDxChBrgFdbMacEntryInvalidate ,
*               cpssDxChBrgFdbPortGroupMacEntryInvalidate
*
*         The application logic should be:
*             if last action was 'write by index' then :
*               - if the previous entry (in the index) was valid      -- do no call this function.
*               - if the previous entry (in the index) was not valid  -- do 'increment'.
*             if last action was 'invalidate by index' then :
*               - if the previous entry (in the index) was valid      -- do 'decrement'.
*               - if the previous entry (in the index) was not valid  -- do no call this function.
*
* @param[in] devNum                - device number
* @param[in] portGroupId           - the portGroupId. to support multi-port-groups device-port-groups device
* @param[in] bankIndex             - the index of the bank
*                                    (APPLICABLE RANGES: 0..15)
* @param[in] incOrDec              - increment or decrement the counter by one.
*                                    GT_TRUE - increment the counter
*                                    GT_FALSE - decrement the counter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - bankIndex > 15
* @retval GT_BAD_STATE             - the PP is not ready to get a new update from CPU.
*                                       (PP still busy with previous update)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBrgFdbManagerPortGroupBankCounterUpdate
(
    IN GT_U8            devNum,
    IN GT_U32           portGroupId,
    IN GT_U32           bankIndex,
    IN GT_BOOL          incOrDec
)
{
    GT_STATUS   rc;

    LOCK_DEV_NUM(devNum);

    rc = internal_prvCpssDxChBrgFdbManagerPortGroupBankCounterUpdate(devNum, portGroupId, bankIndex, incOrDec);

    UNLOCK_DEV_NUM(devNum);
    return rc;
}

/*
* @internal prvCpssDxChFdbManagerHwCounterSyncUpWithDb function
* @endinternal
*
* @brief   The function Updates HW per bank counter, according to the current DB counter.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChFdbManagerHwCounterSyncUpWithDb
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr
)
{

#ifdef PRV_CPSS_DXCH_BRG_FDB_MANAGER_ALLOW_HW_BANK_COUNTER_UPDATE_CNS
    GT_STATUS       rc;
    GT_U8           devNum;
    GT_U32          regAddr;
    GT_U32          bankIndex;
    GT_U32          portGroupId;
    GT_U32          numOfTiles;
    GT_U32          tileId;

    /* Set per bank FDB counter, to all device */
    rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_TRUE, &devNum);
    while(rc == GT_OK)
    {
        LOCK_DEV_NUM(devNum);
        numOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles?PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles : 1;
        /* loop on all the tiles in the device */
        for(tileId = 0; tileId < numOfTiles; tileId++)
        {
            portGroupId = tileId * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;
            for(bankIndex = 0; bankIndex<CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_BANKS_CNS; bankIndex++)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                    FDB.FDBCore.FDBBankCntrs.FDBBankCntr[bankIndex];

                /* Set counter value */
                rc = prvCpssHwPpPortGroupWriteRegister(devNum,
                        portGroupId,
                        regAddr,
                        fdbManagerPtr->bankInfoArr[bankIndex].bankPopulation);
                if(rc != GT_OK)
                {
                    UNLOCK_DEV_NUM(devNum);
                    return rc;
                }
            }
        }
        UNLOCK_DEV_NUM(devNum);
        rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_FALSE, &devNum);
    }
#else
    GT_UNUSED_PARAM(fdbManagerPtr);
#endif

    return GT_OK;
}


/*
* @internal prvCpssDxChFdbManagerHwDeviceConfigUpdate function
* @endinternal
*
* @brief   This function do HW updates on global parameters for existing device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] devNum                 - the device number (cpss devNum)
* @param[in] entryAttrPtr          - (pointer to) parameters to override entryAttrPtr
*                                       that was given during 'manager create' API.
*                                     NOTE: ignored if NULL
* @param[in] learningPtr           - (pointer to) parameters to override learningPtr
*                                       that was given during 'manager create' API.
*                                     NOTE: ignored if NULL
* @param[in] agingPtr              - (pointer to) parameters to override agingPtr
*                                       that was given during 'manager create' API.
*                                     NOTE: ignored if NULL
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChFdbManagerHwDeviceConfigUpdate(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr,
    IN GT_U8                                devNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC   *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC           *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC              *agingPtr
)
{
    GT_STATUS   rc;

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbUnderHa == GT_TRUE)
    {
        /* no more to do , as we not update the HW */
        return GT_OK;
    }

    if(fdbManagerPtr->entryAttrInfo.shadowType != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E)
    {
        if(entryAttrPtr)
        {
            if(fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
            {
                rc = cpssDxChBrgFdbSaDaCmdDropModeSet(devNum,
                        fdbManagerPtr->entryAttrInfo.saDropCommand == CPSS_PACKET_CMD_DROP_HARD_E ?
                        CPSS_MAC_TABLE_DROP_E : CPSS_MAC_TABLE_SOFT_DROP_E,
                        fdbManagerPtr->entryAttrInfo.daDropCommand == CPSS_PACKET_CMD_DROP_HARD_E ?
                        CPSS_MAC_TABLE_DROP_E : CPSS_MAC_TABLE_SOFT_DROP_E);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }

            rc = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(devNum,
                        fdbManagerPtr->entryAttrInfo.ipNhPacketcommand);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        if(agingPtr)
        {
            rc = prvCpssDxChBrgFdbAgeBitDaRefreshEnableSet(devNum,
                        fdbManagerPtr->agingInfo.destinationUcRefreshEnable,
                        fdbManagerPtr->agingInfo.destinationMcRefreshEnable);
            if(rc != GT_OK)
            {
                return rc;
            }

            rc = cpssDxChBrgFdbRoutingUcRefreshEnableSet(devNum,
                        fdbManagerPtr->agingInfo.ipUcRefreshEnable);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if(learningPtr)
    {
        rc = cpssDxChBrgFdbRoutedLearningEnableSet(devNum,
                fdbManagerPtr->learningInfo.macRoutedLearningEnable);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

