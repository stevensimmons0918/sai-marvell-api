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
* @file cpssDxChBrgVlan.c
*
* @brief CPSS DxCh VLAN facility implementation
*
*
* @version   127
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgFdb.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrg.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
#include <cpss/dxCh/dxChxGen/mirror/private/prvCpssDxChMirror.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*  max index of entry that may be configured
   to support port protocol based VLANs and QoS */
#define PRV_CPSS_DXCH_XCAT_PORT_PROT_ENTRY_NUM_MAX_CNS  (11)

/* maximal index of analyzer */
#define  PRV_CPSS_DXCH_MIRROR_TO_ANALYZER_MAX_INDEX_CNS 6

/*  check Port Protocol range, only 8 and 12(for xCat3 and above)
    protocols are supported */
#define PRV_CPSS_DXCH_BRG_PORT_PROTOCOL_CHECK_MAC(_devNum, _entryNum)    \
        if((_entryNum) > PRV_CPSS_DXCH_XCAT_PORT_PROT_ENTRY_NUM_MAX_CNS) \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

/* max number of MRU indexes */
#define PRV_CPSS_DXCH_BRG_MRU_INDEX_MAX_CNS     (7)

/* check MRU indx range */
#define PRV_CPSS_DXCH_BRG_MRU_INDEX_CHECK_MAC(mruInd)   \
    if ((mruInd) > PRV_CPSS_DXCH_BRG_MRU_INDEX_MAX_CNS) \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

/* check and convert IPM Bridging mode */
#define PRV_DXCH_BRG_VLAN_IPM_MODE_CONVERT_MAC(_ipmBrgMode, _hwValue) \
    switch(_ipmBrgMode)                                           \
    {                                                             \
        case CPSS_BRG_IPM_SGV_E: _hwValue = 0; break;             \
        case CPSS_BRG_IPM_GV_E:  _hwValue = 1; break;             \
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                             \
    }

/* Converts flood VIDX mode to hardware value */
#define PRV_CPSS_DXCH_CONVERT_FLOOD_VIDX_MODE_TO_HW_VAL_MAC(_val, _cmd) \
    switch (_cmd)                                                       \
    {                                                                   \
        case CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E:             \
            _val = 0;                                                   \
            break;                                                      \
        case CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E:  \
            _val = 1;                                                   \
            break;                                                      \
        default:                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                        \
    }

#define PRV_CPSS_DXCH_CONVERT_PORT_ISOLATION_MODE_TO_HW_VAL_MAC(_val, _cmd)  \
    switch (_cmd)                                                            \
    {                                                                        \
        case CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E:                \
            _val = 0;                                                        \
            break;                                                           \
        case CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_CMD_E:                     \
            _val = 1;                                                        \
            break;                                                           \
        case CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L3_CMD_E:                     \
            _val = 2;                                                        \
            break;                                                           \
        case CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_L3_CMD_E:                  \
            _val = 3;                                                        \
            break;                                                           \
        default:                                                             \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                             \
    }

#define PRV_CPSS_DXCH_CONVERT_VLAN_TAG_MODE_TO_HW_VAL_MAC(_devNum, _val, _cmd)       \
        switch(_cmd)                                                        \
        {                                                                   \
            case CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E                :    \
                _val = 0;                                                   \
                break;                                                      \
            case CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E                    :    \
                _val = 1;                                                   \
                break;                                                      \
            case CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E                    :    \
                _val = 2;                                                   \
                break;                                                      \
            case CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E   :    \
                _val = 3;                                                   \
                break;                                                      \
            case CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E   :    \
                _val = 4;                                                   \
                break;                                                      \
            case CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E               :    \
                if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)\
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                    \
                _val = 5;                                                   \
                break;                                                      \
            case CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E           :    \
                _val = 6;                                                   \
                break;                                                      \
            case CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E       :    \
                if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)\
                    _val = 7;                                               \
                else                                                        \
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                    \
                break;                                                      \
            default:                                                        \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                        \
        }

#define PRV_CPSS_DXCH_CONVERT_HW_VAL_TO_VLAN_TAG_MODE_MAC(_devNum, _val, _cmd)\
            switch(_val)                                                    \
            {                                                               \
                case 0:                                                     \
                    _cmd = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;          \
                    break;                                                  \
                case 1:                                                     \
                    _cmd = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;              \
                    break;                                                  \
                case 2:                                                     \
                    _cmd = CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E;              \
                    break;                                                  \
                case 3:                                                     \
                    _cmd = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E;\
                    break;                                                  \
                case 4:                                                     \
                    _cmd = CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E;\
                    break;                                                  \
                case 5:                                                     \
                    _cmd = CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E;         \
                    break;                                                  \
                case 6:                                                     \
                    _cmd = CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E;     \
                    break;                                                  \
                case 7:                                                     \
                    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)\
                        _cmd = CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E; \
                    else                                                        \
                        _cmd = (CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT)_val;       \
                    break;                                                  \
                default:                                                    \
                    _cmd = (CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT)_val;       \
                    break;                                                  \
            }

#define PRV_CPSS_DXCH_CONVERT_UNREG_IPM_EVIDX_MODE_TO_HW_VAL_MAC(_val, _mode)       \
            switch(_mode)                                                           \
            {                                                                       \
                case CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E:                \
                    _val = 0;                                                       \
                    break;                                                          \
                case CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV4_E:                \
                    _val = 1;                                                       \
                    break;                                                          \
                case CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV6_E:                \
                    _val = 2;                                                       \
                    break;                                                          \
                case CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_IPV6_INCREMENT_E: \
                    _val = 3;                                                       \
                    break;                                                          \
                default:                                                            \
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                            \
            }

#define PRV_CPSS_DXCH_CONVERT_HW_VAL_TO_UNREG_IPM_EVIDX_MODE_MAC(_val, _mode)             \
            switch(_val)                                                                  \
            {                                                                             \
                default:                                                                  \
                case 0:                                                                   \
                    _mode = CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E;               \
                    break;                                                                \
                case 1:                                                                   \
                    _mode = CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV4_E;               \
                    break;                                                                \
                case 2:                                                                   \
                    _mode = CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_IPV6_E;               \
                    break;                                                                \
                case 3:                                                                   \
                    _mode = CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_IPV6_INCREMENT_E;\
                    break;                                                                \
            }

#define PRV_CPSS_DXCH_CONVERT_FDB_LOOKUP_KEY_MODE_TO_HW_VAL_MAC(_val, _mode) \
    switch(_mode)                                                            \
    {                                                                        \
        case CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_E:                        \
            _val = 0;                                                       \
            break;                                                           \
        case CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_VID1_E:                   \
            _val = 1;                                                       \
            break;                                                           \
        default:                                                             \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                             \
    }                                                                        \

#define PRV_CPSS_DXCH_CONVERT_HW_VAL_TO_FDB_LOOKUP_KEY_MODE_MAC(_val, _mode) \
    switch(_val)                                                             \
    {                                                                        \
        default:                                                             \
        case 0:                                                              \
            _mode = CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_E;                 \
            break;                                                           \
        case 1:                                                              \
            _mode = CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_VID1_E;            \
            break;                                                           \
    }                                                                        \

/* maximal number of words in the DxCh and above vlan entry */
#define PRV_CPSS_DXCH_VLAN_ENTRY_MAX_WORDS_COUNT_CNS      (6)

/* VLAN_INGRESS_ENTRY_WORDS_NUM_CNS - is used in Ingress VLAN related code for
   Lion2, Bobcat2, Caelum, Bobcat3 devices. This constant should be maximal one from
   all relevant devices.
   Number of bits in the Bobcat3;
   - L2i Ingress Bridge Port Membership Table entry - 512 bits  - maximal
   Number of bits in the Bobcat2; Caelum; Bobcat3;
   - L2i Ingress Bridge VLAN Table entry - 114 bits
   - L2i Ingress Bridge Port Membership Table entry - 256 bits  - maximal
   - L2i Span State Group Index Table entry - 12 bits
   Number of bits in the Lion2:
   - TxQ SHT Ingress VLAN Table - 140 bits
   Falcon  supports up to 1024 ports */

#define L2I_INGRESS_VLAN_PORT_MEMBERS_SIZE_CNS  1024
/* number of words in the Bobcat2; Caelum; Bobcat3 and above ingress vlan entry */
#define VLAN_INGRESS_ENTRY_WORDS_NUM_CNS     BITS_TO_WORDS_MAC(L2I_INGRESS_VLAN_PORT_MEMBERS_SIZE_CNS)
/* number of words in the BC3 - Egress vlan entry is 16 , support for 512 ports*/
/* number of words in the Falcon - Egress vlan entry is 32 , support for 1024 ports*/
#define VLAN_EGRESS_ENTRY_WORDS_NUM_CNS      (32) /* support for 1024 ports */

/* number of words in the Bobcat3 and above EGF_QAG Egress vlan entry */
#define EGF_QAG_EGRESS_VLAN_ENTRY_WORDS_NUM_CNS      (96) /* this support 1024*3 bits */


/* the macro set field in the buffer according to the ingress vlan entry format*/
/* NOTE: the macro uses next 'implicit' fields:
hwDataArr - the buffer
prvCpssDxChSip5L2iIngressVlanTableFieldsFormat - the Bobcat2; Caelum; Bobcat3 vlan entry format

!!!!! So the macro gets only the value and the field name !!!!!!
*/
#define SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(_dev,_value,_fieldName)    \
    U32_SET_FIELD_BY_ID_MAC(hwDataArr,                          \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E].fieldsInfoPtr,        \
        _fieldName,                                              \
        _value)

/* the macro get value of field from the buffer according to the ingress vlan entry format*/
/* NOTE: the macro uses next 'implicit' fields:
hwDataArr - the buffer
prvCpssDxChSip5L2iIngressVlanTableFieldsFormat - the Bobcat2; Caelum; Bobcat3 vlan entry format

!!!!! So the macro gets only the value and the field name !!!!!!
*/
#define GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(_dev,_value,_fieldName)    \
    U32_GET_FIELD_BY_ID_MAC(hwDataArr,                          \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E].fieldsInfoPtr,        \
        _fieldName,                                              \
        _value)


/* the macro set value of field from the buffer according to the EGF_SHT egress vlan entry format*/
/* NOTE: the macro uses next 'implicit' fields:
hwDataArr - the buffer
prvCpssDxChSip5EgfShtEgressEVlanTableFieldsFormat - the Bobcat2; Caelum; Bobcat3 EGF_SHT egress vlan entry format

!!!!! So the macro gets only the value and the field name !!!!!!
*/
#define SET_FIELD_IN_EGF_SHT_EGRESS_VLAN_ENTRY_MAC(_dev,_value,_fieldName)    \
    U32_SET_FIELD_BY_ID_MAC(egfSht_hwDataArr,                          \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_EGRESS_EVLAN_E].fieldsInfoPtr,        \
        _fieldName,                                              \
        _value)

/* the macro get value of field from the buffer according to the EGF_SHT egress vlan entry format*/
/* NOTE: the macro uses next 'implicit' fields:
hwDataArr - the buffer
prvCpssDxChSip5EgfShtEgressEVlanTableFieldsFormat - the Bobcat2; Caelum; Bobcat3 EGF_SHT egress vlan entry format

!!!!! So the macro gets only the value and the field name !!!!!!
*/
#define GET_FIELD_IN_EGF_SHT_EGRESS_VLAN_ENTRY_MAC(_dev,_value,_fieldName)    \
    U32_GET_FIELD_BY_ID_MAC(egfSht_hwDataArr,                      \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_EGRESS_EVLAN_E].fieldsInfoPtr,        \
        _fieldName,                                                \
        _value)

#define TCI_GET_LOW_16_MAC(addr)   ((addr[5]) |             \
                                    (addr[4] << 8))
#define TCI_GET_HIGH_32_MAC(addr)  ((addr[0] << 24) |       \
                                    (addr[1] << 16) |       \
                                    (addr[2] << 8)  |       \
                                    (addr[3]))

static GT_STATUS vlanEntrySplitTableWrite
(
    IN  GT_U8                                 devNum,
    IN  GT_U16                                vlanId,
    IN  CPSS_PORTS_BMP_STC                    *portsMembersPtr,
    IN  CPSS_PORTS_BMP_STC                    *portsTaggingPtr,
    IN  CPSS_DXCH_BRG_VLAN_INFO_STC           *vlanInfoPtr,
    IN  CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr
);

static GT_STATUS vlanEntrySplitTableRead
(
    IN  GT_U8                                devNum,
    IN  GT_U16                               vlanId,
    OUT CPSS_PORTS_BMP_STC                   *portsMembersPtr,
    OUT CPSS_PORTS_BMP_STC                   *portsTaggingPtr,
    OUT CPSS_DXCH_BRG_VLAN_INFO_STC          *vlanInfoPtr,
    OUT GT_BOOL                              *isValidPtr,
    OUT CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC *portsTaggingCmdPtr
);

/**
* @internal vlanEntrySplitTableMembersInHemisphereGet function
* @endinternal
*
* @brief   Returns VLAN members in specific hemisphere.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] hemisphere               -  to read from
*
* @param[out] portsMembersPtr          - (pointer to) bmp of vlan port members in the hemisphere
*                                      ports are local to the hemishpere
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS vlanEntrySplitTableMembersInHemisphereGet
(
    IN  GT_U8                               devNum,
    IN  GT_U16                              vlanId,
    IN  GT_U32                              hemisphere,
    OUT CPSS_PORTS_BMP_STC                  *portsMembersPtr
)
{
    GT_STATUS           rc;
    GT_U32              ingressHwData[VLAN_INGRESS_ENTRY_WORDS_NUM_CNS];
    GT_U32              portGroupId;

    cpssOsMemSet(portsMembersPtr,0,sizeof(*portsMembersPtr));

    /* to get members in hemisphere 0, read from portGroup 0,
       and from hemisphere 1 read from portGroup 4 */
    portGroupId =
        (hemisphere * PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS);

    /* read entry from HW ingress table */
    rc = prvCpssDxChPortGroupReadTableEntry(
        devNum, portGroupId,
        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
        (GT_U32)vlanId,/* vid */
        &(ingressHwData[0]));
    if(rc != GT_OK)
    {
        return rc;
    }

    /* convert HW format to SW format */
    /* retrieve port members from HW format (bits 2..65) */
    U32_GET_FIELD_IN_ENTRY_MAC(ingressHwData, 2,32,portsMembersPtr->ports[0]);
    U32_GET_FIELD_IN_ENTRY_MAC(ingressHwData,34,32,portsMembersPtr->ports[1]);

    return GT_OK;
}

/**
* @internal internal_cpssDxChBrgVlanInit function
* @endinternal
*
* @brief   Initialize VLAN for specific device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on system init error.
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanInit
(
    IN GT_U8    devNum
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanInit function
* @endinternal
*
* @brief   Initialize VLAN for specific device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on system init error.
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanInit
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChBrgVlanInit(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChBrgCpuPortBmpConvert function
* @endinternal
*
* @brief   Convert port bitmap according to physical CPU port connection.
*
* @note   APPLICABLE DEVICES:       Lion2.
* @note   NOT APPLICABLE DEVICES:   Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
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
)
{
    GT_U32 cpuPortPortGroupId; /* actual CPU port group */
    GT_U32 cpuPortNum;         /* actual(global) CPU port number */

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* do not call this , as the CPU port not need the manipulation */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    cpuPortPortGroupId = (PRV_CPSS_PP_MAC(devNum)->cpuPortMode !=
                          CPSS_NET_CPU_PORT_MODE_SDMA_E) ?
                          PRV_CPSS_CPU_PORT_PORT_GROUP_ID_MAC(devNum) :
                          PRV_CPSS_NETIF_SDMA_PORT_GROUP_ID_MAC(devNum);

    cpuPortNum = (cpuPortPortGroupId * 16 + 15);

    if(GT_TRUE == isWriteConversion)
    {   /* write conversion - CPU port <63> to actual CPU port */
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(portBitmapPtr, CPSS_CPU_PORT_NUM_CNS))
        {
            if(CPSS_CPU_PORT_NUM_CNS != cpuPortNum)
            {
                CPSS_PORTS_BMP_PORT_SET_MAC(portBitmapPtr, cpuPortNum);
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(portBitmapPtr, CPSS_CPU_PORT_NUM_CNS);
            }
        }
    }
    else
    {   /* read conversion - actual CPU port to CPU port <63> */
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(portBitmapPtr, cpuPortNum))
        {
            if(CPSS_CPU_PORT_NUM_CNS != cpuPortNum)
            {
                CPSS_PORTS_BMP_PORT_SET_MAC(portBitmapPtr, CPSS_CPU_PORT_NUM_CNS);
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(portBitmapPtr, cpuPortNum);
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssBrgVlanHwEntryBuild function
* @endinternal
*
* @brief    Builds vlan entry to buffer.
*           NOTE: for multi-port-groups device , this function called per port group , with the
*           per port group info
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portsMembersPtr          - (pointer to) bmp of ports members in vlan
*                                      CPU port supported
* @param[in] portsTaggingPtr          - (pointer to) bmp of ports tagged in the vlan -
*                                      the relevant parameter for DxCh3 and beyond and
*                                      not relevant for TR101 supported devices.
* @param[in] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[in] portsTaggingCmdPtr       - (pointer to) ports tagging commands in the vlan -
*                                      the relevant parameter for xCat3 and above
*                                      with TR101 feature support.
*
* @param[out] hwDataArr[PRV_CPSS_DXCH_VLAN_ENTRY_MAX_WORDS_COUNT_CNS]
*                                      - pointer to hw VLAN entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr or portsTaggingPtr is
*                                       out of range
* @retval or vlanInfoPtr           ->stgId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS prvCpssBrgVlanHwEntryBuild
(
    IN GT_U8                        devNum,
    IN CPSS_PORTS_BMP_STC           *portsMembersPtr,
    IN CPSS_PORTS_BMP_STC           *portsTaggingPtr,
    IN CPSS_DXCH_BRG_VLAN_INFO_STC  *vlanInfoPtr,
    IN CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr,
    OUT GT_U32                      hwDataArr[PRV_CPSS_DXCH_VLAN_ENTRY_MAX_WORDS_COUNT_CNS]
)
{
    GT_U32      value;          /* for port vlan info building */
    GT_U32      value1;         /* for port vlan info building */
    GT_U32      fieldOffset;
    GT_U32      wordOffset;
    GT_U32      bitOffset;
    GT_U32      ii;
    GT_U32      maxPortNum;   /* max port number that vlan entry support */
    GT_BOOL     tr101Supported;   /* TR101 Feature Support */

    if(vlanInfoPtr->stgId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_STG_ID_MAC(devNum))/* 8 bits in HW */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    maxPortNum = 28;

    /* get TR101 feature support */
    tr101Supported = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.tr101Supported;

    if (portsMembersPtr->ports[0] >= ((GT_U32)(1 << maxPortNum)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if ((tr101Supported == GT_FALSE) && (portsTaggingPtr->ports[0] >= ((GT_U32)(1 << maxPortNum))))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* check ipCtrlToCpuEn */
    if(vlanInfoPtr->ipCtrlToCpuEn > CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E) /* ||
       vlanInfoPtr->ipCtrlToCpuEn < CPSS_DXCH_BRG_IP_CTRL_NONE_E) <-- This can't happen */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* reset all words in hwDataArr */
    cpssOsMemSet((char *) &hwDataArr[0], 0, PRV_CPSS_DXCH_VLAN_ENTRY_MAX_WORDS_COUNT_CNS * sizeof(GT_U32));

    /* Set Word0 */

    switch(vlanInfoPtr->ipv6SiteIdMode)
    {
        case CPSS_IP_SITE_ID_INTERNAL_E: value = 0; break;
        case CPSS_IP_SITE_ID_EXTERNAL_E: value = 1; break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    hwDataArr[0] |= (value << 23);

    /* IPv6 IPM Bridging enable */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv6IpmBrgEn);
    hwDataArr[0] |= (value << 22);

    /* IPv4 IPM Bridging enable */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv4IpmBrgEn);
    hwDataArr[0] |= (value << 21);

    /* IPv6 IPM Bridging mode */
    PRV_DXCH_BRG_VLAN_IPM_MODE_CONVERT_MAC(vlanInfoPtr->ipv6IpmBrgMode, value);
    hwDataArr[0] |= (value << 20);

    /* IPv4 IPM Bridging mode */
    PRV_DXCH_BRG_VLAN_IPM_MODE_CONVERT_MAC(vlanInfoPtr->ipv4IpmBrgMode, value);
    hwDataArr[0] |= (value << 19);

    /* IPv4 Control traffic to CPU enable */
    if ((vlanInfoPtr->ipCtrlToCpuEn == CPSS_DXCH_BRG_IP_CTRL_IPV4_E) ||
        (vlanInfoPtr->ipCtrlToCpuEn == CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E))
    {
        value = 1;
    }
    else
        value = 0;
    hwDataArr[0] |= (value << 18);

    /* IPv6 ICMP to CPU enable */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv6IcmpToCpuEn);
    hwDataArr[0] |= (value << 17);

    /* Mirror to Ingress Analyzer */
    value = BOOL2BIT_MAC(vlanInfoPtr->mirrToRxAnalyzerEn);
    hwDataArr[0] |= (value << 16);

    /* IPv4 IGMP to CPU enable */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv4IgmpToCpuEn);
    hwDataArr[0] |= (value << 15);

    /* Unknown unicast cmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                             (vlanInfoPtr->unkUcastCmd))
    hwDataArr[0] |= (value << 12);

    /* Unregistered IPv6 multicast cmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                             (vlanInfoPtr->unregIpv6McastCmd))
    hwDataArr[0] |= (value << 9);

    /* Unregistered IPv4 multicast cmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                             (vlanInfoPtr->unregIpv4McastCmd))
    hwDataArr[0] |= (value << 6);

    /* Unregistered Non IP multicast cmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                             (vlanInfoPtr->unregNonIpMcastCmd))
    hwDataArr[0] |= (value << 3);


    /* NewSrc Address Is Not Security - revert the bit */
    value = 1 - (BOOL2BIT_MAC(vlanInfoPtr->unkSrcAddrSecBreach));
    hwDataArr[0] |= (value << 1);

    /* valid bit is set to 1 */
    hwDataArr[0] |= 1;


    /* Set Word2 */

    /* Span State Group Index */
    hwDataArr[2] |= ((vlanInfoPtr->stgId & 0xff) << 24);

    /* Unregistered Non IPv4 BC Cmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                            vlanInfoPtr->unregNonIpv4BcastCmd);
    hwDataArr[2] |= (value << 21);

    /* Unregistered IPv4 BC Cmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                            vlanInfoPtr->unregIpv4BcastCmd);
    hwDataArr[2] |= (value << 18);

    /* IPv6 Multicast Route enable */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv6McastRouteEn);
    hwDataArr[2] |= (value << 17);

    /* IPv4 Multicast Route enable */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv4McastRouteEn);
    hwDataArr[2] |= (value << 15);

    /* IPv6 Unicast Route enable */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv6UcastRouteEn);
    hwDataArr[2] |= (value << 16);

    /* IPv4 Unicast Route enable */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv4UcastRouteEn);
    hwDataArr[2] |= (value << 14);

    /* IPv6 Control traffic to CPU enable */
    if ((vlanInfoPtr->ipCtrlToCpuEn == CPSS_DXCH_BRG_IP_CTRL_IPV6_E) ||
        (vlanInfoPtr->ipCtrlToCpuEn == CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E))
    {
        value = 1;
    }
    else
        value = 0;
    hwDataArr[3] |= (value << 7);

    /* BCU DP Trap Mirror enable */
    value = BOOL2BIT_MAC(vlanInfoPtr->bcastUdpTrapMirrEn);
    hwDataArr[3] |= (value << 6);

    /* MRU Index (bit5 is reserved) */
    value = vlanInfoPtr->mruIdx;
    /* check MRU index */
    PRV_CPSS_DXCH_BRG_MRU_INDEX_CHECK_MAC(value);
    hwDataArr[3] |= (value << 2);

    /* Na Msg to CPU enable */
    value = BOOL2BIT_MAC(vlanInfoPtr->naMsgToCpuEn);
    hwDataArr[3] |= (value << 1);

    /* Auto Learning disable */
    value = BOOL2BIT_MAC(vlanInfoPtr->autoLearnDisable);
    hwDataArr[3] |= value;

    if (vlanInfoPtr->floodVidx > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_VIDX_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Flood Vidx */
    hwDataArr[3] |= (vlanInfoPtr->floodVidx << 8);

    /* Multicast Local Switching Enable */
    hwDataArr[3] |= (BOOL2BIT_MAC(vlanInfoPtr->mcastLocalSwitchingEn) << 20);

    /* Port Isolation VLAN command */
    PRV_CPSS_DXCH_CONVERT_PORT_ISOLATION_MODE_TO_HW_VAL_MAC(value, vlanInfoPtr->portIsolationMode);
    hwDataArr[3] |= ( value << 21);

    /* set word5 for xCat */

    /* Unicast Local Switching Enable */
    hwDataArr[5] |= (BOOL2BIT_MAC(vlanInfoPtr->ucastLocalSwitchingEn) << 17);

    /* Flood VIDX Mode */
    PRV_CPSS_DXCH_CONVERT_FLOOD_VIDX_MODE_TO_HW_VAL_MAC(value, vlanInfoPtr->floodVidxMode);
    hwDataArr[5] |= (value << 18);

    /* build ports info */
    for (ii = 0; (ii < PRV_CPSS_PP_MAC(devNum)->numOfPorts); ii++)
    {
        /* get port information: member and tagging */
        value  = (CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsMembersPtr, ii) ? 1 : 0);

        if ( tr101Supported == GT_FALSE )
        {
            value |= (CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsTaggingPtr, ii) ? 2 : 0);

            value1 = 0;
        }
        else
        {
            if (portsTaggingCmdPtr->portsCmd[ii] >=
                CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E)
            {
                /* supported by Bobcat2 and above only */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            value |= ((portsTaggingCmdPtr->portsCmd[ii] & 1) ? 2 : 0);

            value1 = (portsTaggingCmdPtr->portsCmd[ii] >> 1);
        }

        if((value == 0) && (value1 == 0))
        {
            continue;
        }

        if(ii == 27) /* DxCh2 and above has 28 ports */
        {
            /* port 27 data is placed in word 3 bits[23:24] */
            wordOffset = 3;
            bitOffset  = 23;
        }
        else
        {
            /* calculate first field offset in bits */
            fieldOffset = 24 + (ii * 2);    /* 24 first bits */
            wordOffset = fieldOffset >> 5;  /* / 32 */
            bitOffset  = fieldOffset & 0x1f; /* % 32 */
        }
        hwDataArr[wordOffset] |= value << bitOffset;

        /* set port tagging command for xCat3 and above devices */
        /* words 3,4,5 */
        if ( (tr101Supported != GT_FALSE) && (value1 != 0))
        {
            fieldOffset = 25 + (ii * 2);
            wordOffset = 3 + (fieldOffset >> 5);
            bitOffset  = fieldOffset & 0x1f;

            if (bitOffset == 31)
            {
                hwDataArr[wordOffset] |= (value1 & 1) << bitOffset;
                hwDataArr[wordOffset + 1] |= (value1 >> 1);
            }
            else
            {
                hwDataArr[wordOffset] |= value1 << bitOffset;
            }
        }
    }

    if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(
        portsMembersPtr, CPSS_CPU_PORT_NUM_CNS))
    {
        /* CPU port is a member of VLAN */
        hwDataArr[0] |= (1 << 2);
    }

    return GT_OK;
}


/**
* @internal prvCpssBrgVlanHwEntryParse function
* @endinternal
*
* @brief   parse vlan entry from buffer.
*         NOTE: for multi-port-groups device , this function called per port group , to get the
*         per port group info
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] hwDataArr[PRV_CPSS_DXCH_VLAN_ENTRY_MAX_WORDS_COUNT_CNS] - pointer to hw VLAN entry.
*
* @param[out] portsMembersPtr          - (pointer to) bmp of ports members in vlan
*                                      CPU port supported
* @param[out] portsTaggingPtr          - (pointer to) bmp of ports tagged in the vlan -
*                                      the relevant parameter for DxCh3 and beyond and
*                                      not relevant for TR101 supported devices.
* @param[out] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[out] isValidPtr               - (pointer to) VLAN entry status
*                                      GT_TRUE = Vlan is valid
*                                      GT_FALSE = Vlan is not Valid
* @param[out] portsTaggingCmdPtr       - (pointer to) ports tagging commands in the vlan -
*                                      the relevant parameter for xCat3 and above
*                                      with TR101 feature support.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr or portsTaggingPtr is
*                                       out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS prvCpssBrgVlanHwEntryParse
(
    IN GT_U8                        devNum,
    IN GT_U32                       hwDataArr[PRV_CPSS_DXCH_VLAN_ENTRY_MAX_WORDS_COUNT_CNS],
    OUT CPSS_PORTS_BMP_STC          *portsMembersPtr,
    OUT CPSS_PORTS_BMP_STC          *portsTaggingPtr,
    OUT CPSS_DXCH_BRG_VLAN_INFO_STC *vlanInfoPtr,
    OUT GT_BOOL                     *isValidPtr,
    OUT CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr
)
{
    GT_U32      ii;
    GT_U32      value;
    GT_U32      value1 = 0; /* Variable used for words 4,5 parsing. Algorithm uses
                            assumption that the variable is initialized to 0 */
    GT_U32      fieldOffset;
    GT_U32      wordOffset;
    GT_U32      bitOffset;
    GT_BOOL     tr101Supported;   /* TR101 Feature Support */
    GT_U32      maxPortNum;   /* max port number that vlan entry support */

    *isValidPtr = BIT2BOOL_MAC(hwDataArr[0] & 0x1);

    cpssOsMemSet(vlanInfoPtr,0,sizeof(*vlanInfoPtr));

    PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(portsMembersPtr);

    /* get TR101 feature support */
    tr101Supported = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.tr101Supported;

    if (tr101Supported != GT_FALSE)
    {
        cpssOsMemSet(portsTaggingCmdPtr,0,sizeof(*portsTaggingCmdPtr));
    }
    else
    {
        PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(portsTaggingPtr);
    }

    /* Parse Word0 */

    /* NewSrc Address Is Not Security Breach - revert the value */
    value = U32_GET_FIELD_MAC(hwDataArr[0] , 1 , 1);
    vlanInfoPtr->unkSrcAddrSecBreach = BIT2BOOL_MAC(value - 1);

    /* Unregistered Non IP multicast cmd */
    value = U32_GET_FIELD_MAC(hwDataArr[0] , 3 , 3);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unregNonIpMcastCmd,
                                              value)

    /* Unregistered IPv4 multicast cmd */
    value = U32_GET_FIELD_MAC(hwDataArr[0] , 6 , 3);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unregIpv4McastCmd,
                                              value)

    /* Unregistered IPv6 multicast cmd */
    value = U32_GET_FIELD_MAC(hwDataArr[0], 9, 3);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unregIpv6McastCmd,
                                              value);

    /* Unknown unicast cmd */
    value = U32_GET_FIELD_MAC(hwDataArr[0], 12, 3);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unkUcastCmd,
                                              value);

    /* IPv4 IGMP to CPU enable */
    value = U32_GET_FIELD_MAC(hwDataArr[0], 15, 1);
    vlanInfoPtr->ipv4IgmpToCpuEn = BIT2BOOL_MAC(value);

    /* Mirror to Ingress Analyzer */
    value = U32_GET_FIELD_MAC(hwDataArr[0], 16, 1);
    vlanInfoPtr->mirrToRxAnalyzerEn = BIT2BOOL_MAC(value);

    /* IPv6 ICMP to CPU enable */
    value = U32_GET_FIELD_MAC(hwDataArr[0], 17, 1);
    vlanInfoPtr->ipv6IcmpToCpuEn = BIT2BOOL_MAC(value);

    /* IPv4 Control to CPU enable, the IPv6 Control to Cpu will be read later
       and the ipCtrlToCpuEn will be changed accordingly */
    value = U32_GET_FIELD_MAC(hwDataArr[0], 18, 1);

    vlanInfoPtr->ipCtrlToCpuEn = (value == 0x1) ?
                                    CPSS_DXCH_BRG_IP_CTRL_IPV4_E :
                                    CPSS_DXCH_BRG_IP_CTRL_NONE_E;

    /* IPv4 IPM Bridging mode */
    value = U32_GET_FIELD_MAC(hwDataArr[0], 19, 1);
    vlanInfoPtr->ipv4IpmBrgMode = (value == 0x0) ? CPSS_BRG_IPM_SGV_E :
                                                 CPSS_BRG_IPM_GV_E;

    /* IPv6 IPM Bridging mode */
    value = U32_GET_FIELD_MAC(hwDataArr[0], 20, 1);
    vlanInfoPtr->ipv6IpmBrgMode = (value == 0x0) ? CPSS_BRG_IPM_SGV_E :
                                                 CPSS_BRG_IPM_GV_E;

    /* IPv4 IPM Bridging enable */
    value = U32_GET_FIELD_MAC(hwDataArr[0], 21, 1);
    vlanInfoPtr->ipv4IpmBrgEn = BIT2BOOL_MAC(value);

    /* IPv6 IPM Bridging enable */
    value = U32_GET_FIELD_MAC(hwDataArr[0], 22, 1);
    vlanInfoPtr->ipv6IpmBrgEn = BIT2BOOL_MAC(value);

    /* IPv6 Site ID */
    value = U32_GET_FIELD_MAC(hwDataArr[0], 23, 1);

    vlanInfoPtr->ipv6SiteIdMode = (value == 0x0) ?
        CPSS_IP_SITE_ID_INTERNAL_E :
        CPSS_IP_SITE_ID_EXTERNAL_E;


    /* Parse Word2 */

    /* IPv4/6 Multicast Route enable */
    value = U32_GET_FIELD_MAC(hwDataArr[2], 15, 1);
    vlanInfoPtr->ipv4McastRouteEn = BIT2BOOL_MAC(value);

    /* IPv6 Multicast Route enable */
    value = U32_GET_FIELD_MAC(hwDataArr[2], 17, 1);
    vlanInfoPtr->ipv6McastRouteEn = BIT2BOOL_MAC(value);

    /* IPv4 Unicast Route enable */
    value = U32_GET_FIELD_MAC(hwDataArr[2], 14, 1);
    vlanInfoPtr->ipv4UcastRouteEn = BIT2BOOL_MAC(value);

    /* IPv6 Unicast Route enable */
    value = U32_GET_FIELD_MAC(hwDataArr[2], 16, 1);
    vlanInfoPtr->ipv6UcastRouteEn = BIT2BOOL_MAC(value);

    /* Unregistered IPv4 BC Cmd */
    value = U32_GET_FIELD_MAC(hwDataArr[2], 18, 3);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unregIpv4BcastCmd,
                                              value)
    /* Unregistered Non IPv4 BC Cmd */
    value = U32_GET_FIELD_MAC(hwDataArr[2], 21, 3);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unregNonIpv4BcastCmd,
                                              value)

    /* Span State Group Index */
    vlanInfoPtr->stgId = U32_GET_FIELD_MAC(hwDataArr[2], 24, 8);


    /* Parse Word3 - Cheetah has not the Word 3*/
    /* get IPv6 Control traffic to CPU enable */
    /* set IP Control to CPU enable */
    value = U32_GET_FIELD_MAC(hwDataArr[3], 7, 1);
    if (vlanInfoPtr->ipCtrlToCpuEn == CPSS_DXCH_BRG_IP_CTRL_IPV4_E)
    {
        if (value == 1)
            vlanInfoPtr->ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E;
    }
    else
    {
        if (value == 1)
            vlanInfoPtr->ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_IPV6_E;
    }

    /* Auto Learning disable */
    value = U32_GET_FIELD_MAC(hwDataArr[3], 0, 1);
    vlanInfoPtr->autoLearnDisable = BIT2BOOL_MAC(value);

    /* Na Msg to CPU enable */
    value = U32_GET_FIELD_MAC(hwDataArr[3], 1, 1);
    vlanInfoPtr->naMsgToCpuEn = BIT2BOOL_MAC(value);

    /* MRU Index (bit5 is reserved) */
    vlanInfoPtr->mruIdx = U32_GET_FIELD_MAC(hwDataArr[3], 2, 3);

    /* BCU DP Trap Mirror enable */
    value = U32_GET_FIELD_MAC(hwDataArr[3], 6, 1);
    vlanInfoPtr->bcastUdpTrapMirrEn = BIT2BOOL_MAC(value);

    /* xCat3 and above fields */

    /* Flood VIDX */
    vlanInfoPtr->floodVidx = (GT_U16) U32_GET_FIELD_MAC(hwDataArr[3], 8, 12);

    /* Multicast Local Switching Enable */
    value = U32_GET_FIELD_MAC(hwDataArr[3], 20, 1);
    vlanInfoPtr->mcastLocalSwitchingEn = BIT2BOOL_MAC(value);

    /* Port Isolation L2 VLAN command */
    value = U32_GET_FIELD_MAC(hwDataArr[3], 21, 2);
    switch (value)
    {
        case 0:
            vlanInfoPtr->portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
            break;
        case 1:
            vlanInfoPtr->portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_CMD_E;
            break;
        case 2:
            vlanInfoPtr->portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L3_CMD_E;
            break;
        default:
            vlanInfoPtr->portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_L3_CMD_E;
    }

    /* Unicast Local Switching Enable */
    value = U32_GET_FIELD_MAC(hwDataArr[5], 17, 1);
    vlanInfoPtr->ucastLocalSwitchingEn = BIT2BOOL_MAC(value);

    /* Flood VIDX Mode */
    value = U32_GET_FIELD_MAC(hwDataArr[5], 18, 1);
    vlanInfoPtr->floodVidxMode = (value == 0) ? CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E
                                              : CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;

    maxPortNum = PRV_CPSS_PP_MAC(devNum)->numOfPorts;

    /* get ports info */
    for (ii = 0; ii < maxPortNum; ii++)
    {
        if(ii == 27)
        {
            /* port 27 data is placed in word 3 bits[23:24] */
            wordOffset = 3;
            bitOffset  = 23;
        }
        else
        {
            /* calculate first field offset in bits */
            fieldOffset = 24 + (ii * 2);    /* 24 first bits */
            wordOffset = fieldOffset >> 5;  /* / 32 */
            bitOffset  = fieldOffset & 0x1f; /* % 32 */
        }

        value = (hwDataArr[wordOffset] >> bitOffset) & 0x3;

        if (tr101Supported != GT_FALSE)
        {
            /* calculate first field offset in bits */
            fieldOffset = 25 + (ii * 2);    /* 24 first bits */
            wordOffset = 3 + (fieldOffset >> 5);  /* / 32 */
            bitOffset  = fieldOffset & 0x1f; /* % 32 */

            if (bitOffset == 31)
            {
                value1 = (hwDataArr[wordOffset] >> bitOffset);
                value1 |= ((hwDataArr[wordOffset + 1] & 1) << 1 );
            }
            else
            {
                value1 = (hwDataArr[wordOffset] >> bitOffset) & 0x3;
            }
        }

        if ((value == 0) && (value1 == 0))
        {
            continue;
        }

        /* set port information: member and tagging */
        if (value & 1)
            CPSS_PORTS_BMP_PORT_SET_MAC(portsMembersPtr,ii) ;

        if (tr101Supported != GT_FALSE)
        {
            portsTaggingCmdPtr->portsCmd[ii] = value >> 1 ;
            portsTaggingCmdPtr->portsCmd[ii] |= (value1 << 1);
        }
        else
        {
            if (value & 2)
            {
                CPSS_PORTS_BMP_PORT_SET_MAC(portsTaggingPtr,ii) ;
            }
        }
    }

    if ((hwDataArr[0] & (1 << 2)) != 0)
    {
        /* CPU port is a member of VLAN */
        CPSS_PORTS_BMP_PORT_SET_MAC(
            portsMembersPtr, CPSS_CPU_PORT_NUM_CNS);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChBrgVlanEntryWrite function
* @endinternal
*
* @brief   Builds and writes vlan entry to HW.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portsMembersPtr          - (pointer to) bitmap of physical ports members in vlan
*                                      CPU port supported
* @param[in] portsTaggingPtr          - (pointer to) bitmap of physical ports tagged in the vlan -
*                                      The parameter is relevant for xCat3 and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat3 and above devices with TR101 feature
*                                      support.
* @param[in] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[in] portsTaggingCmdPtr       - (pointer to) physical ports tagging commands in the vlan -
*                                      The parameter is relevant only for xCat3 and
*                                      above with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr, portsTaggingPtr
* @retval or vlanInfoPtr           ->stgId is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanEntryWrite
(
    IN  GT_U8                                 devNum,
    IN  GT_U16                                vlanId,
    IN  CPSS_PORTS_BMP_STC                    *portsMembersPtr,
    IN  CPSS_PORTS_BMP_STC                    *portsTaggingPtr,
    IN  CPSS_DXCH_BRG_VLAN_INFO_STC           *vlanInfoPtr,
    IN  CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr
)
{
    GT_U32      hwData[PRV_CPSS_DXCH_VLAN_ENTRY_MAX_WORDS_COUNT_CNS];
                                  /* buffer to build entry; size is max  */
                                  /* possible size in Prestera system device */
    GT_BOOL     tr101Supported;   /* TR101 Feature Support */

    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);
    CPSS_NULL_PTR_CHECK_MAC(portsMembersPtr);
    CPSS_NULL_PTR_CHECK_MAC(vlanInfoPtr);

    /* get TR101 feature support */
    tr101Supported = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.tr101Supported;

    if (tr101Supported != GT_FALSE)
    {
        /* for TR101 feature supported tagging command parameter must be valid */
        CPSS_NULL_PTR_CHECK_MAC(portsTaggingCmdPtr);
    }
    else
    {
        /* Simple port tagging */
        CPSS_NULL_PTR_CHECK_MAC(portsTaggingPtr);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        rc = vlanEntrySplitTableWrite(devNum,vlanId,
                                     portsMembersPtr,
                                     portsTaggingPtr,
                                     vlanInfoPtr,
                                     portsTaggingCmdPtr);

        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* build VLAN entry */
        rc = prvCpssBrgVlanHwEntryBuild(devNum,
                                        portsMembersPtr,
                                        portsTaggingPtr,
                                        vlanInfoPtr,
                                        portsTaggingCmdPtr,
                                        hwData);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* write VLAN entry to the VLAN Table */
        rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_TABLE_VLAN_E,
                                        (GT_U32)vlanId,/* vid */
                                        hwData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* check for Cheetah3 and above device
       xCat2 does not support VRF ID */
    if (0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        if(vlanInfoPtr->vrfId >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.vrfIdNum)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* write VRF-ID entry */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                              CPSS_DXCH3_TABLE_VRF_ID_E,
                                              (GT_U32)vlanId,
                                              0,
                                              0,
                                              12,
                                              vlanInfoPtr->vrfId);
        if (rc != GT_OK)
        {
            return rc;
        }
    }


    /* If Lion2 errata is initialized set port 14 (hemisphere 0) and
       port 64+14 (hemisphere 1) as a member in the vlan */
    if (((vlanInfoPtr->ipv4McastRouteEn == GT_TRUE)||(vlanInfoPtr->ipv6McastRouteEn == GT_TRUE)) &&
         (PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
            info_PRV_CPSS_DXCH_LION2_IPM_BRIDGE_COPY_GET_DROPPED_WA_E.enabled == GT_TRUE))
    {
        /* add port 14 to vlan */
        rc = cpssDxChBrgVlanMemberAdd(devNum, vlanId, 14, GT_FALSE, CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E);

        if(rc != GT_OK)
            return rc;

        if (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_MAC(devNum) >= 2)
        {
            /* add port 64+14to vlan */
            rc = cpssDxChBrgVlanMemberAdd(devNum, vlanId, 78, GT_FALSE, CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E);

            if(rc != GT_OK)
                return rc;
        }

    }


    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanEntryWrite function
* @endinternal
*
* @brief   Builds and writes vlan entry to HW.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portsMembersPtr          - (pointer to) bitmap of physical ports members in vlan
*                                      CPU port supported
* @param[in] portsTaggingPtr          - (pointer to) bitmap of physical ports tagged in the vlan -
*                                      The parameter is relevant for xCat3 and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat3 and above devices with TR101 feature
*                                      support.
* @param[in] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[in] portsTaggingCmdPtr       - (pointer to) physical ports tagging commands in the vlan -
*                                      The parameter is relevant only for xCat3 and
*                                      above with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr, portsTaggingPtr
* @retval or vlanInfoPtr           ->stgId is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanEntryWrite
(
    IN  GT_U8                                 devNum,
    IN  GT_U16                                vlanId,
    IN  CPSS_PORTS_BMP_STC                    *portsMembersPtr,
    IN  CPSS_PORTS_BMP_STC                    *portsTaggingPtr,
    IN  CPSS_DXCH_BRG_VLAN_INFO_STC           *vlanInfoPtr,
    IN  CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanEntryWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, portsMembersPtr, portsTaggingPtr, vlanInfoPtr, portsTaggingCmdPtr));

    rc = internal_cpssDxChBrgVlanEntryWrite(devNum, vlanId, portsMembersPtr, portsTaggingPtr, vlanInfoPtr, portsTaggingCmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, portsMembersPtr, portsTaggingPtr, vlanInfoPtr, portsTaggingCmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanEntriesRangeWrite function
* @endinternal
*
* @brief   Function set multiple vlans with the same configuration
*         function needed for performances , when the device use "indirect" access
*         to the vlan table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] vlanId                   - first VLAN Id
* @param[in] numOfEntries             - number of entries (vlans) to set
* @param[in] portsMembersPtr          - (pointer to) bitmap of physical ports members in vlan
*                                      CPU port supported
* @param[in] portsTaggingPtr          - (pointer to) bitmap of physical ports tagged in the vlan -
*                                      The parameter is relevant for xCat3 and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat3 and above devices with TR101 feature
*                                      support.
* @param[in] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[in] portsTaggingCmdPtr       - (pointer to) physical ports tagging commands in the vlan -
*                                      The parameter is relevant only for xCat3 and
*                                      above with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or vlanId
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr, portsTaggingPtr
* @retval or vlanInfoPtr           ->stgId is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanEntriesRangeWrite
(
    IN  GT_U8                                 devNum,
    IN  GT_U16                                vlanId,
    IN  GT_U32                                numOfEntries,
    IN  CPSS_PORTS_BMP_STC                    *portsMembersPtr,
    IN  CPSS_PORTS_BMP_STC                    *portsTaggingPtr,
    IN  CPSS_DXCH_BRG_VLAN_INFO_STC           *vlanInfoPtr,
    IN  CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr
)
{
    GT_U32      hwData[PRV_CPSS_DXCH_VLAN_ENTRY_MAX_WORDS_COUNT_CNS];
                                  /* buffer to build entry; size is max  */
                                  /* possible size in Prestera system device */
    GT_STATUS   rc;               /* return code*/
    GT_U32      i;                /* iterator */
    const PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    GT_BOOL     tr101Supported;   /* TR101 Feature Support */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(
        devNum, (GT_U32)((GT_U32)vlanId + (numOfEntries - 1)));
    CPSS_NULL_PTR_CHECK_MAC(portsMembersPtr);
    CPSS_NULL_PTR_CHECK_MAC(vlanInfoPtr);

    /* get TR101 feature support */
    tr101Supported = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.tr101Supported;

    if (tr101Supported != GT_FALSE)
    {
        /* for TR101 feature supported tagging command parameter must be valid */
        CPSS_NULL_PTR_CHECK_MAC(portsTaggingCmdPtr);
    }
    else
    {
        /* Simple port tagging */
        CPSS_NULL_PTR_CHECK_MAC(portsTaggingPtr);
    }

    if (numOfEntries == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        for(i = 0; i < numOfEntries; i++)
        {
            rc = vlanEntrySplitTableWrite(devNum,(GT_U16)(vlanId + i),
                                         portsMembersPtr,
                                         portsTaggingPtr,
                                         vlanInfoPtr,
                                         portsTaggingCmdPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        return GT_OK;
    }
    else
    {
        /* build VLAN entry */
        rc = prvCpssBrgVlanHwEntryBuild(devNum,
                                        portsMembersPtr,
                                        portsTaggingPtr,
                                        vlanInfoPtr,
                                        portsTaggingCmdPtr,
                                        hwData);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* write VLAN entry to the VLAN Table */
        rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_TABLE_VLAN_E,
                                        (GT_U32)vlanId,/* vid */
                                        hwData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* check for Cheetah3 and above device
       xCat2 does not support VRF ID */
    if (0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        if(vlanInfoPtr->vrfId >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.vrfIdNum)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* write VRF-ID entry */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                              CPSS_DXCH3_TABLE_VRF_ID_E,
                                              (GT_U32)vlanId,
                                              0,
                                              0,
                                              12,
                                              vlanInfoPtr->vrfId);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    tableInfoPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->accessTableInfoPtr[CPSS_DXCH_TABLE_VLAN_E]);

     /* Write the above entry to all vlans */
    for(i = 1; i < numOfEntries; i++)
    {
        if(tableInfoPtr->writeAccessType == PRV_CPSS_DXCH_DIRECT_ACCESS_E)
        {
            /* write full vlan entry */
            rc = prvCpssDxChWriteTableEntry(devNum,
                                    CPSS_DXCH_TABLE_VLAN_E,
                                    (vlanId + i),
                                    hwData);
        }
        else
        {
            /*
                NOTE: the multi-port-groups device not need special case ,
                      because in indirect access , we need to loop on all port groups
                      without any change of 'data' , only 'control' changes.
                      and this loop on port groups will be done inside the cpssDriver
                      called from prvCpssDxChWriteTableEntry(...)
            */


            /* The indirect access implementation use NULL pointer to entry as
               sign to write only to control word. This result in all VLAN entries
               get same data written above.  */
            rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_TABLE_VLAN_E,
                                        (vlanId + i),
                                        NULL);
        }

        if (rc != GT_OK)
        {
            return rc;
        }

        /* check for Cheetah3 and above device
           xCat2 does not support VRF ID */
        if (0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
        {
            /* write VRF-ID entry */
            rc = prvCpssDxChWriteTableEntryField(devNum,
                                                  CPSS_DXCH3_TABLE_VRF_ID_E,
                                                  (vlanId + i),
                                                  0,
                                                  0,
                                                  12,
                                                  vlanInfoPtr->vrfId);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanEntriesRangeWrite function
* @endinternal
*
* @brief   Function set multiple vlans with the same configuration
*         function needed for performances , when the device use "indirect" access
*         to the vlan table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] vlanId                   - first VLAN Id
* @param[in] numOfEntries             - number of entries (vlans) to set
* @param[in] portsMembersPtr          - (pointer to) bitmap of physical ports members in vlan
*                                      CPU port supported
* @param[in] portsTaggingPtr          - (pointer to) bitmap of physical ports tagged in the vlan -
*                                      The parameter is relevant for xCat3 and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat3 and above devices with TR101 feature
*                                      support.
* @param[in] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[in] portsTaggingCmdPtr       - (pointer to) physical ports tagging commands in the vlan -
*                                      The parameter is relevant only for xCat3 and
*                                      above with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or vlanId
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr, portsTaggingPtr
* @retval or vlanInfoPtr           ->stgId is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanEntriesRangeWrite
(
    IN  GT_U8                                 devNum,
    IN  GT_U16                                vlanId,
    IN  GT_U32                                numOfEntries,
    IN  CPSS_PORTS_BMP_STC                    *portsMembersPtr,
    IN  CPSS_PORTS_BMP_STC                    *portsTaggingPtr,
    IN  CPSS_DXCH_BRG_VLAN_INFO_STC           *vlanInfoPtr,
    IN  CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanEntriesRangeWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, numOfEntries, portsMembersPtr, portsTaggingPtr, vlanInfoPtr, portsTaggingCmdPtr));

    rc = internal_cpssDxChBrgVlanEntriesRangeWrite(devNum, vlanId, numOfEntries, portsMembersPtr, portsTaggingPtr, vlanInfoPtr, portsTaggingCmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, numOfEntries, portsMembersPtr, portsTaggingPtr, vlanInfoPtr, portsTaggingCmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanEntryRead function
* @endinternal
*
* @brief   Read vlan entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
*
* @param[out] portsMembersPtr          - (pointer to) bitmap of physical ports members in vlan
*                                      CPU port supported
* @param[out] portsTaggingPtr          - (pointer to) bitmap of physical ports tagged in the vlan -
*                                      The parameter is relevant for xCat3 and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat3 and above devices with TR101 feature
*                                      support.
* @param[out] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[out] isValidPtr               - (pointer to) VLAN entry status
*                                      GT_TRUE = Vlan is valid
*                                      GT_FALSE = Vlan is not Valid
* @param[out] portsTaggingCmdPtr       - (pointer to) physical ports tagging commands in the vlan -
*                                      The parameter is relevant only for xCat3 and
*                                      above with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanEntryRead
(
    IN  GT_U8                                devNum,
    IN  GT_U16                               vlanId,
    OUT CPSS_PORTS_BMP_STC                   *portsMembersPtr,
    OUT CPSS_PORTS_BMP_STC                   *portsTaggingPtr,
    OUT CPSS_DXCH_BRG_VLAN_INFO_STC          *vlanInfoPtr,
    OUT GT_BOOL                              *isValidPtr,
    OUT CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC *portsTaggingCmdPtr
)
{
    GT_U32  hwData[PRV_CPSS_DXCH_VLAN_ENTRY_MAX_WORDS_COUNT_CNS];
                                    /* buffer to build entry; size is max     */
                                    /* possible size in Prestera system device*/
    GT_BOOL     tr101Supported;   /* TR101 Feature Support */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);
    CPSS_NULL_PTR_CHECK_MAC(portsMembersPtr);
    CPSS_NULL_PTR_CHECK_MAC(vlanInfoPtr);
    CPSS_NULL_PTR_CHECK_MAC(isValidPtr);

    /* get TR101 feature support */
    tr101Supported = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.tr101Supported;

    if (tr101Supported != GT_FALSE)
    {
        /* for TR101 feature supported tagging command parameter must be valid */
        CPSS_NULL_PTR_CHECK_MAC(portsTaggingCmdPtr);
    }
    else
    {
        /* Simple port tagging */
        CPSS_NULL_PTR_CHECK_MAC(portsTaggingPtr);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        rc = vlanEntrySplitTableRead(devNum,vlanId,
                                     portsMembersPtr,
                                     portsTaggingPtr,
                                     vlanInfoPtr,
                                     isValidPtr,
                                     portsTaggingCmdPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* do not call this , as the CPU port not need the manipulation */
        }
        else
        {
            /* CPU port bitmap conversion */
            rc = prvCpssDxChBrgCpuPortBmpConvert(devNum,
                                                 GT_FALSE,
                                                 portsMembersPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    {
        /* read entry from HW */
        rc = prvCpssDxChReadTableEntry(devNum,
                                       CPSS_DXCH_TABLE_VLAN_E,
                                       (GT_U32)vlanId,/* vid */
                                       hwData);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssBrgVlanHwEntryParse(devNum,
                                        hwData,
                                        portsMembersPtr,
                                        portsTaggingPtr,
                                        vlanInfoPtr,
                                        isValidPtr,
                                        portsTaggingCmdPtr);

        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* check for Cheetah3 and above device
       xCat2 does not support VRF ID */
    if (0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* read VRF-ID entry */
        return prvCpssDxChReadTableEntryField(devNum,
                                              CPSS_DXCH3_TABLE_VRF_ID_E,
                                              (GT_U32)vlanId,
                                              0,
                                              0,
                                              12,
                                              &(vlanInfoPtr->vrfId));
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanEntryRead function
* @endinternal
*
* @brief   Read vlan entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
*
* @param[out] portsMembersPtr          - (pointer to) bitmap of physical ports members in vlan
*                                      CPU port supported
* @param[out] portsTaggingPtr          - (pointer to) bitmap of physical ports tagged in the vlan -
*                                      The parameter is relevant for xCat3 and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat3 and above devices with TR101 feature
*                                      support.
* @param[out] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[out] isValidPtr               - (pointer to) VLAN entry status
*                                      GT_TRUE = Vlan is valid
*                                      GT_FALSE = Vlan is not Valid
* @param[out] portsTaggingCmdPtr       - (pointer to) physical ports tagging commands in the vlan -
*                                      The parameter is relevant only for xCat3 and
*                                      above with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanEntryRead
(
    IN  GT_U8                                devNum,
    IN  GT_U16                               vlanId,
    OUT CPSS_PORTS_BMP_STC                   *portsMembersPtr,
    OUT CPSS_PORTS_BMP_STC                   *portsTaggingPtr,
    OUT CPSS_DXCH_BRG_VLAN_INFO_STC          *vlanInfoPtr,
    OUT GT_BOOL                              *isValidPtr,
    OUT CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC *portsTaggingCmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanEntryRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, portsMembersPtr, portsTaggingPtr, vlanInfoPtr, isValidPtr, portsTaggingCmdPtr));

    rc = internal_cpssDxChBrgVlanEntryRead(devNum, vlanId, portsMembersPtr, portsTaggingPtr, vlanInfoPtr, isValidPtr, portsTaggingCmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, portsMembersPtr, portsTaggingPtr, vlanInfoPtr, isValidPtr, portsTaggingCmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal vlanMemberSet function
* @endinternal
*
* @brief   set port as vlan member/not and it's tag mode.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portNum                  - port number
* @param[in] isMember                 - is port member of the vlan
* @param[in] isTagged                 - GT_TRUE, to set the port as tagged member,
*                                      GT_FALSE, to set the port as untagged
*                                      The parameter is relevant for xCat3 and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat3 and above devices with TR101 feature
*                                      support.
* @param[in] portTaggingCmd           - port tagging command
*                                      The parameter is relevant only for xCat3 and above
*                                      with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vid or portTaggingCmd
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS vlanMemberSet
(
    IN  GT_U8                   devNum,
    IN  GT_U16                  vlanId,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 isMember,
    IN  GT_BOOL                 isTagged,
    IN  CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT portTaggingCmd
)
{
    GT_U32      hwData;             /* buffer to build part entry          */
    GT_STATUS   rc;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT currTagCmd;
    GT_U32      hwDataTagCmd = 0;       /* hw format for currTagCmd */
    GT_BOOL     tr101Supported;   /* TR101 Feature Support */
    CPSS_PORTS_BMP_STC portsMembersInHemisphere;    /* VLAN port members in current hemisphere
                                                       before adding the additional port */
    GT_U32      portGroupInOtherHemishpere;         /* port group which resides in the other hemisphere */
    GT_BOOL     needToModifyOtherHemisphere;        /* weather port 12 of other hemisphere need to be modified */
    GT_U32      value;                              /* value to write to hardware */
    GT_U32      cfgOrderArr[3];                     /* order of configuration steps */
    GT_U32      cfgStepIdx;                         /* configuration steps index    */
    union
    {
        GT_U32 egrVlanEntryBufArr[VLAN_EGRESS_ENTRY_WORDS_NUM_CNS];
        GT_U32 ingrVlanEntryBufArr[VLAN_INGRESS_ENTRY_WORDS_NUM_CNS];
    } hwTabEntryBufArr;                             /* buffer for table entry       */
    GT_U32      fieldsAmount;                       /* amount of fields             */
    GT_U32      fieldOffsetArr[2];                  /* array of field offsets       */
    GT_U32      fieldLengthArr[2];                  /* array of field lengths       */
    GT_U32      fieldValueArr[2];                   /* array of field values        */
    GT_BOOL     isCpu;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);
    PRV_CPSS_DXCH_PHY_PORT_RESERVED_CHECK_MAC(devNum, portNum);

    /* get TR101 feature support */
    tr101Supported = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.tr101Supported;

    rc = cpssDxChPortPhysicalPortMapIsCpuGet(devNum, portNum, &isCpu);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (tr101Supported == GT_FALSE)
    {
        currTagCmd = (isTagged == GT_TRUE) ? CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E :
            CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    }
    else
    {
        currTagCmd = portTaggingCmd;
    }

    if (isCpu == GT_FALSE)
    {
        PRV_CPSS_DXCH_CONVERT_VLAN_TAG_MODE_TO_HW_VAL_MAC(devNum, hwDataTagCmd,currTagCmd);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);

        if (isMember == GT_FALSE)
        {
            /* removing port */
            cfgOrderArr[0] = 0;  /* ingress VLAN membership */
            cfgOrderArr[1] = 1;  /* egress VLAN membership  */
            cfgOrderArr[2] = 2;  /* VLAN command            */
        }
        else
        {
            /* adding port */
            cfgOrderArr[0] = 2;  /* VLAN command            */
            cfgOrderArr[1] = 1;  /* egress VLAN membership  */
            cfgOrderArr[2] = 0;  /* ingress VLAN membership */
        }

        for (cfgStepIdx = 0; (cfgStepIdx < 3); cfgStepIdx++)
        {
            switch (cfgOrderArr[cfgStepIdx])
            {
                case 0:
                    /* the ingress vlan entry looks the same on all port groups */
                    /* set the 'port members' in the Ingress Bridge Port Membership table */
                    rc = prvCpssDxChWriteTableEntryField(
                        devNum,
                        CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E,
                        vlanId,
                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,/* global start bit */
                        portNum, /* bit index */
                        1,/* single bit */
                        BOOL2BIT_MAC(isMember));
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    break;

                case 1:
                    /* the SHT egress vlan entry looks the same on all port groups */

                    /* set the 'sub field' <port Member> in the field of <PORT_X_MEMBER>*/
                    rc = prvCpssDxChWriteTableEntryField(
                        devNum,
                        CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
                        vlanId,
                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_SUB_FIELD_IN_FIELD_NAME_CNS,/* use of sub field ! */
                        SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS_PORT_X_MEMBER_E, /* field name */
                        PRV_CPSS_DXCH_TABLE_SUB_FIELD_OFFSET_AND_LENGTH_MAC(portNum,1),/* sub field info {offset,length}*/
                        BOOL2BIT_MAC(isMember));
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    break;

                case 2:
                    /* the QAG egress vlan entry looks the same on all hemispheres */

                    /* set the <egress tag mode> for the proper port */
                    rc = prvCpssDxChWriteTableEntryField(
                        devNum,
                        CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E,
                        vlanId,
                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,/* global bit */
                        3*portNum, /* start bit */
                        3,/* number of bits*/
                        hwDataTagCmd);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }

        return GT_OK;
    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        if (portNum == CPSS_CPU_PORT_NUM_CNS)
        {
            portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
            fieldsAmount      = 1;
            fieldOffsetArr[0] = 2;
            fieldLengthArr[0] = 1;
            fieldValueArr[0]  = BOOL2BIT_MAC(isMember);
        }
        else
        {
            PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);

            /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
            portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
            localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);
            /* calculate the word number and the bit offset to read */
            if (localPort < 27)
            {
                /* calculate first field offset in bits */
                fieldOffsetArr[0] = 24 + (2 * localPort);   /* 24 first byte */
            }
            else
            {
                /* port 27 data is placed in word 3 bits[23:24] */
                fieldOffsetArr[0] = 119;
            }

            /* build the data */
            hwData = BOOL2BIT_MAC(isMember);
            if (tr101Supported != GT_FALSE)
            {
                hwData |= ((hwDataTagCmd & 1) << 1) ;
            }
            else
            {
                hwData |= ((BOOL2BIT_MAC(isTagged)) << 1);
            }
            fieldLengthArr[0] = 2;
            fieldValueArr[0]  = hwData;
            fieldsAmount      = 1;

            if (tr101Supported != GT_FALSE)
            {
                /* build the data */
                hwData = (hwDataTagCmd >> 1);

                /* calculate first field offset in bits */
                fieldOffsetArr[1] = 121 + (2 * localPort);
                fieldLengthArr[1] = 2;
                fieldValueArr[1]  = hwData;
                fieldsAmount      = 2;
            }
        }

        return prvCpssDxChPortGroupWriteTableEntryFieldList(
            devNum, portGroupId,
            CPSS_DXCH_TABLE_VLAN_E,
            (GT_U32)vlanId, (GT_U32*)&hwTabEntryBufArr,
            fieldsAmount, fieldOffsetArr, fieldLengthArr, fieldValueArr);
    }
    else
    {
        if (CPSS_CPU_PORT_NUM_CNS == portNum)
        {
            /* CPU port conversion - set actual CPU port */

            /* CPU port group id  */
            portGroupId = (PRV_CPSS_PP_MAC(devNum)->cpuPortMode !=
                          CPSS_NET_CPU_PORT_MODE_SDMA_E) ?
                          PRV_CPSS_CPU_PORT_PORT_GROUP_ID_MAC(devNum) :
                          PRV_CPSS_NETIF_SDMA_PORT_GROUP_ID_MAC(devNum);

            portNum = (portGroupId * 16 + 15);
        }
        else
        {
            portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portNum);
        }


        portNum = PRV_CPSS_DXCH_GLOBAL_TO_HEM_LOCAL_PORT(devNum,portNum);

        CPSS_TBD_BOOKMARK_EARCH
        if(portNum > 63 /*PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PHY_PORT_MAC(devNum)*/)
        {
            /* eArch not defined vlan entry format that supports 128 ports*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* set port as member in ingress table */
        rc = prvCpssDxChPortGroupWriteTableEntryField(
            devNum, portGroupId,
            CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
            (GT_U32)vlanId,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            2 + portNum,
            1, /* 1 bit */
            BOOL2BIT_MAC(isMember));
        if(rc != GT_OK)
        {
            return rc;
        }

        /* set port as member in egress table */
        fieldOffsetArr[0] = (1 + portNum);
        fieldLengthArr[0] = 1;
        fieldValueArr[0]  = BOOL2BIT_MAC(isMember);
        fieldsAmount      = 1;
        if(portNum != CPSS_CPU_PORT_NUM_CNS)
        {
            /* set port tagCmp */
            fieldOffsetArr[1] = (65 + (3 * portNum));
            fieldLengthArr[1] = 3;
            fieldValueArr[1]  = hwDataTagCmd;
            fieldsAmount      = 2;
        }
        rc = prvCpssDxChPortGroupWriteTableEntryFieldList(
            devNum, portGroupId,
            CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
            (GT_U32)vlanId, (GT_U32*)&hwTabEntryBufArr,
            fieldsAmount, fieldOffsetArr, fieldLengthArr, fieldValueArr);
        if(rc != GT_OK)
        {
            return rc;
        }

        if (GT_FALSE != PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                info_PRV_CPSS_DXCH_LION2_MC_BANDWIDTH_RESTRICTION_WA_E.enabled)
        {
            /* read current VLAN members in the current hemishpere */
            rc = vlanEntrySplitTableMembersInHemisphereGet(
                devNum, vlanId,
                (portGroupId / PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS), /* current hemisphere */
                &portsMembersInHemisphere);
            if (rc != GT_OK)
                return rc;
            /* don't take into account port 12 which is used to send traffic to other hemisphere */
            CPSS_PORTS_BMP_PORT_CLEAR_MAC(&portsMembersInHemisphere,12);

            needToModifyOtherHemisphere = GT_FALSE;
            if (isMember == GT_TRUE) /* adding member */
            {
                /* check if was is the first member added to this hemisphere */
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&portsMembersInHemisphere,portNum); /* remove it from the list */
                if ((portsMembersInHemisphere.ports[0] == 0) && (portsMembersInHemisphere.ports[1] == 0))
                {
                    needToModifyOtherHemisphere = GT_TRUE;
                }
            }
            else /* deleting member */
            {
                /* check if deleted member was the last in current hemisphere */
                if ((portsMembersInHemisphere.ports[0] == 0) && (portsMembersInHemisphere.ports[1] == 0))
                {
                    needToModifyOtherHemisphere = GT_TRUE;
                }
            }

            /* update port 12 of the other hemisphere if needed */
            if (needToModifyOtherHemisphere == GT_TRUE)
            {
                /* any portGroupId in the same hemisphere adresses the same VLAN tables */
                portGroupInOtherHemishpere =
                    (portGroupId >= PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS)
                        ? (portGroupId - PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS)
                        : (portGroupId + PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS);
                /* to make port 12 of other hemisphere VLAN member "if end only if" */
                /* the VLAN contains members in the current hemisphere              */
                value = (isMember == GT_TRUE) ? 0 : 1; /* port 12 is with reverse logic */

                /* set port as member in ingress table */
                rc = prvCpssDxChPortGroupWriteTableEntryField(devNum, portGroupInOtherHemishpere,
                                                      CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                                      (GT_U32)vlanId,
                                                      PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                                      2 + 12,
                                                      1, /* 1 bit */
                                                      value);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* set port as member in egress table */
                rc = prvCpssDxChPortGroupWriteTableEntryField(devNum, portGroupInOtherHemishpere,
                                                      CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
                                                      (GT_U32)vlanId,
                                                      PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                                      1 + 12,
                                                      1, /* 1 bit */
                                                      value);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChBrgVlanMemberAdd function
* @endinternal
*
* @brief   Add new port member to vlan entry. This function can be called only for
*         add port belongs to device that already member of the vlan.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portNum                  - port number
*                                      CPU port supported
* @param[in] isTagged                 - GT_TRUE, to set the port as tagged member,
*                                      GT_FALSE, to set the port as untagged
*                                      The parameter is relevant for xCat3 and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat3 and above devices with TR101 feature
*                                      support.
* @param[in] portTaggingCmd           - port tagging command
*                                      The parameter is relevant only for xCat3 and above
*                                      with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vid or portTaggingCmd
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In case that added port belongs to device that is new device for vlan
*       other function is used - cpssDxChBrgVlanEntryWrite.
*
*/
static GT_STATUS internal_cpssDxChBrgVlanMemberAdd
(
    IN  GT_U8                   devNum,
    IN  GT_U16                  vlanId,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 isTagged,
    IN  CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT portTaggingCmd
)
{
    return vlanMemberSet(devNum,vlanId,portNum,GT_TRUE,isTagged,portTaggingCmd);
}

/**
* @internal cpssDxChBrgVlanMemberAdd function
* @endinternal
*
* @brief   Add new port member to vlan entry. This function can be called only for
*         add port belongs to device that already member of the vlan.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portNum                  - port number
*                                      CPU port supported
* @param[in] isTagged                 - GT_TRUE, to set the port as tagged member,
*                                      GT_FALSE, to set the port as untagged
*                                      The parameter is relevant for xCat3 and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat3 and above devices with TR101 feature
*                                      support.
* @param[in] portTaggingCmd           - port tagging command
*                                      The parameter is relevant only for xCat3 and above
*                                      with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vid or portTaggingCmd
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In case that added port belongs to device that is new device for vlan
*       other function is used - cpssDxChBrgVlanEntryWrite.
*
*/
GT_STATUS cpssDxChBrgVlanMemberAdd
(
    IN  GT_U8                   devNum,
    IN  GT_U16                  vlanId,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 isTagged,
    IN  CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT portTaggingCmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanMemberAdd);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, portNum, isTagged, portTaggingCmd));

    rc = internal_cpssDxChBrgVlanMemberAdd(devNum, vlanId, portNum, isTagged, portTaggingCmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, portNum, isTagged, portTaggingCmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgVlanPortDelete function
* @endinternal
*
* @brief   Delete port member from vlan entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portNum                  - port number
*                                      CPU port supported
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortDelete
(
    IN GT_U8                    devNum,
    IN GT_U16                   vlanId,
    IN GT_PHYSICAL_PORT_NUM     portNum
)
{
    return vlanMemberSet(devNum,vlanId,portNum,GT_FALSE,GT_FALSE,
                        CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E);

}

/**
* @internal cpssDxChBrgVlanPortDelete function
* @endinternal
*
* @brief   Delete port member from vlan entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portNum                  - port number
*                                      CPU port supported
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortDelete
(
    IN GT_U8                    devNum,
    IN GT_U16                   vlanId,
    IN GT_PHYSICAL_PORT_NUM     portNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortDelete);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, portNum));

    rc = internal_cpssDxChBrgVlanPortDelete(devNum, vlanId, portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanMemberSet function
* @endinternal
*
* @brief   Set specific member at VLAN entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portNum                  - port number
*                                      CPU port supported
* @param[in] isMember                 - GT_TRUE,  port would set as member
*                                      GT_FALSE, port would set as not-member
* @param[in] isTagged                 - GT_TRUE, to set the port as tagged member,
*                                      GT_FALSE, to set the port as untagged
*                                      The parameter is relevant for xCat3 and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat3 and above devices with TR101 feature
*                                      support.
* @param[in] taggingCmd               - port tagging command
*                                      The parameter is relevant only for xCat3 and above
*                                      with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or taggingCmd
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanMemberSet
(
    IN  GT_U8                   devNum,
    IN  GT_U16                  vlanId,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 isMember,
    IN  GT_BOOL                 isTagged,
    IN  CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT taggingCmd
)
{
    return vlanMemberSet(devNum,vlanId,portNum,isMember,isTagged,taggingCmd);
}

/**
* @internal cpssDxChBrgVlanMemberSet function
* @endinternal
*
* @brief   Set specific member at VLAN entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portNum                  - port number
*                                      CPU port supported
* @param[in] isMember                 - GT_TRUE,  port would set as member
*                                      GT_FALSE, port would set as not-member
* @param[in] isTagged                 - GT_TRUE, to set the port as tagged member,
*                                      GT_FALSE, to set the port as untagged
*                                      The parameter is relevant for xCat3 and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat3 and above devices with TR101 feature
*                                      support.
* @param[in] taggingCmd               - port tagging command
*                                      The parameter is relevant only for xCat3 and above
*                                      with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or taggingCmd
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanMemberSet
(
    IN  GT_U8                   devNum,
    IN  GT_U16                  vlanId,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 isMember,
    IN  GT_BOOL                 isTagged,
    IN  CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT taggingCmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanMemberSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, portNum, isMember, isTagged, taggingCmd));

    rc = internal_cpssDxChBrgVlanMemberSet(devNum, vlanId, portNum, isMember, isTagged, taggingCmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, portNum, isMember, isTagged, taggingCmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal ingressPortVidGet function
* @endinternal
*
* @brief   Get port's default VLAN Id. - ingress.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] vidPtr                   - default VLAN ID.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS ingressPortVidGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U16          *vidPtr
)
{
    GT_STATUS   rc;
    GT_U32      startWord; /* the table word at which the field starts */
    GT_U32      startBit;  /* the word's bit at which the field starts */
    GT_U32      hwData;    /* data to write to Hw */

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read from pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_P_EVLAN_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &hwData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

        startWord = 0;
        startBit  = 15;

        /* read from port-vlan-qos table  */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                        portNum,/* port */
                                        startWord,
                                        startBit,
                                        12, /* 12 bits */
                                        &hwData);
    }

    *vidPtr = (GT_U16)hwData;

    return rc;

}

/**
* @internal egressPortVidSet function
* @endinternal
*
* @brief   Set VID0 assigned to the egress packet if <VID0 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] vid0                     - VID0 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS egressPortVidSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U16           vid0
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    PRV_CPSS_DXCH_VLAN_VALUE_CHECK_MAC(devNum, vid0);

    /* set EGF_QAG unit */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EVLAN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        vid0);
    return rc;

}

/**
* @internal egressPortVidGet function
* @endinternal
*
* @brief   Return the VID0 assigned to the egress packet if <VID0 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] vid0Ptr                  - (pointer to) VID0 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS egressPortVidGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U16          *vid0Ptr
)
{
    GT_STATUS   rc;     /* function return code              */
    GT_U32      value;  /* value to write                    */

    /* check parameters */

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EVLAN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);

    *vid0Ptr = (GT_U16)value;

    return rc;
}


/**
* @internal internal_cpssDxChBrgVlanPortVidGet function
* @endinternal
*
* @brief   For ingress direction : Get port's default VID0.
*         For egress direction : Get port's VID0 when egress port <VID0 Command>=Enabled
*         APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*         APPLICABLE DEVICES:
*         xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - device number
*         portNum - port number
*         direction - ingress/egress direction
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] direction                - ingress/egress direction
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[out] vidPtr                   - default VLAN ID.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or port or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortVidGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN CPSS_DIRECTION_ENT          direction,
    OUT  GT_U16          *vidPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(vidPtr);

    if(GT_FALSE == PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        direction = CPSS_DIRECTION_INGRESS_E;
    }

    switch(direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            return ingressPortVidGet(devNum,portNum,vidPtr);

        case CPSS_DIRECTION_EGRESS_E:
            return egressPortVidGet(devNum,portNum,vidPtr);
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal cpssDxChBrgVlanPortVidGet function
* @endinternal
*
* @brief   For ingress direction : Get port's default VID0.
*         For egress direction : Get port's VID0 when egress port <VID0 Command>=Enabled
*         APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*         APPLICABLE DEVICES:
*         xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - device number
*         portNum - port number
*         direction - ingress/egress direction
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] direction                - ingress/egress direction
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[out] vidPtr                   - default VLAN ID.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or port or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortVidGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN CPSS_DIRECTION_ENT          direction,
    OUT  GT_U16          *vidPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortVidGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, vidPtr));

    rc = internal_cpssDxChBrgVlanPortVidGet(devNum, portNum, direction, vidPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, vidPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal multiPortGroupDebugBrgVlanPortVidSet function
* @endinternal
*
* @brief   Set port's default VLAN Id.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] vlanId                   - VLAN Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum port, or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS multiPortGroupDebugBrgVlanPortVidSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_U16          vlanId
)
{
    GT_STATUS   rc;
    GT_U32  ii,jj; /*iterators*/
    GT_U32  globalPort,tmpPort;/* uplink global port , tmp interconnection port */


    /* in this function we assume that application will set all the uplinks with
       the same PVID , so for a particular uplink we set only it's interconnection
       ports , and not all interconnection ports (when multi-uplinks) */


    /* check if this port is one of the uplink ports ,if so */
    /* loop on all it's interconnection ports , and set them with the same PVID */
    for(ii = 0 ;
        ii < PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_INFO_PTR_MAC(devNum)->numOfUplinkPorts;
        ii++)
    {
        globalPort = PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum,
            PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_INFO_PTR_MAC(devNum)->uplinkInfoArr[ii].uplinkPortGroupPortInfo.portGroupId,
            PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_INFO_PTR_MAC(devNum)->uplinkInfoArr[ii].uplinkPortGroupPortInfo.localPort);
        if(globalPort != portNum)
        {
            continue;
        }

        /* the PVID on this port need to be set on other ports as well */
        for(jj = 0 ;
            jj < PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_INFO_PTR_MAC(devNum)->uplinkInfoArr[ii].numberOfInternalConnections;
            jj++)
        {
            /*convert localPort, portGroupId to global port */
            tmpPort = PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum,
                PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_INFO_PTR_MAC(devNum)->uplinkInfoArr[ii].internalConnectionsArr[jj].portGroupId,
                PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_INFO_PTR_MAC(devNum)->uplinkInfoArr[ii].internalConnectionsArr[jj].localPort);

            rc = cpssDxChBrgVlanPortVidSet(devNum,(GT_U8)tmpPort,CPSS_DIRECTION_INGRESS_E,vlanId);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        /* we handled the 'Interconnection ports' of this uplink ,
           so no more needed */
        break;
    }

    return GT_OK;
}

/**
* @internal ingressPortVidSet function
* @endinternal
*
* @brief   Set port's default VLAN Id. (ingress)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] vlanId                   - VLAN Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum port, or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS ingressPortVidSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_U16          vlanId
)
{
    GT_STATUS   rc;
    GT_U32      startWord; /* the table word at which the field starts */
    GT_U32      startBit;  /* the word's bit at which the field starts */
    GT_U32      hwData;    /* data to write to Hw */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_VALUE_CHECK_MAC(devNum, vlanId);

    hwData = vlanId;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to pre-tti-lookup-ingress-eport table */
         rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_P_EVLAN_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            hwData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

        startWord = 0;
        startBit  = 15;

        /* write to port-vlan-qos table  */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                         portNum,/* port */
                                         startWord,
                                         startBit,
                                         12, /* 12 bits */
                                         hwData);

        if(rc == GT_OK &&
           PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_INFO_PTR_MAC(devNum)->debugInfoValid == GT_TRUE)
        {
            rc = multiPortGroupDebugBrgVlanPortVidSet(devNum,portNum,vlanId);
        }
    }
    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortVidSet function
* @endinternal
*
* @brief   For ingress direction : Set port's default VID0.
*         For egress direction : Set port's VID0 when egress port <VID0 Command>=Enabled
*         APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*         APPLICABLE DEVICES:
*         xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - device number
*         portNum - port number
*         direction - ingress/egress direction
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] direction                - ingress/egress direction
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @param[in] vlanId                   - VLAN Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum port, or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortVidSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN CPSS_DIRECTION_ENT          direction,
    IN  GT_U16          vlanId
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if(GT_FALSE == PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        direction = CPSS_DIRECTION_INGRESS_E;
    }

    switch(direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            return ingressPortVidSet(devNum,portNum,vlanId);

        case CPSS_DIRECTION_EGRESS_E:
            return egressPortVidSet(devNum,portNum,vlanId);
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal cpssDxChBrgVlanPortVidSet function
* @endinternal
*
* @brief   For ingress direction : Set port's default VID0.
*          For egress direction  : Set port's VID0 when egress port <VID0 Command>=Enabled
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] direction                - ingress/egress direction
*                                      APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @param[in] vlanId                   - VLAN Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum port, or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortVidSet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    IN CPSS_DIRECTION_ENT          direction,
    IN  GT_U16          vlanId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortVidSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, vlanId));

    rc = internal_cpssDxChBrgVlanPortVidSet(devNum, portNum, direction, vlanId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, vlanId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgVlanPortIngFltEnable function
* @endinternal
*
* @brief   Enable/disable Ingress Filtering for specific port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE,  ingress filtering
*                                      GT_FALSE, disable ingress filtering
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortIngFltEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS   rc;
    GT_U32      val;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    val = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {

        /* set the <VLAN_INGRESS_FILTERING>*/
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                       CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_EPORT_TABLE_FIELDS_VLAN_INGRESS_FILTERING_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        val);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        rc = prvCpssHwPpPortGroupSetRegField(devNum,
                                                  PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum),
                                                  PRV_CPSS_DEV_BRG_PORT_CONFIG_0_REGADDR_PTR(devNum,portNum),
                                                  21,
                                                  1,
                                                  val);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortIngFltEnable function
* @endinternal
*
* @brief   Enable/disable Ingress Filtering for specific port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE,  ingress filtering
*                                      GT_FALSE, disable ingress filtering
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortIngFltEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortIngFltEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgVlanPortIngFltEnable(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortIngFltEnableGet function
* @endinternal
*
* @brief   Get status of Ingress Filtering for specific port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - Pointer to Ingress Filtering status
*                                      - GT_TRUE, ingress filtering is enabled
*                                      - GT_FALSE, ingress filtering is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortIngFltEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32      val;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {

        /* get the <VLAN_INGRESS_FILTERING>*/
        rc = prvCpssDxChReadTableEntryField(devNum,
                                       CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_EPORT_TABLE_FIELDS_VLAN_INGRESS_FILTERING_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &val);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        rc = prvCpssHwPpPortGroupGetRegField(devNum,
                                                  PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum),
                                                  PRV_CPSS_DEV_BRG_PORT_CONFIG_0_REGADDR_PTR(devNum,portNum),
                                                  21,
                                                  1,
                                                  &val);
    }

    *enablePtr = BIT2BOOL_MAC(val);
    return rc;
}

/**
* @internal cpssDxChBrgVlanPortIngFltEnableGet function
* @endinternal
*
* @brief   Get status of Ingress Filtering for specific port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - Pointer to Ingress Filtering status
*                                      - GT_TRUE, ingress filtering is enabled
*                                      - GT_FALSE, ingress filtering is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortIngFltEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortIngFltEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgVlanPortIngFltEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortProtoClassVlanEnable function
* @endinternal
*
* @brief   Enable/Disable Port Protocol VLAN assignment to packets received
*         on this port,according to it's Layer2 protocol
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE for enable, otherwise GT_FALSE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortProtoClassVlanEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_U32      hwData;     /* data to write into register */
    GT_U32      startWord;  /* the table word at which the field starts */
    GT_U32      startBit;   /* the word's bit at which the field starts */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    hwData = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to pre-tti-lookup-ingress-eport table */
         rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PROT_BASED_VLAN_EN_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            hwData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

        startWord = 0;
        startBit  = 30;

        /* write to port-vlan-qos table  */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,/* port */
                                             startWord,
                                             startBit,
                                             1, /* 1 bit */
                                             hwData);
    }
    return rc;
}

/**
* @internal cpssDxChBrgVlanPortProtoClassVlanEnable function
* @endinternal
*
* @brief   Enable/Disable Port Protocol VLAN assignment to packets received
*         on this port,according to it's Layer2 protocol
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE for enable, otherwise GT_FALSE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortProtoClassVlanEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortProtoClassVlanEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgVlanPortProtoClassVlanEnable(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortProtoClassVlanEnableGet function
* @endinternal
*
* @brief   Get status of Port Protocol VLAN assignment to packets received
*         on this port,according to it's Layer2 protocol
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - GT_TRUE for enable, otherwise GT_FALSE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChBrgVlanPortProtoClassVlanEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
)
{
    GT_U32      hwData;     /* register data  */
    GT_U32      startWord;  /* the table word at which the field starts */
    GT_U32      startBit;   /* the word's bit at which the field starts */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read from pre-tti-lookup-ingress-eport table */
         rc = prvCpssDxChReadTableEntryField(devNum,
                                             CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                             portNum,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                             SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PROT_BASED_VLAN_EN_E, /* field name */
                                             PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &hwData);
         if(GT_OK != rc)
         {
             return rc;
         }
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

        startWord = 0;
        startBit  = 30;

        /* write to port-vlan-qos table  */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,/* port */
                                             startWord,
                                             startBit,
                                             1, /* 1 bit */
                                             &hwData);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    *enablePtr = BIT2BOOL_MAC(hwData);

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortProtoClassVlanEnableGet function
* @endinternal
*
* @brief   Get status of Port Protocol VLAN assignment to packets received
*         on this port,according to it's Layer2 protocol
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - GT_TRUE for enable, otherwise GT_FALSE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgVlanPortProtoClassVlanEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortProtoClassVlanEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgVlanPortProtoClassVlanEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortProtoClassQosEnable function
* @endinternal
*
* @brief   Enable/Disable Port Protocol QoS Assignment to packets received on this
*         port, according to it's Layer2 protocol
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE for enable, otherwise GT_FALSE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortProtoClassQosEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_U32      hwData;     /* data to write into register */
    GT_U32      startWord;  /* the table word at which the field starts */
    GT_U32      startBit;   /* the word's bit at which the field starts */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    hwData = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to pre-tti-lookup-ingress-eport table */
         rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PROT_BASED_QOS_EN_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            hwData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

        startWord = 1;
        startBit  = 11;

        /* write to port-vlan-qos table  */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,/* port */
                                             startWord,
                                             startBit,
                                             1, /* 1 bit */
                                             hwData);
    }
    return rc;
}

/**
* @internal cpssDxChBrgVlanPortProtoClassQosEnable function
* @endinternal
*
* @brief   Enable/Disable Port Protocol QoS Assignment to packets received on this
*         port, according to it's Layer2 protocol
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE for enable, otherwise GT_FALSE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortProtoClassQosEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortProtoClassQosEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgVlanPortProtoClassQosEnable(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortProtoClassQosEnableGet function
* @endinternal
*
* @brief   Get status of Port Protocol QoS Assignment to packets received on this
*         port, according to it's Layer2 protocol
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - GT_TRUE for enable, otherwise GT_FALSE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChBrgVlanPortProtoClassQosEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
)
{
    GT_U32      hwData;     /* register data */
    GT_U32      startWord;  /* the table word at which the field starts */
    GT_U32      startBit;   /* the word's bit at which the field starts */
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read from pre-tti-lookup-ingress-eport table */
         rc = prvCpssDxChReadTableEntryField(devNum,
                                             CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                             portNum,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                             SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PROT_BASED_QOS_EN_E, /* field name */
                                             PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &hwData);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

        startWord = 1;
        startBit  = 11;

        /* write to port-vlan-qos table  */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,/* port */
                                             startWord,
                                             startBit,
                                             1, /* 1 bit */
                                             &hwData);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    *enablePtr = BIT2BOOL_MAC(hwData);

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortProtoClassQosEnableGet function
* @endinternal
*
* @brief   Get status of Port Protocol QoS Assignment to packets received on this
*         port, according to it's Layer2 protocol
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - GT_TRUE for enable, otherwise GT_FALSE.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgVlanPortProtoClassQosEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_BOOL      *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortProtoClassQosEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgVlanPortProtoClassQosEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanProtoClassSet function
* @endinternal
*
* @brief   Write etherType and encapsulation of Protocol based classification
*         for specific device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES:
*                                      xCat3, AC5, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
* @param[in] etherType                - Ether Type or DSAP/SSAP
* @param[in] encListPtr               - encapsulation.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanProtoClassSet
(
    IN GT_U8                            devNum,
    IN GT_U32                           entryNum,
    IN GT_U16                           etherType,
    IN CPSS_PROT_CLASS_ENCAP_STC        *encListPtr
)
{
    GT_U32      hwData;              /* value to write into register    */
    GT_U32      regAddr;             /* register address                */
    GT_U32      wordOffset;          /* offset of register              */
    GT_U32      offset;              /* offset in a register            */
    GT_STATUS   rc ;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(encListPtr);
    PRV_CPSS_DXCH_BRG_PORT_PROTOCOL_CHECK_MAC(devNum, entryNum);

    /* build encapsulation bitmap */
    hwData = (BOOL2BIT_MAC(encListPtr->ethV2) |
              ((BOOL2BIT_MAC(encListPtr->nonLlcSnap)) << 1) |
              ((BOOL2BIT_MAC(encListPtr->llcSnap)) << 2));

    if (entryNum > 7)
    {
        /* get address of Protocols Encapsulation Register1 */
        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
        {
            regAddr =
                PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).protocolMatch.protocolsEncapsulationConfig1;
        }
        else
        {
            regAddr =
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.vlanRegs.protoEncBased1;
        }

        /* write encapsulation */
        offset = ((entryNum - 8) * 3);
    }
    else
    {
        /* get address of Protocols Encapsulation Register */
        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
        {
            regAddr =
                PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).protocolMatch.protocolsEncapsulationConfig0;
        }
        else
        {
            regAddr =
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.vlanRegs.protoEncBased;
        }

        /* write encapsulation */
        offset = (entryNum * 3);
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, offset, 3, hwData);
    if (rc != GT_OK)
        return rc;

    if (entryNum > 7)
    {
        /* write validity bit */
        offset = (entryNum - 8) + 12;
    }
    else
    {
        /* write validity bit */
        offset = entryNum + 24;
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, offset, 1, 1);
    if (rc != GT_OK)
        return rc;

    /* get address of Protocols Configuration Register */
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).protocolMatch.protocolsConfig[0];
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.vlanRegs.protoTypeBased;
    }

    /* write Ether Type */
    hwData = etherType;
    wordOffset = entryNum >> 1; /* /2 */
    offset = (entryNum & 0x1) ? 16 : 0;

    regAddr += (wordOffset * 4);
    rc = prvCpssHwPpSetRegField(devNum, regAddr, offset, 16, hwData);

    return rc;
}

/**
* @internal cpssDxChBrgVlanProtoClassSet function
* @endinternal
*
* @brief   Write etherType and encapsulation of Protocol based classification
*         for specific device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES:
*                                      xCat3, AC5, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
* @param[in] etherType                - Ether Type or DSAP/SSAP
* @param[in] encListPtr               - encapsulation.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanProtoClassSet
(
    IN GT_U8                            devNum,
    IN GT_U32                           entryNum,
    IN GT_U16                           etherType,
    IN CPSS_PROT_CLASS_ENCAP_STC        *encListPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanProtoClassSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryNum, etherType, encListPtr));

    rc = internal_cpssDxChBrgVlanProtoClassSet(devNum, entryNum, etherType, encListPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryNum, etherType, encListPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanProtoClassGet function
* @endinternal
*
* @brief   Read etherType and encapsulation of Protocol based classification
*         for specific device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES:
*                                      xCat3, AC5, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
*
* @param[out] etherTypePtr             - (pointer to) Ether Type or DSAP/SSAP
* @param[out] encListPtr               - (pointer to) encapsulation.
* @param[out] validEntryPtr            - (pointer to) validity bit
*                                      GT_TRUE - the entry is valid
*                                      GT_FALSE - the entry is invalid
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanProtoClassGet
(
    IN GT_U8                            devNum,
    IN GT_U32                           entryNum,
    OUT GT_U16                          *etherTypePtr,
    OUT CPSS_PROT_CLASS_ENCAP_STC       *encListPtr,
    OUT GT_BOOL                         *validEntryPtr
)
{
    GT_U32      hwData;              /* value to write into register    */
    GT_U32      regAddr;             /* register address                */
    GT_U32      wordOffset;          /* offset of register              */
    GT_U32      offset;              /* offset in a register            */
    GT_STATUS   rc ;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_BRG_PORT_PROTOCOL_CHECK_MAC(devNum, entryNum);
    CPSS_NULL_PTR_CHECK_MAC(encListPtr);
    CPSS_NULL_PTR_CHECK_MAC(validEntryPtr);
    CPSS_NULL_PTR_CHECK_MAC(etherTypePtr);

    if (entryNum > 7)
    {
        /* get address of Protocols Encapsulation Register1 */
        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
        {
            regAddr =
                PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).protocolMatch.protocolsEncapsulationConfig1;
        }
        else
        {
            regAddr =
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.vlanRegs.protoEncBased1;
        }

        /* read validity bit */
        offset = (entryNum - 8) + 12;
    }
    else
    {
        /* get address of Protocols Encapsulation Register */
        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
        {
            regAddr =
                PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).protocolMatch.protocolsEncapsulationConfig0;
        }
        else
        {
            regAddr =
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.vlanRegs.protoEncBased;
        }

        /* read validity bit */
        offset = entryNum + 24;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, offset, 1, &hwData);
    if (rc != GT_OK)
        return rc;
    *validEntryPtr = BIT2BOOL_MAC(hwData & 0x1);

    if (entryNum > 7)
    {
        /* read encapsulation */
        offset = ((entryNum - 8) * 3);
    }
    else
    {
        /* read encapsulation */
        offset = (entryNum * 3);
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, offset, 3, &hwData);
    if (rc != GT_OK)
        return rc;

    /* build encapsulation */
    encListPtr->ethV2 = BIT2BOOL_MAC(hwData & 0x1);
    encListPtr->nonLlcSnap = BIT2BOOL_MAC((hwData >> 1) & 0x1);
    encListPtr->llcSnap = BIT2BOOL_MAC((hwData >> 2) & 0x1);

    /* get address of Protocols Configuration Register */
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).protocolMatch.protocolsConfig[0];
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        bridgeRegs.vlanRegs.protoTypeBased;
    }

    /* get Ether Type */
    wordOffset = entryNum >> 1; /* /2 */
    offset = (entryNum & 0x1) ? 16 : 0;

    regAddr += (wordOffset * 4);
    rc = prvCpssHwPpGetRegField(devNum, regAddr, offset, 16, &hwData);
    if (rc != GT_OK)
        return rc;

    *etherTypePtr = (GT_U16)hwData;

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanProtoClassGet function
* @endinternal
*
* @brief   Read etherType and encapsulation of Protocol based classification
*         for specific device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES:
*                                      xCat3, AC5, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
*
* @param[out] etherTypePtr             - (pointer to) Ether Type or DSAP/SSAP
* @param[out] encListPtr               - (pointer to) encapsulation.
* @param[out] validEntryPtr            - (pointer to) validity bit
*                                      GT_TRUE - the entry is valid
*                                      GT_FALSE - the entry is invalid
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanProtoClassGet
(
    IN GT_U8                            devNum,
    IN GT_U32                           entryNum,
    OUT GT_U16                          *etherTypePtr,
    OUT CPSS_PROT_CLASS_ENCAP_STC       *encListPtr,
    OUT GT_BOOL                         *validEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanProtoClassGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryNum, etherTypePtr, encListPtr, validEntryPtr));

    rc = internal_cpssDxChBrgVlanProtoClassGet(devNum, entryNum, etherTypePtr, encListPtr, validEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryNum, etherTypePtr, encListPtr, validEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanProtoClassInvalidate function
* @endinternal
*
* @brief   Invalidates entry of Protocol based classification for specific device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES:
*                                      xCat3, AC5, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanProtoClassInvalidate
(
    IN GT_U8                            devNum,
    IN GT_U32                           entryNum
)
{
    GT_U32      hwData;              /* value to write into register    */
    GT_U32      regAddr;             /* register address                */
    GT_U32      offset;              /* offset in a register            */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_BRG_PORT_PROTOCOL_CHECK_MAC(devNum, entryNum);

    if (entryNum > 7)
    {
        /* get address of Protocols Encapsulation Register1 */
        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
        {
            regAddr =
                PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).protocolMatch.protocolsEncapsulationConfig1;
        }
        else
        {
            regAddr =
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.vlanRegs.protoEncBased1;
        }

        /* write validity bit */
        offset = (entryNum - 8) + 12;
    }
    else
    {
        /* get address of Protocols Encapsulation Register */
        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
        {
            regAddr =
                PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).protocolMatch.protocolsEncapsulationConfig0;
        }
        else
        {
            regAddr =
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.vlanRegs.protoEncBased;
        }

        /* write validity bit */
        offset = entryNum + 24;
    }

    /* invalidate validity bit */
    hwData = 0;
    return prvCpssHwPpSetRegField(devNum, regAddr, offset, 1, hwData);
}

/**
* @internal cpssDxChBrgVlanProtoClassInvalidate function
* @endinternal
*
* @brief   Invalidates entry of Protocol based classification for specific device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES:
*                                      xCat3, AC5, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanProtoClassInvalidate
(
    IN GT_U8                            devNum,
    IN GT_U32                           entryNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanProtoClassInvalidate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryNum));

    rc = internal_cpssDxChBrgVlanProtoClassInvalidate(devNum, entryNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortProtoVlanQosSet function
* @endinternal
*
* @brief   Write Port Protocol VID and QOS Assignment values for specific device
*         and port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES:
*                                      xCat3, AC5, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
* @param[in] vlanCfgPtr               - pointer to VID parameters
* @param[in] qosCfgPtr                - pointer to QoS parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or vlanId
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note It is possible to update only VLAN or QOS parameters, when one
*       of those input pointers is NULL.
*
*/
static GT_STATUS internal_cpssDxChBrgVlanPortProtoVlanQosSet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_NUM                              portNum,
    IN GT_U32                                   entryNum,
    IN CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC    *vlanCfgPtr,
    IN CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC     *qosCfgPtr
)
{
    GT_U32      hwData;         /* data to read/write from/to register */
    GT_U32      hwMask;         /* mask for hwData with raised bits for updated fields */
    GT_U32      value;          /* value to update */
    GT_U32      tableEntryIndex;/* index in VLAN_PORT_PROTOCOL table */
    GT_U32      wordNumber;     /* word number */
    GT_U32      validBitNum;    /* position of the Valid bit */
    GT_U32      cmdFirstBit;    /* VLAN/QoS command field first bit */
    GT_U32      precedenceFirstBit; /* VLAN/QoS precedence field first bit */
    GT_U32      qosProfileFirstBit; /* QoS profile field first bit */
    GT_U32      modifyDscpFirstBit; /* QoS enable modify DSCP field first bit */
    GT_U32      modifyUpFirstBit;   /* QoS enable modify UP field first bit */
    GT_STATUS   rc;
    GT_U32      fieldLength;/* total bits used in the word */
    GT_U32      maskForKeepVlan;/*mask for reset all QoS related bits and don't change VLAN related bits */
    GT_U32      maskForKeepQos;/*mask for reset all QoS related bits and don't change VLAN related bits */
    GT_U32      length;/*length of field*/
    PRV_CPSS_DXCH_PP_TABLE_FORMAT_INFO_STC * initTablesFormatPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_BRG_PORT_PROTOCOL_CHECK_MAC(devNum, entryNum);

    initTablesFormatPtr = PRV_TABLE_FORMAT_INFO(devNum);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        fieldLength = 0;/* NOT used */

        maskForKeepVlan = 0x0000FFFF;
        maskForKeepQos  = 0xFFFE0000;
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

        fieldLength = 28;

        maskForKeepVlan = 0x00007FFF;
        maskForKeepQos  = 0x0FFF0000;
    }

    /* table is actually a matrix of Port (0..63) and Protocol (0..11).
       for every port are allocated 16 protocol entries (12 valid) */
    tableEntryIndex = portNum * 16 + entryNum;
    /* the entry has only one word */
    wordNumber = 0;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* Valid bit number */
        GET_FIELD_INFO_BY_ID_MAC(
            initTablesFormatPtr[PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E].fieldsInfoPtr,
            SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_VALID_E,
            validBitNum,length);
    }
    else
    {
        /* Valid bit number */
        validBitNum = 15;
    }

    if ((vlanCfgPtr == NULL) && (qosCfgPtr == NULL))
    {
        /* at least one configuration need to be defined */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* fix warnings */
    hwData = 0;

    /* for eArch devices we will call  prvCpssDxChWriteTableEntryMasked
       so we don't need to read the entry at all */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        /* Read entry from Port Protocol VID and QOS Register */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E,
                                            tableEntryIndex, /* table index */
                                            wordNumber,  /* word number */
                                            0,/* start at bit 0*/
                                            fieldLength, /* 28 or 32 bits */
                                            &hwData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* check if both VID and QOS parameters should be set,
       if not mask the appropriate bits */
    if (vlanCfgPtr == NULL)
    {
        /* reset all QoS related bits and don't change VLAN related bits */
        hwData &= maskForKeepVlan;
        hwMask = ~(GT_U32)maskForKeepVlan;
    }
    else if (qosCfgPtr == NULL)
    {
        /* reset all VLAN related bits and don't change QOS related bits */
        hwData &= maskForKeepQos;
        hwMask = ~(GT_U32)maskForKeepQos;
    }
    else
    {
        /* reset all fields */
        hwData = 0;
        hwMask = ~(GT_U32)0;            /* 0xfff..ff */
    }

    if (vlanCfgPtr != NULL)
    {
        /* check validity VLAN id */
        PRV_CPSS_DXCH_VLAN_VALUE_CHECK_MAC(devNum,vlanCfgPtr->vlanId);

        /* setting 12 bits of <ProtocolVID> in bits [0:11]*/
        hwData |= (GT_U32)vlanCfgPtr->vlanId;

        /* define field positions */
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            GET_FIELD_INFO_BY_ID_MAC(
                initTablesFormatPtr[PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E].fieldsInfoPtr,
                SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_EVLAN_COMMAND_E,
                cmdFirstBit,length);
            GET_FIELD_INFO_BY_ID_MAC(
                initTablesFormatPtr[PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E].fieldsInfoPtr,
                SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_EVLAN_PRECEDENCE_E,
                precedenceFirstBit,length);
        }
        else
        {   /* xCat3 and above */
            cmdFirstBit = 12;
            precedenceFirstBit = 14;
        }

        /* setting <VLANCmd> */
        PRV_CPSS_CONVERT_ATTR_ASSIGN_CMD_TO_HW_VAL_MAC(
                                           value, vlanCfgPtr->vlanIdAssignCmd);
        hwData |= (value << cmdFirstBit);

        /* <VIDPrecedence> */
        PRV_CPSS_CONVERT_PRECEDENCE_TO_HW_VAL_MAC(
                                     value, vlanCfgPtr->vlanIdAssignPrecedence);
        hwData |= (value << precedenceFirstBit);
    }

    if (qosCfgPtr != NULL)
    {
        /* check profile ID */
        PRV_CPSS_DXCH_QOS_PROFILE_ID_CHECK_MAC(devNum, qosCfgPtr->qosProfileId);

        /* define field positions */
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            GET_FIELD_INFO_BY_ID_MAC(
                initTablesFormatPtr[PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E].fieldsInfoPtr,
                SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_QOS_PROFILE_E,
                cmdFirstBit,length);
            GET_FIELD_INFO_BY_ID_MAC(
                initTablesFormatPtr[PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E].fieldsInfoPtr,
                SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_QOS_PRECEDENCE_E,
                precedenceFirstBit,length);
            GET_FIELD_INFO_BY_ID_MAC(
                initTablesFormatPtr[PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E].fieldsInfoPtr,
                SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_QOS_PROFILE_E,
                qosProfileFirstBit,length);
            GET_FIELD_INFO_BY_ID_MAC(
                initTablesFormatPtr[PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E].fieldsInfoPtr,
                SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_DSCP_E,
                modifyDscpFirstBit,length);
            GET_FIELD_INFO_BY_ID_MAC(
                initTablesFormatPtr[PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E].fieldsInfoPtr,
                SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_UP_E,
                modifyUpFirstBit,length);
        }
        else
        {  /* xCat3 and above */
            cmdFirstBit         = 18;
            precedenceFirstBit  = 27;
            qosProfileFirstBit  = 20;
            modifyDscpFirstBit  = 17;
            modifyUpFirstBit    = 16;
        }

        /* setting <ProtocolQosProfile> */
        hwData |= (qosCfgPtr->qosProfileId << qosProfileFirstBit);

        /* setting <ModifyDSCP> */
        value = BOOL2BIT_MAC(qosCfgPtr->enableModifyDscp);
        hwData |= (value  << modifyDscpFirstBit);

        /* setting <ModifyUP> */
        value = BOOL2BIT_MAC(qosCfgPtr->enableModifyUp);
        hwData |= (value  << modifyUpFirstBit);

        /* setting <ProtocolQoSPresence> */
        PRV_CPSS_CONVERT_PRECEDENCE_TO_HW_VAL_MAC(value,
                                                  qosCfgPtr->qosAssignPrecedence);
        hwData |= (value << precedenceFirstBit);

        /* setting the Protocol <QoSProfile Cmd> */
        PRV_CPSS_CONVERT_ATTR_ASSIGN_CMD_TO_HW_VAL_MAC(value,
                                                       qosCfgPtr->qosAssignCmd);
        hwData |= (value << cmdFirstBit);
    }

    /* set validity bit */
    hwData |= (0x1 << validBitNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* Write entry to Port Protocol VID and QOS table */
        rc = prvCpssDxChWriteTableEntryMasked(devNum,
                                              CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E,
                                              tableEntryIndex, /* table index */
                                              &hwData,
                                              &hwMask);
    }
    else
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E,
                                             tableEntryIndex, /* table index */
                                             wordNumber,  /* word number */
                                             0,/* start at bit 0*/
                                             fieldLength, /* 28 or 32 bits */
                                             hwData);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortProtoVlanQosSet function
* @endinternal
*
* @brief   Write Port Protocol VID and QOS Assignment values for specific device
*         and port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES:
*                                      xCat3, AC5, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
* @param[in] vlanCfgPtr               - pointer to VID parameters
* @param[in] qosCfgPtr                - pointer to QoS parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or vlanId
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note It is possible to update only VLAN or QOS parameters, when one
*       of those input pointers is NULL.
*
*/
GT_STATUS cpssDxChBrgVlanPortProtoVlanQosSet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_NUM                              portNum,
    IN GT_U32                                   entryNum,
    IN CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC    *vlanCfgPtr,
    IN CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC     *qosCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortProtoVlanQosSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, entryNum, vlanCfgPtr, qosCfgPtr));

    rc = internal_cpssDxChBrgVlanPortProtoVlanQosSet(devNum, portNum, entryNum, vlanCfgPtr, qosCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, entryNum, vlanCfgPtr, qosCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortProtoVlanQosGet function
* @endinternal
*
* @brief   Get Port Protocol VID and QOS Assignment values for specific device
*         and port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES:
*                                      xCat3, AC5, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
*
* @param[out] vlanCfgPtr               - pointer to VID parameters (can be NULL)
* @param[out] qosCfgPtr                - pointer to QoS parameters (can be NULL)
* @param[out] validEntryPtr            - pointer to validity bit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or vlanId
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note It is possible to get only one of VLAN or QOS parameters by receiving
*       one of those pointers as NULL pointer or get only validity bit.
*
*/
static GT_STATUS internal_cpssDxChBrgVlanPortProtoVlanQosGet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_NUM                              portNum,
    IN GT_U32                                   entryNum,
    OUT CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC   *vlanCfgPtr,
    OUT CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC    *qosCfgPtr,
    OUT GT_BOOL                                 *validEntryPtr
)
{
    GT_U32      hwData;         /* data to read from register */
    GT_U32      value;          /* temporary data */
    GT_U32      tableEntryIndex;/* index in VLAN_PORT_PROTOCOL table */
    GT_U32      wordNumber;     /* word number */
    GT_U32      validBitNum;    /* position of the Valid bit */
    GT_U32      cmdFirstBit;    /* VLAN/QoS command field first bit */
    GT_U32      precedenceFirstBit; /* VLAN/QoS precedence field first bit */
    GT_U32      qosProfileFirstBit; /* QoS profile field first bit */
    GT_U32      modifyDscpFirstBit; /* QoS enable modify DSCP field first bit */
    GT_U32      modifyUpFirstBit;   /* QoS enable modify UP field first bit */
    GT_STATUS   rc;
    GT_U32      fieldLength;/* total bits used in the word */
    GT_U32      length;/*length of field*/
    GT_U32      bit,qosProfileIdMask = 0x7f;
    PRV_CPSS_DXCH_PP_TABLE_FORMAT_INFO_STC * initTablesFormatPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_BRG_PORT_PROTOCOL_CHECK_MAC(devNum, entryNum);
    CPSS_NULL_PTR_CHECK_MAC(validEntryPtr);

    initTablesFormatPtr = PRV_TABLE_FORMAT_INFO(devNum);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);
        fieldLength = 0;/* NOT used */
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
        fieldLength = 28;
    }

    /* table is actually a matrix of Port (0..63) and Protocol (0..11).
       for every port are allocated 16 protocol entries (12 valid) */
    tableEntryIndex = portNum * 16 + entryNum;
    /* the entry has only one word */
    wordNumber = 0;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* Valid bit number */
        GET_FIELD_INFO_BY_ID_MAC(
            initTablesFormatPtr[PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E].fieldsInfoPtr,
            SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_VALID_E,
            validBitNum,length);
    }
    else
    {
        /* Valid bit number */
        validBitNum = 15;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* Read entry from Port Protocol VID and QOS table */
        rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E,
                                        tableEntryIndex, /* table index */
                                        &hwData);
    }
    else
    {
        /* Read entry from Port Protocol VID and QOS Register */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E,
                                            tableEntryIndex, /* table index */
                                            wordNumber,  /* word number */
                                            0,/* start at bit 0*/
                                            fieldLength, /* 28 or 32 bits */
                                            &hwData);
    }

    if (rc != GT_OK)
    {
        return rc;
    }

    /* get validity bit */
    value = (hwData >> validBitNum) & 0x1;
    *validEntryPtr = BIT2BOOL_MAC(value);

    /* check if VLAN is not NULL pointer */
    if (vlanCfgPtr != NULL)
    {
        /* define field positions */
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            GET_FIELD_INFO_BY_ID_MAC(
                initTablesFormatPtr[PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E].fieldsInfoPtr,
                SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_EVLAN_COMMAND_E,
                cmdFirstBit,length);
            GET_FIELD_INFO_BY_ID_MAC(
                initTablesFormatPtr[PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E].fieldsInfoPtr,
                SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_EVLAN_PRECEDENCE_E,
                precedenceFirstBit,length);
        }
        else
        {   /* xCat3 and above */
            cmdFirstBit = 12;
            precedenceFirstBit = 14;
        }

        /* get VLAN entry parameters */

        /* get Protocol VID */
        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
            GET_FIELD_INFO_BY_ID_MAC(
                initTablesFormatPtr[PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E].fieldsInfoPtr,
                SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_EVLAN_E,
                bit,length);
            vlanCfgPtr->vlanId = (GT_U16)(hwData & BIT_MASK_MAC(length));
        }
        else
        {
            vlanCfgPtr->vlanId = (GT_U16)(hwData & 0xFFF);
        }

        /* get (VLANCmd> */
        value = (hwData >> cmdFirstBit) & 0x3;
        PRV_CPSS_CONVERT_HW_VAL_TO_ATTR_ASSIGN_CMD_MAC(vlanCfgPtr->vlanIdAssignCmd,
                                                       value);
        /* get <VIDPrecedence> */
        value = (hwData >> precedenceFirstBit) & 0x1;
        PRV_CPSS_CONVERT_HW_VAL_TO_PRECEDENCE_MAC(vlanCfgPtr->vlanIdAssignPrecedence,
                                                  value);
    }

    /* check if QOS pointer is not NULL*/
    if (qosCfgPtr != NULL)
    {
        /* define field positions */
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            GET_FIELD_INFO_BY_ID_MAC(
                initTablesFormatPtr[PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E].fieldsInfoPtr,
                SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_QOS_PROFILE_E,
                cmdFirstBit,length);
            GET_FIELD_INFO_BY_ID_MAC(
                initTablesFormatPtr[PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E].fieldsInfoPtr,
                SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_QOS_PRECEDENCE_E,
                precedenceFirstBit,length);
            GET_FIELD_INFO_BY_ID_MAC(
                initTablesFormatPtr[PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E].fieldsInfoPtr,
                SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_QOS_PROFILE_E,
                qosProfileFirstBit,length);
            qosProfileIdMask = BIT_MASK_MAC(length);
            GET_FIELD_INFO_BY_ID_MAC(
                initTablesFormatPtr[PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E].fieldsInfoPtr,
                SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_DSCP_E,
                modifyDscpFirstBit,length);
            GET_FIELD_INFO_BY_ID_MAC(
                initTablesFormatPtr[PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E].fieldsInfoPtr,
                SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_UP_E,
                modifyUpFirstBit,length);
        }
        else
        {   /* xCat3 and above */
            cmdFirstBit         = 18;
            precedenceFirstBit  = 27;
            qosProfileFirstBit  = 20;
            modifyDscpFirstBit  = 17;
            modifyUpFirstBit    = 16;
        }

        /* get QOS entry parameters */

        /* get <ProtocolQosProfile> */
        qosCfgPtr->qosProfileId = (hwData >> qosProfileFirstBit) & qosProfileIdMask;

        /* get <ModifyDSCP> */
        qosCfgPtr->enableModifyDscp = BIT2BOOL_MAC((hwData >> modifyDscpFirstBit) & 0x1);

        /* get <ModifyUP> */
        qosCfgPtr->enableModifyUp = BIT2BOOL_MAC((hwData >> modifyUpFirstBit) & 0x1);

        /* get <ProtocolQoSPresence> */
        value = (hwData >> precedenceFirstBit) & 0x1;
        PRV_CPSS_CONVERT_HW_VAL_TO_PRECEDENCE_MAC(qosCfgPtr->qosAssignPrecedence,
                                                  value);
        /* get Protocol <QoSProfile Cmd> */
        value = (hwData >> cmdFirstBit) & 0x3;
        PRV_CPSS_CONVERT_HW_VAL_TO_ATTR_ASSIGN_CMD_MAC(qosCfgPtr->qosAssignCmd,
                                                       value);
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanPortProtoVlanQosGet function
* @endinternal
*
* @brief   Get Port Protocol VID and QOS Assignment values for specific device
*         and port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES:
*                                      xCat3, AC5, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
*
* @param[out] vlanCfgPtr               - pointer to VID parameters (can be NULL)
* @param[out] qosCfgPtr                - pointer to QoS parameters (can be NULL)
* @param[out] validEntryPtr            - pointer to validity bit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or vlanId
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note It is possible to get only one of VLAN or QOS parameters by receiving
*       one of those pointers as NULL pointer or get only validity bit.
*
*/
GT_STATUS cpssDxChBrgVlanPortProtoVlanQosGet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_NUM                              portNum,
    IN GT_U32                                   entryNum,
    OUT CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC   *vlanCfgPtr,
    OUT CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC    *qosCfgPtr,
    OUT GT_BOOL                                 *validEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortProtoVlanQosGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, entryNum, vlanCfgPtr, qosCfgPtr, validEntryPtr));

    rc = internal_cpssDxChBrgVlanPortProtoVlanQosGet(devNum, portNum, entryNum, vlanCfgPtr, qosCfgPtr, validEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, entryNum, vlanCfgPtr, qosCfgPtr, validEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortProtoVlanQosInvalidate function
* @endinternal
*
* @brief   Invalidate protocol based QoS and VLAN assignment entry for
*         specific port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES:
*                                      xCat3, AC5, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortProtoVlanQosInvalidate
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_U32                   entryNum
)
{
    GT_STATUS   rc;
    GT_U32      hwData;          /* value to write into HW   */
    GT_U32      tableEntryIndex; /* index in VLAN_PORT_PROTOCOL table */
    GT_U32      wordNumber;      /* word number */
    GT_U32      bitNum;          /* isValid bit number - not relevant to E_ARCH_ENABLED*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_BRG_PORT_PROTOCOL_CHECK_MAC(devNum, entryNum);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);
    }

    /* table is actually a matrix of Port (0..63) and Protocol (0..11).
       for every port are allocated 16 protocol entries (12 valid) */
    tableEntryIndex = portNum * 16 + entryNum;
    /* the entry has only one word */
    wordNumber = 0;
    bitNum = 15;

    hwData = 0;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* Write entry to Port Protocol VID and QOS table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E,
                                             tableEntryIndex, /* table index */
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                             SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_VALID_E, /* field name */
                                             PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                             hwData);
    }
    else
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E,
                                             tableEntryIndex, /* table index */
                                             wordNumber,  /* word number */
                                             bitNum,/* first bit */
                                             1, /* 1 bit */
                                             hwData);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortProtoVlanQosInvalidate function
* @endinternal
*
* @brief   Invalidate protocol based QoS and VLAN assignment entry for
*         specific port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] entryNum                 - entry number
*                                      (APPLICABLE RANGES:
*                                      xCat3, AC5, Lion2, Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X 0..11)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortProtoVlanQosInvalidate
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_U32                   entryNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortProtoVlanQosInvalidate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, entryNum));

    rc = internal_cpssDxChBrgVlanPortProtoVlanQosInvalidate(devNum, portNum, entryNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, entryNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanLearningStateSet function
* @endinternal
*
* @brief   Sets state of VLAN based learning to specified VLAN on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - vlan Id
* @param[in] status                   - GT_TRUE for enable  or GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanLearningStateSet
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId,
    IN GT_BOOL  status
)
{
    GT_STATUS   rc;      /* return code*/
    GT_U32      hwData;  /* value to write to VLAN entry*/

    /* check for device type */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    /* if the user want to enable learning on a vlan then we :
    set Automatic Learning Disable - bit 0 in word 3 */
    hwData = (status == GT_TRUE) ? 0 : 1;

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_VLAN_E,
                                             vlanId,/* vid */
                                             3,/* word 3 */
                                             0,/* start at bit 0*/
                                             1, /* one bit */
                                             hwData);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the <AUTO_LEARN_DIS>*/
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_AUTO_LEARN_DIS_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        hwData);
    }
    else
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                            (GT_U32)vlanId,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            97,/* start at bit 97*/
                                            1, /* one bit */
                                            hwData);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanLearningStateSet function
* @endinternal
*
* @brief   Sets state of VLAN based learning to specified VLAN on specified device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - vlan Id
* @param[in] status                   - GT_TRUE for enable  or GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanLearningStateSet
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId,
    IN GT_BOOL  status
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanLearningStateSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, status));

    rc = internal_cpssDxChBrgVlanLearningStateSet(devNum, vlanId, status);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, status));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgVlanIsDevMember function
* @endinternal
*
* @brief   Checks if specified device has members belong to specified vlan
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - Vlan ID
*
* @retval GT_TRUE                  - if port is member of vlan
* @retval GT_FALSE                 - if port does not member of vlan or other status error
*/
static GT_BOOL internal_cpssDxChBrgVlanIsDevMember
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId
)
{
    GT_STATUS   rc;
    GT_U32      hwData[PRV_CPSS_DXCH_VLAN_ENTRY_MAX_WORDS_COUNT_CNS]; /* word to read
                                                            /write entry info */
    GT_U32      hwDataMask[PRV_CPSS_DXCH_VLAN_ENTRY_MAX_WORDS_COUNT_CNS]; /* word Mask
                                                     to read/write entry info */
    GT_BOOL     result = GT_FALSE;     /* device membership result */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32      i;                /* iterator */
    CPSS_PORTS_BMP_STC   portsMembers;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* get the bridge port members */
        rc = prvCpssDxChReadTableEntry(devNum,
                                       CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E,
                                       vlanId,/* vid */
                                       &portsMembers.ports[0]);
        if(rc != GT_OK)
        {
            return GT_FALSE;
        }

        /* adjusted physical ports of BMP to hold only bmp of existing ports*/
        rc = prvCpssDxChTablePortsBmpAdjustToDevice(devNum , &portsMembers , &portsMembers);
        if(rc != GT_OK)
        {
            return rc;
        }


        return PRV_CPSS_PORTS_BMP_IS_ZERO_MAC(&portsMembers) ?
                GT_FALSE : /* no members */
                GT_TRUE;   /* at least one member */
    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* mask the vlan ports membership only --- without the tagging */
        hwDataMask[0] = 0x55000000; /* 4 ports -- 8 bits */
        hwDataMask[1] = 0x55555555; /* 16 ports -- 32 bits */
        hwDataMask[2] = 0x00001555; /* 7 ports -- 14 bits */

        /* port 27 data is placed in word 3 bits[23:24] */
        hwDataMask[3] = 0x00800000; /* 1 port -- 2 bits */

        /* get the entry from all port groups , each port group get different ports */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
        {
            /* check if we have local port in the vlan */
            rc = prvCpssDxChPortGroupReadTableEntry(devNum,portGroupId,
                                           CPSS_DXCH_TABLE_VLAN_E,
                                           (GT_U32)vlanId,/* vlanId */
                                           hwData);
            if(rc != GT_OK)
            {
                return GT_FALSE;
            }

            result = ((hwData[0] & hwDataMask[0]) ||
                      (hwData[1] & hwDataMask[1]) ||
                      (hwData[2] & hwDataMask[2]) ||
                      (hwData[3] & hwDataMask[3]) ) ?
                    GT_TRUE : GT_FALSE;

            if(result == GT_TRUE)
            {
                /* no need to continue loop port groups , this port group already has port member */
                return GT_TRUE;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    }
    else
    {
        /* mask the vlan ports membership only --- without the tagging */
        /* members are in bits 2..65 */
        hwDataMask[0] = 0xFFFFFFFC; /* 30 ports */
        hwDataMask[1] = 0xFFFFFFFF; /* 32 ports */
        hwDataMask[2] = 0x00000003; /* 2 ports  */

        if (GT_FALSE != PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                info_PRV_CPSS_DXCH_LION2_MC_BANDWIDTH_RESTRICTION_WA_E.enabled)
        {
            /* mask relay port 12 - bit 14 */
            U32_SET_FIELD_MAC(hwDataMask[0], 14, 1, 0);
        }

        /* loop on all TXQ units */
        for (i =0; i < PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_MAC(devNum); i++)
        {
            /* check if we have local port in the vlan */
            rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                            i*PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS,
                                       CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                       (GT_U32)vlanId,/* vlanId */
                                       &hwData[0]);
            if(rc != GT_OK)
            {
                return GT_FALSE;
            }

            result |= ((hwData[0] & hwDataMask[0]) ||
                  (hwData[1] & hwDataMask[1]) ||
                  (hwData[2] & hwDataMask[2]) ) ?
                GT_TRUE : GT_FALSE;
    }

    }

    return result;
}

/**
* @internal cpssDxChBrgVlanIsDevMember function
* @endinternal
*
* @brief   Checks if specified device has members belong to specified vlan
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - Vlan ID
*
* @retval GT_TRUE                  - if port is member of vlan
* @retval GT_FALSE                 - if port does not member of vlan or other status error
*/
GT_BOOL cpssDxChBrgVlanIsDevMember
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId
)
{
    GT_STATUS rc;
    GT_BOOL boolRc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIsDevMember);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId));

    boolRc = internal_cpssDxChBrgVlanIsDevMember(devNum, vlanId);

    if( GT_TRUE == boolRc)
    {
        rc = GT_OK;
    }
    else
    {
        rc = GT_FAIL;
    }

    /* To avoid compilation warning until LOG MACROs implementation */
    rc = rc;

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return boolRc;
}

/**
* @internal internal_cpssDxChBrgVlanToStpIdBind function
* @endinternal
*
* @brief   Bind VLAN to STP Id.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device Id
* @param[in] vlanId                   - vlan Id
* @param[in] stpId                    - STP Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_OUT_OF_RANGE          - out of range stpId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanToStpIdBind
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId,
    IN GT_U16   stpId
)
{
    GT_STATUS   rc;
    GT_U32      hwData;     /* word to read/write entry info   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    if(stpId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_STG_ID_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    hwData = stpId;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* read entry from HW ingress Span State Group Index table */
        rc = prvCpssDxChWriteTableEntry(devNum,
                                   CPSS_DXCH_SIP5_TABLE_INGRESS_SPAN_STATE_GROUP_INDEX_E,
                                   vlanId,/* vid */
                                   &hwData);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* egress : set the <SPAN_STATE_GROUP_INDEX>*/
        rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_SPANNING_E,
                                        vlanId,/* vid */
                                        &hwData);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_VLAN_E,
                                             (GT_U32)vlanId,/* vid */
                                             2,/* start at word 2 */
                                             24,/* start at bit 24 */
                                             8, /* 8 bits to write */
                                             hwData);
    }
    else
    {
        /* write to ingress vlan */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                            (GT_U32)vlanId,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            130, /* start at bit 130 */
                                            8, /* 8 bits */
                                            hwData);

        if(rc != GT_OK)
        {
            return rc;
        }

        /* write to egress vlan */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
                                            (GT_U32)vlanId,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            254, /* start at bit 254 */
                                            8, /* 8 bits */
                                            hwData);

        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;

}

/**
* @internal cpssDxChBrgVlanToStpIdBind function
* @endinternal
*
* @brief   Bind VLAN to STP Id.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device Id
* @param[in] vlanId                   - vlan Id
* @param[in] stpId                    - STP Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_OUT_OF_RANGE          - out of range stpId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanToStpIdBind
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId,
    IN GT_U16   stpId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanToStpIdBind);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, stpId));

    rc = internal_cpssDxChBrgVlanToStpIdBind(devNum, vlanId, stpId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, stpId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanStpIdGet function
* @endinternal
*
* @brief   Read STP Id that bind to specified VLAN.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device Id
* @param[in] vlanId                   - vlan Id
*
* @param[out] stpIdPtr                 - pointer to STP Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanStpIdGet
(
    IN  GT_U8    devNum,
    IN  GT_U16   vlanId,
    OUT GT_U16   *stpIdPtr
)
{
    GT_U32      hwData;
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);
    CPSS_NULL_PTR_CHECK_MAC(stpIdPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* read entry from HW ingress Span State Group Index table */
        rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_SIP5_TABLE_INGRESS_SPAN_STATE_GROUP_INDEX_E,
                                   vlanId,/* vid */
                                   &hwData);
    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                         CPSS_DXCH_TABLE_VLAN_E,
                                         (GT_U32)vlanId,/* vid */
                                         2,/* start at word 2 */
                                         24,/* start at bit 24 */
                                         8, /* 8 bits to read */
                                         &hwData);
    }
    else
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                              CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                              (GT_U32)vlanId,
                                              PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                              130,/* start at bit 130 */
                                              8, /* 8 bits */
                                              &hwData);
    }

    *stpIdPtr = (GT_U16)hwData;

    return rc;
}

/**
* @internal cpssDxChBrgVlanStpIdGet function
* @endinternal
*
* @brief   Read STP Id that bind to specified VLAN.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device Id
* @param[in] vlanId                   - vlan Id
*
* @param[out] stpIdPtr                 - pointer to STP Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanStpIdGet
(
    IN  GT_U8    devNum,
    IN  GT_U16   vlanId,
    OUT GT_U16   *stpIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanStpIdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, stpIdPtr));

    rc = internal_cpssDxChBrgVlanStpIdGet(devNum, vlanId, stpIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, stpIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanTableInvalidate function
* @endinternal
*
* @brief   This function invalidates VLAN Table entries by writing 0 to the
*         first word.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanTableInvalidate
(
    IN GT_U8 devNum
)
{
    GT_STATUS   rc;
    GT_U16      vlanId;
    GT_U32      maxVid;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    maxVid = PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(devNum);

    /* reset all valid entries in vlan table */
    for(vlanId = 0 ; vlanId <= maxVid ; vlanId ++)
    {
        /* write to word 0 bit 0 in HW */
        rc = cpssDxChBrgVlanEntryInvalidate(devNum, vlanId);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Disable ingress mirroring of packets to the Ingress Analyzer */
        rc = cpssDxChBrgVlanIngressMirrorEnable(devNum, vlanId, GT_FALSE,0);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanTableInvalidate function
* @endinternal
*
* @brief   This function invalidates VLAN Table entries by writing 0 to the
*         first word.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanTableInvalidate
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanTableInvalidate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChBrgVlanTableInvalidate(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanEntryInvalidate function
* @endinternal
*
* @brief   This function invalidates VLAN entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] vlanId                   - VLAN id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanEntryInvalidate
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId
)
{
    GT_STATUS   rc;
    CPSS_PORTS_BMP_STC                   portsMembers;
    CPSS_PORTS_BMP_STC                   portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC          vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* Sip5.20 need to reset all members before invalidate,  */
        /* because there is no valid bit on the Egress VLAN      */
        if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {

            cpssOsBzero((GT_CHAR *)&portsTagging, sizeof(portsTagging));
            cpssOsBzero((GT_CHAR *)&vlanInfo, sizeof(vlanInfo));
            cpssOsBzero((GT_CHAR *)&portsTaggingCmd, sizeof(portsTaggingCmd));
            PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);

            rc = cpssDxChBrgVlanEntryWrite(devNum, vlanId, &portsMembers,
                    &portsTagging, &vlanInfo, &portsTaggingCmd);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* ingress : set the <VALID>*/
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_VALID_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        0);
        if(rc != GT_OK)
        {
            return rc;
        }

        if (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            /* egress : set the <VALID>*/
            rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
                                            vlanId,/* vid */
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS_VALID_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            0);
        }

    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* write to word 0 bit 0 in HW */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_TABLE_VLAN_E,
                                         vlanId,/* vid */
                                         0,/* start at word 0 */
                                         0,/* start at bit 0 */
                                         1, /* 1 bit */
                                         0);
    }
    else
    {
        /* invalidate the ingress vlan table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                              CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                              vlanId,
                                              PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                              0,
                                              1,
                                              0);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* invalidate the egress vlan table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                              CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
                                              vlanId,
                                              PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                              0,
                                              1,
                                              0);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanEntryInvalidate function
* @endinternal
*
* @brief   This function invalidates VLAN entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] vlanId                   - VLAN id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanEntryInvalidate
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanEntryInvalidate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId));

    rc = internal_cpssDxChBrgVlanEntryInvalidate(devNum, vlanId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanUnkUnregFilterSet function
* @endinternal
*
* @brief   Set per VLAN filtering command for specified Unknown or Unregistered
*         packet type
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] vlanId                   - VLAN ID
* @param[in] packetType               - packet type
* @param[in] cmd                      - command for the specified packet type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanUnkUnregFilterSet
(
    IN GT_U8                                devNum,
    IN GT_U16                               vlanId,
    IN CPSS_DXCH_BRG_VLAN_PACKET_TYPE_ENT   packetType,
    IN CPSS_PACKET_CMD_ENT                  cmd

)
{
    GT_STATUS   rc;
    GT_U32      hwValue;             /* hardware value of the command */
    GT_U32      wordOffset;          /* the word number in the VLAN Table */
    GT_U32      bitOffset;           /* bit offset in the word */
    GT_U32      globalOffset;/* relevant only to splitTablesSupported */
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_ENT fieldName;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    /* check the type of the packet */
    switch (packetType)
    {
        case CPSS_DXCH_BRG_VLAN_PACKET_UNK_UCAST_E:
             /* set unknown unicast - word 0, bits 12-14 */
            wordOffset = 0;
            bitOffset = 12;

            globalOffset = 75;
            fieldName = SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNKNOWN_UNICAST_CMD_E;
            break;

        case CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IP_MCAST_E:
            /* set unregistered non IP multicast command - word 0, bits 3-5 */
            wordOffset = 0;
            bitOffset = 3;

            globalOffset = 66;
            fieldName = SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_NON_IP_MULTICAST_CMD_E;

            break;

        case CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_MCAST_E:
            /* set unregistered IPv4 multicast command - word 0, bits 6-8 */
            wordOffset = 0;
            bitOffset = 6;

            globalOffset = 69;
            fieldName = SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV4_MULTICAST_CMD_E;
            break;

        case CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV6_MCAST_E:
            /* set unregistered IPv6 multicast command - word 0, bits 9-11 */
            wordOffset = 0;
            bitOffset = 9;

            globalOffset = 72;
            fieldName = SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV6_MULTICAST_CMD_E;
            break;

        case CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IPV4_BCAST_E:
            /* set unregistered non IPv4 broadcast command - word 2,
                                                            bits 21-23 */
            wordOffset = 2;
            bitOffset = 21;

            globalOffset = 89;
            fieldName = SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_NON_IPV4_BC_CMD_E;
            break;

        case CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_BCAST_E:
            /* set unregistered IPv4 broadcast command - word 2, bits 18-20 */
            wordOffset = 2;
            bitOffset = 18;

            globalOffset = 86;
            fieldName = SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV4_BC_CMD_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }

    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(hwValue, cmd)

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the relevant field */
        return prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        fieldName, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        hwValue);

    }
    else
    if(PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        return prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                             vlanId,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                             globalOffset,
                                             3, /* 3 bits */
                                             hwValue);
    }
    else
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_VLAN_E,
                                             vlanId,/* vid */
                                             wordOffset,    /* word # */
                                             bitOffset,     /* start at bit */
                                             3, /* 3 bits */
                                             hwValue);
        return rc;
    }

}

/**
* @internal cpssDxChBrgVlanUnkUnregFilterSet function
* @endinternal
*
* @brief   Set per VLAN filtering command for specified Unknown or Unregistered
*         packet type
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] vlanId                   - VLAN ID
* @param[in] packetType               - packet type
* @param[in] cmd                      - command for the specified packet type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanUnkUnregFilterSet
(
    IN GT_U8                                devNum,
    IN GT_U16                               vlanId,
    IN CPSS_DXCH_BRG_VLAN_PACKET_TYPE_ENT   packetType,
    IN CPSS_PACKET_CMD_ENT                  cmd

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanUnkUnregFilterSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, packetType, cmd));

    rc = internal_cpssDxChBrgVlanUnkUnregFilterSet(devNum, vlanId, packetType, cmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, packetType, cmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanForcePvidEnable function
* @endinternal
*
* @brief   Set Port VID Assignment mode.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE  - PVID is assigned to all packets.
*                                      GT_FALSE - PVID is assigned to untagged or priority tagged
*                                      packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanForcePvidEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS   rc;
    GT_U32      startWord;  /* the table word at which the field starts */
    GT_U32      startBit;   /* the word's bit at which the field starts */
    GT_U32      hwData;     /* data to write to Hw */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    hwData = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to pre-tti-lookup-ingress-eport table */
         rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_P_EVLAN_MODE_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            hwData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);

        startWord = 0;
        startBit  = 27;

        /* write the value "forceEn" to <PVIDMode> field of port-vlan-qos table  */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,/* port */
                                             startWord,
                                             startBit,
                                             1, /* set 1 bits */
                                             hwData);
    }
    return rc;

}

/**
* @internal cpssDxChBrgVlanForcePvidEnable function
* @endinternal
*
* @brief   Set Port VID Assignment mode.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE  - PVID is assigned to all packets.
*                                      GT_FALSE - PVID is assigned to untagged or priority tagged
*                                      packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanForcePvidEnable
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanForcePvidEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgVlanForcePvidEnable(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanForcePvidEnableGet function
* @endinternal
*
* @brief   Get Port VID Assignment mode.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - GT_TRUE  - PVID is assigned to all packets.
*                                      GT_FALSE - PVID is assigned to untagged or priority tagged
*                                      packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanForcePvidEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32      startWord;  /* the table word at which the field starts */
    GT_U32      startBit;   /* the word's bit at which the field starts */
    GT_U32      hwData;     /* data to write to Hw */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read the value "forceEn" from <PVIDMode> field of pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_P_EVLAN_MODE_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &hwData);


    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);

        startWord = 0;
        startBit  = 27;

        /* read the value "forceEn" from <PVIDMode> field of port-vlan-qos table  */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,/* port */
                                            startWord,
                                            startBit,
                                            1, /* set 1 bits */
                                            &hwData);
    }
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(hwData);

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanForcePvidEnableGet function
* @endinternal
*
* @brief   Get Port VID Assignment mode.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] enablePtr                - GT_TRUE  - PVID is assigned to all packets.
*                                      GT_FALSE - PVID is assigned to untagged or priority tagged
*                                      packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanForcePvidEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanForcePvidEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgVlanForcePvidEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortVidPrecedenceSet function
* @endinternal
*
* @brief   Set Port VID Precedence.
*         Relevant for packets with assigned VLAN ID of Pvid.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] precedence               -  type - soft or hard
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortVidPrecedenceSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT  precedence
)
{
    GT_STATUS   rc;
    GT_U32      startWord;  /* the table word at which the field starts */
    GT_U32      startBit;   /* the word's bit at which the field starts */
    GT_U32      hwData;     /* data to write to Hw */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_CONVERT_PRECEDENCE_TO_HW_VAL_MAC(hwData, precedence);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* write to pre-tti-lookup-ingress-eport table */
         rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_P_VID_PRECEDENCE_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            hwData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);

        startWord = 0;
        startBit  = 29;

        /* write the value to <PVIDPrecedence> field of port-vlan-qos table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                             portNum,/* port */
                                             startWord,
                                             startBit,
                                             1, /* 1 bit */
                                             hwData);
    }
    return rc;
}

/**
* @internal cpssDxChBrgVlanPortVidPrecedenceSet function
* @endinternal
*
* @brief   Set Port VID Precedence.
*         Relevant for packets with assigned VLAN ID of Pvid.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] precedence               -  type - soft or hard
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortVidPrecedenceSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT  precedence
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortVidPrecedenceSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, precedence));

    rc = internal_cpssDxChBrgVlanPortVidPrecedenceSet(devNum, portNum, precedence);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, precedence));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortVidPrecedenceGet function
* @endinternal
*
* @brief   Get Port VID Precedence.
*         Relevant for packets with assigned VLAN ID of Pvid.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] precedencePtr            - (pointer to) precedence type - soft or hard
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChBrgVlanPortVidPrecedenceGet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    OUT CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT *precedencePtr
)
{
    GT_STATUS   rc;
    GT_U32      startWord;  /* the table word at which the field starts */
    GT_U32      startBit;   /* the word's bit at which the field starts */
    GT_U32      hwData;     /* data to read from Hw */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(precedencePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

        /* read the value to <PVIDPrecedence> field of pre-tti-lookup-ingress-eport table  */
         rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_P_VID_PRECEDENCE_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &hwData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);

        startWord = 0;
        startBit  = 29;

        /* read the value to <PVIDPrecedence> field of port-vlan-qos table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                            portNum,/* port */
                                            startWord,
                                            startBit,
                                            1, /* 1 bit */
                                            &hwData);
    }
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_CONVERT_HW_VAL_TO_PRECEDENCE_MAC(*precedencePtr, hwData)

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanPortVidPrecedenceGet function
* @endinternal
*
* @brief   Get Port VID Precedence.
*         Relevant for packets with assigned VLAN ID of Pvid.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
*
* @param[out] precedencePtr            - (pointer to) precedence type - soft or hard
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgVlanPortVidPrecedenceGet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    OUT CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT *precedencePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortVidPrecedenceGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, precedencePtr));

    rc = internal_cpssDxChBrgVlanPortVidPrecedenceGet(devNum, portNum, precedencePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, precedencePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIpUcRouteEnable function
* @endinternal
*
* @brief   Enable/Disable FCoE forwarding and/or IPv4/Ipv6
*          Unicast Routing on Vlan
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - Vlan ID
* @param[in] protocol                 - ipv4,ipv6 and/or fcoe
* @param[in] enable                   - GT_TRUE - enable fcoe forwarding and/or ip unicast routing;
*                                     - GT_FALSE  - disable fcoe forwarding and/or ip unicast routing;
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanIpUcRouteEnable
(
    IN GT_U8                                    devNum,
    IN GT_U16                                   vlanId,
    IN CPSS_IP_PROTOCOL_STACK_ENT               protocol,
    IN GT_BOOL                                  enable
)
{
    GT_U32      globalOffset;/* relevant only to splitTablesSupported */
    GT_U32      bitOffset;      /* bit offset in the VLAN Entry word */
    GT_U32      hwData;         /* data to write to register */
    GT_STATUS   rc;             /* return status */
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_ENT fieldName;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    /* when the user want to Ip routing or disable */
    hwData = BOOL2BIT_MAC(enable);
    rc = GT_OK;

    switch(protocol)
    {
        case CPSS_IP_PROTOCOL_IPV4_E :
            bitOffset = 14;
            globalOffset = 92;
            fieldName = SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_UNICAST_ROUTE_EN_E;
            break;
        case CPSS_IP_PROTOCOL_IPV4V6_E:
            /* ipv4 */
            rc = cpssDxChBrgVlanIpUcRouteEnable(devNum,vlanId,
                                                 CPSS_IP_PROTOCOL_IPV4_E,
                                                 enable);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* ipv6 */
            return cpssDxChBrgVlanIpUcRouteEnable(devNum,vlanId,
                                                 CPSS_IP_PROTOCOL_IPV6_E,
                                                 enable);
        case CPSS_IP_PROTOCOL_IPV6_E :
            bitOffset = 16;
            globalOffset = 94;
            fieldName = SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_UNICAST_ROUTE_EN_E;
            break;

        case CPSS_IP_PROTOCOL_FCOE_E:
            return cpssDxChBrgVlanFcoeForwardingEnableSet(devNum, vlanId, enable);
            break;

        case CPSS_IP_PROTOCOL_ALL_E:
            rc = cpssDxChBrgVlanIpUcRouteEnable(devNum,vlanId,
                                                 CPSS_IP_PROTOCOL_IPV4V6_E,
                                                 enable);
            if(GT_OK != rc)
            {
                return rc;
            }
            return cpssDxChBrgVlanFcoeForwardingEnableSet(devNum, vlanId, enable);
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        return prvCpssDxChWriteTableEntryField(devNum,
                                           CPSS_DXCH_TABLE_VLAN_E,
                                           vlanId,/* vid */
                                           2,             /* word 2 */
                                           bitOffset,     /* offset */
                                           1,             /* 1 bit */
                                           hwData);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the relevant field */
        return prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        fieldName, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        hwData);
    }
    else
    {
        return prvCpssDxChWriteTableEntryField(devNum,
                                               CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                               vlanId,/* vid */
                                               PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                               globalOffset,  /* offset */
                                               1,             /* 1 bit */
                                               hwData);
    }
}

/**
* @internal cpssDxChBrgVlanIpUcRouteEnable function
* @endinternal
*
* @brief   Enable/Disable FCoE forwarding and/or IPv4/Ipv6
*          Unicast Routing on Vlan
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - Vlan ID
* @param[in] protocol                 - ipv4,ipv6 and/or fcoe
* @param[in] enable                   - GT_TRUE - enable fcoe forwarding and/or ip unicast routing;
*                                     - GT_FALSE  - disable fcoe forwarding and/or ip unicast routing;
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpUcRouteEnable
(
    IN GT_U8                                    devNum,
    IN GT_U16                                   vlanId,
    IN CPSS_IP_PROTOCOL_STACK_ENT               protocol,
    IN GT_BOOL                                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIpUcRouteEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, protocol, enable));

    rc = internal_cpssDxChBrgVlanIpUcRouteEnable(devNum, vlanId, protocol, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, protocol, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIpMcRouteEnable function
* @endinternal
*
* @brief   Enable/Disable IPv4/Ipv6 Multicast Routing on Vlan
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - Vlan ID
* @param[in] protocol                 - ipv4 or ipv6
* @param[in] enable                   - GT_TRUE -  ip multicast routing;
*                                      GT_FALSE -  disable ip multicast routing.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanIpMcRouteEnable
(
    IN GT_U8                            devNum,
    IN GT_U16                           vlanId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN GT_BOOL                          enable
)
{
    GT_U32      globalOffset;/* relevant only to splitTablesSupported */
    GT_U32      bitOffset;    /* bit offset in the VLAN Entry word */
    GT_U32      hwData;       /* data to write to register */
    GT_STATUS   rc;           /* return status */
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_ENT fieldName;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    /* when the user want to Ip routing or disable */
    hwData = BOOL2BIT_MAC(enable);
    rc = GT_OK;

    switch(protocol)
    {
        case CPSS_IP_PROTOCOL_IPV4_E :
            bitOffset = 15;
            globalOffset = 93;
            fieldName = SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_MULTICAST_ROUTE_EN_E;
            break;

        case CPSS_IP_PROTOCOL_IPV4V6_E:
            /* ipv4 */
            rc = cpssDxChBrgVlanIpMcRouteEnable(devNum,vlanId,
                                                 CPSS_IP_PROTOCOL_IPV4_E,
                                                 enable);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* ipv6 */
            return cpssDxChBrgVlanIpMcRouteEnable(devNum,vlanId,
                                                 CPSS_IP_PROTOCOL_IPV6_E,
                                                 enable);
        case CPSS_IP_PROTOCOL_IPV6_E :
            bitOffset = 17;
            globalOffset = 95;
            fieldName = SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_MULTICAST_ROUTE_EN_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                           CPSS_DXCH_TABLE_VLAN_E,
                                           vlanId,/* vid */
                                           2,             /* word 2 */
                                           bitOffset,     /* offset */
                                           1,             /* 1 bit */
                                           hwData);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the relevant field */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        fieldName, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        hwData);
    }
    else
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                               CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                               vlanId,/* vid */
                                               PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                               globalOffset,  /* offset */
                                               1,             /* 1 bit */
                                               hwData);
    }

    if(rc != GT_OK)
        return rc;

    /* If Lion2 errata is initialized set port 14 (hemisphere 0) and
       port 64+14 (hemisphere 1) as a member in the vlan */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
            info_PRV_CPSS_DXCH_LION2_IPM_BRIDGE_COPY_GET_DROPPED_WA_E.enabled == GT_TRUE)
    {
        /* add port 14 to vlan */
        rc = cpssDxChBrgVlanMemberAdd(devNum, vlanId, 14, GT_FALSE, CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E);

        if(rc != GT_OK)
            return rc;

        if (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_MAC(devNum) >= 2)
        {
            /* add port 64+14to vlan */
            rc = cpssDxChBrgVlanMemberAdd(devNum, vlanId, 78, GT_FALSE, CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E);

            if(rc != GT_OK)
                return rc;
        }

    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanIpMcRouteEnable function
* @endinternal
*
* @brief   Enable/Disable IPv4/Ipv6 Multicast Routing on Vlan
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - Vlan ID
* @param[in] protocol                 - ipv4 or ipv6
* @param[in] enable                   - GT_TRUE -  ip multicast routing;
*                                      GT_FALSE -  disable ip multicast routing.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpMcRouteEnable
(
    IN GT_U8                            devNum,
    IN GT_U16                           vlanId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN GT_BOOL                          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIpMcRouteEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, protocol, enable));

    rc = internal_cpssDxChBrgVlanIpMcRouteEnable(devNum, vlanId, protocol, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, protocol, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanNASecurEnable function
* @endinternal
*
* @brief   This function enables/disables per VLAN the generation of
*         security breach event for packets with unknown Source MAC addresses.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] enable                   - GT_TRUE -  generation of security breach event
*                                      GT_FALSE- disable generation of security breach event
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanNASecurEnable
(
    IN GT_U8           devNum,
    IN GT_U16          vlanId,
    IN GT_BOOL         enable
)
{
    GT_U32  hwData;         /* for write entry word */
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    /* when the user want to enable NA security breach on the vlan we
       clear the bit of NA NOT security breach */
    hwData = (enable == GT_TRUE) ? 0 : 1;

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* set bit 1 in word 0 accordingly */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_TABLE_VLAN_E,
                                         vlanId,/* vid */
                                         0,/* word 0 */
                                         1,/* start at bit 1*/
                                         1, /* one bit */
                                         hwData);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the <NEW_SRC_ADDR_IS_NOT_SECURITY_BREACH> */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_NEW_SRC_ADDR_IS_NOT_SECURITY_BREACH_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        hwData);
    }
    else
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                             vlanId,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                             1,
                                             1, /* 1 bit */
                                             hwData);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanNASecurEnable function
* @endinternal
*
* @brief   This function enables/disables per VLAN the generation of
*         security breach event for packets with unknown Source MAC addresses.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] enable                   - GT_TRUE -  generation of security breach event
*                                      GT_FALSE- disable generation of security breach event
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanNASecurEnable
(
    IN GT_U8           devNum,
    IN GT_U16          vlanId,
    IN GT_BOOL         enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanNASecurEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, enable));

    rc = internal_cpssDxChBrgVlanNASecurEnable(devNum, vlanId, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssDxChBrgVlanIgmpSnoopingEnable function
* @endinternal
*
* @brief   Enable/Disable IGMP trapping or mirroring to the CPU according to the
*         global setting.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - the VLAN-ID in which IGMP trapping is enabled/disabled.
* @param[in] enable                   - GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on on bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanIgmpSnoopingEnable
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_BOOL enable
)
{
    GT_U32      value;  /* value to write into register */
    GT_STATUS   rc;     /* function return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    value = BOOL2BIT_MAC(enable);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* VLAN<vlanId> Entry Word<0>, field <IPv4IGMPToCPUEn> */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                    CPSS_DXCH_TABLE_VLAN_E,
                                    vlanId,
                                    0,
                                    15,
                                    1,
                                    value);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the <IPV4_IGMP_TO_CPU_EN> */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IGMP_TO_CPU_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        value);
    }
    else
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        78,
                                        1,
                                        value);
    }


    return rc;
}

/**
* @internal cpssDxChBrgVlanIgmpSnoopingEnable function
* @endinternal
*
* @brief   Enable/Disable IGMP trapping or mirroring to the CPU according to the
*         global setting.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - the VLAN-ID in which IGMP trapping is enabled/disabled.
* @param[in] enable                   - GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on on bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIgmpSnoopingEnable
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIgmpSnoopingEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, enable));

    rc = internal_cpssDxChBrgVlanIgmpSnoopingEnable(devNum, vlanId, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIgmpSnoopingEnableGet function
* @endinternal
*
* @brief   Gets the status of IGMP trapping or mirroring to the CPU according to the
*         global setting.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - the VLAN-ID in which IGMP trapping is enabled/disabled.
*
* @param[out] enablePtr                - GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on on bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanIgmpSnoopingEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlanId,
    OUT GT_BOOL *enablePtr
)
{
    GT_U32      value;  /* value to write into register */
    GT_STATUS   rc;     /* function return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* VLAN<vlanId> Entry Word<0>, field <IPv4IGMPToCPUEn> */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                    CPSS_DXCH_TABLE_VLAN_E,
                                    vlanId,
                                    0,
                                    15,
                                    1,
                                    &value);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* get the <IPV4_IGMP_TO_CPU_EN> */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IGMP_TO_CPU_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);
    }
    else
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        78,
                                        1,
                                        &value);
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgVlanIgmpSnoopingEnableGet function
* @endinternal
*
* @brief   Gets the status of IGMP trapping or mirroring to the CPU according to the
*         global setting.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - the VLAN-ID in which IGMP trapping is enabled/disabled.
*
* @param[out] enablePtr                - GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on on bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIgmpSnoopingEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlanId,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIgmpSnoopingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, enablePtr));

    rc = internal_cpssDxChBrgVlanIgmpSnoopingEnableGet(devNum, vlanId, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIpCntlToCpuSet function
* @endinternal
*
* @brief   Enable/disable IP control traffic trapping/mirroring to CPU.
*         When set, this enables the following control traffic to be eligible
*         for mirroring/trapping to the CPU:
*         - ARP
*         - IPv6 Neighbor Solicitation
*         - IPv4/v6 Control Protocols Running Over Link-Local Multicast
*         - RIPv1 MAC Broadcast
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
* @param[in] ipCntrlType              - IP control type
*                                      DXCH devices support : CPSS_DXCH_BRG_IP_CTRL_NONE_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E
*                                      DXCH2 and above devices support: like DXCH device ,and also
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV6_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or IP control type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanIpCntlToCpuSet
(
    IN GT_U8                               devNum,
    IN GT_U16                              vlanId,
    IN CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT      ipCntrlType
)
{
    GT_U32      value;  /* value to write into register */
    GT_U32      value1; /* value to write into register */
    GT_STATUS   rc;     /* function return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    switch(ipCntrlType)
    {

        case CPSS_DXCH_BRG_IP_CTRL_NONE_E:
            value = 0;
            value1 = 0;
            break;

        case CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E:
            value = 1;
            value1 = 1;
            break;

        case CPSS_DXCH_BRG_IP_CTRL_IPV4_E:
            value = 1;
            value1 = 0;
            break;

        case CPSS_DXCH_BRG_IP_CTRL_IPV6_E:
            value = 0;
            value1 = 1;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* VLAN<vlanId> Entry Word<0>, field <IPv4ControlToCPUEnable> */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_VLAN_E,
                                            (GT_U32)vlanId,
                                            0,
                                            18,
                                            1,
                                            value);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* VLAN<vlanId> Entry Word<3>, field <IPv6ControlToCPUEnable> */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_VLAN_E,
                                            (GT_U32)vlanId,
                                            3,
                                            7,
                                            1,
                                            value1);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the <IPV4_CONTROL_TO_CPU_EN> */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_CONTROL_TO_CPU_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        value);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* set the <IPV6_CONTROL_TO_CPU_EN> */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_CONTROL_TO_CPU_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        value1);
    }
    else
    {
        /*Ipv4ControlToCpuEn*/
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                            (GT_U32)vlanId,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            83,
                                            1,
                                            value);
        if(rc != GT_OK)
        {
            return rc;
        }

        /*Ipv6ControlToCpuEn*/
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                            (GT_U32)vlanId,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            103,
                                            1,
                                            value1);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanIpCntlToCpuSet function
* @endinternal
*
* @brief   Enable/disable IP control traffic trapping/mirroring to CPU.
*         When set, this enables the following control traffic to be eligible
*         for mirroring/trapping to the CPU:
*         - ARP
*         - IPv6 Neighbor Solicitation
*         - IPv4/v6 Control Protocols Running Over Link-Local Multicast
*         - RIPv1 MAC Broadcast
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
* @param[in] ipCntrlType              - IP control type
*                                      DXCH devices support : CPSS_DXCH_BRG_IP_CTRL_NONE_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E
*                                      DXCH2 and above devices support: like DXCH device ,and also
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV6_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or IP control type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpCntlToCpuSet
(
    IN GT_U8                               devNum,
    IN GT_U16                              vlanId,
    IN CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT      ipCntrlType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIpCntlToCpuSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, ipCntrlType));

    rc = internal_cpssDxChBrgVlanIpCntlToCpuSet(devNum, vlanId, ipCntrlType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, ipCntrlType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIpCntlToCpuGet function
* @endinternal
*
* @brief   Gets IP control traffic trapping/mirroring to CPU status.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
*
* @param[out] ipCntrlTypePtr           - IP control type
*                                      DXCH devices support : CPSS_DXCH_BRG_IP_CTRL_NONE_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E
*                                      DXCH2 and above devices support: like DXCH device ,and also
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV6_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or IP control type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanIpCntlToCpuGet
(
    IN  GT_U8                               devNum,
    IN  GT_U16                              vlanId,
    OUT CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT      *ipCntrlTypePtr
)
{
    GT_U32      value=0;  /* value to read from register */
    GT_U32      value1=0; /* value to read from register */
    GT_STATUS   rc;      /* function return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);
    CPSS_NULL_PTR_CHECK_MAC(ipCntrlTypePtr);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* VLAN<vlanId> Entry Word<0>, field <IPv4ControlToCPUEnable> */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_VLAN_E,
                                            (GT_U32)vlanId,
                                            0,
                                            18,
                                            1,
                                            &value);

        if(rc != GT_OK)
            return rc;

        /* VLAN<vlanId> Entry Word<3>, field <IPv6ControlToCPUEnable> */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_TABLE_VLAN_E,
                                            (GT_U32)vlanId,
                                            3,
                                            7,
                                            1,
                                            &value1);
        if(rc != GT_OK)
            return rc;

    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* get the <IPV4_CONTROL_TO_CPU_EN> */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_CONTROL_TO_CPU_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);

        /* get the <IPV6_CONTROL_TO_CPU_EN> */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_CONTROL_TO_CPU_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value1);
    }
    else
    {
        /*Ipv4ControlToCpuEn*/
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                            (GT_U32)vlanId,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            83,
                                            1,
                                            &value);
        if(rc != GT_OK)
        {
            return rc;
        }

        /*Ipv6ControlToCpuEn*/
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                            (GT_U32)vlanId,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            103,
                                            1,
                                            &value1);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if((value == 0)&&(value1 == 0))
    {
        *ipCntrlTypePtr = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
    }
    else
    {
        if((value == 1)&&(value1 == 1))
        {
            *ipCntrlTypePtr = CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E;
        }
        else
        {
            if((value == 1)&&(value1 == 0))
            {
                *ipCntrlTypePtr = CPSS_DXCH_BRG_IP_CTRL_IPV4_E;
            }
            else/* ((value == 0)&&(value1 == 1)) */
            {
                 *ipCntrlTypePtr = CPSS_DXCH_BRG_IP_CTRL_IPV6_E;
            }
        }
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanIpCntlToCpuGet function
* @endinternal
*
* @brief   Gets IP control traffic trapping/mirroring to CPU status.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
*
* @param[out] ipCntrlTypePtr           - IP control type
*                                      DXCH devices support : CPSS_DXCH_BRG_IP_CTRL_NONE_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E
*                                      DXCH2 and above devices support: like DXCH device ,and also
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV4_E ,
*                                      CPSS_DXCH_BRG_IP_CTRL_IPV6_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or IP control type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpCntlToCpuGet
(
    IN  GT_U8                               devNum,
    IN  GT_U16                              vlanId,
    OUT CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT      *ipCntrlTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIpCntlToCpuGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, ipCntrlTypePtr));

    rc = internal_cpssDxChBrgVlanIpCntlToCpuGet(devNum, vlanId, ipCntrlTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, ipCntrlTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIpV6IcmpToCpuEnable function
* @endinternal
*
* @brief   Enable/Disable ICMPv6 trapping or mirroring to
*         the CPU, according to global ICMPv6 message type
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
* @param[in] enable                   - GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanIpV6IcmpToCpuEnable
(
    IN  GT_U8      devNum,
    IN  GT_U16     vlanId,
    IN  GT_BOOL    enable
)
{
    GT_U32      hwData;     /* data to write into register */
    GT_STATUS   rc;         /* function return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    hwData = BOOL2BIT_MAC(enable);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* VLAN<vlanId> Entry Word<0>, field <IPv6ICMPToCPUEn> */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_TABLE_VLAN_E,
                                        (GT_U32)vlanId,
                                        0,  /* word0 */
                                        17,
                                        1,
                                        hwData);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the <IPV6_ICMP_TO_CPU_EN>*/
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_ICMP_TO_CPU_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        hwData);
    }
    else
    {
        /* <Ipv6IcmpToCpuEn> */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                            (GT_U32)vlanId,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            82,
                                            1,
                                            hwData);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanIpV6IcmpToCpuEnable function
* @endinternal
*
* @brief   Enable/Disable ICMPv6 trapping or mirroring to
*         the CPU, according to global ICMPv6 message type
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
* @param[in] enable                   - GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpV6IcmpToCpuEnable
(
    IN  GT_U8      devNum,
    IN  GT_U16     vlanId,
    IN  GT_BOOL    enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIpV6IcmpToCpuEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, enable));

    rc = internal_cpssDxChBrgVlanIpV6IcmpToCpuEnable(devNum, vlanId, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIpV6IcmpToCpuEnableGet function
* @endinternal
*
* @brief   Gets status of ICMPv6 trapping or mirroring to
*         the CPU, according to global ICMPv6 message type
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
*
* @param[out] enablePtr                - GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanIpV6IcmpToCpuEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_U16     vlanId,
    OUT GT_BOOL    *enablePtr
)
{
    GT_U32      hwData;     /* data to write into register */
    GT_STATUS   rc;         /* function return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* VLAN<vlanId> Entry Word<0>, field <IPv6ICMPToCPUEn> */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_TABLE_VLAN_E,
                                        (GT_U32)vlanId,
                                        0,  /* word0 */
                                        17,
                                        1,
                                        &hwData);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* get the <IPV6_ICMP_TO_CPU_EN>*/
        rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_ICMP_TO_CPU_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwData);
    }
    else
    {
        /* <Ipv6IcmpToCpuEn> */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                            (GT_U32)vlanId,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            82,
                                            1,
                                            &hwData);
    }

    *enablePtr = BIT2BOOL_MAC(hwData);

    return rc;
}

/**
* @internal cpssDxChBrgVlanIpV6IcmpToCpuEnableGet function
* @endinternal
*
* @brief   Gets status of ICMPv6 trapping or mirroring to
*         the CPU, according to global ICMPv6 message type
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
*
* @param[out] enablePtr                - GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpV6IcmpToCpuEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_U16     vlanId,
    OUT GT_BOOL    *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIpV6IcmpToCpuEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, enablePtr));

    rc = internal_cpssDxChBrgVlanIpV6IcmpToCpuEnableGet(devNum, vlanId, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanUdpBcPktsToCpuEnable function
* @endinternal
*
* @brief   Enable or disable per Vlan the Broadcast UDP packets Mirror/Trap to the
*         CPU based on their destination UDP port. Destination UDP port, command
*         (trap or mirror) and CPU code configured by
*         cpssDxChBrgGenUdpBcDestPortCfgSet
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   -  vlan id
* @param[in] enable                   - Enable/disable UDP Trap Mirror.
*                                      GT_TRUE - enable trapping or mirroring, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanUdpBcPktsToCpuEnable
(
    IN  GT_U8       devNum,
    IN  GT_U16      vlanId,
    IN  GT_BOOL     enable
)
{
    GT_U32      hwData;     /* data to write into register */

    /* only for Cheetah2 */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    hwData = BOOL2BIT_MAC(enable);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* Enable/disable trapping or mirroring BC UDP packets based on the
        destination UDP port for UDP relay support */
        return prvCpssDxChWriteTableEntryField(devNum,
                                           CPSS_DXCH_TABLE_VLAN_E,
                                           (GT_U32)vlanId,
                                           3,   /* word3 */
                                           6,
                                           1,
                                           hwData);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the <BC_UDP_TRAP_MIRROR_EN>*/
        return prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_BC_UDP_TRAP_MIRROR_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        hwData);
    }
    else
    {
        return prvCpssDxChWriteTableEntryField(devNum,
                                               CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                               (GT_U32)vlanId,
                                               PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                               102,
                                               1,
                                               hwData);
    }
}

/**
* @internal cpssDxChBrgVlanUdpBcPktsToCpuEnable function
* @endinternal
*
* @brief   Enable or disable per Vlan the Broadcast UDP packets Mirror/Trap to the
*         CPU based on their destination UDP port. Destination UDP port, command
*         (trap or mirror) and CPU code configured by
*         cpssDxChBrgGenUdpBcDestPortCfgSet
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   -  vlan id
* @param[in] enable                   - Enable/disable UDP Trap Mirror.
*                                      GT_TRUE - enable trapping or mirroring, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanUdpBcPktsToCpuEnable
(
    IN  GT_U8       devNum,
    IN  GT_U16      vlanId,
    IN  GT_BOOL     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanUdpBcPktsToCpuEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, enable));

    rc = internal_cpssDxChBrgVlanUdpBcPktsToCpuEnable(devNum, vlanId, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanUdpBcPktsToCpuEnableGet function
* @endinternal
*
* @brief   Gets per Vlan if the Broadcast UDP packets are Mirror/Trap to the
*         CPU based on their destination UDP port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   -  vlan id
*
* @param[out] enablePtr                - UDP Trap Mirror status
*                                      GT_TRUE - enable trapping or mirroring, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanUdpBcPktsToCpuEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_U16      vlanId,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32      hwData;     /* data to write into register */

    /* only for Cheetah2 */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                           CPSS_DXCH_TABLE_VLAN_E,
                                           (GT_U32)vlanId,
                                           3,   /* word3 */
                                           6,
                                           1,
                                           &hwData);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* get the <BC_UDP_TRAP_MIRROR_EN>*/
        rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_BC_UDP_TRAP_MIRROR_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwData);
    }
    else
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                            (GT_U32)vlanId,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                            102,
                                            1,
                                            &hwData);
    }


    *enablePtr = BIT2BOOL_MAC(hwData);

    return rc;
}

/**
* @internal cpssDxChBrgVlanUdpBcPktsToCpuEnableGet function
* @endinternal
*
* @brief   Gets per Vlan if the Broadcast UDP packets are Mirror/Trap to the
*         CPU based on their destination UDP port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   -  vlan id
*
* @param[out] enablePtr                - UDP Trap Mirror status
*                                      GT_TRUE - enable trapping or mirroring, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanUdpBcPktsToCpuEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_U16      vlanId,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanUdpBcPktsToCpuEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, enablePtr));

    rc = internal_cpssDxChBrgVlanUdpBcPktsToCpuEnableGet(devNum, vlanId, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIpv6SourceSiteIdSet function
* @endinternal
*
* @brief   Sets a vlan ipv6 site id
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - the vlan.
* @param[in] siteId                   - the site id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
static GT_STATUS internal_cpssDxChBrgVlanIpv6SourceSiteIdSet
(
    IN GT_U8                                devNum,
    IN GT_U16                               vlanId,
    IN CPSS_IP_SITE_ID_ENT                  siteId
)
{
    GT_U32      hwData;         /* data to write to register */
    GT_STATUS   rc;     /* status to return */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    if(siteId != CPSS_IP_SITE_ID_INTERNAL_E &&
       siteId != CPSS_IP_SITE_ID_EXTERNAL_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* when the user want to Ip routing or disable */
    hwData = (siteId == CPSS_IP_SITE_ID_INTERNAL_E) ? 0 : 1;

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_TABLE_VLAN_E,
                                         (GT_U32)vlanId,/* vid */
                                         0,             /* word 0 */
                                         23,            /* bit 23*/
                                         1,             /* one bit */
                                         hwData);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the <IPV6_SITEID>*/
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_SITEID_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        hwData);
    }
    else
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                             (GT_U32)vlanId,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                             96,
                                             1,             /* one bit */
                                             hwData);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanIpv6SourceSiteIdSet function
* @endinternal
*
* @brief   Sets a vlan ipv6 site id
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - the vlan.
* @param[in] siteId                   - the site id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
GT_STATUS cpssDxChBrgVlanIpv6SourceSiteIdSet
(
    IN GT_U8                                devNum,
    IN GT_U16                               vlanId,
    IN CPSS_IP_SITE_ID_ENT                  siteId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIpv6SourceSiteIdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, siteId));

    rc = internal_cpssDxChBrgVlanIpv6SourceSiteIdSet(devNum, vlanId, siteId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, siteId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIpmBridgingEnable function
* @endinternal
*
* @brief   IPv4/Ipv6 Multicast Bridging Enable
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device id
* @param[in] vlanId                   - Vlan Id
* @param[in] ipVer                    - IPv4 or Ipv6
* @param[in] enable                   - GT_TRUE - IPv4 multicast packets are bridged
*                                      according to ipv4IpmBrgMode
*                                      GT_FALSE - IPv4 multicast packets are bridged
*                                      according to MAC DA
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat3, Caelum, Aldrin, AC3X and Bobcat3 devices, this API also takes part
*       in enabling the IP multicast routing by VLAN feature according to the
*       following logic:
*       BrgEnable  |     BrgMode     |  Bridging  |  Routing
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_SGV_E (0) |  MAC DA   |  VRF-ID
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_GV_E (1) |  MAC DA   |   VID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_SGV_E (0) | (S,G,V)   |  VRF-ID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_GV_E (1) | (,G,V)   |  VRF-ID
*       explanation for the table:
*       'BrgEnable' - set by this API (per VLAN, per IP protocol).
*       'BrgMode' - set by cpssDxChBrgVlanIpmBridgingModeSet (per VLAN, per IP protocol).
*       'Bridging' - multicast bridging mode, relevant to all devices.
*       'Routing' - multicast routing mode, relevant only to xCat3, Caelum, Aldrin, AC3X and Bobcat3
*       devices. There are 2 possible vrf-id assignments for the ingress packet descriptor:
*       1. VRF-ID: IP multicast routing by VLAN feature is off and vrf-id is assigned according
*       to the VRF-ID field in the VLAN entry.
*       2. VID: IP multicast routing by VLAN feature is on and vrf-id is assigned to be the
*       ingress packet's VID itself.
*       In devices that do not support IP multicast routing by VLAN feature, routing mode is always VRF-ID.
*
*/
static GT_STATUS internal_cpssDxChBrgVlanIpmBridgingEnable
(
    IN  GT_U8                       devNum,
    IN  GT_U16                      vlanId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
    IN  GT_BOOL                     enable
)
{
    GT_U32  bitOffset;      /* bit offset in the VLAN Entry word */
    GT_U32  dataLength;     /* length of write data */
    GT_U32  hwData;         /* data to write to register */
    GT_U32  globalOffset;/* relevant only to splitTablesSupported */
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_ENT fieldName;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    hwData = BOOL2BIT_MAC(enable);

    switch (ipVer)
    {
        case CPSS_IP_PROTOCOL_IPV6_E:
            bitOffset = 22;
            dataLength = 1;
            globalOffset = 85;
            fieldName = SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_IPM_BRIDGING_EN_E;
            break;
        case CPSS_IP_PROTOCOL_IPV4_E:
            bitOffset = 21;
            dataLength = 1;
            globalOffset = 84;
            fieldName = SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IPM_BRIDGING_EN_E;
            break;
        case CPSS_IP_PROTOCOL_IPV4V6_E:
            bitOffset = 21;
            dataLength = 2;
            hwData = hwData | (hwData << 1);
            globalOffset = 84;
            fieldName = SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IPM_BRIDGING_EN_E;/*2 fields from this one */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        return prvCpssDxChWriteTableEntryField(devNum,
                                           CPSS_DXCH_TABLE_VLAN_E,
                                           (GT_U32)vlanId, /* vid    */
                                           0,              /* word 0 */
                                           bitOffset,      /* offset */
                                           dataLength,
                                           hwData);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the relevant field(s)*/
        return prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        fieldName, /* field name */
                                        dataLength,/* indication for 1 or 2 fields */
                                        hwData);
    }
    else
    {
        return prvCpssDxChWriteTableEntryField(devNum,
                                               CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                               (GT_U32)vlanId,
                                               PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                               globalOffset,
                                               dataLength,
                                               hwData);
    }
}

/**
* @internal cpssDxChBrgVlanIpmBridgingEnable function
* @endinternal
*
* @brief   IPv4/Ipv6 Multicast Bridging Enable
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device id
* @param[in] vlanId                   - Vlan Id
* @param[in] ipVer                    - IPv4 or Ipv6
* @param[in] enable                   - GT_TRUE - IPv4 multicast packets are bridged
*                                      according to ipv4IpmBrgMode
*                                      GT_FALSE - IPv4 multicast packets are bridged
*                                      according to MAC DA
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat3, Caelum, Aldrin, AC3X and Bobcat3 devices, this API also takes part
*       in enabling the IP multicast routing by VLAN feature according to the
*       following logic:
*       BrgEnable  |     BrgMode     |  Bridging  |  Routing
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_SGV_E (0) |  MAC DA   |  VRF-ID
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_GV_E (1) |  MAC DA   |   VID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_SGV_E (0) | (S,G,V)   |  VRF-ID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_GV_E (1) | (,G,V)   |  VRF-ID
*       explanation for the table:
*       'BrgEnable' - set by this API (per VLAN, per IP protocol).
*       'BrgMode' - set by cpssDxChBrgVlanIpmBridgingModeSet (per VLAN, per IP protocol).
*       'Bridging' - multicast bridging mode, relevant to all devices.
*       'Routing' - multicast routing mode, relevant only to xCat3, Caelum, Aldrin, AC3X and Bobcat3
*       devices. There are 2 possible vrf-id assignments for the ingress packet descriptor:
*       1. VRF-ID: IP multicast routing by VLAN feature is off and vrf-id is assigned according
*       to the VRF-ID field in the VLAN entry.
*       2. VID: IP multicast routing by VLAN feature is on and vrf-id is assigned to be the
*       ingress packet's VID itself.
*       In devices that do not support IP multicast routing by VLAN feature, routing mode is always VRF-ID.
*
*/
GT_STATUS cpssDxChBrgVlanIpmBridgingEnable
(
    IN  GT_U8                       devNum,
    IN  GT_U16                      vlanId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
    IN  GT_BOOL                     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIpmBridgingEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, ipVer, enable));

    rc = internal_cpssDxChBrgVlanIpmBridgingEnable(devNum, vlanId, ipVer, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, ipVer, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIpmBridgingModeSet function
* @endinternal
*
* @brief   Sets the IPM bridging mode of Vlan. Relevant when IPM Bridging enabled.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device id
* @param[in] vlanId                   - Vlan Id
* @param[in] ipVer                    - IPv4 or Ipv6
* @param[in] ipmMode                  - IPM bridging mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat3, Caelum, Aldrin, AC3X and Bobcat3 devices, this API also takes part
*       in enabling the IP multicast routing by VLAN feature according to the
*       following logic:
*       BrgEnable  |     BrgMode     |  Bridging  |  Routing
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_SGV_E (0) |  MAC DA   |  VRF-ID
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_GV_E (1) |  MAC DA   |   VID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_SGV_E (0) | (S,G,V)   |  VRF-ID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_GV_E (1) | (,G,V)   |  VRF-ID
*       explanation for the table:
*       'BrgEnable' - set by cpssDxChBrgVlanIpmBridgingEnable (per VLAN, per IP protocol).
*       'BrgMode' - set by this API (per VLAN, per IP protocol).
*       'Bridging' - multicast bridging mode, relevant to all devices.
*       'Routing' - multicast routing mode, relevant only to xCat3, Caelum, Aldrin, AC3X and Bobcat3
*       devices. There are 2 possible vrf-id assignments for the ingress packet descriptor:
*       1. VRF-ID: IP multicast routing by VLAN feature is off and vrf-id is assigned according
*       to the VRF-ID field in the VLAN entry.
*       2. VID: IP multicast routing by VLAN feature is on and vrf-id is assigned to be the
*       ingress packet's VID itself.
*       In devices that do not support IP multicast routing by VLAN feature, routing mode is always VRF-ID.
*
*/
static GT_STATUS internal_cpssDxChBrgVlanIpmBridgingModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_U16                      vlanId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
    IN  CPSS_BRG_IPM_MODE_ENT       ipmMode
)
{
    GT_U32  bitOffset;      /* bit offset in the VLAN Entry word */
    GT_U32  dataLength;     /* length of write data */
    GT_U32  hwData;         /* data to write to register */
    GT_U32  globalOffset;/* relevant only to splitTablesSupported */
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_ENT fieldName;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    switch (ipmMode)
    {
        case CPSS_BRG_IPM_SGV_E:
            hwData = 0;
            break;
        case CPSS_BRG_IPM_GV_E:
            hwData = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (ipVer)
    {
        case CPSS_IP_PROTOCOL_IPV6_E:
            bitOffset = 20;
            dataLength = 1;
            globalOffset = 80;
            fieldName = SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_IPM_BRIDGING_MODE_E;
            break;
        case CPSS_IP_PROTOCOL_IPV4_E:
            bitOffset = 19;
            dataLength = 1;
            globalOffset = 79;
            fieldName = SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IPM_BRIDGING_MODE_E;
            break;
        case CPSS_IP_PROTOCOL_IPV4V6_E:
            bitOffset = 19;
            dataLength = 2;
            hwData = hwData | (hwData << 1);
            globalOffset = 79;
            fieldName = SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IPM_BRIDGING_MODE_E;/*2 fields from this one */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        return prvCpssDxChWriteTableEntryField(devNum,
                                           CPSS_DXCH_TABLE_VLAN_E,
                                           (GT_U32)vlanId,  /* vid    */
                                           0,               /* word 0 */
                                           bitOffset,       /* offset */
                                           dataLength,
                                           hwData);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the relevant field(s)*/
        return prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        fieldName, /* field name */
                                        dataLength,/* indication for 1 or 2 fields */
                                        hwData);
    }
    else
    {
        return prvCpssDxChWriteTableEntryField(devNum,
                                               CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                               (GT_U32)vlanId,
                                               PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                               globalOffset,
                                               dataLength,
                                               hwData);
    }
}

/**
* @internal cpssDxChBrgVlanIpmBridgingModeSet function
* @endinternal
*
* @brief   Sets the IPM bridging mode of Vlan. Relevant when IPM Bridging enabled.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device id
* @param[in] vlanId                   - Vlan Id
* @param[in] ipVer                    - IPv4 or Ipv6
* @param[in] ipmMode                  - IPM bridging mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat3, Caelum, Aldrin, AC3X and Bobcat3 devices, this API also takes part
*       in enabling the IP multicast routing by VLAN feature according to the
*       following logic:
*       BrgEnable  |     BrgMode     |  Bridging  |  Routing
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_SGV_E (0) |  MAC DA   |  VRF-ID
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_GV_E (1) |  MAC DA   |   VID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_SGV_E (0) | (S,G,V)   |  VRF-ID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_GV_E (1) | (,G,V)   |  VRF-ID
*       explanation for the table:
*       'BrgEnable' - set by cpssDxChBrgVlanIpmBridgingEnable (per VLAN, per IP protocol).
*       'BrgMode' - set by this API (per VLAN, per IP protocol).
*       'Bridging' - multicast bridging mode, relevant to all devices.
*       'Routing' - multicast routing mode, relevant only to xCat3, Caelum, Aldrin, AC3X and Bobcat3
*       devices. There are 2 possible vrf-id assignments for the ingress packet descriptor:
*       1. VRF-ID: IP multicast routing by VLAN feature is off and vrf-id is assigned according
*       to the VRF-ID field in the VLAN entry.
*       2. VID: IP multicast routing by VLAN feature is on and vrf-id is assigned to be the
*       ingress packet's VID itself.
*       In devices that do not support IP multicast routing by VLAN feature, routing mode is always VRF-ID.
*
*/
GT_STATUS cpssDxChBrgVlanIpmBridgingModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_U16                      vlanId,
    IN  CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
    IN  CPSS_BRG_IPM_MODE_ENT       ipmMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIpmBridgingModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, ipVer, ipmMode));

    rc = internal_cpssDxChBrgVlanIpmBridgingModeSet(devNum, vlanId, ipVer, ipmMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, ipVer, ipmMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIngressMirrorEnable function
* @endinternal
*
* @brief   Enable/Disable ingress mirroring of packets to the Ingress Analyzer
*         port for packets assigned the given VLAN-ID.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device id
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
* @param[in] enable                   - GT_TRUE - Ingress mirrored traffic assigned to
*                                      this Vlan to the analyzer port
*                                      GT_FALSE - Don't ingress mirrored traffic assigned
*                                      to this Vlan to the analyzer port
* @param[in] index                    - Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanIngressMirrorEnable
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_BOOL enable,
    IN GT_U32  index
)
{
    GT_U32      hwData;   /* data to write to Hw */
    GT_STATUS   rc;      /* function call return value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        hwData = BOOL2BIT_MAC(enable);

        rc = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_TABLE_VLAN_E,
                                         (GT_U32)vlanId,
                                         0,
                                         16,
                                         1,
                                         hwData);
    }
    else
    {
        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
        {
            if(enable == GT_FALSE) /* ignore index and set index 0 */
            {
                hwData = 0;
            }
            else
            {
                if(index >  PRV_CPSS_DXCH_MIRROR_TO_ANALYZER_MAX_INDEX_CNS)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                }
                /* the index in the API is 0 based , but in HW it is 1 based
                   because in HW 0 means 'no mirror' */
                hwData = index + 1;
            }

            /* set the <MIRROR_TO_INGRESS_ANALYZER>*/
            rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                            vlanId,/* vid */
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_MIRROR_TO_INGRESS_ANALYZER_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            hwData);
        }
        else
        {
            hwData = BOOL2BIT_MAC(enable);

            rc = prvCpssDxChWriteTableEntryField(devNum,
                                                 CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                                 (GT_U32)vlanId,
                                                 PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                                 81,
                                                 1,
                                                 hwData);
        }
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanIngressMirrorEnable function
* @endinternal
*
* @brief   Enable/Disable ingress mirroring of packets to the Ingress Analyzer
*         port for packets assigned the given VLAN-ID.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device id
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
* @param[in] enable                   - GT_TRUE - Ingress mirrored traffic assigned to
*                                      this Vlan to the analyzer port
*                                      GT_FALSE - Don't ingress mirrored traffic assigned
*                                      to this Vlan to the analyzer port
* @param[in] index                    - Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIngressMirrorEnable
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_BOOL enable,
    IN GT_U32  index
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIngressMirrorEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, enable, index));

    rc = internal_cpssDxChBrgVlanIngressMirrorEnable(devNum, vlanId, enable, index);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, enable, index));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortAccFrameTypeSet function
* @endinternal
*
* @brief   Set port access frame type.
*         There are three types of port configuration:
*         - Admit only Vlan tagged frames
*         - Admit only tagged and priority tagged,
*         - Admit all frames.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] frameType                -
*                                      CPSS_PORT_ACCEPT_FRAME_TAGGED_E    -
*                                      All Untagged/Priority Tagged packets received on
*                                      this port are discarded. Only Tagged accepted.
*                                      CPSS_PORT_ACCEPT_FRAME_ALL_E       -
*                                      Both Tagged and Untagged packets are accepted
*                                      on the port.
*                                      CPSS_PORT_ACCEPT_FRAME_UNTAGGED_E  -
*                                      All Tagged packets received on this port are
*                                      discarded. Only Untagged/Priority Tagged accepted.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortAccFrameTypeSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_PORT_ACCEPT_FRAME_TYPE_ENT     frameType
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;    /* register address */
    GT_U32      hwData;     /* data to write to register */
    GT_U32      portGroupId;/* the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    switch (frameType)
    {
        case CPSS_PORT_ACCEPT_FRAME_ALL_E:
            hwData = 0;
            break;
        case CPSS_PORT_ACCEPT_FRAME_TAGGED_E:
            hwData = 1;
            break;
        case CPSS_PORT_ACCEPT_FRAME_UNTAGGED_E:
            hwData = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the <ACCEPT_FRAME_TYPE>*/
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                       CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_EPORT_TABLE_FIELDS_ACCEPT_FRAME_TYPE_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        hwData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

        /* get address of Protocols Encapsulation Register */
        regAddr = PRV_CPSS_DEV_BRG_PORT_CONFIG_1_REGADDR_PTR(devNum,portNum);

        /* Set port access frame type. */
        rc = prvCpssHwPpPortGroupSetRegField(devNum,portGroupId, regAddr, 21, 2, hwData);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortAccFrameTypeSet function
* @endinternal
*
* @brief   Set port access frame type.
*         There are three types of port configuration:
*         - Admit only Vlan tagged frames
*         - Admit only tagged and priority tagged,
*         - Admit all frames.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] frameType                -
*                                      CPSS_PORT_ACCEPT_FRAME_TAGGED_E    -
*                                      All Untagged/Priority Tagged packets received on
*                                      this port are discarded. Only Tagged accepted.
*                                      CPSS_PORT_ACCEPT_FRAME_ALL_E       -
*                                      Both Tagged and Untagged packets are accepted
*                                      on the port.
*                                      CPSS_PORT_ACCEPT_FRAME_UNTAGGED_E  -
*                                      All Tagged packets received on this port are
*                                      discarded. Only Untagged/Priority Tagged accepted.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortAccFrameTypeSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  CPSS_PORT_ACCEPT_FRAME_TYPE_ENT     frameType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortAccFrameTypeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, frameType));

    rc = internal_cpssDxChBrgVlanPortAccFrameTypeSet(devNum, portNum, frameType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, frameType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortAccFrameTypeGet function
* @endinternal
*
* @brief   Get port access frame type.
*         There are three types of port configuration:
*         - Admit only Vlan tagged frames
*         - Admit only tagged and priority tagged,
*         - Admit all frames.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] frameTypePtr             -
*                                      CPSS_PORT_ACCEPT_FRAME_TAGGED_E    -
*                                      All Untagged/Priority Tagged packets received on
*                                      this port are discarded. Only Tagged accepted.
*                                      CPSS_PORT_ACCEPT_FRAME_ALL_E       -
*                                      Both Tagged and Untagged packets are accepted
*                                      on the port.
*                                      CPSS_PORT_ACCEPT_FRAME_UNTAGGED_E  -
*                                      All Tagged packets received on this port are
*                                      discarded. Only Untagged/Priority Tagged accepted.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortAccFrameTypeGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT CPSS_PORT_ACCEPT_FRAME_TYPE_ENT     *frameTypePtr
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      hwData;    /* data to write to register */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    GT_STATUS   rc;     /* return code  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(frameTypePtr);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* get the <ACCEPT_FRAME_TYPE>*/
        rc = prvCpssDxChReadTableEntryField(devNum,
                                       CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_EPORT_TABLE_FIELDS_ACCEPT_FRAME_TYPE_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwData);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

        /* get address of Protocols Encapsulation Register */
        regAddr = PRV_CPSS_DEV_BRG_PORT_CONFIG_1_REGADDR_PTR(devNum,portNum);

        /* Set port access frame type. */
        rc = prvCpssHwPpPortGroupGetRegField(devNum,portGroupId, regAddr, 21, 2, &hwData);
    }

    if(rc != GT_OK)
        return rc;

    switch (hwData)
    {
        case 0:
            *frameTypePtr = CPSS_PORT_ACCEPT_FRAME_ALL_E;
            break;
        case 1:
            *frameTypePtr = CPSS_PORT_ACCEPT_FRAME_TAGGED_E;
            break;
        case 2:
            *frameTypePtr = CPSS_PORT_ACCEPT_FRAME_UNTAGGED_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    return GT_OK;

}

/**
* @internal cpssDxChBrgVlanPortAccFrameTypeGet function
* @endinternal
*
* @brief   Get port access frame type.
*         There are three types of port configuration:
*         - Admit only Vlan tagged frames
*         - Admit only tagged and priority tagged,
*         - Admit all frames.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] frameTypePtr             -
*                                      CPSS_PORT_ACCEPT_FRAME_TAGGED_E    -
*                                      All Untagged/Priority Tagged packets received on
*                                      this port are discarded. Only Tagged accepted.
*                                      CPSS_PORT_ACCEPT_FRAME_ALL_E       -
*                                      Both Tagged and Untagged packets are accepted
*                                      on the port.
*                                      CPSS_PORT_ACCEPT_FRAME_UNTAGGED_E  -
*                                      All Tagged packets received on this port are
*                                      discarded. Only Untagged/Priority Tagged accepted.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortAccFrameTypeGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT CPSS_PORT_ACCEPT_FRAME_TYPE_ENT     *frameTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortAccFrameTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, frameTypePtr));

    rc = internal_cpssDxChBrgVlanPortAccFrameTypeGet(devNum, portNum, frameTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, frameTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanMruProfileIdxSet function
* @endinternal
*
* @brief   Set Maximum Receive Unit MRU profile index for a VLAN.
*         MRU VLAN profile sets maximum packet size that can be received
*         for the given VLAN.
*         Value of MRU for profile is set by cpssDxChBrgVlanMruProfileValueSet.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - vlan id
* @param[in] mruIndex                 - MRU profile index (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanMruProfileIdxSet
(
    IN GT_U8     devNum,
    IN GT_U16    vlanId,
    IN GT_U32    mruIndex
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);
    PRV_CPSS_DXCH_BRG_MRU_INDEX_CHECK_MAC(mruIndex);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        /* Set Maximum Receive Unit MRU profile index for the VLAN */
        return prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_TABLE_VLAN_E,
                                         (GT_U32)vlanId,/* vid */
                                         3,/* start at word 3 */
                                         2,/* start at bit 2*/
                                         3, /* 3 bits */
                                         mruIndex);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the <MRU_INDEX>*/
        return prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_MRU_INDEX_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        mruIndex);
    }
    else
    {
        return prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                             (GT_U32)vlanId,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                             99,
                                             3, /* 3 bits */
                                             mruIndex);
    }

}

/**
* @internal cpssDxChBrgVlanMruProfileIdxSet function
* @endinternal
*
* @brief   Set Maximum Receive Unit MRU profile index for a VLAN.
*         MRU VLAN profile sets maximum packet size that can be received
*         for the given VLAN.
*         Value of MRU for profile is set by cpssDxChBrgVlanMruProfileValueSet.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - vlan id
* @param[in] mruIndex                 - MRU profile index (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanMruProfileIdxSet
(
    IN GT_U8     devNum,
    IN GT_U16    vlanId,
    IN GT_U32    mruIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanMruProfileIdxSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, mruIndex));

    rc = internal_cpssDxChBrgVlanMruProfileIdxSet(devNum, vlanId, mruIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, mruIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanMruProfileValueSet function
* @endinternal
*
* @brief   Set MRU value for a VLAN MRU profile.
*         MRU VLAN profile sets maximum packet size that can be received
*         for the given VLAN.
*         cpssDxChBrgVlanMruProfileIdxSet set index of profile for a specific VLAN.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mruIndex                 - MRU profile index (APPLICABLE RANGES: 0..7)
* @param[in] mruValue                 - MRU value in bytes
*                                      (APPLICABLE RANGES: xCat3, AC5, Lion2 0..0xFFFF;
*                                      Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X  0..0x3FFF)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanMruProfileValueSet
(
    IN GT_U8     devNum,
    IN GT_U32    mruIndex,
    IN GT_U32    mruValue
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      fieldOffset;    /* field offset of the data in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_BRG_MRU_INDEX_CHECK_MAC(mruIndex);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(mruValue >= BIT_14)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_FALCON_MRU_BRIDGE_EVLAN_WA_E)) &&
           (!PRV_CPSS_PP_MAC(devNum)->isGmDevice))
        {
            if (mruValue < 4)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* reduce MRU to 4 bytes. GM does not have such problem. */
            mruValue -= 4;
        }

        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    VLANMRUProfilesConfig.VLANMRUProfileConfig[mruIndex / 2];
    }
    else
    {
        if(mruValue >= BIT_16)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.vlanMruProfilesConfigReg;

        regAddr = regAddr + (mruIndex / 2) * 0x4;
    }
    fieldOffset = (mruIndex % 2) * 16;

    /* Set Maximum Receive Unit MRU profile value for the index */
    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset,
                                     16, mruValue);

}

/**
* @internal cpssDxChBrgVlanMruProfileValueSet function
* @endinternal
*
* @brief   Set MRU value for a VLAN MRU profile.
*         MRU VLAN profile sets maximum packet size that can be received
*         for the given VLAN.
*         cpssDxChBrgVlanMruProfileIdxSet set index of profile for a specific VLAN.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mruIndex                 - MRU profile index (APPLICABLE RANGES: 0..7)
* @param[in] mruValue                 - MRU value in bytes
*                                      (APPLICABLE RANGES: xCat3, AC5, Lion2 0..0xFFFF;
*                                      Bobcat2, Caelum, Bobcat3, Aldrin2, Aldrin, AC3X  0..0x3FFF)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanMruProfileValueSet
(
    IN GT_U8     devNum,
    IN GT_U32    mruIndex,
    IN GT_U32    mruValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanMruProfileValueSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mruIndex, mruValue));

    rc = internal_cpssDxChBrgVlanMruProfileValueSet(devNum, mruIndex, mruValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mruIndex, mruValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanMruProfileValueGet function
* @endinternal
*
* @brief   Get MRU value for a VLAN MRU profile.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mruIndex                 - MRU profile index (APPLICABLE RANGES: 0..7)
*
* @param[out] mruValuePtr              - (pointer to) MRU value in bytes
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChBrgVlanMruProfileValueGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    mruIndex,
    OUT GT_U32    *mruValuePtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;        /* register address */
    GT_U32      fieldOffset;    /* field offset of the data in the register */
    GT_U32      value;          /* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_BRG_MRU_INDEX_CHECK_MAC(mruIndex);
    CPSS_NULL_PTR_CHECK_MAC(mruValuePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    VLANMRUProfilesConfig.VLANMRUProfileConfig[mruIndex / 2];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.vlanMruProfilesConfigReg;

        regAddr = regAddr + (mruIndex / 2) * 0x4;
    }

    fieldOffset = (mruIndex % 2) * 16;

    /* Set Maximum Receive Unit MRU profile value for the index */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset,
                                     16, &value);
    if(GT_OK != rc)
    {
        return rc;
    }

    if((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_FALCON_MRU_BRIDGE_EVLAN_WA_E)) &&
       (!PRV_CPSS_PP_MAC(devNum)->isGmDevice))
    {

        /* increase MRU by 4 bytes. GM does not have such problem. */
        value += 4;
    }

    *mruValuePtr = value;

    return rc;
}

/**
* @internal cpssDxChBrgVlanMruProfileValueGet function
* @endinternal
*
* @brief   Get MRU value for a VLAN MRU profile.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mruIndex                 - MRU profile index (APPLICABLE RANGES: 0..7)
*
* @param[out] mruValuePtr              - (pointer to) MRU value in bytes
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgVlanMruProfileValueGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    mruIndex,
    OUT GT_U32    *mruValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanMruProfileValueGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mruIndex, mruValuePtr));

    rc = internal_cpssDxChBrgVlanMruProfileValueGet(devNum, mruIndex, mruValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mruIndex, mruValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgVlanNaToCpuEnable function
* @endinternal
*
* @brief   Enable/Disable New Address (NA) Message Sending to CPU per VLAN .
*         To send NA to CPU both VLAN and port must be set to send NA to CPU.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - vlan id
* @param[in] enable                   - GT_TRUE  -  New Address Message Sending to CPU
*                                      GT_FALSE - disable New Address Message Sending to CPU
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanNaToCpuEnable
(
    IN GT_U8     devNum,
    IN GT_U16    vlanId,
    IN GT_BOOL   enable
)
{
    GT_U32      hwData; /* data to write to register */
    GT_STATUS   rc;     /* return status */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    hwData = BOOL2BIT_MAC(enable);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_TABLE_VLAN_E,
                                         (GT_U32)vlanId,
                                         3, /* word 3 */
                                         1, /* start at bit 1 */
                                         1, /* 1 bit */
                                         hwData);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the <NA_MSG_TO_CPU_EN>*/
        return prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_NA_MSG_TO_CPU_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        hwData);
    }
    else
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                             (GT_U32)vlanId,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                             98,
                                             1, /* 1 bit */
                                             hwData);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanNaToCpuEnable function
* @endinternal
*
* @brief   Enable/Disable New Address (NA) Message Sending to CPU per VLAN .
*         To send NA to CPU both VLAN and port must be set to send NA to CPU.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - vlan id
* @param[in] enable                   - GT_TRUE  -  New Address Message Sending to CPU
*                                      GT_FALSE - disable New Address Message Sending to CPU
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanNaToCpuEnable
(
    IN GT_U8     devNum,
    IN GT_U16    vlanId,
    IN GT_BOOL   enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanNaToCpuEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, enable));

    rc = internal_cpssDxChBrgVlanNaToCpuEnable(devNum, vlanId, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanVrfIdSet function
* @endinternal
*
* @brief   Sets vlan Virtual Router ID
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - the vlan id.
* @param[in] vrfId                    - the virtual router id (APPLICABLE RANGES: 0..4095).
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_OUT_OF_RANGE          - when vrfId is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
static GT_STATUS internal_cpssDxChBrgVlanVrfIdSet
(
    IN GT_U8                   devNum,
    IN GT_U16                  vlanId,
    IN GT_U32                  vrfId
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    /* check vrf ID */
    if(vrfId >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.vrfIdNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* write TTI VLAN entry (vrf-Id field)*/
        return prvCpssDxChWriteTableEntry(devNum,
                        CPSS_DXCH_SIP6_TABLE_TTI_VLAN_TO_VRF_ID_TABLE_E,
                        vlanId,
                        &vrfId);
    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        return prvCpssDxChWriteTableEntryField(devNum,
                                          CPSS_DXCH3_TABLE_VRF_ID_E,
                                          (GT_U32)vlanId,
                                          0, /* word 0 */
                                          0,
                                          12, /* 12 bits */
                                          vrfId);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the <VRF_ID>*/
        return prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_VRF_ID_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        vrfId);
    }
    else
    {
        return prvCpssDxChWriteTableEntryField(devNum,
                                              CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                              (GT_U32)vlanId,
                                              PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                              116,
                                              12, /* 12 bits */
                                              vrfId);
    }
}

/**
* @internal cpssDxChBrgVlanVrfIdSet function
* @endinternal
*
* @brief   Sets vlan Virtual Router ID
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - the vlan id.
* @param[in] vrfId                    - the virtual router id (APPLICABLE RANGES: 0..4095).
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_OUT_OF_RANGE          - when vrfId is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
GT_STATUS cpssDxChBrgVlanVrfIdSet
(
    IN GT_U8                   devNum,
    IN GT_U16                  vlanId,
    IN GT_U32                  vrfId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanVrfIdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, vrfId));

    rc = internal_cpssDxChBrgVlanVrfIdSet(devNum, vlanId, vrfId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, vrfId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanBridgingModeSet function
* @endinternal
*
* @brief   Set bridging mode either VLAN-aware (802.1Q) or VLAN-unaware (802.1D) mode
*         The device supports a VLAN-unaware mode for 802.1D bridging.
*         When this mode is enabled:
*         - In VLAN-unaware mode, the device does not perform any packet
*         modifications. Packets are always transmitted as-received, without any
*         modification (i.e., packets received tagged are transmitted tagged;
*         packets received untagged are transmitted untagged).
*         - Packets are implicitly assigned with VLAN-ID 1, regardless of
*         VLAN-assignment mechanisms.
*         - Packets are implicitly assigned a VIDX Multicast group index 0xFFF,
*         indicating that the packet flood domain is according to the VLAN-in
*         this case VLAN 1. Registered Multicast is not supported in this mode.
*         All other features are operational in this mode, including internal
*         packet QoS, trapping, filtering, policy, etc.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] brgMode                  - bridge mode: IEEE 802.1Q bridge or IEEE 802.1D bridge
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanBridgingModeSet
(
    IN GT_U8                devNum,
    IN CPSS_BRG_MODE_ENT    brgMode
)
{
    GT_U32      hwData;     /* data to write to register */
    GT_U32      regAddr;    /* hw register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    switch (brgMode)
    {
        case CPSS_BRG_MODE_802_1Q_E:
            hwData = 0;
            break;
        case CPSS_BRG_MODE_802_1D_E:
            hwData = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }

    /* get address of Global Control Register */
    regAddr =
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.globalControl;

    /* set Vlan - unaware mode in Global Control Register */
    return prvCpssHwPpSetRegField(devNum, regAddr, 3, 1, hwData);
}

/**
* @internal cpssDxChBrgVlanBridgingModeSet function
* @endinternal
*
* @brief   Set bridging mode either VLAN-aware (802.1Q) or VLAN-unaware (802.1D) mode
*         The device supports a VLAN-unaware mode for 802.1D bridging.
*         When this mode is enabled:
*         - In VLAN-unaware mode, the device does not perform any packet
*         modifications. Packets are always transmitted as-received, without any
*         modification (i.e., packets received tagged are transmitted tagged;
*         packets received untagged are transmitted untagged).
*         - Packets are implicitly assigned with VLAN-ID 1, regardless of
*         VLAN-assignment mechanisms.
*         - Packets are implicitly assigned a VIDX Multicast group index 0xFFF,
*         indicating that the packet flood domain is according to the VLAN-in
*         this case VLAN 1. Registered Multicast is not supported in this mode.
*         All other features are operational in this mode, including internal
*         packet QoS, trapping, filtering, policy, etc.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] brgMode                  - bridge mode: IEEE 802.1Q bridge or IEEE 802.1D bridge
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanBridgingModeSet
(
    IN GT_U8                devNum,
    IN CPSS_BRG_MODE_ENT    brgMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanBridgingModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, brgMode));

    rc = internal_cpssDxChBrgVlanBridgingModeSet(devNum, brgMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, brgMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanBridgingModeGet function
* @endinternal
*
* @brief   Get bridging mode either VLAN-aware (802.1Q) or VLAN-unaware (802.1D) mode
*         The device supports a VLAN-unaware mode for 802.1D bridging.
*         When this mode is enabled:
*         - In VLAN-unaware mode, the device does not perform any packet
*         modifications. Packets are always transmitted as-received, without any
*         modification (i.e., packets received tagged are transmitted tagged;
*         packets received untagged are transmitted untagged).
*         - Packets are implicitly assigned with VLAN-ID 1, regardless of
*         VLAN-assignment mechanisms.
*         - Packets are implicitly assigned a VIDX Multicast group index 0xFFF,
*         indicating that the packet flood domain is according to the VLAN-in
*         this case VLAN 1. Registered Multicast is not supported in this mode.
*         All other features are operational in this mode, including internal
*         packet QoS, trapping, filtering, policy, etc.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] brgModePtr               - (pointer to) bridge mode: IEEE 802.1Q bridge or IEEE 802.1D bridge
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssDxChBrgVlanBridgingModeGet
(
    IN  GT_U8                devNum,
    OUT CPSS_BRG_MODE_ENT    *brgModePtr
)
{
    GT_U32      rc;
    GT_U32      hwData;     /* data to write to register */
    GT_U32      regAddr;    /* hw register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(brgModePtr);


    /* get address of Global Control Register */
    regAddr =
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.globalControl;

    /* set Vlan - unaware mode in Global Control Register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 3, 1, &hwData);
    if(GT_OK != rc)
    {
        return rc;
    }

    *brgModePtr = (hwData) ? CPSS_BRG_MODE_802_1D_E : CPSS_BRG_MODE_802_1Q_E;

    return rc;
}

/**
* @internal cpssDxChBrgVlanBridgingModeGet function
* @endinternal
*
* @brief   Get bridging mode either VLAN-aware (802.1Q) or VLAN-unaware (802.1D) mode
*         The device supports a VLAN-unaware mode for 802.1D bridging.
*         When this mode is enabled:
*         - In VLAN-unaware mode, the device does not perform any packet
*         modifications. Packets are always transmitted as-received, without any
*         modification (i.e., packets received tagged are transmitted tagged;
*         packets received untagged are transmitted untagged).
*         - Packets are implicitly assigned with VLAN-ID 1, regardless of
*         VLAN-assignment mechanisms.
*         - Packets are implicitly assigned a VIDX Multicast group index 0xFFF,
*         indicating that the packet flood domain is according to the VLAN-in
*         this case VLAN 1. Registered Multicast is not supported in this mode.
*         All other features are operational in this mode, including internal
*         packet QoS, trapping, filtering, policy, etc.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] brgModePtr               - (pointer to) bridge mode: IEEE 802.1Q bridge or IEEE 802.1D bridge
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChBrgVlanBridgingModeGet
(
    IN  GT_U8                devNum,
    OUT CPSS_BRG_MODE_ENT    *brgModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanBridgingModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, brgModePtr));

    rc = internal_cpssDxChBrgVlanBridgingModeGet(devNum, brgModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, brgModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortTranslationEnableSet function
* @endinternal
*
* @brief   Enable/Disable Vlan Translation per ingress or egress port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - port number or CPU port number for egress direction.
*                                      In eArch devices portNum is default ePort for ingress direction.
* @param[in] direction                - ingress or egress
* @param[in] enable                   - enable/disable ingress/egress Vlan Translation.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortTranslationEnableSet
(
    IN GT_U8                              devNum,
    IN GT_PORT_NUM                        portNum,
    IN CPSS_DIRECTION_ENT                 direction,
    IN CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT enable
)
{

    GT_U32      regAddr;
    GT_U32      hwValue;
    GT_U32      startWord;  /* the table word at which the field starts */
    GT_U32      startBit;   /* the word's bit at which the field starts */
    GT_STATUS   rc;
    GT_U32      portGroupId;/* the port group Id - support multi-port-groups device */
    GT_U32       localPort;  /* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if (GT_TRUE != PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.vlanTranslationSupported)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }


    switch (enable)
    {
        case CPSS_DXCH_BRG_VLAN_TRANSLATION_DISABLE_E:
            hwValue = 0;
            break;
        case CPSS_DXCH_BRG_VLAN_TRANSLATION_VID0_E:
            hwValue = 1;
            break;
        case CPSS_DXCH_BRG_VLAN_TRANSLATION_SID_VID0_E:
            hwValue = 2;
            break;
        case CPSS_DXCH_BRG_VLAN_TRANSLATION_SID_VID0_VID1_E:
            hwValue = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (hwValue > 1 && (direction != CPSS_DIRECTION_EGRESS_E ||
                        !PRV_CPSS_SIP_5_10_CHECK_MAC(devNum)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
            {
                PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

                /* write to pre-tti-lookup-ingress-eport table */
                rc = prvCpssDxChWriteTableEntryField(devNum,
                                                CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                                portNum,
                                                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_VLAN_TRANSLATION_E, /* field name */
                                                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                hwValue);
            }
            else
            {
                PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);

                startWord = 0;
                startBit  = 31;

                rc = prvCpssDxChWriteTableEntryField(devNum,
                                                     CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                                     (GT_U32)portNum,
                                                     startWord,
                                                     startBit,
                                                     1, /* 1 bit */
                                                     hwValue);
            }
            break;

        case CPSS_DIRECTION_EGRESS_E:
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
            {
                PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
                rc = prvCpssDxChWriteTableEntryField(devNum,
                                                    CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                                   portNum,
                                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                    SIP5_HA_EPORT_TABLE_1_FIELDS_VLAN_TRANS_EN_E, /* field name */
                                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                   hwValue);

            }
            else
            {
                PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

                /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
                portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

                /* get the start bit 0..63 (in the bmp of registers) */
                startBit = (localPort == CPSS_CPU_PORT_NUM_CNS) ?
                         PRV_CPSS_DXCH_PP_HW_INFO_HA_CPU_PORT_BIT_INDEX_MAC(devNum) :
                         localPort;

                /* get address of Header Alteration Vlan Translation Enable register */
                regAddr =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.
                        haVlanTransEnReg[OFFSET_TO_WORD_MAC(startBit)];

                rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, OFFSET_TO_BIT_MAC(startBit), 1, hwValue);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortTranslationEnableSet function
* @endinternal
*
* @brief   Enable/Disable Vlan Translation per ingress or egress port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - port number or CPU port number for egress direction.
*                                      In eArch devices portNum is default ePort for ingress direction.
* @param[in] direction                - ingress or egress
* @param[in] enable                   - enable/disable ingress/egress Vlan Translation.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortTranslationEnableSet
(
    IN GT_U8                              devNum,
    IN GT_PORT_NUM                        portNum,
    IN CPSS_DIRECTION_ENT                 direction,
    IN CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortTranslationEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, enable));

    rc = internal_cpssDxChBrgVlanPortTranslationEnableSet(devNum, portNum, direction, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortTranslationEnableGet function
* @endinternal
*
* @brief   Get the status of Vlan Translation (Enable/Disable) per ingress or
*         egress port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] direction                - ingress or egress
* @param[in] portNum                  - port number or CPU port number for egress direction.
*                                      In eArch devices portNum is default ePort for ingress direction.
*
* @param[out] enablePtr                - (pointer to) ingress/egress Vlan Translation status
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortTranslationEnableGet
(
    IN  GT_U8                              devNum,
    IN  GT_PORT_NUM                        portNum,
    IN  CPSS_DIRECTION_ENT                 direction,
    OUT CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT *enablePtr
)
{
    GT_U32      regAddr;
    GT_U32      hwValue=0;
    GT_U32      startWord;  /* the table word at which the field starts */
    GT_U32      startBit;   /* the word's bit at which the field starts */
    GT_STATUS   rc=GT_OK;
    GT_U32      portGroupId;/* the port group Id - support multi-port-groups device */
    GT_U32       localPort;  /* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if (GT_TRUE != PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.vlanTranslationSupported)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    switch (direction)
    {
    case CPSS_DIRECTION_INGRESS_E:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

            /* read the value from pre-tti-lookup-ingress-eport table  */
            rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_VLAN_TRANSLATION_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &hwValue);
        }
        else
        {
            PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);

            startWord = 0;
            startBit  = 31;

            /* read the value from port-vlan-qos table  */
            rc = prvCpssDxChReadTableEntryField(devNum,
                                                CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
                                                (GT_U32)portNum,
                                                startWord,
                                                startBit,
                                                1, /* 1 bit */
                                                &hwValue);
        }
        break;

    case CPSS_DIRECTION_EGRESS_E:
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
            rc = prvCpssDxChReadTableEntryField(devNum,
                                               CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                               portNum,
                                                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                SIP5_HA_EPORT_TABLE_1_FIELDS_VLAN_TRANS_EN_E, /* field name */
                                                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                               &hwValue);
        }
        else
        {
            PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

            /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
            portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
            localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

            /* get the start bit 0..63 (in the bmp of registers) */
            startBit = (localPort == CPSS_CPU_PORT_NUM_CNS) ?
                     PRV_CPSS_DXCH_PP_HW_INFO_HA_CPU_PORT_BIT_INDEX_MAC(devNum) :
                     localPort;

            /* get address of Header Alteration Vlan Translation Enable register */
            regAddr =
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.
                    haVlanTransEnReg[OFFSET_TO_WORD_MAC(startBit)];

            rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, OFFSET_TO_BIT_MAC(startBit), 1, &hwValue);
        }
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (hwValue > 1 && (direction != CPSS_DIRECTION_EGRESS_E ||
                        !PRV_CPSS_SIP_5_10_CHECK_MAC(devNum)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    switch (hwValue)
    {
        case 0:
            *enablePtr = CPSS_DXCH_BRG_VLAN_TRANSLATION_DISABLE_E;
            break;
        case 1:
            *enablePtr = CPSS_DXCH_BRG_VLAN_TRANSLATION_VID0_E;
            break;
        case 2:
            *enablePtr = CPSS_DXCH_BRG_VLAN_TRANSLATION_SID_VID0_E;
            break;
        case 3:
            *enablePtr = CPSS_DXCH_BRG_VLAN_TRANSLATION_SID_VID0_VID1_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortTranslationEnableGet function
* @endinternal
*
* @brief   Get the status of Vlan Translation (Enable/Disable) per ingress or
*         egress port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] direction                - ingress or egress
* @param[in] portNum                  - port number or CPU port number for egress direction.
*                                      In eArch devices portNum is default ePort for ingress direction.
*
* @param[out] enablePtr                - (pointer to) ingress/egress Vlan Translation status
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortTranslationEnableGet
(
    IN  GT_U8                              devNum,
    IN  GT_PORT_NUM                        portNum,
    IN  CPSS_DIRECTION_ENT                 direction,
    OUT CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortTranslationEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, enablePtr));

    rc = internal_cpssDxChBrgVlanPortTranslationEnableGet(devNum, portNum, direction, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanTranslationEntryWrite function
* @endinternal
*
* @brief   Write an entry into Vlan Translation Table Ingress or Egress
*         This mapping avoids collisions between a VLAN-ID
*         used for a logical interface and a VLAN-ID used by the local network.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - VLAN id, used as index in the Translation Table
*                                      (APPLICABLE RANGES: 0..4095).
*                                      for ingress : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X 0..4095)
*                                      for egress  : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X 0..8191)
* @param[in] direction                - ingress or egress
* @param[in] transVlanId              - Translated Vlan ID, use as value in the
*                                      Translation Table (APPLICABLE RANGES: 0..4095).
*                                      for ingress : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X 0..8191)
*                                      for egress  : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_OUT_OF_RANGE          - when transVlanId is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanTranslationEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U16                       vlanId,
    IN CPSS_DIRECTION_ENT           direction,
    IN GT_U16                       transVlanId
)
{
    CPSS_DXCH_TABLE_ENT     tableType;
    GT_U32                      numOfBits = 12;/*number of bit to write*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if (GT_TRUE != PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.vlanTranslationSupported)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    switch (direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            PRV_CPSS_VLAN_VALUE_CHECK_MAC(vlanId);
            PRV_CPSS_DXCH_VLAN_VALUE_CHECK_MAC(devNum, transVlanId);

            tableType = CPSS_DXCH3_TABLE_INGRESS_VLAN_TRANSLATION_E;

            numOfBits = PRV_CPSS_DXCH_PP_HW_NUM_BITS_VID_MAC(devNum);

            break;

        case CPSS_DIRECTION_EGRESS_E:
            PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);
            PRV_CPSS_VLAN_VALUE_CHECK_MAC(transVlanId);

            tableType = CPSS_DXCH3_TABLE_EGRESS_VLAN_TRANSLATION_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    return prvCpssDxChWriteTableEntryField(devNum,
                                           tableType,
                                           vlanId,
                                           0,   /* word */
                                           0,   /* start bit */
                                           numOfBits,
                                           transVlanId);
}

/**
* @internal cpssDxChBrgVlanTranslationEntryWrite function
* @endinternal
*
* @brief   Write an entry into Vlan Translation Table Ingress or Egress
*         This mapping avoids collisions between a VLAN-ID
*         used for a logical interface and a VLAN-ID used by the local network.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - VLAN id, used as index in the Translation Table
*                                      (APPLICABLE RANGES: 0..4095).
*                                      for ingress : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X 0..4095)
*                                      for egress  : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X 0..8191)
* @param[in] direction                - ingress or egress
* @param[in] transVlanId              - Translated Vlan ID, use as value in the
*                                      Translation Table (APPLICABLE RANGES: 0..4095).
*                                      for ingress : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X 0..8191)
*                                      for egress  : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_OUT_OF_RANGE          - when transVlanId is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanTranslationEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U16                       vlanId,
    IN CPSS_DIRECTION_ENT           direction,
    IN GT_U16                       transVlanId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanTranslationEntryWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, direction, transVlanId));

    rc = internal_cpssDxChBrgVlanTranslationEntryWrite(devNum, vlanId, direction, transVlanId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, direction, transVlanId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanTranslationEntryRead function
* @endinternal
*
* @brief   Read an entry from Vlan Translation Table, Ingress or Egress.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - VLAN id, used as index in the Translation Table, 0..4095.
*                                      for ingress : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X 0..4095)
*                                      for egress  : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X 0..8191)
* @param[in] direction                - ingress or egress
*
* @param[out] transVlanIdPtr           - (pointer to) Translated Vlan ID, used as value in the
*                                      Translation Table.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanTranslationEntryRead
(
    IN  GT_U8                       devNum,
    IN  GT_U16                      vlanId,
    IN CPSS_DIRECTION_ENT           direction,
    OUT GT_U16                      *transVlanIdPtr
)
{
    CPSS_DXCH_TABLE_ENT     tableType;  /* table type               */
    GT_U32                      hwValue;    /* value to read from hw    */
    GT_STATUS                   rc;         /* return code              */
    GT_U32                      numOfBits = 12;/*number of bit to read*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(transVlanIdPtr);

    if (GT_TRUE != PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.vlanTranslationSupported)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    switch (direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            PRV_CPSS_VLAN_VALUE_CHECK_MAC(vlanId);

            tableType = CPSS_DXCH3_TABLE_INGRESS_VLAN_TRANSLATION_E;

            numOfBits = PRV_CPSS_DXCH_PP_HW_NUM_BITS_VID_MAC(devNum);
            break;

        case CPSS_DIRECTION_EGRESS_E:
            PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

            tableType = CPSS_DXCH3_TABLE_EGRESS_VLAN_TRANSLATION_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChReadTableEntryField(devNum,
                                        tableType,
                                        vlanId,
                                        0,   /* word */
                                        0,   /* start bit */
                                        numOfBits,
                                        &hwValue);

    *transVlanIdPtr = (GT_U16)hwValue;

    return rc;

}

/**
* @internal cpssDxChBrgVlanTranslationEntryRead function
* @endinternal
*
* @brief   Read an entry from Vlan Translation Table, Ingress or Egress.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] vlanId                   - VLAN id, used as index in the Translation Table, 0..4095.
*                                      for ingress : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X 0..4095)
*                                      for egress  : (APPLICABLE RANGES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X 0..8191)
* @param[in] direction                - ingress or egress
*
* @param[out] transVlanIdPtr           - (pointer to) Translated Vlan ID, used as value in the
*                                      Translation Table.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanTranslationEntryRead
(
    IN  GT_U8                       devNum,
    IN  GT_U16                      vlanId,
    IN CPSS_DIRECTION_ENT           direction,
    OUT GT_U16                      *transVlanIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanTranslationEntryRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, direction, transVlanIdPtr));

    rc = internal_cpssDxChBrgVlanTranslationEntryRead(devNum, vlanId, direction, transVlanIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, direction, transVlanIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanValidCheckEnableSet function
* @endinternal
*
* @brief   Enable/disable check of "Valid" field in the VLAN entry
*         When check enable and VLAN entry is not valid then packets are dropped.
*         When check disable and VLAN entry is not valid then packets are
*         not dropped and processed like in case of valid VLAN.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE  - Enable check of "Valid" field in the VLAN entry
*                                      GT_FALSE - Disable check of "Valid" field in the VLAN entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanValidCheckEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 value;       /* value to write into register */
    GT_U32 fieldOffset; /* The start bit number in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig0;
        fieldOffset = 2;
    }
    else
    {
        /* Bridge Global Configuration Register0, field <VLAN Valid Check Enable> */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bridgeRegs.bridgeGlobalConfigRegArray[0];
        fieldOffset = 4;
    }
    value = BOOL2BIT_MAC(enable);

    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 1, value);

}

/**
* @internal cpssDxChBrgVlanValidCheckEnableSet function
* @endinternal
*
* @brief   Enable/disable check of "Valid" field in the VLAN entry
*         When check enable and VLAN entry is not valid then packets are dropped.
*         When check disable and VLAN entry is not valid then packets are
*         not dropped and processed like in case of valid VLAN.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE  - Enable check of "Valid" field in the VLAN entry
*                                      GT_FALSE - Disable check of "Valid" field in the VLAN entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanValidCheckEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanValidCheckEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgVlanValidCheckEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanValidCheckEnableGet function
* @endinternal
*
* @brief   Get check status of "Valid" field in the VLAN entry
*         When check enable and VLAN entry is not valid then packets are dropped.
*         When check disable and VLAN entry is not valid then packets are
*         not dropped and processed like in case of valid VLAN.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - pointer to check status  of "Valid" field in the VLAN entry.
*                                      - GT_TRUE  - Enable check of "Valid" field in the VLAN entry
*                                      GT_FALSE - Disable check of "Valid" field in the VLAN entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - failed to read from hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanValidCheckEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr
)
{
    GT_U32    regAddr;     /* register address */
    GT_U32    value;       /* value to read from register */
    GT_STATUS rc;          /* return status */
    GT_U32    fieldOffset; /* The start bit number in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    bridgeEngineConfig.bridgeGlobalConfig0;
        fieldOffset = 2;
    }
    else
    {
        /* Bridge Global Configuration Register0, field <VLAN Valid Check Enable> */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                    bridgeRegs.bridgeGlobalConfigRegArray[0];
        fieldOffset = 4;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanValidCheckEnableGet function
* @endinternal
*
* @brief   Get check status of "Valid" field in the VLAN entry
*         When check enable and VLAN entry is not valid then packets are dropped.
*         When check disable and VLAN entry is not valid then packets are
*         not dropped and processed like in case of valid VLAN.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - pointer to check status  of "Valid" field in the VLAN entry.
*                                      - GT_TRUE  - Enable check of "Valid" field in the VLAN entry
*                                      GT_FALSE - Disable check of "Valid" field in the VLAN entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - failed to read from hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanValidCheckEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanValidCheckEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgVlanValidCheckEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanTpidEntrySet function
* @endinternal
*
* @brief   Function sets TPID (Tag Protocol ID) table entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tableType                - etherType table selection
* @param[in] entryIndex               - entry index for TPID table (APPLICABLE RANGES: 0..7)
* @param[in] etherType                - Tag Protocol ID value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex, tableType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E, TPID0 use entry indexes (0,2,4,6)
*       CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E, TPID1 use entry indexes (1,3,5,7)
*       Egress Logical Port TPID table each entry defines a Tag0 TPID and Tag1 TPID;
*       4 entries of (16b Tag0 TPID, 16b Tag1 TPID))
*
*/
static GT_STATUS internal_cpssDxChBrgVlanTpidEntrySet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_ETHERTYPE_TABLE_ENT   tableType,
    IN  GT_U32                          entryIndex,
    IN  GT_U16                          etherType
)
{
    GT_STATUS rc;
    GT_U32    regAddr;  /* register address */
    GT_U32    regValue; /* register value */
    GT_U32    startBit;/*start bit of the field */
    GT_U16    oldEtherType; /* Check whether etherType is changed */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* table index validity check */
    if (entryIndex > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    startBit = 16 * (entryIndex % 2);

    /* choose the appropriate table */
    switch (tableType)
    {
        case CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                                    TTI.VLANAssignment.ingrTPIDConfig[entryIndex / 2];
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    bridgeRegs.bridgeIngressVlanEtherTypeTable[entryIndex / 2];
            }
            break;

        case CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).TPIDConfigReg[entryIndex];
                startBit = 0;/*each TPID in different register (because there is <Tag Type> also in each register)*/
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        haRegs.bridgeEgressVlanEtherTypeTable[entryIndex / 2];
            }
            break;

         case CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E:
            PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    haRegs.passengerEgressVlanEtherTypeTable[entryIndex / 2];
            break;

        case CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_TS_E:
            PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    haRegs.tunnelStartEgressVlanEtherTypeTable[entryIndex / 2];
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, startBit, 16, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    oldEtherType = (GT_U16) regValue;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, startBit , 16, etherType);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ((tableType == CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E) &&
        (etherType != oldEtherType) &&
        PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        GT_U32      profile;    /* profile iterator */
        CPSS_ETHER_MODE_ENT  ethMode = CPSS_VLAN_ETHERTYPE0_E;
        GT_U32      tpidBmp;    /* profile TPID bitmap */
        GT_U32      affectedProfileBmp;    /* profiles which may be affected */

        affectedProfileBmp = 0;
        for (profile = 0; profile < 8; profile++)
        {
            /* Check if profile is using this entry, may need to update */
            rc = cpssDxChBrgVlanIngressTpidProfileGet(devNum, profile,
                            ethMode,
                            &tpidBmp);
            if (rc != GT_OK)
            {
                if (rc == GT_NOT_APPLICABLE_DEVICE)
                {
                    rc = GT_OK;
                }
                return rc;
            }
            if ((tpidBmp >> entryIndex) & 1)
            {
                /* profile may be affected */
                affectedProfileBmp |= (1 << profile);
            }
        }

        if (affectedProfileBmp != 0)
        {
            GT_PORT_NUM portNum;
            GT_U32      portMacNum;
            CPSS_MACDRV_OBJ_STC * portMacObjPtr;

            /* Profile for tag-0 has changed */
            /* Go over all remote ports - update those using this profile */
            for (portNum = 0; portNum < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; portNum++)
            {
                if(prvCpssDxChPortRemotePortCheck(devNum,portNum))
                {
                    /* If port is using this profile, may need to update */
                    rc = cpssDxChBrgVlanPortIngressTpidProfileGet(devNum, portNum,
                                    ethMode,
                                    GT_TRUE,
                                    &profile);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    if ((affectedProfileBmp >> profile) & 1)
                    {
                        /* Remote port is using an affected profile, may need to update */
                        rc = prvCpssDxChPortPhysicalPortMapCheckAndConvertForRemote(
                                    devNum,
                                    portNum,
                                    PRV_CPSS_DXCH_PORT_TYPE_MAC_E,
                                    &portMacNum);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                        /* Get PHY MAC object ptr */
                        portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

                        /* check if MACPHY callback should run */
                        if (portMacObjPtr != NULL &&
                            portMacObjPtr->macDrvMacPortTpidProfileSetFunc)
                        {
                            GT_BOOL doPpMacConfig = GT_TRUE;

                            rc = portMacObjPtr->macDrvMacPortTpidProfileSetFunc(devNum,
                                                                portNum,
                                                                ethMode, profile,
                                                                CPSS_MACDRV_STAGE_PRE_E,
                                                                &doPpMacConfig);
                            if(rc!=GT_OK)
                            {
                                return rc;
                            }
                        }
                    }
                }
            }
        }
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanTpidEntrySet function
* @endinternal
*
* @brief   Function sets TPID (Tag Protocol ID) table entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tableType                - etherType table selection
* @param[in] entryIndex               - entry index for TPID table (APPLICABLE RANGES: 0..7)
* @param[in] etherType                - Tag Protocol ID value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex, tableType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E, TPID0 use entry indexes (0,2,4,6)
*       CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E, TPID1 use entry indexes (1,3,5,7)
*       Egress Logical Port TPID table each entry defines a Tag0 TPID and Tag1 TPID;
*       4 entries of (16b Tag0 TPID, 16b Tag1 TPID))
*
*/
GT_STATUS cpssDxChBrgVlanTpidEntrySet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_ETHERTYPE_TABLE_ENT   tableType,
    IN  GT_U32                          entryIndex,
    IN  GT_U16                          etherType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanTpidEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tableType, entryIndex, etherType));

    rc = internal_cpssDxChBrgVlanTpidEntrySet(devNum, tableType, entryIndex, etherType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tableType, entryIndex, etherType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanTpidEntryGet function
* @endinternal
*
* @brief   Function gets TPID (Tag Protocol ID) table entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tableType                - etherType table selection
* @param[in] entryIndex               - entry index for TPID table (APPLICABLE RANGES: 0..7)
*
* @param[out] etherTypePtr             - (pointer to) Tag Protocol ID value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex, tableType
* @retval GT_BAD_PTR               - etherTypePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanTpidEntryGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_ETHERTYPE_TABLE_ENT   tableType,
    IN  GT_U32                          entryIndex,
    OUT GT_U16                         *etherTypePtr
)
{
    GT_U32    regAddr;  /* register address */
    GT_U32    regValue; /* register value */
    GT_STATUS rc;       /* returned status */
    GT_U32    startBit;/*start bit of the field */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(etherTypePtr);

    /* table index validity check */
    if (entryIndex > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    startBit = 16 * (entryIndex % 2);

    /* choose the appropriate table */
    switch (tableType)
    {
         case CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                                    TTI.VLANAssignment.ingrTPIDConfig[entryIndex / 2];
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        bridgeRegs.bridgeIngressVlanEtherTypeTable[entryIndex / 2];
            }
            break;

         case CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).TPIDConfigReg[entryIndex];
                startBit = 0;/*each TPID in different register (because there is <Tag Type> also in each register)*/
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        haRegs.bridgeEgressVlanEtherTypeTable[entryIndex / 2];
            }
            break;

        case CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E:
            PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    haRegs.passengerEgressVlanEtherTypeTable[entryIndex / 2];
            break;

        case CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_TS_E:
            PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                    haRegs.tunnelStartEgressVlanEtherTypeTable[entryIndex / 2];
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, startBit, 16, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *etherTypePtr = (GT_U16) regValue;

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanTpidEntryGet function
* @endinternal
*
* @brief   Function gets TPID (Tag Protocol ID) table entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tableType                - etherType table selection
* @param[in] entryIndex               - entry index for TPID table (APPLICABLE RANGES: 0..7)
*
* @param[out] etherTypePtr             - (pointer to) Tag Protocol ID value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, entryIndex, tableType
* @retval GT_BAD_PTR               - etherTypePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanTpidEntryGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_ETHERTYPE_TABLE_ENT   tableType,
    IN  GT_U32                          entryIndex,
    OUT GT_U16                         *etherTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanTpidEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tableType, entryIndex, etherTypePtr));

    rc = internal_cpssDxChBrgVlanTpidEntryGet(devNum, tableType, entryIndex, etherTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tableType, entryIndex, etherTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIngressTpidProfileSet function
* @endinternal
*
* @brief   Function sets bitmap of TPID (Tag Protocol ID) per profile .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - TPID profile. (APPLICABLE RANGES: 0..7)
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] tpidBmp                  - bitmap represent 8 entries at the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, profile, ethMode
* @retval GT_OUT_OF_RANGE          - tpidBmp initialized with more than 8 bits
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For the ingress direction multiple TPID may be used to
*       identify Tag0 and Tag1. Each bit at the bitmap represents one of
*       the 8 entries at the TPID Table.
*       2. bind 'port' to 'profile' using function cpssDxChBrgVlanPortIngressTpidProfileSet
*
*/
static GT_STATUS internal_cpssDxChBrgVlanIngressTpidProfileSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               profile,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_U32               tpidBmp
)
{
    GT_STATUS   rc;      /*return code*/
    GT_U32      regAddr; /* register address */
    GT_U32      bit;/* start bit */
    GT_U32      tag;/* tag 0 or 1*/
    GT_U32      oldTpidBmp;     /* may need to update remote ports if TPID bitmap changed */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    if(profile >= 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (ethMode)
    {
        case CPSS_VLAN_ETHERTYPE0_E:
            tag = 0;
            break;

        case CPSS_VLAN_ETHERTYPE1_E:
            tag = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (tpidBmp >= BIT_8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* each tag hold  2 registers , with 4 profiles in register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                        TTI.VLANAssignment.ingrTagProfileTPIDSelect[tag][profile/4];

    bit = (profile % 4) * 8;/* 8 bits per profile */

    rc = prvCpssHwPpGetRegField(devNum, regAddr, bit, 8, &oldTpidBmp);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssHwPpSetRegField(devNum, regAddr, bit, 8, tpidBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ((ethMode == CPSS_VLAN_ETHERTYPE0_E) &&
        (tpidBmp != oldTpidBmp))
    {
        GT_U32      tempProfile;    /* temporary profile check */
        GT_PORT_NUM portNum;
        GT_U32      portMacNum;
        CPSS_MACDRV_OBJ_STC * portMacObjPtr;

        /* Profile for tag-0 has changed */
        /* Go over all remote ports - update those using this profile */
        for (portNum = 0; portNum < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; portNum++)
        {
            if(prvCpssDxChPortRemotePortCheck(devNum,portNum))
            {
                /* If port is using this profile, may need to update */
                rc = cpssDxChBrgVlanPortIngressTpidProfileGet(devNum, portNum,
                                ethMode,
                                GT_TRUE,
                                &tempProfile);
                if (rc != GT_OK)
                {
                    return rc;
                }
                if (tempProfile == profile)
                {
                    /* Remote port is using this profile, may need to update */
                    rc = prvCpssDxChPortPhysicalPortMapCheckAndConvertForRemote(
                                devNum,
                                portNum,
                                PRV_CPSS_DXCH_PORT_TYPE_MAC_E,
                                &portMacNum);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    /* Get PHY MAC object ptr */
                    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

                    /* check if MACPHY callback should run */
                    if (portMacObjPtr != NULL && portMacObjPtr->macDrvMacPortTpidProfileSetFunc)
                    {
                        GT_BOOL doPpMacConfig = GT_TRUE;

                        rc = portMacObjPtr->macDrvMacPortTpidProfileSetFunc(devNum, portNum,ethMode, profile,
                            CPSS_MACDRV_STAGE_PRE_E,&doPpMacConfig);
                        if(rc!=GT_OK)
                        {
                            return rc;
                        }
                    }
                }
            }
        }
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanIngressTpidProfileSet function
* @endinternal
*
* @brief   Function sets bitmap of TPID (Tag Protocol ID) per profile .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - TPID profile. (APPLICABLE RANGES: 0..7)
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] tpidBmp                  - bitmap represent 8 entries at the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, profile, ethMode
* @retval GT_OUT_OF_RANGE          - tpidBmp initialized with more than 8 bits
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For the ingress direction multiple TPID may be used to
*       identify Tag0 and Tag1. Each bit at the bitmap represents one of
*       the 8 entries at the TPID Table.
*       2. bind 'port' to 'profile' using function cpssDxChBrgVlanPortIngressTpidProfileSet
*
*/
GT_STATUS cpssDxChBrgVlanIngressTpidProfileSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               profile,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_U32               tpidBmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIngressTpidProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, ethMode, tpidBmp));

    rc = internal_cpssDxChBrgVlanIngressTpidProfileSet(devNum, profile, ethMode, tpidBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, ethMode, tpidBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIngressTpidProfileGet function
* @endinternal
*
* @brief   Function gts bitmap of TPID (Tag Protocol ID) per profile .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - TPID profile. (APPLICABLE RANGES: 0..7)
* @param[in] ethMode                  - TAG0/TAG1 selector
*
* @param[out] tpidBmpPtr               - (pointer to) bitmap represent 8 entries at the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, profile, ethMode
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanIngressTpidProfileGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               profile,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    OUT GT_U32               *tpidBmpPtr
)
{
    GT_STATUS   rc;      /*return code*/
    GT_U32      regAddr; /* register address */
    GT_U32      bit;/* start bit */
    GT_U32      tag;/* tag 0 or 1*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(tpidBmpPtr);

    if(profile >= 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (ethMode)
    {
        case CPSS_VLAN_ETHERTYPE0_E:
            tag = 0;
            break;

        case CPSS_VLAN_ETHERTYPE1_E:
            tag = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* each tag hold  2 registers , with 4 profiles in register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                        TTI.VLANAssignment.ingrTagProfileTPIDSelect[tag][profile/4];

    bit = (profile % 4) * 8;/* 8 bits per profile */

    rc = prvCpssHwPpGetRegField(devNum, regAddr, bit, 8, tpidBmpPtr);

    return rc;
}

/**
* @internal cpssDxChBrgVlanIngressTpidProfileGet function
* @endinternal
*
* @brief   Function gts bitmap of TPID (Tag Protocol ID) per profile .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - TPID profile. (APPLICABLE RANGES: 0..7)
* @param[in] ethMode                  - TAG0/TAG1 selector
*
* @param[out] tpidBmpPtr               - (pointer to) bitmap represent 8 entries at the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, profile, ethMode
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIngressTpidProfileGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               profile,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    OUT GT_U32               *tpidBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIngressTpidProfileGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, ethMode, tpidBmpPtr));

    rc = internal_cpssDxChBrgVlanIngressTpidProfileGet(devNum, profile, ethMode, tpidBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, ethMode, tpidBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanTpidTagTypeSet function
* @endinternal
*
* @brief   Set TPID tag type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress direction
* @param[in] index                    - TPID  (APPLICABLE RANGES: 0..7)
* @param[in] type                     - the TPID type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong input parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanTpidTagTypeSet
(
    IN GT_U8                        devNum,
    IN CPSS_DIRECTION_ENT           direction,
    IN GT_U32                       index,
    IN CPSS_BRG_TPID_SIZE_TYPE_ENT  type
)
{
    GT_U32 regAddr;     /* the register address */
    GT_U32 offset;      /* the offset in the register */
    GT_U32 value;       /* the value to write */
    GT_U32 length;      /* number of bits to write */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    if (index > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).VLANAssignment.ingrTPIDTagType;
            if(0 == PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                offset = index;
                length = 1;
            }
            else
            {
                offset = 2*index;
                length = 2;
            }
            break;

        case CPSS_DIRECTION_EGRESS_E:
            regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).TPIDConfigReg[index];
            offset = 16;
            length = 2;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (type)
    {
        case CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E:
            value = 0;
            break;

        case CPSS_BRG_TPID_SIZE_TYPE_8_BYTES_E:
            value = 1;
            break;

        case CPSS_BRG_TPID_SIZE_TYPE_6_BYTES_E:
            if(0 == PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                /* the device not supports the value */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            value = 2;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, offset, length, value);
}

/**
* @internal cpssDxChBrgVlanTpidTagTypeSet function
* @endinternal
*
* @brief   Set TPID tag type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress direction
* @param[in] index                    - TPID  (APPLICABLE RANGES: 0..7)
* @param[in] type                     - the TPID type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong input parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanTpidTagTypeSet
(
    IN GT_U8                        devNum,
    IN CPSS_DIRECTION_ENT           direction,
    IN GT_U32                       index,
    IN CPSS_BRG_TPID_SIZE_TYPE_ENT  type
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanTpidTagTypeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, direction, index, type));

    rc = internal_cpssDxChBrgVlanTpidTagTypeSet(devNum, direction, index, type);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, direction, index, type));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanTpidTagTypeGet function
* @endinternal
*
* @brief   Get TPID tag type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress direction
* @param[in] index                    - TPID  (APPLICABLE RANGES: 0..7)
*
* @param[out] typePtr                  - (pointer to) the TPID type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong input parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanTpidTagTypeGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DIRECTION_ENT          direction,
    IN  GT_U32                      index,
    OUT CPSS_BRG_TPID_SIZE_TYPE_ENT *typePtr
)
{
    GT_U32      regAddr, offset, value, length;
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(typePtr);

    if (index > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).VLANAssignment.ingrTPIDTagType;
            if(0 == PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                offset = index;
                length = 1;
            }
            else
            {
                offset = 2*index;
                length = 2;
            }
            break;

        case CPSS_DIRECTION_EGRESS_E:
            regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).TPIDConfigReg[index];
            offset = 16;
            length = 2;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, offset, length, &value);
    if (rc == GT_OK)
    {
        switch (value)
        {
            case 0:
                *typePtr = CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E;
                break;

            case 1:
                *typePtr = CPSS_BRG_TPID_SIZE_TYPE_8_BYTES_E;
                break;

            case 2:
                if(0 == PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                {
                    /* the device not supports the value */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }
                *typePtr = CPSS_BRG_TPID_SIZE_TYPE_6_BYTES_E;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }
    return rc;
}

/**
* @internal cpssDxChBrgVlanTpidTagTypeGet function
* @endinternal
*
* @brief   Get TPID tag type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress direction
* @param[in] index                    - TPID  (APPLICABLE RANGES: 0..7)
*
* @param[out] typePtr                  - (pointer to) the TPID type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong input parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanTpidTagTypeGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DIRECTION_ENT          direction,
    IN  GT_U32                      index,
    OUT CPSS_BRG_TPID_SIZE_TYPE_ENT *typePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanTpidTagTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, direction, index, typePtr));

    rc = internal_cpssDxChBrgVlanTpidTagTypeGet(devNum, direction, index, typePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, direction, index, typePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortIngressTpidProfileSet function
* @endinternal
*
* @brief   Function sets TPID (Tag Protocol ID) profile per ingress port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port.
*                                      In eArch devices portNum is default ePort for default profile.
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] isDefaultProfile         - indication that the profile is the default profile or
*                                      used for re-parse after TTI lookup:
*                                      1. Parse inner Layer2 after TT
*                                      2. Re-parse Layer2 after popping EVB/BPE tag
*                                      3. Parsing of passenger packet
*                                      GT_TRUE  - used as default profile
*                                      GT_FALSE - used for re-parse after TTI lookup.
* @param[in] profile                  - TPID profile. (APPLICABLE RANGE: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - profile > 7
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For setting TPID bmp per profile use function cpssDxChBrgVlanIngressTpidProfileSet
*
*/
static GT_STATUS internal_cpssDxChBrgVlanPortIngressTpidProfileSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN GT_BOOL               isDefaultProfile,
    IN  GT_U32               profile
)
{
    GT_STATUS   rc;      /*return code*/
    GT_U32      fieldName_defEprot;/* the field name in default eport entry */
    GT_U32      fieldName_EportAttributes; /* the field name in eport attributes entry */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;
    GT_U32                  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    if(profile >= BIT_3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    switch (ethMode)
    {
        case CPSS_VLAN_ETHERTYPE0_E:
            /* field name in pre TTI  */
            fieldName_defEprot = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_INGRESS_TAG0_TPID_PROFILE_E;
            /* field name in post TTI */
            fieldName_EportAttributes = SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_TAG0_TPID_PROFILE_E;
            break;

        case CPSS_VLAN_ETHERTYPE1_E:
            /* field name in pre TTI  */
            fieldName_defEprot = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_INGRESS_TAG1_TPID_PROFILE_E;
            /* field name in post TTI */
            fieldName_EportAttributes = SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_TAG1_TPID_PROFILE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(isDefaultProfile == GT_TRUE)
    {
        /* support remote port that need remote TPID support */
        if(prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

            /* Get PHY MAC object ptr */
            portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

            /* check if MACPHY callback should run */
            if (portMacObjPtr != NULL && portMacObjPtr->macDrvMacPortTpidProfileSetFunc)
            {
                GT_BOOL doPpMacConfig = GT_TRUE;

                rc = portMacObjPtr->macDrvMacPortTpidProfileSetFunc(devNum, portNum,ethMode, profile,
                    CPSS_MACDRV_STAGE_PRE_E,&doPpMacConfig);
                if(rc!=GT_OK)
                {
                    return rc;
                }

                if(doPpMacConfig == GT_FALSE)
                {
                    return GT_OK;
                }
            }
        }

        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);
        /* write to pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,/*table type*/
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            fieldName_defEprot, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            profile);
    }
    else
    {
        PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
       /* write to post-tti-lookup-ingress-eport table */

        rc = prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E,/*table type*/
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            fieldName_EportAttributes,    /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            profile);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortIngressTpidProfileSet function
* @endinternal
*
* @brief   Function sets TPID (Tag Protocol ID) profile per ingress port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port.
*                                      In eArch devices portNum is default ePort for default profile.
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] isDefaultProfile         - indication that the profile is the default profile or
*                                      used for re-parse after TTI lookup:
*                                      1. Parse inner Layer2 after TT
*                                      2. Re-parse Layer2 after popping EVB/BPE tag
*                                      3. Parsing of passenger packet
*                                      GT_TRUE  - used as default profile
*                                      GT_FALSE - used for re-parse after TTI lookup.
* @param[in] profile                  - TPID profile. (APPLICABLE RANGE: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - profile > 7
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For setting TPID bmp per profile use function cpssDxChBrgVlanIngressTpidProfileSet
*
*/
GT_STATUS cpssDxChBrgVlanPortIngressTpidProfileSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN GT_BOOL               isDefaultProfile,
    IN  GT_U32               profile
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortIngressTpidProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ethMode, isDefaultProfile, profile));

    rc = internal_cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, portNum, ethMode, isDefaultProfile, profile);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ethMode, isDefaultProfile, profile));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortIngressTpidProfileGet function
* @endinternal
*
* @brief   Function gets TPID (Tag Protocol ID) profile per ingress port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port.
*                                      In eArch devices portNum is default ePort for default profile.
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] isDefaultProfile         - indication that the profile is the default profile or
*                                      used for re-parse after TTI lookup:
*                                      1. Parse inner Layer2 after TT
*                                      2. Re-parse Layer2 after popping EVB/BPE tag
*                                      3. Parsing of passenger packet
*                                      GT_TRUE  - used as default profile
*                                      GT_FALSE - used for re-parse after TTI lookup.
*
* @param[out] profilePtr               - (pointer to)TPID profile.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortIngressTpidProfileGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN GT_BOOL               isDefaultProfile,
    OUT GT_U32               *profilePtr
)
{
    GT_STATUS   rc;      /*return code*/
    GT_U32      fieldName_defEprot;/* the field name in default eport entry */
    GT_U32      fieldName_EportAttributes; /* the field name in eport attributes entry */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(profilePtr);

    switch (ethMode)
    {
        case CPSS_VLAN_ETHERTYPE0_E:
            /* field name in pre TTI  */
            fieldName_defEprot = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_INGRESS_TAG0_TPID_PROFILE_E;
            /* field name in post TTI */
            fieldName_EportAttributes = SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_TAG0_TPID_PROFILE_E;
            break;

        case CPSS_VLAN_ETHERTYPE1_E:
            /* field name in pre TTI  */
            fieldName_defEprot = SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_INGRESS_TAG1_TPID_PROFILE_E;
            /* field name in post TTI */
            fieldName_EportAttributes = SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_TAG1_TPID_PROFILE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(isDefaultProfile == GT_TRUE)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);
        /* read from pre-tti-lookup-ingress-eport table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,/*table type*/
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            fieldName_defEprot, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            profilePtr);
    }
    else
    {
        PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
       /* read from post-tti-lookup-ingress-eport table */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E,/*table type*/
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            fieldName_EportAttributes, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            profilePtr);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortIngressTpidProfileGet function
* @endinternal
*
* @brief   Function gets TPID (Tag Protocol ID) profile per ingress port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number or CPU port.
*                                      In eArch devices portNum is default ePort for default profile.
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] isDefaultProfile         - indication that the profile is the default profile or
*                                      used for re-parse after TTI lookup:
*                                      1. Parse inner Layer2 after TT
*                                      2. Re-parse Layer2 after popping EVB/BPE tag
*                                      3. Parsing of passenger packet
*                                      GT_TRUE  - used as default profile
*                                      GT_FALSE - used for re-parse after TTI lookup.
*
* @param[out] profilePtr               - (pointer to)TPID profile.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortIngressTpidProfileGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN GT_BOOL               isDefaultProfile,
    OUT GT_U32               *profilePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortIngressTpidProfileGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ethMode, isDefaultProfile, profilePtr));

    rc = internal_cpssDxChBrgVlanPortIngressTpidProfileGet(devNum, portNum, ethMode, isDefaultProfile, profilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ethMode, isDefaultProfile, profilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgVlanPortIngressTpidSet function
* @endinternal
*
* @brief   Function sets bitmap of TPID (Tag Protocol ID) table indexes per
*         ingress port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:   Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] ethMode                  - etherType mode selector; valid values:
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE1_E
* @param[in] tpidBmp                  - bitmap represent 8 entries at the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - tpidBmp initialized with more than 8 bits
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note that for the ingress direction multiple TPID may be use to
*       identify Tag0 and Tag1. Each bit at the bitmap represents one of
*       the 8 entries at the TPID Table.
*
*/
static GT_STATUS internal_cpssDxChBrgVlanPortIngressTpidSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode,
    IN  GT_U32               tpidBmp
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;    /* register address */
    GT_U32      vlanOffset;
    GT_U32       localPort;  /* local port - support multi-port-groups device */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

    if (tpidBmp >= BIT_8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    switch (ethMode)
    {
         case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E:
            vlanOffset = 0 + 16 * (portNum % 2);
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.bridgeIngressVlanSelect[localPort / 2];
            break;

         case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E:
            vlanOffset = 8 + 16 * (portNum % 2);
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.bridgeIngressVlanSelect[localPort / 2];
            break;

         case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE0_E:
            PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

            vlanOffset = 0 + 16 * (portNum % 2);
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ttIngressTpidSelect[localPort / 2];
            break;

        case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE1_E:
            PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

            vlanOffset = 8 + 16 * (portNum % 2);
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ttIngressTpidSelect[localPort / 2];
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, vlanOffset, 8, tpidBmp);

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortIngressTpidSet function
* @endinternal
*
* @brief   Function sets bitmap of TPID (Tag Protocol ID) table indexes per
*         ingress port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:   Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] ethMode                  - etherType mode selector; valid values:
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE1_E
* @param[in] tpidBmp                  - bitmap represent 8 entries at the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - tpidBmp initialized with more than 8 bits
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note that for the ingress direction multiple TPID may be use to
*       identify Tag0 and Tag1. Each bit at the bitmap represents one of
*       the 8 entries at the TPID Table.
*
*/
GT_STATUS cpssDxChBrgVlanPortIngressTpidSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode,
    IN  GT_U32               tpidBmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortIngressTpidSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ethMode, tpidBmp));

    rc = internal_cpssDxChBrgVlanPortIngressTpidSet(devNum, portNum, ethMode, tpidBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ethMode, tpidBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortIngressTpidGet function
* @endinternal
*
* @brief   Function gets bitmap of TPID (Tag Protocol ID) table indexes per
*         ingress port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:   Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] ethMode                  - etherType mode selector. valid values:
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE1_E
*
* @param[out] tpidBmpPtr               - (pointer to) bitmap represent 8 entries at the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - tpidBmpPtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note that for the ingress direction multiple TPID may be use to
*       identify Tag0 and Tag1. Each bit at the bitmap represents one of
*       the 8 entries at the TPID Table.
*
*/
static GT_STATUS internal_cpssDxChBrgVlanPortIngressTpidGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode,
    OUT GT_U32               *tpidBmpPtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;    /* register address */
    GT_U32      vlanOffset;
    GT_U32       localPort;  /* local port - support multi-port-groups device */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(tpidBmpPtr);

      /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    switch (ethMode)
    {
        case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E:
            vlanOffset = 0 + 16 * (portNum % 2);
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.bridgeIngressVlanSelect[localPort / 2];
            break;

        case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E:
            vlanOffset = 8 + 16 * (portNum % 2);
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.bridgeIngressVlanSelect[localPort / 2];
            break;

        case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE0_E:
            PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

            vlanOffset = 0 + 16 * (portNum % 2);
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ttIngressTpidSelect[localPort / 2];
            break;

        case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE1_E:
            PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

            vlanOffset = 8 + 16 * (portNum % 2);
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ttIngressTpidSelect[localPort / 2];
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, vlanOffset, 8, tpidBmpPtr);

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortIngressTpidGet function
* @endinternal
*
* @brief   Function gets bitmap of TPID (Tag Protocol ID) table indexes per
*         ingress port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:   Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] ethMode                  - etherType mode selector. valid values:
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE1_E
*
* @param[out] tpidBmpPtr               - (pointer to) bitmap represent 8 entries at the TPID Table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - tpidBmpPtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note that for the ingress direction multiple TPID may be use to
*       identify Tag0 and Tag1. Each bit at the bitmap represents one of
*       the 8 entries at the TPID Table.
*
*/
GT_STATUS cpssDxChBrgVlanPortIngressTpidGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode,
    OUT GT_U32               *tpidBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortIngressTpidGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ethMode, tpidBmpPtr));

    rc = internal_cpssDxChBrgVlanPortIngressTpidGet(devNum, portNum, ethMode, tpidBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ethMode, tpidBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortEgressTpidSet function
* @endinternal
*
* @brief   Function sets index of TPID (Tag protocol ID) table per egress port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] ethMode                  - ethertype mode selector; valid values:
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_EGRESS_TS_ETHERTYPE_E
* @param[in] tpidEntryIndex           - TPID table entry index (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - illegal tpidEntryIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortEgressTpidSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode,
    IN  GT_U32               tpidEntryIndex
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      vlanOffset;
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32       localPort;  /* local port - support multi-port-groups device */
    GT_U32      fieldOffset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    if (tpidEntryIndex >= 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        switch (ethMode)
        {
            case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E:
                fieldOffset = SIP5_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG0_TPID_INDEX_E;
                break;

             case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E:
                fieldOffset = SIP5_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG1_TPID_INDEX_E;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        return prvCpssDxChWriteTableEntryField(devNum,
                                           CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                           portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            fieldOffset, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                           tpidEntryIndex);
    }
    else
    {

        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        if (localPort == CPSS_CPU_PORT_NUM_CNS)
        {
            localPort = (GT_U8)PRV_CPSS_DXCH_PP_HW_INFO_HA_CPU_PORT_BIT_INDEX_MAC(devNum);
        }

        vlanOffset = 3 * (localPort % 8);

        switch (ethMode)
        {
            case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        haRegs.bridgeEgressPortTag0TpidSelect[localPort / 8];
                break;

             case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            haRegs.bridgeEgressPortTag1TpidSelect[localPort / 8];
                break;

             case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_EGRESS_TS_ETHERTYPE_E:
                PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            haRegs.tunnelStartEgressPortTagTpidSelect[localPort / 8];
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, vlanOffset, 3, tpidEntryIndex);
    }
}

/**
* @internal cpssDxChBrgVlanPortEgressTpidSet function
* @endinternal
*
* @brief   Function sets index of TPID (Tag protocol ID) table per egress port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] ethMode                  - ethertype mode selector; valid values:
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_EGRESS_TS_ETHERTYPE_E
* @param[in] tpidEntryIndex           - TPID table entry index (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - illegal tpidEntryIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortEgressTpidSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode,
    IN  GT_U32               tpidEntryIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortEgressTpidSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ethMode, tpidEntryIndex));

    rc = internal_cpssDxChBrgVlanPortEgressTpidSet(devNum, portNum, ethMode, tpidEntryIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ethMode, tpidEntryIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortEgressTpidGet function
* @endinternal
*
* @brief   Function gets index of TPID (Tag Protocol ID) table per egress port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] ethMode                  - ethertype mode selector; valid values:
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_EGRESS_TS_ETHERTYPE_E
*
* @param[out] tpidEntryIndexPtr        - (pointer to) TPID table entry index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - tpidEntryIndexPtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortEgressTpidGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode,
    OUT GT_U32               *tpidEntryIndexPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr;  /* register address */
    GT_U32    vlanOffset;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32   localPort;/* local port - support multi-port-groups device */
    GT_U32  hwValue;
    GT_U32  fieldOffset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(tpidEntryIndexPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        switch (ethMode)
        {
             case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E:
                fieldOffset = SIP5_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG0_TPID_INDEX_E;
                break;

            case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E:
                fieldOffset = SIP5_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG1_TPID_INDEX_E;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                               portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            fieldOffset, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                               &hwValue);
    }
    else
    {

        PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        if (localPort == CPSS_CPU_PORT_NUM_CNS)
        {
            localPort = (GT_U8)PRV_CPSS_DXCH_PP_HW_INFO_HA_CPU_PORT_BIT_INDEX_MAC(devNum);
        }

        vlanOffset = 3 * (localPort % 8);

        switch (ethMode)
        {
             case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        haRegs.bridgeEgressPortTag0TpidSelect[localPort / 8];
                break;

             case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        haRegs.bridgeEgressPortTag1TpidSelect[localPort / 8];
                break;

             case CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_EGRESS_TS_ETHERTYPE_E:
                PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            haRegs.tunnelStartEgressPortTagTpidSelect[localPort / 8];
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, vlanOffset, 3, &hwValue);
    }

    if(rc != GT_OK)
        return rc;

    *tpidEntryIndexPtr = hwValue;

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortEgressTpidGet function
* @endinternal
*
* @brief   Function gets index of TPID (Tag Protocol ID) table per egress port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port is supported)
* @param[in] ethMode                  - ethertype mode selector; valid values:
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E
*                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_EGRESS_TS_ETHERTYPE_E
*
* @param[out] tpidEntryIndexPtr        - (pointer to) TPID table entry index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - tpidEntryIndexPtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortEgressTpidGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT  ethMode,
    OUT GT_U32               *tpidEntryIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortEgressTpidGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ethMode, tpidEntryIndexPtr));

    rc = internal_cpssDxChBrgVlanPortEgressTpidGet(devNum, portNum, ethMode, tpidEntryIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ethMode, tpidEntryIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanEgressTagTpidSelectModeSet function
* @endinternal
*
* @brief   Set Tag 0/1 TPID (Tag Protocol Identifies) Select Mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] mode                     - 0: TPID of Tag0/1 is selected according to
*                                      <Egress Tag 0/1 TPID select>
*                                      1: TPID of Tag0/1 is selected according to
*                                      VID0/1 to TPID select table
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanEgressTagTpidSelectModeSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_ETHER_MODE_ENT                          ethMode,
    IN CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT  mode
)
{
    GT_U32   hwData;
    GT_U32   bitOffset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    switch(mode)
    {
        case CPSS_DXCH_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E:
            hwData = 0;
            break;
        case CPSS_DXCH_BRG_VLAN_VID_TO_TPID_SELECT_TABLE_MODE_E:
            hwData = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (ethMode)
    {
        case CPSS_VLAN_ETHERTYPE0_E:
            bitOffset = SIP5_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG0_TPID_E;
            break;

        case CPSS_VLAN_ETHERTYPE1_E:
            bitOffset = SIP5_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG1_TPID_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    bitOffset, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   hwData);
}

/**
* @internal cpssDxChBrgVlanEgressTagTpidSelectModeSet function
* @endinternal
*
* @brief   Set Tag 0/1 TPID (Tag Protocol Identifies) Select Mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ethMode                  - TAG0/TAG1 selector
* @param[in] mode                     - 0: TPID of Tag0/1 is selected according to
*                                      <Egress Tag 0/1 TPID select>
*                                      1: TPID of Tag0/1 is selected according to
*                                      VID0/1 to TPID select table
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanEgressTagTpidSelectModeSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_ETHER_MODE_ENT                          ethMode,
    IN CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT  mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanEgressTagTpidSelectModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ethMode, mode));

    rc = internal_cpssDxChBrgVlanEgressTagTpidSelectModeSet(devNum, portNum, ethMode, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ethMode, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanEgressTagTpidSelectModeGet function
* @endinternal
*
* @brief   Returns Tag 0/1 TPID (Tag Protocol Identifies) Select Mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ethMode                  - TAG0/TAG1 selector
*
* @param[out] modePtr                  - (pointer to)
*                                      0: TPID of Tag0/1 is selected according to
*                                      <Egress Tag 0/1 TPID select>
*                                      1: TPID of Tag0/1 is selected according to
*                                      VID0/1 to TPID select table
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanEgressTagTpidSelectModeGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PORT_NUM                                         portNum,
    IN  CPSS_ETHER_MODE_ENT                                 ethMode,
    OUT CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT         *modePtr
)
{
    GT_U32      hwValue;    /* hardware value */
    GT_STATUS   rc;
    GT_U32      bitOffset;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);



    switch (ethMode)
    {
        case CPSS_VLAN_ETHERTYPE0_E:
            bitOffset = SIP5_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG0_TPID_E;
            break;

        case CPSS_VLAN_ETHERTYPE1_E:
            bitOffset = SIP5_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG1_TPID_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChReadTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    bitOffset, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch(hwValue)
    {
        case 0:
            *modePtr  = CPSS_DXCH_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E;
            break;
        case 1:
            *modePtr  = CPSS_DXCH_BRG_VLAN_VID_TO_TPID_SELECT_TABLE_MODE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

   return rc;
}

/**
* @internal cpssDxChBrgVlanEgressTagTpidSelectModeGet function
* @endinternal
*
* @brief   Returns Tag 0/1 TPID (Tag Protocol Identifies) Select Mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ethMode                  - TAG0/TAG1 selector
*
* @param[out] modePtr                  - (pointer to)
*                                      0: TPID of Tag0/1 is selected according to
*                                      <Egress Tag 0/1 TPID select>
*                                      1: TPID of Tag0/1 is selected according to
*                                      VID0/1 to TPID select table
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanEgressTagTpidSelectModeGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PORT_NUM                                         portNum,
    IN  CPSS_ETHER_MODE_ENT                                 ethMode,
    OUT CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT         *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanEgressTagTpidSelectModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ethMode, modePtr));

    rc = internal_cpssDxChBrgVlanEgressTagTpidSelectModeGet(devNum, portNum, ethMode, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ethMode, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgVlanRangeSet function
* @endinternal
*
* @brief   Function configures the valid VLAN Range.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vidRange                 - VID range for VLAN filtering (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - illegal vidRange
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanRangeSet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vidRange
)
{
    GT_U32    regAddr;  /* register address */
    GT_STATUS rc;       /* returned status  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_VLAN_VALUE_CHECK_MAC(devNum, vidRange);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    VLANRangesConfig.ingrVLANRangeConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.vlan0;
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0,
        PRV_CPSS_DXCH_PP_HW_NUM_BITS_VID_MAC(devNum), vidRange);

    return rc;
}

/**
* @internal cpssDxChBrgVlanRangeSet function
* @endinternal
*
* @brief   Function configures the valid VLAN Range.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vidRange                 - VID range for VLAN filtering (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - illegal vidRange
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanRangeSet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vidRange
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanRangeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vidRange));

    rc = internal_cpssDxChBrgVlanRangeSet(devNum, vidRange);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vidRange));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanRangeGet function
* @endinternal
*
* @brief   Function gets the valid VLAN Range.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] vidRangePtr              - (pointer to) VID range for VLAN filtering
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - vidRangePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanRangeGet
(
    IN  GT_U8   devNum,
    OUT GT_U16  *vidRangePtr
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value   */
    GT_STATUS rc;       /* returned status  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(vidRangePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                    VLANRangesConfig.ingrVLANRangeConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.vlan0;
    }

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, 0 ,
            PRV_CPSS_DXCH_PP_HW_NUM_BITS_VID_MAC(devNum),&regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *vidRangePtr = (GT_U16)regValue;

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanRangeGet function
* @endinternal
*
* @brief   Function gets the valid VLAN Range.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] vidRangePtr              - (pointer to) VID range for VLAN filtering
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - vidRangePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanRangeGet
(
    IN  GT_U8   devNum,
    OUT GT_U16  *vidRangePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanRangeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vidRangePtr));

    rc = internal_cpssDxChBrgVlanRangeGet(devNum, vidRangePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vidRangePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortIsolationCmdSet function
* @endinternal
*
* @brief   Function sets port isolation command per egress VLAN.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id
* @param[in] cmd                      - port isolation command for given VID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or cmd
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortIsolationCmdSet
(
    IN  GT_U8                                      devNum,
    IN  GT_U16                                     vlanId,
    IN  CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_CMD_ENT  cmd
)
{
    GT_U32  value;  /* HW value for L2/L3 Port Isolation command */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    PRV_CPSS_DXCH_CONVERT_PORT_ISOLATION_MODE_TO_HW_VAL_MAC(value, cmd);

    /* write 2 bit for Port Isolation command */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        return prvCpssDxChWriteTableEntryField(
            devNum,
            CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_ATTRIBUTES_E,
            vlanId,/* vid */
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_PORT_ISOLATION_VLAN_CMD_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            value);
    }
    else
    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        return prvCpssDxChWriteTableEntryField(devNum,
                                           CPSS_DXCH_TABLE_VLAN_E,
                                           (GT_U32)vlanId,/* vid */
                                           3,
                                           21,
                                           2,
                                           value);
    }
    else
    {
        return prvCpssDxChWriteTableEntryField(devNum,
                                               CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
                                               (GT_U32)vlanId,
                                               PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                               263,
                                               2,
                                               value);
    }
}

/**
* @internal cpssDxChBrgVlanPortIsolationCmdSet function
* @endinternal
*
* @brief   Function sets port isolation command per egress VLAN.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id
* @param[in] cmd                      - port isolation command for given VID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or cmd
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortIsolationCmdSet
(
    IN  GT_U8                                      devNum,
    IN  GT_U16                                     vlanId,
    IN  CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_CMD_ENT  cmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortIsolationCmdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, cmd));

    rc = internal_cpssDxChBrgVlanPortIsolationCmdSet(devNum, vlanId, cmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, cmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanLocalSwitchingEnableSet function
* @endinternal
*
* @brief   Function sets local switching of Multicast, known and unknown Unicast,
*         and Broadcast traffic per VLAN.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id
* @param[in] trafficType              - local switching traffic type
* @param[in] enable                   - Enable/disable of local switching
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or trafficType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. To enable local switching of Multicast, unknown Unicast, and
*       Broadcast traffic, both this field in the VLAN entry and the
*       egress port configuration must be enabled for Multicast local switching
*       in function cpssDxChBrgPortEgressMcastLocalEnable.
*       2. To enable local switching of known Unicast traffic, both this
*       field in the VLAN entry and the ingress port configuration must
*       be enabled for Unicast local switching in function
*       cpssDxChBrgGenUcLocalSwitchingEnable.
*
*/
static GT_STATUS internal_cpssDxChBrgVlanLocalSwitchingEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlanId,
    IN  CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_ENT  trafficType,
    IN  GT_BOOL enable
)
{
    GT_U32  wordOffset;  /* word offset */
    GT_U32  fieldOffset; /* field offset */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        switch (trafficType)
        {
            case CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_KNOWN_UC_E:
                wordOffset = 5;
                fieldOffset = 17;
                break;

            case CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_FLOODED_E:
                wordOffset = 3;
                fieldOffset = 20;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

            /* write 1 bit local switching command */
        return prvCpssDxChWriteTableEntryField(devNum,
                                               CPSS_DXCH_TABLE_VLAN_E,
                                               (GT_U32)vlanId,/* vid */
                                               wordOffset,
                                               fieldOffset,
                                               1,
                                               BOOL2BIT_MAC(enable));
    }
    else
    {
        switch (trafficType)
        {
            case CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_KNOWN_UC_E:
                if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
                {
                    /* set the <UC_LOCAL_EN>*/
                    return prvCpssDxChWriteTableEntryField(devNum,
                                                    CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                                    vlanId,/* vid */
                                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UC_LOCAL_EN_E, /* field name */
                                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                    BOOL2BIT_MAC(enable));
                }
                else
                {
                    /* write to ingress table */
                    return prvCpssDxChWriteTableEntryField(devNum,
                                                           CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                                           (GT_U32)vlanId,
                                                           PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                                           128,
                                                           1,
                                                           BOOL2BIT_MAC(enable));
                }

            case CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_FLOODED_E:
                if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
                {
                    return prvCpssDxChWriteTableEntryField(
                        devNum,
                        CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_ATTRIBUTES_E,
                        vlanId,/* vid */
                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                        SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_LOCAL_SWITHING_EN_E, /* field name */
                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                        BOOL2BIT_MAC(enable));
                }
                else
                {
                    /* write to egress table */
                    return prvCpssDxChWriteTableEntryField(devNum,
                                                           CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
                                                           (GT_U32)vlanId,
                                                           PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                                           262,
                                                           1,
                                                           BOOL2BIT_MAC(enable));

                }
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
}

/**
* @internal cpssDxChBrgVlanLocalSwitchingEnableSet function
* @endinternal
*
* @brief   Function sets local switching of Multicast, known and unknown Unicast,
*         and Broadcast traffic per VLAN.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id
* @param[in] trafficType              - local switching traffic type
* @param[in] enable                   - Enable/disable of local switching
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or trafficType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. To enable local switching of Multicast, unknown Unicast, and
*       Broadcast traffic, both this field in the VLAN entry and the
*       egress port configuration must be enabled for Multicast local switching
*       in function cpssDxChBrgPortEgressMcastLocalEnable.
*       2. To enable local switching of known Unicast traffic, both this
*       field in the VLAN entry and the ingress port configuration must
*       be enabled for Unicast local switching in function
*       cpssDxChBrgGenUcLocalSwitchingEnable.
*
*/
GT_STATUS cpssDxChBrgVlanLocalSwitchingEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlanId,
    IN  CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_ENT  trafficType,
    IN  GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanLocalSwitchingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, trafficType, enable));

    rc = internal_cpssDxChBrgVlanLocalSwitchingEnableSet(devNum, vlanId, trafficType, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, trafficType, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanMirrorToTxAnalyzerSet function
* @endinternal
*
* @brief   Function sets Tx Analyzer port index to given VLAN.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id
* @param[in] enable                   - Enable/disable Tx Mirroring
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
* @param[in] txAnalyzerIndex          - Tx Analyzer index
*                                      (APPLICABLE RANGES: 0..6)
*                                      relevant only if mirrToTxAnalyzerEn == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or trafficType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanMirrorToTxAnalyzerSet
(
    IN  GT_U8       devNum,
    IN  GT_U16      vlanId,
    IN  GT_BOOL     enable,
    IN  GT_U32      txAnalyzerIndex
)
{
    GT_U32  hwValue;  /* HW value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (enable == GT_FALSE)
    {
        hwValue = 0;
    }
    else
    {
        if (txAnalyzerIndex > 6)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        hwValue = (txAnalyzerIndex + 1);
    }

    return prvCpssDxChWriteTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_ATTRIBUTES_E,
        vlanId,/* vid */
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_EGRESS_MIRROR_TO_ANALYZER_INDEX_E, /* field name */
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        hwValue);
}

/**
* @internal cpssDxChBrgVlanMirrorToTxAnalyzerSet function
* @endinternal
*
* @brief   Function sets Tx Analyzer port index to given VLAN.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id
* @param[in] enable                   - Enable/disable Tx Mirroring
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
* @param[in] txAnalyzerIndex          - Tx Analyzer index
*                                      (APPLICABLE RANGES: 0..6)
*                                      relevant only if mirrToTxAnalyzerEn == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or trafficType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanMirrorToTxAnalyzerSet
(
    IN  GT_U8       devNum,
    IN  GT_U16      vlanId,
    IN  GT_BOOL     enable,
    IN  GT_U32      txAnalyzerIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanMirrorToTxAnalyzerSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, enable, txAnalyzerIndex));

    rc = internal_cpssDxChBrgVlanMirrorToTxAnalyzerSet(devNum, vlanId, enable, txAnalyzerIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, enable, txAnalyzerIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanMirrorToTxAnalyzerModeSet function
* @endinternal
*
* @brief   Function sets Egress Mirroring mode to given VLAN.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id
* @param[in] egressMirroringMode      - one of 4 options of the enum mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or egressMirroringMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanMirrorToTxAnalyzerModeSet
(
    IN  GT_U8                            devNum,
    IN  GT_U16                           vlanId,
    IN  CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode
)
{
    GT_U32  hwValue;  /* HW value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    PRV_CPSS_DXCH_MIRROR_CONVERT_EGRESS_MIRROR_MODE_TO_HW_VAL_MAC(hwValue, egressMirroringMode);

    return prvCpssDxChWriteTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_ATTRIBUTES_E,
        vlanId,/* vid */
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP6_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E, /* field name */
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        hwValue);
}

/**
* @internal cpssDxChBrgVlanMirrorToTxAnalyzerModeSet function
* @endinternal
*
* @brief   Function sets Egress Mirroring mode to given VLAN.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id
* @param[in] egressMirroringMode      - one of 4 options of the enum mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or egressMirroringMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanMirrorToTxAnalyzerModeSet
(
    IN  GT_U8                            devNum,
    IN  GT_U16                           vlanId,
    IN  CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirroringMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanMirrorToTxAnalyzerModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, egressMirroringMode));

    rc = internal_cpssDxChBrgVlanMirrorToTxAnalyzerModeSet(devNum, vlanId, egressMirroringMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, egressMirroringMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanFloodVidxModeSet function
* @endinternal
*
* @brief   Function sets Flood VIDX and Flood VIDX Mode per VLAN.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id
* @param[in] floodVidx                - VIDX value (12bit) - VIDX value applied to Broadcast,
*                                      Unregistered Multicast and unknown Unicast packets,
*                                      depending on the VLAN entry field <Flood VIDX Mode>.
* @param[in] floodVidxMode            - Flood VIDX Mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or floodVidxMode
* @retval GT_OUT_OF_RANGE          - illegal floodVidx
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanFloodVidxModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U16                                  vlanId,
    IN  GT_U16                                  floodVidx,
    IN  CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT  floodVidxMode
)
{
    GT_U32  hwValue;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    if (floodVidx > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_VIDX_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_CONVERT_FLOOD_VIDX_MODE_TO_HW_VAL_MAC(hwValue, floodVidxMode);

    if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(devNum))
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_TABLE_VLAN_E,
                                             (GT_U32)vlanId,/* vid */
                                             3,
                                             8,
                                             12,
                                             (GT_U32)floodVidx);
        if (rc != GT_OK)
        {
            return rc;
        }

            /* write 1 bit floodVidxMode */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                               CPSS_DXCH_TABLE_VLAN_E,
                                               (GT_U32)vlanId,/* vid */
                                               5,
                                               18,
                                               1,
                                               hwValue);
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* set the <FLOOD_EVIDX>*/
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FLOOD_EVIDX_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        floodVidx);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* set the <FLOOD_VIDX_MODE>*/
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FLOOD_VIDX_MODE_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        hwValue);
    }
    else
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                             (GT_U32)vlanId,
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                             104,
                                             12,
                                             (GT_U32)floodVidx);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* write 1 bit floodVidxMode */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                                               CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                               (GT_U32)vlanId,
                                               PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                               129,
                                               1,
                                               hwValue);

        if (rc != GT_OK)
        {
            return rc;
        }

    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanFloodVidxModeSet function
* @endinternal
*
* @brief   Function sets Flood VIDX and Flood VIDX Mode per VLAN.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id
* @param[in] floodVidx                - VIDX value (12bit) - VIDX value applied to Broadcast,
*                                      Unregistered Multicast and unknown Unicast packets,
*                                      depending on the VLAN entry field <Flood VIDX Mode>.
* @param[in] floodVidxMode            - Flood VIDX Mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or floodVidxMode
* @retval GT_OUT_OF_RANGE          - illegal floodVidx
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanFloodVidxModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U16                                  vlanId,
    IN  GT_U16                                  floodVidx,
    IN  CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT  floodVidxMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanFloodVidxModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, floodVidx, floodVidxMode));

    rc = internal_cpssDxChBrgVlanFloodVidxModeSet(devNum, vlanId, floodVidx, floodVidxMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, floodVidx, floodVidxMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal sip5VlanHwEntryBuildIngress function
* @endinternal
*
* @brief   Bobcat2; Caelum; Aldrin; AC3X; Bobcat3 : Builds Ingress vlan entry to buffer.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanInfoPtr              - (pointer to) VLAN specific information
*
* @param[out] hwDataArr[VLAN_INGRESS_ENTRY_WORDS_NUM_CNS]
*                                      - pointer to hw VLAN entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr or portsTaggingPtr is
*                                       out of range
* @retval or vlanInfoPtr           ->stgId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS sip5VlanHwEntryBuildIngress
(
    IN GT_U8                        devNum,
    IN CPSS_DXCH_BRG_VLAN_INFO_STC  *vlanInfoPtr,
    OUT GT_U32                      hwDataArr[VLAN_INGRESS_ENTRY_WORDS_NUM_CNS]
)
{
    GT_U32      value;          /* value in HW format */



    /* reset all words in hwDataArr */
    cpssOsMemSet((char *) &hwDataArr[0], 0, VLAN_INGRESS_ENTRY_WORDS_NUM_CNS * sizeof(GT_U32));

    /* valid bit */
    value = 1;
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_VALID_E);

    /* NewSrcAddrIsNotSecurityBreach - revert the bit */
    value = 1 - (BOOL2BIT_MAC(vlanInfoPtr->unkSrcAddrSecBreach));
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_NEW_SRC_ADDR_IS_NOT_SECURITY_BREACH_E);

    /* UnregisteredNonIPMulticastCmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                             (vlanInfoPtr->unregNonIpMcastCmd))
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_NON_IP_MULTICAST_CMD_E);

    /* UnregisteredIpv4MulticastCmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                             (vlanInfoPtr->unregIpv4McastCmd))
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV4_MULTICAST_CMD_E);


    /* UnregisteredIpv6MulticastCmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                             (vlanInfoPtr->unregIpv6McastCmd))
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV6_MULTICAST_CMD_E);

    /* UnknownUnicastCmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                             (vlanInfoPtr->unkUcastCmd))
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNKNOWN_UNICAST_CMD_E);

    /* Ipv4IgmpToCpuEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv4IgmpToCpuEn);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IGMP_TO_CPU_EN_E);

    if(vlanInfoPtr->mirrToRxAnalyzerEn == GT_FALSE) /* ignore index and set 0 to all bits */
    {
        value = 0;
    }
    else
    {
        if(vlanInfoPtr->mirrToRxAnalyzerIndex >  PRV_CPSS_DXCH_MIRROR_TO_ANALYZER_MAX_INDEX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        value = vlanInfoPtr->mirrToRxAnalyzerIndex + 1;
    }
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_MIRROR_TO_INGRESS_ANALYZER_E);

    /* Ipv6IcmpToCpuEn*/
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv6IcmpToCpuEn);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_ICMP_TO_CPU_EN_E);

    /* check ipCtrlToCpuEn for ipv4 */
    switch(vlanInfoPtr->ipCtrlToCpuEn)
    {
        case CPSS_DXCH_BRG_IP_CTRL_NONE_E     :
        case CPSS_DXCH_BRG_IP_CTRL_IPV6_E     :
            value = 0;
            break;
        case CPSS_DXCH_BRG_IP_CTRL_IPV4_E     :
        case CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E:
            value = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Ipv4ControlToCpuEn*/
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_CONTROL_TO_CPU_EN_E);

    /* Ipv4IpmBridgingEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv4IpmBrgEn);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IPM_BRIDGING_EN_E);

    /* Ipv6IpmBridgingEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv6IpmBrgEn);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_IPM_BRIDGING_EN_E);

    /* Ipv4IpmBridgingMode */
    PRV_DXCH_BRG_VLAN_IPM_MODE_CONVERT_MAC(vlanInfoPtr->ipv4IpmBrgMode, value);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IPM_BRIDGING_MODE_E);

    /* Ipv6IpmBridgingMode */
    PRV_DXCH_BRG_VLAN_IPM_MODE_CONVERT_MAC(vlanInfoPtr->ipv6IpmBrgMode, value);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_IPM_BRIDGING_MODE_E);

    /* UnregisteredIpv4BcCmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                            vlanInfoPtr->unregIpv4BcastCmd);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV4_BC_CMD_E);

    /* UnregisteredNonIpv4BcCmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                            vlanInfoPtr->unregNonIpv4BcastCmd);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_NON_IPV4_BC_CMD_E);

    /* Ipv4UnicastRouteEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv4UcastRouteEn);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_UNICAST_ROUTE_EN_E);

    /* Ipv4MulticastRouteEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv4McastRouteEn);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_MULTICAST_ROUTE_EN_E);

    /* Ipv6UnicastRouteEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv6UcastRouteEn);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_UNICAST_ROUTE_EN_E);

    /* Ipv6MulticastRouteEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv6McastRouteEn);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_MULTICAST_ROUTE_EN_E);

    switch(vlanInfoPtr->ipv6SiteIdMode)
    {
        case CPSS_IP_SITE_ID_INTERNAL_E:
            value = 0;
            break;
        case CPSS_IP_SITE_ID_EXTERNAL_E:
            value = 1;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* Ipv6SiteId */
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_SITEID_E);

    /* AutoLearnDis */
    value = BOOL2BIT_MAC(vlanInfoPtr->autoLearnDisable);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_AUTO_LEARN_DIS_E);

    /* NaMsgToCpuEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->naMsgToCpuEn);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_NA_MSG_TO_CPU_EN_E);

    value = vlanInfoPtr->mruIdx;
    /* check MRU index */
    PRV_CPSS_DXCH_BRG_MRU_INDEX_CHECK_MAC(value);
    /* MRU Index */
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_MRU_INDEX_E);

    /* BcUdpTrapMirrorEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->bcastUdpTrapMirrEn);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_BC_UDP_TRAP_MIRROR_EN_E);

    /* check ipCtrlToCpuEn for ipv6 */
    switch(vlanInfoPtr->ipCtrlToCpuEn)
    {
        case CPSS_DXCH_BRG_IP_CTRL_NONE_E     :
        case CPSS_DXCH_BRG_IP_CTRL_IPV4_E     :
            value = 0;
            break;
        case CPSS_DXCH_BRG_IP_CTRL_IPV6_E     :
        case CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E:
            value = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Ipv6ControlToCpuEn */
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_CONTROL_TO_CPU_EN_E);

    if(vlanInfoPtr->floodVidx > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_VIDX_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    value = vlanInfoPtr->floodVidx;
    /* Vidx */
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FLOOD_EVIDX_E);

    if(vlanInfoPtr->vrfId >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.vrfIdNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* VrfId */
        value = vlanInfoPtr->vrfId;
        SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_VRF_ID_E);
    }

    /* UcLocalEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->ucastLocalSwitchingEn);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UC_LOCAL_EN_E);

    /* FloodVidxMode */
    PRV_CPSS_DXCH_CONVERT_FLOOD_VIDX_MODE_TO_HW_VAL_MAC(value, vlanInfoPtr->floodVidxMode);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FLOOD_VIDX_MODE_E);

    if(vlanInfoPtr->fidValue > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* fid value  */
    value = vlanInfoPtr->fidValue;
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FID_E);

    /* unknown Mac Sa Command  */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value, vlanInfoPtr->unknownMacSaCmd);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNKOWN_MAC_SA_CMD_E);

    /* IPv4 MC/BC Mirror to Analyzer Index */
    if(vlanInfoPtr->ipv4McBcMirrToAnalyzerEn == GT_FALSE) /* ignore index and set 0 to all bits */
    {
        value = 0;
    }
    else
    {
        if(vlanInfoPtr->ipv4McBcMirrToAnalyzerIndex >  PRV_CPSS_DXCH_MIRROR_TO_ANALYZER_MAX_INDEX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        value = vlanInfoPtr->ipv4McBcMirrToAnalyzerIndex + 1;
    }

    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_MC_BC_TO_MIRROR_ANLYZER_IDX_E);

    /* IPv6 MC Mirror to Analyzer Index */
    if(vlanInfoPtr->ipv6McMirrToAnalyzerEn == GT_FALSE) /* ignore index and set 0 to all bits */
    {
        value = 0;
    }
    else
    {
        if(vlanInfoPtr->ipv6McMirrToAnalyzerIndex >  PRV_CPSS_DXCH_MIRROR_TO_ANALYZER_MAX_INDEX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        value = vlanInfoPtr->ipv6McMirrToAnalyzerIndex + 1;
    }

    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_MC_TO_MIRROR_ANALYZER_IDX_E);

    value = BOOL2BIT_MAC(vlanInfoPtr->fcoeForwardingEn);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(
        devNum,value, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FCOE_FORWARDING_EN_E);

    PRV_CPSS_DXCH_CONVERT_UNREG_IPM_EVIDX_MODE_TO_HW_VAL_MAC(
        value, vlanInfoPtr->unregIpmEVidxMode);
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(
        devNum,value, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREG_IPM_EVIDX_MODE_E);

    if (vlanInfoPtr->unregIpmEVidx >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    value = vlanInfoPtr->unregIpmEVidx;
    SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(
        devNum,value, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREG_IPM_EVIDX_E);

    /* FDB Lookup Key mode */
    if (PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        PRV_CPSS_DXCH_CONVERT_FDB_LOOKUP_KEY_MODE_TO_HW_VAL_MAC(value,
                                                 vlanInfoPtr->fdbLookupKeyMode);
        SET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,
                     SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FDB_LOOKUP_KEY_MODE_E);
    }

    return GT_OK;
}

/**
* @internal prvCpssBrgVlanHwEntryBuildIngress function
* @endinternal
*
* @brief   Builds Ingress vlan entry to buffer.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portsMembersPtr          - (pointer to) bmp of ports members in vlan
* @param[in] vlanInfoPtr              - (pointer to) VLAN specific information
*
* @param[out] hwDataArr[VLAN_INGRESS_ENTRY_WORDS_NUM_CNS]
*                                      - pointer to hw VLAN entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr or portsTaggingPtr is
*                                       out of range
* @retval or vlanInfoPtr           ->stgId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS prvCpssBrgVlanHwEntryBuildIngress
(
    IN GT_U8                        devNum,
    IN GT_U32                       *portsMembersPtr,
    IN CPSS_DXCH_BRG_VLAN_INFO_STC  *vlanInfoPtr,
    OUT GT_U32                      hwDataArr[VLAN_INGRESS_ENTRY_WORDS_NUM_CNS]
)
{
    GT_U32      value;          /* value in HW format */

    /* reset all words in hwDataArr */
    cpssOsMemSet((char *) &hwDataArr[0], 0, VLAN_INGRESS_ENTRY_WORDS_NUM_CNS * sizeof(GT_U32));

    /* bit 0 - valid bit */
    value = 1;
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,0,1,value);

    /* bit 1 - NewSrcAddrIsNotSecurityBreach - revert the bit */
    value = 1 - (BOOL2BIT_MAC(vlanInfoPtr->unkSrcAddrSecBreach));
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,1,1,value);

    /* bit 2..65 port members */
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr, 2,32,portsMembersPtr[0]);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,34,32,portsMembersPtr[1]);

    /* bits 66..68 - UnregisteredNonIPMulticastCmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                             (vlanInfoPtr->unregNonIpMcastCmd))
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,66,3,value);

    /* bits 69..71 - UnregisteredIpv4MulticastCmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                             (vlanInfoPtr->unregIpv4McastCmd))
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,69,3,value);


    /* bits 72..74 - UnregisteredIpv6MulticastCmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                             (vlanInfoPtr->unregIpv6McastCmd))
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,72,3,value);

    /* bits 75..77 - UnknownUnicastCmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                             (vlanInfoPtr->unkUcastCmd))
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,75,3,value);

    /* bit 78 - Ipv4IgmpToCpuEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv4IgmpToCpuEn);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,78,1,value);

    /* bit 81 - mirrToRxAnalyzerEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->mirrToRxAnalyzerEn);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,81,1,value);


    /* bit 82 - Ipv6IcmpToCpuEn*/
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv6IcmpToCpuEn);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,82,1,value);

    /* check ipCtrlToCpuEn for ipv4 */
    switch(vlanInfoPtr->ipCtrlToCpuEn)
    {
        case CPSS_DXCH_BRG_IP_CTRL_NONE_E     :
        case CPSS_DXCH_BRG_IP_CTRL_IPV6_E     :
            value = 0;
            break;
        case CPSS_DXCH_BRG_IP_CTRL_IPV4_E     :
        case CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E:
            value = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* bit 83 - Ipv4ControlToCpuEn*/
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,83,1,value);

    /* bit 84 - Ipv4IpmBridgingEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv4IpmBrgEn);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,84,1,value);

    /* bit 85 - Ipv6IpmBridgingEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv6IpmBrgEn);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,85,1,value);

    /* bit 79 - Ipv4IpmBridgingMode */
    PRV_DXCH_BRG_VLAN_IPM_MODE_CONVERT_MAC(vlanInfoPtr->ipv4IpmBrgMode, value);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,79,1,value);

    /* bit 80 - Ipv6IpmBridgingMode */
    PRV_DXCH_BRG_VLAN_IPM_MODE_CONVERT_MAC(vlanInfoPtr->ipv6IpmBrgMode, value);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,80,1,value);

    /* bits 86..88 - UnregisteredIpv4BcCmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                            vlanInfoPtr->unregIpv4BcastCmd);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,86,3,value);

    /* bits 89..91 - UnregisteredNonIpv4BcCmd */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(value,
                                            vlanInfoPtr->unregNonIpv4BcastCmd);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,89,3,value);

    /* bit 92 - Ipv4UnicastRouteEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv4UcastRouteEn);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,92,1,value);

    /* bit 93 - Ipv4MulticastRouteEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv4McastRouteEn);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,93,1,value);

    /* bit 94 - Ipv6UnicastRouteEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv6UcastRouteEn);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,94,1,value);

    /* bit 95 - Ipv6MulticastRouteEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->ipv6McastRouteEn);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,95,1,value);

    switch(vlanInfoPtr->ipv6SiteIdMode)
    {
        case CPSS_IP_SITE_ID_INTERNAL_E:
            value = 0;
            break;
        case CPSS_IP_SITE_ID_EXTERNAL_E:
            value = 1;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* bit 96 - Ipv6SiteId */
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,96,1,value);

    /* bit 97 - AutoLearnDis */
    value = BOOL2BIT_MAC(vlanInfoPtr->autoLearnDisable);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,97,1,value);

    /* bit 98 - NaMsgToCpuEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->naMsgToCpuEn);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,98,1,value);

    value = vlanInfoPtr->mruIdx;
    /* check MRU index */
    PRV_CPSS_DXCH_BRG_MRU_INDEX_CHECK_MAC(value);
    /* bits 99..101 MRU Index */
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,99,3,value);

    /* bit 102 - BcUdpTrapMirrorEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->bcastUdpTrapMirrEn);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,102,1,value);

    /* check ipCtrlToCpuEn for ipv6 */
    switch(vlanInfoPtr->ipCtrlToCpuEn)
    {
        case CPSS_DXCH_BRG_IP_CTRL_NONE_E     :
        case CPSS_DXCH_BRG_IP_CTRL_IPV4_E     :
            value = 0;
            break;
        case CPSS_DXCH_BRG_IP_CTRL_IPV6_E     :
        case CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E:
            value = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* bit 103 - Ipv6ControlToCpuEn */
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,103,1,value);

    if(vlanInfoPtr->floodVidx > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_VIDX_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    value = vlanInfoPtr->floodVidx;
    /* bit 104..115 - Vidx */
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,104,12,value);

    if(vlanInfoPtr->vrfId >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.vrfIdNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    /* bit 116..127 - VrfId */
    value = vlanInfoPtr->vrfId;
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,116,12,value);

    /* bit 128 - UcLocalEn */
    value = BOOL2BIT_MAC(vlanInfoPtr->ucastLocalSwitchingEn);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,128,1,value);

    /* bit 129 - FloodVidxMode */
    PRV_CPSS_DXCH_CONVERT_FLOOD_VIDX_MODE_TO_HW_VAL_MAC(value, vlanInfoPtr->floodVidxMode);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,129,1,value);

    if(vlanInfoPtr->stgId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_STG_ID_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    value = vlanInfoPtr->stgId;
    /* bit 130..137 - SpanStateGroupIndex */
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,130,8,value);

    return GT_OK;
}

/**
* @internal prvCpssBrgVlanHwEntryBuildEgress function
* @endinternal
*
* @brief   Builds Egress vlan entry to buffer.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portsMembersPtr          - (pointer to) bmp of ports members in vlan
* @param[in] portsTaggingPtr          - (pointer to) bmp of ports tagged in the vlan -
*                                      the relevant parameter for DxCh3 and beyond and
*                                      not relevant for TR101 supported devices.
* @param[in] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[in] portsTaggingCmdPtr       - (pointer to) ports tagging commands in the vlan -
*                                      the relevant parameter for xCat3 and above
*                                      with TR101 feature support.
*
* @param[out] hwDataArr[VLAN_EGRESS_ENTRY_WORDS_NUM_CNS]
*                                      - pointer to hw VLAN entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr or portsTaggingPtr is
*                                       out of range
* @retval or vlanInfoPtr           ->stgId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS prvCpssBrgVlanHwEntryBuildEgress
(
    IN GT_U8                        devNum,
    IN GT_U32                       *portsMembersPtr,
    IN GT_U32                       *portsTaggingPtr,
    IN CPSS_DXCH_BRG_VLAN_INFO_STC  *vlanInfoPtr,
    IN CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT  *portsTaggingCmdPtr,
    OUT GT_U32                      hwDataArr[VLAN_EGRESS_ENTRY_WORDS_NUM_CNS]
)
{
    GT_U32      value;          /* value in HW format */
    GT_U32      ii;             /* iterator */
    GT_U32      tagCmdMaxPorts = 63;  /* 63 ports with tagCmd -- NOT 64 !!! */
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT currTagCmd;
    GT_U32      startBit;/* start bit in entry */
    GT_BOOL     tr101Supported;   /* TR101 Feature Support */

    /* reset all words in hwDataArr */
    cpssOsMemSet((char *) &hwDataArr[0], 0, VLAN_EGRESS_ENTRY_WORDS_NUM_CNS * sizeof(GT_U32));

    /* get TR101 feature support */
    tr101Supported = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.tr101Supported;

    /* bit 0 - valid bit */
    value = 1;
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,0,1,value);

    /* bit 1..64 port members */
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr, 1,32,portsMembersPtr[0]);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,33,32,portsMembersPtr[1]);

    startBit = 65;
    /* tagCmd info for all ports */
    for (ii = 0; ii < tagCmdMaxPorts; ii++ , startBit += 3)
    {
        if (tr101Supported == GT_FALSE)
        {
            /*U32_GET_FIELD_IN_ENTRY_MAC*/
            currTagCmd = ((portsTaggingPtr[(ii)>>5] & (1 << ((ii)& 0x1f)))? 1 : 0) ?
                CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E :
                CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
        }
        else
        {
            currTagCmd = portsTaggingCmdPtr[ii];
        }

        PRV_CPSS_DXCH_CONVERT_VLAN_TAG_MODE_TO_HW_VAL_MAC(devNum, value,currTagCmd);

        /* 65..253 - tagCmd of ports */
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,startBit, 3 ,value);
    }

    value = vlanInfoPtr->stgId;
    /* bit 254..261 - SpanStateGroupIndex */
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,254,8,value);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        value = (vlanInfoPtr->stgId >> 8);
        /* bit 265..268 - extra 4 bits from SpanStateGroupIndex */
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,265,4,value);
    }

    /* bit 262 - LocalSwithingEn*/
    value = BOOL2BIT_MAC(vlanInfoPtr->mcastLocalSwitchingEn);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,262,1,value);

    /* bits 263..264 - PortIsolationVlanCmd */
    PRV_CPSS_DXCH_CONVERT_PORT_ISOLATION_MODE_TO_HW_VAL_MAC(value, vlanInfoPtr->portIsolationMode);
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,263,2,value);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* Mirror to egress analyzer port- new field bits [269:271] */

        if(vlanInfoPtr->mirrToTxAnalyzerEn == GT_FALSE) /* ignore index and set 0 to all bits */
        {
            value = 0;
        }
        else
        {
            if(vlanInfoPtr->mirrToTxAnalyzerIndex >  PRV_CPSS_DXCH_MIRROR_TO_ANALYZER_MAX_INDEX_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            value = vlanInfoPtr->mirrToTxAnalyzerIndex + 1;
        }

        U32_SET_FIELD_IN_ENTRY_MAC(hwDataArr,269,3,value);
    }


    return GT_OK;
}

/**
* @internal sip5VlanHwEntryBuildEgress function
* @endinternal
*
* @brief   Builds Egress vlan entry to buffer - Bobcat2; Caelum; Aldrin; AC3X; Bobcat3
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; .
*
* @param[in] devNum                   - device number
* @param[in] portsMembersPtr          - (pointer to) bmp of ports members in vlan
* @param[in] portsTaggingPtr          - (pointer to) bmp of ports tagged in the vlan -
*                                      not relevant for TR101 supported devices.
* @param[in] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[in] portsTaggingCmdPtr       - (pointer to) ports tagging commands in the vlan -
*                                      the relevant parameter for xCat3 and above
*                                      with TR101 feature support.
*
* @param[out] egfSht_hwDataArr[VLAN_EGRESS_ENTRY_WORDS_NUM_CNS] - pointer to hw VLAN entry - EGF_SHT entry
* @param[out] egfShtVlanAttrib_hwDataArr[1] - pointer to hw VLAN Attributes entry
* @param[out] egfShtVlanSpanId_hwDataArr[1] - pointer to hw VLAN SpanTree Id entry
* @param[out] egfQag_hwDataArr[EGF_QAG_EGRESS_VLAN_ENTRY_WORDS_NUM_CNS] - pointer to hw VLAN entry - EGF_QAG entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr or portsTaggingPtr is
*                                       out of range
* @retval or vlanInfoPtr           ->stgId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS sip5VlanHwEntryBuildEgress
(
    IN GT_U8                        devNum,
    IN GT_U32                       *portsMembersPtr,
    IN GT_U32                       *portsTaggingPtr,
    IN CPSS_DXCH_BRG_VLAN_INFO_STC  *vlanInfoPtr,
    IN CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT  *portsTaggingCmdPtr,
    OUT GT_U32                       egfSht_hwDataArr[VLAN_EGRESS_ENTRY_WORDS_NUM_CNS],
    OUT GT_U32                       egfShtVlanAttrib_hwDataArr[1],
    OUT GT_U32                       egfShtVlanSpanId_hwDataArr[1],
    OUT GT_U32                       egfQag_hwDataArr[EGF_QAG_EGRESS_VLAN_ENTRY_WORDS_NUM_CNS]
)
{
    GT_U32      value;          /* value in HW format */
    GT_U32      ii;             /* iterator */
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT currTagCmd;
    GT_BOOL     tr101Supported;   /* TR101 Feature Support */
    GT_U32      mirrorIndex;      /* mirror index in HW format */
    CPSS_PORTS_BMP_STC          existingPorts;

    /* reset all words in hwDataArr */
    cpssOsMemSet((char *) &egfSht_hwDataArr[0], 0, VLAN_EGRESS_ENTRY_WORDS_NUM_CNS * sizeof(GT_U32));
    cpssOsMemSet((char *) &egfQag_hwDataArr[0], 0, EGF_QAG_EGRESS_VLAN_ENTRY_WORDS_NUM_CNS * sizeof(GT_U32));

    /* get TR101 feature support */
    tr101Supported = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.tr101Supported;

    /* Mirror to egress analyzer port */
    if(vlanInfoPtr->mirrToTxAnalyzerEn == GT_FALSE) /* ignore index and set 0 to all bits */
    {
        mirrorIndex = 0;
    }
    else
    {
        if(vlanInfoPtr->mirrToTxAnalyzerIndex >  PRV_CPSS_DXCH_MIRROR_TO_ANALYZER_MAX_INDEX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        mirrorIndex = vlanInfoPtr->mirrToTxAnalyzerIndex + 1;
    }

/**************** EGF_SHT section *********************************************/

    if (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* valid bit */
        value = 1;
        SET_FIELD_IN_EGF_SHT_EGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS_VALID_E);
    }

    /* port members */
    U_ANY_SET_FIELD_BY_ID_MAC(egfSht_hwDataArr,
        PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_EGRESS_EVLAN_E].fieldsInfoPtr,
        SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS_PORT_X_MEMBER_E,
        portsMembersPtr);/* pointer to the array of 256 ports */

    /* SpanStateGroupIndex */
    if (vlanInfoPtr->stgId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_STG_ID_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    egfShtVlanSpanId_hwDataArr[0] = vlanInfoPtr->stgId;

    egfShtVlanAttrib_hwDataArr[0] = 0;

    /* LocalSwithingEn*/
    value = BOOL2BIT_MAC(vlanInfoPtr->mcastLocalSwitchingEn);

    U32_SET_FIELD_BY_ID_MAC(
        egfShtVlanAttrib_hwDataArr,
        PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_VLAN_ATTRIBUTES_E].fieldsInfoPtr,
        SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_LOCAL_SWITHING_EN_E,
        value);

    /* PortIsolationVlanCmd */
    PRV_CPSS_DXCH_CONVERT_PORT_ISOLATION_MODE_TO_HW_VAL_MAC(value, vlanInfoPtr->portIsolationMode);

    U32_SET_FIELD_BY_ID_MAC(
        egfShtVlanAttrib_hwDataArr,
        PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_VLAN_ATTRIBUTES_E].fieldsInfoPtr,
        SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_PORT_ISOLATION_VLAN_CMD_E,
        value);

    U32_SET_FIELD_BY_ID_MAC(
        egfShtVlanAttrib_hwDataArr,
        PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_VLAN_ATTRIBUTES_E].fieldsInfoPtr,
        SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_EGRESS_MIRROR_TO_ANALYZER_INDEX_E,
        mirrorIndex);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* EgressMirroringMode */
        PRV_CPSS_DXCH_MIRROR_CONVERT_EGRESS_MIRROR_MODE_TO_HW_VAL_MAC(value, vlanInfoPtr->mirrToTxAnalyzerMode);

        U32_SET_FIELD_BY_ID_MAC(
            egfShtVlanAttrib_hwDataArr,
            PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_VLAN_ATTRIBUTES_E].fieldsInfoPtr,
            SIP6_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E,
            value);
    }

/**************** EGF_QAG section *********************************************/

    /* get the bmp of supported port on this device */
    prvCpssDrvPortsFullMaskGet(
        PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum),
            &existingPorts);

    /* tagCmd info for all ports */
    for (ii = 0; ii < CPSS_MAX_PORTS_NUM_CNS ; ii++)
    {
        if (tr101Supported == GT_FALSE)
        {
            /*U32_GET_FIELD_IN_ENTRY_MAC*/
            currTagCmd = ((portsTaggingPtr[(ii)>>5] & (1 << ((ii)& 0x1f)))? 1 : 0) ?
                CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E :
                CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
        }
        else
        {
            currTagCmd = portsTaggingCmdPtr[ii];
        }

        PRV_CPSS_DXCH_CONVERT_VLAN_TAG_MODE_TO_HW_VAL_MAC(devNum, value,currTagCmd);

        if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&existingPorts,ii))
        {
            /* make sure that caller 'ignores' non exists ports */
            if(currTagCmd != CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            continue;
        }

        /* tagCmd of ports */
        U32_SET_FIELD_IN_ENTRY_MAC(egfQag_hwDataArr,(ii*3), 3 ,value);
    }

    return GT_OK;
}



/**
* @internal vlanEntrySplitTableWrite function
* @endinternal
*
* @brief   Builds and writes vlan entry to HW - for split table (ingress,egress)
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] portsMembersPtr          - (pointer to) bmp of ports members in vlan
* @param[in] portsTaggingPtr          - (pointer to) bmp of ports tagged in the vlan -
*                                      The parameter is relevant for xCat3 and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat3 and above devices with TR101 feature
*                                      support.
* @param[in] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[in] portsTaggingCmdPtr       - (pointer to) ports tagging commands in the vlan -
*                                      The parameter is relevant only for xCat3 and
*                                      above with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr, portsTaggingPtr
* @retval or vlanInfoPtr           ->stgId is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS vlanEntrySplitTableWrite
(
    IN  GT_U8                                 devNum,
    IN  GT_U16                                vlanId,
    IN  CPSS_PORTS_BMP_STC                    *portsMembersPtr,
    IN  CPSS_PORTS_BMP_STC                    *portsTaggingPtr,
    IN  CPSS_DXCH_BRG_VLAN_INFO_STC           *vlanInfoPtr,
    IN  CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr
)
{
    GT_STATUS   rc;
    GT_U32      i;                /* iterator */
    /* ingress HW format */
    GT_U32      ingressHwData[VLAN_INGRESS_ENTRY_WORDS_NUM_CNS];
    /* egress HW format */
    GT_U32      egressHwData[VLAN_EGRESS_ENTRY_WORDS_NUM_CNS];
    GT_U32      efgQag_egressHwData[EGF_QAG_EGRESS_VLAN_ENTRY_WORDS_NUM_CNS];/*Bobcat2: EGF_QAG info*/

    CPSS_PORTS_BMP_STC portsMembersBmp; /* local variable to avoid portsMembersPtr change */
    GT_U32      egfShtVlanAttrib_hwDataArr[1]; /* HW entry of Ergess Vlan Attributes Table  */
    GT_U32      egfShtVlanSpanId_hwDataArr[1]; /* HW entry of Ergess Vlan Span Tree Id Table */
    GT_U32      portNum;
    const GT_PHYSICAL_PORT_NUM            *reservedPortsArrPtr;

    /* CPU port conversion */
    cpssOsMemCpy(&portsMembersBmp, portsMembersPtr, sizeof(CPSS_PORTS_BMP_STC));

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        reservedPortsArrPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.reservedPortsPtr;
        if (reservedPortsArrPtr)
        {
            /* Iterate all reserved ports in array */
            for (i = 0; reservedPortsArrPtr[i] != GT_NA; i++)
            {
                portNum = reservedPortsArrPtr[i];
                /* Check there are no reserved ports in ports bitmap */
                if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsMembersPtr, portNum))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Reserved port[%d], can not be used as VLAN member!", portNum);
                }
            }
        }

        /* the ingress table looks the same on all port groups */

        /* check that the device supports all the physical ports that defined in the BMP */
        rc = prvCpssDxChTablePortsBmpCheck(devNum , portsMembersPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* build ingress VLAN entry */
        rc = sip5VlanHwEntryBuildIngress(devNum, vlanInfoPtr, &ingressHwData[0]);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* write ingress VLAN entry */
        rc = prvCpssDxChWriteTableEntry(devNum,
                                    CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                    vlanId,/* vid */
                                    &ingressHwData[0]);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {

            /* write TTI VLAN entry (vrf-Id field)*/
            rc = prvCpssDxChWriteTableEntry(devNum,
                            CPSS_DXCH_SIP6_TABLE_TTI_VLAN_TO_VRF_ID_TABLE_E,
                            vlanId,
                            &vlanInfoPtr->vrfId);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        PRV_CPSS_DXCH_FDB_LOOKUP_KEY_MODE_PER_FID_SET_MAC(
            devNum, vlanInfoPtr->fidValue, vlanInfoPtr->fdbLookupKeyMode);

        /* write ingress bridge port members entry */
        rc = prvCpssDxChWriteTableEntry(devNum,
                                    CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E,
                                    vlanId,/* vid */
                                    &portsMembersPtr->ports[0]);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* write ingress spanning state group index */
        rc = prvCpssDxChWriteTableEntry(devNum,
                                    CPSS_DXCH_SIP5_TABLE_INGRESS_SPAN_STATE_GROUP_INDEX_E,
                                    vlanId,/* vid */
                                    &vlanInfoPtr->stgId);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* build egress VLAN entry */
        rc = sip5VlanHwEntryBuildEgress(devNum,
                  &(portsMembersBmp.ports[0]),
                  &(portsTaggingPtr->ports[0]),
                  vlanInfoPtr,
                  &(portsTaggingCmdPtr->portsCmd[0]),
                  &egressHwData[0],
                  &egfShtVlanAttrib_hwDataArr[0],
                  &egfShtVlanSpanId_hwDataArr[0],
                  &efgQag_egressHwData[0]);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* the ingress table looks the same on all port groups */

        /* write entry to HW EGF_SHT egress table */
        rc = prvCpssDxChWriteTableEntry(devNum,
                                    CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
                                    vlanId,/* vid */
                                    &egressHwData[0]);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* write entry to HW EGF_SHT egress Vlan Attributes table */
        rc = prvCpssDxChWriteTableEntry(devNum,
                                    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_ATTRIBUTES_E,
                                    vlanId,/* vid */
                                    &egfShtVlanAttrib_hwDataArr[0]);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* write entry to HW EGF_SHT egress Vlan Span Tree Id table */
        rc = prvCpssDxChWriteTableEntry(devNum,
                                    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_SPANNING_E,
                                    vlanId,/* vid */
                                    &egfShtVlanSpanId_hwDataArr[0]);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* write entry to HW EGF_QAG egress table */
        rc = prvCpssDxChWriteTableEntry(devNum,
                                   CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E,
                                   vlanId,/* vid */
                                   &efgQag_egressHwData[0]);

        return rc;
    }
    else
    {

        /* Update port bitmap according to physical CPU port connection */
        rc = prvCpssDxChBrgCpuPortBmpConvert(devNum,
                                             GT_TRUE,
                                             &portsMembersBmp);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (GT_FALSE != PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                info_PRV_CPSS_DXCH_LION2_MC_BANDWIDTH_RESTRICTION_WA_E.enabled)
        {
            /* port 12 on each hemisphere is used to duplicate the packet to the other hemisphere;
               this port should be member of the VLAN only if there are additional members on the
               other hemisphere; note that port 12 is reversed logic (0: member, 1: not a member) */

            /* check if the VLAN contains members on the 1st hemisphere (other than port 12) */
            if (((portsMembersBmp.ports[0] & 0xffffefff) == 0) && (portsMembersBmp.ports[1] == 0))
            {
                /* internally set port 12 on 2nd hemisphere to indicate it is NOT a member */
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembersBmp,64+12);
            }
            else
            {
                /* internally clear port 12 on 2nd hemisphere to indicate it IS a member */
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&portsMembersBmp,64+12);
            }

            /* check if the VLAN contains members on the 2nd hemisphere (other than port 12) */
            if (((portsMembersBmp.ports[2] & 0xffffefff) == 0) && (portsMembersBmp.ports[3] == 0))
            {
                /* internally set port 12 on 1st hemisphere to indicate it is NOT a member */
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembersBmp,12);
            }
            else
            {
                /* internally clear port 12 on 1st hemisphere to indicate it IS a member */
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&portsMembersBmp,12);
            }
        }



        /* loop on all TXQ units */
        for (i =0; i < PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_MAC(devNum); i++)
        {
            /* build ingress VLAN entry */
            rc = prvCpssBrgVlanHwEntryBuildIngress(devNum,
                      &(portsMembersBmp.ports[i*PRV_CPSS_DXCH_PORT_WITH_CPU_BMP_NUM_IN_HEMISPHERE_CNS]),
                                            vlanInfoPtr,
                                            &ingressHwData[0]);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* write ingress VLAN entry to the ingress VLAN Table */
            rc = prvCpssDxChPortGroupWriteTableEntry(devNum,
                                i*PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        (GT_U32)vlanId,/* vid */
                                        &ingressHwData[0]);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* build egress VLAN entry */
            rc = prvCpssBrgVlanHwEntryBuildEgress(devNum,
                      &(portsMembersBmp.ports[i*PRV_CPSS_DXCH_PORT_WITH_CPU_BMP_NUM_IN_HEMISPHERE_CNS]),
                      &(portsTaggingPtr->ports[i*PRV_CPSS_DXCH_PORT_WITH_CPU_BMP_NUM_IN_HEMISPHERE_CNS]),
                                            vlanInfoPtr,
                      &(portsTaggingCmdPtr->portsCmd[i*PRV_CPSS_DXCH_PORT_WITH_CPU_NUM_IN_HEMISPHERE_CNS]),
                                            &egressHwData[0]);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* write egress VLAN entry to the egress VLAN Table */
            rc = prvCpssDxChPortGroupWriteTableEntry(devNum,
                                i*PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS,
                                        CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
                                        (GT_U32)vlanId,/* vid */
                                        &egressHwData[0]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        return GT_OK;
    }
}
/**
* @internal sip5VlanHwEntryParseIngress function
* @endinternal
*
* @brief   Bobcat2; Caelum; Aldrin; AC3X; Bobcat3 : parse vlan entry from buffer. - ingress vlan
*         NOTE: for multi-port-groups device , this function called per port group , to get the
*         per port group info
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
* @param[in] hwDataArr[VLAN_INGRESS_ENTRY_WORDS_NUM_CNS] - pointer to hw VLAN entry.
*
* @param[out] portsMembersPtr          - (pointer to) bmp of ports members in vlan
* @param[out] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[out] isValidPtr               - (pointer to) VLAN entry status
*                                      GT_TRUE = Vlan is valid
*                                      GT_FALSE = Vlan is not Valid
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr or portsTaggingPtr is
*                                       out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS sip5VlanHwEntryParseIngress
(
    IN  GT_U8                       devNum,
    IN GT_U16                       vlanId,
    IN GT_U32                       hwDataArr[VLAN_INGRESS_ENTRY_WORDS_NUM_CNS],
    OUT GT_U32                      *portsMembersPtr,
    OUT CPSS_DXCH_BRG_VLAN_INFO_STC *vlanInfoPtr,
    OUT GT_BOOL                     *isValidPtr
)
{
    GT_STATUS   rc;
    GT_U32      value;

    /* valid bit */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_VALID_E);
    *isValidPtr = BIT2BOOL_MAC(value);

    /* NewSrcAddrIsNotSecurityBreach - revert the bit */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_NEW_SRC_ADDR_IS_NOT_SECURITY_BREACH_E);
    vlanInfoPtr->unkSrcAddrSecBreach = BIT2BOOL_MAC(value - 1);

    /* UnregisteredNonIPMulticastCmd */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_NON_IP_MULTICAST_CMD_E);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unregNonIpMcastCmd,
                                              value)

    /* UnregisteredIpv4MulticastCmd */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV4_MULTICAST_CMD_E);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unregIpv4McastCmd,
                                              value)


    /* UnregisteredIpv6MulticastCmd */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV6_MULTICAST_CMD_E);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unregIpv6McastCmd,
                                              value)

    /* UnknownUnicastCmd */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNKNOWN_UNICAST_CMD_E);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unkUcastCmd,
                                              value)

    /* Ipv4IgmpToCpuEn */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IGMP_TO_CPU_EN_E);
    vlanInfoPtr->ipv4IgmpToCpuEn = BIT2BOOL_MAC(value);

    /* Mirror to ingress analyzer port*/
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_MIRROR_TO_INGRESS_ANALYZER_E);
    if(value == 0)
    {
        /* ignore index */
        vlanInfoPtr->mirrToRxAnalyzerEn = GT_FALSE;
    }
    else
    {
        vlanInfoPtr->mirrToRxAnalyzerEn = GT_TRUE;
        vlanInfoPtr->mirrToRxAnalyzerIndex = value - 1;
    }

    /* Ipv6IcmpToCpuEn*/
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_ICMP_TO_CPU_EN_E);
    vlanInfoPtr->ipv6IcmpToCpuEn = BIT2BOOL_MAC(value);

    /* parse the Ipv4ControlToCpuEn,
       (the Ipv6ControlToCpuEn will be read later , and the ipCtrlToCpuEn will
       be changed accordingly */

    /* Ipv4ControlToCpuEn*/
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_CONTROL_TO_CPU_EN_E);
    vlanInfoPtr->ipCtrlToCpuEn = (value == 0x1) ?
                                    CPSS_DXCH_BRG_IP_CTRL_IPV4_E :
                                    CPSS_DXCH_BRG_IP_CTRL_NONE_E;

    /* Ipv4IpmBridgingEn */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IPM_BRIDGING_EN_E);
    vlanInfoPtr->ipv4IpmBrgEn = BIT2BOOL_MAC(value);

    /* Ipv6IpmBridgingEn */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_IPM_BRIDGING_EN_E);
    vlanInfoPtr->ipv6IpmBrgEn = BIT2BOOL_MAC(value);

    /* Ipv4IpmBridgingMode */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IPM_BRIDGING_MODE_E);
    vlanInfoPtr->ipv4IpmBrgMode = (value == 0x0) ? CPSS_BRG_IPM_SGV_E :
                                                 CPSS_BRG_IPM_GV_E;

    /* Ipv6IpmBridgingMode */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_IPM_BRIDGING_MODE_E);
    vlanInfoPtr->ipv6IpmBrgMode = (value == 0x0) ? CPSS_BRG_IPM_SGV_E :
                                                 CPSS_BRG_IPM_GV_E;

    /* UnregisteredIpv4BcCmd */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV4_BC_CMD_E);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unregIpv4BcastCmd,
                                              value)

    /* UnregisteredNonIpv4BcCmd */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_NON_IPV4_BC_CMD_E);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unregNonIpv4BcastCmd,
                                              value)

    /* Ipv4UnicastRouteEn */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_UNICAST_ROUTE_EN_E);
    vlanInfoPtr->ipv4UcastRouteEn = BIT2BOOL_MAC(value);

    /* Ipv4MulticastRouteEn */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_MULTICAST_ROUTE_EN_E);
    vlanInfoPtr->ipv4McastRouteEn = BIT2BOOL_MAC(value);

    /* Ipv6UnicastRouteEn */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_UNICAST_ROUTE_EN_E);
    vlanInfoPtr->ipv6UcastRouteEn = BIT2BOOL_MAC(value);

    /* Ipv6MulticastRouteEn */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_MULTICAST_ROUTE_EN_E);
    vlanInfoPtr->ipv6McastRouteEn = BIT2BOOL_MAC(value);

    /* Ipv6SiteId */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_SITEID_E);
    vlanInfoPtr->ipv6SiteIdMode = (value == 0x0) ?
        CPSS_IP_SITE_ID_INTERNAL_E :
        CPSS_IP_SITE_ID_EXTERNAL_E;

    /* AutoLearnDis */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_AUTO_LEARN_DIS_E);
    vlanInfoPtr->autoLearnDisable = BIT2BOOL_MAC(value);

    /* NaMsgToCpuEn */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_NA_MSG_TO_CPU_EN_E);
    vlanInfoPtr->naMsgToCpuEn = BIT2BOOL_MAC(value);

    /* MRU Index */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_MRU_INDEX_E);
    vlanInfoPtr->mruIdx = value;

    /* BcUdpTrapMirrorEn */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_BC_UDP_TRAP_MIRROR_EN_E);
    vlanInfoPtr->bcastUdpTrapMirrEn = BIT2BOOL_MAC(value);

    /* Ipv6ControlToCpuEn */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_CONTROL_TO_CPU_EN_E);

    if(vlanInfoPtr->ipCtrlToCpuEn == CPSS_DXCH_BRG_IP_CTRL_NONE_E)
    {
        vlanInfoPtr->ipCtrlToCpuEn = (value == 0x1) ?
                                    CPSS_DXCH_BRG_IP_CTRL_IPV6_E :
                                    CPSS_DXCH_BRG_IP_CTRL_NONE_E;
    }
    else /*CPSS_DXCH_BRG_IP_CTRL_IPV4_E*/
    {
        vlanInfoPtr->ipCtrlToCpuEn = (value == 0x1) ?
                                    CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E :
                                    CPSS_DXCH_BRG_IP_CTRL_IPV4_E;
    }

    /* Vidx */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FLOOD_EVIDX_E);
    vlanInfoPtr->floodVidx = (GT_U16)value;

    /* VrfId */
    if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_VRF_ID_E);
        vlanInfoPtr->vrfId = value;
    }

    /* UcLocalEn */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UC_LOCAL_EN_E);
    vlanInfoPtr->ucastLocalSwitchingEn = BIT2BOOL_MAC(value);

    /* FloodVidxMode */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FLOOD_VIDX_MODE_E);
    vlanInfoPtr->floodVidxMode = (value == 0) ? CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E
                                              : CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;

    /* read entry from HW ingress Span State Group Index table */
    rc = prvCpssDxChReadTableEntry(devNum,
                               CPSS_DXCH_SIP5_TABLE_INGRESS_SPAN_STATE_GROUP_INDEX_E,
                               vlanId,/* vid */
                               &vlanInfoPtr->stgId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* fid value */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FID_E);
    vlanInfoPtr->fidValue = value;

    /* unknown Mac Sa Command  */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNKOWN_MAC_SA_CMD_E);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unknownMacSaCmd, value);

    /* IPv4 MC/BC Mirror to Analyzer Index */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_MC_BC_TO_MIRROR_ANLYZER_IDX_E);
    if(value == 0)
    {
        /* ignore index */
        vlanInfoPtr->ipv4McBcMirrToAnalyzerEn = GT_FALSE;
    }
    else
    {
        vlanInfoPtr->ipv4McBcMirrToAnalyzerEn = GT_TRUE;
        vlanInfoPtr->ipv4McBcMirrToAnalyzerIndex = value - 1;
    }

    /* IPv6 MC Mirror to Analyzer Index */
    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_MC_TO_MIRROR_ANALYZER_IDX_E);
    if(value == 0)
    {
        /* ignore index */
        vlanInfoPtr->ipv6McMirrToAnalyzerEn = GT_FALSE;
    }
    else
    {
        vlanInfoPtr->ipv6McMirrToAnalyzerEn = GT_TRUE;
        vlanInfoPtr->ipv6McMirrToAnalyzerIndex = value - 1;
    }


    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(
        devNum,value, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FCOE_FORWARDING_EN_E);
    vlanInfoPtr->fcoeForwardingEn = BIT2BOOL_MAC(value);

    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(
        devNum,value, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREG_IPM_EVIDX_MODE_E);
    PRV_CPSS_DXCH_CONVERT_HW_VAL_TO_UNREG_IPM_EVIDX_MODE_MAC(
        value, vlanInfoPtr->unregIpmEVidxMode);

    GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(
        devNum,value, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREG_IPM_EVIDX_E);
    vlanInfoPtr->unregIpmEVidx = value;

    /* FDB Lookup Key mode */
    if (PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        GET_FIELD_IN_INGRESS_VLAN_ENTRY_MAC(devNum,value,
                     SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FDB_LOOKUP_KEY_MODE_E);
        PRV_CPSS_DXCH_CONVERT_HW_VAL_TO_FDB_LOOKUP_KEY_MODE_MAC(value,
                                                 vlanInfoPtr->fdbLookupKeyMode);
    }
    else
    {
        vlanInfoPtr->fdbLookupKeyMode = CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_E;
    }

    /* read entry from HW ingress port members table */
    rc = prvCpssDxChReadTableEntry(devNum,
                               CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E,
                               vlanId,/* vid */
                               portsMembersPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssBrgVlanHwEntryParseIngress function
* @endinternal
*
* @brief   parse vlan entry from buffer. - ingress vlan
*         NOTE: for multi-port-groups device , this function called per port group , to get the
*         per port group info
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] hwDataArr[VLAN_INGRESS_ENTRY_WORDS_NUM_CNS] - pointer to hw VLAN entry.
*
* @param[out] portsMembersPtr          - (pointer to) bmp of ports members in vlan
* @param[out] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[out] isValidPtr               - (pointer to) VLAN entry status
*                                      GT_TRUE = Vlan is valid
*                                      GT_FALSE = Vlan is not Valid
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr or portsTaggingPtr is
*                                       out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS prvCpssBrgVlanHwEntryParseIngress
(
    IN GT_U32                       hwDataArr[VLAN_INGRESS_ENTRY_WORDS_NUM_CNS],
    OUT GT_U32                      *portsMembersPtr,
    OUT CPSS_DXCH_BRG_VLAN_INFO_STC *vlanInfoPtr,
    OUT GT_BOOL                     *isValidPtr
)
{
    GT_U32      value;

    /* bit 0 - valid bit */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,0,1,value);
    *isValidPtr = BIT2BOOL_MAC(value);

    /* bit 1 - NewSrcAddrIsNotSecurityBreach - revert the bit */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,1,1,value);
    vlanInfoPtr->unkSrcAddrSecBreach = BIT2BOOL_MAC(value - 1);

    /* bit 2..65 port members */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr, 2,32,portsMembersPtr[0]);
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,34,32,portsMembersPtr[1]);

    /* bits 66..68 - UnregisteredNonIPMulticastCmd */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,66,3,value);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unregNonIpMcastCmd,
                                              value)

    /* bits 69..71 - UnregisteredIpv4MulticastCmd */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,69,3,value);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unregIpv4McastCmd,
                                              value)


    /* bits 72..74 - UnregisteredIpv6MulticastCmd */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,72,3,value);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unregIpv6McastCmd,
                                              value)

    /* bits 75..77 - UnknownUnicastCmd */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,75,3,value);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unkUcastCmd,
                                              value)

    /* bit 78 - Ipv4IgmpToCpuEn */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,78,1,value);
    vlanInfoPtr->ipv4IgmpToCpuEn = BIT2BOOL_MAC(value);

   /* bit 81 - mirrToRxAnalyzerEn */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,81,1,value);
    vlanInfoPtr->mirrToRxAnalyzerEn = BIT2BOOL_MAC(value);

    /* bit 82 - Ipv6IcmpToCpuEn*/
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,82,1,value);
    vlanInfoPtr->ipv6IcmpToCpuEn = BIT2BOOL_MAC(value);

    /* parse the Ipv4ControlToCpuEn,
       (the Ipv6ControlToCpuEn will be read later , and the ipCtrlToCpuEn will
       be changed accordingly */

    /* bit 83 - Ipv4ControlToCpuEn*/
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,83,1,value);
    vlanInfoPtr->ipCtrlToCpuEn = (value == 0x1) ?
                                    CPSS_DXCH_BRG_IP_CTRL_IPV4_E :
                                    CPSS_DXCH_BRG_IP_CTRL_NONE_E;

    /* bit 84 - Ipv4IpmBridgingEn */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,84,1,value);
    vlanInfoPtr->ipv4IpmBrgEn = BIT2BOOL_MAC(value);

    /* bit 85 - Ipv6IpmBridgingEn */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,85,1,value);
    vlanInfoPtr->ipv6IpmBrgEn = BIT2BOOL_MAC(value);

    /* bit 79 - Ipv4IpmBridgingMode */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,79,1,value);
    vlanInfoPtr->ipv4IpmBrgMode = (value == 0x0) ? CPSS_BRG_IPM_SGV_E :
                                                 CPSS_BRG_IPM_GV_E;

    /* bit 80 - Ipv6IpmBridgingMode */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,80,1,value);
    vlanInfoPtr->ipv6IpmBrgMode = (value == 0x0) ? CPSS_BRG_IPM_SGV_E :
                                                 CPSS_BRG_IPM_GV_E;

    /* bits 86..88 - UnregisteredIpv4BcCmd */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,86,3,value);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unregIpv4BcastCmd,
                                              value)

    /* bits 89..91 - UnregisteredNonIpv4BcCmd */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,89,3,value);
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(vlanInfoPtr->unregNonIpv4BcastCmd,
                                              value)

    /* bit 92 - Ipv4UnicastRouteEn */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,92,1,value);
    vlanInfoPtr->ipv4UcastRouteEn = BIT2BOOL_MAC(value);

    /* bit 93 - Ipv4MulticastRouteEn */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,93,1,value);
    vlanInfoPtr->ipv4McastRouteEn = BIT2BOOL_MAC(value);

    /* bit 94 - Ipv6UnicastRouteEn */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,94,1,value);
    vlanInfoPtr->ipv6UcastRouteEn = BIT2BOOL_MAC(value);

    /* bit 95 - Ipv6MulticastRouteEn */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,95,1,value);
    vlanInfoPtr->ipv6McastRouteEn = BIT2BOOL_MAC(value);

    /* bit 96 - Ipv6SiteId */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,96,1,value);
    vlanInfoPtr->ipv6SiteIdMode = (value == 0x0) ?
        CPSS_IP_SITE_ID_INTERNAL_E :
        CPSS_IP_SITE_ID_EXTERNAL_E;

    /* bit 97 - AutoLearnDis */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,97,1,value);
    vlanInfoPtr->autoLearnDisable = BIT2BOOL_MAC(value);

    /* bit 98 - NaMsgToCpuEn */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,98,1,value);
    vlanInfoPtr->naMsgToCpuEn = BIT2BOOL_MAC(value);

    /* bits 99..101 MRU Index */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,99,3,value);
    vlanInfoPtr->mruIdx = value;

    /* bit 102 - BcUdpTrapMirrorEn */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,102,1,value);
    vlanInfoPtr->bcastUdpTrapMirrEn = BIT2BOOL_MAC(value);

    /* bit 103 - Ipv6ControlToCpuEn */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,103,1,value);

    if(vlanInfoPtr->ipCtrlToCpuEn == CPSS_DXCH_BRG_IP_CTRL_NONE_E)
    {
        vlanInfoPtr->ipCtrlToCpuEn = (value == 0x1) ?
                                    CPSS_DXCH_BRG_IP_CTRL_IPV6_E :
                                    CPSS_DXCH_BRG_IP_CTRL_NONE_E;
    }
    else /*CPSS_DXCH_BRG_IP_CTRL_IPV4_E*/
    {
        vlanInfoPtr->ipCtrlToCpuEn = (value == 0x1) ?
                                    CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E :
                                    CPSS_DXCH_BRG_IP_CTRL_IPV4_E;
    }

    /* bit 104..115 - Vidx */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,104,12,value);
    vlanInfoPtr->floodVidx = (GT_U16)value;

    /* bit 116..127 - VrfId */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,116,12,value);
    vlanInfoPtr->vrfId = value;

    /* bit 128 - UcLocalEn */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,128,1,value);
    vlanInfoPtr->ucastLocalSwitchingEn = BIT2BOOL_MAC(value);

    /* bit 129 - FloodVidxMode */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,129,1,value);
    vlanInfoPtr->floodVidxMode = (value == 0) ? CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E
                                              : CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;

    /* bit 130..137 - SpanStateGroupIndex */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,130,8,value);

    vlanInfoPtr->stgId = value;

    return GT_OK;
}

/**
* @internal prvCpssBrgVlanHwEntryParseEgress function
* @endinternal
*
* @brief   parse vlan entry from buffer. -- egress vlan
*         NOTE: for multi-port-groups device , this function called per port group , to get the
*         per port group info
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] hwDataArr[VLAN_EGRESS_ENTRY_WORDS_NUM_CNS] - pointer to hw VLAN entry.
* @param[in] isValidPtr               - (pointer to) INGRESS VLAN entry status
* @param[in] portsMembersPtr          - (pointer to) bmp of ports members in INGRESS vlan
* @param[in,out] vlanInfoPtr              - (pointer to) VLAN specific information -
*                                      field of : PortIsolationVlanCmd is taken from stgId
*
* @param[out] portsTaggingPtr          - (pointer to) bmp of ports tagged in the vlan -
*                                      the relevant parameter for DxCh3 and beyond and
*                                      not relevant for TR101 supported devices.
* @param[in,out] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[out] isValidPtr               - (pointer to) egress VLAN entry status
*                                      GT_TRUE = Vlan is valid
*                                      GT_FALSE = Vlan is not Valid
* @param[out] portsTaggingCmdPtr       - (pointer to) ports tagging commands in the vlan -
*                                      the relevant parameter for xCat3 and above
*                                      with TR101 feature support.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr or portsTaggingPtr is
*                                       out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - the ingress and egress vlan hold 'same fields' but with
*                                       different values : valid,members,stgId
*/
static GT_STATUS prvCpssBrgVlanHwEntryParseEgress
(
    IN GT_U8                        devNum,
    IN GT_U32                       hwDataArr[VLAN_EGRESS_ENTRY_WORDS_NUM_CNS],
    IN GT_U32                       *portsMembersPtr,
    OUT GT_U32                      *portsTaggingPtr,
    INOUT CPSS_DXCH_BRG_VLAN_INFO_STC *vlanInfoPtr,
    IN GT_BOOL                      *isValidPtr,
    OUT CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT  *portsTaggingCmdPtr
)
{
    GT_U32      value;          /* value in HW format */
    GT_U32      value1;          /* value in HW format */
    GT_U32      ii;             /* iterator */
    GT_U32      tagCmdMaxPorts = 63;  /* 63 ports with tagCmd -- NOT 64 !!! */
    GT_U32      startBit;/* start bit in entry */
    GT_BOOL     tr101Supported;   /* TR101 Feature Support */

    /* get TR101 feature support */
    tr101Supported = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.tr101Supported;

    /* bit 0 - valid bit */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,0,1,value);
    if(value != (GT_U32)(BOOL2BIT_MAC(*isValidPtr)))
    {
        /* the ingress vlan and the egress vlan not synchronized ?! */
        /* we have not API to allow this situation */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* bit 1..64 port members */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr, 1,32,value);
    if(portsMembersPtr[0] != value)
    {
        /* the ingress vlan and the egress vlan not synchronized ?! */
        /* we have not API to allow this situation */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,33,32,value);
    if(portsMembersPtr[1] != value)
    {
        /* the ingress vlan and the egress vlan not synchronized ?! */
        /* we have not API to allow this situation */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    startBit = 65;
    /* tagCmd info for all ports */
    for (ii = 0; ii < tagCmdMaxPorts; ii++ , startBit += 3)
    {
        /* 65..253 - tagCmd of ports */
        U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,startBit, 3 ,value);

        if (tr101Supported == GT_FALSE)
        {
            if(value != 0)
            {
                portsTaggingPtr[(ii)>>5] |= 1 << ((ii)& 0x1f);
            }
        }
        else
        {
            PRV_CPSS_DXCH_CONVERT_HW_VAL_TO_VLAN_TAG_MODE_MAC(devNum, value,
                    portsTaggingCmdPtr[ii]);
        }
    }

    /* bit 254..261 - SpanStateGroupIndex */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,254,8,value);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* bit 265..268 - extra 4 bits from SpanStateGroupIndex */
        U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,265,4,value1);
        value |= (value1 << 8);
    }

    if(value != vlanInfoPtr->stgId)
    {
        /* the ingress vlan and the egress vlan not synchronized ?! */
        /* we have not API to allow this situation */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* bit 262 - LocalSwithingEn*/
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,262,1,value);
    vlanInfoPtr->mcastLocalSwitchingEn = BIT2BOOL_MAC (value);

    /* bits 263..264 - PortIsolationVlanCmd */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,263,2,value);
    switch (value)
    {
        case 0:
            vlanInfoPtr->portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
            break;
        case 1:
            vlanInfoPtr->portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_CMD_E;
            break;
        case 2:
            vlanInfoPtr->portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L3_CMD_E;
            break;
        default:
            vlanInfoPtr->portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_L3_CMD_E;
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* Mirror to egress analyzer port-new fiels bits [269:271] */

        U32_GET_FIELD_IN_ENTRY_MAC(hwDataArr,269,3,value);

        if(value == 0)
        {
            /* ignore index */
            vlanInfoPtr->mirrToTxAnalyzerEn = GT_FALSE;
        }
        else
        {
            vlanInfoPtr->mirrToTxAnalyzerEn = GT_TRUE;

             U32_SET_FIELD_IN_ENTRY_MAC(&vlanInfoPtr->mirrToTxAnalyzerIndex,0,3,value);

             vlanInfoPtr->mirrToTxAnalyzerIndex -= 1;
        }
    }

    return GT_OK;
}


/**
* @internal sip5VlanHwEntryParseEgress function
* @endinternal
*
* @brief   parse vlan entry from buffer. -- egress vlan - Bobcat2; Caelum; Aldrin; AC3X; Bobcat3
*         NOTE: all port groups hold the same info
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] egfSht_hwDataArr[VLAN_EGRESS_ENTRY_WORDS_NUM_CNS] - pointer to hw VLAN entry - EGF_SHT entry
* @param[in] egfShtVlanAttrib_hwDataArr[1] - pointer to hw VLAN Attributes entry
* @param[in] egfShtVlanSpanId_hwDataArr[1] - pointer to hw VLAN SpanTree Id entry
* @param[in] egfQag_hwDataArr[EGF_QAG_EGRESS_VLAN_ENTRY_WORDS_NUM_CNS] - pointer to hw VLAN entry - EGF_QAG entry
* @param[in] isValidPtr               - (pointer to) INGRESS VLAN entry status
* @param[in,out] vlanInfoPtr              - (pointer to) VLAN specific information -
*                                      field of : stgId is taken from INGRESS vlan
*
* @param[out] portsMembersPtr          - (pointer to) bmp of ports members in INGRESS vlan
* @param[out] portsTaggingPtr          - (pointer to) bmp of ports tagged in the vlan -
*                                      not relevant for TR101 supported devices.
* @param[in,out] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[out] isValidPtr               - (pointer to) egress VLAN entry status
*                                      GT_TRUE = Vlan is valid
*                                      GT_FALSE = Vlan is not Valid
* @param[out] portsTaggingCmdPtr       - (pointer to) ports tagging commands in the vlan -
*                                      the relevant parameter for xCat3 and above
*                                      with TR101 feature support.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr or portsTaggingPtr is
*                                       out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - the ingress and egress vlan hold 'same fields' but with
*                                       different values : valid,members,stgId
*/
static GT_STATUS sip5VlanHwEntryParseEgress
(
    IN GT_U8                        devNum,
    IN GT_U32                       egfSht_hwDataArr[VLAN_EGRESS_ENTRY_WORDS_NUM_CNS],
    IN GT_U32                       egfShtVlanAttrib_hwDataArr[1],
    IN GT_U32                       egfShtVlanSpanId_hwDataArr[1],
    IN GT_U32                       egfQag_hwDataArr[EGF_QAG_EGRESS_VLAN_ENTRY_WORDS_NUM_CNS],
    OUT CPSS_PORTS_BMP_STC          *portsMembersPtr,
    OUT CPSS_PORTS_BMP_STC          *portsTaggingPtr,
    INOUT CPSS_DXCH_BRG_VLAN_INFO_STC *vlanInfoPtr,
    IN GT_BOOL                      *isValidPtr,
    OUT CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC *portsTaggingCmdPtr
)
{
    GT_STATUS   rc;
    GT_U32      ii;                     /* iterator */
    GT_U32      value;                  /* value in HW format */
    GT_U32      hwMirrorIndex;          /* Mirror Index in HW format      */
    GT_U32      hwPortIsolationMode;    /* PortIsolationMode in HW format */
    GT_BOOL     tr101Supported;         /* TR101 Feature Support */
    CPSS_PORTS_BMP_STC  portsBmpMembers;/*ports members in vlan*/
    CPSS_PORTS_BMP_STC  adjusted_portsBmpMembers;/* adjusted ports bmp of only existing ports */
    CPSS_PORTS_BMP_STC  adjusted_IN_portsBmpMembers;/* adjusted ports bmp of only existing ports */
    CPSS_PORTS_BMP_STC  existingPorts;
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egress_mirroring_mode;

    /* get TR101 feature support */
    tr101Supported = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.tr101Supported;

/**************** EGF_SHT section *********************************************/

    /* valid bit */
    if (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        GET_FIELD_IN_EGF_SHT_EGRESS_VLAN_ENTRY_MAC(devNum,value,SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS_VALID_E);
        *isValidPtr = BIT2BOOL_MAC(value);

        if(value != (GT_U32)(BOOL2BIT_MAC(*isValidPtr)))
        {
            /* the ingress vlan and the egress vlan not synchronized ?! */
            /* we have not API to allow this situation */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "the ingress vlan valid bit and the egress vlan valid bit not synchronized ?!");
        }
    }

    /* port members */
    U_ANY_GET_FIELD_BY_ID_MAC(egfSht_hwDataArr,
        PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_EGRESS_EVLAN_E].fieldsInfoPtr,
        SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS_PORT_X_MEMBER_E,
        &portsBmpMembers.ports[0]);/* pointer to the array of 256 ports */

    /* adjusted physical ports of BMP to hold only bmp of existing ports*/
    rc = prvCpssDxChTablePortsBmpAdjustToDevice(devNum , &portsBmpMembers , &adjusted_portsBmpMembers);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* adjusted physical ports of BMP to hold only bmp of existing ports*/
    rc = prvCpssDxChTablePortsBmpAdjustToDevice(devNum , portsMembersPtr , &adjusted_IN_portsBmpMembers);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(0 == PRV_CPSS_PORTS_BMP_ARE_EQUAL_MAC(&adjusted_portsBmpMembers,&adjusted_IN_portsBmpMembers))
    {
        /* the ingress vlan and the egress vlan not synchronized ?! */
        /* we have no API to allow this situation */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "the ingress vlan ports members and the egress vlan portt members not synchronized ?!");
    }

    /* copy value into the OUT parameter */
    *portsMembersPtr = adjusted_IN_portsBmpMembers;

    /* SpanStateGroupIndex */
    if(egfShtVlanSpanId_hwDataArr[0] != (GT_U32)vlanInfoPtr->stgId)
    {
        /* the ingress STG in vlan and the egress STG in vlan not synchronized ?! */
        /* we have not API to allow this situation */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "the ingress STG[%d] in vlan and the egress STG[%d] in vlan not synchronized",
            vlanInfoPtr->stgId,egfShtVlanSpanId_hwDataArr[0]);
    }

    /* LocalSwithingEn*/
    U32_GET_FIELD_BY_ID_MAC(egfShtVlanAttrib_hwDataArr,
        PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_VLAN_ATTRIBUTES_E].fieldsInfoPtr,
        SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_LOCAL_SWITHING_EN_E,
        value);
    vlanInfoPtr->mcastLocalSwitchingEn = BIT2BOOL_MAC(value);

    /* PortIsolationVlanCmd */
    U32_GET_FIELD_BY_ID_MAC(egfShtVlanAttrib_hwDataArr,
        PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_VLAN_ATTRIBUTES_E].fieldsInfoPtr,
        SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_PORT_ISOLATION_VLAN_CMD_E,
        hwPortIsolationMode);

    /* Mirror to egress analyzer port */
    U32_GET_FIELD_BY_ID_MAC(egfShtVlanAttrib_hwDataArr,
        PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_VLAN_ATTRIBUTES_E].fieldsInfoPtr,
        SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_EGRESS_MIRROR_TO_ANALYZER_INDEX_E,
        hwMirrorIndex);
    switch (hwPortIsolationMode)
    {
        case 0:
            vlanInfoPtr->portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
            break;
        case 1:
            vlanInfoPtr->portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_CMD_E;
            break;
        case 2:
            vlanInfoPtr->portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L3_CMD_E;
            break;
        default:
            vlanInfoPtr->portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_L2_L3_CMD_E;
    }

    if(hwMirrorIndex == 0)
    {
        /* ignore index */
        vlanInfoPtr->mirrToTxAnalyzerEn = GT_FALSE;
    }
    else
    {
        vlanInfoPtr->mirrToTxAnalyzerEn = GT_TRUE;
        vlanInfoPtr->mirrToTxAnalyzerIndex = hwMirrorIndex - 1;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* Mirror to egress mode */
        U32_GET_FIELD_BY_ID_MAC(egfShtVlanAttrib_hwDataArr,
            PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_VLAN_ATTRIBUTES_E].fieldsInfoPtr,
            SIP6_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E,
            value);
        PRV_CPSS_DXCH_MIRROR_CONVERT_EGRESS_MIRROR_MODE_TO_SW_VAL_MAC(value, egress_mirroring_mode);
        vlanInfoPtr->mirrToTxAnalyzerMode = egress_mirroring_mode;
    }
/**************** EGF_QAG section *********************************************/
    /* get the bmp of supported port on this device */
    prvCpssDrvPortsFullMaskGet(
        PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum),
            &existingPorts);

    /* tagCmd info for all ports */
    for (ii = 0; ii < CPSS_MAX_PORTS_NUM_CNS; ii++)
    {
        if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&existingPorts,ii))
        {
            /* make sure to 'ignores' non exists ports */
            continue;
        }

        U32_GET_FIELD_IN_ENTRY_MAC(egfQag_hwDataArr,(ii*3), 3 ,value);

        if (tr101Supported == GT_FALSE)
        {
            if(value)
            {
                CPSS_PORTS_BMP_PORT_SET_MAC(portsTaggingPtr,ii);
            }
        }
        else
        {
            PRV_CPSS_DXCH_CONVERT_HW_VAL_TO_VLAN_TAG_MODE_MAC(devNum, value,
                    portsTaggingCmdPtr->portsCmd[ii]);
        }
    }

    return GT_OK;
}



/**
* @internal vlanEntrySplitTableRead function
* @endinternal
*
* @brief   Read vlan entry. - for split table (ingress,egress)
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN Id
*
* @param[out] portsMembersPtr          - (pointer to) bmp of ports members in vlan
* @param[out] portsTaggingPtr          - (pointer to) bmp of ports tagged in the vlan -
*                                      The parameter is relevant for xCat3 and
*                                      above devices without TR101 feature support
*                                      The parameter is not relevant and ignored for
*                                      xCat3 and above devices with TR101 feature
*                                      support.
* @param[out] vlanInfoPtr              - (pointer to) VLAN specific information
* @param[out] isValidPtr               - (pointer to) VLAN entry status
*                                      GT_TRUE = Vlan is valid
*                                      GT_FALSE = Vlan is not Valid
* @param[out] portsTaggingCmdPtr       - (pointer to) ports tagging commands in the vlan -
*                                      The parameter is relevant only for xCat3 and
*                                      above with TR101 feature support.
*                                      The parameter is not relevant and ignored for
*                                      other devices.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS vlanEntrySplitTableRead
(
    IN  GT_U8                                devNum,
    IN  GT_U16                               vlanId,
    OUT CPSS_PORTS_BMP_STC                   *portsMembersPtr,
    OUT CPSS_PORTS_BMP_STC                   *portsTaggingPtr,
    OUT CPSS_DXCH_BRG_VLAN_INFO_STC          *vlanInfoPtr,
    OUT GT_BOOL                              *isValidPtr,
    OUT CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC *portsTaggingCmdPtr
)
{
    GT_STATUS   rc;
    /* ingress HW format */
    GT_U32      ingressHwData[VLAN_INGRESS_ENTRY_WORDS_NUM_CNS];
    /* egress HW format */
    GT_U32      egressHwData[VLAN_EGRESS_ENTRY_WORDS_NUM_CNS];
    GT_U32      efgQag_egressHwData[EGF_QAG_EGRESS_VLAN_ENTRY_WORDS_NUM_CNS];/*Bobcat2: EGF_QAG info*/
    GT_BOOL     tr101Supported;   /* TR101 Feature Support */
    GT_U32      i;                /* iterator */
    GT_U32      egfShtVlanAttrib_hwDataArr[1]; /* SHT Egress Vlan Attribute table HW entry   */
    GT_U32      egfShtVlanSpanId_hwDataArr[1]; /* SHT Egress Vlan Span Tree Id table HW entry*/

    /* get TR101 feature support */
    tr101Supported = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.tr101Supported;

    cpssOsMemSet(vlanInfoPtr,0,sizeof(*vlanInfoPtr));
    cpssOsMemSet(portsMembersPtr,0,sizeof(*portsMembersPtr));

    /* get TR101 feature support */
    if (tr101Supported != GT_FALSE)
    {
        cpssOsMemSet(portsTaggingCmdPtr,0,sizeof(*portsTaggingCmdPtr));
    }
    else
    {
        cpssOsMemSet(portsTaggingPtr,0,sizeof(*portsTaggingPtr));
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* the ingress table looks the same on all port groups */

        /* read entry from HW ingress table */
        rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                   vlanId,/* vid */
                                   &ingressHwData[0]);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* convert HW format to SW format */
        rc = sip5VlanHwEntryParseIngress(
                devNum,
                vlanId,
                &ingressHwData[0],
                &(portsMembersPtr->ports[0]),
                vlanInfoPtr,
                isValidPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* read TTI VLAN entry (vrf-Id field)*/
            rc = prvCpssDxChReadTableEntry(devNum,
                        CPSS_DXCH_SIP6_TABLE_TTI_VLAN_TO_VRF_ID_TABLE_E,
                        vlanId,
                        &vlanInfoPtr->vrfId);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        /* read entry from HW EGF_SHT egress table */
        rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
                                   vlanId,/* vid */
                                   &egressHwData[0]);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* read entry from HW EGF_SHT egress Vlan Attributes table */
        rc = prvCpssDxChReadTableEntry(
            devNum,
            CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_ATTRIBUTES_E,
            vlanId,/* vid */
            &egfShtVlanAttrib_hwDataArr[0]);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* read entry from HW EGF_SHT egress Vlan Span Tree Id table */
        rc = prvCpssDxChReadTableEntry(
            devNum,
            CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_SPANNING_E,
            vlanId,/* vid */
            &egfShtVlanSpanId_hwDataArr[0]);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* read entry from HW EGF_QAG egress table */
        rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E,
                                   vlanId,/* vid */
                                   &efgQag_egressHwData[0]);
        if(rc != GT_OK)
        {
            return rc;
        }


        /* convert HW format to SW format */
        rc = sip5VlanHwEntryParseEgress(devNum,
                  &egressHwData[0],         /*EGF_SHT info*/
                  &egfShtVlanAttrib_hwDataArr[0],
                  &egfShtVlanSpanId_hwDataArr[0],
                  &efgQag_egressHwData[0],  /*EGF_QAG info*/
                  portsMembersPtr,
                  portsTaggingPtr,
                  vlanInfoPtr,
                  isValidPtr,
                  portsTaggingCmdPtr);

        return rc;
    }
    else
    {
        /* loop on all TXQ units */
        for (i = 0; i < PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_MAC(devNum); i++)
        {
            if(GT_FALSE == PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
            {
                /* read entry from HW ingress table */
                rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                                    i*PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS,
                                           CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                           (GT_U32)vlanId,/* vid */
                                           &ingressHwData[0]);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* convert HW format to SW format */
                rc = prvCpssBrgVlanHwEntryParseIngress(
                          &ingressHwData[0],
                          &(portsMembersPtr->ports[i*PRV_CPSS_DXCH_PORT_WITH_CPU_BMP_NUM_IN_HEMISPHERE_CNS]),
                          vlanInfoPtr,
                          isValidPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

            /* read entry from HW egress table */
            rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                                i*PRV_CPSS_DXCH_PORT_GROUPS_NUM_IN_HEMISPHERE_CNS,
                                       CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
                                       (GT_U32)vlanId,/* vid */
                                       &egressHwData[0]);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* convert HW format to SW format */
            rc = prvCpssBrgVlanHwEntryParseEgress(devNum,
                      &egressHwData[0],
                      &(portsMembersPtr->ports[i*PRV_CPSS_DXCH_PORT_WITH_CPU_BMP_NUM_IN_HEMISPHERE_CNS]),
                      &(portsTaggingPtr->ports[i*PRV_CPSS_DXCH_PORT_WITH_CPU_BMP_NUM_IN_HEMISPHERE_CNS]),
                      vlanInfoPtr,
                      isValidPtr,
                      &(portsTaggingCmdPtr->portsCmd[i*PRV_CPSS_DXCH_PORT_WITH_CPU_NUM_IN_HEMISPHERE_CNS]));

            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if (GT_FALSE != PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                info_PRV_CPSS_DXCH_LION2_MC_BANDWIDTH_RESTRICTION_WA_E.enabled)
        {
            /* port 12 on each hemisphere is used to duplicate the packet to the other hemisphere;
               this port is set internally so it should be cleared upon reading the entry */
            CPSS_PORTS_BMP_PORT_CLEAR_MAC(portsMembersPtr,12);      /* 1st hemisphere -> port 12 */
            CPSS_PORTS_BMP_PORT_CLEAR_MAC(portsMembersPtr,(64+12)); /* 2nd hemisphere -> port 76 */
        }

        return GT_OK;
    }
}


/**
* @internal internal_cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet function
* @endinternal
*
* @brief   Set Tag1 removal mode from the egress port Tag State if Tag1 VID
*         is assigned a value of 0.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Vlan Tag1 Removal  when Tag1 VID=0
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT     mode
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value   */
    GT_U32  regValEArch;/* register for Bobcat2, Caelum, Bobcat3 */
    GT_U32  regOffset;  /* offset in register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    switch (mode)
    {
        case CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E:
            regValue = 0;
            regValEArch = 0;
            break;
        case CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_AND_INGRESS_WITHOUT_TAG1_E:
        case CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_NOT_DOUBLE_TAG_E:
            /* NOTE: as the functionality was slightly changed in e_ARCH devices
               the enum of  CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_AND_INGRESS_WITHOUT_TAG1_E
               should be used for those devices instead of
               CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_NOT_DOUBLE_TAG_E */
            if((PRV_CPSS_SIP_5_CHECK_MAC(devNum) != GT_TRUE) &&
                (mode == CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_AND_INGRESS_WITHOUT_TAG1_E))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            else
            if ((PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE) &&
                (mode == CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_NOT_DOUBLE_TAG_E))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            regValue = 1;
            regValEArch = 2;
            break;
        case CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_E:
            regValue = 3;
            regValEArch = 1;
            break;
        case CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_INGRESS_WITHOUT_TAG1_AND_NOT_ADD_TAG_1_FROM_SRC_ID_E:
            if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) != GT_TRUE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            regValue    = 0;/*don't care*/
            regValEArch = 3;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.distributor.generalConfig;
        regOffset = 13;
    }
    else if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).
                distributor.distributorGeneralConfigs;
        regOffset = 13;
        regValue = regValEArch;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueMiscCtrl;
        regOffset = 6;
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, regOffset, 2, regValue);
}

/**
* @internal cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet function
* @endinternal
*
* @brief   Set Tag1 removal mode from the egress port Tag State if Tag1 VID
*         is assigned a value of 0.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Vlan Tag1 Removal  when Tag1 VID=0
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT     mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet function
* @endinternal
*
* @brief   Get Tag1 removal mode from the egress port Tag State when Tag1 VID
*         is assigned a value of 0.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) Vlan Tag1 Removal mode when Tag1 VID=0
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_BAD_STATE             - on bad state of register
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet
(
    IN  GT_U8                                               devNum,
    OUT CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT     *modePtr
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value */
    GT_U32  regOffset;  /* offset in register */
    GT_STATUS rc;       /* returned status */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.distributor.generalConfig;
        regOffset = 13;
    }
    else if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).
                distributor.distributorGeneralConfigs;
        regOffset = 13;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueMiscCtrl;
        regOffset = 6;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, regOffset, 2, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        switch (regValue)
        {
            case 0:
                *modePtr = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E;
                break;
            case 1:
                *modePtr = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_E;
                break;
            case 2:
                *modePtr = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_AND_INGRESS_WITHOUT_TAG1_E;
                break;
            case 3:
                if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) == GT_TRUE)
                {
                    *modePtr = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_INGRESS_WITHOUT_TAG1_AND_NOT_ADD_TAG_1_FROM_SRC_ID_E;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        switch (regValue)
        {
            case 0:
                *modePtr = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E;
                break;
            case 1:
                *modePtr = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_NOT_DOUBLE_TAG_E;
                break;
            case 3:
                *modePtr = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_E;
                break;
            default:
                /* may get here on regValue == 2 */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet function
* @endinternal
*
* @brief   Get Tag1 removal mode from the egress port Tag State when Tag1 VID
*         is assigned a value of 0.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) Vlan Tag1 Removal mode when Tag1 VID=0
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_BAD_STATE             - on bad state of register
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet
(
    IN  GT_U8                                               devNum,
    OUT CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT     *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanForceNewDsaToCpuEnableSet function
* @endinternal
*
* @brief   Enable / Disable preserving the original VLAN tag
*         and VLAN EtherType (or tags),
*         and including the new VLAN Id assignment in the TO_CPU DSA tag
*         for tagged packets that are sent to the CPU.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE - Double Tag to CPU: Packets sent to the
*                                      CPU whose Vlan Id was modified by the device,
*                                      with a DSA tag that contains the newly
*                                      assigned Vlan Id and the original Vlan tag
*                                      (or tags) that is preserved after the DSA tag.
*                                      - GT_FALSE - If the original packet contained a Vlan tag,
*                                      it is removed and the DSA tag contains
*                                      the newly assigned Vlan Id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanForceNewDsaToCpuEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_U32    regAddr;           /* register address */
    GT_U32    data;              /* reg sub field data */
    GT_U32    numOfNetIfs, i;
    GT_PHYSICAL_PORT_NUM cpuPortNumArr[CPSS_MAX_SDMA_CPU_PORTS_CNS];
    GT_STATUS  rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);

    data = (enable == GT_TRUE) ? 1 : 0;

    /*Note:in Bobcat2; Caelum; Bobcat3 the configuration is 'per physical port'*/
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* run over all cpu ports*/
        rc = prvCpssDxChPortMappingCPUPortGet(devNum, cpuPortNumArr, &numOfNetIfs);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(numOfNetIfs == 0)
        {
            /* this BC2 A0 device and function is called from phase1init before mapping is set.
               see PRV_CPSS_DXCH_BOBCAT2_NON_ORIG_VID_ON_TRAP_TO_CPU_WA_E. */
            numOfNetIfs = 1;
            cpuPortNumArr[0] = CPSS_CPU_PORT_NUM_CNS;
        }

        for (i=0; i < numOfNetIfs; i++)
        {
            rc =  prvCpssDxChWriteTableEntryField(devNum,
                                                  CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                                  cpuPortNumArr[i],/*global port*/
                                                  PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                  SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_TO_CPU_E, /* field name */
                                                  PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                  data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        return rc;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltGlobalConfig;

        /* Enable / Disable preserving the original VLAN tag  */
        return prvCpssHwPpSetRegField(devNum, regAddr, 17, 1, data);
    }

}

/**
* @internal cpssDxChBrgVlanForceNewDsaToCpuEnableSet function
* @endinternal
*
* @brief   Enable / Disable preserving the original VLAN tag
*         and VLAN EtherType (or tags),
*         and including the new VLAN Id assignment in the TO_CPU DSA tag
*         for tagged packets that are sent to the CPU.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE - Double Tag to CPU: Packets sent to the
*                                      CPU whose Vlan Id was modified by the device,
*                                      with a DSA tag that contains the newly
*                                      assigned Vlan Id and the original Vlan tag
*                                      (or tags) that is preserved after the DSA tag.
*                                      - GT_FALSE - If the original packet contained a Vlan tag,
*                                      it is removed and the DSA tag contains
*                                      the newly assigned Vlan Id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanForceNewDsaToCpuEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanForceNewDsaToCpuEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgVlanForceNewDsaToCpuEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanForceNewDsaToCpuEnableGet function
* @endinternal
*
* @brief   Get Force New DSA To Cpu mode for enabling / disabling
*         preserving the original VLAN tag and VLAN EtherType (or tags),
*         and including the new VLAN Id assignment in the TO_CPU DSA tag
*         for tagged packets that are sent to the CPU.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - pointer to Force New DSA To Cpu mode.
*                                      - GT_TRUE - Double Tag to CPU: Packets sent to the
*                                      CPU whose Vlan Id was modified by the device,
*                                      with a DSA tag that contains the newly
*                                      assigned Vlan Id and the original Vlan tag
*                                      (or tags) that is preserved after the DSA tag.
*                                      - GT_FALSE - If the original packet contained a Vlan tag,
*                                      it is removed and the DSA tag contains
*                                      the newly assigned Vlan Id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanForceNewDsaToCpuEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_U32    regAddr;     /* register address */
    GT_U32    data;        /* reg sub field data */
    GT_STATUS rc;          /* return status                */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /*Note:in Bobcat2; Caelum; Bobcat3 the configuration is 'per physical port'*/
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                              CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                              CPSS_CPU_PORT_NUM_CNS,/*global port*/
                                              PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                              SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_TO_CPU_E, /* field name */
                                              PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                              &data);
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.hdrAltGlobalConfig;

        /* Get Force New DSA To Cpu mode for enabling / disabling
           preserving the original VLAN tag */
        rc =  prvCpssHwPpGetRegField(devNum, regAddr, 17, 1, &data);
    }

    *enablePtr = (data == 0) ? GT_FALSE : GT_TRUE;

    return rc;

}

/**
* @internal cpssDxChBrgVlanForceNewDsaToCpuEnableGet function
* @endinternal
*
* @brief   Get Force New DSA To Cpu mode for enabling / disabling
*         preserving the original VLAN tag and VLAN EtherType (or tags),
*         and including the new VLAN Id assignment in the TO_CPU DSA tag
*         for tagged packets that are sent to the CPU.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - pointer to Force New DSA To Cpu mode.
*                                      - GT_TRUE - Double Tag to CPU: Packets sent to the
*                                      CPU whose Vlan Id was modified by the device,
*                                      with a DSA tag that contains the newly
*                                      assigned Vlan Id and the original Vlan tag
*                                      (or tags) that is preserved after the DSA tag.
*                                      - GT_FALSE - If the original packet contained a Vlan tag,
*                                      it is removed and the DSA tag contains
*                                      the newly assigned Vlan Id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanForceNewDsaToCpuEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanForceNewDsaToCpuEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgVlanForceNewDsaToCpuEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanKeepVlan1EnableSet function
* @endinternal
*
* @brief   Enable/Disable keeping VLAN1 in the packet, for packets received with VLAN1,
*         even-though the tag-state of this egress-port is configured in the
*         VLAN-table to "untagged" or "VLAN0".
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up                       - VLAN tag 0 User Priority (APPLICABLE RANGES: 0..7)
* @param[in] enable                   - GT_TRUE: If the packet is received with VLAN1 and
*                                      VLAN Tag state is "VLAN0" or "untagged"
*                                      then VLAN1 is not removed from the packet.
*                                      GT_FALSE: Tag state assigned by VLAN is preserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or up
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanKeepVlan1EnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U8                       up,
    IN  GT_BOOL                     enable
)
{
    GT_U32    regAddr;     /* register address                              */
    GT_U32    data;        /* reg sub field data                            */
    GT_U32    regOffset;   /* register offset                               */
    GT_U32    fieldOffset; /* field offset inside register                  */
    GT_U32    portGroupId; /* the port group Id - support
                              multi-port-groups device                      */
    GT_U32     localPort;  /* local port - support multi-port-groups device  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(up);

    data = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);

        return prvCpssDxChWriteTableEntryField(devNum,
                                              CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
                                              portNum,/*global port*/
                                              PRV_CPSS_DXCH_TABLE_WORD_INDICATE_SUB_FIELD_IN_FIELD_NAME_CNS,
                                              SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PER_UP0_KEEP_VLAN1_ENABLE_E, /* field name */
                                              PRV_CPSS_DXCH_TABLE_SUB_FIELD_OFFSET_AND_LENGTH_MAC(up,1),/* sub field info {offset,length}*/
                                              data);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        regOffset = (localPort / 4);
        fieldOffset = ((localPort % 4) * 8) + up;
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.keepVlan1Reg[regOffset];

        return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                              fieldOffset, 1, data);
    }
}

/**
* @internal cpssDxChBrgVlanKeepVlan1EnableSet function
* @endinternal
*
* @brief   Enable/Disable keeping VLAN1 in the packet, for packets received with VLAN1,
*         even-though the tag-state of this egress-port is configured in the
*         VLAN-table to "untagged" or "VLAN0".
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up                       - VLAN tag 0 User Priority (APPLICABLE RANGES: 0..7)
* @param[in] enable                   - GT_TRUE: If the packet is received with VLAN1 and
*                                      VLAN Tag state is "VLAN0" or "untagged"
*                                      then VLAN1 is not removed from the packet.
*                                      GT_FALSE: Tag state assigned by VLAN is preserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or up
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanKeepVlan1EnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U8                       up,
    IN  GT_BOOL                     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanKeepVlan1EnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, up, enable));

    rc = internal_cpssDxChBrgVlanKeepVlan1EnableSet(devNum, portNum, up, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, up, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanKeepVlan1EnableGet function
* @endinternal
*
* @brief   Get status of keeping VLAN1 in the packet, for packets received with VLAN1.
*         (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up                       - VLAN tag 0 User Priority (APPLICABLE RANGES: 0..7)
*
* @param[out] enablePtr                - GT_TRUE: If the packet is received with VLAN1 and
*                                      VLAN Tag state is "VLAN0" or "untagged"
*                                      then VLAN1 is not removed from the packet.
*                                      GT_FALSE: Tag state assigned by VLAN is preserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or up
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanKeepVlan1EnableGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U8                       up,
    OUT GT_BOOL                     *enablePtr
)
{
    GT_U32    regAddr;     /* register address                              */
    GT_U32    data;        /* reg sub field data                            */
    GT_U32    regOffset;   /* register offset                               */
    GT_U32    fieldOffset; /* field offset inside register                  */
    GT_U32    portGroupId; /* the port group Id - support
                              multi-port-groups device                      */
    GT_U32     localPort;   /* local port - support multi-port-groups device */
    GT_STATUS rc;          /* return status                                 */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(up);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);

        rc = prvCpssDxChReadTableEntryField(devNum,
                                              CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
                                              portNum,/*global port*/
                                              PRV_CPSS_DXCH_TABLE_WORD_INDICATE_SUB_FIELD_IN_FIELD_NAME_CNS,
                                              SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PER_UP0_KEEP_VLAN1_ENABLE_E, /* field name */
                                              PRV_CPSS_DXCH_TABLE_SUB_FIELD_OFFSET_AND_LENGTH_MAC(up,1),/* sub field info {offset,length}*/
                                              &data);
    }
    else
    {
        PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum, portNum);

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        regOffset = (localPort / 4);
        fieldOffset = ((localPort % 4) * 8) + up;

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.keepVlan1Reg[regOffset];

        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                fieldOffset, 1, &data);
    }

    *enablePtr = BIT2BOOL_MAC(data);

    return rc;
}

/**
* @internal cpssDxChBrgVlanKeepVlan1EnableGet function
* @endinternal
*
* @brief   Get status of keeping VLAN1 in the packet, for packets received with VLAN1.
*         (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up                       - VLAN tag 0 User Priority (APPLICABLE RANGES: 0..7)
*
* @param[out] enablePtr                - GT_TRUE: If the packet is received with VLAN1 and
*                                      VLAN Tag state is "VLAN0" or "untagged"
*                                      then VLAN1 is not removed from the packet.
*                                      GT_FALSE: Tag state assigned by VLAN is preserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or up
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanKeepVlan1EnableGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U8                       up,
    OUT GT_BOOL                     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanKeepVlan1EnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, up, enablePtr));

    rc = internal_cpssDxChBrgVlanKeepVlan1EnableGet(devNum, portNum, up, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, up, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet function
* @endinternal
*
* @brief   Enable Learning Of Original Tag1 VID Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE  -  learning
*                                      GT_FALSE - disable learning
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_U32    hwData;      /* value to write */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);


    hwData = BOOL2BIT_MAC(enable);

    /* set the <EN_LEARN_ORIG_TAG1_VID>*/
    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                    portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_EPORT_TABLE_FIELDS_EN_LEARN_ORIG_TAG1_VID_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                    hwData);
}

/**
* @internal cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet function
* @endinternal
*
* @brief   Enable Learning Of Original Tag1 VID Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE  -  learning
*                                      GT_FALSE - disable learning
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet function
* @endinternal
*
* @brief   Get Learning mode Of Original Tag1 VID Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable learning
*                                      GT_FALSE - disable learning
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    GT_U32    hwData;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);


    /* get the <EN_LEARN_ORIG_TAG1_VID>*/
    rc = prvCpssDxChReadTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
                                    portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_EPORT_TABLE_FIELDS_EN_LEARN_ORIG_TAG1_VID_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                    &hwData);
    if(rc!=GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(hwData);

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet function
* @endinternal
*
* @brief   Get Learning mode Of Original Tag1 VID Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - enable learning
*                                      GT_FALSE - disable learning
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet function
* @endinternal
*
* @brief   When enabled, IPv4 MC/BC packets in this eVLAN are mirrored to the
*         analyzer specified in this field. This is independent of the analyzer
*         configuration of other traffic in this eVLAN.
*         0x0 is a reserved value that means Disabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
* @param[in] enable                   - GT_TRUE - IPv4 MC/BC packets in this eVLAN are mirrored to the
*                                      analyzer specified in this field
*                                      GT_FALSE - IPv4 MC/BC packets in this eVLAN are NOT mirrored
* @param[in] index                    - Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_BOOL enable,
    IN GT_U32  index
)
{
    GT_STATUS rc;
    GT_U32    hwData;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);


    if(enable == GT_FALSE) /* ignore index and set 0 */
    {
        hwData = 0;
    }
    else
    {
        if(index >  PRV_CPSS_DXCH_MIRROR_TO_ANALYZER_MAX_INDEX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        hwData = index + 1;
    }

    rc = prvCpssDxChWriteTableEntryField(devNum,
                                       CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                       (GT_U32)vlanId,
                                       PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                       SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_MC_BC_TO_MIRROR_ANLYZER_IDX_E,
                                       PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                       hwData);

    return rc;
}

/**
* @internal cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet function
* @endinternal
*
* @brief   When enabled, IPv4 MC/BC packets in this eVLAN are mirrored to the
*         analyzer specified in this field. This is independent of the analyzer
*         configuration of other traffic in this eVLAN.
*         0x0 is a reserved value that means Disabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
* @param[in] enable                   - GT_TRUE - IPv4 MC/BC packets in this eVLAN are mirrored to the
*                                      analyzer specified in this field
*                                      GT_FALSE - IPv4 MC/BC packets in this eVLAN are NOT mirrored
* @param[in] index                    - Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_BOOL enable,
    IN GT_U32  index
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, enable, index));

    rc = internal_cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet(devNum, vlanId, enable, index);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, enable, index));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet function
* @endinternal
*
* @brief   Get Ipv4 MC/BC Mirror To Analyzer Index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE - IPv4 MC/BC packets in this eVLAN are mirrored to the
*                                      analyzer specified in this field
*                                      GT_FALSE - IPv4 MC/BC packets in this eVLAN are NOT mirrored
* @param[out] indexPtr                 - (pointer to) Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlanId,
    OUT GT_BOOL *enablePtr,
    OUT GT_U32  *indexPtr
)
{
    GT_STATUS rc;
    GT_U32    hwData;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(indexPtr);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);


     rc = prvCpssDxChReadTableEntryField(devNum,
                                       CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                       (GT_U32)vlanId,
                                       PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                       SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_MC_BC_TO_MIRROR_ANLYZER_IDX_E,
                                       PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                       &hwData);

     if(rc != GT_OK)
         return rc;

    if(hwData == 0) /* ignore index and set 0 */
    {
        *enablePtr = GT_FALSE;
        *indexPtr  = 0;
    }
    else
    {
        *enablePtr = GT_TRUE;
        *indexPtr = hwData-1;
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet function
* @endinternal
*
* @brief   Get Ipv4 MC/BC Mirror To Analyzer Index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE - IPv4 MC/BC packets in this eVLAN are mirrored to the
*                                      analyzer specified in this field
*                                      GT_FALSE - IPv4 MC/BC packets in this eVLAN are NOT mirrored
* @param[out] indexPtr                 - (pointer to) Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlanId,
    OUT GT_BOOL *enablePtr,
    OUT GT_U32  *indexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, enablePtr, indexPtr));

    rc = internal_cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet(devNum, vlanId, enablePtr, indexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, enablePtr, indexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet function
* @endinternal
*
* @brief   When enabled, IPv6 MC packets in this eVLAN are mirrored to the
*         analyzer. This is independent of the analyzer configuration of other
*         traffic in this eVLAN.
*         0x0 is a reserved value that means Disabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
* @param[in] enable                   - GT_TRUE - IPv6 MC packets in this eVLAN are mirrored to the
*                                      analyzer specified in this field
*                                      GT_FALSE - IPv6 MC packets in this eVLAN are NOT mirrored to the
*                                      analyzer specified in this field
* @param[in] index                    - Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_BOOL enable,
    IN GT_U32  index
)
{
    GT_STATUS rc;
    GT_U32    hwData;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);


    if(enable == GT_FALSE) /* ignore index and set 0 */
    {
        hwData = 0;
    }
    else
    {
        if(index >  PRV_CPSS_DXCH_MIRROR_TO_ANALYZER_MAX_INDEX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        hwData = index + 1;
    }

    /* set the <IPV6_MC_TO_MIRROR_ANALYZER_IDX>*/
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                    CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                    vlanId,/* vid */
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_MC_TO_MIRROR_ANALYZER_IDX_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                    hwData);

    return rc;
}

/**
* @internal cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet function
* @endinternal
*
* @brief   When enabled, IPv6 MC packets in this eVLAN are mirrored to the
*         analyzer. This is independent of the analyzer configuration of other
*         traffic in this eVLAN.
*         0x0 is a reserved value that means Disabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
* @param[in] enable                   - GT_TRUE - IPv6 MC packets in this eVLAN are mirrored to the
*                                      analyzer specified in this field
*                                      GT_FALSE - IPv6 MC packets in this eVLAN are NOT mirrored to the
*                                      analyzer specified in this field
* @param[in] index                    - Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_BOOL enable,
    IN GT_U32  index
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, enable, index));

    rc = internal_cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet(devNum, vlanId, enable, index);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, enable, index));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet function
* @endinternal
*
* @brief   Get Ipv6 MC Mirror To Analyzer Index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE - IPv6 MC packets in this eVLAN are mirrored to the
*                                      analyzer specified in this field
*                                      GT_FALSE - IPv6 MC packets in this eVLAN are NOT mirrored to the
*                                      analyzer specified in this field
* @param[out] indexPtr                 - (pointer to) Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlanId,
    OUT GT_BOOL *enablePtr,
    OUT GT_U32  *indexPtr
)
{
    GT_STATUS rc;
    GT_U32    hwData;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(indexPtr);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);


    /* get the <IPV6_MC_TO_MIRROR_ANALYZER_IDX>*/
    rc = prvCpssDxChReadTableEntryField(devNum,
                                    CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                    vlanId,/* vid */
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_MC_TO_MIRROR_ANALYZER_IDX_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                    &hwData);

     if(rc != GT_OK)
         return rc;

    if(hwData == 0) /* ignore index and set 0 */
    {
        *enablePtr = GT_FALSE;
        *indexPtr  = 0;
    }
    else
    {
        *enablePtr = GT_TRUE;
        *indexPtr = hwData-1;
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet function
* @endinternal
*
* @brief   Get Ipv6 MC Mirror To Analyzer Index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE - IPv6 MC packets in this eVLAN are mirrored to the
*                                      analyzer specified in this field
*                                      GT_FALSE - IPv6 MC packets in this eVLAN are NOT mirrored to the
*                                      analyzer specified in this field
* @param[out] indexPtr                 - (pointer to) Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlanId,
    OUT GT_BOOL *enablePtr,
    OUT GT_U32  *indexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, enablePtr, indexPtr));

    rc = internal_cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet(devNum, vlanId, enablePtr, indexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, enablePtr, indexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanForwardingIdSet function
* @endinternal
*
* @brief   Set Forwarding ID value.
*         Used for virtual birdges per eVLAN.
*         Used by the bridge engine for entry lookup and entry match (replaces VID
*         in bridge entry)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
* @param[in] fidValue                 - Forwarding ID value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanForwardingIdSet
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_U32  fidValue
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);


    if(fidValue > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* set the <FID>*/
    return prvCpssDxChWriteTableEntryField(devNum,
                                    CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                    vlanId,/* vid */
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FID_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                    fidValue);
}

/**
* @internal cpssDxChBrgVlanForwardingIdSet function
* @endinternal
*
* @brief   Set Forwarding ID value.
*         Used for virtual birdges per eVLAN.
*         Used by the bridge engine for entry lookup and entry match (replaces VID
*         in bridge entry)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
* @param[in] fidValue                 - Forwarding ID value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanForwardingIdSet
(
    IN GT_U8   devNum,
    IN GT_U16  vlanId,
    IN GT_U32  fidValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanForwardingIdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, fidValue));

    rc = internal_cpssDxChBrgVlanForwardingIdSet(devNum, vlanId, fidValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, fidValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChBrgVlanForwardingIdGet function
* @endinternal
*
* @brief   Get Forwarding ID value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
*
* @param[out] fidValuePtr              - (pointer to) Forwarding ID value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanForwardingIdGet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlanId,
    OUT GT_U32  *fidValuePtr
)
{
    GT_STATUS rc;
    GT_U32    hwData;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(fidValuePtr);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);


    /* get the <FID>*/
    rc = prvCpssDxChReadTableEntryField(devNum,
                                    CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                    vlanId,/* vid */
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FID_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                    &hwData);
    if(rc != GT_OK)
        return rc;

    *fidValuePtr = hwData;

    return rc;

}

/**
* @internal cpssDxChBrgVlanForwardingIdGet function
* @endinternal
*
* @brief   Get Forwarding ID value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
*
* @param[out] fidValuePtr              - (pointer to) Forwarding ID value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanForwardingIdGet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlanId,
    OUT GT_U32  *fidValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanForwardingIdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, fidValuePtr));

    rc = internal_cpssDxChBrgVlanForwardingIdGet(devNum, vlanId, fidValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, fidValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgVlanUnknownMacSaCommandSet function
* @endinternal
*
* @brief   Set bridge forwarding decision for packets with unknown Source Address.
*         Only relevant in controlled address learning mode, when <Auto-learning
*         enable> = Disabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E , CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanUnknownMacSaCommandSet
(
    IN GT_U8                devNum,
    IN GT_U16               vlanId,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
    GT_U32 hwData=0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);


    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(hwData, cmd);

    /* set the <UNKOWN_MAC_SA>*/
    return prvCpssDxChWriteTableEntryField(devNum,
                                    CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                    vlanId,/* vid */
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNKOWN_MAC_SA_CMD_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                    hwData);

}

/**
* @internal cpssDxChBrgVlanUnknownMacSaCommandSet function
* @endinternal
*
* @brief   Set bridge forwarding decision for packets with unknown Source Address.
*         Only relevant in controlled address learning mode, when <Auto-learning
*         enable> = Disabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
* @param[in] cmd                      - supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E , CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanUnknownMacSaCommandSet
(
    IN GT_U8                devNum,
    IN GT_U16               vlanId,
    IN CPSS_PACKET_CMD_ENT  cmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanUnknownMacSaCommandSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, cmd));

    rc = internal_cpssDxChBrgVlanUnknownMacSaCommandSet(devNum, vlanId, cmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, cmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChBrgVlanUnknownMacSaCommandGet function
* @endinternal
*
* @brief   Get bridge forwarding decision for packets with unknown Source Address.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
*
* @param[out] cmdPtr                   - (pointer to) supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E , CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanUnknownMacSaCommandGet
(
    IN GT_U8                devNum,
    IN GT_U16               vlanId,
    OUT CPSS_PACKET_CMD_ENT *cmdPtr
)
{
    GT_STATUS   rc;
    GT_U32      hwData;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(cmdPtr);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);


    /* get the <UNKOWN_MAC_SA>*/
    rc = prvCpssDxChReadTableEntryField(devNum,
                                    CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                    vlanId,/* vid */
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNKOWN_MAC_SA_CMD_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                    &hwData);
    if(rc != GT_OK)
        return rc;

    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(*cmdPtr, hwData);

    return rc;

}

/**
* @internal cpssDxChBrgVlanUnknownMacSaCommandGet function
* @endinternal
*
* @brief   Get bridge forwarding decision for packets with unknown Source Address.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
*
* @param[out] cmdPtr                   - (pointer to) supported commands:
*                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E , CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanUnknownMacSaCommandGet
(
    IN GT_U8                devNum,
    IN GT_U16               vlanId,
    OUT CPSS_PACKET_CMD_ENT *cmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanUnknownMacSaCommandGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, cmdPtr));

    rc = internal_cpssDxChBrgVlanUnknownMacSaCommandGet(devNum, vlanId, cmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, cmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssDxChBrgVlanEgressPortTagStateModeSet function
* @endinternal
*
* @brief   Set the egress port tag state mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stateMode                - state mode: eVLAN or ePort
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanEgressPortTagStateModeSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_NUM                                      portNum,
    IN CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT      stateMode
)
{
    GT_U32      value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    switch (stateMode)
    {
        case CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E:
            value = 0;
            break;
        case CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E:
            value = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChWriteTableEntryField(devNum,
                                    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                    portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EPORT_TAG_STATE_MODE_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   value);
}

/**
* @internal cpssDxChBrgVlanEgressPortTagStateModeSet function
* @endinternal
*
* @brief   Set the egress port tag state mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] stateMode                - state mode: eVLAN or ePort
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanEgressPortTagStateModeSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_NUM                                      portNum,
    IN CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT      stateMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanEgressPortTagStateModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, stateMode));

    rc = internal_cpssDxChBrgVlanEgressPortTagStateModeSet(devNum, portNum, stateMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, stateMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanEgressPortTagStateModeGet function
* @endinternal
*
* @brief   Get the egress port tag state mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] stateModePtr             - (pointer to) state mode: eVLAN or ePort
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanEgressPortTagStateModeGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PORT_NUM                                         portNum,
    OUT CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT         *stateModePtr
)
{
    GT_U32      value;      /* hardware value */
    GT_STATUS   rc;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(stateModePtr);



    rc = prvCpssDxChReadTableEntryField(devNum,
                                    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                    portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EPORT_TAG_STATE_MODE_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   &value);

    switch (value)
    {
        case 0:
            *stateModePtr = CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E;
            break;
        case 1:
            *stateModePtr = CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanEgressPortTagStateModeGet function
* @endinternal
*
* @brief   Get the egress port tag state mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] stateModePtr             - (pointer to) state mode: eVLAN or ePort
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanEgressPortTagStateModeGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PORT_NUM                                         portNum,
    OUT CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT         *stateModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanEgressPortTagStateModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, stateModePtr));

    rc = internal_cpssDxChBrgVlanEgressPortTagStateModeGet(devNum, portNum, stateModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, stateModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgVlanEgressPortTagStateSet function
* @endinternal
*
* @brief   Determines the egress port state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tagCmd                   - Port tag state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanEgressPortTagStateSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_NUM                                      portNum,
    IN CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT              tagCmd
)
{
    GT_U32      hwDataTagCmd;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    PRV_CPSS_DXCH_CONVERT_VLAN_TAG_MODE_TO_HW_VAL_MAC(devNum, hwDataTagCmd,tagCmd);

    return prvCpssDxChWriteTableEntryField(devNum,
                                    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                    portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EPORT_TAG_STATE_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   hwDataTagCmd);
}

/**
* @internal cpssDxChBrgVlanEgressPortTagStateSet function
* @endinternal
*
* @brief   Determines the egress port state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tagCmd                   - Port tag state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanEgressPortTagStateSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_NUM                                      portNum,
    IN CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT              tagCmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanEgressPortTagStateSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tagCmd));

    rc = internal_cpssDxChBrgVlanEgressPortTagStateSet(devNum, portNum, tagCmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tagCmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanEgressPortTagStateGet function
* @endinternal
*
* @brief   Returns the egress port state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] tagCmdPtr                - (pointer to) Port tag state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanEgressPortTagStateGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PORT_NUM                                         portNum,
    OUT CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT                 *tagCmdPtr
)
{
    GT_U32      value;
    GT_U32      hwValue;    /* hardware value */
    GT_STATUS   rc;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(tagCmdPtr);



    rc = prvCpssDxChReadTableEntryField(devNum,
                                    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                    portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EPORT_TAG_STATE_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   &hwValue);

   PRV_CPSS_DXCH_CONVERT_HW_VAL_TO_VLAN_TAG_MODE_MAC(devNum, hwValue, value);
   *tagCmdPtr = value;

    return rc;
}

/**
* @internal cpssDxChBrgVlanEgressPortTagStateGet function
* @endinternal
*
* @brief   Returns the egress port state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] tagCmdPtr                - (pointer to) Port tag state
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanEgressPortTagStateGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PORT_NUM                                         portNum,
    OUT CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT                 *tagCmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanEgressPortTagStateGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tagCmdPtr));

    rc = internal_cpssDxChBrgVlanEgressPortTagStateGet(devNum, portNum, tagCmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tagCmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortPushVlanCommandSet function
* @endinternal
*
* @brief   Set Push Vlan Command
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] vlanCmd                  - vlan command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortPushVlanCommandSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_NUM                                 portNum,
    IN  CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT    vlanCmd
)
{
    GT_STATUS   rc;
    GT_U32      value;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    switch(vlanCmd)
    {
        case CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E:
            value = 0;
            break;
        case CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_TUNNEL_E:
            value = 1;
            break;
        case CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_PASSENGER_E:
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
                value = 2;
            else
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_EPORT_TABLE_2_PUSH_VLAN_COMMAND_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        value);
    return rc;

}

/**
* @internal cpssDxChBrgVlanPortPushVlanCommandSet function
* @endinternal
*
* @brief   Set Push Vlan Command
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] vlanCmd                  - vlan command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortPushVlanCommandSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_NUM                                 portNum,
    IN  CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT    vlanCmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortPushVlanCommandSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, vlanCmd));

    rc = internal_cpssDxChBrgVlanPortPushVlanCommandSet(devNum, portNum, vlanCmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, vlanCmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortPushVlanCommandGet function
* @endinternal
*
* @brief   Get Push Vlan Command
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] vlanCmdPtr               - (pointer to)  push vlan command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortPushVlanCommandGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_NUM                                 portNum,
    OUT CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT    *vlanCmdPtr
)
{
    GT_STATUS   rc;     /* function return code              */
    GT_U32      value;  /* value to write                    */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(vlanCmdPtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_EPORT_TABLE_2_PUSH_VLAN_COMMAND_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);

    if(rc != GT_OK)
        return rc;

    switch(value)
    {
    case 0:
        *vlanCmdPtr = CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E;
        break;
    case 1:
        *vlanCmdPtr = CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_TUNNEL_E;
        break;
    case 2:
        *vlanCmdPtr = CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_PASSENGER_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortPushVlanCommandGet function
* @endinternal
*
* @brief   Get Push Vlan Command
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] vlanCmdPtr               - (pointer to)  push vlan command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortPushVlanCommandGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_NUM                                 portNum,
    OUT CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT    *vlanCmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortPushVlanCommandGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, vlanCmdPtr));

    rc = internal_cpssDxChBrgVlanPortPushVlanCommandGet(devNum, portNum, vlanCmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, vlanCmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet function
* @endinternal
*
* @brief   Set TPID select table index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tpidEntryIndex           - TPID select table index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U32           tpidEntryIndex
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    if(tpidEntryIndex >= BIT_3)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_EPORT_TABLE_2_PUSHED_TAG_TPID_SELECT_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        tpidEntryIndex);
    return rc;

}

/**
* @internal cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet function
* @endinternal
*
* @brief   Set TPID select table index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tpidEntryIndex           - TPID select table index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U32           tpidEntryIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tpidEntryIndex));

    rc = internal_cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(devNum, portNum, tpidEntryIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tpidEntryIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet function
* @endinternal
*
* @brief   Get TPID select table index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] tpidEntryIndexPtr        - (pointer to) TPID select table index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *tpidEntryIndexPtr
)
{
    GT_STATUS   rc;     /* function return code              */
    GT_U32      value;  /* value to write                    */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(tpidEntryIndexPtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_EPORT_TABLE_2_PUSHED_TAG_TPID_SELECT_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);

    if(rc != GT_OK)
        return rc;

    *tpidEntryIndexPtr = value;

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet function
* @endinternal
*
* @brief   Get TPID select table index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] tpidEntryIndexPtr        - (pointer to) TPID select table index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *tpidEntryIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tpidEntryIndexPtr));

    rc = internal_cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet(devNum, portNum, tpidEntryIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tpidEntryIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortPushedTagValueSet function
* @endinternal
*
* @brief   Set the tag value to push
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tagValue                 - tag value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortPushedTagValueSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U16           tagValue
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    if(tagValue >= BIT_12)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_EPORT_TABLE_2_PUSHED_TAG_VALUE_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        tagValue);
    return rc;

}

/**
* @internal cpssDxChBrgVlanPortPushedTagValueSet function
* @endinternal
*
* @brief   Set the tag value to push
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tagValue                 - tag value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortPushedTagValueSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U16           tagValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortPushedTagValueSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tagValue));

    rc = internal_cpssDxChBrgVlanPortPushedTagValueSet(devNum, portNum, tagValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tagValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortPushedTagValueGet function
* @endinternal
*
* @brief   Get the tag value to push
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] tagValuePtr              - (pointer to) tag value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortPushedTagValueGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U16          *tagValuePtr
)
{
    GT_STATUS   rc;     /* function return code              */
    GT_U32      value;  /* value to write                    */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(tagValuePtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_EPORT_TABLE_2_PUSHED_TAG_VALUE_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);

    if(rc != GT_OK)
        return rc;

    *tagValuePtr = (GT_U16)value;

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortPushedTagValueGet function
* @endinternal
*
* @brief   Get the tag value to push
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] tagValuePtr              - (pointer to) tag value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortPushedTagValueGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U16          *tagValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortPushedTagValueGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tagValuePtr));

    rc = internal_cpssDxChBrgVlanPortPushedTagValueGet(devNum, portNum, tagValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tagValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet function
* @endinternal
*
* @brief   Determines if UP and CFI values will be assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - Enable/Disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_EPORT_TABLE_2_UP_CFI_ASSIGNMENT_COMMAND_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        BOOL2BIT_MAC(enable));
    return rc;

}

/**
* @internal cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet function
* @endinternal
*
* @brief   Determines if UP and CFI values will be assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - Enable/Disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet function
* @endinternal
*
* @brief   Get if UP and CFI values will be assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) Enable/Disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS   rc;     /* function return code              */
    GT_U32      value;  /* value to write                    */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_EPORT_TABLE_2_UP_CFI_ASSIGNMENT_COMMAND_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);

    if(rc != GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet function
* @endinternal
*
* @brief   Get if UP and CFI values will be assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) Enable/Disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortUpSet function
* @endinternal
*
* @brief   Set UP value to assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up                       - UP value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortUpSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U32           up
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    if(up >= BIT_3)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);



    rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_EPORT_TABLE_2_UP_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        up);
    return rc;

}

/**
* @internal cpssDxChBrgVlanPortUpSet function
* @endinternal
*
* @brief   Set UP value to assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up                       - UP value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUpSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U32           up
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortUpSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, up));

    rc = internal_cpssDxChBrgVlanPortUpSet(devNum, portNum, up);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, up));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortUpGet function
* @endinternal
*
* @brief   Get UP value to assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] upPtr                    - (pointer to)  UP value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortUpGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *upPtr
)
{
    GT_STATUS   rc;     /* function return code              */
    GT_U32      value;  /* value to write                    */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(upPtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_EPORT_TABLE_2_UP_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);

    if(rc != GT_OK)
        return rc;

    *upPtr = value;

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortUpGet function
* @endinternal
*
* @brief   Get UP value to assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] upPtr                    - (pointer to)  UP value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUpGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *upPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortUpGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, upPtr));

    rc = internal_cpssDxChBrgVlanPortUpGet(devNum, portNum, upPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, upPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortCfiEnableSet function
* @endinternal
*
* @brief   Set CFI value to assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - CFI value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortCfiEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_EPORT_TABLE_2_CFI_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        BOOL2BIT_MAC(enable));
    return rc;

}

/**
* @internal cpssDxChBrgVlanPortCfiEnableSet function
* @endinternal
*
* @brief   Set CFI value to assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - CFI value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortCfiEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortCfiEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgVlanPortCfiEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortCfiEnableGet function
* @endinternal
*
* @brief   Get CFI value to assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) CFI value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortCfiEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS   rc;     /* function return code              */
    GT_U32      value;  /* value to write                    */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_EPORT_TABLE_2_CFI_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);

    if(rc != GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortCfiEnableGet function
* @endinternal
*
* @brief   Get CFI value to assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to) CFI value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortCfiEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortCfiEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgVlanPortCfiEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortNumOfTagWordsToPopSet function
* @endinternal
*
* @brief   Set the number of bytes to pop for traffic that ingress from the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] numBytesToPop            - enumeration to set the number of bytes to pop.
*                                      (for popping 0/4/8/6 bytes )
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortNumOfTagWordsToPopSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT numBytesToPop
)
{
    GT_STATUS   rc;
    GT_U32      hwValue;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    switch(numBytesToPop)
    {
        case GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_NONE_E:
            hwValue = 0;
            break;
        case GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_4_E:
            hwValue = 1;
            break;
        case GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_8_E:
            hwValue = 2;
            break;
        case GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_6_E:
            if (!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            hwValue = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E,
                                         portNum,
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                         SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_NUM_OF_TAGS_TO_POP_E, /* field name */
                                         PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                         hwValue);
    return rc;
}

/**
* @internal cpssDxChBrgVlanPortNumOfTagWordsToPopSet function
* @endinternal
*
* @brief   Set the number of bytes to pop for traffic that ingress from the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] numBytesToPop            - enumeration to set the number of bytes to pop.
*                                      (for popping 0/4/8/6 bytes )
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortNumOfTagWordsToPopSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT numBytesToPop
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortNumOfTagWordsToPopSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, numBytesToPop));

    rc = internal_cpssDxChBrgVlanPortNumOfTagWordsToPopSet(devNum, portNum, numBytesToPop);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, numBytesToPop));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortNumOfTagWordsToPopGet function
* @endinternal
*
* @brief   Get the number of bytes to pop for traffic that ingress from the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] numBytesToPopPtr         - (pointer to) enumeration to set the number of bytes to pop.
*                                      (for popping 0/4/8/6 bytes )
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on not supported value read from the HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortNumOfTagWordsToPopGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT  GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT *numBytesToPopPtr
)
{
    GT_STATUS   rc;     /* function return code              */
    GT_U32      hwValue;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(numBytesToPopPtr);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_NUM_OF_TAGS_TO_POP_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(hwValue)
    {
        case 0:
            *numBytesToPopPtr = GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_NONE_E;
            break;
        case 1:
            *numBytesToPopPtr = GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_4_E;
            break;
        case 2:
            *numBytesToPopPtr = GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_8_E;
            break;
        case 3:
            if (!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
            *numBytesToPopPtr = GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_6_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanPortNumOfTagWordsToPopGet function
* @endinternal
*
* @brief   Get the number of bytes to pop for traffic that ingress from the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] numBytesToPopPtr         - (pointer to) enumeration to set the number of bytes to pop.
*                                      (for popping 0/4/8/6 bytes )
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on not supported value read from the HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortNumOfTagWordsToPopGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT  GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT *numBytesToPopPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortNumOfTagWordsToPopGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, numBytesToPopPtr));

    rc = internal_cpssDxChBrgVlanPortNumOfTagWordsToPopGet(devNum, portNum, numBytesToPopPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, numBytesToPopPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortVid0CommandEnableSet function
* @endinternal
*
* @brief   Enable/Disable changing VID0 of the egress packet
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE: Packet VID0 is not changed.
*                                      GT_TRUE:  Assign new VID0 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortVid0CommandEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChWriteTableEntryField(devNum,
                                    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                    portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EVLAN_COMMAND_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                    BOOL2BIT_MAC(enable));
    return rc;

}

/**
* @internal cpssDxChBrgVlanPortVid0CommandEnableSet function
* @endinternal
*
* @brief   Enable/Disable changing VID0 of the egress packet
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE: Packet VID0 is not changed.
*                                      GT_TRUE:  Assign new VID0 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortVid0CommandEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortVid0CommandEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgVlanPortVid0CommandEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortVid0CommandEnableGet function
* @endinternal
*
* @brief   Return if changing VID0 of the egress packet is enabled/disabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: Packet VID0 is not changed.
*                                      GT_TRUE:  Assign new VID0 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortVid0CommandEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS   rc;     /* function return code              */
    GT_U32      value;  /* value to write                    */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChReadTableEntryField(devNum,
                                    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                    portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EVLAN_COMMAND_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                    &value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortVid0CommandEnableGet function
* @endinternal
*
* @brief   Return if changing VID0 of the egress packet is enabled/disabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: Packet VID0 is not changed.
*                                      GT_TRUE:  Assign new VID0 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortVid0CommandEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortVid0CommandEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgVlanPortVid0CommandEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortVid1CommandEnableSet function
* @endinternal
*
* @brief   Enable/Disable changing VID1 of the egress packet
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE: Packet VID1 is not changed.
*                                      GT_TRUE:  Assign new VID1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortVid1CommandEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChWriteTableEntryField(devNum,
                                    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                    portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_VID1_COMMAND_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                    BOOL2BIT_MAC(enable));
    return rc;

}

/**
* @internal cpssDxChBrgVlanPortVid1CommandEnableSet function
* @endinternal
*
* @brief   Enable/Disable changing VID1 of the egress packet
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE: Packet VID1 is not changed.
*                                      GT_TRUE:  Assign new VID1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortVid1CommandEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortVid1CommandEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgVlanPortVid1CommandEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortVid1CommandEnableGet function
* @endinternal
*
* @brief   Return if changing VID1 of the egress packet is enabled/disabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: Packet VID1 is not changed.
*                                      GT_TRUE:  Assign new VID1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortVid1CommandEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS   rc;     /* function return code              */
    GT_U32      value;  /* value to write                    */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChReadTableEntryField(devNum,
                                    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                    portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_VID1_COMMAND_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                    &value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortVid1CommandEnableGet function
* @endinternal
*
* @brief   Return if changing VID1 of the egress packet is enabled/disabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: Packet VID1 is not changed.
*                                      GT_TRUE:  Assign new VID1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortVid1CommandEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortVid1CommandEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgVlanPortVid1CommandEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortUp0CommandEnableSet function
* @endinternal
*
* @brief   Enable/Disable changing UP1 of the egress packet
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE: Packet UP1 is not changed.
*                                      GT_TRUE:  Assign new UP1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortUp0CommandEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChWriteTableEntryField(devNum,
                                    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                    portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP0_COMMAND_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                    BOOL2BIT_MAC(enable));
    return rc;

}

/**
* @internal cpssDxChBrgVlanPortUp0CommandEnableSet function
* @endinternal
*
* @brief   Enable/Disable changing UP1 of the egress packet
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE: Packet UP1 is not changed.
*                                      GT_TRUE:  Assign new UP1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUp0CommandEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortUp0CommandEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgVlanPortUp0CommandEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortUp0CommandEnableGet function
* @endinternal
*
* @brief   Return if changing UP0 of the egress packet is enabled/disabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: Packet UP1 is not changed.
*                                      GT_TRUE:  Assign new UP1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortUp0CommandEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS   rc;     /* function return code              */
    GT_U32      value;  /* value to write                    */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChReadTableEntryField(devNum,
                                    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                    portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP0_COMMAND_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                    &value);

    if(rc != GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortUp0CommandEnableGet function
* @endinternal
*
* @brief   Return if changing UP0 of the egress packet is enabled/disabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: Packet UP1 is not changed.
*                                      GT_TRUE:  Assign new UP1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUp0CommandEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortUp0CommandEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgVlanPortUp0CommandEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortUp1CommandEnableSet function
* @endinternal
*
* @brief   Enable/Disable changing UP1 of the egress packet
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE: Packet UP1 is not changed.
*                                      GT_TRUE:  Assign new UP1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortUp1CommandEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChWriteTableEntryField(devNum,
                                    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                    portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP1_COMMAND_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                    BOOL2BIT_MAC(enable));
    return rc;

}

/**
* @internal cpssDxChBrgVlanPortUp1CommandEnableSet function
* @endinternal
*
* @brief   Enable/Disable changing UP1 of the egress packet
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_FALSE: Packet UP1 is not changed.
*                                      GT_TRUE:  Assign new UP1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUp1CommandEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortUp1CommandEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgVlanPortUp1CommandEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortUp1CommandEnableGet function
* @endinternal
*
* @brief   Return if changing UP1 of the egress packet is enabled/disabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: Packet UP1 is not changed.
*                                      GT_TRUE:  Assign new UP1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortUp1CommandEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS   rc;     /* function return code              */
    GT_U32      value;  /* value to write                    */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChReadTableEntryField(devNum,
                                    CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                    portNum,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                    SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP1_COMMAND_E, /* field name */
                                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                    &value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortUp1CommandEnableGet function
* @endinternal
*
* @brief   Return if changing UP1 of the egress packet is enabled/disabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: Packet UP1 is not changed.
*                                      GT_TRUE:  Assign new UP1 to the egress packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUp1CommandEnableGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortUp1CommandEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgVlanPortUp1CommandEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgVlanPortVid1Set function
* @endinternal
*
* @brief   For ingress direction : Set port's default Vid1.
*         For egress direction : Set port's Vid1 when egress port <VID1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort for ingress direction.
* @param[in] direction                - ingress/egress direction
* @param[in] vid1                     - VID1 value (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortVid1Set
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         portNum,
    IN CPSS_DIRECTION_ENT   direction,
    IN  GT_U16              vid1
)
{
    GT_STATUS   rc;


    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_VLAN_VALUE_CHECK_MAC(vid1);


    switch(direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

            /* write to pre-tti-lookup-ingress-eport table */
            rc = prvCpssDxChWriteTableEntryField(devNum,
                                                CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                                portNum,
                                                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_DEF_TAG1_VLAN_ID_E, /* field name */
                                                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                vid1);
            return rc;

        case CPSS_DIRECTION_EGRESS_E:
            PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

            rc = prvCpssDxChWriteTableEntryField(devNum,
                                                CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                                portNum,
                                                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_VID1_E, /* field name */
                                                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                vid1);
            return rc;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

}

/**
* @internal cpssDxChBrgVlanPortVid1Set function
* @endinternal
*
* @brief   For ingress direction : Set port's default Vid1.
*         For egress direction : Set port's Vid1 when egress port <VID1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort for ingress direction.
* @param[in] direction                - ingress/egress direction
* @param[in] vid1                     - VID1 value (APPLICABLE RANGES: 0..4095)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortVid1Set
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         portNum,
    IN CPSS_DIRECTION_ENT   direction,
    IN  GT_U16              vid1
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortVid1Set);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, vid1));

    rc = internal_cpssDxChBrgVlanPortVid1Set(devNum, portNum, direction, vid1);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, vid1));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortVid1Get function
* @endinternal
*
* @brief   For ingress direction : Get port's default Vid1.
*         For egress direction : Get port's Vid1 when egress port <VID1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort for ingress direction.
* @param[in] direction                - ingress/egress direction
*
* @param[out] vid1Ptr                  - (pointer to)VID1 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortVid1Get
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         portNum,
    IN CPSS_DIRECTION_ENT   direction,
    OUT  GT_U16             *vid1Ptr
)
{
    GT_STATUS   rc;
    GT_U32      value;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(vid1Ptr);


    switch(direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum,portNum);

            /* read from pre-tti-lookup-ingress-eport table */
            rc = prvCpssDxChReadTableEntryField(devNum,
                                                CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                                                portNum,
                                                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_DEF_TAG1_VLAN_ID_E, /* field name */
                                                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                &value);
            break;
        case CPSS_DIRECTION_EGRESS_E:
            PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

            rc = prvCpssDxChReadTableEntryField(devNum,
                                                CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                                portNum,
                                                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                                SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_VID1_E, /* field name */
                                                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                                &value);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    *vid1Ptr = (GT_U16)value;

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortVid1Get function
* @endinternal
*
* @brief   For ingress direction : Get port's default Vid1.
*         For egress direction : Get port's Vid1 when egress port <VID1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort for ingress direction.
* @param[in] direction                - ingress/egress direction
*
* @param[out] vid1Ptr                  - (pointer to)VID1 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortVid1Get
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         portNum,
    IN CPSS_DIRECTION_ENT   direction,
    OUT  GT_U16             *vid1Ptr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortVid1Get);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, vid1Ptr));

    rc = internal_cpssDxChBrgVlanPortVid1Get(devNum, portNum, direction, vid1Ptr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, vid1Ptr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortUp0Set function
* @endinternal
*
* @brief   Set Up0 assigned to the egress packet if <UP0 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up0                      - UP0 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortUp0Set
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U32           up0
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    if(up0 >= BIT_3)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP0_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        up0);
    return rc;

}

/**
* @internal cpssDxChBrgVlanPortUp0Set function
* @endinternal
*
* @brief   Set Up0 assigned to the egress packet if <UP0 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up0                      - UP0 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUp0Set
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U32           up0
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortUp0Set);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, up0));

    rc = internal_cpssDxChBrgVlanPortUp0Set(devNum, portNum, up0);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, up0));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortUp0Get function
* @endinternal
*
* @brief   Return the UP0 assigned to the egress packet if <UP0 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] up0Ptr                   - (pointer to) UP0 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortUp0Get
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *up0Ptr
)
{
    GT_STATUS   rc;     /* function return code              */
    GT_U32      value;  /* value to write                    */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(up0Ptr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP0_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);

    *up0Ptr = value;

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortUp0Get function
* @endinternal
*
* @brief   Return the UP0 assigned to the egress packet if <UP0 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] up0Ptr                   - (pointer to) UP0 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUp0Get
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *up0Ptr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortUp0Get);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, up0Ptr));

    rc = internal_cpssDxChBrgVlanPortUp0Get(devNum, portNum, up0Ptr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, up0Ptr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortUp1Set function
* @endinternal
*
* @brief   Set Up1 assigned to the egress packet if <UP1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up1                      - UP1 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortUp1Set
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U32           up1
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    if(up1 >= BIT_3)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP1_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        up1);
    return rc;

}

/**
* @internal cpssDxChBrgVlanPortUp1Set function
* @endinternal
*
* @brief   Set Up1 assigned to the egress packet if <UP1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] up1                      - UP1 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUp1Set
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_U32           up1
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortUp1Set);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, up1));

    rc = internal_cpssDxChBrgVlanPortUp1Set(devNum, portNum, up1);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, up1));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortUp1Get function
* @endinternal
*
* @brief   Return the UP1 assigned to the egress packet if <UP1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] up1Ptr                   - (pointer to) UP1 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortUp1Get
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *up1Ptr
)
{
    GT_STATUS   rc;     /* function return code              */
    GT_U32      value;  /* value to write                    */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(up1Ptr);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP1_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);

    *up1Ptr = value;

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortUp1Get function
* @endinternal
*
* @brief   Return the UP1 assigned to the egress packet if <UP1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] up1Ptr                   - (pointer to) UP1 value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortUp1Get
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *up1Ptr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortUp1Get);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, up1Ptr));

    rc = internal_cpssDxChBrgVlanPortUp1Get(devNum, portNum, up1Ptr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, up1Ptr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet function
* @endinternal
*
* @brief   Enable/Disable VLAN Tag1 removing from packets arrived without VLAN Tag1.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
* @param[in] enable                   - GT_FALSE - according to global configuration
*                                      see cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet
*                                      GT_TRUE  - Tag1 removed if original packet arrived without Tag1
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS   rc;   /* return code */
    GT_U32      data; /* HW value */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);



    data = BOOL2BIT_MAC(enable);

    rc = prvCpssDxChWriteTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_REMOVE_VLAN_TAG_1_IF_RX_WITHOUT_TAG_1_E, /* field name */
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        data);

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet function
* @endinternal
*
* @brief   Enable/Disable VLAN Tag1 removing from packets arrived without VLAN Tag1.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
* @param[in] enable                   - GT_FALSE - according to global configuration
*                                      see cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet
*                                      GT_TRUE  - Tag1 removed if original packet arrived without Tag1
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of VLAN Tag1 removing from packets arrived without VLAN Tag1.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE - according to global configuration
*                                      see cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet
*                                      GT_TRUE  - Tag1 removed if original packet arrived without Tag1
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *enablePtr
)
{
    GT_STATUS   rc;   /* return code */
    GT_U32      data; /* HW value */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);



    rc = prvCpssDxChReadTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_REMOVE_VLAN_TAG_1_IF_RX_WITHOUT_TAG_1_E, /* field name */
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(data);

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of VLAN Tag1 removing from packets arrived without VLAN Tag1.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE - according to global configuration
*                                      see cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet
*                                      GT_TRUE  - Tag1 removed if original packet arrived without Tag1
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortRemoveTag0IfRxWithoutTag0EnableSet function
* @endinternal
*
* @brief   Enable/Disable VLAN Tag0 removing from packets arrived without VLAN Tag0.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
* @param[in] enable                   - GT_FALSE - Tag0 can egress even if original packet arrived without Tag0
*                                       GT_TRUE  - Tag0 removed if original packet arrived without Tag0
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortRemoveTag0IfRxWithoutTag0EnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS   rc;   /* return code */
    GT_U32      data; /* HW value */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    data = BOOL2BIT_MAC(enable);

    rc = prvCpssDxChWriteTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP6_30_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_REMOVE_VLAN_TAG_0_IF_RX_WITHOUT_TAG_0_E, /* field name */
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        data);

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortRemoveTag0IfRxWithoutTag0EnableSet function
* @endinternal
*
* @brief   Enable/Disable VLAN Tag0 removing from packets arrived without VLAN Tag0.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
* @param[in] enable                   - GT_FALSE - Tag0 can egress even if original packet arrived without Tag0
*                                       GT_TRUE  - Tag0 removed if original packet arrived without Tag0
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortRemoveTag0IfRxWithoutTag0EnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortRemoveTag0IfRxWithoutTag0EnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgVlanPortRemoveTag0IfRxWithoutTag0EnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortRemoveTag0IfRxWithoutTag0EnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of VLAN Tag0 removing from packets arrived without VLAN Tag0.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
*
* @param[out] enablePtr                - (pointer to)
*                                       GT_FALSE - Tag0 can egress even if original packet arrived without Tag0
*                                       GT_TRUE  - Tag0 removed if original packet arrived without Tag0
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortRemoveTag0IfRxWithoutTag0EnableGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *enablePtr
)
{
    GT_STATUS   rc;   /* return code */
    GT_U32      data; /* HW value */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    rc = prvCpssDxChReadTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP6_30_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_REMOVE_VLAN_TAG_0_IF_RX_WITHOUT_TAG_0_E, /* field name */
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(data);

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanPortRemoveTag0IfRxWithoutTag0EnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of VLAN Tag0 removing from packets arrived without VLAN Tag0.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number
*
* @param[out] enablePtr                - (pointer to)
*                                       GT_FALSE - Tag0 can egress even if original packet arrived without Tag0
*                                       GT_TRUE  - Tag0 removed if original packet arrived without Tag0
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortRemoveTag0IfRxWithoutTag0EnableGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortRemoveTag0IfRxWithoutTag0EnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgVlanPortRemoveTag0IfRxWithoutTag0EnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgVlanIndependentNonFloodVidxEnableSet function
* @endinternal
*
* @brief   Enable/Disable Independent non-flood VIDX
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE: VIDX portlist is always AND'ed with VID portlist
*                                      GT_TRUE:  If packet has VIDX target and VIDX != 0xFFF
*                                      (i.e. not flooded to all ports in the VLAN),
*                                      then the VIDX portlist is the distribution portlist,
*                                      regardless of the VID portlist.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanIndependentNonFloodVidxEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_BOOL          enable
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);



    return prvCpssHwPpSetRegField(devNum,
        PRV_DXCH_REG1_UNIT_EGF_SHT_MAC(devNum).global.SHTGlobalConfigs,
        4,1,BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChBrgVlanIndependentNonFloodVidxEnableSet function
* @endinternal
*
* @brief   Enable/Disable Independent non-flood VIDX
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE: VIDX portlist is always AND'ed with VID portlist
*                                      GT_TRUE:  If packet has VIDX target and VIDX != 0xFFF
*                                      (i.e. not flooded to all ports in the VLAN),
*                                      then the VIDX portlist is the distribution portlist,
*                                      regardless of the VID portlist.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIndependentNonFloodVidxEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIndependentNonFloodVidxEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgVlanIndependentNonFloodVidxEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanIndependentNonFloodVidxEnableGet function
* @endinternal
*
* @brief   Get Independent non-flood VIDX status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: VIDX portlist is always AND'ed with VID portlist
*                                      GT_TRUE:  If packet has VIDX target and VIDX != 0xFFF
*                                      (i.e. not flooded to all ports in the VLAN),
*                                      then the VIDX portlist is the distribution portlist,
*                                      regardless of the VID portlist.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanIndependentNonFloodVidxEnableGet
(
    IN  GT_U8           devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS   rc;     /* function return code              */
    GT_U32      value;  /* value to write                    */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);



    rc = prvCpssHwPpGetRegField(devNum,
        PRV_DXCH_REG1_UNIT_EGF_SHT_MAC(devNum).global.SHTGlobalConfigs,
        4,1,&value);

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgVlanIndependentNonFloodVidxEnableGet function
* @endinternal
*
* @brief   Get Independent non-flood VIDX status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: VIDX portlist is always AND'ed with VID portlist
*                                      GT_TRUE:  If packet has VIDX target and VIDX != 0xFFF
*                                      (i.e. not flooded to all ports in the VLAN),
*                                      then the VIDX portlist is the distribution portlist,
*                                      regardless of the VID portlist.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanIndependentNonFloodVidxEnableGet
(
    IN  GT_U8           devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanIndependentNonFloodVidxEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgVlanIndependentNonFloodVidxEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanFcoeForwardingEnableSet function
* @endinternal
*
* @brief   Set FCoE Forwarding Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which FCoE Forwarding enabled/disabled..
* @param[in] enable                   - FCoE Forwarding Enable
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanFcoeForwardingEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_U16           vlanId,
    IN  GT_BOOL          enable
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwData;  /* HW data     */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);


    hwData = BOOL2BIT_MAC(enable);

    rc = prvCpssDxChWriteTableEntryField(
        devNum,
        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
        vlanId,/* vid */
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FCOE_FORWARDING_EN_E, /* field name */
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        hwData);

    return rc;
}

/**
* @internal cpssDxChBrgVlanFcoeForwardingEnableSet function
* @endinternal
*
* @brief   Set FCoE Forwarding Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which FCoE Forwarding enabled/disabled..
* @param[in] enable                   - FCoE Forwarding Enable
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanFcoeForwardingEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_U16           vlanId,
    IN  GT_BOOL          enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanFcoeForwardingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, enable));

    rc = internal_cpssDxChBrgVlanFcoeForwardingEnableSet(devNum, vlanId, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanFcoeForwardingEnableGet function
* @endinternal
*
* @brief   Return FCoE Forwarding Enable status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which FCoE Forwarding enabled/disabled..
*
* @param[out] enablePtr                - (pointer to) FCoE Forwarding Enable
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanFcoeForwardingEnableGet
(
    IN  GT_U8            devNum,
    IN  GT_U16           vlanId,
    OUT GT_BOOL          *enablePtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwData;  /* HW data     */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);


    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                                        vlanId,/* vid */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FCOE_FORWARDING_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &hwData);

    if(GT_OK != rc)
    {
        return rc;
    }
    *enablePtr = (0 == hwData) ? GT_FALSE : GT_TRUE;
    return rc;
}

/**
* @internal cpssDxChBrgVlanFcoeForwardingEnableGet function
* @endinternal
*
* @brief   Return FCoE Forwarding Enable status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID which FCoE Forwarding enabled/disabled..
*
* @param[out] enablePtr                - (pointer to) FCoE Forwarding Enable
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanFcoeForwardingEnableGet
(
    IN  GT_U8            devNum,
    IN  GT_U16           vlanId,
    OUT GT_BOOL          *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanFcoeForwardingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, enablePtr));

    rc = internal_cpssDxChBrgVlanFcoeForwardingEnableGet(devNum, vlanId, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChBrgVlanUnregisteredIpmEVidxSet function
* @endinternal
*
* @brief   Set Unregistered IPM eVidx Assignment Mode and Value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID.
* @param[in] unregIpmEVidx            - Unregistered IPM eVidx Assignment Mode.
* @param[in] unregIpmEVidx            - Unregistered IPM eVidx Assignment Value.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_OUT_OF_RANGE          - on out-of-range parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanUnregisteredIpmEVidxSet
(
    IN GT_U8                                        devNum,
    IN GT_U16                                       vlanId,
    IN CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_ENT  unregIpmEVidxMode,
    IN GT_U32                                       unregIpmEVidx
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwData;  /* HW data     */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);


    if (unregIpmEVidx >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_CONVERT_UNREG_IPM_EVIDX_MODE_TO_HW_VAL_MAC(
        hwData, unregIpmEVidxMode);

    rc = prvCpssDxChWriteTableEntryField(
        devNum,
        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
        vlanId,/* vid */
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREG_IPM_EVIDX_MODE_E, /* field name */
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChWriteTableEntryField(
        devNum,
        CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
        vlanId,/* vid */
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREG_IPM_EVIDX_E, /* field name */
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        unregIpmEVidx);

    return rc;
}

/**
* @internal cpssDxChBrgVlanUnregisteredIpmEVidxSet function
* @endinternal
*
* @brief   Set Unregistered IPM eVidx Assignment Mode and Value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID.
* @param[in] unregIpmEVidx            - Unregistered IPM eVidx Assignment Mode.
* @param[in] unregIpmEVidx            - Unregistered IPM eVidx Assignment Value.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_OUT_OF_RANGE          - on out-of-range parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanUnregisteredIpmEVidxSet
(
    IN GT_U8                                        devNum,
    IN GT_U16                                       vlanId,
    IN CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_ENT  unregIpmEVidxMode,
    IN GT_U32                                       unregIpmEVidx
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanUnregisteredIpmEVidxSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, unregIpmEVidxMode, unregIpmEVidx));

    rc = internal_cpssDxChBrgVlanUnregisteredIpmEVidxSet(devNum, vlanId, unregIpmEVidxMode, unregIpmEVidx);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, unregIpmEVidxMode, unregIpmEVidx));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanMembersTableIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN Members table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress direction
* @param[in] mode                     - table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanMembersTableIndexingModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      direction,
    IN  CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode
)
{
    GT_STATUS   rc = GT_BAD_PARAM;  /* return code */
    GT_U32      fieldValue;         /* register field value  */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);



    switch(mode)
    {
        case CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E:
            fieldValue = 0;
            break;

        case CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E:
            fieldValue = 1;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if( (CPSS_DIRECTION_INGRESS_E == direction) ||
        (CPSS_DIRECTION_BOTH_E == direction) )
    {
        rc = prvCpssHwPpSetRegField(devNum,
                    PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                                         bridgeEngineConfig.bridgeGlobalConfig1,
                                          26, 1, fieldValue);
        if( GT_OK != rc )
        {
            return rc;
        }
    }

    if( (CPSS_DIRECTION_EGRESS_E == direction) ||
        (CPSS_DIRECTION_BOTH_E == direction) )
    {
        rc = prvCpssHwPpSetRegField(devNum,
                    PRV_DXCH_REG1_UNIT_EGF_SHT_MAC(devNum).
                                          global.SHTGlobalConfigs,
                                           7, 1, fieldValue);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanMembersTableIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN Members table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress direction
* @param[in] mode                     - table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanMembersTableIndexingModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      direction,
    IN  CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanMembersTableIndexingModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, direction, mode));

    rc = internal_cpssDxChBrgVlanMembersTableIndexingModeSet(devNum, direction, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, direction, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanMembersTableIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN Members table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress direction
*
* @param[out] modePtr                  - (pointer to) table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanMembersTableIndexingModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      direction,
    OUT CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   *modePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      fieldValue; /* register field value  */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);



    switch(direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            rc = prvCpssHwPpGetRegField(devNum,
                  PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                                         bridgeEngineConfig.bridgeGlobalConfig1,
                                          26, 1, &fieldValue);
            break;

        case CPSS_DIRECTION_EGRESS_E:
            rc = prvCpssHwPpGetRegField(devNum,
                  PRV_DXCH_REG1_UNIT_EGF_SHT_MAC(devNum).
                                          global.SHTGlobalConfigs,
                                           7, 1, &fieldValue);
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (rc != GT_OK)
    {
        return rc;
    }

    *modePtr = (fieldValue == 0x0) ?
                           CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E :
                           CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E;

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanMembersTableIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN Members table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress direction
*
* @param[out] modePtr                  - (pointer to) table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanMembersTableIndexingModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      direction,
    OUT CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanMembersTableIndexingModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, direction, modePtr));

    rc = internal_cpssDxChBrgVlanMembersTableIndexingModeGet(devNum, direction, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, direction, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanStgIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN table to fetch span state
*         group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress direction
* @param[in] mode                     - table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanStgIndexingModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      direction,
    IN  CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode
)
{
    GT_STATUS   rc = GT_BAD_PARAM;  /* return code */
    GT_U32      fieldValue;         /* register field value  */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);



    switch(mode)
    {
        case CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E:
            fieldValue = 0;
            break;

        case CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E:
            fieldValue = 1;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if( (CPSS_DIRECTION_INGRESS_E == direction) ||
        (CPSS_DIRECTION_BOTH_E == direction) )
    {
        rc = prvCpssHwPpSetRegField(devNum,
                    PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                                         bridgeEngineConfig.bridgeGlobalConfig1,
                                          27, 1, fieldValue);
        if( GT_OK != rc )
        {
            return rc;
        }
    }

    if( (CPSS_DIRECTION_EGRESS_E == direction) ||
        (CPSS_DIRECTION_BOTH_E == direction) )
    {
        rc = prvCpssHwPpSetRegField(devNum,
                    PRV_DXCH_REG1_UNIT_EGF_SHT_MAC(devNum).
                                          global.SHTGlobalConfigs,
                                           6, 1, fieldValue);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanStgIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN table to fetch span state
*         group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress direction
* @param[in] mode                     - table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanStgIndexingModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      direction,
    IN  CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanStgIndexingModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, direction, mode));

    rc = internal_cpssDxChBrgVlanStgIndexingModeSet(devNum, direction, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, direction, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanStgIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN table to fetch span state
*         group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress direction
*
* @param[out] modePtr                  - (pointer to) table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanStgIndexingModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      direction,
    OUT CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   *modePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      fieldValue; /* register field value  */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);



    switch(direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            rc = prvCpssHwPpGetRegField(devNum,
                  PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                                         bridgeEngineConfig.bridgeGlobalConfig1,
                                          27, 1, &fieldValue);
            break;

        case CPSS_DIRECTION_EGRESS_E:
            rc = prvCpssHwPpGetRegField(devNum,
                  PRV_DXCH_REG1_UNIT_EGF_SHT_MAC(devNum).
                                          global.SHTGlobalConfigs,
                                           6, 1, &fieldValue);
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (rc != GT_OK)
    {
        return rc;
    }

    *modePtr = (fieldValue == 0x0) ?
                           CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E :
                           CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E;

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanStgIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN table to fetch span state
*         group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] direction                - ingress/egress direction
*
* @param[out] modePtr                  - (pointer to) table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanStgIndexingModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      direction,
    OUT CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanStgIndexingModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, direction, modePtr));

    rc = internal_cpssDxChBrgVlanStgIndexingModeGet(devNum, direction, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, direction, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanTagStateIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN table to fetch tag state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port supported)
* @param[in] mode                     - table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanTagStateIndexingModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode
)
{
    GT_U32      fieldValue;     /* register field value  */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);


    switch(mode)
    {
        case CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E:
            fieldValue = 0;
            break;

        case CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E:
            fieldValue = 1;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        return prvCpssDxChWriteTableEntryField(devNum,
                    CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E,
                    portNum,
                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                    SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_EUSE_VLAN_TAG_1_FOR_TAG_STATE_E, /* field name */
                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                    fieldValue);
    }

    return prvCpssHwPpSetRegField(devNum,
                    PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).
                     distributor.useVlanTag1ForTagStateReg[(portNum/32)],
                                                (portNum%32), 1, fieldValue);
}

/**
* @internal cpssDxChBrgVlanTagStateIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN table to fetch tag state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port supported)
* @param[in] mode                     - table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanTagStateIndexingModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanTagStateIndexingModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mode));

    rc = internal_cpssDxChBrgVlanTagStateIndexingModeSet(devNum, portNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanTagStateIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN table to fetch tag state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port supported)
*
* @param[out] modePtr                  - (pointer to) table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanTagStateIndexingModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    OUT CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   *modePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      fieldValue; /* register field value  */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);


    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                    CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E,
                    portNum,
                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                    SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_EUSE_VLAN_TAG_1_FOR_TAG_STATE_E, /* field name */
                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                    &fieldValue);
    }
    else
    {
        rc = prvCpssHwPpGetRegField(devNum,
                    PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(devNum).
                     distributor.useVlanTag1ForTagStateReg[(portNum/32)],
                                                (portNum%32), 1, &fieldValue);
    }

    if (rc != GT_OK)
    {
        return rc;
    }

    *modePtr = (fieldValue == 0x0) ?
                           CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VLAN_E :
                           CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_VID1_E;

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanTagStateIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN table to fetch tag state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (CPU port supported)
*
* @param[out] modePtr                  - (pointer to) table access indexing mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanTagStateIndexingModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    OUT CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanTagStateIndexingModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, modePtr));

    rc = internal_cpssDxChBrgVlanTagStateIndexingModeGet(devNum, portNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet function
* @endinternal
*
* @brief   Set the BPE E-Tag Ingress_E-CID_base calculation algorithm
*         for Looped Back Forwarded Bridged Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] forceSrcCidDefault       - GT_TRUE - Ingress_E-CID_base field of E-TAG forced
*                                      to get default value configured by
*                                      cpssDxChBrgVlanBpeTagMcCfgSet (defaultSrcECid).
*                                      GT_FALSE - Ingress_E-CID_base field of E-TAG is pushed VID.
*                                      configured by cpssDxChBrgVlanPortPushedTagValueSet
*                                      (tagValue) per source ePort.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For regular multicast packets E-Tag Ingress_E-CID_base is always S_VID.
*
*/
static GT_STATUS internal_cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          forceSrcCidDefault
)
{
    GT_STATUS   rc; /* return code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    rc = prvCpssDxChWriteTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_HA_EPORT_TABLE_2_FORCE_E_TAG_IE_PID_TO_DEFAULT_E, /* field name */
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        BOOL2BIT_MAC(forceSrcCidDefault));

    return rc;
}

/**
* @internal cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet function
* @endinternal
*
* @brief   Set the BPE E-Tag Ingress_E-CID_base calculation algorithm
*         for Looped Back Forwarded Bridged Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] forceSrcCidDefault       - GT_TRUE - Ingress_E-CID_base field of E-TAG forced
*                                      to get default value configured by
*                                      cpssDxChBrgVlanBpeTagMcCfgSet (defaultSrcECid).
*                                      GT_FALSE - Ingress_E-CID_base field of E-TAG is pushed VID.
*                                      configured by cpssDxChBrgVlanPortPushedTagValueSet
*                                      (tagValue) per source ePort.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For regular multicast packets E-Tag Ingress_E-CID_base is always S_VID.
*
*/
GT_STATUS cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          forceSrcCidDefault
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, forceSrcCidDefault));

    rc = internal_cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet(devNum, portNum, forceSrcCidDefault);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, forceSrcCidDefault));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet function
* @endinternal
*
* @brief   Get the BPE E-Tag Ingress_E-CID_base calculation algorithm
*         for Looped Back Forwarded Bridged Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] forceSrcCidDefaultPtr    - (pointer to)
*                                      GT_TRUE - Ingress_E-CID_base field of E-TAG forced
*                                      to get default value configured by
*                                      cpssDxChBrgVlanBpeTagMcCfgSet (defaultSrcECid).
*                                      GT_FALSE - Ingress_E-CID_base field of E-TAG is pushed VID.
*                                      configured by cpssDxChBrgVlanPortPushedTagValueSet
*                                      (tagValue) per source ePort.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For regular multicast packets E-Tag Ingress_E-CID_base is always S_VID.
*
*/
static GT_STATUS internal_cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *forceSrcCidDefaultPtr
)
{
    GT_STATUS   rc;     /* function return code              */
    GT_U32      value;  /* value to write                    */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(forceSrcCidDefaultPtr);

    rc = prvCpssDxChReadTableEntryField(
        devNum,
        CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP5_HA_EPORT_TABLE_2_FORCE_E_TAG_IE_PID_TO_DEFAULT_E, /* field name */
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        &value);

    if(rc != GT_OK)
    {
        return rc;
    }

    *forceSrcCidDefaultPtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet function
* @endinternal
*
* @brief   Get the BPE E-Tag Ingress_E-CID_base calculation algorithm
*         for Looped Back Forwarded Bridged Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] forceSrcCidDefaultPtr    - (pointer to)
*                                      GT_TRUE - Ingress_E-CID_base field of E-TAG forced
*                                      to get default value configured by
*                                      cpssDxChBrgVlanBpeTagMcCfgSet (defaultSrcECid).
*                                      GT_FALSE - Ingress_E-CID_base field of E-TAG is pushed VID.
*                                      configured by cpssDxChBrgVlanPortPushedTagValueSet
*                                      (tagValue) per source ePort.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For regular multicast packets E-Tag Ingress_E-CID_base is always S_VID.
*
*/
GT_STATUS cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *forceSrcCidDefaultPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, forceSrcCidDefaultPtr));

    rc = internal_cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet(devNum, portNum, forceSrcCidDefaultPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, forceSrcCidDefaultPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanBpeTagMcCfgSet function
* @endinternal
*
* @brief   Set the BPE E-Tag Configuration for Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] eCidOffset               - GT_TRUE - negative, GT_FALSE - positive
* @param[in] eCidOffset               - E-CID Offset negative or positive
*                                      for Multicast packets, field
*                                      {GRP,E-CID_base} calculated as (eVIDX + eCidOffset)
*                                      (APPLICABLE RANGES: 0..0x3FFF)
* @param[in] defaultSrcECid           - default value for Ingress_E-CID_base field of E-TAG  -
*                                      the value that not filtered by source VM filtering.
*                                      Used for all Unicast and some looped back Multicast packets.
*                                      About looped back Multicast packets see
*                                      cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet
*                                      (APPLICABLE RANGES: 0..0xFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_RANGE          - when one of parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanBpeTagMcCfgSet
(
    IN  GT_U8             devNum,
    IN  GT_BOOL           eCidOffsetNegative,
    IN  GT_U32            eCidOffset,
    IN  GT_U32            defaultSrcECid
)
{
    GT_U32 regAddr;     /* the register address */
    GT_U32 value;       /* the value to write   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);


    if (eCidOffset > 0x3FFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (defaultSrcECid > 0xFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).BPEConfigReg1;
    value =
        ((BOOL2BIT_MAC(eCidOffsetNegative) << 26)
         | (eCidOffset << 12) | defaultSrcECid);

    return prvCpssHwPpSetRegField(
        devNum, regAddr, 0 /*offset*/, 27 /*length*/, value);
}

/**
* @internal cpssDxChBrgVlanBpeTagMcCfgSet function
* @endinternal
*
* @brief   Set the BPE E-Tag Configuration for Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] eCidOffset               - GT_TRUE - negative, GT_FALSE - positive
* @param[in] eCidOffset               - E-CID Offset negative or positive
*                                      for Multicast packets, field
*                                      {GRP,E-CID_base} calculated as (eVIDX + eCidOffset)
*                                      (APPLICABLE RANGES: 0..0x3FFF)
* @param[in] defaultSrcECid           - default value for Ingress_E-CID_base field of E-TAG  -
*                                      the value that not filtered by source VM filtering.
*                                      Used for all Unicast and some looped back Multicast packets.
*                                      About looped back Multicast packets see
*                                      cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet
*                                      (APPLICABLE RANGES: 0..0xFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_RANGE          - when one of parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanBpeTagMcCfgSet
(
    IN  GT_U8             devNum,
    IN  GT_BOOL           eCidOffsetNegative,
    IN  GT_U32            eCidOffset,
    IN  GT_U32            defaultSrcECid
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanBpeTagMcCfgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, eCidOffsetNegative, eCidOffset, defaultSrcECid));

    rc = internal_cpssDxChBrgVlanBpeTagMcCfgSet(devNum, eCidOffsetNegative, eCidOffset, defaultSrcECid);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, eCidOffsetNegative, eCidOffset, defaultSrcECid));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanBpeTagMcCfgGet function
* @endinternal
*
* @brief   Get the BPE E-Tag Configuration for Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] eCidOffsetNegativePtr    - (pointer to)GT_TRUE - negative, GT_FALSE - positive
* @param[out] eCidOffsetPtr            - (pointer to)E-CID Offset negative or positive
*                                      for Multicast packets, field
*                                      {GRP,E-CID_base} calculated as (eVIDX + eCidOffset)
* @param[out] defaultSrcECidPtr        - (pointer to)default value for Ingress_E-CID_base field of E-TAG  -
*                                      the value that not filtered by source VM filtering.
*                                      Used for all Unicast and some looped back Multicast packets.
*                                      About looped back Multicast packets see
*                                      cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanBpeTagMcCfgGet
(
    IN  GT_U8             devNum,
    OUT GT_BOOL           *eCidOffsetNegativePtr,
    OUT GT_U32            *eCidOffsetPtr,
    OUT GT_U32            *defaultSrcECidPtr
)
{
    GT_STATUS   rc;     /* return code            */
    GT_U32 regAddr;     /* the register address   */
    GT_U32 value;       /* the value from regiter */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(eCidOffsetNegativePtr);
    CPSS_NULL_PTR_CHECK_MAC(eCidOffsetPtr);
    CPSS_NULL_PTR_CHECK_MAC(defaultSrcECidPtr);

    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).BPEConfigReg1;

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *eCidOffsetNegativePtr = BIT2BOOL_MAC(((value >> 26) & 1));
    *eCidOffsetPtr         = ((value >> 12) & 0x3FFF);
    *defaultSrcECidPtr     = (value & 0xFFF);

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanBpeTagMcCfgGet function
* @endinternal
*
* @brief   Get the BPE E-Tag Configuration for Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] eCidOffsetNegativePtr    - (pointer to)GT_TRUE - negative, GT_FALSE - positive
* @param[out] eCidOffsetPtr            - (pointer to)E-CID Offset negative or positive
*                                      for Multicast packets, field
*                                      {GRP,E-CID_base} calculated as (eVIDX + eCidOffset)
* @param[out] defaultSrcECidPtr        - (pointer to)default value for Ingress_E-CID_base field of E-TAG  -
*                                      the value that not filtered by source VM filtering.
*                                      Used for all Unicast and some looped back Multicast packets.
*                                      About looped back Multicast packets see
*                                      cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanBpeTagMcCfgGet
(
    IN  GT_U8             devNum,
    OUT GT_BOOL           *eCidOffsetNegativePtr,
    OUT GT_U32            *eCidOffsetPtr,
    OUT GT_U32            *defaultSrcECidPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanBpeTagMcCfgGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, eCidOffsetNegativePtr, eCidOffsetPtr, defaultSrcECidPtr));

    rc = internal_cpssDxChBrgVlanBpeTagMcCfgGet(devNum, eCidOffsetNegativePtr, eCidOffsetPtr, defaultSrcECidPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, eCidOffsetNegativePtr, eCidOffsetPtr, defaultSrcECidPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanBpeTagReservedFieldsSet function
* @endinternal
*
* @brief   Set the values for BPE E-Tag Reserved Fields of all packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] reValue                  - value for reserved field called "re-"
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] ingressECidExtValue      - value for field "ingress_E-CID_ext"
*                                      (APPLICABLE RANGES: 0..0xFF)
* @param[in] eCidExtValue             - value for field "E-CID_ext"
*                                      (APPLICABLE RANGES: 0..0xFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_RANGE          - when one of parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanBpeTagReservedFieldsSet
(
    IN  GT_U8             devNum,
    IN  GT_U32            reValue,
    IN  GT_U32            ingressECidExtValue,
    IN  GT_U32            eCidExtValue
)
{
    GT_U32 regAddr;     /* the register address */
    GT_U32 value;       /* the value to write   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);


    if (reValue > 0x3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (ingressECidExtValue > 0xFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (eCidExtValue > 0xFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).BPEConfigReg2;
    value =
        ((reValue << 16) | (ingressECidExtValue << 8) | eCidExtValue);

    return prvCpssHwPpSetRegField(
        devNum, regAddr, 0 /*offset*/, 18 /*length*/, value);
}

/**
* @internal cpssDxChBrgVlanBpeTagReservedFieldsSet function
* @endinternal
*
* @brief   Set the values for BPE E-Tag Reserved Fields of all packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] reValue                  - value for reserved field called "re-"
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] ingressECidExtValue      - value for field "ingress_E-CID_ext"
*                                      (APPLICABLE RANGES: 0..0xFF)
* @param[in] eCidExtValue             - value for field "E-CID_ext"
*                                      (APPLICABLE RANGES: 0..0xFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_RANGE          - when one of parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanBpeTagReservedFieldsSet
(
    IN  GT_U8             devNum,
    IN  GT_U32            reValue,
    IN  GT_U32            ingressECidExtValue,
    IN  GT_U32            eCidExtValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanBpeTagReservedFieldsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, reValue, ingressECidExtValue, eCidExtValue));

    rc = internal_cpssDxChBrgVlanBpeTagReservedFieldsSet(devNum, reValue, ingressECidExtValue, eCidExtValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, reValue, ingressECidExtValue, eCidExtValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanBpeTagReservedFieldsGet function
* @endinternal
*
* @brief   Get the values for BPE E-Tag Reserved Fields of all packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] reValuePtr               - (pointer to)value for reserved field called "re-"
* @param[out] ingressECidExtValuePtr   - (pointer to)value for field "ingress_E-CID_ext"
* @param[out] eCidExtValuePtr          - (pointer to)value for field "E-CID_ext"
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanBpeTagReservedFieldsGet
(
    IN  GT_U8             devNum,
    OUT GT_U32            *reValuePtr,
    OUT GT_U32            *ingressECidExtValuePtr,
    OUT GT_U32            *eCidExtValuePtr
)
{
    GT_STATUS   rc;     /* return code            */
    GT_U32 regAddr;     /* the register address   */
    GT_U32 value;       /* the value from regiter */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(reValuePtr);
    CPSS_NULL_PTR_CHECK_MAC(ingressECidExtValuePtr);
    CPSS_NULL_PTR_CHECK_MAC(eCidExtValuePtr);

    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).BPEConfigReg2;

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *reValuePtr              = ((value >> 16) & 3);
    *ingressECidExtValuePtr  = ((value >> 8) & 0xFF);
    *eCidExtValuePtr         = (value & 0xFF);

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanBpeTagReservedFieldsGet function
* @endinternal
*
* @brief   Get the values for BPE E-Tag Reserved Fields of all packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] reValuePtr               - (pointer to)value for reserved field called "re-"
* @param[out] ingressECidExtValuePtr   - (pointer to)value for field "ingress_E-CID_ext"
* @param[out] eCidExtValuePtr          - (pointer to)value for field "E-CID_ext"
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanBpeTagReservedFieldsGet
(
    IN  GT_U8             devNum,
    OUT GT_U32            *reValuePtr,
    OUT GT_U32            *ingressECidExtValuePtr,
    OUT GT_U32            *eCidExtValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanBpeTagReservedFieldsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, reValuePtr, ingressECidExtValuePtr, eCidExtValuePtr));

    rc = internal_cpssDxChBrgVlanBpeTagReservedFieldsGet(devNum, reValuePtr, ingressECidExtValuePtr, eCidExtValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, reValuePtr, ingressECidExtValuePtr, eCidExtValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanFdbLookupKeyModeSet function
* @endinternal
*
* @brief   Set an FDB Lookup key mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
* @param[in] mode                     - FDB Lookup key mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanFdbLookupKeyModeSet
(
    IN GT_U8                                 devNum,
    IN GT_U16                                vlanId,
    IN CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT mode
)
{
    GT_STATUS   rc;
    GT_U32      hwData;
    GT_U32      fid;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_VLAN_INDEX_CHECK_MAC(devNum, vlanId);

    if (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_CONVERT_FDB_LOOKUP_KEY_MODE_TO_HW_VAL_MAC(hwData, mode);

    rc = prvCpssDxChWriteTableEntryField(devNum,
                      CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
                      vlanId,
                      PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                      SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FDB_LOOKUP_KEY_MODE_E,
                      PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                      hwData);

    /* store FDB Lookup key mode of the vlan into DB according to a vlan's FID */
    if (GT_OK == rc)
    {
        rc = cpssDxChBrgVlanForwardingIdGet(devNum, vlanId, &fid);
        if (rc != GT_OK)
        {
            return rc;
        }
        PRV_CPSS_DXCH_FDB_LOOKUP_KEY_MODE_PER_FID_SET_MAC(devNum, fid, mode);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanFdbLookupKeyModeSet function
* @endinternal
*
* @brief   Set an FDB Lookup key mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN ID
* @param[in] mode                     - FDB Lookup key mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanFdbLookupKeyModeSet
(
    IN GT_U8                                 devNum,
    IN GT_U16                                vlanId,
    IN CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanFdbLookupKeyModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vlanId, mode));

    rc = internal_cpssDxChBrgVlanFdbLookupKeyModeSet(devNum, vlanId, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vlanId, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlan6BytesTagConfigSet function
* @endinternal
*
* @brief   Set global configurations for 6 bytes v-tag.
*         NOTE: not relevant to standard 8 bytes BPE 802.1BR.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] isPortExtender           - is this 'port Extender' or 'Control bridge'
*                                      GT_TRUE - 'port Extender'
*                                      GT_FALSE - 'Control bridge'
* @param[in] lBitInSrcId              - the bit index of 'L bit' in the srcId field passed from ingress pipe to egress pipe.
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] dBitInSrcId              - the bit index of 'D bit' in the srcId field passed from ingress pipe to egress pipe.
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter devNum
* @retval GT_OUT_OF_RANGE          - out of range in value of lBitInSrcId or dBitInSrcId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlan6BytesTagConfigSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  isPortExtender,
    IN GT_U32   lBitInSrcId,
    IN GT_U32   dBitInSrcId
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;             /* register address                */
    GT_U32      startBit; /* start bit in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E);

    if(lBitInSrcId >= BIT_4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(dBitInSrcId >= BIT_4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    startBit = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 26 : 25;

    regAddr =
        PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig;
    /* set isPortExtender */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, startBit + 4, 1, BOOL2BIT_MAC(isPortExtender));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set lBitInSrcId */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, startBit, 4, lBitInSrcId);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr =
        PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig1;

    /* set dBitInSrcId */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 26, 4, dBitInSrcId);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlan6BytesTagConfigSet function
* @endinternal
*
* @brief   Set global configurations for 6 bytes v-tag.
*         NOTE: not relevant to standard 8 bytes BPE 802.1BR.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
* @param[in] isPortExtender           - is this 'port Extender' or 'Control bridge'
*                                      GT_TRUE - 'port Extender'
*                                      GT_FALSE - 'Control bridge'
* @param[in] lBitInSrcId              - the bit index of 'L bit' in the srcId field passed from ingress pipe to egress pipe.
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] dBitInSrcId              - the bit index of 'D bit' in the srcId field passed from ingress pipe to egress pipe.
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter devNum
* @retval GT_OUT_OF_RANGE          - out of range in value of lBitInSrcId or dBitInSrcId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlan6BytesTagConfigSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  isPortExtender,
    IN GT_U32   lBitInSrcId,
    IN GT_U32   dBitInSrcId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlan6BytesTagConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, isPortExtender, lBitInSrcId , dBitInSrcId));

    rc = internal_cpssDxChBrgVlan6BytesTagConfigSet(devNum, isPortExtender, lBitInSrcId , dBitInSrcId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, isPortExtender, lBitInSrcId , dBitInSrcId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlan6BytesTagConfigGet function
* @endinternal
*
* @brief   Get global configurations for 6 bytes v-tag.
*         NOTE: not relevant to standard 8 bytes BPE 802.1BR.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] isPortExtenderPtr        - (pointer to) is this 'port Extender' or 'Control bridge'
*                                      GT_TRUE - 'port Extender'
*                                      GT_FALSE - 'Control bridge'
* @param[out] lBitInSrcIdPtr           - (pointer to) the bit index of 'L bit' in the srcId field passed from ingress pipe to egress pipe.
* @param[out] dBitInSrcIdPtr           - (pointer to) the bit index of 'D bit' in the srcId field passed from ingress pipe to egress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlan6BytesTagConfigGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL  *isPortExtenderPtr,
    OUT GT_U32   *lBitInSrcIdPtr,
    OUT GT_U32   *dBitInSrcIdPtr
)
{
    GT_STATUS   rc;     /* return code            */
    GT_U32 regAddr;     /* the register address   */
    GT_U32 value;       /* the value from register */
    GT_U32      startBit; /* start bit in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    if (!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(isPortExtenderPtr);
    CPSS_NULL_PTR_CHECK_MAC(lBitInSrcIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(dBitInSrcIdPtr);

    startBit = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 26 : 25;

    regAddr =
        PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig;
    /* get isPortExtender */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, startBit + 4, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *isPortExtenderPtr = BIT2BOOL_MAC(value);

    /* get lBitInSrcId */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, startBit , 4, lBitInSrcIdPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr =
        PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig1;

    /* get dBitInSrcId */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 26, 4, dBitInSrcIdPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlan6BytesTagConfigGet function
* @endinternal
*
* @brief   Get global configurations for 6 bytes v-tag.
*         NOTE: not relevant to standard 8 bytes BPE 802.1BR.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] isPortExtenderPtr        - (pointer to) is this 'port Extender' or 'Control bridge'
*                                      GT_TRUE - 'port Extender'
*                                      GT_FALSE - 'Control bridge'
* @param[out] lBitInSrcIdPtr           - (pointer to) the bit index of 'L bit' in the srcId field passed from ingress pipe to egress pipe.
* @param[out] dBitInSrcIdPtr           - (pointer to) the bit index of 'D bit' in the srcId field passed from ingress pipe to egress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlan6BytesTagConfigGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL  *isPortExtenderPtr,
    OUT GT_U32   *lBitInSrcIdPtr,
    OUT GT_U32   *dBitInSrcIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlan6BytesTagConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, isPortExtenderPtr, lBitInSrcIdPtr , dBitInSrcIdPtr));

    rc = internal_cpssDxChBrgVlan6BytesTagConfigGet(devNum, isPortExtenderPtr, lBitInSrcIdPtr , dBitInSrcIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, isPortExtenderPtr, lBitInSrcIdPtr , dBitInSrcIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanBpeTagForceIngressEcidEnableSet
* @endinternal
*
* @brief   Enable/disable the force assignment of the <Ingress E-CID> field
*          of multicast E-tagged packet to be the source ePort’s PCID
*
* @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:       xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                     - device number
* @param[in] enable                     - GT_TRUE  - Force enable
*                                       - GT_FALSE - Force disable
*
* @retval GT_OK                         - on success
* @retval GT_HW_ERROR                   - on hardware error
* @retval GT_BAD_PARAM                  - wrong value in any of the parameter
* @retval GT_NOT_APPLICABLE_DEVICE      - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChBrgVlanBpeTagForceIngressEcidEnableSet
(
    IN  GT_U8          devNum,
    IN  GT_BOOL        enable
)
{
    GT_STATUS   rc;      /* return code */
    GT_32       regAddr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                          CPSS_BOBCAT3_E| CPSS_XCAT3_E | CPSS_AC5_E | CPSS_CAELUM_E | CPSS_BOBCAT2_E |
                                          CPSS_AC3X_E | CPSS_LION2_E| CPSS_ALDRIN_E | CPSS_ALDRIN2_E);

    /* Get the address of HA global configuration register */
    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig;

    /* set <force_Ingress_E-CID> field(15th bit) of the register */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 15, 1, BOOL2BIT_MAC(enable));

    return rc;
}

/**
* @internal cpssDxChBrgVlanBpeTagForceIngressEcidEnableSet
* @endinternal
*
* @brief   Enable/disable the force assignment of the <Ingress E-CID> field
*          of multicast E-tagged packet to be the source ePort’s PCID
*
* @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:       xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                     - device number
* @param[in] portNum                    - port number
* @param[in] enable                     - GT_TRUE  - Force enable
*                                       - GT_FALSE - Force disable
*
* @retval GT_OK                         - on success
* @retval GT_HW_ERROR                   - on hardware error
* @retval GT_BAD_PARAM                  - wrong value in any of the parameter
* @retval GT_NOT_APPLICABLE_DEVICE      - on not applicable device
*
*/
GT_STATUS cpssDxChBrgVlanBpeTagForceIngressEcidEnableSet
(
    IN  GT_U8          devNum,
    IN  GT_BOOL        enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanBpeTagForceIngressEcidEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgVlanBpeTagForceIngressEcidEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal  internal_cpssDxChBrgVlanBpeTagForceIngressEcidEnableGet function
* @endinternal
*
* @brief   Get the state of the Ingress E-CID field of multicast E-tagged packet.
*
*
* @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:       xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - device number
* @param[in] portNum                   - port number
*
* @param[out] enablePtr                - (pointer to)
*                                        GT_TRUE  - Force enable
*                                        GT_FALSE - Force disable
*
* @retval GT_OK                        - on success
* @retval GT_BAD_PTR                   - on NULL pointer
* @retval GT_BAD_PARAM                 - wrong value in any of the parameter
* @retval GT_HW_ERROR                  - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE     - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChBrgVlanBpeTagForceIngressEcidEnableGet
(
    IN  GT_U8            devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS   rc;     /* function return code              */
    GT_U32      value;  /* value to write                    */
    GT_U32      regAddr;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                          CPSS_BOBCAT3_E| CPSS_XCAT3_E | CPSS_AC5_E | CPSS_CAELUM_E | CPSS_BOBCAT2_E |
                                          CPSS_AC3X_E | CPSS_LION2_E| CPSS_ALDRIN_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* Get the address of HA global configuration register */
    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).haGlobalConfig;

    /* Get the value of <force Ingress_E-CID>(15th bit) field of the register*/
    rc = prvCpssHwPpGetRegField(devNum, regAddr,15, 1, &value);

    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal  cpssDxChBrgVlanBpeTagForceIngressEcidEnableGet function
* @endinternal
*
* @brief   Get the state of the Ingress E-CID field of multicast E-tagged packet.
*
*
* @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:       xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - device number
*
* @param[out] enablePtr                - (pointer to)
*                                        GT_TRUE  - Force enable
*                                        GT_FALSE - Force disable
*
* @retval GT_OK                        - on success
* @retval GT_BAD_PTR                   - on NULL pointer
* @retval GT_BAD_PARAM                 - wrong value in any of the parameter
* @retval GT_HW_ERROR                  - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE     - on not applicable device
*
*/
GT_STATUS cpssDxChBrgVlanBpeTagForceIngressEcidEnableGet
(
    IN  GT_U8            devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanBpeTagForceIngressEcidEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgVlanBpeTagForceIngressEcidEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChBrgVlanPortPushedTagUsePhysicalEnableSet function
* @endinternal
*
* @brief   enable/disable use physical port push tag VLAN Id value
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;
*                                  AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] portNum               - physical port number
* @param[in] enable                - enable/disable use physical port push tag VLAN Id
*                                    GT_TRUE - the push tag value is taken from physical port
*                                    GT_FALSE - the push tag value is taken from ePort
*
* @retval GT_OK                        - on success
* @retval GT_BAD_PARAM                 - on bad devNum or portNum
* @retval GT_HW_ERROR                  - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE     - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortPushedTagUsePhysicalEnableSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  enable
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT3_E| CPSS_XCAT3_E | CPSS_AC5_E |
                                          CPSS_CAELUM_E | CPSS_BOBCAT2_E | CPSS_AC3X_E | CPSS_LION2_E|
                                          CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /* Write <use_push_tag_vid>field from HA Physical Port table 2 */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_USE_PHYSICAL_PORT_PUSH_TAG_VID_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        BOOL2BIT_MAC(enable));
    return rc;
}
/**
* @internal cpssDxChBrgVlanPortPushedTagUsePhysicalEnableSet function
* @endinternal
*
* @brief   Enable/disable use physical port push tag VLAN Id value
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;
*                                  AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] portNum               - physical port number
* @param[in] enable                - enable/disable use physical port push tag VLAN Id
*                                    GT_TRUE - the push tag value is taken from physical port
*                                    GT_FALSE - the push tag value is taken from ePort
*
* @retval GT_OK                        - on success
* @retval GT_BAD_PARAM                 - on bad devNum or portNum
* @retval GT_HW_ERROR                  - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE     - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortPushedTagUsePhysicalEnableSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortPushedTagUsePhysicalEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChBrgVlanPortPushedTagUsePhysicalEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortPushedTagUsePhysicalEnableGet function
* @endinternal
*
* @brief   Get the status of use physical port push tag VLAN Id value
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;
*                                  AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] portNum               - physical port number
*
* @param[out] enablePtr            - (pointer to) use physical port push tag VLAN Id value
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on bad devNum or portNum
* @retval GT_BAD_PTR                - on NULL pointer
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortPushedTagUsePhysicalEnableGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                  *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32      value;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT3_E| CPSS_XCAT3_E | CPSS_AC5_E |
                                          CPSS_CAELUM_E | CPSS_BOBCAT2_E | CPSS_AC3X_E | CPSS_LION2_E|
                                          CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* Read <use_push_tag_vid>field from HA Physical Port table 2 */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_USE_PHYSICAL_PORT_PUSH_TAG_VID_EN_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortPushedTagUsePhysicalEnableGet function
* @endinternal
*
* @brief   Get the status of use physical port push tag VLAN Id value
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;
*                                  AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] portNum               - physical port number
*
* @param[out] enablePtr            - (pointer to) use physical port push tag VLAN Id value
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on bad devNum or portNum
* @retval GT_BAD_PTR                - on NULL pointer
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortPushedTagUsePhysicalEnableGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortPushedTagUsePhysicalEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChBrgVlanPortPushedTagUsePhysicalEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortPushedTagPhysicalValueSet function
* @endinternal
*
* @brief   Set the push tag VLAN Id value by mapping to physical target port
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;
*                                  AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum            - device number
* @param[in] portNum           - physical port number
* @param[in] pushTagVidValue   - pushed tag VLAN Id value
*
* @retval GT_OK                        - on success
* @retval GT_OUT_OF_RANGE              - pushTagVidValue not in valid range
* @retval GT_BAD_PARAM                 - on bad devNum or portNum
* @retval GT_HW_ERROR                  - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE     - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortPushedTagPhysicalValueSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U16                   pushTagVidValue
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT3_E| CPSS_XCAT3_E | CPSS_AC5_E |
                                          CPSS_CAELUM_E | CPSS_BOBCAT2_E | CPSS_AC3X_E | CPSS_LION2_E|
                                          CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    if(pushTagVidValue >= BIT_12)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Write <pushed_tag_vlan_id_value>field from HA Physical Port table 2 */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PUSHED_TAG_VLAN_ID_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        pushTagVidValue);
    return rc;
}

/**
* @internal cpssDxChBrgVlanPortPushedTagPhysicalValueSet function
* @endinternal
*
* @brief   Set the push tag VLAN Id value by mapping to physical target port
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;
*                                  AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum            - device number
* @param[in] portNum           - physical port number
* @param[in] pushTagVidValue   - pushed tag VLAN Id value
*
* @retval GT_OK                        - on success
* @retval GT_OUT_OF_RANGE              - pushTagVidValue not in valid range
* @retval GT_BAD_PARAM                 - on bad devNum and portNum
* @retval GT_HW_ERROR                  - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE     - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortPushedTagPhysicalValueSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U16                   pushTagVidValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortPushedTagPhysicalValueSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, pushTagVidValue));

    rc = internal_cpssDxChBrgVlanPortPushedTagPhysicalValueSet(devNum, portNum, pushTagVidValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, pushTagVidValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPortPushedTagPhysicalValueGet function
* @endinternal
*
* @brief   Get the state of push tag VLAN Id value mapped with target physical port
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;
*                                  AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                  - device number
* @param[in] portNum                 - physical port number
*
* @param[out] pushTagVidValuePtr     - (pointer to)pushed tag VLAN Id value
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM               - on bad devNum or portNum
* @retval GT_BAD_PTR                 - on NULL pointer
* @retval GT_HW_ERROR                - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE   - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPortPushedTagPhysicalValueGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_U16                   *pushTagVidValuePtr
)
{
    GT_STATUS   rc;
    GT_U32      value;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT3_E| CPSS_XCAT3_E | CPSS_AC5_E |
                                          CPSS_CAELUM_E | CPSS_BOBCAT2_E | CPSS_AC3X_E | CPSS_LION2_E|
                                          CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(pushTagVidValuePtr);

    /* Read <pushed_tag_vlan_id_value>field from HA Physical Port table 2 */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PUSHED_TAG_VLAN_ID_E, /* field name */
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *pushTagVidValuePtr = (GT_U16)value;

    return rc;
}

/**
* @internal cpssDxChBrgVlanPortPushedTagPhysicalValueGet function
* @endinternal
*
* @brief   Get the state of push tag VLAN Id value mapped with target physical port
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;
*                                  AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                  - device number
* @param[in] portNum                 - physical port number
*
* @param[out] pushTagVidValuePtr     - (pointer to)pushed tag VLAN Id value
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM               - on bad devNum or portNum
* @retval GT_BAD_PTR                 - on NULL pointer
* @retval GT_HW_ERROR                - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE   - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPortPushedTagPhysicalValueGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_U16                   *pushTagVidValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPortPushedTagPhysicalValueGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, pushTagVidValuePtr));

    rc = internal_cpssDxChBrgVlanPortPushedTagPhysicalValueGet(devNum, portNum, pushTagVidValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, pushTagVidValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChBrgVlanPort8BTagInfoSet function
* @endinternal
*
* @brief   Set 8B tag parsing related configuration
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] direction                - The direction.
*                                       (APPLICABLE VALUES: ingress/egress/both)
*                                       (APPLICABLE DEVICES: Ironman)
* @param[in] tpidEntryIndex           - TPID table entry index (APPLICABLE RANGES: 0...7)
* @param[in] parseConfigPtr           - (pointer to)8B tag parsing configuration
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - illegal extractedValueOffset
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPort8BTagInfoSet
(
    IN GT_U8                                        devNum,
    IN  CPSS_DIRECTION_ENT                          direction,
    IN GT_U32                                       tpidEntryIndex,
    IN CPSS_DXCH_BRG_VLAN_8B_TAG_PARSE_CONFIG_STC   *parseConfigPtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;      /* register address */
    GT_U32      regVal = 0;   /* register value */
    GT_U32      data;
    GT_U32      wordsArr[2]; /* common values for ingress and egress */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT3_E| CPSS_XCAT3_E | CPSS_AC5_E |
                                          CPSS_CAELUM_E | CPSS_BOBCAT2_E | CPSS_AC3X_E | CPSS_LION2_E|
                                          CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(parseConfigPtr);

    if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        /* the device not supports the 'direction' so ignore the input from the
           caller and force 'ingress' to allow common code with sip_6_30 device */
        direction = CPSS_DIRECTION_INGRESS_E;
    }

    switch(direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
        case CPSS_DIRECTION_EGRESS_E:
        case CPSS_DIRECTION_BOTH_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(direction);
    }

    if(tpidEntryIndex > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(parseConfigPtr->extractedValueOffset > 4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    wordsArr[0] = 0;
    /* 8B Tag Extracted Value Offset */
    data = parseConfigPtr->extractedValueOffset;
    U32_SET_FIELD_MAC(wordsArr[0], 1, 3, data);

    /* 8B Tag TCI Value[15:0] */
    data = TCI_GET_LOW_16_MAC(parseConfigPtr->tagTciValue);
    U32_SET_FIELD_MAC(wordsArr[0], 4, 16, data);

    /* 8B Tag TCI Value[16:47] */
    wordsArr[1] = TCI_GET_HIGH_32_MAC(parseConfigPtr->tagTciValue);

    if(direction == CPSS_DIRECTION_INGRESS_E ||
       direction == CPSS_DIRECTION_BOTH_E)
    {
        /* Enable 8B tag1 parsing -- TTI format for bit 0*/
        data = BOOL2BIT_MAC(parseConfigPtr->enable);
        U32_SET_FIELD_MAC(wordsArr[0], 0, 1, data);

        if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        {
            /* <hsrTag> used as HSR tag  -- TTI format for bit 20 */
            data = BOOL2BIT_MAC(parseConfigPtr->hsrTag);
            U32_SET_FIELD_MAC(wordsArr[0], 20, 1, data);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            TTI.VLANAssignment.ingrTPIDEnhClasification[tpidEntryIndex];
        rc = prvCpssHwPpWriteRegister(devNum, regAddr, wordsArr[0]);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            TTI.VLANAssignment.ingrTPIDEnhClasificationExt[tpidEntryIndex];
        rc = prvCpssHwPpWriteRegister(devNum, regAddr, wordsArr[1]);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* 8B Tag TCI Mask[15:0] */
        regVal = TCI_GET_LOW_16_MAC(parseConfigPtr->tagTciMask);
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            TTI.VLANAssignment.ingrTPIDEnhClasificationExt2[tpidEntryIndex];
        rc = prvCpssHwPpWriteRegister(devNum, regAddr, regVal);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* 8B Tag TCI Mask[16:47] */
        regVal =  TCI_GET_HIGH_32_MAC(parseConfigPtr->tagTciMask);
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            TTI.VLANAssignment.ingrTPIDEnhClasificationExt3[tpidEntryIndex];
        rc = prvCpssHwPpWriteRegister(devNum, regAddr, regVal);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        {
            /* bit 20 is not relevant to the HA unit , and we restore it to 0 */
            U32_SET_FIELD_MAC(wordsArr[0], 20, 1, 0);
        }
    }

    if(direction == CPSS_DIRECTION_EGRESS_E ||
       direction == CPSS_DIRECTION_BOTH_E)
    {
        if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        {
            /* <hsrTag> used as HSR tag  -- HA format for bit 0 */
            data = BOOL2BIT_MAC(parseConfigPtr->hsrTag);
            U32_SET_FIELD_MAC(wordsArr[0], 0, 1, data);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            HA.egrTPIDEnhClasificationExt1[tpidEntryIndex];
        rc = prvCpssHwPpWriteRegister(devNum, regAddr, wordsArr[0]);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            HA.egrTPIDEnhClasificationExt2[tpidEntryIndex];
        rc = prvCpssHwPpWriteRegister(devNum, regAddr, wordsArr[1]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanPort8BTagInfoSet function
* @endinternal
*
* @brief   Set 8B tag parsing related configuration
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] direction                - The direction.
*                                       (APPLICABLE VALUES: ingress/egress/both)
*                                       (APPLICABLE DEVICES: Ironman)
* @param[in] tpidEntryIndex           - TPID table entry index (APPLICABLE RANGES: 0...7)
* @param[in] parseConfigPtr           - (pointer to)8B tag parsing configuration
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - illegal extractedValueOffset
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPort8BTagInfoSet
(
    IN GT_U8                                        devNum,
    IN  CPSS_DIRECTION_ENT                          direction,
    IN GT_U32                                       tpidEntryIndex,
    IN CPSS_DXCH_BRG_VLAN_8B_TAG_PARSE_CONFIG_STC   *parseConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPort8BTagInfoSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, direction , tpidEntryIndex, parseConfigPtr));

    rc = internal_cpssDxChBrgVlanPort8BTagInfoSet(devNum, direction , tpidEntryIndex, parseConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, direction , tpidEntryIndex, parseConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanPort8BTagInfoGet function
* @endinternal
*
* @brief   Get 8B tag parsing related configuration
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] direction             - The direction.
*                                    (APPLICABLE VALUES: ingress/egress)
*                                    (APPLICABLE DEVICES: Ironman)
* @param[in] tpidEntryIndex        - TPID table entry index (APPLICABLE RANGES: 0...7)
* @param[out] parseConfigPtr       - (pointer to)8B tag parsing configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanPort8BTagInfoGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DIRECTION_ENT                          direction,
    IN  GT_U32                                      tpidEntryIndex,
    OUT CPSS_DXCH_BRG_VLAN_8B_TAG_PARSE_CONFIG_STC  *parseConfigPtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;      /* register address */
    GT_U32      regVal;       /* register value */
    GT_U32      data;
    GT_U32      wordsArr[2]; /* common values for ingress and egress */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT3_E| CPSS_XCAT3_E | CPSS_AC5_E |
                                          CPSS_CAELUM_E | CPSS_BOBCAT2_E | CPSS_AC3X_E | CPSS_LION2_E|
                                          CPSS_ALDRIN_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(parseConfigPtr);

    if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        /* the device not supports the 'direction' so ignore the input from the
           caller and force 'ingress' to allow common code with sip_6_30 device */
        direction = CPSS_DIRECTION_INGRESS_E;
    }

    switch(direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
        case CPSS_DIRECTION_EGRESS_E:
        /*case CPSS_DIRECTION_BOTH_E: NOT support 'both' on the 'Get' APIs */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(direction);
    }

    if(tpidEntryIndex > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(direction == CPSS_DIRECTION_INGRESS_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            TTI.VLANAssignment.ingrTPIDEnhClasification[tpidEntryIndex];
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &wordsArr[0]);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            TTI.VLANAssignment.ingrTPIDEnhClasificationExt[tpidEntryIndex];
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &wordsArr[1]);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* 8B Tag TCI Mask[15:0] */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            TTI.VLANAssignment.ingrTPIDEnhClasificationExt2[tpidEntryIndex];
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &regVal);
        if (rc != GT_OK)
        {
            return rc;
        }
        parseConfigPtr->tagTciMask[5] = (GT_U8) regVal;
        parseConfigPtr->tagTciMask[4] = (GT_U8) (regVal >> 8);

        /* 8B Tag TCI Mask[16:47] */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            TTI.VLANAssignment.ingrTPIDEnhClasificationExt3[tpidEntryIndex];
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &regVal);
        if (rc != GT_OK)
        {
            return rc;
        }

        parseConfigPtr->tagTciMask[3] = (GT_U8) regVal;
        parseConfigPtr->tagTciMask[2] = (GT_U8) (regVal >> 8);
        parseConfigPtr->tagTciMask[1] = (GT_U8) (regVal >> 16);
        parseConfigPtr->tagTciMask[0] = (GT_U8) (regVal >> 24);

        /* Enable 8B tag1 parsing -- only in the TTI unit */
        data = U32_GET_FIELD_MAC(wordsArr[0], 0, 1);
        parseConfigPtr->enable = BIT2BOOL_MAC(data);

        if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        {
            /* <hsrTag> used as HSR tag -- in the TTI unit bit 20 */
            data = U32_GET_FIELD_MAC(wordsArr[0], 20, 1);
            parseConfigPtr->hsrTag = BIT2BOOL_MAC(data);
        }
    }
    else  /*CPSS_DIRECTION_EGRESS_E*/
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            HA.egrTPIDEnhClasificationExt1[tpidEntryIndex];
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &wordsArr[0]);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            HA.egrTPIDEnhClasificationExt2[tpidEntryIndex];
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &wordsArr[1]);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        {
            /* <hsrTag> used as HSR tag -- in the HA unit bit 0 */
            data = U32_GET_FIELD_MAC(wordsArr[0], 0, 1);
            parseConfigPtr->hsrTag = BIT2BOOL_MAC(data);
        }

        parseConfigPtr->enable = GT_FALSE;/* no such configuration in the HA */
    }

    /* 8B Tag Extracted Value Offset */
    data = U32_GET_FIELD_MAC(wordsArr[0], 1, 3);
    parseConfigPtr->extractedValueOffset = (GT_U8) data;

    /* 8B Tag TCI Value[15:0] */
    data = U32_GET_FIELD_MAC(wordsArr[0], 4, 16);
    parseConfigPtr->tagTciValue[5] = (GT_U8) data;
    parseConfigPtr->tagTciValue[4] = (GT_U8) (data >> 8);

    parseConfigPtr->tagTciValue[3] = (GT_U8)  wordsArr[1];
    parseConfigPtr->tagTciValue[2] = (GT_U8) (wordsArr[1] >> 8);
    parseConfigPtr->tagTciValue[1] = (GT_U8) (wordsArr[1] >> 16);
    parseConfigPtr->tagTciValue[0] = (GT_U8) (wordsArr[1] >> 24);

    return GT_OK;
}

/**
* @internal cpssDxChBrgVlanPort8BTagInfoGet function
* @endinternal
*
* @brief   Get 8B tag parsing related configuration
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] direction             - The direction.
*                                    (APPLICABLE VALUES: ingress/egress)
*                                    (APPLICABLE DEVICES: Ironman)
* @param[in] tpidEntryIndex        - TPID table entry index (APPLICABLE RANGES: 0...7)
* @param[out] parseConfigPtr       - (pointer to)8B tag parsing configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanPort8BTagInfoGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DIRECTION_ENT                          direction,
    IN  GT_U32                                      tpidEntryIndex,
    OUT CPSS_DXCH_BRG_VLAN_8B_TAG_PARSE_CONFIG_STC  *parseConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanPort8BTagInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, direction , tpidEntryIndex, parseConfigPtr));

    rc = internal_cpssDxChBrgVlanPort8BTagInfoGet(devNum, direction , tpidEntryIndex, parseConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, direction , tpidEntryIndex, parseConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgVlanMruCommandSet function
* @endinternal
*
* @brief   Set the packet Command for packets that exceeds VLAN MRU value
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] packetCmd             - the packet command. valid values:
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or packetCmd
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* Note: Default packetCmd is hard drop
*/
static GT_STATUS internal_cpssDxChBrgVlanMruCommandSet
(
    IN GT_U8                         devNum,
    IN CPSS_PACKET_CMD_ENT           packetCmd
)
{
    GT_STATUS rc;           /* return code    */
    GT_U32    regAddr;      /* value to write */
    GT_U32    packetCmdVal;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).bridgeEngineConfig.bridgeCommandConfig0;

    switch(packetCmd)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            packetCmdVal=0;
            break;
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
            packetCmdVal=1;
            break;
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            packetCmdVal=2;
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E:
            packetCmdVal=3;
            break;
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            packetCmdVal=4;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 12, 3, packetCmdVal);

    return rc;
}

/**
* @internal cpssDxChBrgVlanMruCommandSet function
* @endinternal
*
* @brief   Set the packet Command for packets that exceeds VLAN MRU value
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] packetCmd             - the packet command. valid values:
*                                      CPSS_PACKET_CMD_FORWARD_E
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device or packetCmd
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* Note: Default packetCmd is hard drop
*/
GT_STATUS cpssDxChBrgVlanMruCommandSet
(
    IN GT_U8                         devNum,
    IN CPSS_PACKET_CMD_ENT           packetCmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanMruCommandSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetCmd));

    rc = internal_cpssDxChBrgVlanMruCommandSet(devNum, packetCmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetCmd));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChBrgVlanMruCommandGet function
* @endinternal
*
* @brief   Get the packet Command for packets that exceeds VLAN MRU value
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
*
* @param[out] packetCmdPtr         - (pointer to) the packet command.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on unexpected hardware value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgVlanMruCommandGet
(
    IN    GT_U8                      devNum,
    OUT   CPSS_PACKET_CMD_ENT        *packetCmdPtr
)
{
    GT_STATUS   rc;      /* return value */
    GT_U32      hwData;  /* value from hw */
    GT_U32      regAddr; /* value to write */

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(packetCmdPtr);

    regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).bridgeEngineConfig.bridgeCommandConfig0;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 12, 3, &hwData);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(hwData)
    {
        case 0:
            *packetCmdPtr = CPSS_PACKET_CMD_FORWARD_E;
            break;
        case 1:
            *packetCmdPtr = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
            break;
        case 2:
            *packetCmdPtr=CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;
        case 3:
            *packetCmdPtr=CPSS_PACKET_CMD_DROP_HARD_E;
            break;
        case 4:
            *packetCmdPtr=CPSS_PACKET_CMD_DROP_SOFT_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChBrgVlanMruCommandGet function
* @endinternal
*
* @brief   Get the packet Command for packets that exceeds VLAN MRU value
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
*
* @param[out] packetCmdPtr         - (pointer to) the packet command.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on unexpected hardware value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgVlanMruCommandGet
(
    IN    GT_U8                      devNum,
    OUT   CPSS_PACKET_CMD_ENT        *packetCmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgVlanMruCommandGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetCmdPtr));

    rc = internal_cpssDxChBrgVlanMruCommandGet(devNum, packetCmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetCmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

