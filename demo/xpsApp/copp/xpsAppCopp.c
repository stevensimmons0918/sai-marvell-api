// xpsAppCopp.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsAppCopp.h"

XP_STATUS xpsAppWriteCopp(xpDevice_t devId, CoppEntry *coppData)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpCoppEntryData_t entry;
    xpCoppEntryData_t getEntry;
    uint32_t index;
    xpsHashIndexList_t indexList;

    if (!coppData)
    {
        printf("%s:Error: Null pointer received \n", __FUNCTION__);
        return XP_ERR_NULL_POINTER;
    }

    entry.enPolicer = coppData->enPolicer;
    entry.policerId = coppData->policerId;
    entry.updatePktCmd = coppData->updatePktCmd;
    entry.pktCmd = coppData->pktCmd;
    entry.updateReasonCode = coppData->updateReasonCode;
    entry.reasonCode = coppData->reasonCode;

    index = indexList.index[0];
    printf("sourcePort:%d, reasonCodeKey:%d, enPolicer:%d, policerId:%d, updatePktCmd:%d, pktCmd:%d, updateReasonCode:%d, reasonCode:%d\n",
           coppData->sourcePort, coppData->reasonCodeKey, entry.enPolicer, entry.policerId,
           entry.updatePktCmd, entry.pktCmd, entry.updateReasonCode, entry.reasonCode);
    printf("xpsAppWriteCopp : Pass index:%d \n", index);

    printf("Perform CoPP Data lookup for the same entry\n");

    printf("Lookup: CoPP Table: sourcePort:%d, reasonCodeKey:%d, enPolicer:%d, policerId:%d, updatePktCmd:%d, pktCmd:%d, updateReasonCode:%d, reasonCode:%d\n",
           coppData->sourcePort, coppData->reasonCodeKey, getEntry.enPolicer,
           getEntry.policerId, getEntry.updatePktCmd, getEntry.pktCmd,
           getEntry.updateReasonCode, getEntry.reasonCode);

    return retVal;
}

