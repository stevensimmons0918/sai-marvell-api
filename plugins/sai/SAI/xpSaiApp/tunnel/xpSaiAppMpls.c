// xpSaiAppMpls.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiAppMpls.h"
#include "xpsInternal.h"

extern uint32_t vrfIdxToVrf[MAX_VRF_IDX];

uint8_t xpSaiAppConfigureMplsLabelEntry(xpDevice_t devId,
                                        sai_object_id_t switchOid, MplsLabelEntry *xpMplsLabelData)
{
    uint8_t retVal = XP_NO_ERR;
    uint32_t mplsLabel;
    xpsMplsLabelEntry_t *mplsLblEntry = NULL, *getMplsLblEntry = NULL;
    if (!xpMplsLabelData)
    {
        return XP_ERR_NULL_POINTER;
    }

    mplsLblEntry = (xpsMplsLabelEntry_t *)malloc(sizeof(xpsMplsLabelEntry_t));
    if (mplsLblEntry == NULL)
    {
        printf("Error allocating memory for mplsLblEntry\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(mplsLblEntry, 0x0, sizeof(xpsMplsLabelEntry_t));

    mplsLblEntry->keyLabel = xpMplsLabelData->keyMplsLabel;
    mplsLabel = xpMplsLabelData->keyMplsLabel;
    mplsLblEntry->pktCmd = (xpsPktCmd_e)xpMplsLabelData->pktCmd;
    mplsLblEntry->propTTL = (uint32_t)xpMplsLabelData->propTTL;
    mplsLblEntry->swapLabel = xpMplsLabelData->mplsSwapLabel;
    mplsLblEntry->mplsOper = (xpMplsLabelOper)xpMplsLabelData->mplsOper;

    void *r  = xpSaiAppGetNhContext(devId, xpMplsLabelData->nhId);
    if (r == NULL)
    {
        printf("Error: nhId : %d in not found in nhContextMap \n",
               xpMplsLabelData->nhId);
        free(mplsLblEntry);
        return XP_ERR_INIT;
    }
    mplsLblEntry->nextHopData = (*(xpSaiAppNhContextMap **)r)->nh.nextHop;

    printf("MPLS label keyMplsLabel : %d | NhId: %d | InfId: %d \n",
           xpMplsLabelData->keyMplsLabel, xpMplsLabelData->nhId,
           mplsLblEntry->nextHopData.egressIntfId);

    retVal = xpsMplsAddLabelEntry(devId, mplsLblEntry);
    if (retVal != XP_NO_ERR)
    {
        printf("%s:Error in inserting a MPLS label entry : error code: %d\n",
               __FUNCTION__, retVal);
        free(mplsLblEntry);
        return retVal;
    }

    if (retVal == XP_NO_ERR)
    {
        printf("xpsMplsAddLabelEntry: Pass\n");
    }

    printf("Perform lookup for the same entry\n");

    getMplsLblEntry = (xpsMplsLabelEntry_t *)malloc(sizeof(xpsMplsLabelEntry_t));
    if (getMplsLblEntry == NULL)
    {
        printf("Error allocating memory for getMplsLblEntry\n");
        free(mplsLblEntry);
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(getMplsLblEntry, 0x0, sizeof(xpsMplsLabelEntry_t));

    retVal = xpsMplsGetLabelEntry(devId, mplsLabel, getMplsLblEntry);
    if (retVal != XP_NO_ERR)
    {
        printf("%s:Error in MPLS entry lookup : error code: %d\n", __FUNCTION__,
               retVal);
    }

    /* Free allocated memory */
    free(mplsLblEntry);
    free(getMplsLblEntry);

    return XP_NO_ERR;
}

uint8_t xpSaiAppRemoveMplsLabelEntry(xpDevice_t devId, uint32_t mplsLabel)
{
    uint8_t retVal = XP_NO_ERR;

    retVal = xpsMplsRemoveLabelEntry(devId, mplsLabel);
    if (retVal)
    {
        printf("Removing MPLS Label Entry Failed !! \n");
        return retVal;
    }
    return XP_NO_ERR;
}

uint8_t xpSaiAppConfigureMplsTunnelEntry(xpDevice_t devId,
                                         sai_object_id_t switchOid, TunnelMplsEntry *xpTnlMplsData)
{
    uint8_t retVal = XP_NO_ERR;
    xpsMplsTunnelKey_t *mplsTnlKey = NULL;
    xpsInterfaceId_t tnlId = 0;
    xpsMplsTunnelParams_t tunnelParams;
    xpsMplsVpnParams_t vpnParams;
    uint32_t nhIdx = 0;

    if (!xpTnlMplsData)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (xpTnlMplsData->entryType == 1)
    {
        /* VPN config */
        if (xpTnlMplsData->setBd == 1)
        {
            xpsInterfaceId_t intfId;
            retVal = xpsL3CreateVpnIntf(&intfId);

            if (retVal != XP_NO_ERR)
            {
                printf("%s:Error in xpsL3VpnIntfCreate : error code: %d\n", __FUNCTION__,
                       retVal);
                return retVal;
            }

            retVal = xpsL3InitVpnIntf(devId, intfId);

            if (retVal != XP_NO_ERR)
            {
                printf("%s:Error in xpsL3InitVpnIntf : error code: %d\n", __FUNCTION__, retVal);
                return retVal;
            }

            retVal = xpsL3SetVpnIntfVrf(devId, intfId, vrfIdxToVrf[xpTnlMplsData->vrfId]);

            if (retVal != XP_NO_ERR)
            {
                printf("%s:Error in xpsL3SetVpnIntfVrf : error code: %d\n", __FUNCTION__,
                       retVal);
                return retVal;
            }

            if ((retVal = xpsL3BindVpnIntfToLabel(devId, xpTnlMplsData->label0,
                                                  intfId)) != XP_NO_ERR)
            {
                printf("ERROR: xpsL3VpnIntfBindToLabel returns %d\n", retVal);
                return retVal;
            }

            memset(&vpnParams, 0, sizeof(xpsMplsVpnParams_t));
            if ((retVal = xpsMplsSetVpnConfig(devId, xpTnlMplsData->label0,
                                              &vpnParams)) != XP_NO_ERR)
            {
                printf("ERROR: xpsMplsSetVpnConfig returns %d\n", retVal);
                return retVal;
            }
        }
        printf("MPLS VPN Entry :Mpls Tunnel Id: %d | NhId : %d", xpTnlMplsData->mplsIdx,
               xpTnlMplsData->nhId);
    }
    else
    {
        /* MPLS Tunnel config */
        retVal = xpsMplsCreateTunnelInterface(&tnlId);
        if (retVal != XP_NO_ERR)
        {
            printf("%s:Error in xpsMplsCreateTunnelInterface : error code: %d\n",
                   __FUNCTION__, retVal);
            return retVal;
        }

        mplsTnlKey = (xpsMplsTunnelKey_t *)malloc(sizeof(xpsMplsTunnelKey_t));
        if (mplsTnlKey == NULL)
        {
            printf("Error allocating memory for mplsTnlKey\n");
            return XP_ERR_MEM_ALLOC_ERROR;
        }
        memset(mplsTnlKey, 0x0, sizeof(xpsMplsTunnelKey_t));

        mplsTnlKey->numOfLables = xpTnlMplsData->numOfLables;
        if (mplsTnlKey->numOfLables == 1)
        {
            mplsTnlKey->firstLabel = xpTnlMplsData->label0;
        }
        else if (mplsTnlKey->numOfLables == 2)
        {
            mplsTnlKey->firstLabel = xpTnlMplsData->label0;
            mplsTnlKey->secondLabel = xpTnlMplsData->label1;
        }
        retVal = xpsMplsAddTunnelOriginationEntry(devId, mplsTnlKey, tnlId);
        if (retVal != XP_NO_ERR)
        {
            printf("%s:Error in xpsMplsAddTunnelOriginationEntry : error code: %d\n",
                   __FUNCTION__, retVal);
            free(mplsTnlKey);
            return retVal;
        }

        retVal = xpsMplsAddTunnelTerminationEntry(devId, mplsTnlKey);
        if (retVal != XP_NO_ERR)
        {
            printf("%s:Error in xpsMplsAddTunnelOriginationEntry : error code: %d\n",
                   __FUNCTION__, retVal);
            free(mplsTnlKey);
            return retVal;
        }

        //set tunnel params
        memset(&tunnelParams, 0x0, sizeof(xpsMplsTunnelParams_t));

        retVal = xpsMplsSetTunnelConfig(devId, mplsTnlKey, &tunnelParams);
        if (retVal != XP_NO_ERR)
        {
            printf("%s:Error in xpsMplsSetTunnelConfig : error code: %d\n", __FUNCTION__,
                   retVal);
            free(mplsTnlKey);
            return retVal;
        }
        free(mplsTnlKey);

        //NH programming

        void *r = xpSaiAppGetNhContext(devId, xpTnlMplsData->nhId);

        if (r)
        {
            nhIdx = (*(xpSaiAppNhContextMap **)r)->nhIdx;
        }
        else
        {
            printf("ERROR: Nh not available for tnl nh Id %d\n", xpTnlMplsData->nhId);
            return XP_ERR_INVALID_DATA;
        }

        retVal = xpsMplsSetTunnelNextHopData(devId, tnlId, nhIdx);
        if (retVal != XP_NO_ERR)
        {
            printf("%s:Error in xpsMplsSetTunnelNextHopData : error code: %d\n",
                   __FUNCTION__, retVal);
            return retVal;
        }
        allocatedMplsTnlVifMap *mplsMap = (allocatedMplsTnlVifMap *)malloc(sizeof(
                                                                               allocatedMplsTnlVifMap));
        if (mplsMap == NULL)
        {
            printf("Error allocating memory for mplsMap\n");
            return XP_ERR_MEM_ALLOC_ERROR;
        }
        mplsMap->key = xpTnlMplsData->mplsIdx;
        mplsMap->tnlId = tnlId;
        void *ptr = tsearch(mplsMap, &mplsTnlVifRoot[devId], compareMplsTnlVifMap);
        if (ptr == NULL)
        {
            printf("%s:Error: Can't insert tnlId in tree for allocated mpls %d\n",
                   __FUNCTION__, mplsMap->key);
            free(mplsMap);
            return XP_ERR_INIT;
        }
        printf("MPLS Tunnel Entry :Mpls Tunnel Id: %d | NhId : %d  | nhIdx : %d  |TnlInfId :%d\n ",
               xpTnlMplsData->mplsIdx, xpTnlMplsData->nhId, nhIdx, tnlId);
    }
    return XP_NO_ERR;
}

uint8_t xpSaiAppUnconfigureMplsTunnelEntry(xpDevice_t devId, uint32_t tunnelId,
                                           xpsMplsTunnelKey_t *mplsTnlKey)
{
    uint8_t retVal = XP_NO_ERR;

    retVal = xpsMplsRemoveTunnelOriginationEntry(devId, tunnelId);
    if (retVal != XP_NO_ERR)
    {
        printf("%s:Error in xpsMplsRemoveTunnelOriginationEntry : error code: %d\n",
               __FUNCTION__, retVal);
        return retVal;
    }

    retVal = xpsMplsRemoveTunnelTerminationEntry(devId, mplsTnlKey);
    if (retVal != XP_NO_ERR)
    {
        printf("%s:Error in xpsMplsRemoveTunnelTerminationEntry : error code: %d\n",
               __FUNCTION__, retVal);
        return retVal;
    }
    return XP_NO_ERR;
}
