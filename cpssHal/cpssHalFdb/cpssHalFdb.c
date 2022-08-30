/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalFdb.c
*
* @brief Private API implementation for CPSS FDB Table Read, Write, Update which can be used in XPS layer.
*
* @version   01
********************************************************************************
*/

#include "cpssHalFdb.h"
#include "cpssHalUtil.h"
#include "xpsL3.h"
#include "xpsFdb.h"
#include "xpsLock.h"
#include "xpsCommon.h"
#include "xpsInternal.h"
#include "openXpsEnums.h"
#include "openXpsInterface.h"
#include "cpssHalDevice.h"
#include "xpsPort.h"
#include "xpsScope.h"
#include "cpssDriverPpHw.h"
#include "cpssDxChBrgGen.h"
#include "gtOsTimer.h"

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbRouting.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>




GT_PORT_GROUPS_BMP                     gPortGroupsBmp       = 0;
#define NOT_INIT_CNS    0xFFFFFFFF


#if TODO_FBD
/*  1. need to sync read/write/entry for All FDB Entry Types
 *
 */
#endif
/**
* @internal cpssHalBrgFdbMacEntrySet function
* @endinternal
*
* @brief   Create new or update existing entry in Hardware MAC address table
* @param[in] devId         - Device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr              - (pointer to) mac table
* @param[in] fdbAccessMode        - access mode to the FDB table
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - aging timeout exceeds hardware limitation.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalBrgFdbMacEntrySet
(
    int           devId,
    XPS_FDB_ENTRY_TYPE_ENT    fdbEntryType,
    void *macEntryPtr,
    GT_U8         fdbAccessMode
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32  index; /* fdb index */
    GT_U32 bankIndex = 0;
    GT_PORT_GROUPS_BMP portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    CPSS_MAC_ENTRY_EXT_STC      dxChMacEntry;
    GT_U8 devNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        if (fdbAccessMode == PRV_XPS_BRG_FDB_ACCESS_MODE_BY_INDEX_E)
        {
            /* allow to set the entry by index */
            /* calculate and find the index */
            rc = cpssHalBrgFdbMacEntryIndexFind(devNum, fdbEntryType, macEntryPtr, &index,
                                                &bankIndex);
            if (rc != GT_FULL)
            {
                if (GT_OK != rc && GT_NOT_FOUND != rc)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "[XPS]: cpssHalBrgFdbMacEntryIndexFind FAILED, rc = [%d]", rc);
                    return rc;
                }

                rc = cpssHalBrgFdbMacEntryHwWrite(devNum, index, GT_FALSE, fdbEntryType,
                                                  macEntryPtr);
                if (GT_OK != rc)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "[XPS]: cpssHalBrgFdbMacEntryHwWrite FAILED, rc = [%d] devNum %d index %d", rc,
                          devNum, index);
                    return rc;
                }

            }
            else
            {
                /*
                 * NOTE: GT_FULL means that BUCKET is FULL --> the operation would
                 * have failed on 'by messages' !!!
                 * fall through to do the 'by message' action
                 * */
                rc = GT_OK;
            }
        }
        else
        {
            /* convert key data into device specific format */
            rc = cpssHalConvertGenericToDxChMacEntry(devNum, fdbEntryType, macEntryPtr,
                                                     &dxChMacEntry);
            if (GT_OK != rc)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "[XPS]: cpssHalConvertGenericToDxChMacEntry FAILED, rc = [%d]", rc);
                return rc;
            }
            //portGroupsBmp = (PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == GT_FALSE) ? CPSS_PORT_GROUP_UNAWARE_MODE_CNS : gPortGroupsBmp;
            rc = cpssDxChBrgFdbPortGroupMacEntrySet(devNum, portGroupsBmp,  &dxChMacEntry);
            if (GT_OK != rc)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "[XPS]: cpssDxChBrgFdbMacEntrySet FAILED, rc = [%d]", rc);
                return rc;
            }
        }
    }

    return rc;
}

/**
* @internal cpssHalBrgFdbMacEntryGet function
* @endinternal
*
* @brief   Get existing entry in Hardware MAC address table
*
* @param[in] devId             - Device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryKeyPtr           - (pointer to) mac entry key
* @param[out] macEntryPtr              - (pointer to) mac table entry
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - aging timeout exceeds hardware limitation.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalBrgFdbMacEntryGet
(
    int                       devId,
    XPS_FDB_ENTRY_TYPE_ENT    fdbEntryType,
    void                      *macEntryKeyPtr,
    void                      *macEntryPtr
)
{
    GT_STATUS           rc    = GT_OK;
    GT_U32              index = 0, bankIndex = 0;
    GT_BOOL             valid = GT_FALSE;
    GT_BOOL             skip  = GT_FALSE;
    GT_BOOL             aged  = GT_FALSE;
    GT_HW_DEV_NUM       hwDev   = 0;

    GT_U32 numberOfportGroups = XPS_GET_PORT_NUMBER_OF_PORT_GROUPS(devId);
    bool usePortGroupBmp = PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devId);
    if (numberOfportGroups && usePortGroupBmp == GT_FALSE)
    {
        GT_U32      jj;
        rc = GT_FAIL;

        /***************************************************/
        /* lookup in all the port groups for the FDB entry */
        /***************************************************/

        for (jj = 0; jj < numberOfportGroups; jj++)
        {
            rc = cpssHalBrgFdbMacEntryIndexFind(devId, fdbEntryType, macEntryKeyPtr, &index,
                                                &bankIndex);
            if (rc == GT_NOT_FOUND || rc == GT_FULL)
            {
                /* try next port group */
                continue;
            }
            else /* (rc != GT_OK) || (rc == GT_OK) */
            {
                break;
            }
        }

        if (rc == GT_NOT_FOUND || rc == GT_FULL || (jj == numberOfportGroups))
        {
            rc = GT_NOT_FOUND;
        }
        else if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[XPS]: cpssHalBrgFdbMacEntryIndexFind 1 FAILED, rc = [%d]", rc);
        }

        if (rc == GT_OK)
        {
            /* read MAC entry */
            rc = cpssHalBrgFdbMacEntryRead(devId, index, &valid, &skip, &aged, &hwDev,
                                           fdbEntryType, macEntryPtr);
            if (GT_OK != rc)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "[XPS]: cpssHalBrgFdbMacEntryRead 1 FAILED, rc = [%d]", rc);
            }
        }

        return rc;
    }

    /* find the proper index */
    rc = cpssHalBrgFdbMacEntryIndexFind(devId, fdbEntryType, macEntryKeyPtr, &index,
                                        &bankIndex);
    if (rc == GT_NOT_FOUND || rc == GT_FULL)
    {
        return GT_NOT_FOUND;

    }
    else if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "[XPS]: cpssHalBrgFdbMacEntryIndexFind FAILED, rc = [%d]", rc);
        return rc;
    }

    /* read MAC entry */
    rc = cpssHalBrgFdbMacEntryRead(devId, index, &valid, &skip, &aged, &hwDev,
                                   fdbEntryType, macEntryPtr);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "[XPS]: cpssHalBrgFdbMacEntryRead FAILED, rc = [%d]", rc);

        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssHalBrgFdbMacEntryIndexFind_MultiHash function
* @endinternal
*
* @brief   for CRC multi hash mode.
*         function calculate hash index for this mac address , and then start to read
*         from HW the entries to find an existing entry that match the key.
* @param[in] devId             - Device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryKeyPtr           - (pointer to) Fdb mac
*       Entry
*
* @param[out] indexPtr                 - (pointer to) :
*                                      when GT_OK (found) --> index of the entry
*                                      when GT_NOT_FOUND (not found) --> first index that can be used
*                                      when GT_FULL (not found) --> index first index
*                                      on other return value --> not relevant
*
* @param[out] bankIndexPtr         -  bank index. Can be used reference IPv6 Data Entry's Bank in the ipv6 Address Fdb Entry
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found, but bucket is not FULL
* @retval GT_FULL                  - entry not found, but bucket is FULL
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssHalBrgFdbMacEntryIndexFind_MultiHash
(
    int                      devId,
    XPS_FDB_ENTRY_TYPE_ENT   fdbEntryType,
    void                     *macEntryKeyPtr,
    GT_U32                   *indexPtr,
    GT_U32                   *bankIndexPtr
)
{
    GT_STATUS           rc = GT_OK;
    GT_U32              index = 0;
    GT_BOOL             valid;
    GT_BOOL             skip;
    GT_BOOL             aged;
    GT_HW_DEV_NUM       hwDev;
    GT_U32      ii;
    GT_U32      firstEmptyIndex = NOT_INIT_CNS;
    GT_U32      numOfBanks;
    GT_U32      skipBanks = 1;
    CPSS_MAC_ENTRY_EXT_KEY_STC  dxChMacEntryKey;
    GT_U32       crcMultiHashArr[16];
    CPSS_MAC_ENTRY_EXT_STC      dxChMacEntry;
    XPS_FDB_ENTRY_TYPE_ENT   fdbEntryTypeTemp = fdbEntryType;


    GT_PORT_GROUPS_BMP portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    //  portGroupsBmp = (PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == GT_FALSE) ? CPSS_PORT_GROUP_UNAWARE_MODE_CNS : gPortGroupsBmp;

    //Use Address as a key for the data Entry
    if (fdbEntryType == XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E)
    {
        fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
    }

    numOfBanks = PRV_CPSS_DXCH_PP_MAC(devId)->bridge.fdbHashParams.numOfBanks;

    /* convert key data into device specific format */
    rc = cpssHalConvertGenericToDxChMacEntryKey(devId, fdbEntryType, macEntryKeyPtr,
                                                &dxChMacEntryKey);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "[XPS]: cpssHalConvertGenericToDxChMacEntryKey FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChBrgFdbHashCrcMultiResultsCalc(devId, &dxChMacEntryKey,
                                               0,
                                               numOfBanks,
                                               crcMultiHashArr);

    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "[XPS]: cpssDxChBrgFdbHashCrcMultiResultsCalc FAILED, rc = [%d]", rc);

        return rc;
    }

    if ((fdbEntryType == XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E) &&
        (PRV_CPSS_SIP_6_CHECK_MAC(devId)))
    {
        /* IPv6 UC 'Address' entry must be in even bankId */
        skipBanks = 2;
    }

    for (ii = 0; ii < numOfBanks; ii += skipBanks)
    {
        index = crcMultiHashArr[ii];

        /* read MAC entry */
        rc = cpssDxChBrgFdbPortGroupMacEntryRead(devId, portGroupsBmp, index, &valid,
                                                 &skip, &aged, &hwDev, &dxChMacEntry);

        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[XPS]: cpssHalBrgFdbMacEntryRead FAILED, rc = [%d]", rc);

            return rc;
        }

        if (valid /*&& skip == GT_FALSE*/) /* in multi hash - not care about 'skip' bit !*/
        {
            /* used entry */
            if ((0 == cpssHalBrgFdbMacKeyCompare(devId, fdbEntryType, macEntryKeyPtr,
                                                 &dxChMacEntry.key)))
            {
                /* Finding Data Entry using Ipv6 Address Entry's DataBankNumber*/
                if (fdbEntryTypeTemp == XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E)
                {
                    if (PRV_CPSS_SIP_6_CHECK_MAC(devId) == GT_FALSE)
                    {
                        *bankIndexPtr = dxChMacEntry.fdbRoutingInfo.nextHopDataBankNumber;
                        *indexPtr = crcMultiHashArr[*bankIndexPtr];
                    }
                    else
                    {
                        /*Sip6 is not supported yet*/
                    }
                }
                else
                {
                    *indexPtr = index;
                    *bankIndexPtr = ii;
                }
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                      "[XPS]: cpssHalBrgFdbMacKeyCompare is success for device=%d and index=%d",
                      devId, index);
                return GT_OK;

            }
        }
        else
        {
            /* UN-used entry */
            if (firstEmptyIndex == NOT_INIT_CNS)
            {
                if ((fdbEntryType == XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E) &&
                    (PRV_CPSS_SIP_6_CHECK_MAC(devId)))
                {
                    /* Entry following IPv6 UC 'Address' entry must also be empty */
                    /* read MAC entry */
                    rc = cpssDxChBrgFdbPortGroupMacEntryRead(devId, portGroupsBmp,
                                                             crcMultiHashArr[ii + 1], &valid, &skip, &aged, &hwDev, &dxChMacEntry);

                    if (GT_OK != rc)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "[XPS]: cpssHalBrgFdbMacEntryRead_II FAILED, rc = [%d]", rc);

                        return rc;
                    }

                    if (valid == GT_FALSE)
                    {
                        firstEmptyIndex = index;
                        *bankIndexPtr = ii;
                    }
                }
                else
                {
                    firstEmptyIndex = index;
                    *bankIndexPtr = ii;
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                          "[XPS]: cpssHalBrgFdbMacKeyCompare FAILED for device=%d and index=%d", devId,
                          index);
                }
            }
        }
    }

    if (firstEmptyIndex == NOT_INIT_CNS)
    {
        /* all entries are used */
        /* the returned index is of 'first' */
        *indexPtr = index;
        rc = GT_FULL;
    }
    else
    {
        /* the returned index is of first empty entry */
        *indexPtr = firstEmptyIndex;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "[XPS]: cpssHalBrgFdbMacEntryIndexFind_MultiHash is returning GT_NOT_FOUND for device=%d and got index=%d ",
              devId, index);
        rc = GT_NOT_FOUND;
    }

    return rc;
}

/**
* @internal cpssHalBrgFdbMacEntryHashCalc function
* @endinternal
*
* @brief   calculate the hash index for the key
*         NOTE:
*         see also function cpssHalBrgFdbMacEntryHashCalc(...)
*         that find actual place according to actual HW capacity
* @param[in] devId             - Device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryKeyPtr           - (pointer to) mac entry key
*
* @param[out] indexPtr                 - (pointer to) index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalBrgFdbMacEntryHashCalc
(
    int                       devId,
    XPS_FDB_ENTRY_TYPE_ENT    fdbEntryType,
    void                      *macEntryKeyPtr,
    GT_U32                    *indexPtr
)
{
    GT_STATUS   rc = GT_OK;

    CPSS_MAC_ENTRY_EXT_KEY_STC  dxChMacEntryKey;

    /* convert key data into device specific format */
    rc = cpssHalConvertGenericToDxChMacEntryKey(devId, fdbEntryType, macEntryKeyPtr,
                                                &dxChMacEntryKey);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "[XPS]: cpssHalConvertGenericToDxChMacEntryKey FAILED, rc = [%d]", rc);
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChBrgFdbHashCalc(devId, &dxChMacEntryKey, indexPtr);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "[XPS]: cpssDxChBrgFdbHashCalc FAILED, rc = [%d]", rc);
    }

    return rc;
}

/**
* @internal cpssHalBrgFdbMacEntryIndexFind function
* @endinternal
*
* @brief   function calculate hash index for this mac address , and then start to read
*         from HW the entries to find an existing entry that match the key.
* @param devId                       - Device Id
* @param[in] fdbEntryType            - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr           - (pointer to) mac entry key
* @param[out] indexPtr                 - (pointer to) :
*                                      when GT_OK (found) --> index of the entry
*                                      when GT_NOT_FOUND (not found) --> first index that can be used
*                                      when GT_FULL (not found) --> index od start of bucket
*                                      on other return value --> not relevant
* @param[out] bankIndexPtr         -  bank index. Can be used reference IPv6 Data Entry's Bank in the ipv6 Address Fdb Entry
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found, but bucket is not FULL
* @retval GT_FULL                  - entry not found, but bucket is FULL
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssHalBrgFdbMacEntryIndexFind
(
    int                    devId,
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType,
    void                   *macEntryKeyPtr,
    GT_U32                 *indexPtr,
    GT_U32                 *bankIndexPtr
)
{
    GT_STATUS           rc;
    GT_U32              index;
    GT_BOOL             valid;
    GT_BOOL             skip;
    GT_BOOL             aged;
    GT_HW_DEV_NUM       hwDev;
    GT_U32      ii;
    CPSS_MAC_ENTRY_EXT_STC    dxChMacEntry;
    GT_U32      firstEmptyIndex = NOT_INIT_CNS;
    GT_PORT_GROUPS_BMP portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    //portGroupsBmp = (PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == GT_FALSE) ? CPSS_PORT_GROUP_UNAWARE_MODE_CNS : gPortGroupsBmp;

    if (PRV_CPSS_DXCH_PP_MAC(devId)->bridge.fdbHashParams.hashMode ==
        CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E)
    {
        return cpssHalBrgFdbMacEntryIndexFind_MultiHash(devId, fdbEntryType,
                                                        macEntryKeyPtr, indexPtr, bankIndexPtr);
    }

    /* calculate the hash index */
    rc = cpssHalBrgFdbMacEntryHashCalc(devId, fdbEntryType, macEntryKeyPtr, &index);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "[XPS]: cpssHalBrgFdbMacEntryHashCalc FAILED, rc = [%d]", rc);

        return rc;
    }

    for (ii = 0; ii < 4 /* assume length of lookup*/; ii++)
    {

        /* read MAC entry */
        rc = cpssDxChBrgFdbPortGroupMacEntryRead(devId, portGroupsBmp, (index + ii),
                                                 &valid, &skip, &aged, &hwDev, &dxChMacEntry);

        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[XPS]: cpssHalBrgFdbMacEntryIndexFind FAILED, rc = [%d]", rc);

            return rc;
        }

        if (valid && skip == GT_FALSE)
        {
            /* used entry */
            if ((0 == cpssHalBrgFdbMacKeyCompare(devId, fdbEntryType, macEntryKeyPtr,
                                                 &dxChMacEntry.key)))
            {
                *indexPtr = index + ii;
                return GT_OK;
            }
        }
        else
        {
            /* UN-used entry */
            if (firstEmptyIndex == NOT_INIT_CNS)
            {
                firstEmptyIndex = index + ii;
            }
        }
    }

    if (firstEmptyIndex == NOT_INIT_CNS)
    {
        /* all entries are used */
        /* the returned index is of start of bucket */
        *indexPtr = index;
        return GT_FULL;
    }
    else
    {
        /* the returned index is of first empty entry */
        *indexPtr = firstEmptyIndex;
        return GT_NOT_FOUND;
    }
}


/**
* @internal cpssHalBrgFdbMacEntryDelete function
* @endinternal
*
* @brief   Delete an old entry in Hardware MAC address table
*
* @param[in] devId             - Device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryKeyPtr           - (pointer to) key parameters of the mac entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalBrgFdbMacEntryDelete
(
    int                       devId,
    XPS_FDB_ENTRY_TYPE_ENT    fdbEntryType,
    void                      *macEntryKeyPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_PORT_GROUPS_BMP portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    CPSS_MAC_ENTRY_EXT_KEY_STC  dxChMacEntryKey;
    GT_U8 devNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        //portGroupsBmp = (PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == GT_FALSE) ? CPSS_PORT_GROUP_UNAWARE_MODE_CNS : gPortGroupsBmp;
        /* convert key data into device specific format */
        rc = cpssHalConvertGenericToDxChMacEntryKey(devNum, fdbEntryType,
                                                    macEntryKeyPtr, &dxChMacEntryKey);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[XPS]: cpssHalConvertGenericToDxChMacEntryKey FAILED, rc = [%d]", rc);
            return rc;
        }
        rc = cpssDxChBrgFdbPortGroupMacEntryDelete(devNum, portGroupsBmp,
                                                   &dxChMacEntryKey);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[XPS]: cpssDxChBrgFdbPortGroupMacEntryDelete FAILED, rc = [%d]", rc);
            return rc;
        }

    }

    return rc;
}


/**
* @internal cpssHalBrgFdbMacEntryHwWrite function
* @endinternal
*
* @brief   Write the new entry in Hardware MAC address table in
*          specified index into a specific CPSS device
*
* @param[in] CPSS Dev Num             - CPSS device number
* @param[in] index                    - hw mac entry index
* @param[in] skip                     - entry  control
*                                      GT_TRUE - used to "skip" the entry ,
*                                      the HW will treat this entry as "not used"
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,saCommand,daCommand
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalBrgFdbMacEntryHwWrite
(
    GT_U8                         cpssDevNum,
    GT_U32                        index,
    GT_BOOL                       skip,
    XPS_FDB_ENTRY_TYPE_ENT        fdbEntryType,
    void                          *macEntryPtr
)
{
    GT_PORT_GROUPS_BMP portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_STATUS   rc = GT_OK;
    CPSS_MAC_ENTRY_EXT_STC dxChMacEntry;
    GT_U32 nextHopDataBankNumber = 0;

    /* convert key data into device specific format */
    rc = cpssHalConvertGenericToDxChMacEntry(cpssDevNum, fdbEntryType, macEntryPtr,
                                             &dxChMacEntry);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "[XPS]: cpssHalConvertGenericToDxChMacEntry FAILED, rc = [%d]", rc);
        return rc;
    }
    // portGroupsBmp = (PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == GT_FALSE) ? CPSS_PORT_GROUP_UNAWARE_MODE_CNS : gPortGroupsBmp;

    /* Specific to IpV6 Host FDB*/
    if ((fdbEntryType == XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E) ||
        (fdbEntryType == XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E))
    {
        /* This API is not upported for SIP5 programming IPv6 Host Address and Data*/
        if (PRV_CPSS_SIP_6_CHECK_MAC(cpssDevNum) == GT_FALSE)
        {
            return GT_FAIL;
        }

        nextHopDataBankNumber = 0;

        /* Used for Ipv6 where the IPv6 Data Entry referenced in IPv6 address entry for SIP 5*/
        dxChMacEntry.fdbRoutingInfo.nextHopDataBankNumber = nextHopDataBankNumber;
    }

    rc = cpssDxChBrgFdbPortGroupMacEntryWrite(cpssDevNum, portGroupsBmp, index,
                                              skip, &dxChMacEntry);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "[XPS]: cpssDxChBrgFdbPortGroupMacEntryWrite FAILED, rc = [%d]", rc);
        return rc;
    }

    return rc;
}

/**
* @internal cpssHalBrgFdbMacEntryWrite function
* @endinternal
*
* @brief   Write the new entry in Hardware MAC address table in
*          specified index
*
* @param[in] devId                    - device Id
* @param[in] index                    - hw mac entry index
* @param[in] skip                     - entry  control
*                                      GT_TRUE - used to "skip" the entry ,
*                                      the HW will treat this entry as "not used"
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,saCommand,daCommand
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalBrgFdbMacEntryWrite
(
    int                           devId,
    GT_U32                        index,
    GT_BOOL                       skip,
    XPS_FDB_ENTRY_TYPE_ENT        fdbEntryType,
    void                          *macEntryPtr
)
{
    xpsLockTake(XP_LOCKINDEX_XPS_FDB_LOCK);
    GT_STATUS   rc = GT_OK;
    GT_U8                    devNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        rc = cpssHalBrgFdbMacEntryHwWrite(devNum, index, skip, fdbEntryType,
                                          macEntryPtr);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalBrgFdbMacEntryHwWrite FAILED, rc = [%d], devNum : %d index %d", rc,
                  devNum, index);
            xpsLockRelease(XP_LOCKINDEX_XPS_FDB_LOCK);
            return rc;
        }
    }
    xpsLockRelease(XP_LOCKINDEX_XPS_FDB_LOCK);
    return rc;
}

/**
* @internal cpssHalBrgFdbMacEntryWrite_withBankIndex function
* @endinternal
*
* @brief   Write the new entry in Hardware MAC address table in specified index.
*
* @param[in] devId            - device Id
* @param[in] index                   - hw mac entry index
* @param[in] bankIndex               - bank index
* @param[in] skip                    - entry  control
*                                      GT_TRUE - used to "skip" the entry ,
*                                      the HW will treat this entry as "not used"
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,saCommand,daCommand
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalBrgFdbMacEntryWrite_withBankIndex
(
    int                           devId,
    GT_U32                        index,
    GT_U32                        bankIndex,
    GT_BOOL                       skip,
    XPS_FDB_ENTRY_TYPE_ENT        fdbEntryType,
    void                          *macEntryPtr
)
{
    GT_PORT_GROUPS_BMP portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_STATUS   rc = GT_OK;
    CPSS_MAC_ENTRY_EXT_STC      dxChMacEntry;
    GT_U32                                  nextHopDataBankNumber = 0;
    GT_U8 devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        /* convert key data into device specific format */
        rc = cpssHalConvertGenericToDxChMacEntry(devNum, fdbEntryType, macEntryPtr,
                                                 &dxChMacEntry);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[XPS]: cpssHalConvertGenericToDxChMacEntry FAILED, rc = [%d]", rc);
            return rc;
        }

        // portGroupsBmp = (PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == GT_FALSE) ? CPSS_PORT_GROUP_UNAWARE_MODE_CNS : gPortGroupsBmp;

        /* Specific to IpV6 Host FDB*/
        if (fdbEntryType == XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E)
        {
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE)
            {
                nextHopDataBankNumber = bankIndex;
            }
            else
            {
                nextHopDataBankNumber = 0;
            }

            /* Used for Ipv6 where the IPv6 Data Entry referenced in IPv6 address entry for SIP 5*/
            dxChMacEntry.fdbRoutingInfo.nextHopDataBankNumber = nextHopDataBankNumber;
        }

        rc = cpssDxChBrgFdbPortGroupMacEntryWrite(devNum, portGroupsBmp, index, skip,
                                                  &dxChMacEntry);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[XPS]: cpssDxChBrgFdbPortGroupMacEntryWrite FAILED, rc = [%d]", rc);
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssHalBrgFdbMacEntryRead function
* @endinternal
*
* @brief   Reads the new entry in Hardware MAC address table from specified index.
*
* @param[in] index                    - hw mac entry index
*
* @param[out] validPtr                 - (pointer to) is entry valid
* @param[out] skipPtr                  - (pointer to) is entry skip control
* @param[out] agedPtr                  - (pointer to) is entry aged
* @param[out] hwDevNumPtr              - (pointer to) is HW device number associated with the entry
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[out] macEntryPtr              - (pointer to) extended Mac table entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - index out of range
*/
GT_STATUS cpssHalBrgFdbMacEntryRead
(
    int                           devId,
    GT_U32                        index,
    GT_BOOL                       *validPtr,
    GT_BOOL                       *skipPtr,
    GT_BOOL                       *agedPtr,
    GT_HW_DEV_NUM                 *hwDevNumPtr,
    XPS_FDB_ENTRY_TYPE_ENT        fdbEntryType,
    void                          *macEntryPtr
)
{
    GT_STATUS   rc = GT_OK;
    CPSS_MAC_ENTRY_EXT_STC      dxChMacEntry;
    GT_PORT_GROUPS_BMP portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U8       devNum;

    //portGroupsBmp = (PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == GT_FALSE) ? CPSS_PORT_GROUP_UNAWARE_MODE_CNS :  gPortGroupsBmp;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        rc = cpssDxChBrgFdbPortGroupMacEntryRead(devNum, portGroupsBmp, index, validPtr,
                                                 skipPtr, agedPtr, hwDevNumPtr, &dxChMacEntry);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "fdb entry get FAILED for device=%d index=%d, rc = [%d]", devNum, index, rc);

        }
    }


    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb mac entry read failed");
        return rc;
    }

    /* convert key data from device specific format */
    return cpssHalConvertCpssToGenericMacEntry(devId, &dxChMacEntry, fdbEntryType,
                                               macEntryPtr);
}

/**
* @internal cpssHalBrgFdbMacEntryInvalidate function
* @endinternal
*
* @brief   Invalidate an entry in Hardware MAC address table in specified index.
*         the invalidation done by resetting to first word of the entry
* @param[in] index                    - hw mac entry index.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_OUT_OF_RANGE          - index out of range
*/
GT_STATUS cpssHalBrgFdbMacEntryInvalidate
(
    int       devId,
    GT_U32    index
)
{
    GT_STATUS   rc = GT_OK;
    GT_PORT_GROUPS_BMP portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U8       devNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        rc = cpssDxChBrgFdbPortGroupMacEntryInvalidate(devNum, portGroupsBmp, index);

        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[XPS]: cpssDxChBrgFdbPortGroupMacEntryInvalidate FAILED for device=%d index=%d, rc = [%d]",
                  devNum, index, rc);
            return rc;
        }
    }

    return rc;
}

/**
* @internal cpssHalConvertGenericToDxChMacEntryKey function
* @endinternal
*
* @brief   Convert generic MAC entry key into device specific MAC entry key
*
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryKeyPtr           - (pointer to) MAC entry key parameters
* @param[in] macEntryKeyPtr           - (pointer to) MAC entry
*        Type
*
* @param[out] dxChMacEntryKeyPtr       - (pointer to) DxCh MAC entry key parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalConvertGenericToDxChMacEntryKey
(
    int                        devId,
    XPS_FDB_ENTRY_TYPE_ENT     fdbEntryType,
    void                       *macEntryKeyPtr,
    CPSS_MAC_ENTRY_EXT_KEY_STC *dxChMacEntryKeyPtr
)
{
    cpssOsMemSet(dxChMacEntryKeyPtr, 0, sizeof(*dxChMacEntryKeyPtr));

    /* convert entry type into device specific format */
    switch (fdbEntryType)
    {
        case XPS_FDB_ENTRY_TYPE_MAC_ADDR_E:
            dxChMacEntryKeyPtr->entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            dxChMacEntryKeyPtr->entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            dxChMacEntryKeyPtr->entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E;
            break;
        case XPS_FDB_ENTRY_TYPE_IPV4_UC_E:
            dxChMacEntryKeyPtr->entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E:
            dxChMacEntryKeyPtr->entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E:
            dxChMacEntryKeyPtr->entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E;
            break;


        case XPS_FDB_ENTRY_TYPE_MAC_ADDR_FID_VID1_E:
            dxChMacEntryKeyPtr->entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E;
            break;
        case XPS_FDB_ENTRY_TYPE_IPV4_MCAST_FID_VID1_E:
            dxChMacEntryKeyPtr->entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E;
            break;
        case XPS_FDB_ENTRY_TYPE_IPV6_MCAST_FID_VID1_E:
            dxChMacEntryKeyPtr->entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_FID_VID1_E;
            break;


        default:
            return GT_BAD_PARAM;
    }


    /* convert key data into device specific format */
    if (XPS_FDB_ENTRY_TYPE_MAC_ADDR_E == fdbEntryType ||
        XPS_FDB_ENTRY_TYPE_MAC_ADDR_FID_VID1_E == fdbEntryType)
    {
        xpsFdbEntry_t *fdbEntryPtr = (xpsFdbEntry_t *)macEntryKeyPtr;

        cpssOsMemCpy(dxChMacEntryKeyPtr->key.macVlan.macAddr.arEther,
                     &(fdbEntryPtr->macAddr),
                     sizeof(dxChMacEntryKeyPtr->key.macVlan.macAddr));
        dxChMacEntryKeyPtr->key.macVlan.vlanId = fdbEntryPtr->vlanId;
    }
    else
    {
        if (XPS_FDB_ENTRY_TYPE_IPV4_UC_E == fdbEntryType)
        {
            xpsL3HostEntry_t *l3HostEntryPtr = (xpsL3HostEntry_t *)macEntryKeyPtr;
            cpssOsMemCpy(&dxChMacEntryKeyPtr->key.ipv4Unicast.dip,
                         &l3HostEntryPtr->ipv4Addr,
                         sizeof(dxChMacEntryKeyPtr->key.ipv4Unicast.dip));
            dxChMacEntryKeyPtr->key.ipv4Unicast.vrfId = l3HostEntryPtr->vrfId;
        }
        else
        {
            if ((XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E == fdbEntryType) ||
                (XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E == fdbEntryType))
            {
                xpsL3HostEntry_t *l3HostEntryPtr = (xpsL3HostEntry_t *)macEntryKeyPtr;
                cpssOsMemCpy(&dxChMacEntryKeyPtr->key.ipv6Unicast.dip,
                             &l3HostEntryPtr->ipv6Addr,
                             sizeof(dxChMacEntryKeyPtr->key.ipv6Unicast.dip));
                dxChMacEntryKeyPtr->key.ipv6Unicast.vrfId = l3HostEntryPtr->vrfId;

            }
            else
            {
#if TODO_FDB // Multicast Entry
                cpssOsMemCpy(dxChMacEntryKeyPtr->key.ipMcast.sip,
                             macEntryKeyPtr->key.ipMcast.sip,
                             sizeof(dxChMacEntryKeyPtr->key.ipMcast.sip));
                cpssOsMemCpy(dxChMacEntryKeyPtr->key.ipMcast.dip,
                             macEntryKeyPtr->key.ipMcast.dip,
                             sizeof(dxChMacEntryKeyPtr->key.ipMcast.dip));
                dxChMacEntryKeyPtr->key.ipMcast.vlanId = macEntryKeyPtr->key.ipMcast.vlanId;

#endif
            }
        }
    }


#if TODO_FDB // Check if we need to get the vid1

    cpssDxChBrgVlanPortVid1Get(dev, portNum, direction, &vid1);
    dxChMacEntryKeyPtr->vid1 = vid1;
#endif

    return GT_OK;
}

/**
* @internal cpssHalConvertXpsFdbEntryTypeToDxChMacEntryType function
* @endinternal
*
* @brief   return the FDB Mac
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] fdbEntryType           - XPS FDB Entry Type
*
* @param[out] dxChMacEntryType          - (pointer to)
*       dxChMacEntry Type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalConvertXpsFdbEntryTypeToDxChMacEntryType
(
    GT_U8 fdbEntryType,
    GT_U8 *dxChMacEntryType
)
{
    if (dxChMacEntryType == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Bad params recieved");
        return GT_BAD_PARAM;
    }

    switch (fdbEntryType)
    {
        case XPS_FDB_ENTRY_TYPE_MAC_ADDR_E:
            *dxChMacEntryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            *dxChMacEntryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            *dxChMacEntryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_UC_E:
            *dxChMacEntryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E:
            *dxChMacEntryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E:
            *dxChMacEntryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E;
            break;

        case XPS_FDB_ENTRY_TYPE_MAC_ADDR_FID_VID1_E:
            *dxChMacEntryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_MCAST_FID_VID1_E:
            *dxChMacEntryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E;
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_MCAST_FID_VID1_E:
            *dxChMacEntryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_FID_VID1_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal cpssHalConvertPktCmdToDxChFdbTableDaSaCmd
* @endinternal
*
* @brief   Convert generic pkt command to device specific packet command
*
* @param[in] pktCmd                     - generic acket command
* @param[out] *cpssMacTableCmd          - (pointer to) DxCh MAC table command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalConvertPktCmdToDxChFdbTableDaSaCmd
(
    xpPktCmd_e                        pktCmd,
    CPSS_MAC_TABLE_CMD_ENT            *cpssMacTableCmd
)
{
    switch (pktCmd)
    {
        case XP_PKTCMD_FWD:
            *cpssMacTableCmd = CPSS_MAC_TABLE_FRWRD_E;
            break;

        case XP_PKTCMD_DROP:
            *cpssMacTableCmd = CPSS_MAC_TABLE_DROP_E;
            break;

        case XP_PKTCMD_TRAP:
            *cpssMacTableCmd = CPSS_MAC_TABLE_CNTL_E;
            break;

        case XP_PKTCMD_FWD_MIRROR:
            *cpssMacTableCmd = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal cpssHalConvertPktCmdToFdbTableDaSaCmd
* @endinternal
*
* @brief   Convert generic pkt command to device specific packet command
*
* @param[in] pktCmd                     - generic acket command
* @param[out] *cpssMacTableCmd          - (pointer to) DxCh MAC table command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalConvertDxChTablePktCmdToGenric
(
    CPSS_MAC_TABLE_CMD_ENT            cpssMacTableCmd,
    xpPktCmd_e                       *pktCmd
)
{
    switch (cpssMacTableCmd)
    {
        case CPSS_MAC_TABLE_FRWRD_E:
            *pktCmd= XP_PKTCMD_FWD;
            break;

        case CPSS_MAC_TABLE_DROP_E:
            *pktCmd = XP_PKTCMD_DROP;
            break;

        case CPSS_MAC_TABLE_INTERV_E:
            *pktCmd = XP_PKTCMD_DROP;
            break;

        case CPSS_MAC_TABLE_CNTL_E:
            *pktCmd = XP_PKTCMD_TRAP;
            break;

        case CPSS_MAC_TABLE_MIRROR_TO_CPU_E:
            *pktCmd = XP_PKTCMD_FWD_MIRROR;
            break;

        case CPSS_MAC_TABLE_SOFT_DROP_E:
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal cpssHalConvertL3HostEntryToDxChMacEntry function
* @endinternal
*
* @brief   Convert generic MAC entry into device specific MAC entry
*
* @param[in] devId                   - device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr              - (pointer to) MAC entry parameters
*
* @param[out] dxChMacEntryPtr          - (pointer to) DxCh MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalConvertL3HostEntryToDxChMacEntry
(
    int                           devId,
    XPS_FDB_ENTRY_TYPE_ENT        fdbEntryType,
    void                          *macEntryPtr,
    CPSS_MAC_ENTRY_EXT_STC        *dxChMacEntryPtr
)
{
    GT_STATUS rc; /* return code */
    XP_STATUS xpsStatus = XP_NO_ERR;
    xpsArpPointer_t arpPointer;
    xpsInterfaceType_e l3IntfType;
    xpsInterfaceType_e egressIntfType;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    xpsInterfaceInfo_t *intfInfo = NULL;
    xpsInterfaceInfo_t *l3IntfInfo = NULL;
    xpsL3HostEntry_t *l3hostEntry = (xpsL3HostEntry_t *)macEntryPtr;
    xpsInterfaceId_t l3IntfId = 0;

    cpssOsMemSet(dxChMacEntryPtr, 0, sizeof(CPSS_MAC_ENTRY_EXT_STC));


    /*Get arp Pointer from SW DB*/
    xpsStatus = xpsArpGetArpPointer(devId, l3hostEntry->nhEntry.nextHop.macDa,
                                    &arpPointer);
    if (xpsStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed in xpsArpGetArpPointer: %d\n", xpsStatus);
        return GT_FAIL;
    }
    /* Get the type of the router interface */
    xpsStatus = xpsInterfaceGetInfo(l3hostEntry->nhEntry.nextHop.l3InterfaceId,
                                    &intfInfo);
    if (xpsStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed in xpsInterfaceGetInfo: %d\n", xpsStatus);
        return GT_FAIL;
    }

    if (NULL == intfInfo)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid xpsInterfaceGetInfo: %d\n", xpsStatus);
        return GT_FAIL;
    }

    l3IntfType = intfInfo->type;

    /* Get the type of the egress interface only when it is valid interface.
       If FDB is not learned, it will point to INVALID interface or l3IntfId
       if miss action is FWD.
    */

    if (l3hostEntry->nhEntry.nextHop.egressIntfId != XPS_INTF_INVALID_ID)
    {
        /* Get the type of the egress interface */
        xpsStatus = xpsInterfaceGetType(l3hostEntry->nhEntry.nextHop.egressIntfId,
                                        &egressIntfType);
        if (xpsStatus != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed in xpsInterfaceGetType for egress Interface(%d) err %d\n",
                  l3hostEntry->nhEntry.nextHop.egressIntfId, xpsStatus);
            return GT_FAIL;
        }

        /* In Vlan RIF, for resolved nbr, egressIntfId holds port/LAG intfId.
           For unresolved nbr, egressIntfId holds vlan l3IntfId.
           "bdHwId" holds eVlan allocated for port/LAG RIF. This attr is not updated for SVI.
           eVlan of SVI is derived from Vlan's xpsInterfaceId.
         */

        if (l3IntfType == XPS_PORT_ROUTER)
        {
            xpsStatus = xpsL3GetBindPortIntf(l3hostEntry->nhEntry.nextHop.egressIntfId,
                                             &l3IntfId);
            if (xpsStatus != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "xpsL3GetBindPortIntf: %d\n", xpsStatus);
                return GT_FAIL;
            }

            xpsStatus = xpsInterfaceGetInfo(l3IntfId, &l3IntfInfo);
            if ((xpsStatus != XP_NO_ERR) || (l3IntfInfo == NULL))
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get interface information for L3 Interface id : %d\n", l3IntfId);
                return xpsStatus;
            }
        }
    }
    /* convert entry type into device specific format */

    rc = cpssHalConvertXpsFdbEntryTypeToDxChMacEntryType(fdbEntryType,
                                                         (GT_U8 *) &dxChMacEntryPtr->key.entryType);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed in cpssHalConvertXpsFdbEntryTypeToDxChMacEntryType: %d\n", rc);
        return rc;
    }

    if (XPS_FDB_ENTRY_TYPE_IPV4_UC_E == fdbEntryType)
    {
        cpssOsMemCpy(&dxChMacEntryPtr->key.key.ipv4Unicast.dip,
                     &l3hostEntry->ipv4Addr,
                     sizeof(dxChMacEntryPtr->key.key.ipv4Unicast.dip));
        dxChMacEntryPtr->key.key.ipv4Unicast.vrfId = l3hostEntry->vrfId;
    }
    else if ((XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E == fdbEntryType) ||
             (XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E == fdbEntryType))
    {
        cpssOsMemCpy(&dxChMacEntryPtr->key.key.ipv6Unicast.dip,
                     &l3hostEntry->ipv6Addr,
                     sizeof(dxChMacEntryPtr->key.key.ipv6Unicast.dip));
        dxChMacEntryPtr->key.key.ipv6Unicast.vrfId = l3hostEntry->vrfId;
    }

    /*DaCommand And SaCommand are not supported for Ipv4/6 Unicast*/
#if 0
    rc = cpssHalConvertPktCmdToDxChFdbTableDaSaCmd(l3hostEntry->nhEntry.pktCmd,
                                                   &(dxChMacEntryPtr->daCommand));

    if (rc != GT_OK)
    {
        return rc;
    }
#endif

    if ((l3hostEntry->nhEntry.pktCmd != XP_PKTCMD_DROP) &&
        (l3hostEntry->nhEntry.nextHop.egressIntfId != XPS_INTF_INVALID_ID))
    {
        /* Populate CPSS parameters according to the egress L3 interface type */
        switch (l3IntfType)
        {
            case XPS_PORT_ROUTER:
            case XPS_SUBINTERFACE_ROUTER:
            case XPS_VLAN_ROUTER:
                {
                    if (egressIntfType == XPS_PORT)
                    {
                        cpssDevId = xpsGlobalIdToDevId(devId,
                                                       l3hostEntry->nhEntry.nextHop.egressIntfId);
                        cpssPortNum = xpsGlobalPortToPortnum(devId,
                                                             l3hostEntry->nhEntry.nextHop.egressIntfId);
                        if ((cpssDevId == 0xff) || (cpssPortNum == 0xffff))
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "Invalid Device/Port Id :%d : %d  \n", devId,
                                  l3hostEntry->nhEntry.nextHop.egressIntfId);
                            return XP_ERR_INVALID_ARG;
                        }

                        dxChMacEntryPtr->dstInterface.type = CPSS_INTERFACE_PORT_E;
                        dxChMacEntryPtr->dstInterface.devPort.portNum = (GT_PORT_NUM)cpssPortNum;
                        dxChMacEntryPtr->dstInterface.devPort.hwDevNum = (GT_HW_DEV_NUM)cpssDevId;
                        if (l3IntfType == XPS_VLAN_ROUTER)
                        {
                            dxChMacEntryPtr->fdbRoutingInfo.nextHopVlanId =
                                XPS_INTF_MAP_INTF_TO_BD(l3hostEntry->nhEntry.nextHop.l3InterfaceId);
                        }
                        else if (l3IntfType == XPS_PORT_ROUTER)
                        {
                            dxChMacEntryPtr->fdbRoutingInfo.nextHopVlanId =
                                l3IntfInfo->bdHWId;
                        }
                    }
                    else if (egressIntfType == XPS_LAG)
                    {
                        dxChMacEntryPtr->dstInterface.type = CPSS_INTERFACE_TRUNK_E;
                        dxChMacEntryPtr->dstInterface.trunkId = (GT_TRUNK_ID)
                                                                xpsUtilXpstoCpssInterfaceConvert(l3hostEntry->nhEntry.nextHop.egressIntfId,
                                                                                                 egressIntfType);
                        // Using eVLAN for L3 Lag.
                        if (l3IntfType == XPS_VLAN_ROUTER)
                        {
                            dxChMacEntryPtr->fdbRoutingInfo.nextHopVlanId =
                                XPS_INTF_MAP_INTF_TO_BD(l3hostEntry->nhEntry.nextHop.l3InterfaceId);
                        }
                        else if (l3IntfType == XPS_PORT_ROUTER)
                        {
                            dxChMacEntryPtr->fdbRoutingInfo.nextHopVlanId = l3IntfInfo->bdHWId;
                        }
                    }
                    else if (egressIntfType == XPS_VLAN_ROUTER)
                    {
                        dxChMacEntryPtr->dstInterface.type = CPSS_INTERFACE_VIDX_E;
                        dxChMacEntryPtr->dstInterface.vidx = 0xfff;
                        dxChMacEntryPtr->dstInterface.vlanId = XPS_INTF_MAP_INTF_TO_BD(
                                                                   l3hostEntry->nhEntry.nextHop.l3InterfaceId);
                    }
                    else
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "Invalid egress interface type : Egress Interface: \n");
                        return XP_ERR_INVALID_ARG;
                    }
                    break;
                }
            // Other L3 interface types...

            default:
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Unsupported L3 interface type received \n");
                    return XP_ERR_INVALID_ARG;
                }
        }
    }

    /*Workaround: To support pkt command per Fdb-host based IPv4/6 Unicast,
     * Port 62 (resevered Null Port) is used for drop packet.
     *
     */

    if (l3hostEntry->nhEntry.pktCmd == XP_PKTCMD_DROP)
    {
        /* assign to the dropped port*/
        dxChMacEntryPtr->dstInterface.devPort.portNum = CPSS_NULL_PORT_NUM_CNS;
    }
    else if (l3hostEntry->nhEntry.pktCmd != XP_PKTCMD_FWD)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "L3 Host Entry Next Hop entry pktCmd is not FWD return Invalid\n");
        return XP_ERR_INVALID_ARG;
    }

    dxChMacEntryPtr->fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    dxChMacEntryPtr->fdbRoutingInfo.ttlHopLimitDecEnable = GT_TRUE;
    dxChMacEntryPtr->fdbRoutingInfo.ingressMirror = GT_FALSE;
    dxChMacEntryPtr->fdbRoutingInfo.ingressMirrorToAnalyzerIndex = 0;
    dxChMacEntryPtr->fdbRoutingInfo.qosProfileMarkingEnable = GT_FALSE;
    dxChMacEntryPtr->fdbRoutingInfo.qosProfileIndex = 0;
    dxChMacEntryPtr->fdbRoutingInfo.qosPrecedence =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    dxChMacEntryPtr->fdbRoutingInfo.modifyUp =
        CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    dxChMacEntryPtr->fdbRoutingInfo.modifyDscp =
        CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    dxChMacEntryPtr->fdbRoutingInfo.countSet = CPSS_IP_CNT_SET0_E;
    dxChMacEntryPtr->fdbRoutingInfo.trapMirrorArpBcEnable = GT_FALSE;
    dxChMacEntryPtr->fdbRoutingInfo.dipAccessLevel = 0;
    dxChMacEntryPtr->fdbRoutingInfo.ICMPRedirectEnable = GT_FALSE;
    dxChMacEntryPtr->fdbRoutingInfo.mtuProfileIndex = intfInfo->mtuPorfileIndex;
    dxChMacEntryPtr->fdbRoutingInfo.isTunnelStart = GT_FALSE;
    if (l3IntfType == XPS_VLAN_ROUTER)
    {
        dxChMacEntryPtr->fdbRoutingInfo.nextHopVlanId = XPS_INTF_MAP_INTF_TO_BD(
                                                            l3hostEntry->nhEntry.nextHop.l3InterfaceId);
    }
    else
    {
        // TODO
        // Retrieve vlan Id from the L3 interface for Vlan router interfaces
    }

    dxChMacEntryPtr->fdbRoutingInfo.nextHopTunnelPointer = 0;
    dxChMacEntryPtr->fdbRoutingInfo.nextHopARPPointer = arpPointer;
    dxChMacEntryPtr->fdbRoutingInfo.nextHopDataBankNumber = 0;
    dxChMacEntryPtr->fdbRoutingInfo.siteId = CPSS_IP_SITE_ID_INTERNAL_E;
    /* setting age bit to 0.
       When the packet hit the entry the age bit will be changed to 1 incase
       the global refresh bit is enabled */
    dxChMacEntryPtr->age = GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssHalConvertL2MactEntryToDxChMacEntry function
* @endinternal
*
* @brief   Convert generic MAC entry into device specific MAC entry
*
* @param[in] devId                   - device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr              - (pointer to) MAC entry parameters
*
* @param[out] dxChMacEntryPtr          - (pointer to) DxCh MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalConvertL2MactEntryToDxChMacEntry
(
    int                              devId,
    XPS_FDB_ENTRY_TYPE_ENT           fdbEntryType,
    void                             *macEntryPtr,
    CPSS_MAC_ENTRY_EXT_STC           *dxChMacEntryPtr
)
{
    GT_STATUS rc; /* return code */
    xpsFdbEntry_t *fdbEntry  = (xpsFdbEntry_t *)macEntryPtr;
    GT_U32 cpssDevNum;
    GT_U32 cpssPortNum;
    XP_STATUS status = XP_NO_ERR;
    cpssOsMemSet(dxChMacEntryPtr, 0, sizeof(CPSS_MAC_ENTRY_EXT_STC));
    xpsInterfaceType_e type;
    /* convert entry type into device specific format */
    rc = cpssHalConvertXpsFdbEntryTypeToDxChMacEntryType(fdbEntryType,
                                                         (GT_U8 *) &dxChMacEntryPtr->key.entryType);

    if (rc != GT_OK)
    {
        return rc;
    }

    if (XPS_FDB_ENTRY_TYPE_MAC_ADDR_E == fdbEntryType)
    {
        cpssOsMemCpy(&dxChMacEntryPtr->key.key.macVlan.macAddr,
                     &fdbEntry->macAddr,
                     sizeof(dxChMacEntryPtr->key.key.macVlan.macAddr));
        dxChMacEntryPtr->key.key.macVlan.vlanId = fdbEntry->vlanId;


    }

    status = xpsInterfaceGetType(fdbEntry->intfId, &type);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get interface type failed");
        return GT_FAIL;
    }
    /*if it is  physical port*/
    if (type == XPS_PORT)
    {
        cpssDevNum = xpsGlobalIdToDevId(devId, fdbEntry->intfId);
        cpssPortNum = xpsGlobalPortToPortnum(devId, fdbEntry->intfId);
        dxChMacEntryPtr->dstInterface.devPort.portNum = cpssPortNum;
        dxChMacEntryPtr->dstInterface.devPort.hwDevNum = cpssDevNum;
        dxChMacEntryPtr->dstInterface.type = CPSS_INTERFACE_PORT_E;
    }
    else if (type == XPS_LAG) /*if it is lag*/
    {
        dxChMacEntryPtr->dstInterface.trunkId = (GT_TRUNK_ID)
                                                xpsUtilXpstoCpssInterfaceConvert(fdbEntry->intfId, type);
        dxChMacEntryPtr->dstInterface.type = CPSS_INTERFACE_TRUNK_E;
    }
    else if (type == XPS_VLAN_ROUTER)
    {
        dxChMacEntryPtr->dstInterface.vlanId = fdbEntry->vlanId;
        dxChMacEntryPtr->dstInterface.type =  CPSS_INTERFACE_VID_E;
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Unsupported value");
        /*return unsupported error type*/
        return GT_BAD_VALUE;
    }
    if (fdbEntry->isStatic)
    {
        dxChMacEntryPtr->isStatic = GT_TRUE;
        dxChMacEntryPtr->age = GT_FALSE;
    }

    /*to mark router mac entry*/
    if (fdbEntry->isRouter)
    {
        dxChMacEntryPtr->daRoute = GT_TRUE;
    }

    /*copy DA command*/
    /* convert daCommand info into device specific format */

    rc = cpssHalConvertPktCmdToDxChFdbTableDaSaCmd(fdbEntry->pktCmd,
                                                   &(dxChMacEntryPtr->daCommand));

    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Conversion of packet command failed");
        return rc;
    }
    if (fdbEntry->isControl)
    {
        dxChMacEntryPtr->appSpecificCpuCode = GT_TRUE;
        dxChMacEntryPtr->dstInterface.devPort.portNum = CPSS_CPU_PORT_NUM_CNS;
        dxChMacEntryPtr->dstInterface.devPort.hwDevNum = 0; //master device
        dxChMacEntryPtr->dstInterface.type = CPSS_INTERFACE_PORT_E;
    }
    /*TODO VGUNTA  SA command is not provided by SAI, need to clarify intrnally on this*/

    return GT_OK;
}

/**
* @internal cpssHalConvertGenericToDxChMacEntry function
* @endinternal
*
* @brief   Convert generic MAC entry into device specific MAC entry
*
* @param[in] devId                   - device Id
* @param[in] fdbEntryType            - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr              - (pointer to) MAC entry parameters
*
* @param[out] dxChMacEntryPtr          - (pointer to) DxCh MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalConvertGenericToDxChMacEntry
(
    int                       devId,
    XPS_FDB_ENTRY_TYPE_ENT    fdbEntryType,
    void *macEntryPtr,
    CPSS_MAC_ENTRY_EXT_STC *dxChMacEntryPtr
)
{
    GT_STATUS rc = GT_FAIL; /* return code */

    /* call specific format function to convert to device specific format */
    switch (fdbEntryType)
    {
        case XPS_FDB_ENTRY_TYPE_MAC_ADDR_E:
            rc = cpssHalConvertL2MactEntryToDxChMacEntry(devId, fdbEntryType, macEntryPtr,
                                                         dxChMacEntryPtr);
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_UC_E:
        case XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E:
        case XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E:
            rc = cpssHalConvertL3HostEntryToDxChMacEntry(devId, fdbEntryType, macEntryPtr,
                                                         dxChMacEntryPtr);
            break;
        case XPS_FDB_ENTRY_TYPE_MAC_ADDR_FID_VID1_E:
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_MCAST_FID_VID1_E:
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_MCAST_FID_VID1_E:
            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
}


/**
* @internal cpssHalConvertCpssTol3HostMacEntry function
* @endinternal
*
* @brief   Convert device specific MAC entry into generic MAC entry
*
* @param[in] devId                    - device Id
* @param[in] cpssMacEntryPtr          - (pointer to) cpss MAC entry parameters
*
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[out] macEntryPtr              - (pointer to) MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalConvertCpssTol3HostMacEntry
(
    int                     devId,
    CPSS_MAC_ENTRY_EXT_STC  *cpssMacEntryPtr,
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType,
    void                    *macEntryPtr
)
{
    GT_STATUS   rc = GT_OK;
    XP_STATUS xpsStatus = XP_NO_ERR;
    GT_U32 arpPointer;
    xpsScope_t scopeId;
    xpsInterfaceId_t trunkId;
    xpsInterfaceInfo_t *intfInfo;
    xpsL3HostEntry_t *l3HostEntry = (xpsL3HostEntry_t *) macEntryPtr;


    cpssOsMemSet(l3HostEntry, 0, sizeof(xpsL3HostEntry_t));

    if (CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E == cpssMacEntryPtr->key.entryType)
    {
        cpssOsMemCpy(&l3HostEntry->ipv4Addr,
                     &cpssMacEntryPtr->key.key.ipv4Unicast.dip,
                     sizeof(cpssMacEntryPtr->key.key.ipv4Unicast.dip));
        l3HostEntry->vrfId = cpssMacEntryPtr->key.key.ipv4Unicast.vrfId;
        l3HostEntry->type = XP_PREFIX_TYPE_IPV4;
    }
    else if ((CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E ==
              cpssMacEntryPtr->key.entryType) ||
             (CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E ==
              cpssMacEntryPtr->key.entryType))
    {
        cpssOsMemCpy(&l3HostEntry->ipv6Addr,
                     &cpssMacEntryPtr->key.key.ipv6Unicast.dip,
                     sizeof(cpssMacEntryPtr->key.key.ipv6Unicast.dip));
        l3HostEntry->vrfId = cpssMacEntryPtr->key.key.ipv6Unicast.vrfId;
        l3HostEntry->type = XP_PREFIX_TYPE_IPV6;


    }

    arpPointer = cpssMacEntryPtr->fdbRoutingInfo.nextHopARPPointer;
    /*Get arp Pointer from SW DB*/
    xpsStatus = xpsIpRouterArpAddRead(devId, arpPointer,
                                      &l3HostEntry->nhEntry.nextHop.macDa);
    if (xpsStatus != XP_NO_ERR)
    {
        return GT_FAIL;
    }

    l3HostEntry->nhEntry.serviceInstId =
        cpssMacEntryPtr->fdbRoutingInfo.nextHopVlanId;

    //set up the VPN Label.
    if (cpssMacEntryPtr->fdbRoutingInfo.isTunnelStart == GT_TRUE)
    {

    }

    if (cpssMacEntryPtr->dstInterface.devPort.portNum != CPSS_NULL_PORT_NUM_CNS)
    {
        switch (cpssMacEntryPtr->dstInterface.type)
        {
            case CPSS_INTERFACE_PORT_E:
                l3HostEntry->nhEntry.nextHop.egressIntfId = xpsLocalPortToGlobalPortnum(
                                                                cpssMacEntryPtr->dstInterface.devPort.hwDevNum,
                                                                cpssMacEntryPtr->dstInterface.devPort.portNum);
                break;
            case CPSS_INTERFACE_TRUNK_E:
                trunkId = (xpsInterfaceId_t)(cpssMacEntryPtr->dstInterface.trunkId);
                l3HostEntry->nhEntry.nextHop.egressIntfId = xpsUtilCpssToXpsInterfaceConvert(
                                                                trunkId, XPS_LAG);
                break;

            /* This case can hit when Dst Intf is not resolved and
               FDB Unicast miss action set to FWD
             */

            case CPSS_INTERFACE_VID_E:
            case CPSS_INTERFACE_VIDX_E:

                l3HostEntry->nhEntry.nextHop.egressIntfId = XPS_INTF_MAP_BD_TO_INTF(
                                                                cpssMacEntryPtr->dstInterface.vlanId);
                break;

            default:
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Unsupported dstIntfType : %d\n", cpssMacEntryPtr->dstInterface.type);
                    return XP_ERR_INVALID_ARG;
                }
        }
        xpsStatus = xpsScopeGetScopeId(devId, &scopeId);
        if (xpsStatus != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get scope for devId : %d\n", devId);
            return xpsStatus;
        }

        xpsStatus = xpsInterfaceGetInfoScope(scopeId,
                                             l3HostEntry->nhEntry.nextHop.egressIntfId, &intfInfo);

        if (xpsStatus != XP_NO_ERR || (intfInfo == NULL))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get mapped L3 interface with port inteface %d\n",
                  l3HostEntry->nhEntry.nextHop.egressIntfId);
            return xpsStatus;
        }

        if (cpssMacEntryPtr->fdbRoutingInfo.nextHopVlanId < XPS_L3_RESERVED_ROUTER_VLAN(
                devId))
        {
            l3HostEntry->nhEntry.nextHop.l3InterfaceId = XPS_INTF_MAP_BD_TO_INTF(
                                                             cpssMacEntryPtr->fdbRoutingInfo.nextHopVlanId);
        }

        else
        {
            l3HostEntry->nhEntry.nextHop.l3InterfaceId =
                intfInfo->intfMappingInfo.keyIntfId;
        }

        /* convert daCommand info into genric format */
        rc = cpssHalConvertDxChTablePktCmdToGenric(cpssMacEntryPtr->daCommand,
                                                   &(l3HostEntry->nhEntry.pktCmd));
        if (rc != GT_OK)
        {
            return rc;
        }

    }
    else
    {
        l3HostEntry->nhEntry.nextHop.egressIntfId = XPS_INTF_INVALID_ID;
        l3HostEntry->nhEntry.nextHop.l3InterfaceId = XPS_INTF_INVALID_ID;
        l3HostEntry->nhEntry.pktCmd = XP_PKTCMD_DROP;
    }

    return rc;

}

/**
* @internal cpssHalConvertCpssToGenricL2MactEntry function
* @endinternal
*
* @brief   Convert device specific MAC entry into generic MAC entry
*
* @param[in] cpssMacEntryPtr          - (pointer to) cpss MAC entry parameters
*
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[out] macEntryPtr              - (pointer to) MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalConvertCpssToGenricL2MactEntry
(
    CPSS_MAC_ENTRY_EXT_STC   *cpssMacEntryPtr,
    XPS_FDB_ENTRY_TYPE_ENT   fdbEntryType,
    void                     *macEntryPtr
)
{
    GT_STATUS   rc = GT_OK;
    xpsFdbEntry_t *fdbEntry  = (xpsFdbEntry_t *)macEntryPtr;

    cpssOsMemSet(fdbEntry, 0, sizeof(xpsFdbEntry_t));
    if (XPS_FDB_ENTRY_TYPE_MAC_ADDR_E == fdbEntryType)
    {
        cpssOsMemCpy(&fdbEntry->macAddr,
                     &cpssMacEntryPtr->key.key.macVlan.macAddr,
                     sizeof(cpssMacEntryPtr->key.key.macVlan.macAddr));
        fdbEntry->vlanId = cpssMacEntryPtr->key.key.macVlan.vlanId;
    }

    /*TODO VGUNTA currently handle only physical port case, later update to all interface type like LAG */
    if (cpssMacEntryPtr->dstInterface.type == CPSS_INTERFACE_PORT_E)
    {
        fdbEntry->intfId = cpssMacEntryPtr->dstInterface.devPort.portNum;
    }
    else if (cpssMacEntryPtr->dstInterface.type == CPSS_INTERFACE_TRUNK_E)
    {
        fdbEntry->intfId = xpsUtilCpssToXpsInterfaceConvert(
                               cpssMacEntryPtr->dstInterface.trunkId, XPS_LAG);
    }

    if (cpssMacEntryPtr->isStatic)
    {
        fdbEntry->isStatic = GT_TRUE;
    }

    if (cpssMacEntryPtr->daRoute)
    {
        fdbEntry->isRouter = GT_TRUE;
    }



    /*copy DA command*/
    /* convert daCommand info into genric format */

    rc = cpssHalConvertDxChTablePktCmdToGenric(cpssMacEntryPtr->daCommand,
                                               &(fdbEntry->pktCmd));

    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Conversion of packet command failed");
        return rc;
    }

    /*TODO VGUNTA  SA command is not provided by SAI, need to clarify intrnally on this*/

    return GT_OK;
}
/**
* @internal cpssHalConvertCpssToGenericMacEntry function
* @endinternal
*
* @brief   Convert device specific MAC entry into generic MAC entry
*
* @param[in] devId                    - device Id
* @param[in] cpssMacEntryPtr          - (pointer to) cpss MAC entry parameters
*
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[out] macEntryPtr              - (pointer to) MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalConvertCpssToGenericMacEntry
(
    int                        devId,
    CPSS_MAC_ENTRY_EXT_STC  *cpssMacEntryPtr,
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType,
    void                    *macEntryPtr
)
{
    GT_STATUS   rc = GT_NOT_SUPPORTED;
    /* convert entry type from device specific format */
    switch (fdbEntryType)
    {
        case XPS_FDB_ENTRY_TYPE_MAC_ADDR_E:
            rc = cpssHalConvertCpssToGenricL2MactEntry(cpssMacEntryPtr, fdbEntryType,
                                                       macEntryPtr);
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_UC_E:
        case XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E:
        case XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E:
            rc = cpssHalConvertCpssTol3HostMacEntry(devId, cpssMacEntryPtr, fdbEntryType,
                                                    macEntryPtr);
            break;
        case XPS_FDB_ENTRY_TYPE_MAC_ADDR_FID_VID1_E:
            break;

        case XPS_FDB_ENTRY_TYPE_IPV4_MCAST_FID_VID1_E:
            break;

        case XPS_FDB_ENTRY_TYPE_IPV6_MCAST_FID_VID1_E:
            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal cpssHalBrgFdbMacKeyCompare function
* @endinternal
*
* @brief   compare 2 mac keys.
*
* @param[in] devId             - device ID
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] key1Ptr                  - (pointer to) key 1
* @param[in] key2Ptr                  - (pointer to) key 2
*                                      OUTPUTS:
*                                      > 0  - if key1Ptr is  bigger than key2Ptr
*                                      == 0 - if key1Ptr is equal to str2
*                                      < 0  - if key1Ptr is smaller than key2Ptr
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
* @retval GT_TIMEOUT               - on timeout waiting for the QR
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
*/
int cpssHalBrgFdbMacKeyCompare
(
    int                          devId,
    XPS_FDB_ENTRY_TYPE_ENT       fdbEntryType,
    void                         *key1Ptr,
    CPSS_MAC_ENTRY_EXT_KEY_STC   *dxChMacEntryKey2
)
{
    GT_STATUS   rc;
    CPSS_MAC_ENTRY_EXT_KEY_STC  dxChMacEntryKey1;

    /* convert key data into device specific format */
    rc = cpssHalConvertGenericToDxChMacEntryKey(devId, fdbEntryType, key1Ptr,
                                                &dxChMacEntryKey1);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "[XPS]: cpssHalConvertGenericToDxChMacEntryKey FAILED, rc = [%d]", rc);
        return rc;  /* value is not 0 (GT_OK) */
    }
    /* VRF-ID is valid only for IPV6 DATA Entry.
       Hence override it to skip in comparison. */
    if (fdbEntryType == XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E)
    {
        dxChMacEntryKey1.key.ipv6Unicast.vrfId = 0;
    }

    switch (fdbEntryType)
    {
        case XPS_FDB_ENTRY_TYPE_MAC_ADDR_FID_VID1_E:
        case XPS_FDB_ENTRY_TYPE_IPV4_MCAST_FID_VID1_E:
        case XPS_FDB_ENTRY_TYPE_IPV6_MCAST_FID_VID1_E:
            break;
        default:
            /* compare without the vid1 field */
            dxChMacEntryKey1.vid1 = dxChMacEntryKey2->vid1;
            break;
    }

    return cpssOsMemCmp(&dxChMacEntryKey1, dxChMacEntryKey2,
                        sizeof(dxChMacEntryKey1));
}

GT_STATUS cpssHalSetBrgFdbMacVlanLookupMode
(
    int                  devId,
    CPSS_MAC_VL_ENT      mode
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgFdbMacVlanLookupModeSet(devNum, mode);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to configure vlanaware mode\n");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalSetBrgSecurBreachEventPacketCommand
(
    int                                  devId,
    CPSS_BRG_SECUR_BREACH_EVENTS_ENT     eventType,
    CPSS_PACKET_CMD_ENT                  command
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgSecurBreachEventPacketCommandSet(devNum, eventType,
                                                             command);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to configure vlanaware mode\n");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalSetBrgFdbActionsEnable
(
    int      devId,
    GT_BOOL  enable
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgFdbActionsEnableSet(devNum, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to set FDB Action\n");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalSetBrgFdbNaMsgOnChainTooLong
(
    int      devId,
    GT_BOOL  enable
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgFdbNaMsgOnChainTooLongSet(devNum, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to disable NA messages sending limit to CPU\n");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalSetBrgFdbAAandTAToCpu
(
    int                          devId,
    GT_BOOL                      enable
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgFdbAAandTAToCpuSet(devNum, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to disable AA TA update to CPU\n");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalSetBrgFdbAuMsgRateLimit
(
    int                         devId,
    GT_U32                      msgRate,
    GT_BOOL                     enable
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgFdbAuMsgRateLimitSet(devNum, msgRate, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to disable rate limit to  CPU\n");
            return status;
        }
    }
    return status;
}


GT_STATUS cpssHalSetBrgFdbMacTriggerMode
(
    int                          devId,
    CPSS_MAC_ACTION_MODE_ENT     mode
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgFdbMacTriggerModeSet(devNum, mode);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set set Mac address table Triggered action mode\n");
            return status;
        }
    }
    return status;
}


GT_STATUS cpssHalSetBrgFdbActionMode
(
    int                          devId,
    CPSS_FDB_ACTION_MODE_ENT     mode
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgFdbActionModeSet(devNum, mode);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set FDB action mode without setting action trigger to age without delete\n");
            return status;
        }
    }
    return status;
}


GT_STATUS cpssHalSetBrgFdbUploadEnable
(
    int     devId,
    GT_BOOL enable
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgFdbUploadEnableSet(devNum, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set disable reading FDB entries via AU messages to CPU\n");
            return status;
        }
    }
    return status;
}

/**
* @internal cpssHalSetBrgFdbHashMode function
* @endinternal
*
* @brief   Sets the FDB hash function mode.
*         The CRC based hash function provides the best hash index distribution
*         for random addresses and vlans.
*         The XOR based hash function provides optimal hash index distribution
*         for controlled testing scenarios, where sequential addresses and vlans
*         are often used.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devId                   - device number
* @param[in] mode                     - hash function based mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/


GT_STATUS cpssHalSetBrgFdbHashMode
(
    int                          devId,
    CPSS_MAC_HASH_FUNC_MODE_ENT  mode
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgFdbHashModeSet(devNum, mode);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set FDB Hash Mode\n");
            return status;
        }
    }
    return status;
}


GT_STATUS cpssHalSetBrgFdbDropAuEnable
(
    int      devId,
    GT_BOOL  enable
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgFdbDropAuEnableSet(devNum, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to Disable dropping AU messages when queue is full");
            return status;
        }
    }
    return status;
}


GT_STATUS cpssHalSetBrgFdbAgeBitDaRefreshEnable
(
    int     devId,
    GT_BOOL enable
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgFdbAgeBitDaRefreshEnableSet(devNum, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Disable destination address based ageing\n");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalEnableBrgGenDropInvalidSa
(
    int      devId,
    GT_BOOL  enable
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgGenDropInvalidSaEnable(devNum, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to drop all ethernet packets with MAC SA that are Multicast\n");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalSetBrgSecurBreachMovedStaticAddr
(
    int                  devId,
    GT_BOOL              enable
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgSecurBreachMovedStaticAddrSet(devNum, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set security breach moved static address set\n");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalSetBrgSecurBreachEventDropMode
(
    int                                  devId,
    CPSS_BRG_SECUR_BREACH_EVENTS_ENT     eventType,
    CPSS_DROP_MODE_TYPE_ENT              dropMode
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgSecurBreachEventDropModeSet(devNum, eventType, dropMode);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set security breach event drop mode\n");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalSetDrvPpHwRegField
(
    int      devId,
    GT_U32   portGroupId,
    GT_U32   regAddr,
    GT_U32   fieldOffset,
    GT_U32   fieldLength,
    GT_U32   fieldData
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDrvPpHwRegFieldSet(devNum, portGroupId,
                                        regAddr, fieldOffset, fieldLength, fieldData);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set security breach event drop mode\n");
            return status;
        }
    }
    return status;
}

/**
* @internal cpssHalSetBrgFdbRoutingNextHopPacketCmd function
* @endinternal
*
* @brief   Set Packet Commands for FDB routed packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devId                   - device number
* @param[in] nhPacketCmd              - Route entry command. supported commands:
*                                      CPSS_PACKET_CMD_ROUTE_E,
*                                      CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_DROP_SOFT_E ,
*                                      CPSS_PACKET_CMD_DROP_HARD_E,
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/


GT_STATUS cpssHalSetBrgFdbRoutingNextHopPacketCmd
(
    int                         devId,
    CPSS_PACKET_CMD_ENT         nhPacketCmd
)

{
    GT_STATUS           status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgFdbRoutingNextHopPacketCmdSet(devNum, nhPacketCmd);
        if (status!=GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Next hop commanad setting failed");
            return status;
        }
    }

    return status;
}

/**
* @internal cpssHalFlushFdbEntry function
* @endinternal
*
* @brief   Invoke CPSS APIs in a sequence that would enable flush of FDB entries
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devId                 - device number
* @param[in] xpsFlushParams        - Parameters that define type of flush
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS cpssHalFlushFdbEntry
(int devId,
 xpsFlushParams* flushParams
)
{
    GT_STATUS status = GT_OK;

    GT_U16 vlanId = flushParams->vlanId;
    GT_U16 intfId = flushParams->intfId;
    xpsFdbEntryType_e entryType = flushParams->entryType;
    CPSS_FDB_ACTION_MODE_ENT actionMode = CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E;
    CPSS_MAC_ACTION_MODE_ENT macMode = CPSS_ACT_TRIG_E;
    GT_BOOL aaTaMessegesToCpuEnable = GT_FALSE;
    int devNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        /*Save FDB action Mode*/
        status = cpssDxChBrgFdbActionModeGet(devNum, &actionMode);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb Action Mode Get failed");
            return status;
        }

        /*Save MAC Trigger mode */
        status = cpssDxChBrgFdbMacTriggerModeGet(devNum, &macMode);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Fdb Mac Trigger Mode Get failed");
            return status;
        }

        /*Get AA TA message update to CPU status */
        status = cpssDxChBrgFdbAAandTAToCpuGet(devNum, &aaTaMessegesToCpuEnable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "AA to CPU Mode Get failed");
            return status;
        }

        if (aaTaMessegesToCpuEnable == GT_TRUE)
        {
            /* disable AA and TA messages to CPU */
            status = cpssDxChBrgFdbAAandTAToCpuSet(devNum, GT_FALSE);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "AA to CPU Mode Set failed");
                return status;
            }

        }

        break;
    }


    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        GT_BOOL actionComplete = GT_FALSE;
        /*Disable FDB action while flush is getting completed */
        status = cpssDxChBrgFdbActionsEnableSet(devNum, GT_FALSE);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Fdb Action Enable Set Failed");
            return status;
        }

        if (flushParams->flushType == XP_FDB_FLUSH_ALL)
        {
            /*Set Interface Id */
            status = cpssDxChBrgFdbActionActiveInterfaceSet(devNum, 0, 0, 0, 0);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb interface set failed");
                return status;
            }

            /*Set Vlan Id*/
            status = cpssDxChBrgFdbActionActiveVlanSet(devNum, 0, 0);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb active Vlan set failed");
                return status;
            }
        }
        else if (flushParams->flushType == XP_FDB_FLUSH_BY_INTF)
        {
            /*Set Interface Id */
            status = cpssDxChBrgFdbActionActiveInterfaceSet(devNum, 0, 1, intfId,
                                                            (GT_U16)0x1FFF);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb interface set failed");
                return status;
            }

            /*Set Vlan Id*/
            status = cpssDxChBrgFdbActionActiveVlanSet(devNum, 0, 0);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb active Vlan set failed");
                return status;
            }
        }
        else if (flushParams->flushType == XP_FDB_FLUSH_BY_VLAN)
        {
            /*Set Interface Id */
            status = cpssDxChBrgFdbActionActiveInterfaceSet(devNum, 0, 0, 0, 0);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb interface set failed");
                return status;
            }

            /*Set Vlan Id*/
            status = cpssDxChBrgFdbActionActiveVlanSet(devNum, (GT_U16)vlanId,
                                                       (GT_U16)0xFFF);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb active Vlan set failed");
                return status;
            }
        }
        else if (flushParams->flushType == XP_FDB_FLUSH_BY_INTF_VLAN)
        {
            /*Set Interface Id */
            status = cpssDxChBrgFdbActionActiveInterfaceSet(devNum, 0, 1, intfId,
                                                            (GT_U16)0x1FFF);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb interface set failed");
                return status;
            }

            /*Set Vlan Id*/
            status = cpssDxChBrgFdbActionActiveVlanSet(devNum, vlanId, (GT_U16)0xFFF);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb active Vlan set failed");
                return status;
            }
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Flush Type");
            return GT_BAD_PARAM;
        }

        /* Enable if Static entries also should be flushed */
        if (entryType == XP_FDB_ENTRY_TYPE_STATIC)
        {
            status = cpssDxChBrgFdbStaticDelEnable(devNum,
                                                   CPSS_DXCH_BRG_FDB_DEL_MODE_STATIC_ONLY_E);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Enabling entry deletion type failed");
                return status;
            }
        }
        /*Enable if both static and dynamic entries need to be flushed*/
        else if (entryType == XP_FDB_ENTRY_TYPE_ALL)
        {
            status = cpssDxChBrgFdbStaticDelEnable(devNum,
                                                   CPSS_DXCH_BRG_FDB_DEL_MODE_ALL_E);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Enabling entry deletion type failed");
                return status;
            }
        }
        /*Enable if only dynamic entries need to be flushed*/
        else
        {
            status = cpssDxChBrgFdbStaticDelEnable(devNum,
                                                   CPSS_DXCH_BRG_FDB_DEL_MODE_DYNAMIC_ONLY_E);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Enabling entry deletion type failed");
                return status;
            }
        }

        /*Trigger deletion action from CPU*/
        status = cpssDxChBrgFdbTrigActionStart(devNum, CPSS_FDB_ACTION_DELETING_E);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Fdb Action Trigger start failed ");
            return status;
        }

        while (!actionComplete)
        {
            /*Get action completion status */
            status = cpssDxChBrgFdbTrigActionStatusGet(devNum, &actionComplete);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Getting Fdb Action status Failed");
                return status;
            }

            if (actionComplete)
            {

            }
            else
            {
                /*Wait for 10 ms */
                osTimerWkAfter(10);
            }
        }
    }

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        /*Disable static entry deletion */
        status = cpssDxChBrgFdbStaticDelEnable(devNum,
                                               CPSS_DXCH_BRG_FDB_DEL_MODE_DYNAMIC_ONLY_E);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Fdb Static entry deletion mode enable failed");
            return status;
        }

        if (aaTaMessegesToCpuEnable == GT_TRUE)
        {

            /* Enable AA and TA messages to CPU */
            status = cpssDxChBrgFdbAAandTAToCpuSet(devNum, GT_TRUE);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "AA to CPU Mode Set failed");
                return status;
            }

        }
        /*Set Interface Id */
        status = cpssDxChBrgFdbActionActiveInterfaceSet(devNum, 0, 0, 0, 0);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb interface set failed");
            return status;
        }

        /*Restore the action mode*/
        status = cpssDxChBrgFdbActionModeSet(devNum, actionMode);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb Action Mode Set Failed");
            return status;
        }

        /*Set Vlan Id*/
        status = cpssDxChBrgFdbActionActiveVlanSet(devNum, 0, 0);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb active Vlan set failed");
            return status;
        }

        /* Set Trigger mode back to previous trigger mode */
        status = cpssDxChBrgFdbMacTriggerModeSet(devNum, macMode);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb Action Trigger Failed");
            return status;
        }
    }

    return GT_OK;
}

GT_STATUS cpssHalSetBrgFdbPortLearning
(
    int     devId,
    GT_U32  portNum,
    GT_U32  unkSrcCmd,
    GT_BOOL naMsgToCpuEnable
)
{
    GT_STATUS status = GT_OK;
    CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC learningInfo;
    int devNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgFdbManagerPortLearningGet(devNum, portNum, &learningInfo);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgFdbManagerPortLearningGet failed - %d\n", status);
            return status;
        }

        learningInfo.naMsgToCpuEnable = naMsgToCpuEnable;
        learningInfo.unkSrcAddrCmd    = unkSrcCmd;

        status = cpssDxChBrgFdbManagerPortLearningSet(devNum, portNum, &learningInfo);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgFdbManagerPortLearningSet failed - %d\n", status);
            return status;
        }
    }

    return status;
}

GT_STATUS cpssHalGetBrgFdbPortLearning
(
    int     devId,
    GT_U32  portNum,
    GT_U32  *unkSrcCmd,
    GT_BOOL *naMsgToCpuEnable
)
{
    GT_STATUS status = GT_OK;
    CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC learningInfo;
    int devNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgFdbManagerPortLearningGet(devNum, portNum, &learningInfo);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgFdbManagerPortLearningGet failed - %d\n", status);
            return status;
        }

        *naMsgToCpuEnable = learningInfo.naMsgToCpuEnable;
        *unkSrcCmd        = learningInfo.unkSrcAddrCmd;

        break;
    }

    return status;
}


