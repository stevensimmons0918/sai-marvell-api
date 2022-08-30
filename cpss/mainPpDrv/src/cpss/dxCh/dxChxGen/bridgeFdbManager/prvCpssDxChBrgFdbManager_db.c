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
* @file cpssDxChBrgFdbManager_db.c
*
* @brief FDB manager support - manipulations of 'manager DB'
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/cpssDxChBrgFdbManager.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_utils.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_db.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_hw.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_debug.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* #define FDB_MANAGER_REHASHING_DEBUG */

void prvCpssDxChFdbManagerDebug1PrintEnableSet(GT_U32 enable)
{
    PRV_SHARED_FDB_MANAGER_DB_VAR_GET(debug1Print) = (enable) ? GT_TRUE : GT_FALSE;
}

/* macro to allocate memory (associated with the manager)
  and memset to ZERO , into pointer according to needed 'size in bytes'*/
#define MEM_CALLOC_MAC(_fdbManagerId,_pointer,_sizeInBytes)       \
    {                                                             \
        _pointer = cpssOsMalloc(_sizeInBytes);                \
        if(NULL == _pointer)                    \
        {                                       \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "manager[%d] [%s] failed to allocate memory [%d] bytes ", \
                _fdbManagerId,#_pointer,_sizeInBytes);   \
        }                                       \
                                                \
        cpssOsMemSet(_pointer,0,(_sizeInBytes));\
                                                \
        PRV_SHARED_FDB_MANAGER_DB_VAR_GET(prvCpssDxChFdbManagerDbArr)[_fdbManagerId]->numOfAllocatedPointers++;        \
        /* cpssOsPrintf("alloc [%s][%p] size[%d] fdbManagerId[%d] numOfAllocatedPointers[%d]\n",#_pointer,_pointer,_sizeInBytes, _fdbManagerId, prvCpssDxChFdbManagerDbArr[_fdbManagerId]->numOfAllocatedPointers); */ \
    }

/* macro to free memory (associated with the manager) */
#define MEM_FREE_MAC(_fdbManagerId,_pointer)        \
    if(_pointer)                                    \
    {                                               \
        /*cpssOsPrintf("free [%s][%p] fdbManagerId[%d] numOfAllocatedPointers[%d]\n",#_pointer,_pointer, _fdbManagerId, prvCpssDxChFdbManagerDbArr[_fdbManagerId]->numOfAllocatedPointers); */ \
        PRV_SHARED_FDB_MANAGER_DB_VAR_GET(prvCpssDxChFdbManagerDbArr)[_fdbManagerId]->numOfAllocatedPointers--;\
        FREE_PTR_MAC(_pointer);                                         \
    }

/* macro to allow the 'references' of API to 'see' relation to parameter */
#define BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(paramName,lowLevel_apiName)    /*empty*/

#define NUM_ENTRIES_STEPS_CHECK_MAC(_numEntries)         \
    if(0 == (_numEntries))                               \
    {                                                    \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,      \
            "[%s] must not be ZERO",                     \
            #_numEntries);                               \
    }                                                    \
    if(0 != ((_numEntries) % PRV_FDB_MANAGER_NUM_ENTRIES_STEPS_CNS)) \
    {                                                    \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,      \
            "[%s] [%d] must be value that is multiply value of [%d] (without leftovers)", \
            #_numEntries,                                \
            _numEntries,                                 \
            PRV_FDB_MANAGER_NUM_ENTRIES_STEPS_CNS);      \
    }

#define SELF_TEST_BMP_SET_MAC(_fdbManagerPtr, _bitPos)   BMP_SET_MAC(&_fdbManagerPtr->selfTestBitmap, _bitPos)
#define SELF_TEST_BMP_CLR_MAC(_fdbManagerPtr, _bitPos)   BMP_CLEAR_MAC(&_fdbManagerPtr->selfTestBitmap, _bitPos)
#define PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(_resultArr, err, errNumPtr)    \
    {                                                                               \
        _resultArr[*errNumPtr] = err;                                               \
        *errNumPtr += 1;                                                            \
        /* give error to the LOG ... for debug !!! */                               \
        CPSS_LOG_ERROR_MAC("Debug Error number [%d] of Type[%s]",                   \
            *errNumPtr,#err);                                                       \
    }

/*extern*/ GT_STATUS prvCpssDxChSip6GlobalEportMaskCheck
(
    IN  GT_U8   devNum,
    IN  GT_U32  mask,
    IN  GT_U32  pattern,
    OUT GT_U32  *minValuePtr,
    OUT GT_U32  *maxValuePtr
);

/**
* @internal buildDbEntry_dstInterface_macOrIpmc function
* @endinternal
*
* @brief  function to build the DB format
*         for the mac Or Ipmc : the 'dstInterface' part
*         the IN  format is 'Hw format'
*         the OUT format is 'DB format'
*
* @param[in] isMacEntry             - MAC or IP MC FDB entry type:
*                                           GT_TRUE  - MAC entry
*                                           GT_FALSE - IP MC entry
* @param[in] hwDataArr              - (pointer to) The FDB Entry in HW Format.
* @param[out] dstInterfacePtr       - (pointer to) the manager dstInterface format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2 AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   buildDbEntry_dstInterface_macOrIpmc
(
    IN  GT_BOOL                                                 isMacEntry,
    IN  GT_U32                                                 *hwDataArr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC     *dstInterfacePtr
)
{
    GT_U32      hwValue;
    GT_U32      multiple = 0;

    if (isMacEntry)
    {
        /* multiple */
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_MULTIPLE_E,
                hwValue);
        multiple = hwValue;

        if (multiple == 0)
        {/* bit40MacAddr=0 and entry is PORT or TRUNK */

            /* devNum ID */
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_DEV_ID_E,
                hwValue);
            dstInterfacePtr->interfaceInfo.devPort.hwDevNum = hwValue;

            /* is Trunk bit */
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_IS_TRUNK_E,
                hwValue);

            /* PortNum/TrunkNum */
            if(hwValue)
            {
                SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_TRUNK_NUM_E,
                    hwValue);
                dstInterfacePtr->type = CPSS_INTERFACE_TRUNK_E;
                dstInterfacePtr->interfaceInfo.trunkId = (GT_TRUNK_ID)hwValue;
            }
            else
            {
                SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_EPORT_NUM_E,
                    hwValue);
                dstInterfacePtr->type = CPSS_INTERFACE_PORT_E;
                dstInterfacePtr->interfaceInfo.devPort.portNum = hwValue;
            }
        }
    }
    else
    {
        multiple = 1;
    }

    /* MACEntryType = "IPv4" or MACEntryType = "IPv6" or
       (MACEntryType = "MAC" and (MACAddrBit40 = "1" or Multiple = "Multicast")) */
    if (multiple == 1)
    {
        /* VIDX */
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_VIDX_E,
            hwValue);
        dstInterfacePtr->interfaceInfo.vidx = (GT_TRUNK_ID)hwValue;
        dstInterfacePtr->type = (hwValue == 0xFFF)?CPSS_INTERFACE_VID_E:CPSS_INTERFACE_VIDX_E;
    }
    return GT_OK;
}

/**
* @internal buildDbEntry_prvFdbEntryMacAddrFormat_mux_fields function
* @endinternal
*
* @brief  function to build the DB format from HW format.
*         for the fdbEntryMacAddrFormat : the 'mux' part
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] hwDataArr              - (pointer to) The FDB Entry in HW Format.
* @param[out] entryPtr              - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildDbEntry_prvFdbEntryMacAddrFormat_mux_fields
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC         *fdbManagerPtr,
    IN  GT_U32                                                 *hwDataArr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC    *entryPtr
)
{
    GT_U32      hwValue;

    /* memset */
    entryPtr->vid1            = 0;
    entryPtr->sourceID        = 0;
    entryPtr->userDefined     = 0;
    entryPtr->daSecurityLevel = 0;

    switch (fdbManagerPtr->entryAttrInfo.macEntryMuxingMode)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E:
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_0_E,
                    hwValue);
            U32_SET_FIELD_MAC(entryPtr->vid1, 0, 1, hwValue);
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_6_1_E,
                    hwValue);
            U32_SET_FIELD_MAC(entryPtr->vid1, 1, 6, hwValue);
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_11_7_E,
                    hwValue);
            U32_SET_FIELD_MAC(entryPtr->vid1, 7, 5, hwValue);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E:
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_0_E,
                    hwValue);
            U32_SET_FIELD_MAC(entryPtr->sourceID, 0, 1, hwValue);
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1_E,
                    hwValue);
            U32_SET_FIELD_MAC(entryPtr->sourceID, 1, 2, hwValue);
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_6_3_E,
                    hwValue);
            U32_SET_FIELD_MAC(entryPtr->sourceID, 3, 4, hwValue);
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_11_7_E,
                    hwValue);
            U32_SET_FIELD_MAC(entryPtr->sourceID, 7, 5, hwValue);
            entryPtr->userDefined = 0;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_UDB_10_SRC_ID_1_DA_ACCESS_LEVEL_E:
        case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_UDB_8_SRC_ID_3_DA_ACCESS_LEVEL_E:
            if(fdbManagerPtr->entryAttrInfo.macEntryMuxingMode ==
                    CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_UDB_10_SRC_ID_1_DA_ACCESS_LEVEL_E)
            {
                SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                        SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_6_5_E,
                        hwValue);
                U32_SET_FIELD_MAC(entryPtr->userDefined, 5, 2, hwValue);

                SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                        SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_10_7_E,
                        hwValue);
                U32_SET_FIELD_MAC(entryPtr->userDefined, 7, 4, hwValue);

            }
            else
            {
                hwValue = U32_GET_FIELD_MAC(entryPtr->sourceID,1,2);
                SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                        SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1_E,
                        hwValue);
                U32_SET_FIELD_MAC(entryPtr->sourceID, 1, 2, hwValue);

                SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                        SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_10_7_E,
                        hwValue);
                U32_SET_FIELD_MAC(entryPtr->userDefined, 5, 4, hwValue);        /* UNLIKLE the CPSS : the value is continues. (no 2 bits hole) */
            }

            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_4_1_E,
                    hwValue);
            U32_SET_FIELD_MAC(entryPtr->userDefined, 1, 4, hwValue);

            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_0_E,
                    hwValue);
            U32_SET_FIELD_MAC(entryPtr->sourceID, 0, 1, hwValue);

            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL_E,
                    hwValue);
            entryPtr->daSecurityLevel = hwValue;
            break;
        default:
            break;
    }
    return GT_OK;
}

/**
* @internal buildDbEntry_prvFdbEntryMacAddrFormat function
* @endinternal
*
* @brief  function to build the DB format from HW format.
*         for the fdbEntryMacAddrFormat
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] entryPtr               - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildDbEntry_prvFdbEntryMacAddrFormat
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC     *entryPtr
)
{
    GT_STATUS                   rc;
    GT_U32                      hwValue;
    GT_U32                     *hwDataArr = &fdbManagerPtr->tempInfo.sip6_hwFormatWords[0];

    /* age */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_AGE_E,
            hwValue);
    entryPtr->age = BIT2BOOL_MAC(hwValue);

    /* FID */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_FID_E,
        hwValue);
    entryPtr->fid = hwValue;

    /* MAC address */
    SIP6_FDB_HW_ENTRY_FIELD_MAC_ADDR_GET_MAC(hwDataArr, entryPtr->macAddr.arEther);

    /* SA cmd */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_SA_CMD_E,
        hwValue);
    /* SIP6 device in case of Hybrid model, do not support "entryAttrInfo.saDropCommand", to align with other supported device.
     * So the drop type is chosen to SOFT in that case*/
    if(fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E)
    {
        entryPtr->saCommand = (hwValue == 1)?CPSS_PACKET_CMD_DROP_SOFT_E:CPSS_PACKET_CMD_FORWARD_E;
    }
    else
    {
        entryPtr->saCommand = (hwValue == 1)?fdbManagerPtr->entryAttrInfo.saDropCommand:CPSS_PACKET_CMD_FORWARD_E;
    }

    /* DA cmd */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_DA_CMD_E,
            hwValue);
    if(hwValue == 3)
    {
        entryPtr->daCommand = fdbManagerPtr->entryAttrInfo.daDropCommand;
    }
    else
    {
        entryPtr->daCommand = hwValue;
    }

    /* static */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_IS_STATIC_E,
        hwValue);
    entryPtr->isStatic = BIT2BOOL_MAC(hwValue);

    /* DA Route */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_DA_ROUTE_E,
        hwValue);
    entryPtr->daRoute = BIT2BOOL_MAC(hwValue);

    /* AppSpecific CPU Code */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E,
        hwValue);
    entryPtr->appSpecificCpuCode = BIT2BOOL_MAC(hwValue);

    /* SA Access level */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_SA_ACCESS_LEVEL_E,
            hwValue);
    entryPtr->saSecurityLevel = hwValue;

    rc = buildDbEntry_prvFdbEntryMacAddrFormat_mux_fields(fdbManagerPtr, hwDataArr, entryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Not valid for SIP6 */
    entryPtr->mirrorToAnalyzerPort   = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E;
    entryPtr->daQoSParameterSetIndex = 0;
    entryPtr->saQoSParameterSetIndex = 0;

    return buildDbEntry_dstInterface_macOrIpmc(GT_TRUE /* MAC entry */,
            hwDataArr,
            &entryPtr->dstInterface);
}

/**
* @internal buildDbEntry_dstInterface_IpvxUc function
* @endinternal
*
* @brief  function to build the DB Format from HW format.
*         for the IPvx UC routing : the 'dstInterface' part
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
* @param[in] hwDataArr              - (pointer to) The FDB Entry in HW Format.
* @param[out] dstInterfacePtr       - (pointer to) the manager dstInterface format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   buildDbEntry_dstInterface_IpvxUc
(
    IN  GT_U32                                             *hwDataArr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC *dstInterfacePtr
)
{
    GT_U32      hwValue;
    GT_U32      multiple = 0;

    /* devNum ID */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_DEV_E,
            hwValue);
    dstInterfacePtr->interfaceInfo.devPort.hwDevNum = hwValue;

    /* multiple */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_USE_VIDX_E,
            hwValue);
    multiple = hwValue;

    if (multiple == 0)
    {
        /* is Trunk bit */
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_IS_TRUNK_E,
                hwValue);
        /* PortNum/TrunkNum */
        if(hwValue)
        {
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_TRUNK_ID_E,
                    hwValue);
            dstInterfacePtr->type = CPSS_INTERFACE_TRUNK_E;
            dstInterfacePtr->interfaceInfo.trunkId = (GT_TRUNK_ID)hwValue;
        }
        else
        {
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_EPORT_E,
                    hwValue);
            dstInterfacePtr->type = CPSS_INTERFACE_PORT_E;
            dstInterfacePtr->interfaceInfo.devPort.portNum = hwValue;
        }
    }
    else
    {
        /* VIDX */
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_EVIDX_E,
                hwValue);
        dstInterfacePtr->interfaceInfo.vidx = (GT_U16)hwValue;
        dstInterfacePtr->type = (hwValue == 0xFFF)?CPSS_INTERFACE_VID_E:CPSS_INTERFACE_VIDX_E;
    }
    return GT_OK;
}


/**
* @internal buildDbEntry_prvIpv4UcEntryFormat function
* @endinternal
*
* @brief  function to build the DB format from HW format.
*         for the prvIpv4UcEntryFormat
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
*         based on buildFdbUcRouteEntryHwFormatSip6
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] entryPtr               - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildDbEntry_prvIpv4UcEntryFormat
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC         *fdbManagerPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC     *entryPtr
)
{
    GT_STATUS                   rc;
    GT_U32                      hwValue;
    GT_U32                     *hwDataArr = &fdbManagerPtr->tempInfo.sip6_hwFormatWords[0];
    GT_BOOL                     isNatPointer;

    /* IP Addr */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV4_DIP_E,
        hwValue);
    entryPtr->ipv4Addr.arIP[0] = (GT_U8)(hwValue >> 24);
    entryPtr->ipv4Addr.arIP[1] = (GT_U8)(hwValue >> 16);
    entryPtr->ipv4Addr.arIP[2] = (GT_U8)(hwValue >> 8);
    entryPtr->ipv4Addr.arIP[3] = (GT_U8)(hwValue);

    /* age */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_AGE_E,
        hwValue);
    entryPtr->age = BIT2BOOL_MAC(hwValue);

    /* VRF-ID */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_VRF_ID_E,
        hwValue);
    entryPtr->vrfId = hwValue;

    /* Is NAT Pointer */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT_E,
            hwValue);
    isNatPointer = BIT2BOOL_MAC(hwValue);

    /* Route type */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE_E,
            hwValue);

    if(hwValue == 0)
    {
        /*
            case of UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E --> tunnelStartPointer
            case of UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E          --> natPointer
            case of UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E          --> arpPointer
            */
        /* ttl/hop decrement enable */
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT_E,
                hwValue);
        entryPtr->ucRouteInfo.fullFdbInfo.ttlHopLimitDecEnable = hwValue;

        /* Bypass TTL Options Or Hop Extension */
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E,
                hwValue);
        entryPtr->ucRouteInfo.fullFdbInfo.ttlHopLimDecOptionsExtChkByPass = hwValue;

        /* Counter set */
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX_E,
                hwValue);
        entryPtr->ucRouteInfo.fullFdbInfo.countSet = hwValue;

        /* ICMP Redirect enable */
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN_E,
                hwValue);
        entryPtr->ucRouteInfo.fullFdbInfo.ICMPRedirectEnable = hwValue;

        /* mtu profile index */
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_MTU_INDEX_E,
                hwValue);
        entryPtr->ucRouteInfo.fullFdbInfo.mtuProfileIndex = hwValue;

        /* Get vlan associated with this entry */
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN_E,
                hwValue);
        entryPtr->ucRouteInfo.fullFdbInfo.nextHopVlanId = hwValue;

        /* Tunnel start */
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL_E,
                hwValue);
        if(hwValue == 1)
        {
            /* Get tunnel ptr */
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR_E,
                    hwValue);
            if(isNatPointer == 1)
            {
                entryPtr->ucRouteInfo.fullFdbInfo.pointer.natPointer = hwValue;
                entryPtr->ucRouteType =
                    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E;
            }
            else
            {
                entryPtr->ucRouteInfo.fullFdbInfo.pointer.tunnelStartPointer = hwValue;
                entryPtr->ucRouteType =
                    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E;
            }
        }
        else
        {
            entryPtr->ucRouteType = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
            /* arp ptr*/
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ARP_PTR_E,
                    hwValue);
            entryPtr->ucRouteInfo.fullFdbInfo.pointer.arpPointer = hwValue;
        }

        rc = buildDbEntry_dstInterface_IpvxUc(hwDataArr,
                &entryPtr->ucRouteInfo.fullFdbInfo.dstInterface);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else /* UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E /
            UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E        */
    {
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ECMP_OR_NHE_POINTER_E,
            hwValue);
        entryPtr->ucRouteInfo.nextHopPointerToRouter = hwValue;

        /* 'trick' use the 'is_NAT' places in same bit as the 'pointer_type' */
        if(isNatPointer == 1)
        {
            entryPtr->ucRouteType = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E;
        }
        else
        {
            entryPtr->ucRouteType = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E;
        }
    }

    /* Not valid for SIP6 */
    entryPtr->ucCommonInfo.qosProfileMarkingEnable = GT_FALSE;
    entryPtr->ucCommonInfo.qosProfilePrecedence    = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    entryPtr->ucCommonInfo.modifyUp                = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    entryPtr->ucCommonInfo.modifyDscp              = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    entryPtr->ucCommonInfo.arpBcTrapMirrorEnable   = GT_FALSE;
    entryPtr->ucCommonInfo.dipAccessLevel          = 0;
    entryPtr->ucCommonInfo.ingressMirrorToAnalyzerEnable = GT_FALSE;

    return GT_OK;
}

/**
* @internal buildDbEntry_prvIpv6UcEntryFormat function
* @endinternal
*
* @brief  function to build the DB Format from HW format
*         for the prvIpv6UcDataEntryFormat and prvIpv6UcKeyEntryFormat
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
*         based on buildFdbUcRouteEntryHwFormatSip6
*
* @param[in]  fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[out] entryPtr              - (pointer to) the manager ipv6 entry format
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - wrong parameter
*/
static GT_STATUS buildDbEntry_prvIpv6UcEntryFormat
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC        *fdbManagerPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC    *entryPtr
)
{
    GT_STATUS                   rc;
    GT_U32                      hwValue;
    GT_U32                      *hwKeyArr  = fdbManagerPtr->tempInfo.sip6_hwFormatWords;
    GT_U32                      *hwDataArr = fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords;
    GT_U32                      hwValue1;
    GT_U32                      wordNum;
    GT_BOOL                     isNatPointer;

    /* age */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_AGE_E,
        hwValue);
    entryPtr->age = hwValue;

    /* VRF-ID */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_VRF_ID_E,
        hwValue);
    entryPtr->vrfId = hwValue;

    /* Destination Site ID */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID_E,
        hwValue);
    if(hwValue == 0)
    {
        entryPtr->ucRouteInfo.fullFdbInfo.ipv6ExtInfo.siteId = CPSS_IP_SITE_ID_INTERNAL_E;
    }
    else
    {
        entryPtr->ucRouteInfo.fullFdbInfo.ipv6ExtInfo.siteId = CPSS_IP_SITE_ID_EXTERNAL_E;
    }


    /* Scope Check Enable */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK_E,
        hwValue);
    entryPtr->ucRouteInfo.fullFdbInfo.ipv6ExtInfo.scopeCheckingEnable = hwValue;

    for(wordNum = 0; wordNum < 4; wordNum++)
    {
        /* SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_31_0_E
         * SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_63_32_E
         * SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_95_64_E
         * SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_105_96_E
         */
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwKeyArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_31_0_E + wordNum,
                hwValue);
        if(wordNum == 3)
        {
            /* SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_127_106_E - 22 bits from hwDataArr*/
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_127_106_E,
                    hwValue1);
            hwValue = ((hwValue1 << 10) | (hwValue & BIT_MASK_MAC(10)));
        }
        entryPtr->ipv6Addr.arIP[((3 - wordNum) * 4)]       = (GT_U8)(hwValue >> 24);
        entryPtr->ipv6Addr.arIP[((3 - wordNum) * 4) +1]    = (GT_U8)(hwValue >> 16);
        entryPtr->ipv6Addr.arIP[((3 - wordNum) * 4) +2]    = (GT_U8)(hwValue >> 8);
        entryPtr->ipv6Addr.arIP[((3 - wordNum) * 4) +3]    = (GT_U8)(hwValue);
    }

    /* Is NAT Pointer */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT_E,
            hwValue);
    isNatPointer = BIT2BOOL_MAC(hwValue);

    /* Route Type */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE_E,
            hwValue);

    if(hwValue == 0)
    {
        /*
           case of UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E --> tunnelStartPointer
           case of UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E          --> natPointer
           case of UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E          --> arpPointer
           */
        /* ttl/hop decrement enable */
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT_E,
                hwValue);
        entryPtr->ucRouteInfo.fullFdbInfo.ttlHopLimitDecEnable = hwValue;

        /* Bypass TTL Options Or Hop Extension */
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E,
                hwValue);
        entryPtr->ucRouteInfo.fullFdbInfo.ttlHopLimDecOptionsExtChkByPass = hwValue;

        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX_E,
                hwValue);
        entryPtr->ucRouteInfo.fullFdbInfo.countSet = hwValue;

        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN_E,
                hwValue);
        entryPtr->ucRouteInfo.fullFdbInfo.ICMPRedirectEnable = hwValue;

        /* mtu profile index */
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_MTU_INDEX_E,
                hwValue);
        entryPtr->ucRouteInfo.fullFdbInfo.mtuProfileIndex = hwValue;

        /* Get vlan associated with this entry */
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN_E,
                hwValue);
        entryPtr->ucRouteInfo.fullFdbInfo.nextHopVlanId = hwValue;

        /* Tunnel start */
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL_E,
                hwValue);
        if(hwValue == 1)
        {
            /* Get tunnel ptr */
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR_E,
                    hwValue);
            if(isNatPointer == 1)
            {
                entryPtr->ucRouteInfo.fullFdbInfo.pointer.natPointer = hwValue;
                entryPtr->ucRouteType =
                    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E;
            }
            else
            {
                entryPtr->ucRouteInfo.fullFdbInfo.pointer.tunnelStartPointer = hwValue;
                entryPtr->ucRouteType =
                    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E;
            }
        }
        else
        {
            entryPtr->ucRouteType = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
            /* arp ptr*/
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ARP_PTR_E,
                    hwValue);
            entryPtr->ucRouteInfo.fullFdbInfo.pointer.arpPointer = hwValue;
        }

        rc = buildDbEntry_dstInterface_IpvxUc(hwDataArr,
                &entryPtr->ucRouteInfo.fullFdbInfo.dstInterface);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else /* UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E /
            UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E        */
    {
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ECMP_OR_NHE_POINTER_E,
            hwValue);
        entryPtr->ucRouteInfo.nextHopPointerToRouter = hwValue;

        /* 'trick' use the 'is_NAT' places in same bit as the 'pointer_type' */
        if(isNatPointer == 1)
        {
            entryPtr->ucRouteType = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E;
        }
        else
        {
            entryPtr->ucRouteType = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E;
        }
    }

    /* Not valid for SIP6 */
    entryPtr->ucCommonInfo.qosProfileMarkingEnable = GT_FALSE;
    entryPtr->ucCommonInfo.qosProfilePrecedence    = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    entryPtr->ucCommonInfo.modifyUp                = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    entryPtr->ucCommonInfo.modifyDscp              = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    entryPtr->ucCommonInfo.arpBcTrapMirrorEnable   = GT_FALSE;
    entryPtr->ucCommonInfo.dipAccessLevel          = 0;
    entryPtr->ucCommonInfo.ingressMirrorToAnalyzerEnable = GT_FALSE;

    return GT_OK;
}

/**
* @internal buildDbEntry_prvFdbEntryIpMcFormat_mux_fields function
* @endinternal
*
* @brief  function to build the Db format from HW format
*         for the fdbEntryIpv4McFormat and fdbEntryIpv6McFormat : the 'mux' part
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
* @param[in]  fdbManagerPtr        - (pointer to) the FDB Manager.
* @param[in]  hwDataArr            - (pointer to) the FDB Manager entry in HW format.
* @param[out] entryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildDbEntry_prvFdbEntryIpMcFormat_mux_fields
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN  GT_U32                                                  *hwDataArr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC        *entryPtr
)
{
    GT_U32      hwValue;

    /* Memset */
    entryPtr->sourceID = 0;
    entryPtr->userDefined = 0;

    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_0_E,
        hwValue);
    U32_SET_FIELD_MAC(entryPtr->sourceID, 0, 1, hwValue);

    switch (fdbManagerPtr->entryAttrInfo.ipmcEntryMuxingMode)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E:
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1_E,
                hwValue);
            U32_SET_FIELD_MAC(entryPtr->sourceID, 1, 2, hwValue);
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_6_3_E,
                hwValue);
            U32_SET_FIELD_MAC(entryPtr->sourceID, 3, 4, hwValue);
            break;

        case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_4_SRC_ID_3_E:
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1_E,
                hwValue);
            U32_SET_FIELD_MAC(entryPtr->sourceID, 1, 2, hwValue);

            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_4_1_E,
                hwValue);
            U32_SET_FIELD_MAC(entryPtr->userDefined, 1, 4, hwValue);
            break;

        case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_E:
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_4_1_E,
                hwValue);
            U32_SET_FIELD_MAC(entryPtr->userDefined, 1, 4, hwValue);

            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_6_5_E,
                hwValue);
            U32_SET_FIELD_MAC(entryPtr->userDefined, 5, 2, hwValue);
            break;
        default:
            break;
    }

    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL_E,
            hwValue);
    entryPtr->daSecurityLevel = hwValue;

    return GT_OK;
}

/**
* @internal buildDbEntry_prvFdbEntryIpMcAddrFormat function
* @endinternal
*
* @brief  function to build the DB format from HW format
*         for the fdbEntryIpv4McFormat and fdbEntryIpv6McFormat
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
* @param[in]  fdbManagerPtr        - (pointer to) the FDB Manager.
* @param[out] entryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildDbEntry_prvFdbEntryIpMcAddrFormat
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC        *entryPtr
)
{
    GT_STATUS                rc;
    GT_U32                   hwValue;
    GT_U32                   sipAddr = 0;
    GT_U32                  *hwDataArr = fdbManagerPtr->tempInfo.sip6_hwFormatWords;

    entryPtr->vid1 = 0;
    /* age */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_AGE_E,
        hwValue);
    entryPtr->age = hwValue;

    /* FID */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_FID_E,
        hwValue);
    entryPtr->fid = hwValue;

    /*SIP*/
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_SIP_26_0_E,
            hwValue);
    U32_SET_FIELD_MAC(sipAddr, 0, 27, hwValue);
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_SIP_30_27_E,
            hwValue);
    U32_SET_FIELD_MAC(sipAddr, 27, 4, hwValue);
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_SIP_31_E,
            hwValue);
    U32_SET_FIELD_MAC(sipAddr, 31, 1, hwValue);
    entryPtr->sipAddr[0] = (GT_U8)(sipAddr >> 24);
    entryPtr->sipAddr[1] = (GT_U8)(sipAddr >> 16);
    entryPtr->sipAddr[2] = (GT_U8)(sipAddr >> 8 );
    entryPtr->sipAddr[3] = (GT_U8)(sipAddr);

    /*DIP*/
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP6_FDB_FDB_TABLE_FIELDS_DIP_E,
            hwValue);
    entryPtr->dipAddr[0] = (GT_U8)(hwValue >> 24);
    entryPtr->dipAddr[1] = (GT_U8)(hwValue >> 16);
    entryPtr->dipAddr[2] = (GT_U8)(hwValue >> 8 );
    entryPtr->dipAddr[3] = (GT_U8)(hwValue);

    /* static */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_IS_STATIC_E,
        hwValue);
    entryPtr->isStatic = hwValue;

    /* DA cmd */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_DA_CMD_E,
        hwValue);
    if(hwValue == 3)
    {
        entryPtr->daCommand = fdbManagerPtr->entryAttrInfo.daDropCommand;
    }
    else
    {
        entryPtr->daCommand = hwValue;
    }

    /* DA Route */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_DA_ROUTE_E,
        hwValue);
    entryPtr->daRoute = hwValue;

    /* AppSpecific CPU Code */
    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP6_FDB_FDB_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E,
        hwValue);
    entryPtr->appSpecificCpuCode = hwValue;

    rc = buildDbEntry_prvFdbEntryIpMcFormat_mux_fields(fdbManagerPtr, hwDataArr, entryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Not valid for SIP6 */
    entryPtr->mirrorToAnalyzerPort   = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E;
    entryPtr->daQoSParameterSetIndex = 0;
    entryPtr->saQoSParameterSetIndex = 0;
    entryPtr->vid1                   = 0;

    return buildDbEntry_dstInterface_macOrIpmc(GT_FALSE /* IP MC entry */,
            hwDataArr,
            &entryPtr->dstInterface);
}

/**
* @internal buildDbEntry_dstInterface_macOrIpmc_sip4 function
* @endinternal
*
* @brief  function to build the DB format
*         for the mac Or Ipmc : the 'dstInterface' part
*         the IN  format is 'Hw format'
*         the OUT format is 'DB format'
*
* @param[in] isMacEntry             - MAC or IP MC FDB entry type:
*                                           GT_TRUE  - MAC entry
*                                           GT_FALSE - IP MC entry
* @param[in] hwDataArr              - (pointer to) The FDB Entry in HW Format.
* @param[out] dstInterfacePtr       - (pointer to) the manager dstInterface format (manager format)
* @param[out] userDefinedPtr        - (pointer to) the user defined field
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   buildDbEntry_dstInterface_macOrIpmc_sip4
(
    IN  GT_BOOL                                                 isMacEntry,
    IN  GT_U32                                                 *hwDataArr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC     *dstInterfacePtr,
    OUT GT_U32                                                 *userDefinedPtr
)
{
    GT_U32      hwValue;
    GT_U32      multiple = 0;

    if (isMacEntry)
    {
        /* get multiple bit */
        multiple = U32_GET_FIELD_MAC(hwDataArr[2],26,1);

        if (multiple == 0)
        {/* bit40MacAddr=0 and entry is PORT or TRUNK */

            /* devNum ID */
            dstInterfacePtr->interfaceInfo.devPort.hwDevNum = U32_GET_FIELD_MAC(hwDataArr[2],1,5);

            /* is Trunk bit */
            hwValue = U32_GET_FIELD_MAC(hwDataArr[2],13,1);

            /* PortNum/TrunkNum */
            if(hwValue)
            {
                dstInterfacePtr->type = CPSS_INTERFACE_TRUNK_E;
                dstInterfacePtr->interfaceInfo.trunkId = (GT_TRUNK_ID)U32_GET_FIELD_MAC(hwDataArr[2],14,7);
            }
            else
            {
                dstInterfacePtr->type = CPSS_INTERFACE_PORT_E;
                dstInterfacePtr->interfaceInfo.devPort.portNum = U32_GET_FIELD_MAC(hwDataArr[2],14,6);
            }
        }
    }
    else
    {
        multiple = 1;
    }

    /* MACEntryType = "IPv4" or MACEntryType = "IPv6" or
       (MACEntryType = "MAC" and (MACAddrBit40 = "1" or Multiple = "Multicast")) */
    if (multiple == 1)
    {
        /* VIDX */
        hwValue = U32_GET_FIELD_MAC(hwDataArr[2],13,12);
        dstInterfacePtr->interfaceInfo.vidx = (GT_U16)hwValue;
        dstInterfacePtr->type = (hwValue == 0xFFF)?CPSS_INTERFACE_VID_E:CPSS_INTERFACE_VIDX_E;
    }
    else if(userDefinedPtr)
    {
        /* user defined */
        *userDefinedPtr = U32_GET_FIELD_MAC(hwDataArr[2],21,4);
    }
    return GT_OK;
}

/**
* @internal buildDbEntry_prvFdbEntryIpMcAddrFormat_sip4 function
* @endinternal
*
* @brief  function to build the DB format from HW format
*         for the fdbEntryIpv4McFormat and fdbEntryIpv6McFormat
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
* @param[in]  fdbManagerPtr        - (pointer to) the FDB Manager.
* @param[out] entryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildDbEntry_prvFdbEntryIpMcAddrFormat_sip4
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC        *entryPtr
)
{
    GT_U32                  *hwDataArr = fdbManagerPtr->tempInfo.sip6_hwFormatWords;

    entryPtr->age = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataArr[0], 2, 1));
    entryPtr->fid = U32_GET_FIELD_MAC(hwDataArr[0], 5, 12);

    /* set DIP Address */
    entryPtr->dipAddr[3] = (GT_U8)U32_GET_FIELD_MAC(hwDataArr[0],17,8);
    entryPtr->dipAddr[2] = (GT_U8)(U32_GET_FIELD_MAC(hwDataArr[0],25,7) |
                                  (U32_GET_FIELD_MAC(hwDataArr[1],0,1) << 7));
    entryPtr->dipAddr[1] = (GT_U8)U32_GET_FIELD_MAC(hwDataArr[1],1,8);
    entryPtr->dipAddr[0] = (GT_U8)U32_GET_FIELD_MAC(hwDataArr[1],9,8);

    /* set SIP Address */
    entryPtr->sipAddr[3] = (GT_U8)U32_GET_FIELD_MAC(hwDataArr[1],17,8);
    entryPtr->sipAddr[2] = (GT_U8)(U32_GET_FIELD_MAC(hwDataArr[1],25,7) |
                                  (U32_GET_FIELD_MAC(hwDataArr[2],0,1) << 7));
    entryPtr->sipAddr[1] = (GT_U8)U32_GET_FIELD_MAC(hwDataArr[2],1,8);
    entryPtr->sipAddr[0] = (GT_U8)(U32_GET_FIELD_MAC(hwDataArr[2],9,4)         |
                                  (U32_GET_FIELD_MAC(hwDataArr[2],26,1) << 4) |
                                  (U32_GET_FIELD_MAC(hwDataArr[2],30,2) << 5) |
                                  (U32_GET_FIELD_MAC(hwDataArr[3],0,1)  << 7));

    /* Source ID & vid1 - Not supported for IPMC in case of AC5 */
    entryPtr->sourceID   = 0;
    entryPtr->vid1       = 0;

    /* set DA cmd */
    entryPtr->daCommand = (GT_U8)U32_GET_FIELD_MAC(hwDataArr[2],27,3);

    /* set static */
    entryPtr->isStatic  = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataArr[2],25,1));

    /* DA Route */
    entryPtr->daRoute   = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataArr[3],1,1));

    /* SA QoS Profile index */
    entryPtr->saQoSParameterSetIndex = U32_GET_FIELD_MAC(hwDataArr[3],3,3);

    /* DA QoS Profile index */
    entryPtr->daQoSParameterSetIndex = U32_GET_FIELD_MAC(hwDataArr[3],6,3);

    /* Mirror to Analyzer Port */
    if(BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataArr[3],9,1)))
    {
        entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E;
    }
    else
    {
        entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E;
    }

    /* Application Specific CPU Code */
    entryPtr->appSpecificCpuCode = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataArr[3],10,1));

    /* DA Access Level */
    entryPtr->daSecurityLevel = U32_GET_FIELD_MAC(hwDataArr[3],11,3);

    /* Valid only for MAC Entry */
    entryPtr->userDefined = 0;

    return buildDbEntry_dstInterface_macOrIpmc_sip4(GT_FALSE /* MAC entry */,
            hwDataArr,
            &entryPtr->dstInterface,
            NULL);
}

/**
* @internal buildDbEntry_prvFdbEntryMacAddrFormat_sip4 function
* @endinternal
*
* @brief  function to build the DB format from HW format.
*         for the fdbEntryMacAddrFormat
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] entryPtr               - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildDbEntry_prvFdbEntryMacAddrFormat_sip4
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC     *entryPtr
)
{
    GT_U32                     *hwDataArr = &fdbManagerPtr->tempInfo.sip6_hwFormatWords[0];

    entryPtr->age  = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataArr[0], 2, 1));
    entryPtr->fid  = U32_GET_FIELD_MAC(hwDataArr[0], 5, 12);
    entryPtr->vid1 = 0;

    entryPtr->macAddr.arEther[5] = (GT_U8)U32_GET_FIELD_MAC(hwDataArr[0], 17, 8);
    entryPtr->macAddr.arEther[4] = (GT_U8)((U32_GET_FIELD_MAC(hwDataArr[0], 25, 7)) |
                                           (U32_GET_FIELD_MAC(hwDataArr[1], 0, 1) << 7));
    entryPtr->macAddr.arEther[3] = (GT_U8)U32_GET_FIELD_MAC(hwDataArr[1], 1,  8);
    entryPtr->macAddr.arEther[2] = (GT_U8)U32_GET_FIELD_MAC(hwDataArr[1], 9,  8);
    entryPtr->macAddr.arEther[1] = (GT_U8)U32_GET_FIELD_MAC(hwDataArr[1], 17, 8);
    entryPtr->macAddr.arEther[0] = (GT_U8)((U32_GET_FIELD_MAC(hwDataArr[1], 25, 7)) |
                                          (U32_GET_FIELD_MAC(hwDataArr[2], 0, 1) << 7));

    /* set DA cmd */
    entryPtr->daCommand = (GT_U8)U32_GET_FIELD_MAC(hwDataArr[2],27,3);
    /* set SA cmd */
    entryPtr->saCommand = ((U32_GET_FIELD_MAC(hwDataArr[2],30,2)) |
                            ((U32_GET_FIELD_MAC(hwDataArr[3],0,1)) << 2));

    /* Source ID */
    entryPtr->sourceID = U32_GET_FIELD_MAC(hwDataArr[2],6,5);

    /* set static */
    entryPtr->isStatic = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataArr[2],25,1));

    /* DA Route */
    entryPtr->daRoute = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataArr[3],1,1));

    /* SA QoS Profile index */
    entryPtr->saQoSParameterSetIndex = U32_GET_FIELD_MAC(hwDataArr[3],3,3);

    /* DA QoS Profile index */
    entryPtr->daQoSParameterSetIndex = U32_GET_FIELD_MAC(hwDataArr[3],6,3);

    /* Mirror to Analyzer Port */
    if(BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataArr[3],9,1)))
    {
        entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E;
    }
    else
    {
        entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E;
    }

    /* Application Specific CPU Code */
    entryPtr->appSpecificCpuCode = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataArr[3],10,1));

    /* DA Access Level */
    entryPtr->daSecurityLevel = U32_GET_FIELD_MAC(hwDataArr[3],11,3);

    /* SA Access Level */
    entryPtr->saSecurityLevel = U32_GET_FIELD_MAC(hwDataArr[3],14,3);

    return buildDbEntry_dstInterface_macOrIpmc_sip4(GT_TRUE /* MAC entry */,
            hwDataArr,
            &entryPtr->dstInterface,
            &entryPtr->userDefined);
}

/**
* @internal buildDbEntry_dstInterface_macOrIpmc_sip5 function
* @endinternal
*
* @brief  function to build the DB format
*         for the mac Or Ipmc : the 'dstInterface' part
*         the IN  format is 'Hw format'
*         the OUT format is 'DB format'
*
* @param[in] isMacEntry             - MAC or IP MC FDB entry type:
*                                           GT_TRUE  - MAC entry
*                                           GT_FALSE - IP MC entry
* @param[in] hwDataArr              - (pointer to) The FDB Entry in HW Format.
* @param[out] dstInterfacePtr       - (pointer to) the manager dstInterface format (manager format)
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   buildDbEntry_dstInterface_macOrIpmc_sip5
(
    IN  GT_BOOL                                                 isMacEntry,
    IN  GT_U32                                                 *hwDataArr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC     *dstInterfacePtr
)
{
    GT_U32      hwValue;
    GT_U32      multiple = 0;

    if (isMacEntry)
    {
        /* multiple */
        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_MULTIPLE_E,
                hwValue);
        multiple = hwValue;

        if (multiple == 0)
        {/* bit40MacAddr=0 and entry is PORT or TRUNK */

            /* devNum ID */
            SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_DEV_ID_E,
                hwValue);
            dstInterfacePtr->interfaceInfo.devPort.hwDevNum = hwValue;

            /* is Trunk bit */
            SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_IS_TRUNK_E,
                hwValue);

            /* PortNum/TrunkNum */
            if(hwValue)
            {
                SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_TRUNK_NUM_E,
                    hwValue);
                dstInterfacePtr->type = CPSS_INTERFACE_TRUNK_E;
                dstInterfacePtr->interfaceInfo.trunkId = (GT_TRUNK_ID)hwValue;
            }
            else
            {
                SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_EPORT_NUM_E,
                    hwValue);
                dstInterfacePtr->type = CPSS_INTERFACE_PORT_E;
                dstInterfacePtr->interfaceInfo.devPort.portNum = hwValue;
            }
        }
    }
    else
    {
        multiple = 1;
    }

    /* MACEntryType = "IPv4" or MACEntryType = "IPv6" or
       (MACEntryType = "MAC" and (MACAddrBit40 = "1" or Multiple = "Multicast")) */
    if (multiple == 1)
    {
        /* VIDX */
        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_VIDX_E,
            hwValue);
        dstInterfacePtr->interfaceInfo.vidx = (GT_TRUNK_ID)hwValue;
        dstInterfacePtr->type = (hwValue == 0xFFF)?CPSS_INTERFACE_VID_E:CPSS_INTERFACE_VIDX_E;
    }
    return GT_OK;
}

/**
* @internal buildDbEntry_prvFdbEntryMacAddrFormat_sip5 function
* @endinternal
*
* @brief  function to build the DB format from HW format.
*         for the fdbEntryMacAddrFormat
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] entryPtr               - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman; AC5;.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildDbEntry_prvFdbEntryMacAddrFormat_sip5
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC     *entryPtr
)
{
    GT_U32                      hwValue;
    GT_BOOL                     saMirrorToRxAnalyzerPortEn;
    GT_BOOL                     daMirrorToRxAnalyzerPortEn;
    GT_BOOL                     tag1VidFdbEn = GT_FALSE;
    GT_U32                     *hwDataArr = &fdbManagerPtr->tempInfo.sip6_hwFormatWords[0];
    GT_U32                      maxSourceId;

    /* age */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_AGE_E,
            hwValue);
    entryPtr->age = BIT2BOOL_MAC(hwValue);

    /* FID */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_FID_E,
        hwValue);
    entryPtr->fid = hwValue;

    /* MAC address */
    SIP5_FDB_HW_ENTRY_FIELD_MAC_ADDR_GET_MAC(hwDataArr, entryPtr->macAddr.arEther);

    /* SA cmd */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SA_CMD_E,
            hwValue);
    /* SIP6 device in case of Hybrid model, do not support "entryAttrInfo.saDropCommand", to align with other supported device.
     * So the drop type is chosen to SOFT in that case*/
    if(fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E)
    {
        entryPtr->saCommand = (hwValue == 1)?CPSS_PACKET_CMD_DROP_SOFT_E:CPSS_PACKET_CMD_FORWARD_E;
    }
    else
    {
        PRV_CPSS_DXCH_FDB_CONVERT_HW_VAL_TO_SW_CMD_MAC(entryPtr->saCommand, hwValue);
    }

    /* DA cmd */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DA_CMD_E,
            hwValue);
    PRV_CPSS_DXCH_FDB_CONVERT_HW_VAL_TO_SW_CMD_MAC(entryPtr->daCommand, hwValue);

    /* static */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_IS_STATIC_E,
        hwValue);
    entryPtr->isStatic = BIT2BOOL_MAC(hwValue);

    /* DA Route */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_DA_ROUTE_E,
        hwValue);
    entryPtr->daRoute = BIT2BOOL_MAC(hwValue);

    /* SA QoS Profile index */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SA_QOS_PARAM_SET_IDX_E,
            hwValue);
    entryPtr->saQoSParameterSetIndex = hwValue;

    /* DA QoS Profile index */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DA_QOS_PARAM_SET_IDX_E,
            hwValue);
    entryPtr->daQoSParameterSetIndex = hwValue;

    /* SA Lookup Ingress Mirror to Analyzer Enable */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E,
            hwValue);
    saMirrorToRxAnalyzerPortEn = BIT2BOOL_MAC(hwValue);

    /* DA Lookup Ingress Mirror to Analyzer Enable */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E,
            hwValue);
    daMirrorToRxAnalyzerPortEn = BIT2BOOL_MAC(hwValue);
    if((saMirrorToRxAnalyzerPortEn == GT_TRUE) && (daMirrorToRxAnalyzerPortEn == GT_TRUE))
    {
        entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E;
    }
    else if(saMirrorToRxAnalyzerPortEn == GT_TRUE)
    {
        entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_E;
    }
    else if(daMirrorToRxAnalyzerPortEn == GT_TRUE)
    {
        entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_DA_E;
    }
    else
    {
        entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E;
    }

    /* Application Specific CPU Code */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E,
            hwValue);
    entryPtr->appSpecificCpuCode = BIT2BOOL_MAC(hwValue);

    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_USER_DEFINED_E,
        hwValue);
    entryPtr->userDefined = hwValue;

    switch (fdbManagerPtr->entryAttrInfo.entryMuxingMode)
    {
        default:
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E:
            maxSourceId      = BIT_9;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E:
            maxSourceId      = BIT_12;
            entryPtr->userDefined &= 0x1f; /*only 5 bits for the UDB, in case 12 Bits srcId*/
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E:
            maxSourceId      = BIT_6;
            tag1VidFdbEn     = GT_TRUE;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E:
            maxSourceId      = BIT_6;
            tag1VidFdbEn     = GT_TRUE;
            entryPtr->userDefined &= 0x1f; /*only 5 bits for the UDB, in case 12 Bits srcId*/
            break;
    }
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_5_0_E,
            hwValue);
    entryPtr->sourceID = hwValue;

    if(maxSourceId >= BIT_9)
    {
        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_8_6_E,
                hwValue);
        U32_SET_FIELD_MAC(entryPtr->sourceID, 6, 3, hwValue);
    }

    if(maxSourceId >= BIT_12)
    {
        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_11_9_E,
                hwValue);
        U32_SET_FIELD_MAC(entryPtr->sourceID, 9, 3, hwValue);
    }

    if (tag1VidFdbEn == GT_TRUE)
    {
        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_ORIG_VID1_E,
            hwValue);
        entryPtr->vid1 = hwValue;
        entryPtr->saSecurityLevel = 0;
        entryPtr->daSecurityLevel = 0;
    }
    else
    {
        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL_E,
            hwValue);
        entryPtr->daSecurityLevel = hwValue;

        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SA_ACCESS_LEVEL_E,
            hwValue);
        entryPtr->saSecurityLevel = hwValue;
        entryPtr->vid1 = 0;
    }

    return buildDbEntry_dstInterface_macOrIpmc_sip5(GT_TRUE /* MAC entry */,
            hwDataArr,
            &entryPtr->dstInterface);
}

/**
* @internal buildDbEntry_RouteInfo_prvIpvxUcEntryFormat_sip5 function
* @endinternal
*
* @brief  function to build the DB format from HW format.
*         for the prvIpv4UcEntryFormat
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
*         based on buildFdbUcRouteEntryHwFormatSip6
*
* @param[in] hwDataArr              - (pointer to) hwdata.
* @param[in] hwFormatType           - FDB Entry hw format type
* @param[in] entryPtr               - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman; AC5;.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildDbEntry_RouteInfo_prvIpvxUcEntryFormat_sip5
(
    IN  GT_U32                                                          *hwDataArr,
    IN  PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT                 hwFormatType,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT     *nextHopInfoPtr
)
{
    GT_U32                      hwValue;

    /* ttl/hop decrement enable */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DEC_TTL_OR_HOP_COUNT_E,
            hwValue);
    nextHopInfoPtr->fullFdbInfo.ttlHopLimitDecEnable = BIT2BOOL_MAC(hwValue);

    /* Bypass TTL Options Or Hop Extension */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E,
            hwValue);
    nextHopInfoPtr->fullFdbInfo.ttlHopLimDecOptionsExtChkByPass = BIT2BOOL_MAC(hwValue);

    if (nextHopInfoPtr->fullFdbInfo.ttlHopLimitDecEnable && nextHopInfoPtr->fullFdbInfo.ttlHopLimDecOptionsExtChkByPass)
    {
        /* It is not legal to configure bypass the TTL/Hop-Limit Check and to enable the
           TTL/Hop-Limit to be decremented. */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* counter set */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_COUNTER_SET_INDEX_E,
            hwValue);

    switch(hwValue)
    {
        case 0:
            nextHopInfoPtr->fullFdbInfo.countSet = CPSS_IP_CNT_SET0_E;
            break;
        case 1:
            nextHopInfoPtr->fullFdbInfo.countSet = CPSS_IP_CNT_SET1_E;
            break;
        case 2:
            nextHopInfoPtr->fullFdbInfo.countSet = CPSS_IP_CNT_SET2_E;
            break;
        case 3:
            nextHopInfoPtr->fullFdbInfo.countSet = CPSS_IP_CNT_SET3_E;
            break;
        case 7:
            nextHopInfoPtr->fullFdbInfo.countSet = CPSS_IP_CNT_NO_SET_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* ICMP redirect enable */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_ICMP_REDIRECT_EXCEP_MIRROR_EN_E,
            hwValue);
    nextHopInfoPtr->fullFdbInfo.ICMPRedirectEnable = BIT2BOOL_MAC(hwValue);

    /* mtu profile index */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_MTU_INDEX_E,
            hwValue);
    nextHopInfoPtr->fullFdbInfo.mtuProfileIndex = hwValue;

    /* get vlan associated with this entry */
    if(hwFormatType == PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_20_E)
    {
        SIP5_20_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_NEXT_HOP_EVLAN_E,
                hwValue);
    }
    else
    {
        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_NEXT_HOP_EVLAN_E,
                hwValue);
    }
    nextHopInfoPtr->fullFdbInfo.nextHopVlanId = (GT_U16)hwValue;

    /* use VIDX */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_USE_VIDX_E,
            hwValue);
    if (hwValue == 1)
    {
        /* get vidx value */
        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_VIDX_E,
                hwValue);
        nextHopInfoPtr->fullFdbInfo.dstInterface.type = (hwValue == 0xFFF)?CPSS_INTERFACE_VID_E:CPSS_INTERFACE_VIDX_E;
        nextHopInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.vidx = (GT_U16)hwValue;
    }
    else
    {
        /* check if target is trunk */
        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_IS_TRUNK_E,
                hwValue);
        if (hwValue == 1)
        {
            /* TRUNK */
            nextHopInfoPtr->fullFdbInfo.dstInterface.type = CPSS_INTERFACE_TRUNK_E;
            /* get trunk_ID value */
            SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_TRUNK_NUM_E,
                    hwValue);
            nextHopInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.trunkId = (GT_TRUNK_ID)hwValue;
        }
        else
        {
            /* PORT */
            nextHopInfoPtr->fullFdbInfo.dstInterface.type = CPSS_INTERFACE_PORT_E;
            /* get target device */
            if(hwFormatType == PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_20_E)
            {
                SIP5_20_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                        SIP5_FDB_FDB_TABLE_FIELDS_TARGET_DEVICE_E,
                        hwValue);
                nextHopInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.devPort.hwDevNum = hwValue;
                /* get target port */
                SIP5_20_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                        SIP5_FDB_FDB_TABLE_FIELDS_EPORT_NUM_E,
                        hwValue);
                nextHopInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.devPort.portNum = hwValue;
            }
            else
            {
                SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                        SIP5_FDB_FDB_TABLE_FIELDS_TARGET_DEVICE_E,
                        hwValue);
                nextHopInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.devPort.hwDevNum = hwValue;
                /* get target port */
                SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                        SIP5_FDB_FDB_TABLE_FIELDS_EPORT_NUM_E,
                        hwValue);
                nextHopInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.devPort.portNum = hwValue;
            }
        }
    }
    return GT_OK;
}

/**
* @internal buildDbEntry_ComonInfo_prvIpvxUcEntryFormat_sip5 function
* @endinternal
*
* @brief  function to build the DB format from HW format.
*         for the prvIpv4UcEntryFormat
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
*         based on buildFdbUcRouteEntryHwFormatSip6
*
* @param[in] hwDataArr              - (pointer to) hwdata.
* @param[in] entryPtr               - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman; AC5;.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildDbEntry_ComonInfo_prvIpvxUcEntryFormat_sip5
(
    IN  GT_U32                                             *hwDataArr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_IP_UC_COMMON_STC         *ipCommonPtr
)
{
    GT_U32                      hwValue;

    /* ingress mirror to analyzer index */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_INGRESS_MIRROR_TO_ANALYZER_INDEX_E,
            hwValue);
    if (hwValue)
    {
        ipCommonPtr->ingressMirrorToAnalyzerIndex  = hwValue - 1;
        ipCommonPtr->ingressMirrorToAnalyzerEnable = GT_TRUE;
    }
    else
    {
        ipCommonPtr->ingressMirrorToAnalyzerEnable = GT_FALSE;
        ipCommonPtr->ingressMirrorToAnalyzerIndex  = 0;
    }

    /* qos profile mark enable */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_QOS_PROFILE_MARKING_EN_E,
            hwValue);
    ipCommonPtr->qosProfileMarkingEnable = BIT2BOOL_MAC(hwValue);

    /* qos profile index */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_QOS_PROFILE_INDEX_E,
            hwValue);
    ipCommonPtr->qosProfileIndex = hwValue;

    /* qos precedence */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_QOS_PROFILE_PRECEDENCE_E,
            hwValue);
    ipCommonPtr->qosProfilePrecedence = (hwValue == 0)?CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E:CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

    /* modify UP */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_MODIFY_UP_E,
            hwValue);

    switch(hwValue)
    {
        case 0:
            ipCommonPtr->modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
            break;
        case 2:
            ipCommonPtr->modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
            break;
        case 1:
            ipCommonPtr->modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* modify DSCP */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_MODIFY_DSCP_E,
            hwValue);

    switch(hwValue)
    {
        case 0:
            ipCommonPtr->modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
            break;
        case 2:
            ipCommonPtr->modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
            break;
        case 1:
            ipCommonPtr->modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* trap mirror arp bc enable */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_ARP_BC_TRAP_MIRROR_EN_E,
            hwValue);
    ipCommonPtr->arpBcTrapMirrorEnable = BIT2BOOL_MAC(hwValue);

    /* dip access level */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DIP_ACCESS_LEVEL_E,
            hwValue);
    ipCommonPtr->dipAccessLevel =  hwValue;
    return GT_OK;
}

/**
* @internal buildDbEntry_prvIpv4UcEntryFormat_sip5 function
* @endinternal
*
* @brief  function to build the DB format from HW format.
*         for the prvIpv4UcEntryFormat
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
*         based on buildFdbUcRouteEntryHwFormatSip6
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] hwFormatType           - FDB Entry hw format type
* @param[in] entryPtr               - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman; AC5;.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildDbEntry_prvIpv4UcEntryFormat_sip5
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC         *fdbManagerPtr,
    IN  PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT        hwFormatType,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC     *entryPtr
)
{
    GT_U32                      hwValue;
    GT_U32                     *hwDataArr = &fdbManagerPtr->tempInfo.sip6_hwFormatWords[0];

    /* IP Addr */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_IPV4_DIP_E,
        hwValue);
    entryPtr->ipv4Addr.arIP[0] = (GT_U8)(hwValue >> 24);
    entryPtr->ipv4Addr.arIP[1] = (GT_U8)(hwValue >> 16);
    entryPtr->ipv4Addr.arIP[2] = (GT_U8)(hwValue >> 8);
    entryPtr->ipv4Addr.arIP[3] = (GT_U8)(hwValue);

    /* age */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_AGE_E,
        hwValue);
    entryPtr->age = BIT2BOOL_MAC(hwValue);

    /* VRF-ID */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_VRF_ID_E,
        hwValue);
    entryPtr->vrfId = hwValue;

    if(hwFormatType == PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_20_E)
    {
        /* check if entry is tunnel start */
        SIP5_20_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_START_E,
                hwValue);
        if (hwValue == 1)
        {
            /* get tunnel ptr */
            SIP5_20_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_PTR_E,
                    hwValue);
            entryPtr->ucRouteType = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E;
            entryPtr->ucRouteInfo.fullFdbInfo.pointer.tunnelStartPointer = hwValue;
        }
        else
        {
            /* get arp ptr */
            SIP5_20_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_ARP_PTR_E,
                    hwValue);
            entryPtr->ucRouteType = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
            entryPtr->ucRouteInfo.fullFdbInfo.pointer.arpPointer = hwValue;
        }
    }
    else
    {
        /* check if entry is tunnel start */
        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_START_E,
                hwValue);
        if (hwValue == 1)
        {
            /* get tunnel ptr */
            SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_PTR_E,
                    hwValue);
            entryPtr->ucRouteType = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E;
            entryPtr->ucRouteInfo.fullFdbInfo.pointer.tunnelStartPointer = hwValue;
        }
        else
        {
            /* get arp ptr */
            SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_ARP_PTR_E,
                    hwValue);
            entryPtr->ucRouteType = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
            entryPtr->ucRouteInfo.fullFdbInfo.pointer.arpPointer = hwValue;
        }
    }

    buildDbEntry_RouteInfo_prvIpvxUcEntryFormat_sip5(hwDataArr, hwFormatType, &entryPtr->ucRouteInfo);
    buildDbEntry_ComonInfo_prvIpvxUcEntryFormat_sip5(hwDataArr, &entryPtr->ucCommonInfo);

    return GT_OK;
}

/**
* @internal buildDbEntry_prvIpv6UcEntryFormat_sip5 function
* @endinternal
*
* @brief  function to build the DB format from HW format.
*         for the prvIpv4UcEntryFormat
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
*         based on buildFdbUcRouteEntryHwFormatSip6
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] hwFormatType           - FDB Entry hw format type
* @param[in] entryPtr               - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman; AC5;.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildDbEntry_prvIpv6UcEntryFormat_sip5
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC         *fdbManagerPtr,
    IN  PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT        hwFormatType,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC     *entryPtr
)
{
    GT_U32                      hwValue;
    GT_U32                      *hwKeyArr  = fdbManagerPtr->tempInfo.sip6_hwFormatWords;
    GT_U32                      *hwDataArr = fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords;
    GT_U32                      wordNum;

    /* get IPv6 UC DIP */
    for(wordNum = 0; wordNum < 4; wordNum++)
    {
        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwKeyArr,
                SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DIP_0_E + wordNum,
                hwValue);
        entryPtr->ipv6Addr.arIP[((3 - wordNum) * 4)]     = (GT_U8)(hwValue >> 24);
        entryPtr->ipv6Addr.arIP[((3 - wordNum) * 4) + 1] = (GT_U8)(hwValue >> 16);
        entryPtr->ipv6Addr.arIP[((3 - wordNum) * 4) + 2] = (GT_U8)(hwValue >> 8);
        entryPtr->ipv6Addr.arIP[((3 - wordNum) * 4) + 3] = (GT_U8)(hwValue);
    }

    /*
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_NH_DATA_BANK_NUM_E,
            hwValue);
    macEntryPtr->fdbRoutingInfo.nextHopDataBankNumber = hwValue;
    */

    /* age */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_AGE_E,
        hwValue);
    entryPtr->age = BIT2BOOL_MAC(hwValue);

    /* VRF-ID */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_VRF_ID_E,
        hwValue);
    entryPtr->vrfId = hwValue;

    if(hwFormatType == PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_20_E)
    {
        /* check if entry is tunnel start */
        SIP5_20_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_START_E,
                hwValue);
        if (hwValue == 1)
        {
            /* get tunnel ptr */
            SIP5_20_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_PTR_E,
                    hwValue);
            entryPtr->ucRouteType = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E;
            entryPtr->ucRouteInfo.fullFdbInfo.pointer.tunnelStartPointer = hwValue;
        }
        else
        {
            /* get arp ptr */
            SIP5_20_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_ARP_PTR_E,
                    hwValue);
            entryPtr->ucRouteType = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
            entryPtr->ucRouteInfo.fullFdbInfo.pointer.arpPointer = hwValue;
        }
    }
    else
    {
        /* check if entry is tunnel start */
        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_START_E,
                hwValue);
        if (hwValue == 1)
        {
            /* get tunnel ptr */
            SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_PTR_E,
                    hwValue);
            entryPtr->ucRouteType = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E;
            entryPtr->ucRouteInfo.fullFdbInfo.pointer.tunnelStartPointer = hwValue;
        }
        else
        {
            /* get arp ptr */
            SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP5_FDB_FDB_TABLE_FIELDS_ARP_PTR_E,
                    hwValue);
            entryPtr->ucRouteType = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
            entryPtr->ucRouteInfo.fullFdbInfo.pointer.arpPointer = hwValue;
        }
    }

    /* scope checking enable */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_IPV6_SCOPE_CHECK_E,
            hwValue);
    entryPtr->ucRouteInfo.fullFdbInfo.ipv6ExtInfo.scopeCheckingEnable = BIT2BOOL_MAC(hwValue);

    /* site Id */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DST_SITE_ID_E,
            hwValue);
    if(hwValue == 0)
    {
        entryPtr->ucRouteInfo.fullFdbInfo.ipv6ExtInfo.siteId = CPSS_IP_SITE_ID_INTERNAL_E;
    }
    else
    {
        entryPtr->ucRouteInfo.fullFdbInfo.ipv6ExtInfo.siteId = CPSS_IP_SITE_ID_EXTERNAL_E;
    }

    buildDbEntry_RouteInfo_prvIpvxUcEntryFormat_sip5(hwDataArr, hwFormatType, &entryPtr->ucRouteInfo);
    buildDbEntry_ComonInfo_prvIpvxUcEntryFormat_sip5(hwDataArr, &entryPtr->ucCommonInfo);

    return GT_OK;
}

/**
* @internal buildDbEntry_prvFdbEntryIpMcAddrFormat_sip5 function
* @endinternal
*
* @brief  function to build the DB format from HW format
*         for the fdbEntryIpv4McFormat and fdbEntryIpv6McFormat
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
* @param[in]  fdbManagerPtr        - (pointer to) the FDB Manager.
* @param[out] entryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman; AC5;.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildDbEntry_prvFdbEntryIpMcAddrFormat_sip5
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC        *entryPtr
)
{
    GT_U32                   hwValue;
    GT_BOOL                  saMirrorToRxAnalyzerPortEn;
    GT_BOOL                  daMirrorToRxAnalyzerPortEn;
    GT_BOOL                  tag1VidFdbEn = GT_FALSE;
    GT_U32                  *hwDataArr = fdbManagerPtr->tempInfo.sip6_hwFormatWords;
    GT_U32                   maxSourceId;

    /* age */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_AGE_E,
        hwValue);
    entryPtr->age = hwValue;

    /* FID */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_FID_E,
        hwValue);
    entryPtr->fid = hwValue;

    /* get DIP Address */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DIP_E,
            hwValue);

    entryPtr->dipAddr[3] = (GT_U8)(hwValue >> 0);
    entryPtr->dipAddr[2] = (GT_U8)(hwValue >> 8);
    entryPtr->dipAddr[1] = (GT_U8)(hwValue >> 16);
    entryPtr->dipAddr[0] = (GT_U8)(hwValue >> 24);

    /* get SIP Address */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SIP_E,
            hwValue);
    entryPtr->sipAddr[3] = (GT_U8)(hwValue >> 0);
    entryPtr->sipAddr[2] = (GT_U8)(hwValue >> 8);
    entryPtr->sipAddr[1] = (GT_U8)(hwValue >> 16);
    entryPtr->sipAddr[0] = (GT_U8)(hwValue >> 24);

    /* set DA cmd */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DA_CMD_E,
            hwValue);
    PRV_CPSS_DXCH_FDB_CONVERT_HW_VAL_TO_SW_CMD_MAC(entryPtr->daCommand,
            hwValue);

    /* set static */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_IS_STATIC_E,
            hwValue);
    entryPtr->isStatic = BIT2BOOL_MAC(hwValue);

    /* DA Route */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DA_ROUTE_E,
            hwValue);
    entryPtr->daRoute = BIT2BOOL_MAC(hwValue);

    /* SA QoS Profile index */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SA_QOS_PARAM_SET_IDX_E,
            hwValue);
    entryPtr->saQoSParameterSetIndex = hwValue;

    /* DA QoS Profile index */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DA_QOS_PARAM_SET_IDX_E,
            hwValue);
    entryPtr->daQoSParameterSetIndex = hwValue;

    /* SA Lookup Ingress Mirror to Analyzer Enable */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E,
            hwValue);
    saMirrorToRxAnalyzerPortEn = BIT2BOOL_MAC(hwValue);

    /* DA Lookup Ingress Mirror to Analyzer Enable */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E,
            hwValue);
    daMirrorToRxAnalyzerPortEn = BIT2BOOL_MAC(hwValue);
    if((saMirrorToRxAnalyzerPortEn == GT_TRUE) && (daMirrorToRxAnalyzerPortEn == GT_TRUE))
    {
        entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E;
    }
    else if(saMirrorToRxAnalyzerPortEn == GT_TRUE)
    {
        entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_E;
    }
    else if(daMirrorToRxAnalyzerPortEn == GT_TRUE)
    {
        entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_DA_E;
    }
    else
    {
        entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E;
    }

    /* Application Specific CPU Code */
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E,
            hwValue);
    entryPtr->appSpecificCpuCode = BIT2BOOL_MAC(hwValue);

    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_USER_DEFINED_E,
        hwValue);
    entryPtr->userDefined = hwValue;

    switch (fdbManagerPtr->entryAttrInfo.entryMuxingMode)
    {
        default:
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E:
            maxSourceId      = BIT_9;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E:
            maxSourceId      = BIT_12;
            entryPtr->userDefined &= 0x1f; /* only 5 bits for the UDB */
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E:
            maxSourceId      = BIT_6;
            tag1VidFdbEn     = GT_TRUE;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E:
            maxSourceId      = BIT_6;
            tag1VidFdbEn     = GT_TRUE;
            entryPtr->userDefined &= 0x1f; /* only 5 bits for the UDB */
            break;
    }

    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_5_0_E,
            hwValue);
    entryPtr->sourceID = hwValue;

    if(maxSourceId >= BIT_9)
    {
        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_8_6_E,
                hwValue);
        U32_SET_FIELD_MAC(entryPtr->sourceID, 6, 3, hwValue);
    }

    if(maxSourceId >= BIT_12)
    {
        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_11_9_E,
                hwValue);
        U32_SET_FIELD_MAC(entryPtr->sourceID, 9, 3, hwValue);
    }

    if (tag1VidFdbEn == GT_TRUE)
    {
        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_ORIG_VID1_E,
                hwValue);
        entryPtr->vid1 = hwValue;
        entryPtr->daSecurityLevel = 0;
    }
    else
    {
        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL_E,
            hwValue);
        entryPtr->daSecurityLevel = hwValue;
        entryPtr->vid1 = 0;
    }

    return buildDbEntry_dstInterface_macOrIpmc_sip5(GT_FALSE /* MAC entry */,
            hwDataArr,
            &entryPtr->dstInterface);
}

/**
* @internal prvCpssDxChFdbManagerDbBuildDbFormatFromHwEntry_sip5 function
* @endinternal
*
* @brief  function to build the DB entry
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] hwFormatType          - FDB Entry hw format type
* @param[out] entryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman; AC5;.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS   prvCpssDxChFdbManagerDbBuildDbFormatFromHwEntry_sip5
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN  PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT  hwFormatType,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr
)
{
    GT_U32                  hwValue;

    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(&fdbManagerPtr->tempInfo.sip6_hwFormatWords[0],
            SIP5_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
            hwValue);
    switch(hwValue)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
            entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;
            return buildDbEntry_prvFdbEntryMacAddrFormat_sip5(fdbManagerPtr,
                    &entryPtr->format.fdbEntryMacAddrFormat);

        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
            entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E;
            return buildDbEntry_prvIpv4UcEntryFormat_sip5(fdbManagerPtr,
                    hwFormatType,
                    &entryPtr->format.fdbEntryIpv4UcFormat);

        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
            entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E;
            return buildDbEntry_prvIpv6UcEntryFormat_sip5(fdbManagerPtr,
                    hwFormatType,
                    &entryPtr->format.fdbEntryIpv6UcFormat);

        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
            entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E;
            return buildDbEntry_prvFdbEntryIpMcAddrFormat_sip5(fdbManagerPtr,
                    &entryPtr->format.fdbEntryIpv4McFormat);

        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
            entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E;
            return buildDbEntry_prvFdbEntryIpMcAddrFormat_sip5(fdbManagerPtr,
                    &entryPtr->format.fdbEntryIpv6McFormat);

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(hwValue);
    }
}

/**
* @internal prvCpssDxChFdbManagerDbBuildDbFormatFromHwEntry_sip4 function
* @endinternal
*
* @brief  function to build the DB entry
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[out] entryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS   prvCpssDxChFdbManagerDbBuildDbFormatFromHwEntry_sip4
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr
)
{
    GT_U32                  hwValue;

    hwValue = U32_GET_FIELD_MAC(fdbManagerPtr->tempInfo.sip6_hwFormatWords[0],3,2);
    switch(hwValue)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
            entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;
            return buildDbEntry_prvFdbEntryMacAddrFormat_sip4(fdbManagerPtr,
                    &entryPtr->format.fdbEntryMacAddrFormat);
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
            entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E;
            return buildDbEntry_prvFdbEntryIpMcAddrFormat_sip4(fdbManagerPtr,
                    &entryPtr->format.fdbEntryIpv4McFormat);
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
            entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E;
            return buildDbEntry_prvFdbEntryIpMcAddrFormat_sip4(fdbManagerPtr,
                    &entryPtr->format.fdbEntryIpv6McFormat);
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(hwValue);
    }
}

/**
* @internal prvCpssDxChFdbManagerDbBuildDbFormatFromHwEntry function
* @endinternal
*
* @brief  function to build the DB entry
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] hwFormatType          - FDB Entry hw format type
* @param[out] entryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS   prvCpssDxChFdbManagerDbBuildDbFormatFromHwEntry
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN  PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT  hwFormatType,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr
)
{
    GT_U32                  hwValue;

    switch(hwFormatType)
    {
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP4_E:
            return prvCpssDxChFdbManagerDbBuildDbFormatFromHwEntry_sip4(fdbManagerPtr, entryPtr);
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_20_E:
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_E:
            return prvCpssDxChFdbManagerDbBuildDbFormatFromHwEntry_sip5(fdbManagerPtr,
                    hwFormatType,
                    entryPtr);
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP6_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(hwFormatType);
    }

    SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(&fdbManagerPtr->tempInfo.sip6_hwFormatWords[0],
            SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
            hwValue);
    switch(hwValue)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
            entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;
            return buildDbEntry_prvFdbEntryMacAddrFormat(fdbManagerPtr,
                    &entryPtr->format.fdbEntryMacAddrFormat);

        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
            entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E;
            return buildDbEntry_prvIpv4UcEntryFormat(fdbManagerPtr,
                    &entryPtr->format.fdbEntryIpv4UcFormat);

        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
            entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E;
            return buildDbEntry_prvIpv6UcEntryFormat(fdbManagerPtr,
                    &entryPtr->format.fdbEntryIpv6UcFormat);

        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
            entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E;
            return buildDbEntry_prvFdbEntryIpMcAddrFormat(fdbManagerPtr,
                    &entryPtr->format.fdbEntryIpv4McFormat);

        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
            entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E;
            return buildDbEntry_prvFdbEntryIpMcAddrFormat(fdbManagerPtr,
                    &entryPtr->format.fdbEntryIpv6McFormat);

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(hwValue);
    }
}

/**
* @internal prvCpssDxChFdbManagerDbFreeAgeBinIndexGet function
* @endinternal
*
* @brief  The function gets the next free ageBinIndex.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] ageBinIndex           - (pointer to) the agebin index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - not found bank to use
*/
static GT_STATUS prvCpssDxChFdbManagerDbFreeAgeBinIndexGet
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr,
    OUT GT_U32                                          *ageBinIndex
)
{
    GT_U32          level1Index, level2Index;
    GT_U32          level1Size;

    /* check the Right Most unset Bit */
    GET_LSB_BIT_RESET_POSITION_MAC(fdbManagerPtr->agingBinInfo.level2UsageBitmap, level2Index);

    level1Size = 1 + (fdbManagerPtr->agingBinInfo.totalAgeBinAllocated >> 5);
    /* Index validation - Level1 array index should not be more than allocated size */
    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(level2Index, level1Size);

    /* level2 bit position is the index to the level1Bitmap array */
    GET_LSB_BIT_RESET_POSITION_MAC(fdbManagerPtr->agingBinInfo.level1UsageBitmapPtr[level2Index], level1Index);

    *ageBinIndex = (level2Index * 32) + level1Index;
    return GT_OK;
}

/**
* @internal validateNewManagerParams function
* @endinternal
*
* @brief  The function validate 'API level' 'manager create' parameters.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerId          - the FDB Manager id to associate with the newly created manager.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] capacityPtr           - (pointer to) parameters of the HW structure and capacity of its managed PPs FDBs –
*                                    like number of indexes, the amount of hashes etc.
*                                    As FDB Manager is created and entries may be added before PP registered –
*                                    we must know in advance the relevant capacity –
*                                    in Falcon for example we must know the shared memory allocation mode.
* @param[in] entryAttrPtr          - (pointer to) parameters that affects how HW/SW entries are
*                                    structured and specific fields behavior – including:
*                                    MUX'd fields in MAC / IPMC entries format, SA / DA drop commands mode &
*                                    IP NH packet command.
* @param[in] learningPtr           - (pointer to) parameters that affects which sort of entries to learn etc.
*                                    (Learn MAC of Routed packets and MAC no-space updates).
* @param[in] lookupPtr             - (pointer to) parameters that affect lookup of entries: MAC VLAN Lookup mode
*                                    (none, single or double tags), IVL single-tag MSB mode.
*                                    And two new Falcon setting: IPv4 UC and IPv6 UC masks
*                                    (allow using FDB for prefixes lookup and not just exact match)
* @param[in] agingPtr              - (pointer to) parameters that affect refresh and aging
*                                      (Refresh Destination UC, Refresh Destination MC &Refresh IP UC)
* @param[in] dbExist               - Indicates db already exist or not
*                                    GT_TRUE  - DB Exist
*                                    GT_FALSE - Not Exist
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - if the FDB Manager id already exists.
*/
static GT_STATUS    validateNewManagerParams
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           *capacityPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             *lookupPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              *agingPtr,
    IN GT_BOOL                                          dbExist
)
{
    CPSS_NULL_PTR_CHECK_MAC(capacityPtr);
    CPSS_NULL_PTR_CHECK_MAC(entryAttrPtr);
    CPSS_NULL_PTR_CHECK_MAC(learningPtr);
    CPSS_NULL_PTR_CHECK_MAC(lookupPtr);
    CPSS_NULL_PTR_CHECK_MAC(agingPtr);

    if(fdbManagerId >= CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "fdbManagerId [%d] >= [%d] (out of range)",
            fdbManagerId,
            CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS);
    }

    if((dbExist == GT_FALSE) && PRV_SHARED_FDB_MANAGER_DB_VAR_GET(prvCpssDxChFdbManagerDbArr)[fdbManagerId])
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST,
            "fdbManagerId [%d] already exists",
            fdbManagerId);
    }

    BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(capacityPtr->hwCapacity.numOfHwIndexes,prvCpssDxChBrgFdbSizeSet);
    /** number of indexes in HW */
    switch(capacityPtr->hwCapacity.numOfHwIndexes)
    {
        /*case   _4K:*/
        case   _8K:
        case  _16K:
            break;
        case  _32K:
        case  _64K:
        case _128K:
        case _256K:
            if(entryAttrPtr->shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(capacityPtr->hwCapacity.numOfHwIndexes);
            }
            break;
        /*case _512K:*/
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(capacityPtr->hwCapacity.numOfHwIndexes);
    }

    if(capacityPtr->hwCapacity.numOfHwIndexes > (1 << HW_INDEX_NUM_BITS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
            "numOfHwIndexes[%d] is more than the 'manager' supports[%d] (need to set new value to HW_INDEX_NUM_BITS)",
            capacityPtr->hwCapacity.numOfHwIndexes,
            (1 << HW_INDEX_NUM_BITS));
    }

    /* Supported Hash check based on the shadow type */
    switch(entryAttrPtr->shadowType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E:
            if(capacityPtr->hwCapacity.numOfHashes != 1)
            {
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(capacityPtr->hwCapacity.numOfHashes);
            }

            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(capacityPtr->hwCapacity.mode, cpssDxChBrgFdbHashModeSet);
            /** Hash mode - Valid in case of SIP4 devices */
            if((capacityPtr->hwCapacity.mode != CPSS_MAC_HASH_FUNC_XOR_E) &&
                    (capacityPtr->hwCapacity.mode != CPSS_MAC_HASH_FUNC_CRC_E))
            {
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(capacityPtr->hwCapacity.mode);
            }
            /* No Space update messages is not supported by SIP4 devices */
            learningPtr->macNoSpaceUpdatesEnable = GT_FALSE;
            break;

        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E:
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E:
            if(capacityPtr->hwCapacity.numOfHashes != 16)
            {
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(capacityPtr->hwCapacity.numOfHashes);
            }

            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(entryAttrPtr->ipNhPacketcommand,cpssDxChBrgFdbRoutingNextHopPacketCmdSet);
            switch(entryAttrPtr->ipNhPacketcommand)
            {
                case CPSS_PACKET_CMD_ROUTE_E:
                case CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E:
                case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
                case CPSS_PACKET_CMD_DROP_HARD_E:
                case CPSS_PACKET_CMD_DROP_SOFT_E:
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryAttrPtr->ipNhPacketcommand);
            }
            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(agingPtr->destinationUcRefreshEnable , prvCpssDxChBrgFdbAgeBitDaRefreshEnableSet);
            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(agingPtr->ipUcRefreshEnable , cpssDxChBrgFdbRoutingUcRefreshEnableSet);

            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupPtr->ipv4PrefixLength,cpssDxChIpFdbRoutePrefixLenSet);
            if(lookupPtr->ipv4PrefixLength > 32)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "ipv4PrefixLength[%d] must be in range[0..32]",
                        lookupPtr->ipv4PrefixLength);
            }

            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupPtr->ipv6PrefixLength,cpssDxChIpFdbRoutePrefixLenSet);
            if(lookupPtr->ipv6PrefixLength > 128)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "ipv6PrefixLength[%d] must be in range[0..128]",
                        lookupPtr->ipv6PrefixLength);
            }

            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupPtr->crcHashUpperBitsMode,cpssDxChBrgFdbCrcHashUpperBitsModeSet);
            switch(lookupPtr->crcHashUpperBitsMode)
            {
                case CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E:
                case CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_USE_FID_E:
                case CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_USE_MAC_E:
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(lookupPtr->crcHashUpperBitsMode);
            }
            break;

        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E:
            switch(capacityPtr->hwCapacity.numOfHashes)
            {
                case 4:
                case 8:
                case 16:
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(capacityPtr->hwCapacity.numOfHashes);
            }
            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(entryAttrPtr->macEntryMuxingMode,cpssDxChBrgFdbMacEntryMuxingModeSet);
            switch(entryAttrPtr->macEntryMuxingMode)
            {
                case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E:
                case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E:
                case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_UDB_10_SRC_ID_1_DA_ACCESS_LEVEL_E:
                case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_UDB_8_SRC_ID_3_DA_ACCESS_LEVEL_E:
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryAttrPtr->macEntryMuxingMode);
            }

            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(entryAttrPtr->ipmcEntryMuxingMode,cpssDxChBrgFdbIpmcEntryMuxingModeSet);
            switch(entryAttrPtr->ipmcEntryMuxingMode)
            {
                case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E:
                case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_4_SRC_ID_3_E:
                case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_E:
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryAttrPtr->ipmcEntryMuxingMode);
            }

            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(entryAttrPtr->saDropCommand,cpssDxChBrgFdbSaDaCmdDropModeSet);
            /* In case of SIP4 & SIP5 this global param is moved to per entry */
            switch(entryAttrPtr->saDropCommand)
            {
                case CPSS_PACKET_CMD_DROP_HARD_E:
                case CPSS_PACKET_CMD_DROP_SOFT_E:
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryAttrPtr->saDropCommand);
            }

            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(entryAttrPtr->daDropCommand,cpssDxChBrgFdbSaDaCmdDropModeSet);
            /* In case of SIP4 & SIP5 this global param is moved to per entry */
            switch(entryAttrPtr->daDropCommand)
            {
                case CPSS_PACKET_CMD_DROP_HARD_E:
                case CPSS_PACKET_CMD_DROP_SOFT_E:
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryAttrPtr->daDropCommand);
            }
            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(entryAttrPtr->ipNhPacketcommand,cpssDxChBrgFdbRoutingNextHopPacketCmdSet);
            switch(entryAttrPtr->ipNhPacketcommand)
            {
                case CPSS_PACKET_CMD_ROUTE_E:
                case CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E:
                case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
                case CPSS_PACKET_CMD_DROP_HARD_E:
                case CPSS_PACKET_CMD_DROP_SOFT_E:
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryAttrPtr->ipNhPacketcommand);
            }

            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupPtr->ipv4PrefixLength,cpssDxChIpFdbRoutePrefixLenSet);
            if(lookupPtr->ipv4PrefixLength > 32)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "ipv4PrefixLength[%d] must be in range[0..32]",
                        lookupPtr->ipv4PrefixLength);
            }

            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupPtr->ipv6PrefixLength,cpssDxChIpFdbRoutePrefixLenSet);
            if(lookupPtr->ipv6PrefixLength > 128)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "ipv6PrefixLength[%d] must be in range[0..128]",
                        lookupPtr->ipv6PrefixLength);
            }

            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupPtr->crcHashUpperBitsMode,cpssDxChBrgFdbCrcHashUpperBitsModeSet);
            switch(lookupPtr->crcHashUpperBitsMode)
            {
                case CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E:
                case CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_USE_FID_E:
                case CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_USE_MAC_E:
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(lookupPtr->crcHashUpperBitsMode);
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(capacityPtr->hwCapacity.numOfHashes);
    }

    /* Common field validation:
     * fields are valid for SIP5, SIP5_SIP6_HYBRID, SIP6 DB */
    switch(learningPtr->macVlanLookupMode)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_ONLY_E:
        case CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E:
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E:
            if(entryAttrPtr->shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(learningPtr->macVlanLookupMode);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(learningPtr->macVlanLookupMode);
    }

    /* check values  */
    NUM_ENTRIES_STEPS_CHECK_MAC(capacityPtr->maxTotalEntries);
    NUM_ENTRIES_STEPS_CHECK_MAC(capacityPtr->maxEntriesPerAgingScan);
    NUM_ENTRIES_STEPS_CHECK_MAC(capacityPtr->maxEntriesPerDeleteScan);
    NUM_ENTRIES_STEPS_CHECK_MAC(capacityPtr->maxEntriesPerTransplantScan);
    NUM_ENTRIES_STEPS_CHECK_MAC(capacityPtr->maxEntriesPerLearningScan);

    /* check that values make since */
    CHECK_X_NOT_MORE_THAN_Y_MAC(capacityPtr->maxTotalEntries            ,capacityPtr->hwCapacity.numOfHwIndexes);
    CHECK_X_NOT_MORE_THAN_Y_MAC(capacityPtr->maxEntriesPerAgingScan     ,capacityPtr->hwCapacity.numOfHwIndexes);
    CHECK_X_NOT_MORE_THAN_Y_MAC(capacityPtr->maxEntriesPerDeleteScan    ,capacityPtr->hwCapacity.numOfHwIndexes);
    CHECK_X_NOT_MORE_THAN_Y_MAC(capacityPtr->maxEntriesPerTransplantScan,capacityPtr->hwCapacity.numOfHwIndexes);
    CHECK_X_NOT_MORE_THAN_Y_MAC(capacityPtr->maxEntriesPerLearningScan  ,capacityPtr->hwCapacity.numOfHwIndexes);

    BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(learningPtr->macNoSpaceUpdatesEnable,cpssDxChBrgFdbNaMsgOnChainTooLongSet);
    BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(learningPtr->macRoutedLearningEnable,cpssDxChBrgFdbRoutedLearningEnableSet);
    BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(learningPtr->macVlanLookupMode,cpssDxChBrgFdbMacVlanLookupModeSet);
    BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(agingPtr->destinationMcRefreshEnable , prvCpssDxChBrgFdbAgeBitDaRefreshEnableSet);
    return GT_OK;
}

/**
* @internal initNewManager function
* @endinternal
*
* @brief  The function init 'manager' needed DBs (and allocate needed large arrays)
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerId          - the FDB Manager id to associate with the newly created manager.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_OUT_OF_CPU_MEM        - on fail to do Cpu memory allocation.
*/
static GT_STATUS    initNewManager
(
    IN GT_U32                                           fdbManagerId
)
{
    GT_U32    numBytesToAlloc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC *currEntryPtr;
    GT_U32  ii,iiMax;
    GT_U32  numGlobalEports;

    numBytesToAlloc = fdbManagerPtr->capacityInfo.maxTotalEntries * sizeof(PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC);
    MEM_CALLOC_MAC(fdbManagerId,fdbManagerPtr->entryPoolPtr,numBytesToAlloc);

    numBytesToAlloc = fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes * sizeof(PRV_CPSS_DXCH_FDB_MANAGER_DB_INDEX_STC);
    MEM_CALLOC_MAC(fdbManagerId,fdbManagerPtr->indexArr,numBytesToAlloc);

    currEntryPtr = &fdbManagerPtr->entryPoolPtr[0];

    iiMax = fdbManagerPtr->capacityInfo.maxTotalEntries;

    /* init the 'linked list' of 'free list' */
    for(ii = 0 ; ii < iiMax; ii++,currEntryPtr++)
    {
        if(ii != 0)
        {
            currEntryPtr->isValid_prevEntryPointer = 1;
            currEntryPtr->prevEntryPointer         = ii - 1;
        }

        if(ii != (iiMax-1))
        {
            currEntryPtr->isValid_nextEntryPointer = 1;
            /*currEntryPtr->nextEntryPointer         = ii + 1;*/
            SET_nextEntryPointer_MAC(currEntryPtr,(ii + 1));
        }
    }

    /* init the head of free list */
    fdbManagerPtr->headOfFreeList.isValid_entryPointer = 1;/* validate the head */
    fdbManagerPtr->headOfFreeList.entryPointer         = 0;/* point to the first index in entryPoolPtr */

    /* init the head of used list */
    fdbManagerPtr->headOfUsedList.isValid_entryPointer = 0;/* no 'used' entries at this point */
    fdbManagerPtr->headOfUsedList.entryPointer         = 0;/* don't care */

    /* init the tail of used list */
    fdbManagerPtr->tailOfUsedList.isValid_entryPointer = 0;/* no 'used' entries at this point */
    fdbManagerPtr->tailOfUsedList.entryPointer         = 0;/* don't care */

    fdbManagerPtr->cpssHashParams.vlanMode =
        (fdbManagerPtr->learningInfo.macVlanLookupMode ==
         CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_ONLY_E) ?
         CPSS_SVL_E : CPSS_IVL_E;
    switch(fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes)
    {
        case   _8K:fdbManagerPtr->cpssHashParams.size =   CPSS_DXCH_BRG_FDB_TBL_SIZE_8K_E; break;
        case  _16K:fdbManagerPtr->cpssHashParams.size =  CPSS_DXCH_BRG_FDB_TBL_SIZE_16K_E; break;
        case  _32K:fdbManagerPtr->cpssHashParams.size =  CPSS_DXCH_BRG_FDB_TBL_SIZE_32K_E; break;
        case  _64K:fdbManagerPtr->cpssHashParams.size =  CPSS_DXCH_BRG_FDB_TBL_SIZE_64K_E; break;
        case _128K:fdbManagerPtr->cpssHashParams.size = CPSS_DXCH_BRG_FDB_TBL_SIZE_128K_E; break;
        case _256K:fdbManagerPtr->cpssHashParams.size = CPSS_DXCH_BRG_FDB_TBL_SIZE_256K_E; break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);
    }

    switch(fdbManagerPtr->entryAttrInfo.shadowType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E:
            BMP_SET_MAC(fdbManagerPtr->supportedSipArr, CPSS_GEN_SIP_LEGACY_E);
            fdbManagerPtr->cpssHashParams.numOfMultipleHashes = 4;
            break;

        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E:
            BMP_SET_MAC(fdbManagerPtr->supportedSipArr, CPSS_GEN_SIP_5_E);
            BMP_SET_MAC(fdbManagerPtr->supportedSipArr, CPSS_GEN_SIP_5_20_E);
            fdbManagerPtr->cpssHashParams.fid16BitHashEn       = GT_TRUE;
            fdbManagerPtr->cpssHashParams.crcHashUpperBitsMode = (GT_U32)fdbManagerPtr->lookupInfo.crcHashUpperBitsMode;
            break;

        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E:
            /* SIP5 & SIP6 devices can be added to this FDB manager ! */
            BMP_SET_MAC(fdbManagerPtr->supportedSipArr, CPSS_GEN_SIP_6_E);
            BMP_SET_MAC(fdbManagerPtr->supportedSipArr, CPSS_GEN_SIP_5_20_E);
            BMP_SET_MAC(fdbManagerPtr->supportedSipArr, CPSS_GEN_SIP_5_E);
            fdbManagerPtr->cpssHashParams.fid16BitHashEn       = GT_TRUE;
            fdbManagerPtr->cpssHashParams.crcHashUpperBitsMode = (GT_U32)fdbManagerPtr->lookupInfo.crcHashUpperBitsMode;
            break;

        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E:
            /* Only SIP6 devices can be added to this FDB manager ! */
            BMP_SET_MAC(fdbManagerPtr->supportedSipArr, CPSS_GEN_SIP_6_E);
            fdbManagerPtr->cpssHashParams.fid16BitHashEn       = GT_TRUE;
            fdbManagerPtr->cpssHashParams.crcHashUpperBitsMode = (GT_U32)fdbManagerPtr->lookupInfo.crcHashUpperBitsMode;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(fdbManagerPtr->entryAttrInfo.shadowType);
    }

    if(fdbManagerPtr->cpssHashParams.size < CPSS_DXCH_BRG_FDB_TBL_SIZE_64K_E)
    {
        /* !!! not supporting BC2 (and SIP4) !!! */
        fdbManagerPtr->cpssHashParams.useZeroInitValueForCrcHash = GT_FALSE;
    }
    else
    {
        /* !!! not supporting BC2 !!! */
        fdbManagerPtr->cpssHashParams.useZeroInitValueForCrcHash = GT_FALSE;
    }

    /* do single time calculation */
    fdbManagerPtr->maxPopulationPerBank =
        fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes/
        fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes;

    /* Calculate and update the threshold parameters to FDB DB */
    switch(fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes)
    {
        /* For AC5 - number of banks are 4(1 hash) */
        case 1:
        case 4:
            fdbManagerPtr->thresholdB = ((fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes * 40)/100);
            fdbManagerPtr->thresholdC = ((fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes * 90)/100);
            fdbManagerPtr->numOfBanks = 4;
            break;
        case 8:
            fdbManagerPtr->thresholdB = ((fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes * 60)/100);
            fdbManagerPtr->thresholdC = ((fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes * 95)/100);
            fdbManagerPtr->numOfBanks = 8;
            break;
        case 16:
            fdbManagerPtr->thresholdB = ((fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes * 80)/100);
            fdbManagerPtr->thresholdC = ((fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes * 95)/100);
            fdbManagerPtr->numOfBanks = 16;
            break;
    }

    /* Init the threshold type to 'NOT_VALID' */
    fdbManagerPtr->currentThresholdType = PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_NOT_VALID_E;

    /* age-bin usage matrix allocation */
    fdbManagerPtr->agingBinInfo.totalAgeBinAllocated = (fdbManagerPtr->capacityInfo.maxTotalEntries + (fdbManagerPtr->capacityInfo.maxEntriesPerAgingScan - 1)) / fdbManagerPtr->capacityInfo.maxEntriesPerAgingScan;

    /* Level 1 usage allocation. Each member of array holds flag about 32 age bins  */
    numBytesToAlloc = sizeof(GT_U32) * (1 + (fdbManagerPtr->agingBinInfo.totalAgeBinAllocated >> 5));
    MEM_CALLOC_MAC(fdbManagerId, fdbManagerPtr->agingBinInfo.level1UsageBitmapPtr, numBytesToAlloc);

    /* age-bin list allocation */
    numBytesToAlloc = fdbManagerPtr->agingBinInfo.totalAgeBinAllocated * sizeof(PRV_CPSS_DXCH_BRG_FDB_MANAGER_AGING_BIN_STC);
    MEM_CALLOC_MAC(fdbManagerId, fdbManagerPtr->agingBinInfo.ageBinListPtr, numBytesToAlloc);

    switch(fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes)
    {
        case 1:
            /* In case of AC5, multiple hash not supported so no staging */
            fdbManagerPtr->totalStages = 0;
            break;
        case 4:
            fdbManagerPtr->totalStages = PRV_CPSS_DXCH_BRG_FDB_MANAGER_FOUR_BANKS_TOTAL_STAGES_CNS;
            break;
        case 8:
            fdbManagerPtr->totalStages = PRV_CPSS_DXCH_BRG_FDB_MANAGER_EIGHT_BANKS_TOTAL_STAGES_CNS;
            break;
        default:
            fdbManagerPtr->totalStages = PRV_CPSS_DXCH_BRG_FDB_MANAGER_SIXTEEN_BANKS_TOTAL_STAGES_CNS;
            break;
    }

    if(fdbManagerPtr->capacityInfo.enableLearningLimits == GT_TRUE)
    {
        /* dynamicUcMacLimitPtr allocation */
        numBytesToAlloc = sizeof(PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_INFO_STC);
        MEM_CALLOC_MAC(fdbManagerId, fdbManagerPtr->dynamicUcMacLimitPtr, numBytesToAlloc);

        if(fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E)
        {
            numGlobalEports = 0;
        }
        else /* sip6 */
        if(fdbManagerPtr->entryAttrInfo.globalEportInfo.enable ==
            CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E ||
            /* the sip6 converted the CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E to 'min,max' !!! */
            fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
        {
            if(fdbManagerPtr->entryAttrInfo.globalEportInfo.minValue <=
               fdbManagerPtr->entryAttrInfo.globalEportInfo.maxValue &&
               fdbManagerPtr->entryAttrInfo.globalEportInfo.minValue < PRV_CPSS_DXCH_FDB_MANAGER_DB_MAX_EPORT_CNS &&
               fdbManagerPtr->entryAttrInfo.globalEportInfo.maxValue < PRV_CPSS_DXCH_FDB_MANAGER_DB_MAX_EPORT_CNS)
            {
                numGlobalEports = fdbManagerPtr->entryAttrInfo.globalEportInfo.maxValue -
                                  fdbManagerPtr->entryAttrInfo.globalEportInfo.minValue;

                fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEport_base =
                    fdbManagerPtr->entryAttrInfo.globalEportInfo.minValue;
            }
            else
            {
                numGlobalEports = 0;
            }
        }
        else
        {
            /* no optimization on the size */
            numGlobalEports = PRV_CPSS_DXCH_FDB_MANAGER_DB_MAX_EPORT_CNS;
        }

        if(numGlobalEports)
        {
            numBytesToAlloc = numGlobalEports * sizeof(PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_STC);
            MEM_CALLOC_MAC(fdbManagerId, fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEportPtr, numBytesToAlloc);
        }
        fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEport_num = numGlobalEports;

        /* set limits from defaults */
        fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.limitThreshold =
            fdbManagerPtr->capacityInfo.maxDynamicUcMacGlobalLimit;

        for(ii = 0 ; ii < PRV_CPSS_DXCH_FDB_MANAGER_DB_MAX_FID_CNS ; ii++)
        {
            fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_fid[ii].limitThreshold =
                fdbManagerPtr->capacityInfo.maxDynamicUcMacFidLimit;
        }

        for(ii = 0 ; ii < PRV_CPSS_DXCH_FDB_MANAGER_DB_MAX_TRUNK_ID_CNS ; ii++)
        {
            fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_trunkId[ii].limitThreshold =
                fdbManagerPtr->capacityInfo.maxDynamicUcMacTrunkLimit;
        }

        for(ii = 0 ; ii < numGlobalEports ; ii++)
        {
            fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEportPtr[ii].limitThreshold =
                fdbManagerPtr->capacityInfo.maxDynamicUcMacGlobalEportLimit;
        }
    }


    return GT_OK;
}


/**
* @internal createNewManager function
* @endinternal
*
* @brief  The function validate application parameters and then create 'manager' :
*         'prvCpssDxChFdbManagerDbArr[fdbManagerId]' with info from the caller.
*         and save the info from the application
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerId          - the FDB Manager id to associate with the newly created manager.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] capacityPtr           - (pointer to) parameters of the HW structure and capacity of its managed PPs FDBs –
*                                    like number of indexes, the amount of hashes etc.
*                                    As FDB Manager is created and entries may be added before PP registered –
*                                    we must know in advance the relevant capacity –
*                                    in Falcon for example we must know the shared memory allocation mode.
* @param[in] entryAttrPtr          - (pointer to) parameters that affects how HW/SW entries are
*                                    structured and specific fields behavior – including:
*                                    MUX'd fields in MAC / IPMC entries format, SA / DA drop commands mode &
*                                    IP NH packet command.
* @param[in] learningPtr           - (pointer to) parameters that affects which sort of entries to learn etc.
*                                    (Learn MAC of Routed packets and MAC no-space updates).
* @param[in] lookupPtr             - (pointer to) parameters that affect lookup of entries: MAC VLAN Lookup mode
*                                    (none, single or double tags), IVL single-tag MSB mode.
*                                    And two new Falcon setting: IPv4 UC and IPv6 UC masks
*                                    (allow using FDB for prefixes lookup and not just exact match)
* @param[in] agingPtr              - (pointer to) parameters that affect refresh and aging
*                                      (Refresh Destination UC, Refresh Destination MC &Refresh IP UC)
* @param[out] managerAllocatedPtr  - (pointer to) indication that the manager was allocated.
*                                    so on error need to do 'clean-up'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - if the FDB Manager id already exists.
* @retval GT_OUT_OF_CPU_MEM        - on fail to do Cpu memory allocation.
*/
static GT_STATUS    createNewManager
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           *capacityPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             *lookupPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              *agingPtr,
    OUT GT_BOOL                                         *managerAllocatedPtr
)
{
    GT_STATUS rc ;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr;
    GT_U32    numBytesToAlloc;

    /********************************************/
    /* validity checks before the DB allocation */
    /********************************************/
    rc = validateNewManagerParams(fdbManagerId,capacityPtr,entryAttrPtr,learningPtr,lookupPtr,agingPtr,GT_FALSE/*FDB_MANAGER_DB_EXIST*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*************************/
    /* allocate new manager  */
    /*************************/
    numBytesToAlloc = sizeof(PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC);
    MEM_CALLOC_MAC(fdbManagerId,PRV_SHARED_FDB_MANAGER_DB_VAR_GET(prvCpssDxChFdbManagerDbArr)[fdbManagerId],numBytesToAlloc);

    /* indication that the manager was allocated */
    *managerAllocatedPtr = GT_TRUE;

    /* we have valid new manager pointer */
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /*****************/
    /* save the info */
    /*****************/
    fdbManagerPtr->capacityInfo  = *capacityPtr;
    fdbManagerPtr->entryAttrInfo = *entryAttrPtr;
    fdbManagerPtr->learningInfo  = *learningPtr;
    fdbManagerPtr->lookupInfo    = *lookupPtr;
    fdbManagerPtr->agingInfo     = *agingPtr;

    if (IS_FDB_MANAGER_FOR_SIP_4(fdbManagerPtr))
    {
        /* set first word to be Valid but Skipped entry */
        fdbManagerPtr->hwEntryForFlush[0] = 0x3;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerDbCreate function
* @endinternal
*
* @brief  The function creates the FDB Manager and its databases according to
*        input capacity structure. Internally all FDB global parameters / modes
*        initialized to their defaults (No HW access - just SW defaults that will
*        be later applied to registered devices).
*
*        NOTE: the function doing only DB operations (no HW operations)
*
*        NOTE: incase of 'error' : if the manager was allocated the caller must
*           call prvCpssDxChFdbManagerDbDelete(...) to 'undo' the partial create.
*
* @param[in] fdbManagerId          - the FDB Manager id to associate with the newly created manager.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] capacityPtr           - (pointer to) parameters of the HW structure and capacity of its managed PPs FDBs –
*                                    like number of indexes, the amount of hashes etc.
*                                    As FDB Manager is created and entries may be added before PP registered –
*                                    we must know in advance the relevant capacity –
*                                    in Falcon for example we must know the shared memory allocation mode.
* @param[in] entryAttrPtr          - (pointer to) parameters that affects how HW/SW entries are
*                                    structured and specific fields behavior – including:
*                                    MUX'd fields in MAC / IPMC entries format, SA / DA drop commands mode &
*                                    IP NH packet command.
* @param[in] learningPtr           - (pointer to) parameters that affects which sort of entries to learn etc.
*                                    (Learn MAC of Routed packets and MAC no-space updates).
* @param[in] lookupPtr             - (pointer to) parameters that affect lookup of entries: MAC VLAN Lookup mode
*                                    (none, single or double tags), IVL single-tag MSB mode.
*                                    And two new Falcon setting: IPv4 UC and IPv6 UC masks
*                                    (allow using FDB for prefixes lookup and not just exact match)
* @param[in] agingPtr              - (pointer to) parameters that affect refresh and aging
*                                      (Refresh Destination UC, Refresh Destination MC &Refresh IP UC)
* @param[out] managerAllocatedPtr  - (pointer to) indication that the manager was allocated.
*                                    so on error need to do 'clean-up'
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_CPU_MEM        - on fail to do Cpu memory allocation.
* @retval GT_ALREADY_EXIST         - if the FDB Manager id already exists.
*/
GT_STATUS prvCpssDxChFdbManagerDbCreate
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           *capacityPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             *lookupPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              *agingPtr,
    OUT GT_BOOL                                         *managerAllocatedPtr
)
{
    GT_STATUS   rc;

    /*
    *  NOTE: incase of 'error' : if the manager was allocated the caller must
    *     call prvCpssDxChFdbManagerDbDelete(...) to 'undo' the partial create.
    */
    *managerAllocatedPtr = GT_FALSE;

    /***********************/
    /* create new manager  */
    /***********************/
    rc = createNewManager(fdbManagerId,capacityPtr,entryAttrPtr,learningPtr,lookupPtr,agingPtr,managerAllocatedPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*************************/
    /* init the new manager  */
    /*************************/
    rc = initNewManager(fdbManagerId);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerDbDevList_free_mallocs function
* @endinternal
*
* @brief   free the allocations relevant to the 'dev list'.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] devListArr[]          - the array of device ids to free mallocs from the FDB Manager.
* @param[in] numOfDevs             - the number of device ids in the array.
*
* @retval GT_OK                    - on success
*
* @note
*   NONE
*
*/
static GT_STATUS prvCpssDxChFdbManagerDbDevList_free_mallocs(
    IN GT_U32 fdbManagerId,
    IN GT_U8  devListArr[], /*arrSizeVarName=numOfDevs*/
    IN GT_U32 numOfDevs
)
{
    GT_U32  ii;
    GT_U8   devNum;
    GT_HW_DEV_NUM   hwDevNum;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    for(ii = 0 ; ii < numOfDevs ; ii++)
    {
        devNum = devListArr[ii];
        /* we lock the CPSS DB ... to be able to access 'PRV_CPSS_PP_MAC(devNum)' */
        /* we must unlock this DB if exit on ERROR !!! */
        LOCK_DEV_NUM(devNum);
        /* use the hwDevNum to allocate the relevant */
        hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

        if(fdbManagerPtr->dynamicUcMacLimitPtr &&
           fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum])
        {
            MEM_FREE_MAC(fdbManagerId,fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum]->perEportPtr);
            MEM_FREE_MAC(fdbManagerId,fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum]);
        }

        UNLOCK_DEV_NUM(devNum);/* we are done with this device internal DB */
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerDbDelete function
* @endinternal
*
* @brief  The function de-allocates specific FDB Manager Instance.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerId            - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
*/
GT_STATUS prvCpssDxChFdbManagerDbDelete
(
    IN GT_U32 fdbManagerId
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;
    GT_U32  wordId,ii;
    GT_U32  numOfAllocatedPointers;
    GT_U8   devNum;

    FDB_MANAGER_ID_CHECK(fdbManagerId)
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /* remove the devices from the 'global bmp' */
    for(wordId = 0; wordId < NUM_WORDS_DEVS_BMP_CNS; wordId++)
    {
        for(ii = 0 ; ii < 32 ; ii++)
        {
            if(fdbManagerPtr->devsBmpArr[wordId] & (1<<ii))
            {
                devNum = (GT_U8)(32*wordId + ii);
                /* delete the 'per device' DB , that allocated by
                   prvCpssDxChFdbManagerDbDevListAdd(...) */
                (void)prvCpssDxChFdbManagerDbDevList_free_mallocs(fdbManagerId,&devNum,1);
            }
        }

        PRV_SHARED_FDB_MANAGER_DB_VAR_GET(prvCpssDxChFdbManagerDbGlobalDevsBmpArr)[wordId] &= ~fdbManagerPtr->devsBmpArr[wordId];
    }

    /* deallocate all 'malloc' that where done for this manager */
    MEM_FREE_MAC(fdbManagerId,fdbManagerPtr->agingBinInfo.ageBinListPtr);
    MEM_FREE_MAC(fdbManagerId,fdbManagerPtr->agingBinInfo.level1UsageBitmapPtr);
    MEM_FREE_MAC(fdbManagerId,fdbManagerPtr->indexArr);
    MEM_FREE_MAC(fdbManagerId,fdbManagerPtr->entryPoolPtr);

    if(fdbManagerPtr->dynamicUcMacLimitPtr)
    {
        MEM_FREE_MAC(fdbManagerId,fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEportPtr);
        MEM_FREE_MAC(fdbManagerId,fdbManagerPtr->dynamicUcMacLimitPtr);
    }

    /* get the number of pending allocated pointers just before we free the manager itself */
    /* because it is decremented for every 'free' that we do */
    numOfAllocatedPointers = fdbManagerPtr->numOfAllocatedPointers;
    MEM_FREE_MAC(fdbManagerId,fdbManagerPtr);

    /* detach the manager-ID that was just free by upper line */
    PRV_SHARED_FDB_MANAGER_DB_VAR_SET(prvCpssDxChFdbManagerDbArr[fdbManagerId], NULL);

    if(numOfAllocatedPointers != 1)
    {
        /* not all the pointers released as should have been !!! */
        /* we removed the manager successfully ... but we have memory leakage !*/
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "(manager[%d] successfully removed , but we have memory leakage from [%d] pointers (forgot to free)",
            fdbManagerId,
            (numOfAllocatedPointers-1));
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerAllDbDelete function
* @endinternal
*
* @brief  The function de-allocates All FDB Manager Instance.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
*/
GT_STATUS prvCpssDxChFdbManagerAllDbDelete()
{
    GT_U32              fdbManagerId;
    GT_STATUS           rc;

    for(fdbManagerId = 0; fdbManagerId < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; fdbManagerId++)
    {
        rc = prvCpssDxChFdbManagerDbDelete(fdbManagerId);
        if((rc != GT_OK) && (rc != GT_NOT_INITIALIZED))
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerDbFdbEntryIteratorGetNext function
* @endinternal
*
* @brief   This function get first/next valid (pointer to) entry from the manager.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] isFirst               - indication to get First (GT_TRUE)  or the next (GT_FALSE)
* @param[in] devNumPtr             - (pointer to) the '(pointer to)current entry' for which we ask for 'next'
*                                     relevant only when isFirst == GT_FALSE (ask for next)
* @param[out] devNumPtr            - (pointer to) the '(pointer to)first/next entry'
*
*
* @retval GT_OK - there is 'first/next valid entry' from the manager
* @retval GT_NO_MORE - there are no more entries
*                      if isFirst == GT_TRUE --> no entries at all
*                      else                  --> no more entries after 'current' (IN *dbEntryPtrPtr)
*         GT_BAD_STATE - there is error in DB management.
*
*/
GT_STATUS prvCpssDxChFdbManagerDbFdbEntryIteratorGetNext
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC       *fdbManagerPtr,
    IN  GT_BOOL                                             isFirst,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC*    *dbEntryPtrPtr
)
{
    GT_U32  dbIndex;/* index in fdbManagerPtr->entryPoolPtr[] */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC *currDbPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC *nextDbPtr;

    if(GT_TRUE == isFirst)
    {
        if(!fdbManagerPtr->headOfUsedList.isValid_entryPointer)
        {
            return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
        }

        /* we have valid 'head' .. so return it */
        dbIndex        = fdbManagerPtr->headOfUsedList.entryPointer;
    }
    else
    {
        currDbPtr = *dbEntryPtrPtr;  /* current entry */

        /* check for next pointer */
        if(!currDbPtr->isValid_nextEntryPointer)
        {
            return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
        }

        /*dbIndex        = currDbPtr->nextEntryPointer;*/
        GET_nextEntryPointer_MAC(currDbPtr,dbIndex);
    }

    nextDbPtr      = &fdbManagerPtr->entryPoolPtr[dbIndex];
    if(nextDbPtr->isUsedEntry == 0)/* sanity check */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "manager error 'isUsedEntry' expected to be valid on the entry , dbIndex[%d]",
            dbIndex);
    }

    *dbEntryPtrPtr = nextDbPtr;
    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerDbIsAnyDeviceRegistred function
* @endinternal
*
* @brief   This function check if is there are registered device.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
*
* @retval 0 - no device registered.
* @retval 1 - at least one device registered.
*/
GT_U32 prvCpssDxChFdbManagerDbIsAnyDeviceRegistred
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC     *fdbManagerPtr
)
{
    return fdbManagerPtr->numOfDevices ? 1 : 0;
}

/**
* @internal prvCpssDxChFdbManagerDbDeviceIteratorGetNext function
* @endinternal
*
* @brief   This function get first/next registered device.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] isFirst               - indication to get First (GT_TRUE)  or the next (GT_FALSE)
* @param[in] devNumPtr             - (pointer to) the 'current device' for which we ask for 'next'
*                                     relevant only when isFirst == GT_FALSE (ask for next)
* @param[out] devNumPtr            - (pointer to) the 'first/next device'
*
* @retval GT_OK - there is 'first/next device'
* @retval GT_NO_MORE - there are no more devices
*                if isFirst == GT_TRUE --> no devices at all
*                else                  --> no more devices after 'current' (IN *devNumPtr)
*/
GT_STATUS prvCpssDxChFdbManagerDbDeviceIteratorGetNext
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC     *fdbManagerPtr,
    IN  GT_BOOL                                 isFirst,
    INOUT GT_U8                                 *devNumPtr
)
{
    GT_U32  wordId,bitId;

    if(isFirst == GT_TRUE)
    {
        wordId = 0;
        bitId  = 0;
    }
    else  /* the '+1' is to start from 'next' device */
    {
        wordId = ((*devNumPtr) + 1) >> 5;   /* /32 */
        bitId  = ((*devNumPtr) + 1) & 0x1f; /* %32 */
    }

    for(/* continue wordId */; wordId < NUM_WORDS_DEVS_BMP_CNS; wordId++)
    {
        if(fdbManagerPtr->devsBmpArr[wordId] == 0)
        {
            /* allow to jump over a lot of non existing devices */
            /* jump to next word (next 32 devices)              */
            bitId = 0;
            continue;
        }

        for(/* continue bitId */; bitId < 32; bitId++)
        {
            if(fdbManagerPtr->devsBmpArr[wordId] & (1 << bitId))
            {
                *devNumPtr = (GT_U8)((wordId << 5) + bitId);
                return GT_OK;
            }
        }

        bitId = 0;
    }

    return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
}

/**
* @internal prvCpssDxChBrgFdbManagerBanksStateGet function
* @endinternal
*
* @brief   The function checks state of calculated banks according to entry format.
*           
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB manager 
* @param[in] bankId                 - bank ID to get info from calc 
* @param[in] bankStep               - bank step to get info from calc
* @param[inout] calcInfoPtr         - (pointer to) the calculated hash and lookup results 
* @param[inout] bankStatePtr        - (pointer to) banks state 
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - wrong parameter
*/
static GT_STATUS  prvCpssDxChBrgFdbManagerBanksStateGet
(
    IN    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN    GT_U32                                            bankId,
    IN    GT_U32                                            bankStep,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr,
    OUT   PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_STATE_ENT   *banksStatePtr
)
{
    GT_STATUS                               rc;
    PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_STATE_ENT    banksState = PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_NOT_MATCHED_E;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr = NULL;  
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryExtPtr = NULL;
    
    /* Get data base entry - First bank */
    rc = prvCpssDxChBrgFdbManagerDbEntryGet(fdbManagerPtr, bankId, calcInfoPtr, &dbEntryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* IPv6 entry */
    if(bankStep > 1)
    {
        /* Get data base entry - Second bank */
        rc = prvCpssDxChBrgFdbManagerDbEntryGet(fdbManagerPtr, bankId+1, calcInfoPtr, &dbEntryExtPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(calcInfoPtr->usedBanksArr[bankId])
        {
            if(calcInfoPtr->usedBanksArr[bankId + 1])
            {
                /* Two banks occupied */
                if(dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E && 
                   dbEntryExtPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E)
                {
                    banksState = PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_IPV6_TO_IPV6_E; /* IPv6->IPv6 - Lookup/Move Bank[0,1] */
                }
            }
            else
            {
                /* First bank occupied */
                if(dbEntryPtr->hwFdbEntryType != PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E && 
                   dbEntryPtr->hwFdbEntryType != PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E)
                {
                    banksState = PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_IPV6_TO_BANK_0_NON_IPV6_E; /* IPv6->Non-IPv6 - Move Bank[0] */
                }
            }
        }
        else
        {
            if(calcInfoPtr->usedBanksArr[bankId + 1])
            {
                /* Second bank occupied */
                if(dbEntryExtPtr->hwFdbEntryType != PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E && 
                   dbEntryExtPtr->hwFdbEntryType != PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E)
                {
                    banksState = PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_IPV6_TO_BANK_1_NON_IPV6_E; /* IPv6->Non-IPv6 - Move Bank[1] */
                }
            }
            else
            {
                banksState = PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_IPV6_TWO_BANKS_FREE_E; /* IPv6->2*Empty - Free found */
            }
        }
    }
    else
    {
        if(calcInfoPtr->usedBanksArr[bankId])
        {
            if(dbEntryPtr->hwFdbEntryType != PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E && 
               dbEntryPtr->hwFdbEntryType != PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E)
            {
                banksState = PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_NON_IPV6_TO_NON_IPV6_E; /* Non-IPv6->Non-IPv6 - Lookup/Move Bank[0] */
            }
        }
        else
        {
            banksState = PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_NON_IPV6_BANK_FREE_E; /* Non-IPv6->Empty - Free found */
        }
    }

    *banksStatePtr                  = banksState;
    calcInfoPtr->dbEntryPtr         = dbEntryPtr;
    calcInfoPtr->dbEntryExtPtr      = dbEntryExtPtr;
    calcInfoPtr->lastBankIdChecked  = bankId;
    calcInfoPtr->selectedBankId     = bankId;

    return GT_OK;
}

/**
* @internal prvCpssDxChBrgFdbManagerBankCalcInfoGet function
* @endinternal
*
* @brief   This function gets DB calc info per bank in hash calculated in prvCpssDxChFdbManagerDbCalcHashAndLookupResult.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB manager
* @param[in] bankId                 - bank ID to get calc info 
* @param[in] bankStep               - bank step to get calc info 
* @param[in] entryPtr               - (pointer to) the current FDB entry need to be rehashed
* @param[in] addParamsPtr           - (pointer to) info given for 'entry add' that
*                                     may use specific bank from the hash.
*                                     NOTE: ignored if NULL
* @param[inout] calcInfoPtr         - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS  prvCpssDxChBrgFdbManagerBankCalcInfoGet
(
    IN    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN    GT_U32                                            bankId,
    IN    GT_U32                                            bankStep,
    IN    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   *addParamsPtr,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr
)
{
    GT_STATUS                   rc;
    GT_U32                      hwIndex;
    GT_BOOL                     isMatch;
    PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_STATE_ENT banksState;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INDEX_STC *dbIndexPtr;

    rc = prvCpssDxChBrgFdbManagerBanksStateGet(fdbManagerPtr, bankId, bankStep, calcInfoPtr, &banksState);
    if(rc != GT_OK)
    {
        return rc;
    }


    /* Initial bank step */
    calcInfoPtr->bankStepArr[bankId] = calcInfoPtr->bankStep;

    /* Check status of calculated banks */
    switch(banksState)
    {
        case PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_NON_IPV6_BANK_FREE_E:
            /* There is a free bank to add non-IPv6 entry */
            calcInfoPtr->calcEntryType          = PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_FREE_E;
            if(entryPtr == NULL)
            {
                calcInfoPtr->bankStepArr[bankId] = 1;
            }
            return GT_OK;

        case PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_IPV6_TWO_BANKS_FREE_E:
            /* There are free banks to add IPv6 entry */
            calcInfoPtr->calcEntryType          = PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_FREE_E;
            if(entryPtr == NULL)
            {
                calcInfoPtr->bankStepArr[bankId] = 2;
            }
            return GT_OK;

        case PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_NOT_MATCHED_E:
            /* Not free, so can't compare with my entry and can't use as free */
            if(entryPtr == NULL)
            {
                /* Skip current bank */
                calcInfoPtr->calcEntryType      = PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_NOT_FOUND_AND_NO_FREE_E;
            }

            return GT_OK;

        case PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_IPV6_TO_BANK_0_NON_IPV6_E:
            /* The entry in the bank is not ready to be compared */
            if(entryPtr == NULL)
            {
                /* The entry in the bank is ready to be moved  */
                calcInfoPtr->bankStepArr[bankId] = 1;
                calcInfoPtr->calcEntryType      = PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E;
            }
            else
            {
                calcInfoPtr->usedBanksArr[bankId] = GT_TRUE;
                calcInfoPtr->usedBanksArr[bankId+1] = GT_TRUE;
            }
            return GT_OK;
        case PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_IPV6_TO_BANK_1_NON_IPV6_E:
            /* The entry in the bank is not ready to be compared */
            if(entryPtr == NULL)
            {
                /* The entry in the next bank is non-IPv6 and ready to be moved */
                calcInfoPtr->bankStepArr[bankId] = 1;
                /* Skip current bank */
                calcInfoPtr->calcEntryType      = PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E;
            }
            else
            {
                calcInfoPtr->usedBanksArr[bankId] = GT_TRUE;
                calcInfoPtr->usedBanksArr[bankId+1] = GT_TRUE;
            }
            return GT_OK;
        case PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_NON_IPV6_TO_IPV6_E:
            /* The entry in the bank is not ready to be compared */
            if(entryPtr == NULL)
            {
                /* The entry in the bank is IPv6 and ready to be moved  */
                calcInfoPtr->bankStepArr[bankId] = 2;
                calcInfoPtr->calcEntryType     = PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E;
            }
            else
            {
                /* Not free, so can't compare with my entry and can't use as free */
                calcInfoPtr->dbEntryPtr         = 0;
                calcInfoPtr->dbEntryExtPtr      = 0;
                calcInfoPtr->usedBanksArr[bankId] = GT_TRUE;
            }
            return GT_OK;
        case PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_NON_IPV6_TO_NON_IPV6_E:
            /* The entry in the bank is not ready to be compared */
            if(entryPtr == NULL)
            {
                calcInfoPtr->bankStepArr[bankId] = 1;
            }
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_IPV6_TO_IPV6_E:
            /* The entry in the bank is not ready to be compared */
            if(entryPtr == NULL)
            {
                calcInfoPtr->bankStepArr[bankId] = 2;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if (entryPtr)
    {
        /* check if the key of the entry from application (entryPtr) match
                    the key of the entry from the manager DB (dbEntryPtr) */
        if(calcInfoPtr->bankStep > 1)
        {
            rc = prvCpssDxChFdbManagerDbIsIpv6UcEntryKeyMatchDbEntryKey(
                entryPtr,
                calcInfoPtr->dbEntryPtr, calcInfoPtr->dbEntryExtPtr,
                &isMatch);
        }
        else
        {
            rc = prvCpssDxChFdbManagerDbIsEntryKeyMatchDbEntryKey(fdbManagerPtr,
                entryPtr,
                calcInfoPtr->dbEntryPtr,
                &isMatch);
        }
        if(rc != GT_OK)
        {
            return rc;
        }

        if(isMatch == GT_FALSE)
        {
            /* the entry in the DB not match the entry that application trying to add */
            return GT_OK;
        }
    }

    calcInfoPtr->calcEntryType      = PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E;

    if(addParamsPtr &&
            (addParamsPtr->tempEntryExist == GT_TRUE) &&
            (addParamsPtr->tempEntryOffset > bankId))
    {
        /* the caller will check the 'usedBanksArr[]' at index addParamsPtr->tempEntryOffset */
        /* so fill the information for it ... is it used or not */
        bankId = addParamsPtr->tempEntryOffset;
        hwIndex = calcInfoPtr->crcMultiHashArr[bankId];

        /* protect access to array out of range */
        CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

        dbIndexPtr = &fdbManagerPtr->indexArr[hwIndex];
        calcInfoPtr->usedBanksArr[bankId] = dbIndexPtr->isValid_entryPointer;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChBrgFdbManagerDbEntryGet function
* @endinternal
*
* @brief   Returns DB entry per bank ID
*           
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB manager 
* @param[in] bankId                 - bank ID to get info from calc
* @param[inout] calcInfoPtr         - (pointer to) the calculated hash and lookup results
* @param[out] dbEntryPtrPtr         - (pointer to)(pointer to) the entry in memory pool 
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - wrong parameter
*/
GT_STATUS prvCpssDxChBrgFdbManagerDbEntryGet
(
    IN    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN    GT_U32                                            bankId,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr,
    OUT   PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    **dbEntryPtrPtr
)
{
    GT_U32                                  hwIndex;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INDEX_STC  *dbIndexPtr = NULL;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC  *dbEntryPtr = NULL;  

    hwIndex = calcInfoPtr->crcMultiHashArr[bankId];
    dbIndexPtr = &fdbManagerPtr->indexArr[hwIndex];

    if(dbIndexPtr->isValid_entryPointer)
    {
        dbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbIndexPtr->entryPointer];
    }
    *dbEntryPtrPtr = dbEntryPtr;

    calcInfoPtr->usedBanksArr[bankId] = dbIndexPtr->isValid_entryPointer;

    return GT_OK;
}

/**
* @internal prvCpssDxChBrgFdbManagerRehashBankFdbEntryGet function
* @endinternal
*
* @brief   This function retrieve FDB entry from calcInfo per bank from HASH calculated in prvCpssDxChFdbManagerDbCalcHashAndLookupResult.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB manager
* @param[in] bankId                 - bank ID to get rehashing calc info 
* @param[in] bankStep               - rehashing entry bank step
* @param[in] calcInfoPtr            - (pointer to) the calculated hash and lookup results
* @param[out] entryPtr              - (pointer to) the current FDB entry need to be rehashed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS  prvCpssDxChBrgFdbManagerRehashBankFdbEntryGet
(
    IN      PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN      GT_U32                                            bankId,
    IN      GT_U32                                            bankStep,
    IN      PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr,
    OUT     CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr
)
{
    GT_STATUS                                       rc;

    rc = prvCpssDxChBrgFdbManagerBankCalcInfoGet(fdbManagerPtr, bankId, bankStep, NULL, NULL, calcInfoPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (calcInfoPtr->dbEntryPtr == NULL)
    {
        return GT_OK;
    }

    if (calcInfoPtr->calcEntryType == PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_NOT_FOUND_AND_NO_FREE_E)
    {
        return GT_OK;
    }
    

    if (calcInfoPtr->bankStepArr[bankId] > 1)
    {
        /* convert DB format to application format - IPv6 UC */
        rc = prvCpssDxChBrgFdbManagerRestoreEntryIpV6UcFormatParamsFromDb(fdbManagerPtr, calcInfoPtr->dbEntryPtr, calcInfoPtr->dbEntryExtPtr, entryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* convert DB format to application format */
        rc = prvCpssDxChBrgFdbManagerRestoreEntryFormatParamsFromDb(fdbManagerPtr, calcInfoPtr->dbEntryPtr, entryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerDbCalcFreeEntryFind function
* @endinternal
*
* @brief   This function lookup free place in the HASH info filled into calcInfoPtr.
*
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[inout] calcInfoPtr        - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
static GT_STATUS   prvCpssDxChFdbManagerDbCalcFreeEntryFind
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr
)
{
    GT_STATUS                                       rc;
    GT_U32                                          bankId;

    for(bankId = 0 ; bankId < fdbManagerPtr->numOfBanks; bankId += calcInfoPtr->bankStep)
    {
        if (bankId == calcInfoPtr->rehashStageBank)
        {
            /* skip current rehashing bank */
            continue;
        }

        rc = prvCpssDxChBrgFdbManagerBankCalcInfoGet(fdbManagerPtr, bankId, calcInfoPtr->bankStep, NULL, NULL, calcInfoPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        if (calcInfoPtr->calcEntryType == PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_FREE_E)
        {
            break;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerDbHashCalculate function
* @endinternal
*
* @brief   This function calculates hash for the new/existing entry.
*
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*                                     the function actually uses only the 'key' of the entry.
* @param[out] calcInfoPtr          - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS   prvCpssDxChFdbManagerDbHashCalculate
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr
)
{
    GT_STATUS                   rc;
    CPSS_MAC_ENTRY_EXT_KEY_STC  entryKey;
    GT_U32                      bankId;

    /* build the search key */
    switch(entryPtr->fdbEntryType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E :
            if(fdbManagerPtr->learningInfo.macVlanLookupMode !=
                   CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E)
            {
                entryKey.entryType  = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
            }
            else
            {
                entryKey.entryType  = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E;
            }

            entryKey.key.macVlan.macAddr = entryPtr->format.fdbEntryMacAddrFormat.macAddr;
            entryKey.key.macVlan.vlanId  = (GT_U16)entryPtr->format.fdbEntryMacAddrFormat.fid;
            entryKey.vid1       = entryPtr->format.fdbEntryMacAddrFormat.vid1;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E  :
            entryKey.entryType      = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;
            cpssOsMemCpy(entryKey.key.ipMcast.sip, entryPtr->format.fdbEntryIpv4McFormat.sipAddr, 4);
            cpssOsMemCpy(entryKey.key.ipMcast.dip, entryPtr->format.fdbEntryIpv4McFormat.dipAddr, 4);
            entryKey.key.ipMcast.vlanId = (GT_U16)entryPtr->format.fdbEntryIpv4McFormat.fid;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E  :
            entryKey.entryType      = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E;
            cpssOsMemCpy(entryKey.key.ipMcast.sip, entryPtr->format.fdbEntryIpv6McFormat.sipAddr, 4);
            cpssOsMemCpy(entryKey.key.ipMcast.dip, entryPtr->format.fdbEntryIpv6McFormat.dipAddr, 4);
            entryKey.key.ipMcast.vlanId = (GT_U16)entryPtr->format.fdbEntryIpv6McFormat.fid;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E  :
            entryKey.entryType      = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
            cpssOsMemCpy(entryKey.key.ipv4Unicast.dip, entryPtr->format.fdbEntryIpv4UcFormat.ipv4Addr.arIP, 4);
            entryKey.key.ipv4Unicast.vrfId = entryPtr->format.fdbEntryIpv4UcFormat.vrfId;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E  :
            entryKey.entryType      = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E;
            cpssOsMemCpy(entryKey.key.ipv6Unicast.dip, entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP, 16);
            entryKey.key.ipv6Unicast.vrfId = entryPtr->format.fdbEntryIpv6UcFormat.vrfId;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->fdbEntryType);
    }


    if(IS_FDB_MANAGER_FOR_SIP_4(fdbManagerPtr))
    {
        rc = cpssDxChBrgFdbHashByParamsCalc(fdbManagerPtr->capacityInfo.hwCapacity.mode,
                &fdbManagerPtr->cpssHashParams,
                &entryKey,
                &calcInfoPtr->crcMultiHashArr[0]);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* SIP4 calculated 1 hash,
         * fills 4 incremental banks index */
        for(bankId = 1; bankId < fdbManagerPtr->numOfBanks; bankId += 1)
        {
            calcInfoPtr->crcMultiHashArr[bankId] = calcInfoPtr->crcMultiHashArr[0] + bankId;
        }
    }
    else
    {
        /*******************************/
        /* calc the 4/8/16 hash values */
        /*******************************/
        rc = cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc(
                &fdbManagerPtr->cpssHashParams,
                &entryKey,
                0,/*multiHashStartBankIndex*/
                fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes,
                &calcInfoPtr->crcMultiHashArr[0]);

        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(entryKey.entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E)
    {
        calcInfoPtr->bankStep = 2;
        /* for IPv6 odd bank hash eqauls to even bank */
        for(bankId = 0; bankId < fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes; bankId += 2)
        {
            /** ipv6 are consecutive entries in the HW - sip_5_20 and above */
            calcInfoPtr->crcMultiHashArr[bankId + 1] = calcInfoPtr->crcMultiHashArr[bankId] + 1;
        }
    }
    else
    {
        calcInfoPtr->bankStep = 1;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChBrgFdbManagerRehashBankFdbEntryHwIndexCheck function
* @endinternal
*
* @brief   This function validates HW index usage for rehashed FDB entry from calculated HASH.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB manager
* @param[in] rehashStageId          - current rehashing stage
* @param[in] calcInfoPtr            - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_VALUE             - HW index already in use 
*/
static GT_STATUS  prvCpssDxChBrgFdbManagerRehashBankFdbEntryHwIndexCheck
(
    IN      PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN      GT_U32                                           rehashStageId,
    IN      PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr
)
{
    GT_U32      hwIndex;
    GT_U32      ii;

    hwIndex = calcInfoPtr->crcMultiHashArr[calcInfoPtr->rehashStageBank];

    /* Iterate rehashing DB entries */
    for (ii = 0; ii < rehashStageId; ii++)
    {
        /* Compare current index with index from DB */
        if (hwIndex == fdbManagerPtr->rehashingTraceArray[ii].entryHwIndex)
        {
            /* The HW index was used during rehashing */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
        }
    }

    /* HW index was not use durng rehashing */
    fdbManagerPtr->rehashingTraceArray[rehashStageId - 1].entryHwIndex = hwIndex;

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerDbRehashEntry function
* @endinternal
*
* @brief   This function re-hash index for the new entry that can't be added to HW
*          when all banks for the current hash index are occupied.
*
*          NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB manager.
* @param[in] rehashStageId          - rehashing stage level 
* @param[out] calcInfoPtr           - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                   - on operation fail
*
*/
GT_STATUS   prvCpssDxChFdbManagerDbRehashEntry
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN GT_U32                                           rehashStageId,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr
)
{
    GT_STATUS                                           rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC    calcInfoCurrent;
    GT_U32                                              bankId;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                 currentEntry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                *entryPtr = &currentEntry;

    /* Copy current calc info for the next recursion step */
    calcInfoCurrent = *calcInfoPtr;

    /* Traverse all hash results for current FDB entry */
    for (bankId = 0; bankId < fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes; bankId += calcInfoPtr->bankStep)
    {
        /* Calculate the current bank that need to be rehashed */
        calcInfoCurrent.rehashStageBank = (bankId + (rehashStageId - 1) * calcInfoPtr->bankStep) % fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes;

        if(rehashStageId > 1 
           && calcInfoPtr->rehashStageBank == calcInfoCurrent.rehashStageBank)
        {
            /* Same bank in the next stage  */
            continue;
        }

        /* Get FDB entry per rehash stage bank */
        rc = prvCpssDxChBrgFdbManagerRehashBankFdbEntryGet(fdbManagerPtr, calcInfoCurrent.rehashStageBank, calcInfoPtr->bankStep, calcInfoPtr, entryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(calcInfoPtr->dbEntryPtr == NULL)
        {
            if (calcInfoPtr->calcEntryType == PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E)
            {
                /* Get FDB entry from the next bank */
                calcInfoCurrent.rehashStageBank += 1;
                rc = prvCpssDxChBrgFdbManagerRehashBankFdbEntryGet(fdbManagerPtr, calcInfoCurrent.rehashStageBank, 1, calcInfoPtr, entryPtr);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            else
            {
                /* Entry not suitable for moving - skip to the next bank */
                continue;
            }
        }
        else
        {
            if (calcInfoPtr->calcEntryType == PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_NOT_FOUND_AND_NO_FREE_E)
            {
                /* Entry not suitable for moving - skip to the next bank */
                continue;
            }
        }

        /* Recalculate new hash for the current FDB entry on new stage */
        rc = prvCpssDxChFdbManagerDbHashCalculate(fdbManagerPtr, entryPtr, &calcInfoCurrent);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChBrgFdbManagerRehashBankFdbEntryHwIndexCheck(fdbManagerPtr, rehashStageId, &calcInfoCurrent);
        if(rc != GT_OK)
        {
            /* HW index was used in previous rehashing stages */
            continue;
        }

#ifdef FDB_MANAGER_REHASHING_DEBUG
        if (PRV_SHARED_FDB_MANAGER_DB_VAR_GET(reHashDebugPrint)) 
        {
            rc = prvCpssDxChBrgFdbManagerRehashInfoPrint(fdbManagerPtr, entryPtr, &calcInfoCurrent, rehashStageId, calcInfoCurrent.rehashStageBank);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
#endif

        /* Find free banks in the new HASH for the current FDB entry */
        rc = prvCpssDxChFdbManagerDbCalcFreeEntryFind(fdbManagerPtr, &calcInfoCurrent);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* No free place in DB */
        if (calcInfoCurrent.calcEntryType != PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_FREE_E)
        {
            /* Next stage exceeds number of total stages */
            if (rehashStageId + 1 > fdbManagerPtr->totalStages)
            {
                return GT_OK;
            }

            /* Dive into the next stage */
            rc = prvCpssDxChFdbManagerDbRehashEntry(fdbManagerPtr, rehashStageId + 1, &calcInfoCurrent);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* Free DB entry was found */
            if (rehashStageId < calcInfoCurrent.rehashStageId)
            {
                calcInfoPtr->rehashStageId = calcInfoCurrent.rehashStageId;
                /* Save current entry and it's calcInfo in data base */
                calcInfoCurrent.lastBankIdChecked = fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes - 1;
                /* Add current entry and it's hash info to the data base per stage */
                fdbManagerPtr->rehashingTraceArray[rehashStageId - 1].entry    = currentEntry;
                fdbManagerPtr->rehashingTraceArray[rehashStageId - 1].calcInfo = calcInfoCurrent;
                return GT_OK;
            }
            else
            {
                /* Continue look up with next FDB entry from hash */
                continue;
            }
        }
        else
        {
            calcInfoPtr->rehashStageId = calcInfoCurrent.rehashStageId = rehashStageId;
            /* Free DB entry was found - save entry and it's calcInfo in data base */
            calcInfoCurrent.lastBankIdChecked = fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes - 1;
            /* Add current entry and it's hash info to the data base per stage */
            fdbManagerPtr->rehashingTraceArray[rehashStageId - 1].entry    = currentEntry;
            fdbManagerPtr->rehashingTraceArray[rehashStageId - 1].calcInfo = calcInfoCurrent;
            return GT_OK;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerDbCalcHashAndLookupResult function
* @endinternal
*
* @brief   This function hash calculate place for the new/existing entry.
*          the HASH info filled into calcInfoPtr , with indication if entry found ,
*          or not found and no place for it ,
*          or not found but have place for it.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*                                     the function actually uses only the 'key' of the entry.
* @param[in] addParamsPtr          - (pointer to) info given for 'entry add' that
*                                     may use specific bank from the hash.
*                                     NOTE: ignored if NULL
* @param[out] calcInfoPtr          - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS   prvCpssDxChFdbManagerDbCalcHashAndLookupResult
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   *addParamsPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr
)
{
    GT_STATUS                   rc;
    GT_U32                      bankId;

    /* calculate hash for FDB entry */
    rc = prvCpssDxChFdbManagerDbHashCalculate(fdbManagerPtr, entryPtr, calcInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    /*  set defaults */
    calcInfoPtr->calcEntryType     = PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_NOT_FOUND_AND_NO_FREE_E;
    calcInfoPtr->dbEntryPtr        = NULL;
    calcInfoPtr->dbEntryExtPtr     = NULL;

    for(bankId = 0; bankId < fdbManagerPtr->numOfBanks; bankId += calcInfoPtr->bankStep)
    {
        rc = prvCpssDxChBrgFdbManagerBankCalcInfoGet(fdbManagerPtr, bankId, calcInfoPtr->bankStep, entryPtr, addParamsPtr, calcInfoPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        if (calcInfoPtr->calcEntryType == PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E)
        {
            return GT_OK;
        }
    }

    calcInfoPtr->selectedBankId = GT_NA;
    calcInfoPtr->lastBankIdChecked = bankId - 1;

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerDbGetMostPopulatedBankId function
* @endinternal
*
* @brief   This function choose the best bank to use for the entry.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] calcInfoPtr           - using calcInfoPtr->usedBanksArr[] ,
*                                    using calcInfoPtr->lastBankIdChecked
* @param[out] calcInfoPtr          - updating calcInfoPtr->selectedBankId
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - not found bank to use
*
*/
GT_STATUS   prvCpssDxChFdbManagerDbGetMostPopulatedBankId
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC       *fdbManagerPtr,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC  *calcInfoPtr
)
{
    GT_U32  bankId;
    GT_U32  numUsed    = GT_NA;
    GT_U32  bestBankId = GT_NA;

    /* In case of IPv6 entry check the next bank index also free */
    if(calcInfoPtr->bankStep > 1)
    {
        for(bankId = 0 ; bankId <= calcInfoPtr->lastBankIdChecked ; bankId+=calcInfoPtr->bankStep)
        {
            /* Check if the bank index already in use
             * Or the bank is already FULL */
            if((calcInfoPtr->usedBanksArr[bankId] || calcInfoPtr->usedBanksArr[bankId+1]) ||
                    (fdbManagerPtr->bankInfoArr[bankId].bankPopulation == fdbManagerPtr->maxPopulationPerBank) ||
                    (fdbManagerPtr->bankInfoArr[bankId+1].bankPopulation == fdbManagerPtr->maxPopulationPerBank))
            {
                continue;
            }

            /* IPv6 first banks population is considered */
            if((numUsed == GT_NA) ||
                    (numUsed < fdbManagerPtr->bankInfoArr[bankId].bankPopulation))
            {
                numUsed     = fdbManagerPtr->bankInfoArr[bankId].bankPopulation;
                bestBankId  = bankId;
            }
        }
    }
    else
    {
        for(bankId = 0 ; bankId <= calcInfoPtr->lastBankIdChecked ; bankId++)
        {
            /* Check if the bank index already in use
             * Or the bank is already FULL */
            if((calcInfoPtr->usedBanksArr[bankId]) ||
                    (fdbManagerPtr->bankInfoArr[bankId].bankPopulation == fdbManagerPtr->maxPopulationPerBank))
            {
                continue;
            }

            if((numUsed == GT_NA) ||
                    (numUsed < fdbManagerPtr->bankInfoArr[bankId].bankPopulation))
            {
                numUsed     = fdbManagerPtr->bankInfoArr[bankId].bankPopulation;
                bestBankId  = bankId;
            }
        }
    }

    if(bestBankId == GT_NA)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "not found best bankId");
    }

    calcInfoPtr->selectedBankId = bestBankId;
    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerDbAddNewEntry function
* @endinternal
*
* @brief   This function get free entry to be used for new added entry.
*           adding new entry :
*           update metadata DB :indexArr[] , bankInfoArr[] , typeCountersArr[]
*           update headOfFreeList , headOfUsedList , tailOfUsedList
*           update calcInfoPtr->dbEntryPtr as valid pointer
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] primaryBank           - selected bank to adding entry:
*                                     GT_TRUE  - primary bank ID
*                                     GT_FALSE - secondary bank ID - for IPv6 FDB entry type only
* @param[in] calcInfoPtr           - using calcInfoPtr->selectedBankId ,
*                                          calcInfoPtr->counterType ,
*                                          calcInfoPtr->crcMultiHashArr[] ,
* @param[out] calcInfoPtr          - updating calcInfoPtr->dbEntryPtr
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE /GT_BAD_PARAM - non-consistency between different parts of
*                                    the manager.
*
*/
GT_STATUS   prvCpssDxChFdbManagerDbAddNewEntry
(
    IN    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN    GT_BOOL                                                 primaryBank,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC        *calcInfoPtr
)
{
    GT_U32  bankId;
    GT_U32  hwIndex;
    GT_U32  currFree_dbIndex;
    GT_U32  oldTail_dbIndex;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC *freeInPoolPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC *oldTailUsedInPoolPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC *dbEntryPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC *oldHeadOfAgeBinInPoolPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC     *headOfOldAgeBinListPtr;
    GT_U32                                         ageBinIndex;
    GT_STATUS                                      rc;

    /* calcInfoPtr->dbEntryPtr is NULL at this point */
    /* the entry was not found as 'existing' in the DB */

    if (primaryBank == GT_TRUE)
    {
        bankId  =  calcInfoPtr->selectedBankId;
    }
    else
    {
        bankId  =  calcInfoPtr->selectedBankId+1;
    }

    /* Get the Free age-bin Index */
    rc = prvCpssDxChFdbManagerDbFreeAgeBinIndexGet(fdbManagerPtr, &ageBinIndex);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (ageBinIndex >= fdbManagerPtr->agingBinInfo.totalAgeBinAllocated)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* protect access to array out of range */
    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(bankId,CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_BANKS_CNS);

    /*********************************************/
    /* update bankInfoArr[].bankPopulation */
    /*********************************************/
    /* update the number of entries in the bank */
    fdbManagerPtr->bankInfoArr[bankId].bankPopulation++;

    /* coherency check */
    CHECK_X_NOT_MORE_THAN_Y_MAC(
        fdbManagerPtr->bankInfoArr[bankId].bankPopulation,
        fdbManagerPtr->maxPopulationPerBank);

    fdbManagerPtr->totalPopulation++;
    /* coherency check */
    CHECK_X_NOT_MORE_THAN_Y_MAC(
        fdbManagerPtr->totalPopulation,
        fdbManagerPtr->capacityInfo.maxTotalEntries);

    hwIndex = calcInfoPtr->crcMultiHashArr[bankId];

    /*********************/
    /* update indexArr[] */
    /*********************/
    fdbManagerPtr->indexArr[hwIndex].isValid_entryPointer = 1;
    /* 'take' entry from the headOfFreeList */
    fdbManagerPtr->indexArr[hwIndex].entryPointer = fdbManagerPtr->headOfFreeList.entryPointer;

    /*****************************/
    /* update the headOfFreeList */
    /*****************************/
    if(!fdbManagerPtr->headOfFreeList.isValid_entryPointer)
    {
        /* NOTE: this is NOT 'GT_FULL' case , because prvCpssDxChFdbManagerDbCalcHashAndLookupResult(..) show that we have free space ! */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "the DB not found 'free' entry in fdbManagerPtr->headOfFreeList");
    }

    currFree_dbIndex = fdbManagerPtr->headOfFreeList.entryPointer;

    freeInPoolPtr = &fdbManagerPtr->entryPoolPtr[currFree_dbIndex];

    dbEntryPtr = freeInPoolPtr;

    /*****************************/
    /* update the headOfFreeList */
    /*****************************/
    if(freeInPoolPtr->isValid_nextEntryPointer)
    {
        /* use 'next' index in the pool as new 'headOfFreeList' indication */
        fdbManagerPtr->headOfFreeList.isValid_entryPointer = 1;
        /*fdbManagerPtr->headOfFreeList.entryPointer         = freeInPoolPtr->nextEntryPointer;*/
        GET_nextEntryPointer_MAC(freeInPoolPtr,fdbManagerPtr->headOfFreeList.entryPointer);
    }
    else
    {
        /* there are no more free elements ! */
        fdbManagerPtr->headOfFreeList.isValid_entryPointer = 0;
    }

    /*****************************/
    /* update the headOfUsedList */
    /*****************************/
    if(!fdbManagerPtr->headOfUsedList.isValid_entryPointer)
    {
        /* the tail was not yet valid ... now it can be valid ! */
        fdbManagerPtr->headOfUsedList.isValid_entryPointer = 1;
        fdbManagerPtr->headOfUsedList.entryPointer         = currFree_dbIndex;
    }

    /*****************************/
    /* update the tailOfUsedList */
    /*****************************/
    if(fdbManagerPtr->tailOfUsedList.isValid_entryPointer)
    {
        /* Add the new entry to the tail of the list, as the new last entry.
           Connect the previous last with the new entry and set the global last entry
           pointer to the new entry.
        */
        oldTail_dbIndex = fdbManagerPtr->tailOfUsedList.entryPointer;

        oldTailUsedInPoolPtr = &fdbManagerPtr->entryPoolPtr[oldTail_dbIndex];
        oldTailUsedInPoolPtr->isValid_nextEntryPointer = 1;
        /*oldTailUsedInPoolPtr->nextEntryPointer         = currFree_dbIndex;*/
        SET_nextEntryPointer_MAC(oldTailUsedInPoolPtr,currFree_dbIndex);

        /* point the 'new used' (new tail) to the previous tail */
        dbEntryPtr->isValid_prevEntryPointer = 1;
        dbEntryPtr->prevEntryPointer         = oldTail_dbIndex;
    }
    else
    {
        fdbManagerPtr->tailOfUsedList.isValid_entryPointer = 1;

        /* point the 'new used' to have no previous */
        dbEntryPtr->isValid_prevEntryPointer = 0;
        dbEntryPtr->prevEntryPointer         = 0;/* don't care */
    }

    fdbManagerPtr->tailOfUsedList.entryPointer         = currFree_dbIndex;

    /************************************/
    /* update the age bin related data */
    /************************************/
    /* Get the Head of existing age-bin list */
    headOfOldAgeBinListPtr  = &fdbManagerPtr->agingBinInfo.ageBinListPtr[ageBinIndex].headOfAgeBinEntryList;

    /* Add to top of the existing list */
    if(headOfOldAgeBinListPtr->isValid_entryPointer == 0)
    { /* First Node */
        dbEntryPtr->isValid_age_nextEntryPointer                = 0;
        dbEntryPtr->age_nextEntryPointer                        = 0; /* don't care */
    }
    else
    {
        oldHeadOfAgeBinInPoolPtr = &fdbManagerPtr->entryPoolPtr[headOfOldAgeBinListPtr->entryPointer];

        /* Old Head */
        oldHeadOfAgeBinInPoolPtr->isValid_age_prevEntryPointer  = 1;
        oldHeadOfAgeBinInPoolPtr->age_prevEntryPointer          = currFree_dbIndex;

        /* New Head */
        dbEntryPtr->age_nextEntryPointer                        = headOfOldAgeBinListPtr->entryPointer;
        dbEntryPtr->isValid_age_nextEntryPointer                = 1;
    }

    /* Update the Head of age-bin */
    headOfOldAgeBinListPtr->entryPointer                        = currFree_dbIndex;
    headOfOldAgeBinListPtr->isValid_entryPointer                = 1;
    dbEntryPtr->ageBinIndex                                     = ageBinIndex;

    /* New entry inserted at the top, so prev pointer always NOT VALID */
    dbEntryPtr->isValid_age_prevEntryPointer                    = 0;
    dbEntryPtr->age_prevEntryPointer                            = 0; /* don't care */

    INCREASE_AGE_BIN_COUNTER_MAC(fdbManagerPtr, ageBinIndex);

    /********************************/
    /* update the entry in the pool */
    /********************************/
    dbEntryPtr->isUsedEntry = 1;
    dbEntryPtr->hwIndex     = hwIndex;
    /* point the 'new used' to have no 'next' */
    dbEntryPtr->isValid_nextEntryPointer = 0;
    /*dbEntryPtr->nextEntryPointer         = 0;*//* don't care */
    SET_nextEntryPointer_MAC(dbEntryPtr,0);/* don't care */

    /********************************/
    /* update the typeCountersArr */
    /********************************/

    /* coherency check */
    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
        calcInfoPtr->counterType,
        PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE__LAST__E);

    fdbManagerPtr->typeCountersArr[calcInfoPtr->counterType].entryTypePopulation++;

    rc = prvCpssDxChFdbManagerDbLimitOper(fdbManagerPtr,LIMIT_OPER_ADD_NEW_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    /**************************************************/
    /* save the pointer for later use , by the caller */
    /**************************************************/
    if (primaryBank == GT_TRUE)
    {
        calcInfoPtr->dbEntryPtr = dbEntryPtr;
    }
    else
    {
        calcInfoPtr->dbEntryExtPtr = dbEntryPtr;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerDbTableFullDiagnostic function
* @endinternal
*
* @brief   This function do diagnostic to 'FDB full' state
*          the caller function will return this error to the application
*          NOTE: the function always return 'ERROR'. the 'GT_FULL' is expected ,
*                other values are not
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
*
* @retval GT_BAD_STATE             - non-consistency between different parts of
*                                    the manager.
* @retval GT_FULL                  - the table is really FULL (all banks are full).
*
*/
GT_STATUS   prvCpssDxChFdbManagerDbTableFullDiagnostic
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr
)
{
    GT_U32  bankId,totalPopulation;

    if(fdbManagerPtr->totalPopulation != fdbManagerPtr->capacityInfo.maxTotalEntries)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "the FDB is considered FULL , but coherency error detected maxTotalEntries[%d] != totalPopulation[%d]",
            fdbManagerPtr->capacityInfo.maxTotalEntries,
            fdbManagerPtr->totalPopulation);
    }

    /* do checking of 'summary' to see that really FULL and not management
       calculation error */
    totalPopulation = 0;
    for(bankId = 0 ; bankId < fdbManagerPtr->numOfBanks ; bankId++)
    {
        totalPopulation += fdbManagerPtr->bankInfoArr[bankId].bankPopulation;
    }

    if(totalPopulation != fdbManagerPtr->capacityInfo.maxTotalEntries)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "the FDB is considered FULL , but coherency error detected maxTotalEntries[%d] != totalPopulation[%d] (banks summary)",
            fdbManagerPtr->capacityInfo.maxTotalEntries,
            totalPopulation);
    }

    /* the table is FULL ? ... no need to to 'try' the entry can't be 'added'
        if already exists  ... we not update it anyway
        if not exists      ... we can't add it
    */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL,
        "the FDB is FULL with [%d] entries",
        fdbManagerPtr->capacityInfo.maxTotalEntries);
}

/**
* @internal deleteOldEntryFromDbMetadata_NodesCheck function
* @endinternal
*
* @brief   This function check impact on specific 'node' that may point to removed index.
*          update the 'node' if it point to the removed entry
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] removed_dbIndex       - the db index of the removed entry
* @param[in] isValid_prevEntryPointer - indication that the removed entry hold
*                                   valid previous entry in the 'used list'.
* @param[in] prevEntryPointer      -  the previous entry index in the 'used list'.
* @param[in] nodePtr               - (pointer to) the node that need to be checked
*                                   if the 'removed' index impact it.
* @param[out] nodePtr              - (pointer to) the node updated since pointed to removed index.
*
*/
static void   deleteOldEntryFromDbMetadata_specificNode
(
    IN GT_U32                               removed_dbIndex,
    IN GT_U32                               isValid_prevEntryPointer,
    IN GT_U32                               prevEntryPointer,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC     *nodePtr
)
{
    if(!nodePtr->isValid_entryPointer)
    {
        /* no impact on the 'non-valid' node */
        return;
    }

    if(nodePtr->entryPointer != removed_dbIndex)
    {
        /* no impact on this node */
        return;
    }

    /*******************/
    /* update the node */
    /*******************/
    /* we removed the entry from DB ... so need to move the node to point to
       previous entry in 'used' DB */
    if(isValid_prevEntryPointer)
    {
        /* we can move it */
        nodePtr->entryPointer = prevEntryPointer;
    }
    else
    {
        /* we can't move it --> so invalidate it */
        nodePtr->isValid_entryPointer = 0;
        nodePtr->entryPointer         = 0;/* don't care */
    }

    return;
}

/**
* @internal deleteOldEntryFromDbMetadata_NodesCheck function
* @endinternal
*
* @brief   This function check impact on ALL 'node' that may point to the removed index.
*          check impact on :
*          tailOfUsedList ,headOfUsedList , lastGetNextInfo ,
*          lastTranslplantInfo , lastDeleteInfo
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] removed_dbIndex       - the db index of the removed entry
* @param[in] isValid_prevEntryPointer - indication that the removed entry hold
*                                   valid previous entry in the 'used list'.
* @param[in] prevEntryPointer      -  the previous entry index in the 'used list'.
* @param[in] isValid_nextEntryPointer - indication that the removed entry hold
*                                   valid next entry in the 'used list'.
* @param[in] nextEntryPointer      -  the next entry index in the 'used list'.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE /GT_BAD_PARAM - non-consistency between different parts of
*                                    the manager.
*
*/
static GT_STATUS   deleteOldEntryFromDbMetadata_NodesCheck
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC     *fdbManagerPtr,
    IN GT_U32                                   removed_dbIndex,
    IN GT_U32                                   isValid_prevEntryPointer,
    IN GT_U32                                   prevEntryPointer,
    IN GT_U32                                   isValid_nextEntryPointer,
    IN GT_U32                                   nextEntryPointer
)
{
    /*****************************/
    /* update the tailOfUsedList */
    /*****************************/
    CHECK_X_NOT_ZERO_MAC(fdbManagerPtr->tailOfUsedList.isValid_entryPointer);
    deleteOldEntryFromDbMetadata_specificNode(removed_dbIndex,
            isValid_prevEntryPointer,
            prevEntryPointer,
            &fdbManagerPtr->tailOfUsedList);

    /*****************************/
    /* update the headOfUsedList */
    /*****************************/
    CHECK_X_NOT_ZERO_MAC(fdbManagerPtr->headOfUsedList.isValid_entryPointer);
    if(fdbManagerPtr->headOfUsedList.entryPointer == removed_dbIndex)
    {
        /* we remove the 'head of used' ... need to move the 'head' to next */
        fdbManagerPtr->headOfUsedList.isValid_entryPointer = isValid_nextEntryPointer;
        fdbManagerPtr->headOfUsedList.entryPointer = nextEntryPointer;
    }


    /******************************/
    /* update the lastGetNextInfo */
    /******************************/
    deleteOldEntryFromDbMetadata_specificNode(removed_dbIndex,
            isValid_prevEntryPointer,
            prevEntryPointer,
            &fdbManagerPtr->lastGetNextInfo);

    /**********************************/
    /* update the lastTranslplantInfo */
    /**********************************/
    deleteOldEntryFromDbMetadata_specificNode(removed_dbIndex,
            isValid_prevEntryPointer,
            prevEntryPointer,
            &fdbManagerPtr->lastTranslplantInfo);

    /******************************/
    /* update the lastDeleteInfo  */
    /******************************/
    deleteOldEntryFromDbMetadata_specificNode(removed_dbIndex,
            isValid_prevEntryPointer,
            prevEntryPointer,
            &fdbManagerPtr->lastDeleteInfo);

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerDbDeleteOldEntry function
* @endinternal
*
* @brief   This function return entry to the free list removing from the used list.
*           delete old entry :
*           update metadata DB :indexArr[] , bankInfoArr[] , typeCountersArr[]
*           update headOfFreeList , headOfUsedList , tailOfUsedList
*           update calcInfoPtr->dbEntryPtr about valid pointers
*           lastGetNextInfo , lastTranslplantInfo , lastDeleteInfo
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] primaryBank           - selected bank to adding entry:
*                                     GT_TRUE  - primary bank ID
*                                     GT_FALSE - secondary bank ID - for IPv6 FDB entry type only
* @param[in] calcInfoPtr           - using calcInfoPtr->selectedBankId ,
*                                          calcInfoPtr->counterType ,
*                                          calcInfoPtr->dbEntryPtr ,
*                                          calcInfoPtr->crcMultiHashArr[] ,
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE /GT_BAD_PARAM - non-consistency between different parts of
*                                    the manager.
*
*/
GT_STATUS   prvCpssDxChFdbManagerDbDeleteOldEntry
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN GT_BOOL                                                 primaryBank,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC        *calcInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32  bankId;
    GT_U32  dbIndex;
    GT_U32  hwIndex;
    GT_U32  currFree_dbIndex;
    GT_U32  prev_dbIndex;
    GT_U32  next_dbIndex;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC *freeInPoolPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC *dbEntryPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC *prevInPoolPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC *nextInPoolPtr;
    GT_U32 isValid_prevEntryPointer;
    GT_U32 prevEntryPointer;
    GT_BOOL checkToDecrementUcLimit;

    if (primaryBank)
    {
        dbEntryPtr = calcInfoPtr->dbEntryPtr;
        bankId  = calcInfoPtr->selectedBankId;
    }
    else
    {
        dbEntryPtr = calcInfoPtr->dbEntryExtPtr;
        bankId  = calcInfoPtr->selectedBankId + 1;
    }

    if(fdbManagerPtr->dynamicUcMacLimitPtr &&
       dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E)
    {
        /* fill info about the current entry , into:
            fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo
           to be used at the end when calling
           prvCpssDxChFdbManagerDbLimitOper(fdbManagerPtr,LIMIT_OPER_DELETE_OLD_E);
           */
        (void)prvCpssDxChFdbManagerDbIsOverLimitCheck(fdbManagerPtr,
            LIMIT_OPER_CHECK_CURRENT_E,
            &dbEntryPtr->specificFormat.prvMacEntryFormat);

        checkToDecrementUcLimit = GT_TRUE;
    }
    else
    {
        checkToDecrementUcLimit = GT_FALSE;
    }


    isValid_prevEntryPointer = dbEntryPtr->isValid_prevEntryPointer;
    prevEntryPointer         = dbEntryPtr->prevEntryPointer;

    /* protect access to array out of range */
    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(bankId,CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_BANKS_CNS);

    /*********************************************/
    /* update bankInfoArr[].bankPopulation */
    /*********************************************/
    /* coherency check -- can't be ZERO because we are going to decrement it */
    CHECK_X_NOT_ZERO_MAC(fdbManagerPtr->bankInfoArr[bankId].bankPopulation);

    /* update the number of entries in the bank */
    fdbManagerPtr->bankInfoArr[bankId].bankPopulation--;

    /* coherency check -- can't be ZERO because we are going to decrement it */
    CHECK_X_NOT_ZERO_MAC(fdbManagerPtr->totalPopulation);
    fdbManagerPtr->totalPopulation--;

    hwIndex = calcInfoPtr->crcMultiHashArr[bankId];


    CHECK_X_NOT_ZERO_MAC(fdbManagerPtr->indexArr[hwIndex].isValid_entryPointer);
    dbIndex = fdbManagerPtr->indexArr[hwIndex].entryPointer;
    /*********************/
    /* update indexArr[] */
    /*********************/
    fdbManagerPtr->indexArr[hwIndex].isValid_entryPointer = 0;
    fdbManagerPtr->indexArr[hwIndex].entryPointer         = 0;/* don't care */

    /***********************************/
    if(dbEntryPtr->isValid_nextEntryPointer)
    {
        /*next_dbIndex = dbEntryPtr->nextEntryPointer;*/
        GET_nextEntryPointer_MAC(dbEntryPtr,next_dbIndex);
    }
    else
    {
        next_dbIndex = GT_NA;
    }

    if(dbEntryPtr->isValid_prevEntryPointer)
    {
        /* update the previous dbEntry to point to the next of the removed dbEntry entry */
        prev_dbIndex = dbEntryPtr->prevEntryPointer;
    }
    else
    {
        prev_dbIndex = GT_NA;
    }

    if(prev_dbIndex != GT_NA)
    {
        /***********************************/
        /* update prev entry in   the pool */
        /* to point to next  in   the pool */
        /***********************************/
        prevInPoolPtr = &fdbManagerPtr->entryPoolPtr[prev_dbIndex];
        if(next_dbIndex != GT_NA)
        {
            /*prevInPoolPtr->nextEntryPointer = next_dbIndex;*/
            SET_nextEntryPointer_MAC(prevInPoolPtr,next_dbIndex);
        }
        else
        {
            prevInPoolPtr->isValid_nextEntryPointer = 0;
            /*prevInPoolPtr->nextEntryPointer         = 0;*//* don't care */
            SET_nextEntryPointer_MAC(prevInPoolPtr,0);/* don't care */
        }
    }

    if(next_dbIndex != GT_NA)
    {
        /***********************************/
        /* update next entry in   the pool */
        /* to point to prev  in   the pool */
        /***********************************/
        nextInPoolPtr = &fdbManagerPtr->entryPoolPtr[next_dbIndex];
        if(prev_dbIndex != GT_NA)
        {
            nextInPoolPtr->prevEntryPointer = prev_dbIndex;
        }
        else
        {
            nextInPoolPtr->isValid_prevEntryPointer = 0;
            nextInPoolPtr->prevEntryPointer         = 0;/* don't care */
        }
    }

    /**************************************************/
    /* move curr entry from the 'used' to 'free' list */
    /**************************************************/
    if(!fdbManagerPtr->headOfFreeList.isValid_entryPointer)
    {
        /* the 'old' have no 'next' */
        dbEntryPtr->isValid_nextEntryPointer = 0;
        /*dbEntryPtr->nextEntryPointer         = 0;*//* don't care */
        SET_nextEntryPointer_MAC(dbEntryPtr,0);/* don't care */
    }
    else
    {
        /* the 'old' have 'next' */
        dbEntryPtr->isValid_nextEntryPointer = 1;
        /*dbEntryPtr->nextEntryPointer         = fdbManagerPtr->headOfFreeList.entryPointer;*/
        SET_nextEntryPointer_MAC(dbEntryPtr,fdbManagerPtr->headOfFreeList.entryPointer);
    }

    /**********************************/
    /* update the ageBin related info */
    /**********************************/
    if (dbEntryPtr->ageBinIndex >= fdbManagerPtr->agingBinInfo.totalAgeBinAllocated)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    CHECK_X_NOT_ZERO_MAC(fdbManagerPtr->agingBinInfo.ageBinListPtr[dbEntryPtr->ageBinIndex].headOfAgeBinEntryList.isValid_entryPointer);

    if(fdbManagerPtr->agingBinInfo.ageBinListPtr[dbEntryPtr->ageBinIndex].headOfAgeBinEntryList.entryPointer == dbIndex)
    {
        /* HEAD of the age-bin list */
        fdbManagerPtr->agingBinInfo.ageBinListPtr[dbEntryPtr->ageBinIndex].headOfAgeBinEntryList.isValid_entryPointer = dbEntryPtr->isValid_age_nextEntryPointer;
        fdbManagerPtr->agingBinInfo.ageBinListPtr[dbEntryPtr->ageBinIndex].headOfAgeBinEntryList.entryPointer = dbEntryPtr->age_nextEntryPointer;

        if(dbEntryPtr->isValid_age_nextEntryPointer)
        {
            /* Head of List is first one and does not have previous member */
            nextInPoolPtr = &fdbManagerPtr->entryPoolPtr[dbEntryPtr->age_nextEntryPointer];
            nextInPoolPtr->isValid_age_prevEntryPointer = 0;
        }
    }
    else
    {
        /* Middle/End of a age-Bin List */
        if(dbEntryPtr->isValid_age_prevEntryPointer == 0)
        {
            /* middle/end member must have valid previous pointer. */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        prevInPoolPtr = &fdbManagerPtr->entryPoolPtr[dbEntryPtr->age_prevEntryPointer];
        prevInPoolPtr->isValid_age_nextEntryPointer = dbEntryPtr->isValid_age_nextEntryPointer;
        prevInPoolPtr->age_nextEntryPointer         = dbEntryPtr->age_nextEntryPointer;

        /* If its not the last entry */
        if(dbEntryPtr->isValid_age_nextEntryPointer)
        {
            nextInPoolPtr = &fdbManagerPtr->entryPoolPtr[dbEntryPtr->age_nextEntryPointer];
            nextInPoolPtr->isValid_age_prevEntryPointer = dbEntryPtr->isValid_age_prevEntryPointer;
            nextInPoolPtr->age_prevEntryPointer         = dbEntryPtr->age_prevEntryPointer;
        }
    }
    DECREASE_AGE_BIN_COUNTER_MAC(fdbManagerPtr, dbEntryPtr->ageBinIndex);

    /********************************/
    /* invalidate the removed entry */
    /********************************/
    CHECK_X_NOT_ZERO_MAC(dbEntryPtr->isUsedEntry);
    dbEntryPtr->isUsedEntry = 0;
    dbEntryPtr->hwIndex     = 0;/* don't care */
    dbEntryPtr->isValid_prevEntryPointer = 0;
    dbEntryPtr->prevEntryPointer = 0;/* don't care */

    /*****************************/
    /* update the headOfFreeList */
    /*****************************/
    if(!fdbManagerPtr->headOfFreeList.isValid_entryPointer)
    {
        fdbManagerPtr->headOfFreeList.isValid_entryPointer = 1;
    }
    else
    {
        currFree_dbIndex = fdbManagerPtr->headOfFreeList.entryPointer;

        freeInPoolPtr = &fdbManagerPtr->entryPoolPtr[currFree_dbIndex];

        freeInPoolPtr->isValid_prevEntryPointer = 1;
        freeInPoolPtr->prevEntryPointer         = dbIndex;
    }

    fdbManagerPtr->headOfFreeList.entryPointer = dbIndex;


    /********************************************/
    /* check impact on : tailOfUsedList         */
    /* check impact on : headOfUsedList         */
    /* check impact on : lastGetNextInfo        */
    /* check impact on : lastTranslplantInfo    */
    /* check impact on : lastDeleteInfo         */
    /********************************************/
    /* check impact on ALL 'node' that may point to the removed index */
    rc = deleteOldEntryFromDbMetadata_NodesCheck(fdbManagerPtr,dbIndex,
            isValid_prevEntryPointer,prevEntryPointer,
            (next_dbIndex != GT_NA)?1:0,next_dbIndex);
    if(rc != GT_OK)
    {
        return rc;
    }

    /********************************/
    /* update the typeCountersArr */
    /********************************/

    if (calcInfoPtr->counterType >= PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE__LAST__E)
    {
        /* should never happen */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* coherency check -- can't be ZERO because we are going to decrement it */
    CHECK_X_NOT_ZERO_MAC(fdbManagerPtr->typeCountersArr[calcInfoPtr->counterType].entryTypePopulation);
    fdbManagerPtr->typeCountersArr[calcInfoPtr->counterType].entryTypePopulation--;

    if(checkToDecrementUcLimit == GT_TRUE)
    {
        rc = prvCpssDxChFdbManagerDbLimitOper(fdbManagerPtr,LIMIT_OPER_DELETE_OLD_E);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerDbDevListAdd function
* @endinternal
*
* @brief   This function adding (registering) device(s) to specific FDB Manager Instance.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] devListArr[]          - the array of device ids to add to the FDB Manager.
* @param[in] numOfDevs             - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChFdbManagerDbDevListAdd
(
    IN GT_U32 fdbManagerId,
    IN GT_U8  devListArr[], /*arrSizeVarName=numOfDevs*/
    IN GT_U32 numOfDevs
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;
    GT_U32  ii;
    GT_U32  areOverlap;
    GT_U8   devNum;
    GT_U32  numOfHashes;
    GT_HW_DEV_NUM   hwDevNum;
    GT_U32  numOfEports;

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    if(numOfDevs > PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "numOfDevs [%d] > max[%d] ",
            numOfDevs,
            PRV_CPSS_MAX_PP_DEVICES_CNS);
    }
    else
    if(numOfDevs == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "numOfDevs is ZERO ... no meaning calling the function");
    }

    /***********************************************/
    /* validity check on the 'to be added' devices */
    /***********************************************/
    for(ii = 0 ; ii < numOfDevs ; ii++)
    {
        devNum = devListArr[ii];

        if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "device number is not supported");
        }

        /* check if the device already registered with any of the FDB managers */
        if(IS_DEV_BMP_SET_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(prvCpssDxChFdbManagerDbGlobalDevsBmpArr),devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST,
                "devNum[%d] already registered at one of the FDB managers",
                devNum);
        }

        /* we lock the CPSS DB ... to be able to access 'PRV_CPSS_PP_MAC(devNum)' */
        /* we must unlock this DB if exit on ERROR !!! */
        LOCK_DEV_NUM(devNum);

        /* check that the device supports the FDB manager */
        ARE_2_BMPS_OVERLAP_MAC(fdbManagerPtr->supportedSipArr,
                     PRV_CPSS_PP_MAC(devNum)->supportedSipArr,
                     areOverlap);
        if(areOverlap == 0)
        {
            UNLOCK_DEV_NUM(devNum);/* release DB before we exists on error */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,
                "devNum[%d] is not applicable for this FDB manager (not supported)",
                devNum);
        }

        if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb <
           fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes)
        {
            UNLOCK_DEV_NUM(devNum);/* release DB before we exists on error */

            /* the 'fineTuning.tableSize.fdb' reflect the FDB size as given to the 'shared tables' */
            /* so we can't have lower value */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "FDB size in 'shared tables' is [%d] and not support FDB manager [%d] entries",
                    PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb,
                    fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);
        }

        if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                switch (PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbNumOfBitsPerBank)
                {
                    case  2:
                        numOfHashes =  4;
                        break;
                    case 3:
                        numOfHashes =  8;
                        break;
                    default:
                        numOfHashes = 16;
                        break;
                }
            }
            else
            {
                numOfHashes = 16;
            }

            if (fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes != numOfHashes)
            {
                UNLOCK_DEV_NUM(devNum);/* release DB before we exists on error */

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                        "Number of hashes[%d] for FDB size[%d] in 'shared tables' is not supported for SIP6 devices",
                        fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes, fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);
            }
            fdbManagerPtr->cpssHashParams.numOfMultipleHashes = numOfHashes;

            if(fdbManagerPtr->entryAttrInfo.globalEportInfo.enable ==
                CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E)
            {
                /* check if the 'mask' allow to use logic of {min,max} values that sip6 uses in HW */
                rc = prvCpssDxChSip6GlobalEportMaskCheck(devNum,
                    fdbManagerPtr->entryAttrInfo.globalEportInfo.mask,
                    fdbManagerPtr->entryAttrInfo.globalEportInfo.pattern,
                    &fdbManagerPtr->entryAttrInfo.globalEportInfo.minValue,
                    &fdbManagerPtr->entryAttrInfo.globalEportInfo.maxValue);
                if(rc != GT_OK)
                {
                    UNLOCK_DEV_NUM(devNum);/* release DB before we exists on error */

                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "the globalEportInfo.mask[0x%5.5x] must be with continues bits (or use explicit 'minValue' and 'max Value')",
                                fdbManagerPtr->entryAttrInfo.globalEportInfo.mask);
                }
            }
        }
        else
        {
            fdbManagerPtr->cpssHashParams.numOfMultipleHashes = 4;
        }

        UNLOCK_DEV_NUM(devNum);/* we are done with this device internal DB */
    }

    /* after parameter checking we can start doing 'malloc' */
    for(ii = 0 ; ii < numOfDevs ; ii++)
    {
        devNum = devListArr[ii];
        /* we lock the CPSS DB ... to be able to access 'PRV_CPSS_PP_MAC(devNum)' */
        /* we must unlock this DB if exit on ERROR !!! */
        LOCK_DEV_NUM(devNum);
        /* use the hwDevNum to allocate the relevant */
        hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

        if(fdbManagerPtr->dynamicUcMacLimitPtr)
        {
            if(fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum])
            {
                UNLOCK_DEV_NUM(devNum);/* release DB before we exists on error */

                /* release mallocs on error */
                (void)prvCpssDxChFdbManagerDbDevList_free_mallocs(fdbManagerId,
                    devListArr,
                    numOfDevs);

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                    "the hwDevNum[0x%3.3x] for devNum[%d] is already used in the DB",
                            hwDevNum,devNum);
            }

            fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum] =
                cpssOsMalloc(sizeof(PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_PER_HW_DEVICE_STC));

            if(NULL == fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum])
            {
                UNLOCK_DEV_NUM(devNum);/* release DB before we exists on error */
                /* release mallocs on error */
                (void)prvCpssDxChFdbManagerDbDevList_free_mallocs(fdbManagerId,
                    devListArr,
                    numOfDevs);

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                    "Failed to allocate memory for dynamicUcMacLimit_perHwDev[0x%3.3x]",
                            hwDevNum);
            }

            PRV_SHARED_FDB_MANAGER_DB_VAR_GET(prvCpssDxChFdbManagerDbArr)[fdbManagerId]->numOfAllocatedPointers++;

            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                numOfEports = PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(devNum);
            }
            else
            {
                numOfEports = PRV_CPSS_PP_MAC(devNum)->numOfPorts;
            }


            fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum]->numPorts = numOfEports;

            fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum]->perEportPtr =
                cpssOsMalloc(numOfEports * sizeof(PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_STC));

            if(NULL == fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum]->perEportPtr)
            {
                UNLOCK_DEV_NUM(devNum);/* release DB before we exists on error */

                /* release mallocs on error */
                (void)prvCpssDxChFdbManagerDbDevList_free_mallocs(fdbManagerId,
                    devListArr,
                    numOfDevs);

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                    "Failed to allocate memory for dynamicUcMacLimit_perHwDev[0x%3.3x]->perEportPtr for numPorts[%d]",
                            hwDevNum,numOfEports);
            }

            PRV_SHARED_FDB_MANAGER_DB_VAR_GET(prvCpssDxChFdbManagerDbArr)[fdbManagerId]->numOfAllocatedPointers++;

            for(ii = 0 ; ii < numOfEports ; ii++)
            {
                fdbManagerPtr->dynamicUcMacLimitPtr->
                    dynamicUcMacLimit_perHwDev[hwDevNum]->perEportPtr[ii].limitThreshold =
                    fdbManagerPtr->capacityInfo.maxDynamicUcMacPortLimit;

                fdbManagerPtr->dynamicUcMacLimitPtr->
                    dynamicUcMacLimit_perHwDev[hwDevNum]->perEportPtr[ii].actualCounter = 0;
            }

        }

        UNLOCK_DEV_NUM(devNum);/* we are done with this device internal DB */
    }

    /* start DB operations on the new devices */
    for(ii = 0 ; ii < numOfDevs ; ii++)
    {
        devNum = devListArr[ii];

        /* state that the device was added to global DB of device */
        DEV_BMP_SET_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(prvCpssDxChFdbManagerDbGlobalDevsBmpArr),devNum);

        /* state that the device was added to local DB of current manager */
        DEV_BMP_SET_MAC(fdbManagerPtr->devsBmpArr,devNum);

        /* update devices counter */
        fdbManagerPtr->numOfDevices++;
    }


    return GT_OK;
}


/**
* @internal prvCpssDxChFdbManagerDbDevListRemove function
* @endinternal
*
* @brief   This function removing (unregistering) device(s) from specific FDB Manager Instance.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] devListArr[]          - the array of device ids to remove from the FDB Manager.
* @param[in] numOfDevs             - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NO_SUCH               - if one of the devices not in DB.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChFdbManagerDbDevListRemove
(
    IN GT_U32 fdbManagerId,
    IN GT_U8  devListArr[], /*arrSizeVarName=numOfDevs*/
    IN GT_U32 numOfDevs
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;
    GT_U32  ii;
    GT_U8   devNum;
    GT_STATUS rc;
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT    removedDeviceType, currentDeviceType;

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    if(numOfDevs > PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "numOfDevs [%d] > max[%d] ",
            numOfDevs,
            PRV_CPSS_MAX_PP_DEVICES_CNS);
    }
    else
    if(numOfDevs == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "numOfDevs is ZERO ... no meaning calling the function");
    }

    /*************************************************/
    /* validity check on the 'to be removed' devices */
    /*************************************************/
    for(ii = 0 ; ii < numOfDevs ; ii++)
    {
        devNum = devListArr[ii];

        /* check that the device registered with the FDB managers */
        if(!IS_DEV_BMP_SET_MAC(fdbManagerPtr->devsBmpArr,devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH,
                "devNum[%d] is not registered at manager",
                devNum);
        }
    }

    /* remove devices from the bmp of devices */
    for(ii = 0 ; ii < numOfDevs ; ii++)
    {
        devNum = devListArr[ii];

        /* state that the device removed from the global DB of devices */
        DEV_BMP_CLEAR_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(prvCpssDxChFdbManagerDbGlobalDevsBmpArr),devNum);

        /* state that the device removed from the local DB of devices in the manager*/
        DEV_BMP_CLEAR_MAC(fdbManagerPtr->devsBmpArr,devNum);

        /* update devices counter */
        /* coherency check -- can't be ZERO because we are going to decrement it */
        CHECK_X_NOT_ZERO_MAC(fdbManagerPtr->numOfDevices);
        fdbManagerPtr->numOfDevices--;
    }

    /* free mallocs related to those devices */
    rc = prvCpssDxChFdbManagerDbDevList_free_mallocs(fdbManagerId,
        devListArr,
        numOfDevs);
    if(rc != GT_OK)
    {
        return rc;
    }

    if((fdbManagerPtr->entryAttrInfo.shadowType != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E) &&
        (fdbManagerPtr->entryAttrInfo.shadowType != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E))
    {
        return GT_OK;
    }

    /* Remove device type, if its the last device of this type */
    rc = prvCpssDxChFdbManagerDeviceHwFormatGet(fdbManagerPtr, devNum, &removedDeviceType);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(ii = 0; ii < fdbManagerPtr->numOfDevices; ii++)
    {
        if(IS_DEV_BMP_SET_MAC(fdbManagerPtr->devsBmpArr, ii))
        {
            rc = prvCpssDxChFdbManagerDeviceHwFormatGet(fdbManagerPtr, ii, &currentDeviceType);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* Similar device type is still exist in fdb manager, so do not remove the device type */
            if(currentDeviceType == removedDeviceType)
            {
                return GT_OK;
            }
        }
    }

    /* Update the bitmap */
    fdbManagerPtr->hwDeviceTypesBitmap &= (~removedDeviceType);
    return GT_OK;
}

/*
* @internal prvCpssDxChFdbManagerDbApplicationIteratorEntryGetNext function
* @endinternal
*
* @brief   The function return the first/next (pointer to) entry.
*          it is based on 'internally managed last entry' that the manager
*          returned 'last time' to the application.
*
*          NOTE: this function is to be used only for APPLICATION specific 'get next'
*                and not for iterators inside the manager.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[in] getFirst              - indication to get First (GT_TRUE)  or the next (GT_FALSE)
* @param[inout] lastGetNextInfoPtr - Indicates the last DB node fetched by this scan
*                                    OR - DB pointer from where to start the next search.
* @param[out] dbEntryPtrPtr        - (pointer to)  the '(pointer to)first/next entry'
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_BAD_PARAM             - the iterator is not valid for the 'get next' operation
*                                    the logic found coherency issue.
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChFdbManagerDbApplicationIteratorEntryGetNext
(
    IN    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr,
    IN    GT_BOOL                                          getFirst,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC       *lastGetNextInfoPtr,
    OUT   PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC  **dbEntryPtrPtr
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr;
    GT_U32                                  dbIndex;

    if(getFirst == GT_FALSE &&
       !lastGetNextInfoPtr->isValid_entryPointer)
    {
        /* the caller invalidated the 'get next' that was the first entry on previous call */
        getFirst = GT_TRUE;
    }

    if(getFirst == GT_TRUE)
    {
        *lastGetNextInfoPtr = fdbManagerPtr->headOfUsedList;
        if(!lastGetNextInfoPtr->isValid_entryPointer)
        {
            return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
        }
    }

    dbIndex = lastGetNextInfoPtr->entryPointer;
    /* protect access to array out of range */
    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex,fdbManagerPtr->capacityInfo.maxTotalEntries);

    dbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbIndex];

    if(getFirst != GT_TRUE)
    {
        /* check that we have valid 'next' to jump to it */
        if(!dbEntryPtr->isValid_nextEntryPointer)
        {
            /* not updating lastGetNextInfoPtr ! */

            return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
        }

        /* jump to next entry */
        /*dbIndex = dbEntryPtr->nextEntryPointer;*/
        GET_nextEntryPointer_MAC(dbEntryPtr,dbIndex);

        /* protect access to array out of range */
        CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex,fdbManagerPtr->capacityInfo.maxTotalEntries);

        dbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbIndex];

        /* update the 'getNext' last info */
        lastGetNextInfoPtr->isValid_entryPointer = 1;
        lastGetNextInfoPtr->entryPointer         = dbIndex;
    }

    *dbEntryPtrPtr = dbEntryPtr;

    return GT_OK;
}

/*
* @internal prvCpssDxChBrgFdbManagerDbDevListGet function
* @endinternal
*
* @brief The function returns current FDB Manager registered devices (added by 'add device' API).
*
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[inout] numOfDevicesPtr    - (pointer to) the number of devices in the array deviceListArray as 'input parameter'
*                                    and actual number of devices as 'output parameter'.
* @param[out] deviceListArray[]    - array of registered devices (no more than list length).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS prvCpssDxChBrgFdbManagerDbDevListGet
(
    IN GT_U32           fdbManagerId,
    INOUT GT_U32        *numOfDevicesPtr,
    OUT GT_U8           deviceListArray[] /*arrSizeVarName=*numOfDevicesPtr*/
)
{
    GT_U32 numOfDevs = *numOfDevicesPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;
    GT_U32  ii, jj;
    GT_U8   devNum;

    if(numOfDevs > PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "numOfDevs [%d] > max[%d] ",
            numOfDevs,
            PRV_CPSS_MAX_PP_DEVICES_CNS);
    }
    else
    if(numOfDevs == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "numOfDevs is ZERO ... no meaning calling the function");
    }

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    jj = 0;

    /* the input number of devices should not exceed the actual number in DB */
    if (numOfDevs > fdbManagerPtr->numOfDevices)
    {
        numOfDevs = fdbManagerPtr->numOfDevices;
    }
    /* retrieve devices from FDB manager DB */
    for(ii = 0; ii < numOfDevs; ii++)
    {
        if(IS_DEV_BMP_SET_MAC(fdbManagerPtr->devsBmpArr, ii))

        {
            devNum = (GT_U8)ii;
            deviceListArray[jj++] = devNum;
        }
    }

    /* actual number of retrieved devices */
    *numOfDevicesPtr = jj;

    return GT_OK;
}

/*
* @internal prvCpssDxChBrgFdbManagerDbConfigGet function
* @endinternal
*
* @brief The function fetches current FDB Manager setting as they were defined in FDB Manager Create.
*
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] capacityPtr          - (pointer to) parameters of the HW structure and capacity of its managed PPs FDBs:
*                                    like number of indexes, the amount of hashes etc.
*                                    As FDB Manager is created and entries may be added before PP registered,
*                                    we must know in advance the relevant capacity.
*                                    In Falcon, for example, we must know the shared memory allocation mode.
* @param[out] entryAttrPtr         - (pointer to) parameters that affects how HW/SW entries are
*                                    structured and specific fields behavior including:
*                                    MUX'd fields in MAC / IPMC entries format, SA / DA drop commands mode and
*                                    IP NH packet command.
* @param[out] learningPtr          - (pointer to) parameters that affects which sort of entries to learn etc.
*                                    (Learn MAC of Routed packets and MAC no-space updates).
* @param[out] lookupPtr            - (pointer to) parameters that affect lookup of entries: MAC VLAN Lookup mode
*                                    (none, single or double tags), IVL single-tag MSB mode.
*                                    And two new Falcon setting: IPv4 UC and IPv6 UC masks
*                                    (allow using FDB for prefixes lookup and not just exact match)
* @param[out] agingPtr             - (pointer to) parameters that affect refresh and aging
*                                      (Refresh Destination UC, Refresh Destination MC &Refresh IP UC)
*
*/
void prvCpssDxChBrgFdbManagerDbConfigGet
(
    IN  GT_U32                                          fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC          *capacityPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC          *learningPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC            *lookupPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC             *agingPtr
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;

    /* we have valid new manager pointer */
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /*****************/
    /* extract the info */
    /*****************/
    *capacityPtr    = fdbManagerPtr->capacityInfo;
    *entryAttrPtr   = fdbManagerPtr->entryAttrInfo;
    *learningPtr    = fdbManagerPtr->learningInfo;
    *lookupPtr      = fdbManagerPtr->lookupInfo;
    *agingPtr       = fdbManagerPtr->agingInfo;
}

/*
* @prvCpssDxChBrgFdbManagerDbCountersGet function
* @endinternal
*
* @brief The function fetches FDB Manager counters.
*
*
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] countersPtr          - (pointer to) FDB Manager Counters.
*
*/
void prvCpssDxChBrgFdbManagerDbCountersGet
(
    IN GT_U32                                           fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC          *countersPtr
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_ENT     countertype;
    GT_U32                                          *counterValuePtr;
    GT_U32                                          usedEntries;
    GT_U32                                          bankId;

    /* we have valid new manager pointer */
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    cpssOsMemSet(countersPtr, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC));

    usedEntries = 0;
    countertype = PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_DYNAMIC_E;

    while (countertype < PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE__LAST__E)
    {
        counterValuePtr = NULL;
        switch (countertype)
        {
            case PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_DYNAMIC_E:
                counterValuePtr = &countersPtr->macUnicastDynamic;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_STATIC_E:
                counterValuePtr = &countersPtr->macUnicastStatic;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_MULTICAST_DYNAMIC_E:
                counterValuePtr = &countersPtr->macMulticastDynamic;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_MULTICAST_STATIC_E:
                counterValuePtr = &countersPtr->macMulticastStatic;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV4_MULTICAST_E:
                counterValuePtr = &countersPtr->ipv4Multicast;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_MULTICAST_E:
                counterValuePtr = &countersPtr->ipv6Multicast;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV4_UNICAST_ROUTE_E:
                counterValuePtr = &countersPtr->ipv4UnicastRoute;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_UNICAST_ROUTE_KEY_E:
                counterValuePtr = &countersPtr->ipv6UnicastRouteKey;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_UNICAST_ROUTE_DATA_E:
                counterValuePtr = &countersPtr->ipv6UnicastRouteData;
                break;
            default:
                break;
        }

        if (counterValuePtr)
        {
            *counterValuePtr = fdbManagerPtr->typeCountersArr[countertype].entryTypePopulation;
            usedEntries += *counterValuePtr;
        }
        countertype++;
    }

    /* number of used entries */
    countersPtr->usedEntries = usedEntries;
    /* number of free entries - total entries requested by application in Create API minus used entries */
    countersPtr->freeEntries = fdbManagerPtr->capacityInfo.maxTotalEntries - usedEntries;

    for (bankId = 0; bankId < CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_BANKS_CNS; bankId++)
    {
        /* counters per bank */
        countersPtr->bankCounters[bankId] = fdbManagerPtr->bankInfoArr[bankId].bankPopulation;
    }
}

/*
* @prvCpssDxChBrgFdbManagerDbStatisticsGet function
* @endinternal
*
* @brief The function fetches FDB Manager statistics.
*
*
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] statisticsPtr        - (pointer to) FDB Manager Statistics.
*
*/
void prvCpssDxChBrgFdbManagerDbStatisticsGet
(
    IN GT_U32                                           fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC        *statisticsPtr
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC              *fdbManagerPtr;
    GT_U32                                                      statisticType;
    GT_U32                                                      *statisticValuePtr;

    /* we have valid new manager pointer */
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    cpssOsMemSet(statisticsPtr, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC));

    /* add statistics */
    statisticType = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_INPUT_INVALID_E;
    while (statisticType < PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch (statisticType)
        {
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_INPUT_INVALID_E:
                statisticValuePtr = &statisticsPtr->entryAddErrorInputInvalid;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_BAD_STATE_E:
                statisticValuePtr = &statisticsPtr->entryAddErrorBadState;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_ENTRY_EXIST_E:
                statisticValuePtr = &statisticsPtr->entryAddErrorEntryExist;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TABLE_FULL_E:
                statisticValuePtr = &statisticsPtr->entryAddErrorTableFull;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_HW_UPDATE_E:
                statisticValuePtr = &statisticsPtr->entryAddErrorHwUpdate;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_0_E:
                statisticValuePtr = &statisticsPtr->entryAddOkRehashingStage0;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_1_E:
                statisticValuePtr = &statisticsPtr->entryAddOkRehashingStage1;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_2_E:
                statisticValuePtr = &statisticsPtr->entryAddOkRehashingStage2;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_3_E:
                statisticValuePtr = &statisticsPtr->entryAddOkRehashingStage3;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_4_E:
                statisticValuePtr = &statisticsPtr->entryAddOkRehashingStage4;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_5_AND_ABOVE_E:
                statisticValuePtr = &statisticsPtr->entryAddOkRehashingStageMoreThanFour;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_PORT_LIMIT_E  :
                statisticValuePtr = &statisticsPtr->entryAddErrorPortLimit;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TRUNK_LIMIT_E :
                statisticValuePtr = &statisticsPtr->entryAddErrorTrunkLimit;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_EPORT_LIMIT_E:
                statisticValuePtr = &statisticsPtr->entryAddErrorGlobalEPortLimit;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_FID_LIMIT_E   :
                statisticValuePtr = &statisticsPtr->entryAddErrorFidLimit;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_LIMIT_E:
                statisticValuePtr = &statisticsPtr->entryAddErrorGlobalLimit;
                break;
            default:
                continue;/* hold no convert */
        }

        if (statisticValuePtr)
        {
            *statisticValuePtr = fdbManagerPtr->apiEntryAddStatisticsArr[statisticType];
        }
        statisticType++;
    }

    /* delete statistics */
    statisticType = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E;
    while (statisticType < PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch (statisticType)
        {
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E:
                statisticValuePtr = &statisticsPtr->entryDeleteErrorInputInvalid;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_BAD_STATE_E:
                statisticValuePtr = &statisticsPtr->entryDeleteErrorBadState;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_ENTRY_NOT_FOUND_E:
                statisticValuePtr = &statisticsPtr->entryDeleteErrorNotfound;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_HW_UPDATE_E:
                statisticValuePtr = &statisticsPtr->entryDeleteErrorHwUpdate;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_OK_E:
                statisticValuePtr = &statisticsPtr->entryDeleteOk;
                break;
            default:
                continue;/* hold no convert */
        }

        if (statisticValuePtr)
        {
            *statisticValuePtr = fdbManagerPtr->apiEntryDeleteStatisticsArr[statisticType];
        }
        statisticType++;
    }

    /* Temp delete statistics */
    statisticType = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TEMP_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E;
    while (statisticType < PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TEMP_DELETE_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch (statisticType)
        {
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TEMP_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E:
                statisticValuePtr = &statisticsPtr->entryTempDeleteErrorInputInvalid;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TEMP_DELETE_API_STATISTICS_OK_E:
                statisticValuePtr = &statisticsPtr->entryTempDeleteOk;
                break;
            default:
                continue;/* hold no convert */
        }

        if (statisticValuePtr)
        {
            *statisticValuePtr = fdbManagerPtr->apiEntryTempDeleteStatisticsArr[statisticType];
        }
        statisticType++;
    }

    /* update statistics */
    statisticType = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_INPUT_INVALID_E;
    while (statisticType < PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch (statisticType)
        {
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_INPUT_INVALID_E:
                statisticValuePtr = &statisticsPtr->entryUpdateErrorInputInvalid;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_BAD_STATE_E:
                statisticValuePtr = &statisticsPtr->entryUpdateErrorBadState;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_NOT_FOUND_E:
                statisticValuePtr = &statisticsPtr->entryUpdateErrorNotFound;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_HW_UPDATE_E:
                statisticValuePtr = &statisticsPtr->entryUpdateErrorHwUpdate;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_OK_E:
                statisticValuePtr = &statisticsPtr->entryUpdateOk;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_PORT_LIMIT_E  :
                statisticValuePtr = &statisticsPtr->entryUpdateErrorPortLimit;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_TRUNK_LIMIT_E :
                statisticValuePtr = &statisticsPtr->entryUpdateErrorTrunkLimit;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_GLOBAL_EPORT_LIMIT_E:
                statisticValuePtr = &statisticsPtr->entryUpdateErrorGlobalEPortLimit;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_FID_LIMIT_E   :
                statisticValuePtr = &statisticsPtr->entryUpdateErrorFidLimit;
                break;
            default:
                continue;/* hold no convert */
        }

        if (statisticValuePtr)
        {
            *statisticValuePtr = fdbManagerPtr->apiEntryUpdateStatisticsArr[statisticType];
        }
        statisticType++;
    }

    /* learning scan statistics */
    statisticType = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E;
    while (statisticType < PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch (statisticType)
        {
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E:
                statisticValuePtr = &statisticsPtr->scanLearningOk;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_NO_MORE_E:
                statisticValuePtr = &statisticsPtr->scanLearningOkNoMore;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_ERROR_INPUT_INVALID_E:
                statisticValuePtr = &statisticsPtr->scanLearningErrorInputInvalid;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_ERROR_FAILED_TABLE_UPDATE_E:
                statisticValuePtr = &statisticsPtr->scanLearningErrorFailedTableUpdate;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NA_MSG_E:
                statisticValuePtr = &statisticsPtr->scanLearningTotalHwEntryNewMessages;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E:
                statisticValuePtr = &statisticsPtr->scanLearningTotalHwEntryMovedMessages;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NO_SPACE_MSG_E:
                statisticValuePtr = &statisticsPtr->scanLearningTotalHwEntryNoSpaceMessages;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NO_SPACE_FILTERED_MSG_E:
                statisticValuePtr = &statisticsPtr->scanLearningTotalHwEntryNoSpaceFilteredMessages;
                break;
            default:
                continue;/* hold no convert */
        }

        if (statisticValuePtr)
        {
            *statisticValuePtr = fdbManagerPtr->apiLearningScanStatisticsArr[statisticType];
        }
        statisticType++;
    }

    /* aging scan scan statistics */
    statisticType = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_OK_E;
    while (statisticType < PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch (statisticType)
        {
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_OK_E:
                statisticValuePtr = &statisticsPtr->scanAgingOk;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_ERROR_INPUT_INVALID_E:
                statisticValuePtr = &statisticsPtr->scanAgingErrorInputInvalid;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_ERROR_FAILED_TABLE_UPDATE_E:
                statisticValuePtr = &statisticsPtr->scanAgingErrorFailedTableUpdate;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_E:
                statisticValuePtr = &statisticsPtr->scanAgingTotalAgedOutEntries;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_DELETED_E:
                statisticValuePtr = &statisticsPtr->scanAgingTotalAgedOutDeleteEntries;
                break;
            default:
                continue;/* hold no convert */
        }

        if (statisticValuePtr)
        {
            *statisticValuePtr = fdbManagerPtr->apiAgingScanStatisticsArr[statisticType];
        }
        statisticType++;
    }

    /* Delete scan scan statistics */
    statisticType = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_OK_E;
    while (statisticType < PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch (statisticType)
        {
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_OK_E:
                statisticValuePtr = &statisticsPtr->scanDeleteOk;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E:
                statisticValuePtr = &statisticsPtr->scanDeleteErrorInputInvalid;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_ERROR_FAILED_TABLE_UPDATE_E:
                statisticValuePtr = &statisticsPtr->scanDeleteErrorFailedTableUpdate;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_OK_NO_MORE_E:
                statisticValuePtr = &statisticsPtr->scanDeleteOkNoMore;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_TOTAL_DELETED_E:
                statisticValuePtr = &statisticsPtr->scanDeleteTotalDeletedEntries;
                break;
            default:
                continue;/* hold no convert */
        }

        if (statisticValuePtr)
        {
            *statisticValuePtr = fdbManagerPtr->apiDeleteScanStatisticsArr[statisticType];
        }
        statisticType++;
    }

    /* Transplant scan scan statistics */
    statisticType = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_OK_E;
    while (statisticType < PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch (statisticType)
        {
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_OK_E:
                statisticValuePtr = &statisticsPtr->scanTransplantOk;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_ERROR_INPUT_INVALID_E:
                statisticValuePtr = &statisticsPtr->scanTransplantErrorInputInvalid;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_ERROR_FAILED_TABLE_UPDATE_E:
                statisticValuePtr = &statisticsPtr->scanTransplantErrorFailedTableUpdate;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_OK_NO_MORE_E:
                statisticValuePtr = &statisticsPtr->scanTransplantOkNoMore;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_TOTAL_TRANSPLANTED_E:
                statisticValuePtr = &statisticsPtr->scanTransplantTotalTransplantedEntries;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_TOTAL_TRANSPLANTED_REACH_LIMIT_DELETED_E:
                statisticValuePtr = &statisticsPtr->scanTransplantTotalTransplantReachLimitDeletedEntries;
                break;
            default:
                continue;/* hold no convert */
        }

        if (statisticValuePtr)
        {
            *statisticValuePtr = fdbManagerPtr->apiTransplantScanStatisticsArr[statisticType];
        }
        statisticType++;
    }
    /* HA Sync statistics */
    statisticType = PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS_TOTAL_VALID_E;
    while (statisticType < PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch (statisticType)
        {
            case PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS_TOTAL_VALID_E:
                statisticValuePtr = &statisticsPtr->haSyncTotalValid;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS_TOTAL_INVALID_E:
                statisticValuePtr = &statisticsPtr->haSyncTotalInvalid;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS_TOTAL_SP_ENTRY_E:
                statisticValuePtr = &statisticsPtr->haSyncTotalSPEntry;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS_TOTAL_IPV6_DATA_INVALID_ENTRY_E:
                statisticValuePtr = &statisticsPtr->haSyncTotalIPv6DataInvalid;
                break;
            default:
                continue;/* hold no convert */
        }

        if (statisticValuePtr)
        {
            *statisticValuePtr = fdbManagerPtr->apiHASyncStatisticsArr[statisticType];
        }
        statisticType++;
    }
    /* Entries rewrite statistics */
    statisticType = PRV_CPSS_DXCH_FDB_MANAGER_ENTRIES_REWRITE_API_STATISTICS_OK_E;
    while (statisticType < PRV_CPSS_DXCH_FDB_MANAGER_ENTRIES_REWRITE_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch (statisticType)
        {
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRIES_REWRITE_API_STATISTICS_OK_E:
                statisticValuePtr = &statisticsPtr->entriesRewriteOk;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRIES_REWRITE_API_STATISTICS_ERROR_INPUT_INVALID_E:
                statisticValuePtr = &statisticsPtr->entriesRewriteErrorInputInvalid;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_ENTRIES_REWRITE_API_STATISTICS_TOTAL_REWRITE_E:
                statisticValuePtr = &statisticsPtr->entriesRewriteTotalRewrite;
                break;
            default:
                continue;/* hold no convert */
        }

        if (statisticValuePtr)
        {
            *statisticValuePtr = fdbManagerPtr->apiEntriesRewriteStatisticsArr[statisticType];
        }
        statisticType++;
    }

    /* 'limit set' statistics */
    statisticType = PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_OK_PORT_LIMIT_E;

    while (statisticType < PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch (statisticType)
        {
            case PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_OK_PORT_LIMIT_E:
                statisticValuePtr = &statisticsPtr->limitSetOkPortLimit;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_OK_TRUNK_E:
                statisticValuePtr = &statisticsPtr->limitSetOkTrunkLimit;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_OK_GLOBAL_EPORT_E:
                statisticValuePtr = &statisticsPtr->limitSetOkGlobalEPortLimit;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_OK_FID_E:
                statisticValuePtr = &statisticsPtr->limitSetOkFidLimit;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_OK_GLOBAL_E:
                statisticValuePtr = &statisticsPtr->limitSetOkGlobalLimit;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_ERROR_INPUT_E:
                statisticValuePtr = &statisticsPtr->limitSetErrorInput;
                break;
            default:
                continue;/* hold no convert */
        }

        if (statisticValuePtr)
        {
            *statisticValuePtr = fdbManagerPtr->apiLimitSetArr[statisticType];
        }
        statisticType++;
    }

}

/*
* @prvCpssDxChBrgFdbManagerDbStatisticsClear function
* @endinternal
*
* @brief The function clears FDB Manager statistics.
*
*
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*/
void prvCpssDxChBrgFdbManagerDbStatisticsClear
(
    IN GT_U32                                           fdbManagerId
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr;

    /* we have valid new manager pointer */
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /* clear add statistics */
    cpssOsMemSet(fdbManagerPtr->apiEntryAddStatisticsArr, 0, sizeof(fdbManagerPtr->apiEntryAddStatisticsArr));
    /* clear delete statistics */
    cpssOsMemSet(fdbManagerPtr->apiEntryDeleteStatisticsArr, 0, sizeof(fdbManagerPtr->apiEntryDeleteStatisticsArr));
    /* clear update statistics */
    cpssOsMemSet(fdbManagerPtr->apiEntryUpdateStatisticsArr, 0, sizeof(fdbManagerPtr->apiEntryUpdateStatisticsArr));
    /* clear learning statistics */
    cpssOsMemSet(fdbManagerPtr->apiLearningScanStatisticsArr, 0, sizeof(fdbManagerPtr->apiLearningScanStatisticsArr));
    /* clear Aging statistics */
    cpssOsMemSet(fdbManagerPtr->apiAgingScanStatisticsArr, 0, sizeof(fdbManagerPtr->apiAgingScanStatisticsArr));
    /* clear Transplant statistics */
    cpssOsMemSet(fdbManagerPtr->apiTransplantScanStatisticsArr, 0, sizeof(fdbManagerPtr->apiTransplantScanStatisticsArr));
    /* clear Delete statistics */
    cpssOsMemSet(fdbManagerPtr->apiDeleteScanStatisticsArr, 0, sizeof(fdbManagerPtr->apiDeleteScanStatisticsArr));
    /* clear rewrite statistics */
    cpssOsMemSet(fdbManagerPtr->apiEntriesRewriteStatisticsArr, 0, sizeof(fdbManagerPtr->apiEntriesRewriteStatisticsArr));
    /* clear limit statistics */
    cpssOsMemSet(fdbManagerPtr->apiLimitSetArr, 0, sizeof(fdbManagerPtr->apiLimitSetArr));

}

/*
* @internal prvCpssDxChFdbManagerDbAgeBinEntryGetNext function
* @endinternal
*
* @brief   The function return the first/next (pointer to) age-bin entry.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[in] getFirst              - indication to get First (GT_TRUE)  or the next (GT_FALSE)
* @param[in] ageBinID              - The age-bin ID to scan for entries.
* @param[inout] lastGetNextInfoPtr - Indicates the last DB node in the ageBin fetched by this agingscan
*                                    OR - DB pointer from where to start the next search in the ageBin.
* @param[out] dbEntryPtrPtr        - (pointer to)  the '(pointer to)first/next age-bin entry'
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_BAD_PARAM             - the iterator is not valid for the 'get next' operation
*                                    the logic found coherency issue.
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChFdbManagerDbAgeBinEntryGetNext
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN GT_BOOL                                          getFirst,
    IN GT_U32                                           ageBinID,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC     *lastGetNextInfoPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   **dbEntryPtrPtr
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr;
    GT_U32                                           dbIndex;

    if(getFirst == GT_FALSE &&
       (!lastGetNextInfoPtr->isValid_entryPointer))
    {
        /* the caller invalidated the 'get next' that was the first entry on previous call */
        getFirst = GT_TRUE;
    }

    if(getFirst == GT_TRUE)
    {
        if (ageBinID >= fdbManagerPtr->agingBinInfo.totalAgeBinAllocated)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        *lastGetNextInfoPtr = fdbManagerPtr->agingBinInfo.ageBinListPtr[ageBinID].headOfAgeBinEntryList;
        if(!lastGetNextInfoPtr->isValid_entryPointer)
        {
            return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
        }
    }

    dbIndex = lastGetNextInfoPtr->entryPointer;

    dbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbIndex];

    if(getFirst != GT_TRUE)
    {
        /* check that we have valid 'next' to jump to it */
        if(!dbEntryPtr->isValid_age_nextEntryPointer)
        {
            /* not updating lastGetNextInfoPtr ! */

            return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
        }

        /* jump to next entry */
        dbIndex = dbEntryPtr->age_nextEntryPointer;
        dbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbIndex];
        if((dbEntryPtr->ageBinIndex != ageBinID) || (!fdbManagerPtr->indexArr[dbEntryPtr->hwIndex].isValid_entryPointer))
        {
           *dbEntryPtrPtr = dbEntryPtr; /* Updating entry info to intimate the caller */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* update the 'getNext' last info */
        lastGetNextInfoPtr->isValid_entryPointer = 1;
        lastGetNextInfoPtr->entryPointer         = dbIndex;
    }

    *dbEntryPtrPtr = dbEntryPtr;

    return GT_OK;
}

/*
* @internal prvCpssDxChBrgFdbManagerDbCheck_counter_dbInfoGet function
* @endinternal
*
* @brief This function gets the counter details from DB..
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[out] typeCountersArr      - Counters bases on entry type
* @param[out] bankCountersArr      - Counters bases on bank number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChBrgFdbManagerDbCheck_counter_dbInfoGet
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC     *fdbManagerPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TYPE_COUNTERS_STC   typeCountersArr[],
    OUT PRV_CPSS_DXCH_FDB_MANAGER_BANK_INFO_STC             bankCountersArr[]
)
{
    GT_STATUS                                           rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC       *dbEntryPtr;
    GT_BOOL                                             isMacMc;

    rc = prvCpssDxChFdbManagerDbApplicationIteratorEntryGetNext(fdbManagerPtr,
            GT_TRUE,
            &fdbManagerPtr->lastGetNextInfo,
            &dbEntryPtr);
    while(rc == GT_OK)
    {
        switch(dbEntryPtr->hwFdbEntryType)
        {
            /* MAC - Static Unicast / Static Multicast / dynamic unicast / dynamic multicast */
            case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E:
                isMacMc  = dbEntryPtr->specificFormat.prvMacEntryFormat.macAddr_high_16 & 0x0100; /* check bit 40 of the mac address */
                if(dbEntryPtr->specificFormat.prvMacEntryFormat.isStatic == 1)
                {
                    if(isMacMc)
                    {
                        typeCountersArr[PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_MULTICAST_STATIC_E].entryTypePopulation++;
                    }
                    else
                    {
                        typeCountersArr[PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_STATIC_E].entryTypePopulation++;
                    }
                }
                else
                {
                    if(isMacMc)
                    {
                        typeCountersArr[PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_MULTICAST_DYNAMIC_E].entryTypePopulation++;
                    }
                    else
                    {
                        typeCountersArr[PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_DYNAMIC_E].entryTypePopulation++;
                    }
                }
                break;

            case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E:
                typeCountersArr[PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV4_MULTICAST_E].entryTypePopulation++;
                break;

            case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E:
                typeCountersArr[PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_MULTICAST_E].entryTypePopulation++;
                break;

            case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E:
                typeCountersArr[PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV4_UNICAST_ROUTE_E].entryTypePopulation++;
                break;

            case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E:
                typeCountersArr[PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_UNICAST_ROUTE_KEY_E].entryTypePopulation++;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E:
                typeCountersArr[PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_UNICAST_ROUTE_DATA_E].entryTypePopulation++;
                break;
        }

        /* Bank counter update */
        bankCountersArr[(dbEntryPtr->hwIndex % fdbManagerPtr->numOfBanks)].bankPopulation++;
        rc = prvCpssDxChFdbManagerDbApplicationIteratorEntryGetNext(fdbManagerPtr,
                GT_FALSE,
                &fdbManagerPtr->lastGetNextInfo,
                &dbEntryPtr);
    }
    if(rc != GT_NO_MORE)
    {
        return rc;
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChBrgFdbManagerDbCheck_global function
* @endinternal
*
* @brief This function verifies FDB manager global parameters.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChBrgFdbManagerDbCheck_global
(
    IN  GT_U32                                           fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT    resultArray[],
    OUT GT_U32                                           *errorNumPtr
)
{
    GT_STATUS                                           rc = GT_OK;
    GT_BOOL                                             firstHashTooLongEn;
    GT_BOOL                                             nextAuMsgRateEn, firstAuMsgRateEn;
    GT_U32                                              nextAuMsgRate, firstAuMsgRate;
    GT_U8                                               devNum;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC     *fdbManagerPtr;

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /**** Verify global default params ****/
    rc = validateNewManagerParams(fdbManagerId,
                                  &fdbManagerPtr->capacityInfo,
                                  &fdbManagerPtr->entryAttrInfo,
                                  &fdbManagerPtr->learningInfo,
                                  &fdbManagerPtr->lookupInfo,
                                  &fdbManagerPtr->agingInfo,
                                  GT_TRUE/* FDB_MANAGER_DB_EXIST */);
    if(rc != GT_OK)
    {
        PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                     CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E,
                                                     errorNumPtr);
        return GT_OK;
    }

    /**** Verify runtime params ****/
    /* Get Hash too long status and AU message rate, from all devices and it should match */
    rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_TRUE, &devNum);
    if(rc != GT_OK)
    {
        if(rc == GT_NO_MORE)
        {
            return GT_OK;
        }
        return rc;
    }
    rc = cpssDxChBrgFdbAuMsgRateLimitGet(devNum, &firstAuMsgRate, &firstAuMsgRateEn);
    while(rc == GT_OK)
    {
        rc = cpssDxChBrgFdbAuMsgRateLimitGet(devNum, &nextAuMsgRate, &nextAuMsgRateEn);
        if(rc != GT_OK)
        {
            return rc;
        }
        if((firstAuMsgRate != nextAuMsgRate) || (firstAuMsgRateEn != nextAuMsgRateEn))
        {
            PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                         CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E,
                                                         errorNumPtr);
            return rc;
        }
        rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_FALSE, &devNum);
    }
    if(rc != GT_NO_MORE)
    {
        return rc;
    }
    else
    {
        rc = GT_OK;
    }

    rc = cpssDxChBrgFdbNaMsgOnChainTooLongGet(devNum, &firstHashTooLongEn);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Verify HW values according to the FDB manager current threshold */
    switch(fdbManagerPtr->currentThresholdType)
    {
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_A_E:
            if (firstAuMsgRateEn != GT_FALSE)
            {
                PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                        CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E,
                        errorNumPtr);
                return rc;
            }
            break;
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_B_E:
            if((firstAuMsgRate >= PRV_FDB_MANAGER_AU_MSG_RATE_STAGE_B) || (firstHashTooLongEn != GT_TRUE))
            {
                PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                        CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E,
                        errorNumPtr);
                return rc;
            }
            break;
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_C_E:
            if(firstHashTooLongEn != GT_FALSE)      /* hashtooLong Disabled, message rate is not valid */
            {
                PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                        CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E,
                        errorNumPtr);
                return rc;
            }
            break;
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_NOT_VALID_E:
            /* No need to verify */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(fdbManagerPtr->currentThresholdType);
    }

    /* Verify the threshold values */
    switch(fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes)
    {
        case 1:
        case 4:
            if((fdbManagerPtr->thresholdB != ((fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes * 40)/100)) ||
               (fdbManagerPtr->thresholdC != ((fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes * 90)/100)))
            {
                PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                             CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E,
                                                             errorNumPtr);
                return rc;
            }
            break;
        case 8:
            if((fdbManagerPtr->thresholdB != ((fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes * 60)/100)) ||
               (fdbManagerPtr->thresholdC != ((fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes * 95)/100)))
            {
                PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                             CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E,
                                                             errorNumPtr);
                return rc;
            }
            break;
        case 16:
            if((fdbManagerPtr->thresholdB != ((fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes * 80)/100)) ||
               (fdbManagerPtr->thresholdC != ((fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes * 95)/100)))
            {
                PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                             CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E,
                                                             errorNumPtr);
                return rc;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(fdbManagerPtr->currentThresholdType);
    }
    return GT_OK;
}


/*
* @internal prvCpssDxChBrgFdbManagerDbCheck_counters function
* @endinternal
*
* @brief The function verifies the FDB manager counters.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChBrgFdbManagerDbCheck_counters
(
    IN  GT_U32                                           fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT    resultArray[],
    OUT GT_U32                                           *errorNumPtr
)
{
    GT_STATUS                                           rc = GT_OK;
    PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_ENT         counterType;
    GT_U32                                              bankNum;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC              counters;
    PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TYPE_COUNTERS_STC   typeCountersArr[PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE__LAST__E];
    PRV_CPSS_DXCH_FDB_MANAGER_BANK_INFO_STC             bankCountersArr[CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_BANKS_CNS];
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC     *fdbManagerPtr;

#ifdef PRV_CPSS_DXCH_BRG_FDB_MANAGER_ALLOW_HW_BANK_COUNTER_UPDATE_CNS
    GT_U32                                              hwPerBankPopulation = 0;
    GT_U8                                               devNum;
    GT_U32                                              numOfTiles;
    GT_U32                                              portGroupId;
    GT_U32                                              tileId;
#endif
    cpssOsMemSet(typeCountersArr, 0, sizeof(typeCountersArr));
    cpssOsMemSet(bankCountersArr, 0, sizeof(bankCountersArr));

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);
    prvCpssDxChBrgFdbManagerDbCountersGet(fdbManagerId, &counters);
    rc = prvCpssDxChBrgFdbManagerDbCheck_counter_dbInfoGet(fdbManagerPtr, typeCountersArr, bankCountersArr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Verify type counters */
    counterType = PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_DYNAMIC_E;
    for(;counterType <= PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_UNICAST_ROUTE_DATA_E; counterType++)
    {
        if(typeCountersArr[counterType].entryTypePopulation != fdbManagerPtr->typeCountersArr[counterType].entryTypePopulation)
        {
            PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E,
                    errorNumPtr);
            CPSS_LOG_ERROR_MAC("mismatch for counterType[%d] : entryTypePopulation[%d] != db.entryTypePopulation[%d]",
                counterType,
                typeCountersArr[counterType].entryTypePopulation,
                fdbManagerPtr->typeCountersArr[counterType].entryTypePopulation);
            break;
        }
    }

    /* Verify Bank counters */
    for(bankNum = 0; bankNum<fdbManagerPtr->numOfBanks; bankNum++)
    {
        if(bankCountersArr[bankNum].bankPopulation != fdbManagerPtr->bankInfoArr[bankNum].bankPopulation)
        {
            PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E,
                    errorNumPtr);
            CPSS_LOG_ERROR_MAC("mismatch for bankNum[%d] : bankPopulation[%d] != db.bankPopulation[%d]",
                bankNum,
                bankCountersArr[bankNum].bankPopulation,
                fdbManagerPtr->bankInfoArr[bankNum].bankPopulation);
            break;
        }
    }

    if(IS_FDB_MANAGER_FOR_SIP_4(fdbManagerPtr))
    {
        /* HW Bank counters are not supported for SIP4 */
        return rc;
    }
#ifdef PRV_CPSS_DXCH_BRG_FDB_MANAGER_ALLOW_HW_BANK_COUNTER_UPDATE_CNS
    else
    {
        /* Verify HW/SW counters */
        for(bankNum = 0; bankNum<fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes; bankNum++)
        {
            rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_TRUE, &devNum);
            while(rc == GT_OK)
            {
                numOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles?PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles : 1;
                /* loop on all the tiles in the device */
                for(tileId = 0; tileId < numOfTiles; tileId++)
                {
                    portGroupId = tileId * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;
                    rc = prvCpssDxChBrgFdbManagerPortGroupBankCounterValueGet(devNum, portGroupId, bankNum, &hwPerBankPopulation);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    if(hwPerBankPopulation != fdbManagerPtr->bankInfoArr[bankNum].bankPopulation)
                    {
                        PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_SW_BANK_COUNTERS_MISMATCH_E,
                                errorNumPtr);
                        CPSS_LOG_ERROR_MAC("mismatch for bankNum[%d] : hwPerBankPopulation[%d] != db.bankPopulation[%d]",
                                bankNum,
                                hwPerBankPopulation,
                                fdbManagerPtr->bankInfoArr[bankNum].bankPopulation);
                        return rc;
                    }
                }
                rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_FALSE, &devNum);
            }
            if(rc != GT_NO_MORE)
            {
                return rc;
            }
        }
    }
    if(rc != GT_NO_MORE)
    {
        return rc;
    }
#endif
    return GT_OK;
}

/*
* @internal prvCpssDxChBrgFdbManagerDbCheck_dbFreeList function
* @endinternal
*
* @brief The function verifies the FDB manager free list in the DB.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChBrgFdbManagerDbCheck_dbFreeList
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC    *fdbManagerPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT    resultArray[],
    OUT GT_U32                                           *errorNumPtr
)
{
    GT_U32                                              dbIndex;
    GT_U32                                              freeCnt = 0;

    /* Check head of free list, Head is expect to be invalid in case of FDB is complete FULL */
    if((!fdbManagerPtr->headOfFreeList.isValid_entryPointer) &&
            (fdbManagerPtr->capacityInfo.maxTotalEntries != fdbManagerPtr->totalPopulation))
    {
        PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E,
                errorNumPtr);
    }

    /* Traverse till end of free list (Head is verified, start with next)*/
    for(dbIndex = 0; dbIndex < fdbManagerPtr->capacityInfo.maxTotalEntries; dbIndex++)
    {
        /* NOTE - nextEntryPointer is used for pointing to the next used entry
         *        Traversing the index array, using "isUsedEntry" to get the free count */
        if(!fdbManagerPtr->entryPoolPtr[dbIndex].isUsedEntry)
        {
            freeCnt += 1;
        }
    }

    /* Match SW counters with Traverse counter */
    if(freeCnt != (fdbManagerPtr->capacityInfo.maxTotalEntries - fdbManagerPtr->totalPopulation))
    {
        PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E,
                errorNumPtr);
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChBrgFdbManagerDbCheck_dbUsedList function
* @endinternal
*
* @brief The function verifies the FDB manager used list in the DB.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChBrgFdbManagerDbCheck_dbUsedList
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC    *fdbManagerPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT    resultArray[],
    OUT GT_U32                                           *errorNumPtr
)
{

    GT_U32                                              dbIndex;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC      *dbEntryPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INDEX_STC             *dbIndexPtr;
    GT_U32                                              usedCnt = 0;
    GT_BOOL                                             getNextErrorDetected = GT_FALSE, getPrevErrorDetected = GT_FALSE;

    /* Check head of used list:(Below Expected)
     *      entryPointer        == 1
     *      prevEntryPointer    == 0
     */
    if(!fdbManagerPtr->headOfUsedList.isValid_entryPointer)
    {
        /* Head of list not valid, skip the list check
         * Scenario - FDB Manager created with at least one device but no entry populated */
        return GT_OK;
    }
    else
    {
        /* Initialize the iterator with HEAD node, updated counter by 1*/
        dbIndex     = fdbManagerPtr->headOfUsedList.entryPointer;
        dbEntryPtr  = &fdbManagerPtr->entryPoolPtr[dbIndex];
        if(!dbEntryPtr->isUsedEntry)
        {
            PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E,
                    errorNumPtr);
            /* As used list Head is corrupted, should not check for other error */
            return GT_OK;
        }
        /* Head node should not have any previous valid node */
        if(dbEntryPtr->isValid_prevEntryPointer)
        {
            PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E,
                    errorNumPtr);
            /* As used list Head is corrupted, should not check for other error */
            return GT_OK;
        }
        usedCnt += 1;
    }

    /* Traverse till end of Used list (Head is verified, start with next)*/
    while(dbEntryPtr->isValid_nextEntryPointer)
    {
        /* jump to next entry */
        /*dbIndex = dbEntryPtr->nextEntryPointer;*/
        GET_nextEntryPointer_MAC(dbEntryPtr, dbIndex);

        dbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbIndex];
        dbIndexPtr = &fdbManagerPtr->indexArr[dbEntryPtr->hwIndex];

        /* Check elements used list:(Below Expected)
         *      isValid_entryPointer        == 1
         *      isValid_prevEntryPointer    == 1
         */
        if(!dbIndexPtr->isValid_entryPointer && !getNextErrorDetected)
        {
            PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E,
                errorNumPtr);
            if(getPrevErrorDetected)
            {
                break;
            }
            getNextErrorDetected = GT_TRUE;
        }
        if(!dbEntryPtr->isValid_prevEntryPointer && !getPrevErrorDetected)
        {
            PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E,
                    errorNumPtr);
            if(getNextErrorDetected)
            {
                break;
            }
            getPrevErrorDetected = GT_TRUE;
        }
        usedCnt += 1;
    }

    /* Match SW counters with Traverse counter */
    if(usedCnt != fdbManagerPtr->totalPopulation)
    {
        PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E,
                errorNumPtr);
    }

    if(fdbManagerPtr->lastGetNextInfo.isValid_entryPointer)
    {
        dbIndex    = fdbManagerPtr->lastGetNextInfo.entryPointer;
        dbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbIndex];
        if(!dbEntryPtr->isUsedEntry)
        {
            PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E,
                errorNumPtr);
        }
    }

    if(fdbManagerPtr->lastTranslplantInfo.isValid_entryPointer)
    {
        dbIndex    = fdbManagerPtr->lastTranslplantInfo.entryPointer;
        dbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbIndex];
        if(!dbEntryPtr->isUsedEntry)
        {
            PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_TRANS_SCAN_PTR_INVALID_E,
                    errorNumPtr);
        }
    }
    if(fdbManagerPtr->lastDeleteInfo.isValid_entryPointer)
    {
        dbIndex    = fdbManagerPtr->lastDeleteInfo.entryPointer;
        dbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbIndex];
        if(!dbEntryPtr->isUsedEntry)
        {
            PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E,
                    errorNumPtr);
        }
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChBrgFdbManagerDbCheck_indexPointer function
* @endinternal
*
* @brief The function verifies the FDB manager index pointers.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChBrgFdbManagerDbCheck_indexPointer
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC    *fdbManagerPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT    resultArray[],
    OUT GT_U32                                           *errorNumPtr
)
{

    GT_STATUS       rc = GT_OK;
    GT_U32          hwIndex;
    GT_U32          dbIndex;
    GT_BOOL         indexPointInvalidEn = GT_FALSE, indexInvalidEn = GT_FALSE;

    for(hwIndex = 0 ; hwIndex < fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes; hwIndex++)
    {
        dbIndex = fdbManagerPtr->indexArr[hwIndex].entryPointer;

        /* protect access to array out of range */
        CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex,fdbManagerPtr->capacityInfo.maxTotalEntries);

        /* Valid Index should point to not NULL entry,
         * Invalid Index should point to NULL Entry
         */
        if(fdbManagerPtr->indexArr[hwIndex].isValid_entryPointer)
        {
            /* Valid entry but isUsedEntry flag not set. (Add to result array  once) */
            if(!fdbManagerPtr->entryPoolPtr[dbIndex].isUsedEntry && !indexPointInvalidEn)
            {
                PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                        CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E,
                        errorNumPtr);
                if(indexInvalidEn)
                {
                    break;
                }
                indexPointInvalidEn = GT_TRUE;
            }
            /* Valid entry but hwIndex is not matching. (Add to result array  once) */
            if((fdbManagerPtr->entryPoolPtr[dbIndex].hwIndex != hwIndex) && !indexInvalidEn)
            {
                PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                        CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E,
                        errorNumPtr);
                CPSS_LOG_ERROR_MAC("mismatch for dbIndex[%d] : fdbManagerPtr->entryPoolPtr[dbIndex].hwIndex[%d] != hwIndex[%d]",
                    dbIndex,
                    fdbManagerPtr->entryPoolPtr[dbIndex].hwIndex,
                    hwIndex);
                if(indexPointInvalidEn)
                {
                    break;
                }
                indexInvalidEn = GT_TRUE;
            }
        }
        /* else - In case of invalid entry dbIndex will not be valid to check */
    }
    return rc;
}

/*
* @internal prvCpssDxChBrgFdbManagerDbCheck_agingBin function
* @endinternal
*
* @brief The function verifies the FDB manager aging bin.
*
* @param[in]  fdbManagerPtr        - (pointer to) the FDB manager
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChBrgFdbManagerDbCheck_agingBin
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC    *fdbManagerPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT    resultArray[],
    OUT GT_U32                                           *errorNumPtr
)
{
    GT_STATUS                                           rc = GT_OK;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC       *dbEntryPtr;
    GT_U32                                              curAgeBinId;
    GT_U32                                              curAgeBinCnt = 0;
    GT_U32                                              totalAgeBinCnt = 0;
    GT_BOOL                                             scanPtrInvalid = GT_FALSE;
    GT_BOOL                                             entryNumMismatch = GT_FALSE;
    GT_BOOL                                             ageBinIndexInvalid = GT_FALSE;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC           lastGetNextInfo;

    for(curAgeBinId = 0; curAgeBinId<fdbManagerPtr->agingBinInfo.totalAgeBinAllocated; curAgeBinId++)
    {
        rc = prvCpssDxChFdbManagerDbAgeBinEntryGetNext(fdbManagerPtr,
                GT_TRUE,
                curAgeBinId,
                &lastGetNextInfo,
                &dbEntryPtr);
        while(rc == GT_OK)
        {
            curAgeBinCnt += 1;
            rc = prvCpssDxChFdbManagerDbAgeBinEntryGetNext(fdbManagerPtr,
                    GT_FALSE,
                    curAgeBinId,
                    &lastGetNextInfo,
                    &dbEntryPtr);
        }

        /* Age-bin Get next failed */
        if(rc != GT_NO_MORE)
        {
            if(dbEntryPtr->ageBinIndex != curAgeBinId)
            {
                ageBinIndexInvalid = GT_TRUE;
            }
            scanPtrInvalid = GT_TRUE;
        }

        /* Validate age-bin's entry count */
        if(curAgeBinCnt != fdbManagerPtr->agingBinInfo.ageBinListPtr[curAgeBinId].totalUsedEntriesCnt)
        {
            entryNumMismatch = GT_TRUE;
        }
        totalAgeBinCnt += curAgeBinCnt;
        curAgeBinCnt = 0;
        if(ageBinIndexInvalid && scanPtrInvalid && entryNumMismatch)
        {
            break;
        }
    }

    /* Set the error list */
    if(entryNumMismatch)
    {
        PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E,
                errorNumPtr);
    }
    if(totalAgeBinCnt != fdbManagerPtr->totalPopulation)
    {
        PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E,
                errorNumPtr);
        CPSS_LOG_ERROR_MAC("mismatch for totalAgeBinCnt[%d] != totalPopulation[%d]",
            totalAgeBinCnt,
            fdbManagerPtr->totalPopulation);
    }
    if(ageBinIndexInvalid)
    {
        PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E,
                errorNumPtr);
    }
    if(scanPtrInvalid)
    {
        PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E,
                errorNumPtr);
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChBrgFdbManagerDbCheck_ageBinUsageMatrix function
* @endinternal
*
* @brief The function verifies the FDB manager aging bin usage matrix.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChBrgFdbManagerDbCheck_ageBinUsageMatrix
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT    resultArray[],
    OUT GT_U32                                           *errorNumPtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  curAgeBinId;
    GT_BOOL                                 l2BitEnabled = GT_TRUE;
    GT_BOOL                                 l1ErrorEn = GT_FALSE, l2ErrorEn = GT_FALSE;

    for(curAgeBinId = 0; curAgeBinId<fdbManagerPtr->agingBinInfo.totalAgeBinAllocated; curAgeBinId++)
    {
        /* Level-1 Verification */
        if(fdbManagerPtr->agingBinInfo.ageBinListPtr[curAgeBinId].totalUsedEntriesCnt != fdbManagerPtr->capacityInfo.maxEntriesPerAgingScan)
        {
            /* Level-1 usage bit should not be set */
            if((IS_BMP_SET_MAC(fdbManagerPtr->agingBinInfo.level1UsageBitmapPtr, curAgeBinId)) && !l1ErrorEn)
            {
                PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                        CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E,
                        errorNumPtr);
                if(l2ErrorEn)
                {
                    break;
                }
                l1ErrorEn = GT_TRUE;
            }
            l2BitEnabled = GT_FALSE;
        }
        else
        {
            /* Level-1 usage bit should be set */
            if((!IS_BMP_SET_MAC(fdbManagerPtr->agingBinInfo.level1UsageBitmapPtr, curAgeBinId)) && !l1ErrorEn)
            {
                PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                        CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E,
                        errorNumPtr);
                if(l2ErrorEn)
                {
                    break;
                }
                l1ErrorEn = GT_TRUE;
            }
        }

        /* Level-2 Verification */
        if((curAgeBinId != 0) && (((curAgeBinId+1) % 32) == 0))
        {
            if(l2BitEnabled)
            {
                if((!IS_BMP_SET_MAC(&fdbManagerPtr->agingBinInfo.level2UsageBitmap, curAgeBinId>>5)) && !l2ErrorEn)
                {
                    PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                            CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E,
                            errorNumPtr);
                    if(l1ErrorEn)
                    {
                        break;
                    }
                    l2ErrorEn = GT_TRUE;
                }
            }
            else
            {
                if((IS_BMP_SET_MAC(&fdbManagerPtr->agingBinInfo.level2UsageBitmap, curAgeBinId>>5)) && !l2ErrorEn)
                {
                    PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                            CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E,
                            errorNumPtr);
                    if(l1ErrorEn)
                    {
                        break;
                    }
                    l2ErrorEn = GT_TRUE;
                }
            }
            /* Reset L2 Bit, to check next 32 age-bin */
            l2BitEnabled = GT_TRUE;
        }
    }
    return rc;
}

/*
* @internal prvCpssDxChBrgFdbManagerDbCheck_hwList function
* @endinternal
*
* @brief The function verifies the FDB manager used/Free list in HW.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[in] usedListEnable        - Used list validation state.
*                                    GT_TRUE  - Used list validation required.
*                                    GT_FALSE - Used list validation not required.
* @param[in] freeListEnable        - Free list validate state
*                                    GT_TRUE  - Free list validation required.
*                                    GT_FALSE - Free list validation not required.
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - there is error in DB management.
*
*/
static GT_STATUS prvCpssDxChBrgFdbManagerDbCheck_hwList
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN  GT_BOOL                                           usedListEnable,
    IN  GT_BOOL                                           freeListEnable,
    IN  GT_BOOL                                           spIncluded,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT    resultArray[],
    OUT GT_U32                                           *errorNumPtr
)
{
    GT_STATUS                                               rc = GT_OK;
    GT_U32                                                  hwIndex;
    GT_U32                                                  dbIndex;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_CHECK_LIST_TYPE_ENT        listType;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          result;
    GT_BOOL                                                 freeListErrorEn = GT_FALSE;
    GT_BOOL                                                 freeListSpErrorEn = GT_FALSE;
    GT_BOOL                                                 usedListErrorEn = GT_FALSE;

    listType = (spIncluded)?PRV_CPSS_DXCH_FDB_MANAGER_DB_CHECK_FREE_LIST_SP_AS_USED_E:PRV_CPSS_DXCH_FDB_MANAGER_DB_CHECK_FREE_LIST_SP_AS_FREE_E;
    for(hwIndex = 0 ; hwIndex < fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes; hwIndex++)
    {
        result = CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E;
        /* Validate used list content */
        if(fdbManagerPtr->indexArr[hwIndex].isValid_entryPointer && usedListEnable && !usedListErrorEn)
        {
            dbIndex = fdbManagerPtr->indexArr[hwIndex].entryPointer;
            if(fdbManagerPtr->entryPoolPtr[dbIndex].hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E)
            {
                continue;   /* Already verified */
            }

            /* protect access to array out of range */
            CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, fdbManagerPtr->capacityInfo.maxTotalEntries);

            rc = prvCpssDxChBrgFdbManagerHwEntryMatchWithSwEntry_allDevice(fdbManagerPtr,
                    hwIndex,
                    &fdbManagerPtr->entryPoolPtr[dbIndex],
                    PRV_CPSS_DXCH_FDB_MANAGER_DB_CHECK_USED_LIST_E,
                    &result);
            if(rc != GT_OK)
            {
                return rc;
            }
            if(result == CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E)
            {
                usedListErrorEn = GT_TRUE;
            }
        }
        /* Validate free list entry in all device */
        else if((!fdbManagerPtr->indexArr[hwIndex].isValid_entryPointer) && freeListEnable && (!freeListErrorEn || !freeListSpErrorEn))
        {
            rc = prvCpssDxChBrgFdbManagerHwEntryMatchWithSwEntry_allDevice(fdbManagerPtr,
                    hwIndex,
                    NULL,   /* No need to verify content */
                    listType,
                    &result);
            if(rc != GT_OK)
            {
                return rc;
            }
            if(result == CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E)
            {
                freeListErrorEn = GT_TRUE;
            }
            else if(result == CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E)
            {
                freeListSpErrorEn = GT_TRUE;
            }
        }
        if(usedListErrorEn && freeListErrorEn && freeListSpErrorEn)
        {
            break;
        }
    }
    if(rc == GT_NO_MORE)
    {
        rc = GT_OK;
    }

    if(usedListErrorEn)
    {
        PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E,
                errorNumPtr);
    }
    if(freeListErrorEn)
    {
        PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E,
                errorNumPtr);
    }
    if(freeListSpErrorEn)
    {
        PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E,
                errorNumPtr);
    }
    return rc;
}

/*
* @internal prvCpssDxChBrgFdbManagerDbCheck_learnLimit function
* @endinternal
*
* @brief The function verifies the FDB manager DB - for the 'learn limit' DB counters.
*
* @param[in]  fdbManagerId         - The FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in]  checksPtr            - (Pointer to) DB check attributes
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChBrgFdbManagerDbCheck_learnLimit
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC          *checksPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT    resultArray[],
    OUT GT_U32                                           *errorNumPtr
)
{
    GT_U32  fidTotalCounter;
    GT_U32  ii,jj;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_STC *currLimitPtr;
    GT_U32  globalEportTotalCounter;
    GT_U32  portTotalCounter;
    GT_U32  trunkTotalCounter;

    if(NULL == fdbManagerPtr->dynamicUcMacLimitPtr)
    {
        /* the limit feature was not enabled , so no error can be related to it */
        return GT_OK;
    }

    if(checksPtr->learnLimitGlobalCheckEnable == GT_TRUE)
    {
        if(fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.actualCounter !=
           fdbManagerPtr->typeCountersArr[PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_DYNAMIC_E].entryTypePopulation)
        {
            /* error log without exit the function */
            CPSS_LOG_ERROR_MAC("dynamicUcMacLimit_global.actualCounter[%ld] != typeCountersArr[MAC_UNICAST_DYNAMIC_E].entryTypePopulation [%ld]",
                fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.actualCounter,
                fdbManagerPtr->typeCountersArr[PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_DYNAMIC_E].entryTypePopulation);

            PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_GLOBAL_INVALID_E,
                    errorNumPtr);
        }
    }

    /* check if need to sum counters from all FIDs */
    if(checksPtr->learnLimitFidCheckEnable == GT_TRUE)
    {
        fidTotalCounter = 0;
        currLimitPtr = &fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_fid[0];
        for(ii = 0 ; ii < PRV_CPSS_DXCH_FDB_MANAGER_DB_MAX_FID_CNS; ii++,currLimitPtr++)
        {
            fidTotalCounter += currLimitPtr->actualCounter;
        }

        if(fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.actualCounter !=
            fidTotalCounter)
        {
            /* error log without exit the function */
            CPSS_LOG_ERROR_MAC("dynamicUcMacLimit_global.actualCounter[%ld] != fidTotalCounter [%ld]",
                fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.actualCounter,
                fidTotalCounter);

            PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_FID_INVALID_E,
                    errorNumPtr);
        }
    }

    globalEportTotalCounter = 0;
    portTotalCounter        = 0;
    trunkTotalCounter       = 0;

    /* check if need to sum from all global-eports */
    if((checksPtr->learnLimitGlobalEportCheckEnable   == GT_TRUE ||
        checksPtr->learnLimitSrcInterfacesCheckEnable == GT_TRUE) &&
       fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEportPtr)
    {
        currLimitPtr = &fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEportPtr[0];
        for(ii = 0 ; ii < fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEport_num; ii++,currLimitPtr++)
        {
            globalEportTotalCounter += currLimitPtr->actualCounter;
        }

        if(checksPtr->learnLimitGlobalEportCheckEnable   == GT_TRUE)
        {
            if(fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.actualCounter <
                globalEportTotalCounter)
            {
                /* error log without exit the function */
                CPSS_LOG_ERROR_MAC("dynamicUcMacLimit_global.actualCounter[%ld] < globalEportTotalCounter [%ld]",
                    fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.actualCounter,
                    globalEportTotalCounter);

                PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                        CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_GLOBAL_EPORT_INVALID_E,
                        errorNumPtr);
            }
        }
    }

    /* check if need to sum from all trunks */
    if(checksPtr->learnLimitTrunkCheckEnable         == GT_TRUE ||
       checksPtr->learnLimitSrcInterfacesCheckEnable == GT_TRUE)
    {
        currLimitPtr = &fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_trunkId[0];
        for(ii = 0 ; ii < PRV_CPSS_DXCH_FDB_MANAGER_DB_MAX_TRUNK_ID_CNS; ii++,currLimitPtr++)
        {
            trunkTotalCounter += currLimitPtr->actualCounter;
        }

        if(checksPtr->learnLimitTrunkCheckEnable   == GT_TRUE)
        {
            if(fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.actualCounter <
                trunkTotalCounter)
            {
                /* error log without exit the function */
                CPSS_LOG_ERROR_MAC("dynamicUcMacLimit_global.actualCounter[%ld] < trunkTotalCounter [%ld]",
                    fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.actualCounter,
                    trunkTotalCounter);

                PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                        CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_TRUNK_INVALID_E,
                        errorNumPtr);
            }
        }
    }

    /* check if need to sum from all ports from all hwDevNum */
    if(checksPtr->learnLimitPortCheckEnable          == GT_TRUE ||
       checksPtr->learnLimitSrcInterfacesCheckEnable == GT_TRUE)
    {
        for(jj = 0 ; jj < PRV_CPSS_DXCH_FDB_MANAGER_DB_MAX_HW_DEV_NUM_CNS; jj++)
        {
            if(NULL == fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[jj])
            {
                continue;
            }

            currLimitPtr = &fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[jj]->perEportPtr[0];
            for(ii = 0 ; ii < fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[jj]->numPorts; ii++,currLimitPtr++)
            {
                portTotalCounter += currLimitPtr->actualCounter;
            }
        }

        if(checksPtr->learnLimitPortCheckEnable   == GT_TRUE)
        {
            if(fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.actualCounter <
                portTotalCounter)
            {
                /* error log without exit the function */
                CPSS_LOG_ERROR_MAC("dynamicUcMacLimit_global.actualCounter[%ld] < portTotalCounter [%ld]",
                    fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.actualCounter,
                    portTotalCounter);

                PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                        CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_PORT_INVALID_E,
                        errorNumPtr);
            }
        }
    }

    if(checksPtr->learnLimitSrcInterfacesCheckEnable == GT_TRUE)
    {
        if(fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.actualCounter <
            (globalEportTotalCounter + portTotalCounter + trunkTotalCounter))
        {
                /* error log without exit the function */
                CPSS_LOG_ERROR_MAC("dynamicUcMacLimit_global.actualCounter[%ld] < "
                    "'total' [%ld] of (globalEportTotalCounter [%ld] + portTotalCounter[%ld] + trunkTotalCounter[%ld])",
                    fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.actualCounter,
                    (globalEportTotalCounter+portTotalCounter+trunkTotalCounter),
                    globalEportTotalCounter,
                    portTotalCounter,
                    trunkTotalCounter);

                PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                        CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_SRC_INTERFACES_CURRENT_INVALID_E,
                        errorNumPtr);
        }
    }

    return GT_OK;
}


/*
* @internal prvCpssDxChBrgFdbManagerDbCheck function
* @endinternal
*
* @brief The function verifies the FDB manager DB.
*
* @param[in]  fdbManagerId         - The FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in]  checksPtr            - (Pointer to) DB check attributes
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS prvCpssDxChBrgFdbManagerDbCheck
(
    IN  GT_U32                                            fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC          *checksPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT    resultArray[],
    OUT GT_U32                                           *errorNumPtr
)
{
    GT_STATUS                                          rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC     *fdbManagerPtr;

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    *errorNumPtr = 0;
    if(checksPtr->globalCheckEnable)                /* globalCheckEnable */
    {
        rc = prvCpssDxChBrgFdbManagerDbCheck_global(fdbManagerId, resultArray, errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(checksPtr->countersCheckEnable)              /* countersCheckEnable */
    {
        rc = prvCpssDxChBrgFdbManagerDbCheck_counters(fdbManagerId, resultArray, errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(checksPtr->dbFreeListCheckEnable)            /* dbFreeListCheckEnable */
    {
        rc = prvCpssDxChBrgFdbManagerDbCheck_dbFreeList(fdbManagerPtr, resultArray, errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(checksPtr->dbUsedListCheckEnable)            /* dbUsedListCheckEnable */
    {
        rc = prvCpssDxChBrgFdbManagerDbCheck_dbUsedList(fdbManagerPtr, resultArray, errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(checksPtr->dbIndexPointerCheckEnable)        /* dbIndexPointerCheckEnable */
    {
        rc = prvCpssDxChBrgFdbManagerDbCheck_indexPointer(fdbManagerPtr, resultArray, errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(checksPtr->dbAgingBinCheckEnable)            /* dbAgingBinCheckEnable */
    {
        rc = prvCpssDxChBrgFdbManagerDbCheck_agingBin(fdbManagerPtr, resultArray, errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(checksPtr->dbAgingBinUsageMatrixCheckEnable) /* dbAgingBinUsageMatrixCheckEnable */
    {
        rc = prvCpssDxChBrgFdbManagerDbCheck_ageBinUsageMatrix(fdbManagerPtr, resultArray, errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(checksPtr->hwUsedListCheckEnable || checksPtr->hwFreeListCheckEnable)
    {
        rc = prvCpssDxChBrgFdbManagerDbCheck_hwList(fdbManagerPtr,
                                             checksPtr->hwUsedListCheckEnable,
                                             checksPtr->hwFreeListCheckEnable,
                                             checksPtr->hwFreeListWithSpCheckEnable,
                                             resultArray,
                                             errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(GT_TRUE == checksPtr->learnLimitGlobalCheckEnable           ||
       GT_TRUE == checksPtr->learnLimitFidCheckEnable              ||
       GT_TRUE == checksPtr->learnLimitGlobalEportCheckEnable      ||
       GT_TRUE == checksPtr->learnLimitTrunkCheckEnable            ||
       GT_TRUE == checksPtr->learnLimitPortCheckEnable             ||
       GT_TRUE == checksPtr->learnLimitSrcInterfacesCheckEnable)
    {
        /* need to do 'limit learn' checks  */
        rc = prvCpssDxChBrgFdbManagerDbCheck_learnLimit(fdbManagerPtr,checksPtr,resultArray,errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/*
* @internal prvCpssDxChFdbManagerDebugSelfTest function
* @endinternal
*
* @brief The function injects/corrects error in FDB manager DB, as mentioned in expResultArray
*
* @param[in] fdbManagerId         - The FDB Manager id.
*                                   (APPLICABLE RANGES : 0..31)
* @param[in] expResultArray       - (pointer to) Test error types array.
* @param[in] errorNumPtr          - (pointer to) Number of error types filled in expResultArray
* @param[in] errorEnable          - Status value indicates error/correct state
*                                   GT_TRUE  - Error need to injected according to expResultArray and expErrorNum
*                                   GT_FALSE - Error need to be corrected according to expResultArray and expErrorNum
*
* @retval GT_OK                   - on success
* @retval GT_FAIL                 - on error
*
*/
GT_STATUS prvCpssDxChFdbManagerDebugSelfTest
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT   expResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E],
    IN GT_U32                                           expErrorNum,
    IN GT_BOOL                                          errorEnable
)
{
    GT_STATUS                                           rc = GT_OK;
    GT_U32                                              ii, tmpCnt;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC     *fdbManagerPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_RESTORE_INFO_UNT   *restoreInfoPtr;
    GT_U32                                              bankId = 0;
    PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_ENT         counterType = PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_DYNAMIC_E;
    GT_U32                                              dbIndex;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC      *dbEntryPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC          *dbNodePtr;  /* For transplant and delete scan */

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);


    for(ii=0; ii<expErrorNum; ii++)
    {
        /* Validate if error is already injected/corrected */
        if((errorEnable && IS_BMP_SET_MAC(&fdbManagerPtr->selfTestBitmap, expResultArray[ii])) ||
           (!errorEnable && !IS_BMP_SET_MAC(&fdbManagerPtr->selfTestBitmap, expResultArray[ii])))
        {
            continue;
        }
        restoreInfoPtr = &fdbManagerPtr->selfTestRestoreInfo[expResultArray[ii]];
        switch(expResultArray[ii])
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E:
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->counter = fdbManagerPtr->capacityInfo.maxTotalEntries;
                    fdbManagerPtr->capacityInfo.maxTotalEntries -=1;
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    fdbManagerPtr->capacityInfo.maxTotalEntries = restoreInfoPtr->counter;
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E);
                }
                break;

            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E:
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->counter = fdbManagerPtr->typeCountersArr[counterType].entryTypePopulation;
                    fdbManagerPtr->typeCountersArr[counterType].entryTypePopulation +=1;
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    fdbManagerPtr->typeCountersArr[counterType].entryTypePopulation = restoreInfoPtr->counter;
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E);
                }
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E:
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_SW_BANK_COUNTERS_MISMATCH_E:
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->counter = fdbManagerPtr->bankInfoArr[bankId].bankPopulation;
                    fdbManagerPtr->bankInfoArr[bankId].bankPopulation += 1;
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E);
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_SW_BANK_COUNTERS_MISMATCH_E);
                }
                else
                {/* Correct last injected error*/
                    fdbManagerPtr->bankInfoArr[bankId].bankPopulation = restoreInfoPtr->counter;
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E);
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_SW_BANK_COUNTERS_MISMATCH_E);
                }
                break;

            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E:
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->entryStatus = BIT2BOOL_MAC(fdbManagerPtr->headOfFreeList.isValid_entryPointer);
                    fdbManagerPtr->headOfFreeList.isValid_entryPointer = 0;
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    fdbManagerPtr->headOfFreeList.isValid_entryPointer = BOOL2BIT_MAC(restoreInfoPtr->entryStatus);
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E);
                }
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E:
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E:
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E:
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->counter = fdbManagerPtr->totalPopulation;
                    fdbManagerPtr->totalPopulation += 1;
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E);
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E);
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E);
                }
                else
                {/* Correct last injected error*/
                    fdbManagerPtr->totalPopulation = restoreInfoPtr->counter;
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E);
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E);
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E);
                }
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E:
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E:
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E:
                dbIndex     = fdbManagerPtr->headOfUsedList.entryPointer;
                dbEntryPtr  = &fdbManagerPtr->entryPoolPtr[dbIndex];
                GET_nextEntryPointer_MAC(dbEntryPtr, dbIndex);              /* Invalidate the entry next to head */
                dbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbIndex];
                if(errorEnable)
                {/* Inject Error */
                    if(!fdbManagerPtr->indexArr[dbEntryPtr->hwIndex].isValid_entryPointer)
                    {
                        rc = GT_FAIL;
                        break;
                    }
                    restoreInfoPtr->entryStatus = BIT2BOOL_MAC(fdbManagerPtr->indexArr[dbEntryPtr->hwIndex].isValid_entryPointer);
                    fdbManagerPtr->indexArr[dbEntryPtr->hwIndex].isValid_entryPointer = 0;
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E);
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E);
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E);
                }
                else
                {/* Correct last injected error*/
                    fdbManagerPtr->indexArr[dbEntryPtr->hwIndex].isValid_entryPointer = BOOL2BIT_MAC(restoreInfoPtr->entryStatus);
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E);
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E);
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E);
                }
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_TRANS_SCAN_PTR_INVALID_E:
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E:
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E:
                if(expResultArray[ii] == CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_TRANS_SCAN_PTR_INVALID_E)
                {
                    dbNodePtr = &fdbManagerPtr->lastTranslplantInfo;
                }
                else if(expResultArray[ii] == CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E)
                {
                    dbNodePtr = &fdbManagerPtr->lastDeleteInfo;
                }
                else
                {
                    dbNodePtr = &fdbManagerPtr->lastGetNextInfo;
                }

                if(errorEnable)
                {/* Inject Error */
                    /* In case of scan not used at all - assign last scan as head of free list */
                    if(!dbNodePtr->isValid_entryPointer)
                    {
                        dbNodePtr->entryPointer          = fdbManagerPtr->headOfFreeList.entryPointer;
                    }
                    restoreInfoPtr->nodeInfo.entryPointer = dbNodePtr->entryPointer;
                    dbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbNodePtr->entryPointer];

                    restoreInfoPtr->nodeInfo.isValid_entryPointer   = BIT2BOOL_MAC(dbNodePtr->isValid_entryPointer);
                    dbNodePtr->isValid_entryPointer                 = 1;
                    restoreInfoPtr->nodeInfo.isUsedEntry            = BIT2BOOL_MAC(dbEntryPtr->isUsedEntry);
                    dbEntryPtr->isUsedEntry                         = 0;
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_TRANS_SCAN_PTR_INVALID_E);
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E);
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    dbEntryPtr = &fdbManagerPtr->entryPoolPtr[restoreInfoPtr->nodeInfo.entryPointer];
                    dbEntryPtr->isUsedEntry                 = BOOL2BIT_MAC(restoreInfoPtr->nodeInfo.isUsedEntry);
                    dbNodePtr->isValid_entryPointer         = BOOL2BIT_MAC(restoreInfoPtr->nodeInfo.isValid_entryPointer);
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_TRANS_SCAN_PTR_INVALID_E);
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E);
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E);
                }
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E:
                dbIndex     = fdbManagerPtr->headOfUsedList.entryPointer;
                dbEntryPtr  = &fdbManagerPtr->entryPoolPtr[dbIndex];
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->entryStatus = BIT2BOOL_MAC(dbEntryPtr->isValid_prevEntryPointer);
                    dbEntryPtr->isValid_prevEntryPointer = 1;
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    dbEntryPtr->isValid_prevEntryPointer = BOOL2BIT_MAC(restoreInfoPtr->entryStatus);
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E);
                }
                break;

            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E:
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E:
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E:
                dbIndex     = fdbManagerPtr->headOfUsedList.entryPointer;
                dbEntryPtr  = &fdbManagerPtr->entryPoolPtr[dbIndex];
                if(errorEnable)
                {/* Inject Error */
                    if(!fdbManagerPtr->indexArr[dbEntryPtr->hwIndex].isValid_entryPointer)
                    {
                        rc = GT_FAIL;
                        break;
                    }
                    if(expResultArray[ii] == CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E)
                    {
                        restoreInfoPtr->hwIndex = fdbManagerPtr->entryPoolPtr[dbIndex].hwIndex;
                        fdbManagerPtr->entryPoolPtr[dbIndex].hwIndex +=1;
                        SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E);
                    }
                    else
                    {
                        restoreInfoPtr->entryStatus = BIT2BOOL_MAC(fdbManagerPtr->entryPoolPtr[dbIndex].isUsedEntry);
                        fdbManagerPtr->entryPoolPtr[dbIndex].isUsedEntry = 0;
                        SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E);
                        SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E);
                    }
                }
                else
                {/* Correct last injected error*/
                    if(expResultArray[ii] == CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E)
                    {
                        fdbManagerPtr->entryPoolPtr[dbIndex].hwIndex = restoreInfoPtr->hwIndex;
                        SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E);
                    }
                    else
                    {
                        fdbManagerPtr->entryPoolPtr[dbIndex].isUsedEntry = BOOL2BIT_MAC(restoreInfoPtr->entryStatus);
                        SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E);
                        SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E);
                    }
                }
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E:
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E:
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->counter = fdbManagerPtr->agingBinInfo.ageBinListPtr[0].totalUsedEntriesCnt;
                    if(fdbManagerPtr->agingBinInfo.ageBinListPtr[0].totalUsedEntriesCnt < fdbManagerPtr->capacityInfo.maxEntriesPerAgingScan)
                    {
                        fdbManagerPtr->agingBinInfo.ageBinListPtr[0].totalUsedEntriesCnt = fdbManagerPtr->capacityInfo.maxEntriesPerAgingScan;
                    }
                    else
                    {
                        fdbManagerPtr->agingBinInfo.ageBinListPtr[0].totalUsedEntriesCnt -=5;
                    }
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E);
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    fdbManagerPtr->agingBinInfo.ageBinListPtr[0].totalUsedEntriesCnt = restoreInfoPtr->counter;
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E);
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E);
                }
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E:
                dbIndex     = fdbManagerPtr->headOfUsedList.entryPointer;
                dbEntryPtr  = &fdbManagerPtr->entryPoolPtr[dbIndex];
                for(tmpCnt = 0; fdbManagerPtr->capacityInfo.maxTotalEntries; tmpCnt++)
                {
                    dbEntryPtr  = &fdbManagerPtr->entryPoolPtr[dbIndex];
                    GET_nextEntryPointer_MAC(dbEntryPtr, dbIndex);
                    /*  Get the 3rd Element */
                    if(tmpCnt == 3)
                    {
                        break;
                    }
                }
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->ageBinIndex = dbEntryPtr->ageBinIndex;
                    dbEntryPtr->ageBinIndex = (dbEntryPtr->ageBinIndex+1)%(fdbManagerPtr->agingBinInfo.totalAgeBinAllocated-1);
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    dbEntryPtr->ageBinIndex = restoreInfoPtr->ageBinIndex;
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E);
                }
                break;

            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E:
                tmpCnt = 1;
                if(errorEnable)
                {/* Inject Error */
                    if(!IS_BMP_SET_MAC(&fdbManagerPtr->agingBinInfo.level2UsageBitmap, tmpCnt))
                    {
                        BMP_SET_MAC(&fdbManagerPtr->agingBinInfo.level2UsageBitmap, tmpCnt);
                        restoreInfoPtr->l2BitStatus = GT_FALSE;
                    }
                    else
                    {
                        BMP_CLEAR_MAC(&fdbManagerPtr->agingBinInfo.level2UsageBitmap, tmpCnt);
                        restoreInfoPtr->l2BitStatus = GT_TRUE;
                    }
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    if(restoreInfoPtr->l2BitStatus)
                    {
                        BMP_SET_MAC(&fdbManagerPtr->agingBinInfo.level2UsageBitmap, tmpCnt);
                    }
                    else
                    {
                        BMP_CLEAR_MAC(&fdbManagerPtr->agingBinInfo.level2UsageBitmap, tmpCnt);
                    }
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E);
                }
                break;

            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E:
                dbIndex     = fdbManagerPtr->headOfUsedList.entryPointer;
                dbEntryPtr  = &fdbManagerPtr->entryPoolPtr[dbIndex];
                for(tmpCnt = 0; fdbManagerPtr->capacityInfo.maxTotalEntries > tmpCnt; tmpCnt++)
                {
                    dbEntryPtr  = &fdbManagerPtr->entryPoolPtr[dbIndex];
                    GET_nextEntryPointer_MAC(dbEntryPtr, dbIndex);
                    /* break on First mac entry */
                    if(dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E)
                    {
                        break;
                    }
                }
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->macAddr_low_32 = dbEntryPtr->specificFormat.prvMacEntryFormat.macAddr_low_32;
                    dbEntryPtr->specificFormat.prvMacEntryFormat.macAddr_low_32 += 1;
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    dbEntryPtr->specificFormat.prvMacEntryFormat.macAddr_low_32 = restoreInfoPtr->macAddr_low_32;
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E);
                }
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E:
                rc = prvCpssDxChFdbManagerDebugSelfTest_SpEntry(fdbManagerPtr, errorEnable, &restoreInfoPtr->hwIndex);
                if(rc != GT_OK)
                {
                    return rc;
                }
                if(errorEnable)
                {
                    SELF_TEST_BMP_SET_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E);
                }
                else
                {
                    SELF_TEST_BMP_CLR_MAC(fdbManagerPtr, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E);
                }
                break;
            default:
                break;
        }
    }
    return rc;
}

/**
* @internal prvCpssDxChFdbManagerDbIsGlobalEport function
* @endinternal
*
* @brief  function to check if the portNum is in the global eport range/mask
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] portNum               - the portNum to check
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_TRUE              - the portNum is     considered 'Global Eport'
* @retval GT_FALSE             - the portNum is not considered 'Global Eport'
*/
GT_BOOL prvCpssDxChFdbManagerDbIsGlobalEport(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN GT_PORT_NUM                                              portNum
)
{
    if(fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E)
    {
        /* SIP4 hold no 'global eport' support */
        return GT_FALSE;
    }

    if(fdbManagerPtr->entryAttrInfo.globalEportInfo.enable ==
        CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E)
    {
        if((portNum & fdbManagerPtr->entryAttrInfo.globalEportInfo.mask) ==
            (fdbManagerPtr->entryAttrInfo.globalEportInfo.pattern &
             fdbManagerPtr->entryAttrInfo.globalEportInfo.mask))
        {
            return GT_TRUE;
        }
    }
    else
    if(fdbManagerPtr->entryAttrInfo.globalEportInfo.enable ==
        CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E)
    {
        if(portNum >= fdbManagerPtr->entryAttrInfo.globalEportInfo.minValue &&
           portNum <= fdbManagerPtr->entryAttrInfo.globalEportInfo.maxValue)
        {
            return GT_TRUE;
        }
    }

    return GT_FALSE;
}

#define OPER_DIFF_MAC(oper) \
    (oper == LIMIT_OPER_ADD_NEW_E ||    \
     oper == LIMIT_OPER_INTERFACE_UPDATE_NEW_PART_INFO_GET_E) ? 1 : 0
/**
* @internal isOverLimitCheck_global function
* @endinternal
*
* @brief  function to check if there is violation of the dynamic UC MAC limit : Global.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] oper                  - operation type
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_TRUE              - the entry     violate one of the limits
* @retval GT_FALSE             - the entry not violate any of the limits
*/
static GT_BOOL isOverLimitCheck_global(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN LIMIT_OPER_ENT         oper
)
{
    GT_U32  operDiff = OPER_DIFF_MAC(oper);

    if((fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.actualCounter + operDiff) >
        fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.limitThreshold)
    {
        return GT_TRUE;
    }

    return GT_FALSE;
}
/**
* @internal isOverLimitCheck_fid function
* @endinternal
*
* @brief  function to check if the entry violate the dynamic UC MAC limit : fid
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] oper                  - operation type
* @param[in] fid                   - the fid of the entry
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_TRUE              - the entry     violate one of the limits
* @retval GT_FALSE             - the entry not violate any of the limits
*/
static GT_BOOL isOverLimitCheck_fid(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN LIMIT_OPER_ENT         oper,
    IN GT_U32                       fid
)
{
    GT_U32  operDiff = OPER_DIFF_MAC(oper);

    if((fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_fid[fid].actualCounter + operDiff) >
        fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_fid[fid].limitThreshold)
    {
        return GT_TRUE;
    }

    return GT_FALSE;
}
/**
* @internal isOverLimitCheck_global_eport function
* @endinternal
*
* @brief  function to check if the entry violate the dynamic UC MAC limit : global eport
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] oper                  - operation type
* @param[in] portNum               - the portNum of the entry
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_TRUE              - the entry     violate one of the limits
* @retval GT_FALSE             - the entry not violate any of the limits
*/
static GT_BOOL isOverLimitCheck_global_eport(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN LIMIT_OPER_ENT         oper,
    IN GT_PORT_NUM                  portNum,
    OUT GT_BOOL                 *isGlobalEportPtr
)
{
    GT_U32  operDiff = OPER_DIFF_MAC(oper);
    GT_U32  index;

    *isGlobalEportPtr = GT_FALSE;

    if(GT_FALSE == prvCpssDxChFdbManagerDbIsGlobalEport(fdbManagerPtr,portNum))
    {
        return GT_FALSE;
    }

    *isGlobalEportPtr = GT_TRUE;

    if(portNum < fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEport_base)
    {
        /* the eport is less than minimum */
        return GT_FALSE;
    }

    index = portNum - fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEport_base;

    if((fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEportPtr[index].actualCounter + operDiff) >
        fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEportPtr[index].limitThreshold)
    {
        return GT_TRUE;
    }

    return GT_FALSE;
}
/**
* @internal isOverLimitCheck_eport function
* @endinternal
*
* @brief  function to check if the entry violate the dynamic UC MAC limit : eport
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] oper                  - operation type
* @param[in] hwDevNum              - the hwDevNum of the entry
* @param[in] portNum               - the portNum of the entry
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_TRUE              - the entry     violate one of the limits
* @retval GT_FALSE             - the entry not violate any of the limits
*/
static GT_BOOL isOverLimitCheck_eport(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN LIMIT_OPER_ENT         oper,
    IN GT_HW_DEV_NUM                hwDevNum,
    IN GT_PORT_NUM                  portNum
)
{
    GT_U32  operDiff = OPER_DIFF_MAC(oper);

    if(NULL == fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum])
    {
        /* don't know this hwDevNum */
        return GT_FALSE;
    }

    if(fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum]->numPorts <= portNum)
    {
        /* portNum too high */
        return GT_FALSE;
    }

    if((fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum]->perEportPtr[portNum].actualCounter + operDiff) >
        fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum]->perEportPtr[portNum].limitThreshold)
    {
        return GT_TRUE;
    }

    return GT_FALSE;
}

/**
* @internal isOverLimitCheck_fid function
* @endinternal
*
* @brief  function to check if the entry violate the dynamic UC MAC limit : trunkId
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] oper                  - operation type
* @param[in] trunkId               - the trunkId of the entry
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_TRUE              - the entry     violate one of the limits
* @retval GT_FALSE             - the entry not violate any of the limits
*/
static GT_BOOL isOverLimitCheck_trunkId(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN LIMIT_OPER_ENT         oper,
    IN GT_TRUNK_ID                  trunkId
)
{
    GT_U32  operDiff = OPER_DIFF_MAC(oper);

    if((fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_trunkId[trunkId].actualCounter + operDiff) >
        fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_trunkId[trunkId].limitThreshold)
    {
        return GT_TRUE;
    }

    return GT_FALSE;
}
/**
* @internal prvCpssDxChFdbManagerDbIsOverLimitCheck function
* @endinternal
*
* @brief  function to check if the entry violate any of the dynamic UC MAC limits.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] oper                  - options are :
*                                     1. LIMIT_OPER_CHECK_CURRENT_E
*                                     2. LIMIT_OPER_INTERFACE_UPDATE_OLD_PART_INFO_GET_E
* @param[in] dbEntryIpUcPtr        - (pointer to) the MAC format (DB entry)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_TRUE              - the entry     violate one of the limits
* @retval GT_FALSE             - the entry not violate any of the limits
*
*/
GT_BOOL prvCpssDxChFdbManagerDbIsOverLimitCheck(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN LIMIT_OPER_ENT                                           oper,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC  *dbEntryMacPtr
)
{
    GT_BOOL isGlobalEport;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_LAST_LIMIT_INFO_STC *limitInfoPtr;

    if(NULL == fdbManagerPtr->dynamicUcMacLimitPtr)
    {
        return GT_FALSE;
    }

    if(oper == LIMIT_OPER_INTERFACE_UPDATE_OLD_PART_INFO_GET_E)
    {
        limitInfoPtr = &fdbManagerPtr->dynamicUcMacLimitPtr->oldInterfaceLimitInfo;
    }
    else
    {
        /* fill info about the added/deleted entry */
        limitInfoPtr = &fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo;
    }

    limitInfoPtr->rcError          = GT_OK;
    limitInfoPtr->limitCountNeeded = GT_FALSE;

    if(dbEntryMacPtr->isStatic == GT_TRUE)
    {
        /* not relevant to static entry */
        return GT_FALSE;
    }

    if(dbEntryMacPtr->macAddr_high_16 & 0x0100) /* check bit 40 of the mac address */
    {
        /* not relevant to MC/BC entry */
        return GT_FALSE;
    }

    limitInfoPtr->limitCountNeeded = GT_TRUE;
    limitInfoPtr->fid              = dbEntryMacPtr->fid;
    limitInfoPtr->globalEport      = GT_NA;
    limitInfoPtr->trunkId          = GT_NA;
    limitInfoPtr->devPort.hwDevNum = GT_NA;
    limitInfoPtr->devPort.portNum  = GT_NA;
    limitInfoPtr->interfaceType    = dbEntryMacPtr->dstInterface_type;

    switch(dbEntryMacPtr->dstInterface_type)
    {
        case CPSS_INTERFACE_VIDX_E:
        case CPSS_INTERFACE_VID_E:
            /* no more checks */
            break;
        case CPSS_INTERFACE_PORT_E:
            if(oper == LIMIT_OPER_INTERFACE_UPDATE_OLD_PART_INFO_GET_E)
            {
                /* we not care about limit checks , we only need to fill global-eport info */
                isGlobalEport = prvCpssDxChFdbManagerDbIsGlobalEport(fdbManagerPtr,
                    dbEntryMacPtr->dstInterface.devPort.portNum);
            }
            else
            if(GT_TRUE == isOverLimitCheck_global_eport(fdbManagerPtr,oper,
                    dbEntryMacPtr->dstInterface.devPort.portNum,&isGlobalEport))
            {
                limitInfoPtr->globalEport = dbEntryMacPtr->dstInterface.devPort.portNum;
                limitInfoPtr->rcError = GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR;
                return GT_TRUE;
            }

            if(isGlobalEport == GT_TRUE)
            {
                limitInfoPtr->globalEport      = dbEntryMacPtr->dstInterface.devPort.portNum;
            }
            else
            {
                limitInfoPtr->devPort.hwDevNum = dbEntryMacPtr->dstInterface.devPort.hwDevNum;
                limitInfoPtr->devPort.portNum  = dbEntryMacPtr->dstInterface.devPort.portNum;

                if(oper == LIMIT_OPER_INTERFACE_UPDATE_OLD_PART_INFO_GET_E)
                {
                    /* we not care about limit checks , we only need to fill {dev,port} info */
                }
                else
                if(GT_TRUE == isOverLimitCheck_eport(fdbManagerPtr,oper,
                        dbEntryMacPtr->dstInterface.devPort.hwDevNum,
                        dbEntryMacPtr->dstInterface.devPort.portNum))
                {
                    limitInfoPtr->rcError = GT_LEARN_LIMIT_PORT_ERROR;
                    return GT_TRUE;
                }
            }
            break;
        case CPSS_INTERFACE_TRUNK_E:
            limitInfoPtr->trunkId          = dbEntryMacPtr->dstInterface.trunkId;

            if(oper == LIMIT_OPER_INTERFACE_UPDATE_OLD_PART_INFO_GET_E)
            {
                /* we not care about limit checks , we only need to fill trunkId info */
            }
            else
            if(GT_TRUE == isOverLimitCheck_trunkId(fdbManagerPtr,oper,
                    dbEntryMacPtr->dstInterface.trunkId))
            {
                limitInfoPtr->rcError = GT_LEARN_LIMIT_TRUNK_ERROR;
                return GT_TRUE;
            }
            break;
        default:
            /* will not get here */
            break;
    }

    if(oper == LIMIT_OPER_INTERFACE_UPDATE_OLD_PART_INFO_GET_E)
    {
        /* we not care about limit checks */
        return GT_FALSE;
    }


    if(GT_TRUE == isOverLimitCheck_fid(fdbManagerPtr,oper,dbEntryMacPtr->fid))
    {
        /* error priority for the interfaces */
        limitInfoPtr->rcError = GT_LEARN_LIMIT_FID_ERROR;
        return GT_TRUE;
    }

    if(GT_TRUE == isOverLimitCheck_global(fdbManagerPtr,oper))
    {
        /* error priority for the interfaces/fid */
        limitInfoPtr->rcError = GT_LEARN_LIMIT_GLOBAL_ERROR;
        return GT_TRUE;
    }

    return GT_FALSE;
}

/**
* @internal prvCpssDxChFdbManagerDbLimitOper function
* @endinternal
*
* @brief  function to update the dynamic UC MAC current counters. (doing ++ or -- on the counters)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] oper                  - options are :
*                                     1. LIMIT_OPER_ADD_NEW_E
*                                     2. LIMIT_OPER_DELETE_OLD_E
*                                     3. LIMIT_OPER_INTERFACE_UPDATE_E:
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                - on success
* @retval GT_BAD_STATE         - DB coherency issue
*/
GT_STATUS prvCpssDxChFdbManagerDbLimitOper(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN LIMIT_OPER_ENT                                           oper
)
{
    GT_PORT_NUM     globalEport;
    GT_U32          trunkId;
    GT_U32          fid;
    CPSS_INTERFACE_DEV_PORT_STC     devPort;

    if(NULL == fdbManagerPtr->dynamicUcMacLimitPtr)
    {
        /* nothing to do */
        return GT_OK;
    }

    if(fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.limitCountNeeded == GT_FALSE)
    {
        if(oper != LIMIT_OPER_INTERFACE_UPDATE_E)
        {
            /* nothing more to do */
            goto dbCoherencyCheck_lbl;
        }
        else
        {
            /* check if need to decrement counters related to 'old' entry */
            goto startOldInterface_lbl;
        }
    }

    globalEport      = fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.globalEport;
    trunkId          = fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.trunkId;
    fid              = fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.fid;
    devPort.hwDevNum = fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.devPort.hwDevNum;
    devPort.portNum  = fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.devPort.portNum;

    if(devPort.hwDevNum != GT_NA && devPort.portNum != GT_NA)
    {
        if(NULL == fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[devPort.hwDevNum])
        {
            devPort.hwDevNum = GT_NA;
        }
        else
        if(devPort.portNum >= fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[devPort.hwDevNum]->numPorts)
        {
            devPort.portNum = GT_NA;
        }
    }

    if(oper == LIMIT_OPER_ADD_NEW_E || oper == LIMIT_OPER_INTERFACE_UPDATE_E)
    {
        fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.actualCounter ++;

        fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_fid[fid].actualCounter ++;
        if(trunkId != GT_NA)
        {
            fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_trunkId[trunkId].actualCounter ++;
        }
        else
        if(globalEport != GT_NA)
        {
            globalEport -= fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEport_base;
            fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEportPtr[globalEport].actualCounter ++;
        }
        else
        if(devPort.hwDevNum != GT_NA && devPort.portNum != GT_NA)
        {
            fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[devPort.hwDevNum]->perEportPtr[devPort.portNum].actualCounter ++;
        }
    }

    if(oper == LIMIT_OPER_INTERFACE_UPDATE_E)
    {
        startOldInterface_lbl:/* start the 'old interface' operation */

        if(fdbManagerPtr->dynamicUcMacLimitPtr->oldInterfaceLimitInfo.limitCountNeeded == GT_FALSE)
        {
            /* nothing more to do */
            goto dbCoherencyCheck_lbl;
        }

        /* switch to the info of 'old interface' */
        globalEport      = fdbManagerPtr->dynamicUcMacLimitPtr->oldInterfaceLimitInfo.globalEport;
        trunkId          = fdbManagerPtr->dynamicUcMacLimitPtr->oldInterfaceLimitInfo.trunkId;
        fid              = fdbManagerPtr->dynamicUcMacLimitPtr->oldInterfaceLimitInfo.fid;
        devPort.hwDevNum = fdbManagerPtr->dynamicUcMacLimitPtr->oldInterfaceLimitInfo.devPort.hwDevNum;
        devPort.portNum  = fdbManagerPtr->dynamicUcMacLimitPtr->oldInterfaceLimitInfo.devPort.portNum;

        if(devPort.hwDevNum != GT_NA && devPort.portNum != GT_NA)
        {
            if(NULL == fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[devPort.hwDevNum])
            {
                devPort.hwDevNum = GT_NA;
            }
            else
            if(devPort.portNum >= fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[devPort.hwDevNum]->numPorts)
            {
                devPort.portNum = GT_NA;
            }
        }
    }

    if(oper == LIMIT_OPER_DELETE_OLD_E || oper == LIMIT_OPER_INTERFACE_UPDATE_E)
    {
        DECREMENT_PARAM_MAC(fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.actualCounter);

        DECREMENT_PARAM_MAC(fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_fid[fid].actualCounter);
        if(trunkId != GT_NA)
        {
            DECREMENT_PARAM_MAC(fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_trunkId[trunkId].actualCounter);
        }
        else
        if(globalEport != GT_NA)
        {
            globalEport -= fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEport_base;
            DECREMENT_PARAM_MAC(fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEportPtr[globalEport].actualCounter);
        }
        else
        if(devPort.hwDevNum != GT_NA && devPort.portNum != GT_NA)
        {
            DECREMENT_PARAM_MAC(fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[devPort.hwDevNum]->perEportPtr[devPort.portNum].actualCounter);
        }
    }

dbCoherencyCheck_lbl:
    /* do coherency check after we updated dynamicUcMacLimit_global.actualCounter */
    CHECK_X_EQUAL_Y_MAC(fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global.actualCounter,
        fdbManagerPtr->typeCountersArr[PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_DYNAMIC_E].entryTypePopulation)

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerDbLimitPerHwDevNumPerPort function
* @endinternal
*
* @brief  the function increment/decrement from the per {hwDevNum,portNum} current
*           counter of the dynamic UC mac limit.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] oper                  - options are :
*                                     1. LIMIT_OPER_ADD_NEW_E
* @param[in] dbEntryIpUcPtr        - (pointer to) the MAC format (DB entry)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                   - on success
* @retval GT_FAIL                 - on error
*/
GT_STATUS prvCpssDxChFdbManagerDbLimitPerHwDevNumPerPort(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN LIMIT_OPER_ENT                                          oper,
    IN GT_HW_DEV_NUM                                           hwDevNum,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC  *dbEntryMacPtr
)
{
    GT_U32  portNum;

    if(NULL == fdbManagerPtr->dynamicUcMacLimitPtr)
    {
        /* the DB not check limits */
        return GT_OK;
    }

    if(NULL == fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum])
    {
        /* the DB not check this hwDevNum */
        return GT_OK;
    }

    if(dbEntryMacPtr->isStatic == GT_TRUE)
    {
        /* not relevant to static entry */
        return GT_OK;
    }

    if(dbEntryMacPtr->macAddr_high_16 & 0x0100) /* check bit 40 of the mac address */
    {
        /* not relevant to MC/BC entry */
        return GT_OK;
    }

    if(dbEntryMacPtr->dstInterface_type != CPSS_INTERFACE_PORT_E)
    {
        /* only 'port' type hold 'hwDevNum' for this limit */
        return GT_OK;
    }

    portNum = dbEntryMacPtr->dstInterface.devPort.portNum;

    if(dbEntryMacPtr->dstInterface.devPort.hwDevNum != hwDevNum)
    {
        /* the entry not relevant to current checked hwDevNum */
        return GT_OK;
    }

    if(portNum >= fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum]->numPorts)
    {
        /* portNum too high */
        return GT_OK;
    }

    if(GT_TRUE == prvCpssDxChFdbManagerDbIsGlobalEport(fdbManagerPtr,portNum))
    {
        /* global eports are counted 'device less' and not per 'hwDevNum' for this limit */
        return GT_OK;
    }

    if(oper == LIMIT_OPER_ADD_NEW_E)
    {
        /* update the actualCounter (++) */
        fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum]->perEportPtr[portNum].actualCounter++;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerDbIsOverLimitCheck_applicationFormat function
* @endinternal
*
* @brief  function to check if the entry violate any of the dynamic UC MAC limits.
*           the entry in in 'application format'
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] oper                  - options are :
*                                     1. LIMIT_OPER_ADD_NEW_E
*                                     2. LIMIT_OPER_INTERFACE_UPDATE_NEW_PART_INFO_GET_E
*
* @param[in] entryMacPtr           - (pointer to) the MAC format (DB entry)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_TRUE              - the entry     violate one of the limits
* @retval GT_FALSE             - the entry not violate any of the limits
*
* @note : should be 'same' as the code in prvCpssDxChFdbManagerDbIsOverLimitCheck(...)
*/
GT_BOOL prvCpssDxChFdbManagerDbIsOverLimitCheck_applicationFormat(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC       *fdbManagerPtr,
    IN LIMIT_OPER_ENT                                        oper,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC  *entryMacPtr
)
{
    GT_BOOL isGlobalEport;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_LAST_LIMIT_INFO_STC *limitInfoPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_LAST_LIMIT_INFO_STC *oldInterfaceLimitInfoPtr;

    /* fill info about the added/deleted entry */
    limitInfoPtr = &fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo;

    limitInfoPtr->rcError          = GT_OK;
    limitInfoPtr->limitCountNeeded = GT_FALSE;

    if(entryMacPtr->isStatic == GT_TRUE)
    {
        /* not relevant to static entry */
        return GT_FALSE;
    }

    if(entryMacPtr->macAddr.arEther[0] & 0x01) /* check bit 40 of the mac address */
    {
        /* not relevant to MC/BC entry */
        return GT_FALSE;
    }

    limitInfoPtr->limitCountNeeded = GT_TRUE;
    limitInfoPtr->fid              = entryMacPtr->fid;
    limitInfoPtr->globalEport      = GT_NA;
    limitInfoPtr->trunkId          = GT_NA;
    limitInfoPtr->devPort.hwDevNum = GT_NA;
    limitInfoPtr->devPort.portNum  = GT_NA;
    limitInfoPtr->interfaceType    = entryMacPtr->dstInterface.type;

    if(oper == LIMIT_OPER_INTERFACE_UPDATE_NEW_PART_INFO_GET_E)
    {
        /* we need to check 'over the limit' only on 'changed' fid/interface
           compared to the 'old entry' */
        oldInterfaceLimitInfoPtr = &fdbManagerPtr->dynamicUcMacLimitPtr->oldInterfaceLimitInfo;
    }
    else
    {
        oldInterfaceLimitInfoPtr = NULL;
    }

    switch(entryMacPtr->dstInterface.type)
    {
        case CPSS_INTERFACE_VIDX_E:
        case CPSS_INTERFACE_VID_E:
            /* no more checks */
            break;
        case CPSS_INTERFACE_PORT_E:
            if(oldInterfaceLimitInfoPtr &&
               oldInterfaceLimitInfoPtr->interfaceType == limitInfoPtr->interfaceType)
            {
                if(entryMacPtr->dstInterface.interfaceInfo.devPort.portNum == oldInterfaceLimitInfoPtr->globalEport)
                {
                    /* not allow to check global eport violation */
                    limitInfoPtr->globalEport      = entryMacPtr->dstInterface.interfaceInfo.devPort.portNum;
                    goto checkFid_lbl;
                }
                else
                if(entryMacPtr->dstInterface.interfaceInfo.devPort.hwDevNum == oldInterfaceLimitInfoPtr->devPort.hwDevNum &&
                   entryMacPtr->dstInterface.interfaceInfo.devPort.portNum  == oldInterfaceLimitInfoPtr->devPort.portNum)
                {
                    /* not allow to check dev,port violation */
                    limitInfoPtr->devPort.hwDevNum = entryMacPtr->dstInterface.interfaceInfo.devPort.hwDevNum;
                    limitInfoPtr->devPort.portNum  = entryMacPtr->dstInterface.interfaceInfo.devPort.portNum;
                    goto checkFid_lbl;
                }
            }

            if(GT_TRUE == isOverLimitCheck_global_eport(fdbManagerPtr,oper,
                    entryMacPtr->dstInterface.interfaceInfo.devPort.portNum,&isGlobalEport))
            {
                limitInfoPtr->globalEport = entryMacPtr->dstInterface.interfaceInfo.devPort.portNum;
                limitInfoPtr->rcError = GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR;
                return GT_TRUE;
            }

            if(isGlobalEport == GT_TRUE)
            {
                limitInfoPtr->globalEport      = entryMacPtr->dstInterface.interfaceInfo.devPort.portNum;
            }
            else
            {
                limitInfoPtr->devPort.hwDevNum = entryMacPtr->dstInterface.interfaceInfo.devPort.hwDevNum;
                limitInfoPtr->devPort.portNum  = entryMacPtr->dstInterface.interfaceInfo.devPort.portNum;
                if(GT_TRUE == isOverLimitCheck_eport(fdbManagerPtr,oper,
                        entryMacPtr->dstInterface.interfaceInfo.devPort.hwDevNum,
                        entryMacPtr->dstInterface.interfaceInfo.devPort.portNum))
                {
                    limitInfoPtr->rcError = GT_LEARN_LIMIT_PORT_ERROR;
                    return GT_TRUE;
                }
            }

            break;
        case CPSS_INTERFACE_TRUNK_E:
            limitInfoPtr->trunkId          = entryMacPtr->dstInterface.interfaceInfo.trunkId;

            if(oldInterfaceLimitInfoPtr &&
               oldInterfaceLimitInfoPtr->interfaceType == limitInfoPtr->interfaceType)
            {
                if(entryMacPtr->dstInterface.interfaceInfo.trunkId == oldInterfaceLimitInfoPtr->trunkId)
                {
                    /* not allow to check trunk violation */
                    goto checkFid_lbl;
                }
            }

            if(GT_TRUE == isOverLimitCheck_trunkId(fdbManagerPtr,oper,
                    entryMacPtr->dstInterface.interfaceInfo.trunkId))
            {
                limitInfoPtr->rcError = GT_LEARN_LIMIT_TRUNK_ERROR;
                return GT_TRUE;
            }
            break;
        default:
            /* will not get here */
            break;
    }

    checkFid_lbl:
    if(oldInterfaceLimitInfoPtr &&
       oldInterfaceLimitInfoPtr->fid == limitInfoPtr->fid)
    {
        /* not allowing to check fid violation */
    }
    else
    if(GT_TRUE == isOverLimitCheck_fid(fdbManagerPtr,oper,entryMacPtr->fid))
    {
        /* error priority for the interfaces */
        limitInfoPtr->rcError = GT_LEARN_LIMIT_FID_ERROR;
        return GT_TRUE;
    }

    if(oldInterfaceLimitInfoPtr)
    {
        /* not allowing to check global violation */
    }
    else
    if(GT_TRUE == isOverLimitCheck_global(fdbManagerPtr,oper))
    {
        /* error priority for the interfaces/fid */
        limitInfoPtr->rcError = GT_LEARN_LIMIT_GLOBAL_ERROR;
        return GT_TRUE;
    }

    return GT_FALSE;
}

/**
* @internal prvCpssDxChFdbManagerDbIsOverLimitCheck_newSpecificInterface function
* @endinternal
*
* @brief  function to check if the entry violate any of the dynamic UC MAC limits.
*       the function is called only for UC mac entries.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] old_dbEntryMacPtr     - (pointer to) the MAC format with old interface (DB entry)
* @param[in] new_interfacePtr      - (pointer to) the new interface.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_TRUE              - the new interface     violate one of the interface limits
* @retval GT_FALSE             - the new interface not violate any of the interface limits
*
*/
GT_BOOL prvCpssDxChFdbManagerDbIsOverLimitCheck_newSpecificInterface(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC  *old_dbEntryMacPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC      *new_interfacePtr
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_LAST_LIMIT_INFO_STC *limitInfoPtr;
    LIMIT_OPER_ENT         oper;
    GT_BOOL                isGlobalEport;

    /* fill info about the added/deleted entry */
    limitInfoPtr = &fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo;

    limitInfoPtr->limitCountNeeded = GT_FALSE;
    limitInfoPtr->rcError = GT_OK;

    if(old_dbEntryMacPtr->isStatic == GT_TRUE)
    {
        /* not relevant to static entry */
        return GT_FALSE;
    }

    if(old_dbEntryMacPtr->macAddr_high_16 & 0x0100) /* check bit 40 of the mac address */
    {
        /* not relevant to MC/BC entry */
        return GT_FALSE;
    }

    limitInfoPtr->limitCountNeeded = GT_TRUE;
    limitInfoPtr->fid              = old_dbEntryMacPtr->fid;
    limitInfoPtr->globalEport      = GT_NA;
    limitInfoPtr->trunkId          = GT_NA;
    limitInfoPtr->devPort.hwDevNum = GT_NA;
    limitInfoPtr->devPort.portNum  = GT_NA;
    limitInfoPtr->interfaceType    = new_interfacePtr->type;

    oper = LIMIT_OPER_ADD_NEW_E;

    switch(new_interfacePtr->type)
    {
        case CPSS_INTERFACE_VIDX_E:
        case CPSS_INTERFACE_VID_E:
            /* no more checks */
            break;
        case CPSS_INTERFACE_PORT_E:
            if(new_interfacePtr->interfaceInfo.devPort.portNum == old_dbEntryMacPtr->dstInterface.devPort.portNum)
            {
                /* the port not changed so not look for eport violation */
                isGlobalEport = GT_FALSE;
            }
            else
            if(GT_TRUE == isOverLimitCheck_global_eport(fdbManagerPtr,oper,
                    new_interfacePtr->interfaceInfo.devPort.portNum,&isGlobalEport))
            {
                limitInfoPtr->globalEport = new_interfacePtr->interfaceInfo.devPort.portNum;
                limitInfoPtr->rcError = GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR;
                return GT_TRUE;
            }

            if(isGlobalEport == GT_TRUE)
            {
                limitInfoPtr->globalEport      = new_interfacePtr->interfaceInfo.devPort.portNum;
            }
            else
            if(new_interfacePtr->interfaceInfo.devPort.portNum  == old_dbEntryMacPtr->dstInterface.devPort.portNum &&
               new_interfacePtr->interfaceInfo.devPort.hwDevNum == old_dbEntryMacPtr->dstInterface.devPort.hwDevNum)
            {
                /* the {hwDevNum,portNum} not changed so not look for {hwDevNum,portNum} violation */
            }
            else
            {
                limitInfoPtr->devPort.hwDevNum = new_interfacePtr->interfaceInfo.devPort.hwDevNum;
                limitInfoPtr->devPort.portNum  = new_interfacePtr->interfaceInfo.devPort.portNum;

                if(GT_TRUE == isOverLimitCheck_eport(fdbManagerPtr,oper,
                        new_interfacePtr->interfaceInfo.devPort.hwDevNum,
                        new_interfacePtr->interfaceInfo.devPort.portNum))
                {
                    limitInfoPtr->rcError = GT_LEARN_LIMIT_PORT_ERROR;
                    return GT_TRUE;
                }
            }
            break;
        case CPSS_INTERFACE_TRUNK_E:
            limitInfoPtr->trunkId          = new_interfacePtr->interfaceInfo.trunkId;

            if(new_interfacePtr->interfaceInfo.trunkId == old_dbEntryMacPtr->dstInterface.trunkId)
            {
                /* the {hwDevNum,portNum} not changed so not look for {hwDevNum,portNum} violation */
            }
            else
            if(GT_TRUE == isOverLimitCheck_trunkId(fdbManagerPtr,oper,
                    new_interfacePtr->interfaceInfo.trunkId))
            {
                limitInfoPtr->rcError = GT_LEARN_LIMIT_TRUNK_ERROR;
                return GT_TRUE;
            }
            break;
        default:
            /* will not get here */
            break;
    }

    /* not violate the new interface */
    return GT_FALSE;

}

/**
* @internal prvCpssDxChFdbManagerDbUpdateManagerParams function
* @endinternal
*
* @brief  The function validate 'API level' 'manager config update' parameters.
*         The function save those updated values to the DB.
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerId          - the FDB Manager id to associate with the newly created manager.
*                                    (APPLICABLE RANGES : 0..31)
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
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS    prvCpssDxChFdbManagerDbUpdateManagerParams
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC         *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC   *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC           *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC              *agingPtr
)
{
    if(!IS_FDB_MANAGER_FOR_SIP_4(fdbManagerPtr))
    {
        if(entryAttrPtr)
        {
            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(entryAttrPtr->ipNhPacketcommand,cpssDxChBrgFdbRoutingNextHopPacketCmdSet);
            switch(entryAttrPtr->ipNhPacketcommand)
            {
                case CPSS_PACKET_CMD_ROUTE_E:
                case CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E:
                case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
                case CPSS_PACKET_CMD_DROP_HARD_E:
                case CPSS_PACKET_CMD_DROP_SOFT_E:
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryAttrPtr->ipNhPacketcommand);
            }

            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(entryAttrPtr->saDropCommand,cpssDxChBrgFdbSaDaCmdDropModeSet);
            /* In case of AC5 this global param is moved to per entry */
            switch(entryAttrPtr->saDropCommand)
            {
                case CPSS_PACKET_CMD_DROP_HARD_E:
                case CPSS_PACKET_CMD_DROP_SOFT_E:
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryAttrPtr->saDropCommand);
            }

            BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(entryAttrPtr->daDropCommand,cpssDxChBrgFdbSaDaCmdDropModeSet);
            /* In case of AC5 this global param is moved to per entry */
            switch(entryAttrPtr->daDropCommand)
            {
                case CPSS_PACKET_CMD_DROP_HARD_E:
                case CPSS_PACKET_CMD_DROP_SOFT_E:
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryAttrPtr->daDropCommand);
            }
        }

        BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(agingPtr->destinationUcRefreshEnable , prvCpssDxChBrgFdbAgeBitDaRefreshEnableSet);
        BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(agingPtr->ipUcRefreshEnable , cpssDxChBrgFdbRoutingUcRefreshEnableSet);


        BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(agingPtr->destinationMcRefreshEnable , prvCpssDxChBrgFdbAgeBitDaRefreshEnableSet);
    }

    BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(learningPtr->macRoutedLearningEnable,cpssDxChBrgFdbRoutedLearningEnableSet);

    /******************/
    /* save to the DB */
    /******************/
    if(entryAttrPtr)
    {
        fdbManagerPtr->entryAttrInfo.saDropCommand     = entryAttrPtr->saDropCommand    ;
        fdbManagerPtr->entryAttrInfo.daDropCommand     = entryAttrPtr->daDropCommand    ;
        fdbManagerPtr->entryAttrInfo.ipNhPacketcommand = entryAttrPtr->ipNhPacketcommand;
    }

    if(learningPtr)
    {
        fdbManagerPtr->learningInfo.macRoutedLearningEnable = learningPtr->macRoutedLearningEnable;
    }

    if(agingPtr)
    {
        fdbManagerPtr->agingInfo.destinationUcRefreshEnable = agingPtr->destinationUcRefreshEnable;
        fdbManagerPtr->agingInfo.destinationMcRefreshEnable = agingPtr->destinationMcRefreshEnable;
        fdbManagerPtr->agingInfo.ipUcRefreshEnable          = agingPtr->ipUcRefreshEnable;
    }

    return GT_OK;
}

