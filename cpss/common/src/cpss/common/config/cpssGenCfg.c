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
* @file cpssGenCfg.c
*
* @brief CPSS generic configuration functions.
*
* @version   3
********************************************************************************
*/
#ifdef CHX_FAMILY
#define CPSS_LOG_IN_MODULE_ENABLE
#endif /*CHX_FAMILY*/

#include <cpss/common/config/private/prvCpssCommonConfigLog.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define DEVICE_NOT_EXISTS_CNS 0xFF

/**
* @internal internal_cpssPpCfgNextDevGet function
* @endinternal
*
* @brief   Return the number of the next existing device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
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
static GT_STATUS internal_cpssPpCfgNextDevGet
(
    IN  GT_U8 devNum,
    OUT GT_U8 *nextDevNumPtr
)
{
    GT_U8 num;

    /* device should be in the allowed range but not necessary exist */
    if ((devNum > PRV_CPSS_MAX_PP_DEVICES_CNS) && (devNum != DEVICE_NOT_EXISTS_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (nextDevNumPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if (devNum == 0xFF)
        num = 0;
    else
        num = (GT_U8)(devNum+1);

    for (;(NULL!=cpssSharedGlobalVarsPtr)&&num<PRV_CPSS_MAX_PP_DEVICES_CNS; num++)
    {
        if (PRV_CPSS_PP_CONFIG_ARR_MAC[num] != NULL)
        {
            *nextDevNumPtr = num;
            return GT_OK;
        }
    }

    /* no more devices found */
    *nextDevNumPtr = DEVICE_NOT_EXISTS_CNS;
    return /* it's not error for log */ GT_NO_MORE;
}

/**
* @internal cpssPpCfgNextDevGet function
* @endinternal
*
* @brief   Return the number of the next existing device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
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
GT_STATUS cpssPpCfgNextDevGet
(
    IN  GT_U8 devNum,
    OUT GT_U8 *nextDevNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPpCfgNextDevGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, nextDevNumPtr));

    rc = internal_cpssPpCfgNextDevGet(devNum, nextDevNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, nextDevNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPpCfgGetNextInDevBmp function
* @endinternal
*
* @brief   Get the next device from the set of devices specified by bitmap.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] startDevNum           - device number to start from. For the first one  should be 0xFF.
* @param[in] devNumBitmapPtr       - pointer to bitmap of device numbers.
*
* @retval  device number of the next device or 0xFF when no devices more
*/
GT_U8 prvCpssPpCfgGetNextInDevBmp
(
    IN  GT_U32 *devNumBitmapPtr,
    IN  GT_U8  startDevNum
)
{
    GT_U8  devNum;
    GT_U32 word;
    GT_U32 offset;
    GT_U32 bitsNum;
    GT_U32 bitmap;

    /* increment devNum */
    devNum = (startDevNum >= 0xFF) ? 0 : (startDevNum + 1);
    /* look for the next */
    while (devNum < PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        offset = (devNum % 32);
        word   = devNumBitmapPtr[devNum / 32];

        if (word & (1 << offset))
        {
            /* found bit */
            return devNum;
        }

        if ((word & (0xFFFFFFFF << offset)) == 0)
        {
            /* word reminder is empty - pass to the next word */
            devNum += 32;
            devNum -= (devNum % 32);
            continue;
        }

        for (bitsNum = ((32 - offset) / 2); (bitsNum > 0); bitsNum /= 2)
        {
            bitmap = ((1 << bitsNum) - 1);
            if ((word & (bitmap << offset)) == 0)
            {
                /* skip zeros in bitmap */
                devNum += bitsNum;
                continue;
            }
        }

        /* try next bit */
        devNum ++;
    }
    return 0xFF;
}

/**
* @internal prvCpssPpCfgUnlockDevBmp function
* @endinternal
*
* @brief   Unlock set of devices specified by bitmap.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNumBitmapPtr       - pointer to bitmap of device numbers.
* @param[in] lockType              - number of next device after devNum.
*
* @retval - none.
*/
GT_VOID prvCpssPpCfgUnlockDevBmp
(
    IN  GT_U32 *devNumBitmapPtr,
    IN  GT_U32 lockType
)
{
    GT_U8 devNum;

    /* fix compilation error - not used when compiled with API_LOCK_PROTECTION=N */
    GT_UNUSED_PARAM(lockType);

    for (devNum = prvCpssPpCfgGetNextInDevBmp(devNumBitmapPtr, 0xFF);
          (devNum != 0xFF);
          devNum = prvCpssPpCfgGetNextInDevBmp(devNumBitmapPtr, devNum))
    {
        CPSS_API_UNLOCK_MAC(devNum, lockType);
    }

}

/**
* @internal prvCpssPpCfgLockAndCheckDevBmp function
* @endinternal
*
* @brief   Lock and check set of devices specified by bitmap.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNumBitmapPtr       - pointer to bitmap of device numbers.
* @param[in] lockType              - number of next device after devNum.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - some of given devices not exist in DB - so no devices locked
*/
GT_STATUS prvCpssPpCfgLockAndCheckDevBmp
(
    IN  GT_U32 *devNumBitmapPtr,
    IN  GT_U32 lockType
)
{
    GT_U8 devNum;

    for (devNum = prvCpssPpCfgGetNextInDevBmp(devNumBitmapPtr, 0xFF);
          (devNum != 0xFF);
          devNum = prvCpssPpCfgGetNextInDevBmp(devNumBitmapPtr, devNum))
    {
        CPSS_API_LOCK_MAC(devNum, lockType);
    }

    for (devNum = prvCpssPpCfgGetNextInDevBmp(devNumBitmapPtr, 0xFF);
          (devNum != 0xFF);
          devNum = prvCpssPpCfgGetNextInDevBmp(devNumBitmapPtr, devNum))
    {
        if (PRV_CPSS_PP_CONFIG_ARR_MAC[devNum] == NULL)
        {
            prvCpssPpCfgUnlockDevBmp(devNumBitmapPtr, lockType);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}


