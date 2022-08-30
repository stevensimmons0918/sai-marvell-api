/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

#include "cpssHalCounter.h"
#include "cpssHalDevice.h"


/**
* @internal cpssHalCncBlockClientEnableAndBindSet function
* @endinternal
*
* @brief   The function binds/unbinds the selected client to/from a counter block.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devId                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
* @param[in] updateEnable             - the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more than one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalCncBlockClientEnableAndBindSet
(
    IN  GT_U8                               devId,
    IN  GT_U32                              blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT            client,
    IN GT_BOOL                              enable,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT    format
)
{
    GT_STATUS   status;
    GT_U32      i;
    GT_BOOL     updateEnable;
    GT_U64      indexRangesBmp;
    GT_U8       cpssDevNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        /* Multiple ACL clients share same CNC block due to limited CNC resources.
         * Skipping below check for ACL clients */
        if ((GT_TRUE==enable) &&
            (client < CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E ||
             client > CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E))
        {
            /*check that no one took this block before*/
            for (i=0; i<CPSS_DXCH_CNC_CLIENT_LAST_E; i++)
            {
                if (client==(CPSS_DXCH_CNC_CLIENT_ENT)i)
                {
                    continue;
                }
                status = cpssDxChCncBlockClientEnableGet(cpssDevNum, blockNum,
                                                         (CPSS_DXCH_CNC_CLIENT_ENT)i, &updateEnable);
                if ((GT_OK==status)&&(GT_TRUE==updateEnable))
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "The block is already owned by client"
                          "client:%d, blockNum:%d\n",
                          i, blockNum);
                    return GT_BAD_STATE;
                }
            }
        }

        status = cpssDxChCncBlockClientEnableSet(cpssDevNum, blockNum, client, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to bind client to CNC block "
                  "client:%d, blockNum:%d\n",
                  client, blockNum);

            return status;
        }

        if (GT_TRUE==enable)
        {

            indexRangesBmp.l[0] = (1<<index);
            indexRangesBmp.l[1] = 0x0;

            status = cpssDxChCncBlockClientRangesSet(cpssDevNum, blockNum, client,
                                                     &indexRangesBmp);
            if (GT_OK != status)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " cpssDxChCncBlockClientRangesSet failed with err : %d on device :%d\n ",
                      status, cpssDevNum);
                return status;
            }

            status = cpssDxChCncCounterFormatSet(cpssDevNum, blockNum, format);
            if (GT_OK != status)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " cpssDxChCncCounterFormatSet failed with err : %d on device :%d\n ", status,
                      cpssDevNum);
                return status;
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssHalCncCounterClearByReadEnableGet function
* @endinternal
*
* @brief   The function gets clear by read mode status of device.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in]  devId                   - device number
* @param[out] enablePtr               - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalCncCounterClearByReadEnableGet
(
    IN  GT_U8                               devId,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS status;

    status = cpssDxChCncCounterClearByReadEnableGet(devId, enablePtr);
    if (GT_OK != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssDxChCncCounterClearByReadEnableGet failed with err : %d on device :%d\n ",
              status, devId);
        return status;
    }
    return GT_OK;
}

/**
* @internal cpssHalCncCounterGet function
* @endinternal
*
* @brief   The function gets the counter contents
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devId                    - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] counterIdx               - counter  in the block
*                                      valid range see in datasheet of specific device.
* @param[in] format                   - CNC counter HW format
*                                      relevant only for Lion2 and above
*
* @param[out] pktCnt                  - (pointer to) packet counter value.
* @param[out] counterPtr              - (pointer to) byte counter value.
* @param[out] maxVal                  - (pointer to) maximum counter value.

* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalCncCounterGet
(
    IN  GT_U32                              devId,
    IN  GT_U32                              blockNum,
    IN  GT_U32                              counterIdx,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT    format,
    OUT uint64_t                              *pktCnt,
    OUT uint64_t                              *byteCnt,
    OUT uint64_t                              *maxVal
)
{
    GT_STATUS                  rc = GT_OK;
    CPSS_DXCH_CNC_COUNTER_STC  cnt;
    GT_U8                    cpssDevNum  = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        memset(&cnt, 0, sizeof(cnt));
        if ((rc = cpssDxChCncCounterGet(cpssDevNum, blockNum, counterIdx,
                                        format, &cnt)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChCncCounterGet failed with err : %d\n ", rc);
            return rc;

        }
        if (byteCnt)
        {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            *byteCnt += ((uint64_t)cnt.byteCount.l[1]<<32)|cnt.byteCount.l[0];
#else
            *byteCnt += ((uint64_t)cnt.byteCount.l[0]<<32)|cnt.byteCount.l[1];
#endif
        }

        if (pktCnt)
        {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            *pktCnt += ((uint64_t)cnt.packetCount.l[1]<<32)|cnt.packetCount.l[0];
#else
            *pktCnt += ((uint64_t)cnt.packetCount.l[0]<<32)|cnt.packetCount.l[1];
#endif
        }

        if (maxVal)
        {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            *maxVal += ((uint64_t)cnt.maxValue.l[1]<<32)|cnt.maxValue.l[0];
#else
            *maxVal += ((uint64_t)cnt.maxValue.l[0]<<32)|cnt.maxValue.l[1];
#endif
        }
    }

    return rc;
}

GT_STATUS cpssHalCncVlanClientIndexModeSet
(
    IN  GT_U32                             devId,
    IN  CPSS_DXCH_CNC_CLIENT_ENT          vlanClient,
    IN  CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT indexMode
)

{
    GT_STATUS                  rc = GT_OK;
    GT_U8                    cpssDevNum  = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssDxChCncVlanClientIndexModeSet(cpssDevNum, vlanClient, indexMode);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChCncVlanClientIndexModeSet %d failed with err : %d\n ", vlanClient,
                  rc);
            return rc;
        }
    }
    return rc;
}

GT_STATUS cpssHalCncClientByteCountModeSet
(
    IN  GT_U32                             devId,
    IN  CPSS_DXCH_CNC_CLIENT_ENT          vlanClient,
    IN  CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT countMode
)

{
    GT_STATUS                  rc = GT_OK;
    GT_U8                    cpssDevNum  = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssDxChCncClientByteCountModeSet(cpssDevNum, vlanClient, countMode);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChCncClientByteCountModeSet %d failed with err : %d\n ", vlanClient,
                  rc);
            return rc;
        }
    }
    return rc;
}
