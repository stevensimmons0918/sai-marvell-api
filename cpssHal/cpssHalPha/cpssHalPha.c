/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalPha.c
*
* @brief Private API implementation which can be used in XPS layer.
*
* @version   01
********************************************************************************
*/

#include "cpssHalPha.h"
#include "cpssHalDevice.h"
#include <cpssHalDev.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
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
)
{
    GT_STATUS status = GT_OK;
    GT_U32    cpssDevNum;
    GT_U32    cpssPortNum;
    CPSS_CSCD_PORT_TYPE_ENT portType;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, analyzerPort);
    cpssPortNum = xpsGlobalPortToPortnum(devId, analyzerPort);

    if (enable)
    {
        portType = CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E;
    }
    else
    {
        portType = CPSS_CSCD_PORT_NETWORK_E;
    }

    status = cpssDxChCscdPortTypeSet(cpssDevNum, cpssPortNum,
                                     CPSS_PORT_DIRECTION_TX_E, portType);
    if (status!= GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChCscdPortTypeSet fail:%d", status);
        return status;
    }

    return status;
}

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
)
{
    GT_STATUS status = GT_OK;
    int       devNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {

        status = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(devNum,
                                                                 analyzerIndex,
                                                                 erspanEntry);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet fail:%d", status);
            return status;
        }
    }

    return status;

}

GT_STATUS cpssHalPhaThreadInfoGet
(
    GT_U8                devId,
    GT_U32               analyzerIndex,
    CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC *erspanEntry
)
{
    GT_STATUS status = GT_OK;
    int       devNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet(devNum,
                                                                 analyzerIndex,
                                                                 erspanEntry);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet fail:%d", status);
            return status;
        }
    }

    return status;

}


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
)
{
    GT_STATUS status = GT_OK;
    int       devNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {

        status = cpssDxChPhaPortThreadIdSet(devNum, ePort, enable,
                                            analyzerId);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPhaPortThreadIdSet fail:%d", status);
            return status;
        }

    }

    return status;
}

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
)
{
    GT_STATUS status = GT_OK;
    int       devNum;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC commonInfo;
    CPSS_DXCH_PHA_THREAD_INFO_UNT    extInfo;

    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    extInfo.notNeeded = 0;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        if ((cpssHalDevPPFamilyGet(devNum) == CPSS_PP_FAMILY_DXCH_FALCON_E) ||
            (cpssHalDevPPFamilyGet(devNum) == CPSS_PP_FAMILY_DXCH_AC5X_E))
        {

            status = cpssDxChPhaInit(devNum, GT_FALSE, CPSS_DXCH_PHA_FW_IMAGE_ID_DEFAULT_E);
            if (status!= GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPhaInit %d", status);
                return status;
            }

            if (cpssHalDevPPFamilyGet(devNum) == CPSS_PP_FAMILY_DXCH_FALCON_E)
            {
                status = cpssDxChPhaThreadIdEntrySet(devNum,
                                                     CPSSHAL_ERSPAN_IPV4_PHA_THREAD_ID, &commonInfo,
                                                     CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_II_SAME_DEV_IPV4_E,
                                                     &extInfo);
                if (status!= GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChPhaThreadIdEntrySet fail:%d", status);
                    return status;
                }

                status = cpssDxChPhaThreadIdEntrySet(devNum,
                                                     CPSSHAL_ERSPAN_IPV6_PHA_THREAD_ID, &commonInfo,
                                                     CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_II_SAME_DEV_IPV6_E,
                                                     &extInfo);
                if (status!= GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChPhaThreadIdEntrySet fail:%d", status);
                    return status;
                }
            }
        }
    }

    return status;
}

GT_STATUS cpssHalPhaTargetPortEntrySet
(
    GT_U8                devNum,
    GT_PHYSICAL_PORT_NUM portNum,
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  infoType,
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_STATUS status = GT_OK;
    status = cpssDxChPhaTargetPortEntrySet(devNum, portNum, infoType, portInfoPtr);
    if (status!= GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPhaTargetPortEntrySet fail:%d", status);
        return status;
    }

    return status;
}

GT_STATUS cpssHalPhaSourcePortEntrySet
(
    GT_U8                devNum,
    GT_PHYSICAL_PORT_NUM portNum,
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  infoType,
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_STATUS status = GT_OK;
    status = cpssDxChPhaSourcePortEntrySet(devNum, portNum, infoType, portInfoPtr);
    if (status!= GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPhaSourcePortEntrySet fail:%d", status);
        return status;
    }

    return status;
}
