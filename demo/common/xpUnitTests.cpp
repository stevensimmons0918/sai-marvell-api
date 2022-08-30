// xpUnitTests.cpp

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpUnitTests.h"
#include "xpAppL3Uc.h"
#include "xpInit.h"

#if 0
void processCLI(int argc, char *argv[])
{
   XP_PROGNAME.assign(argv[0]);             // Save name of executable.
   XP_CONFIG()->getEnvValues();             // Process environment variables first,
   XP_CONFIG()->getOptValues(argc, argv);       // then process command line options,
   xpConfigFile::instance()->readFile(XP_CONFIG()->configFileName);     // then read configuration file.
}
#endif

xpPrimitiveMgr *tunnelIvifMgr;
xpPrimitiveMgr *fdbMgr;
xpPrimitiveMgr *portVlanMgr;
xpPrimitiveMgr *bdMgr;
xpPrimitiveMgr *mplsTunnelMgr;
xpPrimitiveMgr *acmBnkMgr;
xpPrimitiveMgr *acmRstMgr;
xpMdtMgr *mdtMgr;
xpPortConfigMgr *portCfgMgr;
xpEgressFilterMgr *egFltMgr;
xpTrunkResolutionMgr *trtMgr;
xpHdrModificationMgr *hdrModMgr;
xpTrunkResolutionMgr *trunkResolutionMgr;
static xpControlMacMgr *controlMacMgr;
xpIpRouteLpmMgr *ipv4RouteMgr = NULL, *ipv6RouteMgr = NULL;
static xpIpv4HostMgr *ipv4HostMgr;
static xpIpv6HostMgr *ipv6HostMgr;
xpIaclMgr *iaclMgr;
xpPipeSchedShaperCfgMgr *pipeSchedShaperCfgMgr;
xpPortSchedShaperCfgMgr *portSchedShaperCfgMgr;
xpPortMappingCfgMgr *portMappingCfgMgr;
xpQmappingMgr *qmappingMgr;
xpNATIpv4Mgr *natipv4Mgr;
xpTableProfile *tableProfile;

void compareVifEntry(xpVifEntry *tblEntry, xpVifEntry *getTblEntry)
{
    uint32_t countPorts;
    bool passStatus = true;

    if(tblEntry->type != getTblEntry->type)
    {
        passStatus = false;
        printf("Mismatch::type Expected Value %d Actual Value: %d \n", tblEntry->type, getTblEntry->type);
    }
    if(tblEntry->truncate != getTblEntry->truncate)
    {
        passStatus = false;
        printf("Mismatch::truncate Expected Value %d Actual Value: %d \n", tblEntry->truncate, getTblEntry->truncate);
    }
    if(tblEntry->mtuProfile != getTblEntry->mtuProfile)
    {
        passStatus = false;
        printf("Mismatch::mtuProfile Expected Value %d Actual Value: %d \n", tblEntry->mtuProfile, getTblEntry->mtuProfile);
    }
    if(tblEntry->mirrorEn != getTblEntry->mirrorEn)
    {
        passStatus = false;
        printf("Mismatch::mirrorEn Expected Value %d Actual Value: %d \n", tblEntry->mirrorEn, getTblEntry->mirrorEn);
    }
    if(tblEntry->qMirrorEn != getTblEntry->qMirrorEn)
    {
        passStatus = false;
        printf("Mismatch::qMirrorEn Expected Value %d Actual Value: %d \n", tblEntry->qMirrorEn, getTblEntry->qMirrorEn);
    }
    if(tblEntry->modPtr0 != getTblEntry->modPtr0)
    {
        passStatus = false;
        printf("Mismatch::modPtr0 Expected Value %d Actual Value: %d \n", tblEntry->modPtr0, getTblEntry->modPtr0);
    }
    if(tblEntry->modPtr1 != getTblEntry->modPtr1)
    {
        passStatus = false;
        printf("Mismatch::modPtr1 Expected Value %d Actual Value: %d \n", tblEntry->modPtr1, getTblEntry->modPtr1);
    }
    if(tblEntry->modPtr2 != getTblEntry->modPtr2)
    {
        passStatus = false;
        printf("Mismatch::modPtr2 Expected Value %d Actual Value: %d \n", tblEntry->modPtr2, getTblEntry->modPtr2);
    }
    if(tblEntry->modPtr3 != getTblEntry->modPtr3)
    {
        passStatus = false;
        printf("Mismatch::modPtr3 Expected Value %d Actual Value: %d \n", tblEntry->modPtr3, getTblEntry->modPtr3);
    }
    if(tblEntry->modPtr4 != getTblEntry->modPtr4)
    {
        passStatus = false;
        printf("Mismatch::modPtr4 Expected Value %d Actual Value: %d \n", tblEntry->modPtr4, getTblEntry->modPtr4);
    }
    if(tblEntry->modPtr5 != getTblEntry->modPtr5)
    {
        passStatus = false;
        printf("Mismatch::modPtr5 Expected Value %d Actual Value: %d \n", tblEntry->modPtr5, getTblEntry->modPtr5);
    }
    if(tblEntry->insPtr0 != getTblEntry->insPtr0)
    {
        passStatus = false;
        printf("Mismatch::insPtr0 Expected Value %d Actual Value: %d \n", tblEntry->insPtr0, getTblEntry->insPtr0);
    }
    if(tblEntry->insPtr1 != getTblEntry->insPtr1)
    {
        passStatus = false;
        printf("Mismatch::insPtr1 Expected Value %d Actual Value: %d \n", tblEntry->insPtr1, getTblEntry->insPtr1);
    }
    if(tblEntry->insPtr2 != getTblEntry->insPtr2)
    {
        passStatus = false;
        printf("Mismatch::insPtr2 Expected Value %d Actual Value: %d \n", tblEntry->insPtr2, getTblEntry->insPtr2);
    }
    if(tblEntry->data[0] != getTblEntry->data[0])
    {
        passStatus = false;
        printf("Mismatch::data[0] Expected Value %d Actual Value: %d \n", tblEntry->data[0], getTblEntry->data[0]);
    }
    if(tblEntry->data[1] != getTblEntry->data[1])
    {
        passStatus = false;
        printf("Mismatch::data[1] Expected Value %d Actual Value: %d \n", tblEntry->data[1], getTblEntry->data[1]);
    }
    if(tblEntry->data[2] != getTblEntry->data[2])
    {
        passStatus = false;
        printf("Mismatch::data[2] Expected Value %d Actual Value: %d \n", tblEntry->data[2], getTblEntry->data[2]);
    }
    if(tblEntry->data[3] != getTblEntry->data[3])
    {
        passStatus = false;
        printf("Mismatch::data[3] Expected Value %d Actual Value: %d \n", tblEntry->data[3], getTblEntry->data[3]);
    }
    if(tblEntry->data[4] != getTblEntry->data[4])
    {
        passStatus = false;
        printf("Mismatch::data[4] Expected Value %d Actual Value: %d \n", tblEntry->data[4], getTblEntry->data[4]);
    }
    if(tblEntry->data[5] != getTblEntry->data[5])
    {
        passStatus = false;
        printf("Mismatch::data[5] Expected Value %d Actual Value: %d \n", tblEntry->data[5], getTblEntry->data[5]);
    }
    if(tblEntry->data[6] != getTblEntry->data[6])
    {
        passStatus = false;
        printf("Mismatch::data[6] Expected Value %d Actual Value: %d \n", tblEntry->data[6], getTblEntry->data[6]);
    }
    if(tblEntry->data[7] != getTblEntry->data[7])
    {
        passStatus = false;
        printf("Mismatch::data[7] Expected Value %d Actual Value: %d \n", tblEntry->data[7], getTblEntry->data[7]);
    }
    if(tblEntry->dataSize != getTblEntry->dataSize)
    {
        passStatus = false;
        printf("Mismatch::dataSize Expected Value %d Actual Value: %d \n", tblEntry->dataSize, getTblEntry->dataSize);
    }
    if(tblEntry->mdtPtr != getTblEntry->mdtPtr)
    {
        passStatus = false;
        printf("Mismatch::mdtPtr Expected Value %d Actual Value: %d \n", tblEntry->mdtPtr, getTblEntry->mdtPtr);
    }

    if(tblEntry->portList && getTblEntry->portList)
    {
        if(tblEntry->portList->size() != getTblEntry->portList->size())
        {
            passStatus = false;
            printf("Mismatch::portList size Expected Value %d Actual Value: %d \n", (int)tblEntry->portList->size(), (int)getTblEntry->portList->size());
        } else
        {
            for(countPorts = 0; countPorts < tblEntry->portList->size(); countPorts++)
            {
                if((*tblEntry->portList)[countPorts] != (*getTblEntry->portList)[countPorts])
                {
                    passStatus = false;
                    printf("Mismatch::portList Expected Value %d Actual Value: %d \n", (*tblEntry->portList)[countPorts], (*getTblEntry->portList)[countPorts]);
                }
            }
        }
    }

    if(passStatus)
    {
        printf("compareVifEntry: PASS \n");
    } else
    {
        printf("compareVifEntry: FAIL \n");
    }
}

void printVifEntry(xpVifEntry *entry)
{
    uint32_t countPorts;

    printf("type :: %d \n", entry->type);
    printf("truncate :: %d \n", entry->truncate);
    printf("mtuProfile :: %d \n", entry->mtuProfile);
    printf("mirrorEn :: %d \n", entry->mirrorEn);
    printf("qMirrorEn :: %d \n", entry->qMirrorEn);
    printf("modPtr0 :: %d \n", entry->modPtr0);
    printf("modPtr1 :: %d \n", entry->modPtr1);
    printf("modPtr2 :: %d \n", entry->modPtr2);
    printf("modPtr3 :: %d \n", entry->modPtr3);
    printf("modPtr4 :: %d \n", entry->modPtr4);
    printf("modPtr5 :: %d \n", entry->modPtr5);
    printf("insPtr0 :: %d \n", entry->insPtr0);
    printf("insPtr1 :: %d \n", entry->insPtr1);
    printf("insPtr2 :: %d \n", entry->insPtr2);
    printf("data :: %d,%d,%d,%d,%d,%d,%d,%d \n", entry->data[0], entry->data[1], entry->data[2], entry->data[3],
           entry->data[4], entry->data[5], entry->data[6], entry->data[7]);
    printf("dataSize :: %d \n", entry->dataSize);
    printf("mdtPtr :: %d \n", entry->mdtPtr);

    if(entry->portList)
    {
        printf("PortList available \n");
        if(entry->portList->size() > 0)
        {
            printf("PortList :: %u \n", (unsigned int) entry->portList->size());
            for(countPorts = 0; countPorts < entry->portList->size(); countPorts++)
            {
                printf("ports : %u \n", (*entry->portList)[countPorts]);
            }
        } else printf("No Ports available in PortList \n");
    } else printf("PortList NULL \n");
}

/*
 * Sample API to test Vif Manager basic functionality
 */
#if 0
int testVifMgr(xpDevice_t devId, XP_PIPE_MODE mode)
{
    uint8_t tblCopyNum = tableProfile->getXpDaTblCopyNum(IVIF32_TBL);
    uint32_t vifSingle, vifSingleStart;
    uint32_t vifData, vifDataStart;
    uint32_t vifLarge, vifLargeStart;
    uint32_t vifLag, vifLagStart;
    uint32_t vifTunnel, vifTunnelStart;
    uint32_t vifMulti, vifMultiStart;
    uint32_t count, idx;
    xpVifEntry addEntryT32, addEntryT64, addEntryT128, addEntryT256;
    xpVifEntry readEntryT32, readEntryT64, readEntryT128, readEntryT256;
    xpVifEntry defEntry;

    uint32_t writeTruncate, readTruncate = 0;
    uint32_t writeMtuProfile, readMtuProfile = 0;
    uint32_t writeMirrorEn, readMirrorEn = 0;
    uint32_t writeQMirrorEn, readQMirrorEn = 0;
    std::vector<uint32_t> writeModPtr, readModPtr;
    std::vector<uint32_t> writeInsPtr, readInsPtr;
    uint8_t writeData[4], readData[4];
    uint32_t dataSize;
    uint32_t writeMdtPtr, readMdtPtr = 0;
    xpPortList_t writePortList, readPortList, remPortList;
    bool passStatus = true;
    XP_STATUS status = XP_NO_ERR;

    printf("dpsdbg: Inside %s func\n", __FUNCTION__);

    xpVifMgr *vifMgr = static_cast<xpVifMgr *>(xpIfPl::instance()->getMgr(xpVifMgr::getId()));
    
    vifSingleStart = 0;
    vifDataStart = vifSingleStart + ((mode == XP_SINGLE_PIPE) ? 46 * 1024 : 32 * 1024);
    vifLargeStart = vifDataStart + 2 * 1024;
    vifLagStart = vifLargeStart + ((mode == XP_SINGLE_PIPE) ? 4 * 1024 : 2 * 1024);
    vifTunnelStart = vifLagStart + ((mode == XP_SINGLE_PIPE) ? 4 * 1024 : 2 * 1024);
    vifMultiStart = vifTunnelStart + ((mode == XP_SINGLE_PIPE) ? 8 * 1024 : 4 * 1024);

    // Allocate Id
    // here are the function calls to allocate id, then use this id and call the APIs with corresponding directions
    // Ensure that when APIs are called with an ID allocated from the XP_VIF_MULTI_PORT pool and a direction of XP_INGRESS
    // return error!

    //
    // XP_VIF_SINGLE_PORT_NO_DATA
    //
    addEntryT32.type = 3;
    addEntryT32.truncate = 0;
    addEntryT32.mtuProfile = 4;
    addEntryT32.mirrorEn = 1;
    addEntryT32.qMirrorEn = 0;
    addEntryT32.modPtr0 = 0xaa;
    addEntryT32.modPtr1 = 0xbb;
    addEntryT32.dataSize = 0;
    addEntryT32.portList = new xpPortList_t;
    addEntryT32.portList->push_back(10);

    printf("********** XP_VIF_SINGLE_PORT_NO_DATA **********\n");

    for(count = 0; count < 100; count++)
    {
        vifMgr->allocateId(XP_VIF_SINGLE_PORT_NO_DATA, vifSingle);
        printf("allocateId(): vifSingle: %d\n", vifSingle);

        memcpy(&readEntryT32, &defEntry, sizeof(readEntryT32));
        readEntryT32.portList = new xpPortList_t;

        printf("\nwriteEntry(): egress: \n");
        vifMgr->writeEntry(devId, vifSingle, XP_INGRESS, &addEntryT32);
        readEntryT32.dataSize = 0;
        printf("\nreadEntry: egress: \n");
        vifMgr->readEntry(devId, vifSingle, XP_INGRESS, &readEntryT32, (tblCopyNum-1));
        printf("\ncompareVifEntry: egress: \n");
        compareVifEntry(&addEntryT32, &readEntryT32);
        //printVifEntry(&readEntryT32);

        for(int tblCopyIdx = 0; tblCopyIdx < tblCopyNum; tblCopyIdx++)
        {
            printf("\n32 bit ingress: Table %d\n", tblCopyIdx);
            vifMgr->writeEntry(devId, tblCopyIdx, vifSingle, XP_INGRESS, &addEntryT32);
            readEntryT32.dataSize = 0;
            readEntryT32.portList->clear();
            vifMgr->readEntry(devId, vifSingle, XP_INGRESS, &readEntryT32, tblCopyIdx);
            compareVifEntry(&addEntryT32, &readEntryT32);
            //printVifEntry(&readEntryT32);
        }

        delete readEntryT32.portList;
    }

    //
    // XP_VIF_SINGLE_PORT_DATA
    //
    addEntryT64.type = 3;
    addEntryT64.truncate = 1;
    addEntryT64.mtuProfile = 3;
    addEntryT64.mirrorEn = 1;
    addEntryT64.qMirrorEn = 0;
    addEntryT64.modPtr0 = 0x11;
    addEntryT64.modPtr1 = 0x22;
    addEntryT64.data[0] = 11;
    addEntryT64.data[1] = 12;
    addEntryT64.data[2] = 13;
    addEntryT64.data[3] = 14;
    addEntryT64.dataSize = 4;
    addEntryT64.portList = new xpPortList_t;
    addEntryT64.portList->push_back(10);

    printf("********** XP_VIF_SINGLE_PORT_DATA **********\n");

    for(count = 0; count < 100; count++)
    {
        vifMgr->allocateId(XP_VIF_SINGLE_PORT_DATA, vifData);
        printf("vifData: %d\n", vifData);

        memcpy(&readEntryT64, &defEntry, sizeof(readEntryT64));
        readEntryT64.portList = new xpPortList_t;

        vifMgr->writeEntry(devId, vifData, XP_EGRESS, &addEntryT64);
        readEntryT64.dataSize = 4;
        vifMgr->readEntry(devId, vifData, XP_EGRESS, &readEntryT64, (tblCopyNum-1));
        compareVifEntry(&addEntryT64, &readEntryT64);
        //printVifEntry(&readEntryT64);

        delete readEntryT64.portList;
    }

    //
    // XP_VIF_SINGLE_PORT_LARGE_DATA
    //
    addEntryT128.type = 3;
    addEntryT128.truncate = 1;
    addEntryT128.mtuProfile = 3;
    addEntryT128.mirrorEn = 1;
    addEntryT128.qMirrorEn = 0;
    addEntryT128.modPtr0 = 0x11;
    addEntryT128.modPtr1 = 0x22;
    addEntryT128.modPtr2 = 0x33;
    addEntryT128.modPtr3 = 0x44;
    addEntryT128.modPtr4 = 0x55;
    addEntryT128.modPtr5 = 0x66;
    addEntryT128.data[0] = 11;
    addEntryT128.data[1] = 12;
    addEntryT128.data[2] = 13;
    addEntryT128.data[3] = 14;
    addEntryT128.data[4] = 15;
    addEntryT128.data[5] = 16;
    addEntryT128.data[6] = 17;
    addEntryT128.data[7] = 18;
    addEntryT128.dataSize = 8;
    addEntryT128.portList = new xpPortList_t;
    addEntryT128.portList->push_back(10);

    printf("********** XP_VIF_SINGLE_PORT_LARGE_DATA **********\n");

    for(count = 0; count < 100; count++)
    {
        vifMgr->allocateId(XP_VIF_SINGLE_PORT_LARGE_DATA, vifLarge);
        printf("vifLarge: %d\n", vifLarge);

        memcpy(&readEntryT128, &defEntry, sizeof(readEntryT128));
        readEntryT128.portList = new xpPortList_t;

        vifMgr->writeEntry(devId, vifLarge, XP_EGRESS, &addEntryT128);
        readEntryT128.dataSize = 8;
        vifMgr->readEntry(devId, vifLarge, XP_EGRESS, &readEntryT128, (tblCopyNum-1));
        compareVifEntry(&addEntryT128, &readEntryT128);
        //printVifEntry(&readEntryT128);

        delete readEntryT128.portList;
    }

    //
    // XP_VIF_LAG
    //
    addEntryT256.type = 3;
    addEntryT256.truncate = 1;
    addEntryT256.mtuProfile = 3;
    addEntryT256.mirrorEn = 1;
    addEntryT256.qMirrorEn = 0;
    addEntryT256.modPtr0 = 0x11;
    addEntryT256.modPtr1 = 0x22;
    addEntryT256.insPtr0 = 21;
    addEntryT256.insPtr1 = 22;
    addEntryT256.insPtr2 = 23;
    addEntryT256.data[0] = 11;
    addEntryT256.data[1] = 12;
    addEntryT256.data[2] = 13;
    addEntryT256.data[3] = 14;
    addEntryT256.data[4] = 0;
    addEntryT256.data[5] = 0;
    addEntryT256.data[6] = 0;
    addEntryT256.data[7] = 0;
    addEntryT256.dataSize = 4;
    addEntryT256.portList = new xpPortList_t;
    addEntryT256.portList->push_back(11);
    addEntryT256.portList->push_back(12);
    addEntryT256.portList->push_back(13);
    addEntryT256.portList->push_back(31);
    addEntryT256.portList->push_back(32);
    addEntryT256.portList->push_back(35);
    addEntryT256.portList->push_back(21);
    addEntryT256.portList->push_back(79);
    addEntryT256.portList->push_back(114);
    addEntryT256.portList->push_back(105);
    addEntryT256.portList->push_back(112);
    std::sort(addEntryT256.portList->begin(), addEntryT256.portList->end());

    writeTruncate = 0x1;
    writeMtuProfile = 0x5;
    writeMirrorEn = 0;
    writeQMirrorEn = 1;
    writeModPtr.push_back(0x12);
    writeModPtr.push_back(0xDF);
    writeInsPtr.push_back(0xAABB);
    writeInsPtr.push_back(0x1122);
    writeInsPtr.push_back(0xFEFE);
    writeData[0] = 0xFF;
    writeData[1] = 0x11;
    writeData[2] = 0x34;
    writeData[3] = 0x45;
    dataSize = 4;
    writeMdtPtr = 0xFF11;
    writePortList.push_back(0);
    writePortList.push_back(20);
    writePortList.push_back(34);
    writePortList.push_back(2);
    writePortList.push_back(78);
    writePortList.push_back(104);
    writePortList.push_back(135);
    writePortList.push_back(122);
    std::sort(writePortList.begin(), writePortList.end());

    remPortList.push_back(104);
    remPortList.push_back(135);
    remPortList.push_back(2);
    remPortList.push_back(20);
    remPortList.push_back(34);
    printf("********** XP_VIF_LAG **********\n");

    for(count = 0; count < 100; count++)
    {
        memcpy(&readEntryT256, &defEntry, sizeof(readEntryT256));
        readEntryT256.portList = new xpPortList_t;

        //
        // XP_VIF_LAG
        //
        vifMgr->allocateId(XP_VIF_LAG, vifLag);
        printf("vifLag: %d\n", vifLag);

        vifMgr->writeEntry(devId, vifLag, XP_EGRESS, &addEntryT256);
        readEntryT256.dataSize = 4;
        vifMgr->readEntry(devId, vifLag, XP_EGRESS, &readEntryT256, (tblCopyNum-1));
        compareVifEntry(&addEntryT256, &readEntryT256);
        //printVifEntry(&readEntryT256);

        for(int tblCopyIdx = 0; tblCopyIdx < tblCopyNum; tblCopyIdx++)
        {
            printf("\n256 bit - vifLag Ingress: Table %d\n", tblCopyIdx);
            vifMgr->writeEntry(devId, tblCopyIdx, vifLag, XP_INGRESS, &addEntryT256);
            readEntryT256.dataSize = 4;
            readEntryT256.portList->clear();
            vifMgr->readEntry(devId, vifLag, XP_INGRESS, &readEntryT256, tblCopyIdx);
            compareVifEntry(&addEntryT256, &readEntryT256);
            //printVifEntry(&readEntryT256);
        }

        printf("XP_VIF_LAG EGRESS: setter/ getter\n");

        vifMgr->setTruncate(devId, vifLag, XP_EGRESS, writeTruncate);
        readTruncate = 0;
        vifMgr->getTruncate(devId, vifLag, XP_EGRESS, readTruncate, (tblCopyNum-1));
        if(writeTruncate != readTruncate)
        {
            passStatus = false;
            printf("Mismatch::truncate Expected Value %d Actual Value: %d \n", writeTruncate, readTruncate);
        }
        for(int tblCopyIdx = 0; tblCopyIdx < tblCopyNum; tblCopyIdx++)
        {
            printf("\n256 bit - vifLag MtuProfile: Table %d\n", tblCopyIdx);
            vifMgr->setMtuProfile(devId, tblCopyIdx, vifLag, XP_EGRESS, writeMtuProfile);
            readMtuProfile = 0;
            vifMgr->getMtuProfile(devId, vifLag, XP_EGRESS, readMtuProfile, tblCopyIdx);
            if(writeMtuProfile != readMtuProfile)
            {
                passStatus = false;
                printf("Mismatch::mtuProfile Expected Value %d Actual Value: %d \n", writeMtuProfile, readMtuProfile);
            }
        }
        vifMgr->setMirrorEn(devId, vifLag, XP_EGRESS, writeMirrorEn);
        readMirrorEn = 0;
        vifMgr->getMirrorEn(devId, vifLag, XP_EGRESS, readMirrorEn);
        if(writeMirrorEn != readMirrorEn)
        {
            passStatus = false;
            printf("Mismatch::mirrorEn Expected Value %d Actual Value: %d \n", writeMirrorEn, readMirrorEn);
        }
        vifMgr->setQMirrorEn(devId, vifLag, XP_EGRESS, writeQMirrorEn);
        readQMirrorEn = 0;
        vifMgr->getQMirrorEn(devId, vifLag, XP_EGRESS, readQMirrorEn);
        if(writeQMirrorEn != readQMirrorEn)
        {
            passStatus = false;
            printf("Mismatch::qMirrorEn Expected Value %d Actual Value: %d \n", writeQMirrorEn, readQMirrorEn);
        }
        vifMgr->setModPtr(devId, vifLag, XP_EGRESS, &writeModPtr);
        readModPtr.clear();
        vifMgr->getModPtr(devId, vifLag, XP_EGRESS, &readModPtr, (tblCopyNum-1));
        if(writeModPtr.size() != readModPtr.size())
        {
            passStatus = false;
            printf("Mismatch::modPtr size Expected Value %d Actual Value: %d \n", (int)writeModPtr.size(), (int)readModPtr.size());
        }
        for(idx = 0; idx < readModPtr.size(); idx++)
        {
            if(writeModPtr[idx] != readModPtr[idx])
            {
                passStatus = false;
                printf("Mismatch::modPtr[%d] Expected Value %d Actual Value: %d \n", idx, writeModPtr[idx], readModPtr[idx]);
            }
        }
        vifMgr->setInsPtr(devId, vifLag, XP_EGRESS, &writeInsPtr);
        readInsPtr.clear();
        vifMgr->getInsPtr(devId, vifLag, XP_EGRESS, &readInsPtr);
        if(writeInsPtr.size() != readInsPtr.size())
        {
            passStatus = false;
            printf("Mismatch::insPtr size Expected Value %d Actual Value: %d \n", (int)writeInsPtr.size(), (int)readInsPtr.size());
        }
        for(idx = 0; idx < readInsPtr.size(); idx++)
        {
            if(writeInsPtr != readInsPtr)
            {
                passStatus = false;
                printf("Mismatch::insPtr[%d] Expected Value %d Actual Value: %d \n", idx, writeInsPtr[idx], readInsPtr[idx]);
            }
        }
        vifMgr->setData(devId, vifLag, XP_EGRESS, dataSize, writeData);
        memset(readData, 0x0, sizeof(readData));
        vifMgr->getData(devId, vifLag, XP_EGRESS, dataSize, readData);
        for(idx = 0; idx < dataSize; idx++)
        {
            if(writeData[idx] != readData[idx])
            {
                passStatus = false;
                printf("Mismatch::data[%d] Expected Value %d Actual Value: %d \n", idx, writeData[idx], readData[idx]);
            }
        }
        vifMgr->setMdtPtr(devId, vifLag, XP_EGRESS, writeMdtPtr);
        readMdtPtr = 0;
        vifMgr->getMdtPtr(devId, vifLag, XP_EGRESS, readMdtPtr);
        if(writeMdtPtr != readMdtPtr)
        {
            passStatus = false;
            printf("Mismatch::mdtPtr Expected Value %d Actual Value: %d \n", writeMdtPtr, readMdtPtr);
        }
        vifMgr->setPort(devId, vifLag, XP_EGRESS, &writePortList);
        readPortList.clear();
        vifMgr->getPort(devId, vifLag, XP_EGRESS, &readPortList);
        if(writePortList.size() != readPortList.size())
        {
            passStatus = false;
            printf("Mismatch::portList size Expected Value %d Actual Value: %d \n", (int)writePortList.size(), (int)readPortList.size());
        }
        for(idx = 0; idx < readPortList.size(); idx++)
        {
            if(writePortList[idx] != readPortList[idx])
            {
                passStatus = false;
                printf("Mismatch::portList[%d] Expected Value %d Actual Value: %d \n", idx, writePortList[idx], readPortList[idx]);
            }
        }

        vifMgr->removePort(devId, vifLag, XP_EGRESS, &remPortList);
        readPortList.clear();
        vifMgr->getPort(devId, vifLag, XP_EGRESS, &readPortList, (tblCopyNum-1));
        printf("Portlist after removing ports: ");
        for(idx = 0; idx < readPortList.size(); idx++)
        {
            printf(" %d \t", readPortList[idx]);
        }
        printf("\n");

        if(passStatus)
        {
            printf("XP_VIF_LAG EGRESS: setter/ getter: PASS \n");
        } else
        {
            printf("XP_VIF_LAG EGRESS: setter/ getter: FAIL \n");
        }

        //
        // XP_VIF_TUNNEL_ENCAP
        //
        vifMgr->allocateId(XP_VIF_TUNNEL_ENCAP, vifTunnel);
        printf("vifTunnel: %d\n", vifTunnel);

        vifMgr->writeEntry(devId, vifTunnel, XP_EGRESS, &addEntryT256);
        readEntryT256.dataSize = 4;
        readEntryT256.portList->clear();
        vifMgr->readEntry(devId, vifTunnel, XP_EGRESS, &readEntryT256);
        compareVifEntry(&addEntryT256, &readEntryT256);
        //printVifEntry(&readEntryT256);

        for(int tblCopyIdx = 0; tblCopyIdx < tblCopyNum; tblCopyIdx++)
        {
            printf("\n256 bit - vifTunnel ingress: Table %d\n", tblCopyIdx);
            vifMgr->writeEntry(devId, tblCopyIdx, vifTunnel, XP_INGRESS, &addEntryT256);
            readEntryT256.dataSize = 4;
            readEntryT256.portList->clear();
            vifMgr->readEntry(devId, vifTunnel, XP_INGRESS, &readEntryT256, tblCopyIdx);
            compareVifEntry(&addEntryT256, &readEntryT256);
            //printVifEntry(&readEntryT256);
        }

        //
        // XP_VIF_MULTI_PORT
        //
        vifMgr->allocateId(XP_VIF_MULTI_PORT, vifMulti);
        printf("vifMulti: %d\n", vifMulti);

        vifMgr->writeEntry(devId, vifMulti, XP_EGRESS, &addEntryT256);
        readEntryT256.dataSize = 4;
        readEntryT256.portList->clear();
        vifMgr->readEntry(devId, vifMulti, XP_EGRESS, &readEntryT256, (tblCopyNum-1));
        compareVifEntry(&addEntryT256, &readEntryT256);
        //printVifEntry(&readEntryT256);

        for(int idx = 0; idx < tblCopyNum; idx++)
        {
            printf("\n256 bit - vifMulti ingress: Table %d\n", idx);
            status = vifMgr->writeEntry(devId, idx, vifMulti, XP_INGRESS, &addEntryT256);
            if(status == XP_NO_ERR)
            {
                readEntryT256.dataSize = 4;
                readEntryT256.portList->clear();
                vifMgr->readEntry(devId, vifMulti, XP_INGRESS, &readEntryT256, idx);
                compareVifEntry(&addEntryT256, &readEntryT256);
                //printVifEntry(&readEntryT256);
            }
        }

        delete readEntryT256.portList;
    }

    /*for(count = 0; count < 13 * 1024; count++)
    {
        status = vifMgr->allocateId(XP_VIF_LAG, vifLag);
        if(status != XP_NO_ERR)
        {
            printf("Allocate ID failed vifLag %d : count %d status %d \n", vifLag, count, status);
            continue;
        }
        //printf("Allocated ID vifLag: %d\n", vifLag);
    }*/

    for(count = vifSingleStart; count < 100; count++)
    {
        //printf("\nreleaseId(): vifId: %d\n", count);
        status = vifMgr->releaseId(count);
        if(status != XP_NO_ERR) printf("Release ID failed count %d : status %d \n", count, status);
    }


    for(count = vifDataStart; count < 100; count++)
    {
        //printf("\nreleaseId(): vifId: %d\n", count);
        status = vifMgr->releaseId(count);
        if(status != XP_NO_ERR) printf("Release ID failed count %d : status %d \n", count, status);
    }

    for(count = vifLargeStart; count < 100; count++)
    {
        //printf("\nreleaseId(): vifId: %d\n", count);
        status = vifMgr->releaseId(count);
        if(status != XP_NO_ERR) printf("Release ID failed count %d : status %d \n", count, status);
    }

    for(count = vifTunnelStart; count < 100; count++)
    {
        //printf("\nreleaseId(): vifId: %d\n", count);
        status = vifMgr->releaseId(count);
        if(status != XP_NO_ERR) printf("Release ID failed count %d : status %d \n", count, status);
    }

    for(count = vifLagStart; count < 100; count++)
    {
        //printf("\nreleaseId(): vifId: %d\n", count);
        status = vifMgr->releaseId(count);
        if(status != XP_NO_ERR) printf("Release ID failed count %d : status %d \n", count, status);
    }

    for(count = vifMultiStart; count < 100; count++)
    {
        //printf("\nreleaseId(): vifId: %d\n", count);
        status = vifMgr->releaseId(count);
        if(status != XP_NO_ERR) printf("Release ID failed count %d : status %d \n", count, status);
    }

    // -ve test case
    /*status = vifMgr->allocateId(XP_VIF_LAG, vifLag);
    if(status != XP_NO_ERR) printf("Allocate ID failed vifLag %d : status %d \n", vifLag, status);
    */
    //DPS: We also need to implement the for loop for writing into XP_VIF_TUNNEL_ENCAP
    //and XP_VIF_MULTI_PORT. for Tunnel we should be able to write into both ingress/egress table
    //while for MULTI_PORT we should be able to write only on egress table since there is no ingress table
    //allocated by Primitive manager for this vif type. When trying to write in ingress table, we should get
    //an error from VIF manager PM. So please add these test implementation here.

    delete addEntryT32.portList;
    delete addEntryT64.portList;
    delete addEntryT128.portList;
    delete addEntryT256.portList;

    return 0;
}
#endif
/*********************** INSERTION MGR Test Application **********************************/
void compareInsertionEntry(xpInsertionEntry *tblEntry, xpInsertionEntry *getTblEntry)
{
    uint32_t idx;
    bool passStatus = true;

    if(tblEntry->EntryType != getTblEntry->EntryType)
    {
        passStatus = false;
        printf("Mismatch::EntryType Expected Value %d Actual Value: %d \n", tblEntry->EntryType, getTblEntry->EntryType);
    }
    if(tblEntry->insertInstPtr != getTblEntry->insertInstPtr)
    {
        passStatus = false;
        printf("Mismatch::insertInstPtr Expected Value %d Actual Value: %d \n", tblEntry->insertInstPtr, getTblEntry->insertInstPtr);
    }
    if(tblEntry->InsertDataSz != getTblEntry->InsertDataSz)
    {
        passStatus = false;
        printf("Mismatch::InsertDataSz Expected Value %d Actual Value: %d \n", tblEntry->InsertDataSz, getTblEntry->InsertDataSz);
    }
    for(idx = 0; idx < MAX_INSERT_DATA_SZ; idx++)
    {
        if(tblEntry->InsertData[idx] != getTblEntry->InsertData[idx])
        {
            passStatus = false;
            printf("Mismatch::InsertData[%d] Expected Value %d Actual Value: %d \n", idx, tblEntry->InsertData[idx], getTblEntry->InsertData[idx]);
        }
    }
    if(passStatus)
    {
        printf("compareInsertionEntry: PASS \n");
    } else
    {
        printf("compareInsertionEntry: FAIL \n");
    }
}

void printInsertionEntry(xpInsertionEntry *entry)
{
    uint32_t idx;

    printf("EntryType :: %d \n", entry->EntryType);
    printf("insertInstPtr :: %d \n", entry->insertInstPtr);
    printf("InsertDataSz :: %d \n", entry->InsertDataSz);
    for(idx = 0; idx < MAX_INSERT_DATA_SZ; idx++)
    {
        printf("data[%d] :: %d \t", idx, entry->InsertData[idx]);
    }
}

/*
 * Sample API to test Insertion Manager basic functionality
 */
#if 0
int testInsertionMgr(xpHdrModificationMgr *hdrModMgr, xpDevice_t devId, XP_PIPE_MODE mode)
{
    uint32_t insertion0, insertion0Start;
    uint32_t insertion1, insertion1Start;
    uint32_t insertion2, insertion2Start;
    uint32_t count, idx, dataSize;
    xpInsertionEntry addEntryT0, addEntryT1, addEntryT2;
    xpInsertionEntry readEntryT0, readEntryT1, readEntryT2;
    uint32_t writeIITPtr, readIITPtr;
    uint8_t writeData[MAX_INSERT_DATA_SZ], readData[MAX_INSERT_DATA_SZ];
    bool passStatus = true;
    XP_STATUS status = XP_NO_ERR;

    printf("dpsdbg: Inside %s func\n", __FUNCTION__);

    // Allocate Id
    // here are the function calls to allocate id, then use this id and call the APIs with corresponding directions
    //
    // XP_INSERTION0
    //
    memset(&addEntryT0, 0, sizeof(addEntryT0));
    addEntryT0.EntryType = 1;
    addEntryT0.insertInstPtr = 0xFE;
    addEntryT0.InsertDataSz = 6;
    addEntryT0.InsertData[0] = 0x45;
    addEntryT0.InsertData[1] = 0x22;
    addEntryT0.InsertData[2] = 0x3;
    addEntryT0.InsertData[3] = 0x78;
    addEntryT0.InsertData[4] = 0x9F;
    addEntryT0.InsertData[5] = 0x69;

    writeIITPtr = 0x13;
    dataSize = 6;
    writeData[0] = 0x63;
    writeData[1] = 0xFF;
    writeData[2] = 0xDA;
    writeData[3] = 0xBC;
    writeData[4] = 0xA3;
    writeData[5] = 0x10;

    printf("********** XP_INSERTION0 **********\n");

    for(count = 0; count < 100; count++)
    {
        hdrModMgr->allocateInsPtrId(XP_INSERTION0, insertion0);
        printf("allocateInsPtrId(): insertion0: %d\n", insertion0);
        if(count == 0) insertion0Start = insertion0;
        memset(&readEntryT0, 0, sizeof(readEntryT0));

        hdrModMgr->writeInsertionEntry(devId, insertion0, &addEntryT0);
        readEntryT0.InsertDataSz = 6;
        hdrModMgr->readInsertionEntry(devId, insertion0, &readEntryT0);
        compareInsertionEntry(&addEntryT0, &readEntryT0);
        //printInsertionEntry(&readEntryT0);

        printf("XP_INSERTION0: setter/ getter\n");
        hdrModMgr->setIITPtr(devId, insertion0, writeIITPtr);
        readIITPtr = 0;
        hdrModMgr->getIITPtr(devId, insertion0, readIITPtr);
        if(writeIITPtr != readIITPtr)
        {
            passStatus = false;
            printf("Mismatch::mirrorEn Expected Value %d Actual Value: %d \n", writeIITPtr, readIITPtr);
        }
        hdrModMgr->setInsertData(devId, insertion0, dataSize, writeData);
        memset(readData, 0x0, sizeof(readData));
        hdrModMgr->getInsertData(devId, insertion0, dataSize, readData);
        for(idx = 0; idx < dataSize; idx++)
        {
            if(writeData[idx] != readData[idx])
            {
                passStatus = false;
                printf("Mismatch::data[%d] Expected Value %d Actual Value: %d \n", idx, writeData[idx], readData[idx]);
            }
        }
        if(passStatus)
        {
            printf("XP_INSERTION0: setter/ getter: PASS \n");
        } else
        {
            printf("XP_INSERTION0: setter/ getter: FAIL \n");
        }
        passStatus = true;
    }

    //
    // XP_INSERTION1
    //
    memset(&addEntryT1, 0, sizeof(addEntryT1));
    addEntryT1.EntryType = 2;
    addEntryT1.insertInstPtr = 0xBB;
    addEntryT1.InsertDataSz = 14;
    addEntryT1.InsertData[0] = 0x23;
    addEntryT1.InsertData[1] = 0x11;
    addEntryT1.InsertData[2] = 0x54;
    addEntryT1.InsertData[3] = 0xFA;
    addEntryT1.InsertData[4] = 0xB6;
    addEntryT1.InsertData[5] = 0xFF;
    addEntryT1.InsertData[6] = 0xEF;
    addEntryT1.InsertData[7] = 0x76;
    addEntryT1.InsertData[8] = 0x90;
    addEntryT1.InsertData[9] = 0x1;
    addEntryT1.InsertData[10] = 0xA4;
    addEntryT1.InsertData[11] = 0xB5;
    addEntryT1.InsertData[12] = 0x1C;
    addEntryT1.InsertData[13] = 0xD;

    writeIITPtr = 0xFC;
    dataSize = 14;
    writeData[0] = 0x93;
    writeData[1] = 0xAF;
    writeData[2] = 0x1A;
    writeData[3] = 0xA2;
    writeData[4] = 0xA9;
    writeData[5] = 0x1A;
    writeData[6] = 0x67;
    writeData[7] = 0x79;
    writeData[8] = 0x99;
    writeData[9] = 0x10;
    writeData[10] = 0x4B;
    writeData[11] = 0xFA;
    writeData[12] = 0x25;
    writeData[13] = 0xD0;

    printf("********** XP_INSERTION1 **********\n");

    for(count = 0; count < 100; count++)
    {
        hdrModMgr->allocateInsPtrId(XP_INSERTION1, insertion1);
        printf("insertion1: %d\n", insertion1);
        if(count == 0) insertion1Start = insertion1;
        memset(&readEntryT1, 0, sizeof(readEntryT1));

        hdrModMgr->writeInsertionEntry(devId, insertion1, &addEntryT1);
        readEntryT1.InsertDataSz = 14;
        hdrModMgr->readInsertionEntry(devId, insertion1, &readEntryT1);
        compareInsertionEntry(&addEntryT1, &readEntryT1);
        //printInsertionEntry(&readEntryT1);

        printf("XP_INSERTION1: setter/ getter\n");
        hdrModMgr->setIITPtr(devId, insertion1, writeIITPtr);
        readIITPtr = 0;
        hdrModMgr->getIITPtr(devId, insertion1, readIITPtr);
        if(writeIITPtr != readIITPtr)
        {
            passStatus = false;
            printf("Mismatch::mirrorEn Expected Value %d Actual Value: %d \n", writeIITPtr, readIITPtr);
        }
        hdrModMgr->setInsertData(devId, insertion1, dataSize, writeData);
        memset(readData, 0x0, sizeof(readData));
        hdrModMgr->getInsertData(devId, insertion1, dataSize, readData);
        for(idx = 0; idx < dataSize; idx++)
        {
            if(writeData[idx] != readData[idx])
            {
                passStatus = false;
                printf("Mismatch::data[%d] Expected Value %d Actual Value: %d \n", idx, writeData[idx], readData[idx]);
            }
        }
        if(passStatus)
        {
            printf("XP_INSERTION1: setter/ getter: PASS \n");
        } else
        {
            printf("XP_INSERTION1: setter/ getter: FAIL \n");
        }
        passStatus = true;
    }

    //
    // XP_INSERTION2
    //
    memset(&addEntryT2, 0, sizeof(addEntryT2));
    addEntryT2.EntryType = 3;
    addEntryT2.insertInstPtr = 0x99;
    addEntryT2.InsertDataSz = 30;
    for(idx = 0; idx < addEntryT2.InsertDataSz; idx++)
    {
        addEntryT2.InsertData[idx] = idx + 10;
    }

    writeIITPtr = 0xBF;
    dataSize = 30;
    for(idx = 0; idx < dataSize; idx++)
    {
        writeData[idx] = idx * 2 + 3;
    }

    printf("********** XP_INSERTION2 **********\n");

    for(count = 0; count < 100; count++)
    {
        hdrModMgr->allocateInsPtrId(XP_INSERTION2, insertion2);
        printf("insertion2: %d\n", insertion2);
        if(count == 0) insertion2Start = insertion2;
        memset(&readEntryT2, 0, sizeof(readEntryT2));

        hdrModMgr->writeInsertionEntry(devId, insertion2, &addEntryT2);
        readEntryT2.InsertDataSz = 30;
        hdrModMgr->readInsertionEntry(devId, insertion2, &readEntryT2);
        compareInsertionEntry(&addEntryT2, &readEntryT2);
        //printInsertionEntry(&readEntryT2);

        printf("XP_INSERTION2: setter/ getter\n");
        hdrModMgr->setIITPtr(devId, insertion2, writeIITPtr);
        readIITPtr = 0;
        hdrModMgr->getIITPtr(devId, insertion2, readIITPtr);
        if(writeIITPtr != readIITPtr)
        {
            passStatus = false;
            printf("Mismatch::mirrorEn Expected Value %d Actual Value: %d \n", writeIITPtr, readIITPtr);
        }
        hdrModMgr->setInsertData(devId, insertion2, dataSize, writeData);
        memset(readData, 0x0, sizeof(readData));
        hdrModMgr->getInsertData(devId, insertion2, dataSize, readData);
        for(idx = 0; idx < dataSize; idx++)
        {
            if(writeData[idx] != readData[idx])
            {
                passStatus = false;
                printf("Mismatch::data[%d] Expected Value %d Actual Value: %d \n", idx, writeData[idx], readData[idx]);
            }
        }
        if(passStatus)
        {
            printf("XP_INSERTION2: setter/ getter: PASS \n");
        } else
        {
            printf("XP_INSERTION2: setter/ getter: FAIL \n");
        }
        passStatus = true;
    }

    for(count = insertion0Start; count < 400; count++)
    {
        //printf("\nreleaseId(): insertionId: %d\n", count);
        status = hdrModMgr->releaseInsPtrId(count);
        if(status != XP_NO_ERR) printf("Release ID failed count %d : status %d \n", count, status);
    }

    for(count = insertion1Start; count < 400; count++)
    {
        //printf("\nreleaseId(): insertionId: %d\n", count);
        status = hdrModMgr->releaseInsPtrId(count);
        if(status != XP_NO_ERR) printf("Release ID failed count %d : status %d \n", count, status);
    }
    for(count = insertion2Start; count < 400; count++)
    {
        //printf("\nreleaseId(): insertionId: %d\n", count);
        status = hdrModMgr->releaseInsPtrId(count);
        if(status != XP_NO_ERR) printf("Release ID failed count %d : status %d \n", count, status);
    }

    return 0;
}
#endif
/*********************** header Modification MGR Test Application **********************************/
void compareMITEntry(xpMITEntry *tblEntry, xpMITEntry *getTblEntry)
{
    bool passStatus = true;

    if(tblEntry->cmdNum != getTblEntry->cmdNum)
    {
        passStatus = false;
        printf("Mismatch::cmdNum Expected Value %d Actual Value: %d \n", tblEntry->cmdNum, getTblEntry->cmdNum);
    }
    if(tblEntry->order != getTblEntry->order)
    {
        passStatus = false;
        printf("Mismatch::order Expected Value %d Actual Value: %d \n", tblEntry->order, getTblEntry->order);
    }
    if(tblEntry->layer != getTblEntry->layer)
    {
        passStatus = false;
        printf("Mismatch::layer Expected Value %d Actual Value: %d \n", tblEntry->layer, getTblEntry->layer);
    }
    if(tblEntry->cmd != getTblEntry->cmd)
    {
        passStatus = false;
        printf("Mismatch::cmd Expected Value %d Actual Value: %d \n", tblEntry->cmd, getTblEntry->cmd);
    } else
    {
        switch(tblEntry->cmd)
        {
#if 0
        case MIT_COPY_CMD:
            if(tblEntry->instruction.constData[0] != getTblEntry->instruction.constData[0])
            {
                passStatus = false;
                printf("Mismatch::constData[0] Expected Value %d Actual Value: %d \n", tblEntry->instruction.constData[0], getTblEntry->instruction.constData[0]);
            }
            if(tblEntry->instruction.constData[1] != getTblEntry->instruction.constData[1])
            {
                passStatus = false;
                printf("Mismatch::constData[1] Expected Value %d Actual Value: %d \n", tblEntry->instruction.constData[1], getTblEntry->instruction.constData[1]);
            }
            if(tblEntry->instruction.constData[2] != getTblEntry->instruction.constData[2])
            {
                passStatus = false;
                printf("Mismatch::constData[2] Expected Value %d Actual Value: %d \n", tblEntry->instruction.constData[2], getTblEntry->instruction.constData[2]);
            }
            if(tblEntry->instruction.constData[3] != getTblEntry->instruction.constData[3])
            {
                passStatus = false;
                printf("Mismatch::constData[3] Expected Value %d Actual Value: %d \n", tblEntry->instruction.constData[3], getTblEntry->instruction.constData[3]);
            }
            if(tblEntry->instruction.constData[4] != getTblEntry->instruction.constData[4])
            {
                passStatus = false;
                printf("Mismatch::constData[4] Expected Value %d Actual Value: %d \n", tblEntry->instruction.constData[4], getTblEntry->instruction.constData[4]);
            }
            if(tblEntry->instruction.constData[5] != getTblEntry->instruction.constData[5])
            {
                passStatus = false;
                printf("Mismatch::constData[5] Expected Value %d Actual Value: %d \n", tblEntry->instruction.constData[5], getTblEntry->instruction.constData[5]);
            }
            if(tblEntry->instruction.constData[6] != getTblEntry->instruction.constData[6])
            {
                passStatus = false;
                printf("Mismatch::constData[6] Expected Value %d Actual Value: %d \n", tblEntry->instruction.constData[6], getTblEntry->instruction.constData[6]);
            }
            if(tblEntry->instruction.constData[7] != getTblEntry->instruction.constData[7])
            {
                passStatus = false;
                printf("Mismatch::constData[7] Expected Value %d Actual Value: %d \n", tblEntry->instruction.constData[7], getTblEntry->instruction.constData[7]);
            }
            if(tblEntry->instruction.bitmaskLSB != getTblEntry->instruction.bitmaskLSB)
            {
                passStatus = false;
                printf("Mismatch::bitmaskLSB Expected Value %d Actual Value: %d \n", tblEntry->instruction.bitmaskLSB, getTblEntry->instruction.bitmaskLSB);
            }
            if(tblEntry->instruction.bitmask != getTblEntry->instruction.bitmask)
            {
                passStatus = false;
                printf("Mismatch::bitmask Expected Value %d Actual Value: %d \n", tblEntry->instruction.bitmask, getTblEntry->instruction.bitmask);
            }
            if(tblEntry->instruction.target != getTblEntry->instruction.target)
            {
                passStatus = false;
                printf("Mismatch::target Expected Value %d Actual Value: %d \n", tblEntry->instruction.target, getTblEntry->instruction.target);
            }
            if(tblEntry->instruction.size != getTblEntry->instruction.size)
            {
                passStatus = false;
                printf("Mismatch::size Expected Value %d Actual Value: %d \n", tblEntry->instruction.size, getTblEntry->instruction.size);
            }
            if(tblEntry->instruction.start != getTblEntry->instruction.start)
            {
                passStatus = false;
                printf("Mismatch::start Expected Value %d Actual Value: %d \n", tblEntry->instruction.start, getTblEntry->instruction.start);
            }
            if(tblEntry->instruction.source != getTblEntry->instruction.source)
            {
                passStatus = false;
                printf("Mismatch::source Expected Value %d Actual Value: %d \n", tblEntry->instruction.source, getTblEntry->instruction.source);
            }
            break;
        case MIT_DEL_CMD:
            if(tblEntry->instruction.size != getTblEntry->instruction.size)
            {
                passStatus = false;
                printf("Mismatch::size Expected Value %d Actual Value: %d \n", tblEntry->instruction.size, getTblEntry->instruction.size);
            }
            if(tblEntry->instruction.start != getTblEntry->instruction.start)
            {
                passStatus = false;
                printf("Mismatch::start Expected Value %d Actual Value: %d \n", tblEntry->instruction.start, getTblEntry->instruction.start);
            }
            break;
#endif
        default:
            break;
        }
    }
    if(passStatus)
    {
        printf("compareMITEntry: PASS \n");
    } else
    {
        printf("compareMITEntry: FAIL \n");
    }
}

void printMITEntry(xpMITEntry *entry)
{
    printf("cmdNum :: 0x%x \n", entry->cmdNum);
    printf("order :: 0x%x \n", entry->order);
    printf("layer :: 0x%x \n", entry->layer);
    switch(entry->cmd)
    {
    case MIT_COPY_CMD:
        printf("Copy instruction\n");
        printf("entry->instruction.constData[0] :: 0x%x \n", entry->instruction.constData[0]);
        printf("entry->instruction.constData[1] :: 0x%x \n", entry->instruction.constData[1]);
        printf("entry->instruction.constData[2] :: 0x%x \n", entry->instruction.constData[2]);
        printf("entry->instruction.constData[3] :: 0x%x \n", entry->instruction.constData[3]);
        printf("entry->instruction.constData[4] :: 0x%x \n", entry->instruction.constData[4]);
        printf("entry->instruction.constData[5] :: 0x%x \n", entry->instruction.constData[5]);
        printf("entry->instruction.constData[6] :: 0x%x \n", entry->instruction.constData[6]);
        printf("entry->instruction.constData[7] :: 0x%x \n", entry->instruction.constData[7]);
        printf("entry->instruction.bitmaskLSB :: 0x%x \n", entry->instruction.bitmaskLSB);
        printf("entry->instruction.bitmask :: 0x%x \n", entry->instruction.bitmask);
        printf("entry->instruction.target :: 0x%x \n", entry->instruction.target);
        printf("entry->instruction.size :: 0x%x \n", entry->instruction.size);
        printf("entry->instruction.start :: 0x%x \n", entry->instruction.start);
        printf("entry->instruction.source :: 0x%x \n", entry->instruction.source);
        break;
    case MIT_DEL_CMD:
        printf("Del instruction\n");
        printf("entry->instruction.size :: 0x%x \n", entry->instruction.size);
        printf("entry->instruction.start :: 0x%x \n", entry->instruction.start);
        break;
    default:
        break;
    }
    printf("cmd :: 0x%x \n", entry->cmd);
}

void compareIITEntry(xpIITEntry *tblEntry, xpIITEntry *getTblEntry)
{
    bool passStatus = true;

    if(tblEntry->ptr2constData != getTblEntry->ptr2constData)
    {
        passStatus = false;
        printf("Mismatch::ptr2constData Expected Value %d Actual Value: %d \n", tblEntry->ptr2constData, getTblEntry->ptr2constData);
    }
    if(tblEntry->lengthAdjust != getTblEntry->lengthAdjust)
    {
        passStatus = false;
        printf("Mismatch::lengthAdjust Expected Value %d Actual Value: %d \n", tblEntry->lengthAdjust, getTblEntry->lengthAdjust);
    }
    if(tblEntry->lyrChksumInfo != getTblEntry->lyrChksumInfo)
    {
        passStatus = false;
        printf("Mismatch::lyrChksumInfo Expected Value %d Actual Value: %d \n", tblEntry->lyrChksumInfo, getTblEntry->lyrChksumInfo);
    }
    if(tblEntry->insertCmd7.bitmaskLSB != getTblEntry->insertCmd7.bitmaskLSB)
    {
        passStatus = false;
        printf("Mismatch::insertCmd7.bitmaskLSB Expected Value %d Actual Value: %d \n", tblEntry->insertCmd7.bitmaskLSB, getTblEntry->insertCmd7.bitmaskLSB);
    }
    if(tblEntry->insertCmd7.bitmask != getTblEntry->insertCmd7.bitmask)
    {
        passStatus = false;
        printf("Mismatch::insertCmd7.bitmask Expected Value %d Actual Value: %d \n", tblEntry->insertCmd7.bitmask, getTblEntry->insertCmd7.bitmask);
    }
    if(tblEntry->insertCmd7.target != getTblEntry->insertCmd7.target)
    {
        passStatus = false;
        printf("Mismatch::insertCmd7.target Expected Value %d Actual Value: %d \n", tblEntry->insertCmd7.target, getTblEntry->insertCmd7.target);
    }
    if(tblEntry->insertCmd7.size != getTblEntry->insertCmd7.size)
    {
        passStatus = false;
        printf("Mismatch::insertCmd7.size Expected Value %d Actual Value: %d \n", tblEntry->insertCmd7.size, getTblEntry->insertCmd7.size);
    }
    if(tblEntry->insertCmd7.start != getTblEntry->insertCmd7.start)
    {
        passStatus = false;
        printf("Mismatch::insertCmd7.start Expected Value %d Actual Value: %d \n", tblEntry->insertCmd7.start, getTblEntry->insertCmd7.start);
    }
    if(tblEntry->insertCmd7.source != getTblEntry->insertCmd7.source)
    {
        passStatus = false;
        printf("Mismatch::insertCmd7.source Expected Value %d Actual Value: %d \n", tblEntry->insertCmd7.source, getTblEntry->insertCmd7.source);
    }
    if(tblEntry->insertCmd7.cmd != getTblEntry->insertCmd7.cmd)
    {
        passStatus = false;
        printf("Mismatch::insertCmd7.cmd Expected Value %d Actual Value: %d \n", tblEntry->insertCmd7.cmd, getTblEntry->insertCmd7.cmd);
    }
    if(tblEntry->insertCmd6.bitmaskLSB != getTblEntry->insertCmd6.bitmaskLSB)
    {
        passStatus = false;
        printf("Mismatch::insertCmd6.bitmaskLSB Expected Value %d Actual Value: %d \n", tblEntry->insertCmd6.bitmaskLSB, getTblEntry->insertCmd6.bitmaskLSB);
    }
    if(tblEntry->insertCmd6.bitmask != getTblEntry->insertCmd6.bitmask)
    {
        passStatus = false;
        printf("Mismatch::insertCmd6.bitmask Expected Value %d Actual Value: %d \n", tblEntry->insertCmd6.bitmask, getTblEntry->insertCmd6.bitmask);
    }
    if(tblEntry->insertCmd6.target != getTblEntry->insertCmd6.target)
    {
        passStatus = false;
        printf("Mismatch::insertCmd6.target Expected Value %d Actual Value: %d \n", tblEntry->insertCmd6.target, getTblEntry->insertCmd6.target);
    }
    if(tblEntry->insertCmd6.size != getTblEntry->insertCmd6.size)
    {
        passStatus = false;
        printf("Mismatch::insertCmd6.size Expected Value %d Actual Value: %d \n", tblEntry->insertCmd6.size, getTblEntry->insertCmd6.size);
    }
    if(tblEntry->insertCmd6.start != getTblEntry->insertCmd6.start)
    {
        passStatus = false;
        printf("Mismatch::insertCmd6.start Expected Value %d Actual Value: %d \n", tblEntry->insertCmd6.start, getTblEntry->insertCmd6.start);
    }
    if(tblEntry->insertCmd6.source != getTblEntry->insertCmd6.source)
    {
        passStatus = false;
        printf("Mismatch::insertCmd6.source Expected Value %d Actual Value: %d \n", tblEntry->insertCmd6.source, getTblEntry->insertCmd6.source);
    }
    if(tblEntry->insertCmd6.cmd != getTblEntry->insertCmd6.cmd)
    {
        passStatus = false;
        printf("Mismatch::insertCmd6.cmd Expected Value %d Actual Value: %d \n", tblEntry->insertCmd6.cmd, getTblEntry->insertCmd6.cmd);
    }
    if(tblEntry->insertCmd5.bitmaskLSB != getTblEntry->insertCmd5.bitmaskLSB)
    {
        passStatus = false;
        printf("Mismatch::insertCmd5.bitmaskLSB Expected Value %d Actual Value: %d \n", tblEntry->insertCmd5.bitmaskLSB, getTblEntry->insertCmd5.bitmaskLSB);
    }
    if(tblEntry->insertCmd5.bitmask != getTblEntry->insertCmd5.bitmask)
    {
        passStatus = false;
        printf("Mismatch::insertCmd5.bitmask Expected Value %d Actual Value: %d \n", tblEntry->insertCmd5.bitmask, getTblEntry->insertCmd5.bitmask);
    }
    if(tblEntry->insertCmd5.target != getTblEntry->insertCmd5.target)
    {
        passStatus = false;
        printf("Mismatch::insertCmd5.target Expected Value %d Actual Value: %d \n", tblEntry->insertCmd5.target, getTblEntry->insertCmd5.target);
    }
    if(tblEntry->insertCmd5.size != getTblEntry->insertCmd5.size)
    {
        passStatus = false;
        printf("Mismatch::insertCmd5.size Expected Value %d Actual Value: %d \n", tblEntry->insertCmd5.size, getTblEntry->insertCmd5.size);
    }
    if(tblEntry->insertCmd5.start != getTblEntry->insertCmd5.start)
    {
        passStatus = false;
        printf("Mismatch::insertCmd5.start Expected Value %d Actual Value: %d \n", tblEntry->insertCmd5.start, getTblEntry->insertCmd5.start);
    }
    if(tblEntry->insertCmd5.source != getTblEntry->insertCmd5.source)
    {
        passStatus = false;
        printf("Mismatch::insertCmd5.source Expected Value %d Actual Value: %d \n", tblEntry->insertCmd5.source, getTblEntry->insertCmd5.source);
    }
    if(tblEntry->insertCmd5.cmd != getTblEntry->insertCmd5.cmd)
    {
        passStatus = false;
        printf("Mismatch::insertCmd5.cmd Expected Value %d Actual Value: %d \n", tblEntry->insertCmd5.cmd, getTblEntry->insertCmd5.cmd);
    }
    if(tblEntry->insertCmd4.bitmaskLSB != getTblEntry->insertCmd4.bitmaskLSB)
    {
        passStatus = false;
        printf("Mismatch::insertCmd4.bitmaskLSB Expected Value %d Actual Value: %d \n", tblEntry->insertCmd4.bitmaskLSB, getTblEntry->insertCmd4.bitmaskLSB);
    }
    if(tblEntry->insertCmd4.bitmask != getTblEntry->insertCmd4.bitmask)
    {
        passStatus = false;
        printf("Mismatch::insertCmd4.bitmask Expected Value %d Actual Value: %d \n", tblEntry->insertCmd4.bitmask, getTblEntry->insertCmd4.bitmask);
    }
    if(tblEntry->insertCmd4.target != getTblEntry->insertCmd4.target)
    {
        passStatus = false;
        printf("Mismatch::insertCmd4.target Expected Value %d Actual Value: %d \n", tblEntry->insertCmd4.target, getTblEntry->insertCmd4.target);
    }
    if(tblEntry->insertCmd4.size != getTblEntry->insertCmd4.size)
    {
        passStatus = false;
        printf("Mismatch::insertCmd4.size Expected Value %d Actual Value: %d \n", tblEntry->insertCmd4.size, getTblEntry->insertCmd4.size);
    }
    if(tblEntry->insertCmd4.start != getTblEntry->insertCmd4.start)
    {
        passStatus = false;
        printf("Mismatch::insertCmd4.start Expected Value %d Actual Value: %d \n", tblEntry->insertCmd4.start, getTblEntry->insertCmd4.start);
    }
    if(tblEntry->insertCmd4.source != getTblEntry->insertCmd4.source)
    {
        passStatus = false;
        printf("Mismatch::insertCmd4.source Expected Value %d Actual Value: %d \n", tblEntry->insertCmd4.source, getTblEntry->insertCmd4.source);
    }
    if(tblEntry->insertCmd4.cmd != getTblEntry->insertCmd4.cmd)
    {
        passStatus = false;
        printf("Mismatch::insertCmd4.cmd Expected Value %d Actual Value: %d \n", tblEntry->insertCmd4.cmd, getTblEntry->insertCmd4.cmd);
    }
    if(tblEntry->insertCmd3.bitmaskLSB != getTblEntry->insertCmd3.bitmaskLSB)
    {
        passStatus = false;
        printf("Mismatch::insertCmd3.bitmaskLSB Expected Value %d Actual Value: %d \n", tblEntry->insertCmd3.bitmaskLSB, getTblEntry->insertCmd3.bitmaskLSB);
    }
    if(tblEntry->insertCmd3.bitmask != getTblEntry->insertCmd3.bitmask)
    {
        passStatus = false;
        printf("Mismatch::insertCmd3.bitmask Expected Value %d Actual Value: %d \n", tblEntry->insertCmd3.bitmask, getTblEntry->insertCmd3.bitmask);
    }
    if(tblEntry->insertCmd3.target != getTblEntry->insertCmd3.target)
    {
        passStatus = false;
        printf("Mismatch::insertCmd3.target Expected Value %d Actual Value: %d \n", tblEntry->insertCmd3.target, getTblEntry->insertCmd3.target);
    }
    if(tblEntry->insertCmd3.size != getTblEntry->insertCmd3.size)
    {
        passStatus = false;
        printf("Mismatch::insertCmd3.size Expected Value %d Actual Value: %d \n", tblEntry->insertCmd3.size, getTblEntry->insertCmd3.size);
    }
    if(tblEntry->insertCmd3.start != getTblEntry->insertCmd3.start)
    {
        passStatus = false;
        printf("Mismatch::insertCmd3.start Expected Value %d Actual Value: %d \n", tblEntry->insertCmd3.start, getTblEntry->insertCmd3.start);
    }
    if(tblEntry->insertCmd3.source != getTblEntry->insertCmd3.source)
    {
        passStatus = false;
        printf("Mismatch::insertCmd3.source Expected Value %d Actual Value: %d \n", tblEntry->insertCmd3.source, getTblEntry->insertCmd3.source);
    }
    if(tblEntry->insertCmd3.cmd != getTblEntry->insertCmd3.cmd)
    {
        passStatus = false;
        printf("Mismatch::insertCmd3.cmd Expected Value %d Actual Value: %d \n", tblEntry->insertCmd3.cmd, getTblEntry->insertCmd3.cmd);
    }
    if(tblEntry->insertCmd2.bitmaskLSB != getTblEntry->insertCmd2.bitmaskLSB)
    {
        passStatus = false;
        printf("Mismatch::insertCmd2.bitmaskLSB Expected Value %d Actual Value: %d \n", tblEntry->insertCmd2.bitmaskLSB, getTblEntry->insertCmd2.bitmaskLSB);
    }
    if(tblEntry->insertCmd2.bitmask != getTblEntry->insertCmd2.bitmask)
    {
        passStatus = false;
        printf("Mismatch::insertCmd2.bitmask Expected Value %d Actual Value: %d \n", tblEntry->insertCmd2.bitmask, getTblEntry->insertCmd2.bitmask);
    }
    if(tblEntry->insertCmd2.target != getTblEntry->insertCmd2.target)
    {
        passStatus = false;
        printf("Mismatch::insertCmd2.target Expected Value %d Actual Value: %d \n", tblEntry->insertCmd2.target, getTblEntry->insertCmd2.target);
    }
    if(tblEntry->insertCmd2.size != getTblEntry->insertCmd2.size)
    {
        passStatus = false;
        printf("Mismatch::insertCmd2.size Expected Value %d Actual Value: %d \n", tblEntry->insertCmd2.size, getTblEntry->insertCmd2.size);
    }
    if(tblEntry->insertCmd2.start != getTblEntry->insertCmd2.start)
    {
        passStatus = false;
        printf("Mismatch::insertCmd2.start Expected Value %d Actual Value: %d \n", tblEntry->insertCmd2.start, getTblEntry->insertCmd2.start);
    }
    if(tblEntry->insertCmd2.source != getTblEntry->insertCmd2.source)
    {
        passStatus = false;
        printf("Mismatch::insertCmd2.source Expected Value %d Actual Value: %d \n", tblEntry->insertCmd2.source, getTblEntry->insertCmd2.source);
    }
    if(tblEntry->insertCmd2.cmd != getTblEntry->insertCmd2.cmd)
    {
        passStatus = false;
        printf("Mismatch::insertCmd2.cmd Expected Value %d Actual Value: %d \n", tblEntry->insertCmd2.cmd, getTblEntry->insertCmd2.cmd);
    }
    if(tblEntry->insertCmd1.bitmaskLSB != getTblEntry->insertCmd1.bitmaskLSB)
    {
        passStatus = false;
        printf("Mismatch::insertCmd1.bitmaskLSB Expected Value %d Actual Value: %d \n", tblEntry->insertCmd1.bitmaskLSB, getTblEntry->insertCmd1.bitmaskLSB);
    }
    if(tblEntry->insertCmd1.bitmask != getTblEntry->insertCmd1.bitmask)
    {
        passStatus = false;
        printf("Mismatch::insertCmd1.bitmask Expected Value %d Actual Value: %d \n", tblEntry->insertCmd1.bitmask, getTblEntry->insertCmd1.bitmask);
    }
    if(tblEntry->insertCmd1.target != getTblEntry->insertCmd1.target)
    {
        passStatus = false;
        printf("Mismatch::insertCmd1.target Expected Value %d Actual Value: %d \n", tblEntry->insertCmd1.target, getTblEntry->insertCmd1.target);
    }
    if(tblEntry->insertCmd1.size != getTblEntry->insertCmd1.size)
    {
        passStatus = false;
        printf("Mismatch::insertCmd1.size Expected Value %d Actual Value: %d \n", tblEntry->insertCmd1.size, getTblEntry->insertCmd1.size);
    }
    if(tblEntry->insertCmd1.start != getTblEntry->insertCmd1.start)
    {
        passStatus = false;
        printf("Mismatch::insertCmd1.start Expected Value %d Actual Value: %d \n", tblEntry->insertCmd1.start, getTblEntry->insertCmd1.start);
    }
    if(tblEntry->insertCmd1.source != getTblEntry->insertCmd1.source)
    {
        passStatus = false;
        printf("Mismatch::insertCmd1.source Expected Value %d Actual Value: %d \n", tblEntry->insertCmd1.source, getTblEntry->insertCmd1.source);
    }
    if(tblEntry->insertCmd1.cmd != getTblEntry->insertCmd1.cmd)
    {
        passStatus = false;
        printf("Mismatch::insertCmd1.cmd Expected Value %d Actual Value: %d \n", tblEntry->insertCmd1.cmd, getTblEntry->insertCmd1.cmd);
    }
    if(tblEntry->insertCmd0.bitmaskLSB != getTblEntry->insertCmd0.bitmaskLSB)
    {
        passStatus = false;
        printf("Mismatch::insertCmd0.bitmaskLSB Expected Value %d Actual Value: %d \n", tblEntry->insertCmd0.bitmaskLSB, getTblEntry->insertCmd0.bitmaskLSB);
    }
    if(tblEntry->insertCmd0.bitmask != getTblEntry->insertCmd0.bitmask)
    {
        passStatus = false;
        printf("Mismatch::insertCmd0.bitmask Expected Value %d Actual Value: %d \n", tblEntry->insertCmd0.bitmask, getTblEntry->insertCmd0.bitmask);
    }
    if(tblEntry->insertCmd0.target != getTblEntry->insertCmd0.target)
    {
        passStatus = false;
        printf("Mismatch::insertCmd0.target Expected Value %d Actual Value: %d \n", tblEntry->insertCmd0.target, getTblEntry->insertCmd0.target);
    }
    if(tblEntry->insertCmd0.size != getTblEntry->insertCmd0.size)
    {
        passStatus = false;
        printf("Mismatch::insertCmd0.size Expected Value %d Actual Value: %d \n", tblEntry->insertCmd0.size, getTblEntry->insertCmd0.size);
    }
    if(tblEntry->insertCmd0.start != getTblEntry->insertCmd0.start)
    {
        passStatus = false;
        printf("Mismatch::insertCmd0.start Expected Value %d Actual Value: %d \n", tblEntry->insertCmd0.start, getTblEntry->insertCmd0.start);
    }
    if(tblEntry->insertCmd0.source != getTblEntry->insertCmd0.source)
    {
        passStatus = false;
        printf("Mismatch::insertCmd0.source Expected Value %d Actual Value: %d \n", tblEntry->insertCmd0.source, getTblEntry->insertCmd0.source);
    }
    if(tblEntry->insertCmd0.cmd != getTblEntry->insertCmd0.cmd)
    {
        passStatus = false;
        printf("Mismatch::insertCmd0.cmd Expected Value %d Actual Value: %d \n", tblEntry->insertCmd0.cmd, getTblEntry->insertCmd0.cmd);
    }
    if(tblEntry->totalSize != getTblEntry->totalSize)
    {
        passStatus = false;
        printf("Mismatch::totalSize Expected Value %d Actual Value: %d \n", tblEntry->totalSize, getTblEntry->totalSize);
    }
    if(tblEntry->sourceLayer != getTblEntry->sourceLayer)
    {
        passStatus = false;
        printf("Mismatch::sourceLayer Expected Value %d Actual Value: %d \n", tblEntry->sourceLayer, getTblEntry->sourceLayer);
    }
    if(passStatus)
    {
        printf("compareIITEntry: PASS \n");
    } else
    {
        printf("compareIITEntry: FAIL \n");
    }
}

void printIITEntry(xpIITEntry *entry)
{
    printf("ptr2constData :: 0x%x \n", entry->ptr2constData);
    printf("lengthAdjust :: 0x%x \n", entry->lengthAdjust);
    printf("lyrChksumInfo :: 0x%x \n", entry->lyrChksumInfo);
    printf("insertCmd7.bitmaskLSB :: 0x%x \n", entry->insertCmd7.bitmaskLSB);
    printf("insertCmd7.bitmask :: 0x%x \n", entry->insertCmd7.bitmask);
    printf("insertCmd7.target :: 0x%x \n", entry->insertCmd7.target);
    printf("insertCmd7.size :: 0x%x \n", entry->insertCmd7.size);
    printf("insertCmd7.start :: 0x%x \n", entry->insertCmd7.start);
    printf("insertCmd7.source :: 0x%x \n", entry->insertCmd7.source);
    printf("insertCmd7.cmd :: 0x%x \n", entry->insertCmd7.cmd);
    printf("insertCmd6.bitmaskLSB :: 0x%x \n", entry->insertCmd6.bitmaskLSB);
    printf("insertCmd6.bitmask :: 0x%x \n", entry->insertCmd6.bitmask);
    printf("insertCmd6.target :: 0x%x \n", entry->insertCmd6.target);
    printf("insertCmd6.size :: 0x%x \n", entry->insertCmd6.size);
    printf("insertCmd6.start :: 0x%x \n", entry->insertCmd6.start);
    printf("insertCmd6.source :: 0x%x \n", entry->insertCmd6.source);
    printf("insertCmd6.cmd :: 0x%x \n", entry->insertCmd6.cmd);
    printf("insertCmd5.bitmaskLSB :: 0x%x \n", entry->insertCmd5.bitmaskLSB);
    printf("insertCmd5.bitmask :: 0x%x \n", entry->insertCmd5.bitmask);
    printf("insertCmd5.target :: 0x%x \n", entry->insertCmd5.target);
    printf("insertCmd5.size :: 0x%x \n", entry->insertCmd5.size);
    printf("insertCmd5.start :: 0x%x \n", entry->insertCmd5.start);
    printf("insertCmd5.source :: 0x%x \n", entry->insertCmd5.source);
    printf("insertCmd5.cmd :: 0x%x \n", entry->insertCmd5.cmd);
    printf("insertCmd4.bitmaskLSB :: 0x%x \n", entry->insertCmd4.bitmaskLSB);
    printf("insertCmd4.bitmask :: 0x%x \n", entry->insertCmd4.bitmask);
    printf("insertCmd4.target :: 0x%x \n", entry->insertCmd4.target);
    printf("insertCmd4.size :: 0x%x \n", entry->insertCmd4.size);
    printf("insertCmd4.start :: 0x%x \n", entry->insertCmd4.start);
    printf("insertCmd4.source :: 0x%x \n", entry->insertCmd4.source);
    printf("insertCmd4.cmd :: 0x%x \n", entry->insertCmd4.cmd);
    printf("insertCmd3.bitmaskLSB :: 0x%x \n", entry->insertCmd3.bitmaskLSB);
    printf("insertCmd3.bitmask :: 0x%x \n", entry->insertCmd3.bitmask);
    printf("insertCmd3.target :: 0x%x \n", entry->insertCmd3.target);
    printf("insertCmd3.size :: 0x%x \n", entry->insertCmd3.size);
    printf("insertCmd3.start :: 0x%x \n", entry->insertCmd3.start);
    printf("insertCmd3.source :: 0x%x \n", entry->insertCmd3.source);
    printf("insertCmd3.cmd :: 0x%x \n", entry->insertCmd3.cmd);
    printf("insertCmd2.bitmaskLSB :: 0x%x \n", entry->insertCmd2.bitmaskLSB);
    printf("insertCmd2.bitmask :: 0x%x \n", entry->insertCmd2.bitmask);
    printf("insertCmd2.target :: 0x%x \n", entry->insertCmd2.target);
    printf("insertCmd2.size :: 0x%x \n", entry->insertCmd2.size);
    printf("insertCmd2.start :: 0x%x \n", entry->insertCmd2.start);
    printf("insertCmd2.source :: 0x%x \n", entry->insertCmd2.source);
    printf("insertCmd2.cmd :: 0x%x \n", entry->insertCmd2.cmd);
    printf("insertCmd1.bitmaskLSB :: 0x%x \n", entry->insertCmd1.bitmaskLSB);
    printf("insertCmd1.bitmask :: 0x%x \n", entry->insertCmd1.bitmask);
    printf("insertCmd1.target :: 0x%x \n", entry->insertCmd1.target);
    printf("insertCmd1.size :: 0x%x \n", entry->insertCmd1.size);
    printf("insertCmd1.start :: 0x%x \n", entry->insertCmd1.start);
    printf("insertCmd1.source :: 0x%x \n", entry->insertCmd1.source);
    printf("insertCmd1.cmd :: 0x%x \n", entry->insertCmd1.cmd);
    printf("insertCmd0.bitmaskLSB :: 0x%x \n", entry->insertCmd0.bitmaskLSB);
    printf("insertCmd0.bitmask :: 0x%x \n", entry->insertCmd0.bitmask);
    printf("insertCmd0.target :: 0x%x \n", entry->insertCmd0.target);
    printf("insertCmd0.size :: 0x%x \n", entry->insertCmd0.size);
    printf("insertCmd0.start :: 0x%x \n", entry->insertCmd0.start);
    printf("insertCmd0.source :: 0x%x \n", entry->insertCmd0.source);
    printf("insertCmd0.cmd :: 0x%x \n", entry->insertCmd0.cmd);
    printf("totalSize :: 0x%x \n", entry->totalSize);
    printf("sourceLayer :: 0x%x \n", entry->sourceLayer);
}

void compareICDTEntry(xpInsertConstantDataEntry *tblEntry, xpInsertConstantDataEntry *getTblEntry)
{
    uint32_t idx;
    bool passStatus = true;

    if(tblEntry->Bitmap != getTblEntry->Bitmap)
    {
        passStatus = false;
        printf("Mismatch::Bitmap Expected Value %d Actual Value: %d \n", tblEntry->Bitmap, getTblEntry->Bitmap);
    }
    for(idx = 0; idx < 32; idx++)
    {
        if(tblEntry->Data[idx] != getTblEntry->Data[idx])
        {
            passStatus = false;
            printf("Mismatch::Data[%d] Expected Value %d Actual Value: %d \n", idx, tblEntry->Data[idx], getTblEntry->Data[idx]);
        }
    }
    if(passStatus)
    {
        printf("compareICDTEntry: PASS \n");
    } else
    {
        printf("compareICDTEntry: FAIL \n");
    }
}

void printICDTEntry(xpInsertConstantDataEntry *entry)
{
    uint32_t idx;

    printf("Bitmap :: 0x%x \n", entry->Bitmap);
    for(idx = 0; idx < 32; idx++)
    {
        printf("data[%d] :: 0x%x \t", idx, entry->Data[idx]);
    }
}

/*
 * Sample API to test HeaderModification Manager basic functionality
 */
#if 0
int testHeaderModificationMgr(xpDevice_t devId, XP_PIPE_MODE mode)
{
    uint32_t count;
    xpMITEntry addMITEntry, readMITEntry;
    xpIITEntry addIITEntry, readIITEntry;
    xpInsertConstantDataEntry addICDTEntry, readICDTEntry;

    printf("dpsdbg: Inside %s func\n", __FUNCTION__);

    xpHdrModificationMgr *hdrModMgr = static_cast<xpHdrModificationMgr *>(xpIfPl::instance()->getMgr(xpHdrModificationMgr::getId()));

    for(count = 0; count < 10; count++)
    {
        memset(&readMITEntry, 0, sizeof(readMITEntry));
        printf("\nreadModifyInstructionEntry: \n");
        hdrModMgr->readModifyInstructionEntry(devId, count, &readMITEntry);
        printMITEntry(&readMITEntry);
    }

    for(count = 0; count < 10; count++)
    {
        memset(&readIITEntry, 0, sizeof(readIITEntry));
        printf("\nreadInsertInstructionEntry(): \n");
        hdrModMgr->readInsertInstructionEntry(devId, count, &readIITEntry);
        printIITEntry(&readIITEntry);
    }

    for(count = 0; count < 10; count++)
    {
        memset(&readICDTEntry, 0, sizeof(readICDTEntry));
        printf("\nreadInsertConstantDataEntry(): \n");
        hdrModMgr->readInsertConstantDataEntry(devId, count, &readICDTEntry);
        printICDTEntry(&readICDTEntry);
    }

    //
    // MIT table
    //
    printf("********** MIT: copy instruction **********\n");

    memset(&addMITEntry, 0, sizeof(addMITEntry));
    addMITEntry.cmdNum = 2;
    addMITEntry.order = 0;
    addMITEntry.layer = 3;
    addMITEntry.cmd = MIT_COPY_CMD;
    addMITEntry.instruction.constData[0] = 0x11;
    addMITEntry.instruction.constData[1] = 0x43;
    addMITEntry.instruction.constData[2] = 0xCD;
    addMITEntry.instruction.constData[3] = 0xF1;
    addMITEntry.instruction.constData[4] = 0x18;
    addMITEntry.instruction.constData[5] = 0x90;
    addMITEntry.instruction.constData[6] = 0xAF;
    addMITEntry.instruction.constData[7] = 0xFF;
    addMITEntry.instruction.bitmaskLSB = 0x1;
    addMITEntry.instruction.bitmask = 0xFF;
    addMITEntry.instruction.target = 0x5;
    addMITEntry.instruction.size = 0x2;
    addMITEntry.instruction.start = 0x2F;
    addMITEntry.instruction.source = 0xF;

    for(count = 0; count < 10; count++)
    {
        memset(&readMITEntry, 0, sizeof(readMITEntry));

        printf("\nwriteModifyInstructionEntry(): \n");
        hdrModMgr->writeModifyInstructionEntry(devId, count, &addMITEntry);
        printf("\nreadModifyInstructionEntry: \n");
        hdrModMgr->readModifyInstructionEntry(devId, count, &readMITEntry);
        compareMITEntry(&addMITEntry, &readMITEntry);
        //printMITEntry(&readMITEntry);
    }

    printf("********** MIT: del instruction **********\n");

    memset(&addMITEntry, 0, sizeof(addMITEntry));
    addMITEntry.cmdNum = 1;
    addMITEntry.order = 1;
    addMITEntry.layer = 1;
    addMITEntry.cmd = MIT_DEL_CMD;
    addMITEntry.instruction.size = 0x2;
    addMITEntry.instruction.start = 0x2F;

    for(count = 10; count < 20; count++)
    {
        memset(&readMITEntry, 0, sizeof(readMITEntry));

        printf("\nwriteModifyInstructionEntry(): \n");
        hdrModMgr->writeModifyInstructionEntry(devId, count, &addMITEntry);
        printf("\nreadModifyInstructionEntry: \n");
        hdrModMgr->readModifyInstructionEntry(devId, count, &readMITEntry);
        compareMITEntry(&addMITEntry, &readMITEntry);
        //printMITEntry(&readMITEntry);
    }

    printf("********** MIT: nop instruction **********\n");

    memset(&addMITEntry, 0, sizeof(addMITEntry));
    addMITEntry.cmdNum = 3;
    addMITEntry.order = 0;
    addMITEntry.layer = 0;
    addMITEntry.cmd = MIT_NOP_CMD;

    for(count = 20; count < 30; count++)
    {
        memset(&readMITEntry, 0, sizeof(readMITEntry));

        printf("\nwriteModifyInstructionEntry(): \n");
        hdrModMgr->writeModifyInstructionEntry(devId, count, &addMITEntry);
        printf("\nreadModifyInstructionEntry: \n");
        hdrModMgr->readModifyInstructionEntry(devId, count, &readMITEntry);
        compareMITEntry(&addMITEntry, &readMITEntry);
        //printMITEntry(&readMITEntry);
    }
    //
    // IIT table
    //
    memset(&addIITEntry, 0, sizeof(addIITEntry));
    addIITEntry.ptr2constData = 0x1D;
    addIITEntry.lengthAdjust = 0xE;
    addIITEntry.lyrChksumInfo = 0x5;
    addIITEntry.insertCmd7.bitmaskLSB = 0x0;
    addIITEntry.insertCmd7.bitmask = 0x12;
    addIITEntry.insertCmd7.target = 0x3;
    addIITEntry.insertCmd7.size = 0x2;
    addIITEntry.insertCmd7.start = 0x2D;
    addIITEntry.insertCmd7.source = 0x3;
    addIITEntry.insertCmd7.cmd = 0x1;
    addIITEntry.insertCmd6.bitmaskLSB = 0x1;
    addIITEntry.insertCmd6.bitmask = 0x15;
    addIITEntry.insertCmd6.target = 0x2;
    addIITEntry.insertCmd6.size = 0x0;
    addIITEntry.insertCmd6.start = 0x1E;
    addIITEntry.insertCmd6.source = 0x2;
    addIITEntry.insertCmd6.cmd = 0x0;
    addIITEntry.insertCmd5.bitmaskLSB = 0x0;
    addIITEntry.insertCmd5.bitmask = 0x32;
    addIITEntry.insertCmd5.target = 0x6;
    addIITEntry.insertCmd5.size = 0x6;
    addIITEntry.insertCmd5.start = 0x16;
    addIITEntry.insertCmd5.source = 0x4;
    addIITEntry.insertCmd5.cmd = 0x0;
    addIITEntry.insertCmd4.bitmaskLSB = 0x1;
    addIITEntry.insertCmd4.bitmask = 0xD2;
    addIITEntry.insertCmd4.target = 0x16;
    addIITEntry.insertCmd4.size = 0x7;
    addIITEntry.insertCmd4.start = 0xF;
    addIITEntry.insertCmd4.source = 0x6;
    addIITEntry.insertCmd4.cmd = 0x1;
    addIITEntry.insertCmd3.bitmaskLSB = 0x0;
    addIITEntry.insertCmd3.bitmask = 0x19;
    addIITEntry.insertCmd3.target = 0x2;
    addIITEntry.insertCmd3.size = 0x1;
    addIITEntry.insertCmd3.start = 0x1B;
    addIITEntry.insertCmd3.source = 0x4;
    addIITEntry.insertCmd3.cmd = 0x0;
    addIITEntry.insertCmd2.bitmaskLSB = 0x0;
    addIITEntry.insertCmd2.bitmask = 0xF6;
    addIITEntry.insertCmd2.target = 0x1A;
    addIITEntry.insertCmd2.size = 0x6;
    addIITEntry.insertCmd2.start = 0x21;
    addIITEntry.insertCmd2.source = 0x7;
    addIITEntry.insertCmd2.cmd = 0x0;
    addIITEntry.insertCmd1.bitmaskLSB = 0x1;
    addIITEntry.insertCmd1.bitmask = 0xAA;
    addIITEntry.insertCmd1.target = 0x1A;
    addIITEntry.insertCmd1.size = 0x1;
    addIITEntry.insertCmd1.start = 0xD;
    addIITEntry.insertCmd1.source = 0x5;
    addIITEntry.insertCmd1.cmd = 0x1;
    addIITEntry.insertCmd0.bitmaskLSB = 0x1;
    addIITEntry.insertCmd0.bitmask = 0xD1;
    addIITEntry.insertCmd0.target = 0x4;
    addIITEntry.insertCmd0.size = 0x4;
    addIITEntry.insertCmd0.start = 0x1F;
    addIITEntry.insertCmd0.source = 0x5;
    addIITEntry.insertCmd0.cmd = 0x0;
    addIITEntry.totalSize = 0x2F;
    addIITEntry.sourceLayer = 0x4;

    printf("********** IIT **********\n");

    for(count = 0; count < 50; count++)
    {
        memset(&readIITEntry, 0, sizeof(readIITEntry));
        printf("\nwriteInsertInstructionEntry(): \n");
        hdrModMgr->writeInsertInstructionEntry(devId, count, &addIITEntry);
        printf("\nreadInsertInstructionEntry(): \n");
        hdrModMgr->readInsertInstructionEntry(devId, count, &readIITEntry);
        compareIITEntry(&addIITEntry, &readIITEntry);
        //printIITEntry(&readIITEntry);
    }

    //
    // ICDT table
    //
    memset(&addICDTEntry, 0, sizeof(addICDTEntry));
    addICDTEntry.Bitmap = 0xFE7689FF;
    for(int idx = 0; idx < 32; idx++)
    {
        addICDTEntry.Data[idx] = idx;
    }

    printf("********** ICDT **********\n");

    for(count = 0; count < 30; count++)
    {
        memset(&readICDTEntry, 0, sizeof(readICDTEntry));

        printf("\nwriteInsertConstantDataEntry(): \n");
        hdrModMgr->writeInsertConstantDataEntry(devId, count, &addICDTEntry);
        printf("\nreadInsertConstantDataEntry(): \n");
        hdrModMgr->readInsertConstantDataEntry(devId, count, &readICDTEntry);
        compareICDTEntry(&addICDTEntry, &readICDTEntry);
        //printICDTEntry(&readICDTEntry);
    }

    printf("********** Insertion Table **********\n");
    testInsertionMgr(hdrModMgr, devId, mode);

    return 0;
}
#endif
void compareControlMacData(xpControlMacEntry *tblEntry, xpControlMacEntry *getTblEntry)
{
    bool passStatus = true;

    if(tblEntry->data.routerMac != getTblEntry->data.routerMac)
    {
        passStatus = false;
        printf("Mismatch::routerMac Expected Value %x Actual Value: %x \n", tblEntry->data.routerMac, getTblEntry->data.routerMac);
    }
    if(tblEntry->data.controlMac != getTblEntry->data.controlMac)
    {
        passStatus = false;
        printf("Mismatch::controlMac Expected Value %x Actual Value: %x \n", tblEntry->data.controlMac, getTblEntry->data.controlMac);
    }
    if(passStatus)
    {
        printf("compareControlMacData: PASS \n");
    } else
    {
        printf("compareControlMacData: FAIL \n");
    }
}

void compareControlMacData(xpControlMacData_t *tblEntry, xpControlMacData_t *getTblEntry)
{
    bool passStatus = true;

    if(tblEntry->routerMac != getTblEntry->routerMac)
    {
        passStatus = false;
        printf("Mismatch::routerMac Expected Value %x Actual Value: %x \n", tblEntry->routerMac, getTblEntry->routerMac);
    }
    if(tblEntry->controlMac != getTblEntry->controlMac)
    {
        passStatus = false;
        printf("Mismatch::controlMac Expected Value %x Actual Value: %x \n", tblEntry->controlMac, getTblEntry->controlMac);
    }
    if(passStatus)
    {
        printf("compareControlMacData: PASS \n");
    } else
    {
        printf("compareControlMacData: FAIL \n");
    }
}

void printControlMacKeyMask(xpControlMacKeyMask_t *entry)
{
    printf("VlanID :: %x \n", entry->VlanID);
    printf("MAC :: %x:%x:%x:%x:%x:%x \n", entry->mac[0], entry->mac[1], entry->mac[2], entry->mac[3], entry->mac[4], entry->mac[5]);
}

void printControlMacData(xpControlMacData_t *entry)
{
    printf("routerMac :: %x \n", entry->routerMac);
    printf("controlMac :: %x \n", entry->controlMac);
}

void printControlMacEntry(xpControlMacEntry *entry)
{
    printf("Key.VlanID :: %x \n", entry->key.VlanID);
    printf("Key.mac :: %x:%x:%x:%x:%x:%x \n", entry->key.mac[0], entry->key.mac[1], entry->key.mac[2], entry->key.mac[3], entry->key.mac[4], entry->key.mac[5]);
    printf("Mask.VlanID :: %x \n", entry->mask.VlanID);
    printf("Mask.mac :: %x:%x:%x:%x:%x:%x \n", entry->mask.mac[0], entry->mask.mac[1], entry->mask.mac[2], entry->mask.mac[3], entry->mask.mac[4], entry->mask.mac[5]);
    printf("Data.routerMac :: %x \n", entry->data.routerMac);
    printf("Data.controlMac :: %x \n", entry->data.controlMac);
}

int testControlMacMgr(xpDevice_t devId)
{
    macAddr_t macAddr = { 06, 0x5A, 23, 00, 01, 02 };
    macAddr_t macAddr1 = { 16, 0xAA, 93, 10, 51, 82 };
    xpControlMacEntry tblEntry;
    xpControlMacEntry getTblEntry;
    xpControlMacKeyMask_t tblKey, tblMask;
    xpControlMacKeyMask_t getTblKey, getTblMask;
    xpControlMacData_t tblData;
    xpControlMacData_t getTblData;
    xpControlMacMgr *controlMacMgr;
    uint32_t index = 0;
    int32_t lookupIdx = -1;
    uint32_t routerMac = 1, getRouterMac = 0;

    printf("dpsdbg: Inside %s func\n", __FUNCTION__);

    controlMacMgr = static_cast<xpControlMacMgr *>(xpIfPl::instance()->getMgr(xpControlMacMgr::getId()));

    tblEntry.key.VlanID = 0x1C;
    memcpy(tblEntry.key.mac, macAddr, sizeof(macAddr_t));
    tblEntry.mask.VlanID = 0x0;
    memset(tblEntry.mask.mac, 0x0, sizeof(macAddr_t));
    tblEntry.data.routerMac = 1;
    tblEntry.data.controlMac = 1;

    tblKey.VlanID = 0xAA;
    memcpy(tblKey.mac, macAddr1, sizeof(macAddr_t));
    memset((void *)&tblMask, 0x0, sizeof(tblMask));
    tblData.routerMac = 0;
    tblData.controlMac = 1;

    for(index = 0; index < 256; index++)
    {
        memset(&getTblEntry, 0x0, sizeof(getTblEntry));
        tblEntry.key.VlanID += 0x10;

        printf("\nwriteEntry(): \n");
        if(controlMacMgr->writeEntry(devId, index, (void *)&tblEntry) != XP_NO_ERR)
        {
            printf("ERROR in inserting an entry. Exiting...\n");
            return -1;
        }
        printControlMacEntry(&tblEntry);
        getTblEntry.key.VlanID = tblEntry.key.VlanID;
        memcpy(getTblEntry.key.mac, tblEntry.key.mac, sizeof(macAddr_t));
        printf("\nLookup(): \n");
        controlMacMgr->lookup(devId, (void *)&getTblEntry, lookupIdx);
        if(lookupIdx != (int32_t)index)
        {
            printf("Lookup Mismatch:: Expected Value %d Actual Value %d \n", index, lookupIdx);
        }
        compareControlMacData(&tblEntry, &getTblEntry);
        printf("\nreadEntry(): \n");
        memset(&getTblEntry, 0x0, sizeof(getTblEntry));
        controlMacMgr->readEntry(devId, index, &getTblEntry);
        compareControlMacData(&tblEntry, &getTblEntry);
        printControlMacEntry(&getTblEntry);

        memset((void *)&getTblKey, 0x0, sizeof(getTblKey));
        memset((void *)&getTblMask, 0x0, sizeof(getTblMask));
        tblKey.VlanID += 0x10;

        printf("\nwriteKeyMask(): \n");
        if(controlMacMgr->writeKeyMask(devId, index, &tblKey, &tblMask) != XP_NO_ERR)
        {
            printf("ERROR in inserting an entry. Exiting...\n");
            return -1;
        }
        memset(&getTblEntry, 0x0, sizeof(getTblEntry));
        getTblEntry.key.VlanID = tblKey.VlanID;
        memcpy(getTblEntry.key.mac, tblKey.mac, sizeof(macAddr_t));
        printf("\nLookup(): \n");
        controlMacMgr->lookup(devId, (void *)&getTblEntry, lookupIdx);
        if(lookupIdx != (int32_t)index)
        {
            printf("Lookup Mismatch:: Expected Value %d Actual Value %d \n", index, lookupIdx);
        }
        compareControlMacData(&tblEntry, &getTblEntry);

        printf("\nreadKeyMask(): \n");
        controlMacMgr->readKeyMask(devId, index, &getTblKey, &getTblMask);
        printControlMacKeyMask(&getTblKey);
        printControlMacKeyMask(&getTblMask);

        memset((void *)&getTblData, 0x0, sizeof(getTblData));

        printf("\nwriteData(): \n");
        if(controlMacMgr->writeData(devId, index, &tblData) != XP_NO_ERR)
        {
            printf("ERROR in inserting an entry. Exiting...\n");
            return -1;
        }
        printf("\nreadData(): \n");
        controlMacMgr->readData(devId, index, &getTblData);
        compareControlMacData(&tblData, &getTblData);
        printControlMacData(&getTblData);

        printf("\nsetRouterMac(): \n");
        if(controlMacMgr->setField(devId, index, CONTROLMAC_ROUTER_MAC, (void *)&routerMac) != XP_NO_ERR)
        {
            printf("ERROR in setting field routerMac. Exiting...\n");
            return -1;
        }
        printf("\ngetRouterMac(): \n");
        controlMacMgr->getField(devId, index, CONTROLMAC_ROUTER_MAC, (void *)&getRouterMac);
        if(routerMac != getRouterMac)
        {
            printf("Mismatch::routerMac Expected Value %x Actual Value: %x \n", routerMac, getRouterMac);
            printf("Set/getRouterMac: FAIL \n");
        } else
        {
            printf("Set/getRouterMac: PASS \n");
        }

    }

    memset((void *)&tblEntry, 0x0, sizeof(tblEntry));
    tblEntry.key.VlanID = 0xAA;
    memcpy(tblEntry.key.mac, macAddr1, sizeof(macAddr_t));

    for(index = 0; index < 256; index++)
    {
        tblEntry.key.VlanID += 0x10;
        controlMacMgr->lookup(devId, (void *)&tblEntry, lookupIdx);
        if(lookupIdx != (int32_t)index)
        {
            printf("Lookup Mismatch:: Expected Value %d Actual Value %d \n", index, lookupIdx);
        }

        memset(&getTblEntry, 0x0, sizeof(getTblEntry));
        printf("\nremoveEntry(): \n");
        if(controlMacMgr->removeEntry(devId, index) != XP_NO_ERR)
        {
            printf("ERROR in removing an entry. Exiting...\n");
            return -1;
        }
        printf("\nreadEntry(): \n");
        controlMacMgr->readEntry(devId, index, &getTblEntry);
        printControlMacEntry(&getTblEntry);
    }

    return 0;
}

void compareEgressFilterEntry(xpPortMaskEntry *tblEntry, xpPortMaskEntry *getTblEntry)
{
    bool passStatus = true;

    for(uint32_t j = 0; j < sizeof(xpPortMaskEntry); j++) for(uint32_t i = 0; i < 8; i++) if((((*tblEntry).portMask[j]) & (1 << i)) != (((*getTblEntry).portMask[j]) & (1 << i)))
            {
                passStatus = false;
                printf("Mismatch::type Expected Value %d Actual Value: %d at idx %d position %d\n", j, i, (((*tblEntry).portMask[j]) & (1 << i)) >> i, (((*getTblEntry).portMask[j]) & (1 << i)) >> i);
            }

    if(passStatus)
    {
        printf("compareEgressFilterEntry: PASS \n");
    } else
    {
        printf("compareEgressFilterEntry: FAIL \n");
    }
}

void printEgressFilterEntry(xpPortMaskEntry *entry)
{
    for(uint32_t j = 0; j < sizeof(xpPortMaskEntry); j++)
    {
        for(uint32_t i = 0; i < 8; i++)
        {
            if(((*entry).portMask[j]) & (1 << i))
            {
                printf(" |%d| ", (((*entry).portMask[j]) & (1 << i)) >> i);
            } else
            {
                printf("  %d  ", (((*entry).portMask[j]) & (1 << i)) >> i);
            }
        }
    }
    printf("\n");
}
#if 0
int testEgressFilterMgr(xpDevice_t devId)
{
    uint32_t egressFilterId;
    uint32_t count;
    xpPortMaskEntry addEntryT, writeEntryT, readEntryT, entryT;
    xpPortList_t portList;
    XP_STATUS status;

    printf("dpsdbg: Inside %s func\n", __FUNCTION__);

    xpEgressFilterMgr *egressFilterMgr = static_cast<xpEgressFilterMgr *>(xpIfPl::instance()->getMgr(xpEgressFilterMgr::getId()));

    printf("\nCompare default entries: \n");
    /* Read default entries */
    for(count = 0; count < 100; count++)
    {
        memset(&entryT, 0xFF, sizeof(entryT));
        memset(&readEntryT, 0, sizeof(readEntryT));
        printf("\nreadEntry: \n");
        egressFilterMgr->readEntry(devId, count, &readEntryT);
        compareEgressFilterEntry(&entryT, &readEntryT);
    }

    // Allocate Id
    // here are the function calls to allocate id, then use this id and call the APIs with corresponding directions

    memset(&addEntryT, 0, sizeof(addEntryT));
    for(uint32_t i = 0; i < (sizeof(addEntryT) * 8); i = i + 15)
    {
        addEntryT.setPort(i);
    }

    memset(&writeEntryT, 0, sizeof(writeEntryT));
    for(uint32_t i = 0; i < (sizeof(writeEntryT) * 8); i = i + 35)
    {
        writeEntryT.setPort(i);
    }

    portList.push_back(2);
    portList.push_back(20);
    portList.push_back(23);
    portList.push_back(135);
    portList.push_back(126);
    portList.push_back(106);
    portList.push_back(69);

    for(count = 0; count < 100; count++)
    {
        egressFilterMgr->allocateId(egressFilterId);
        printf("allocateId(): egressFilterId: %d\n", egressFilterId);

        memset(&readEntryT, 0, sizeof(readEntryT));
        printf("\nwriteEntry(): \n");
        egressFilterMgr->writeEntry(devId, egressFilterId, &addEntryT);
        printf("\nreadEntry: \n");
        egressFilterMgr->readEntry(devId, egressFilterId, &readEntryT);
        compareEgressFilterEntry(&addEntryT, &readEntryT);
        printEgressFilterEntry(&readEntryT);

        memset(&readEntryT, 0, sizeof(readEntryT));
        printf("\nwriteEntry(): \n");
        egressFilterMgr->writeEntry(devId, egressFilterId, &writeEntryT);
        printf("\nreadEntry: \n");
        egressFilterMgr->readEntry(devId, egressFilterId, &readEntryT);
        compareEgressFilterEntry(&writeEntryT, &readEntryT);
        printEgressFilterEntry(&readEntryT);

        printf("\nsetPort: \n");
        egressFilterMgr->setPort(devId, egressFilterId, portList);
        printf("\nreadEntry: \n");
        egressFilterMgr->readEntry(devId, egressFilterId, &readEntryT);
        printEgressFilterEntry(&readEntryT);

        printf("\nclearPort: \n");
        egressFilterMgr->clearPort(devId, egressFilterId, portList);
        printf("\nreadEntry: \n");
        egressFilterMgr->readEntry(devId, egressFilterId, &readEntryT);
        printEgressFilterEntry(&readEntryT);

        memset(&entryT, 0xFF, sizeof(entryT));
        memset(&readEntryT, 0, sizeof(readEntryT));
        printf("\nsetAllPort(): \n");
        egressFilterMgr->setAllPort(devId, count);
        printf("\nreadEntry: \n");
        egressFilterMgr->readEntry(devId, count, &readEntryT);
        compareEgressFilterEntry(&entryT, &readEntryT);
        //printEgressFilterEntry(&readEntryT);

        memset(&entryT, 0x00, sizeof(entryT));
        memset(&readEntryT, 0, sizeof(readEntryT));
        printf("\nclearAllPort(): \n");
        egressFilterMgr->clearAllPort(devId, count);
        printf("\nreadEntry: \n");
        egressFilterMgr->readEntry(devId, count, &readEntryT);
        compareEgressFilterEntry(&entryT, &readEntryT);
        //printEgressFilterEntry(&readEntryT);
    }

    for(count = 0; count < 100; count++)
    {
        //printf("\nreleaseId(): egressFilterId: %d\n", count);
        status = egressFilterMgr->releaseId(count);
        if(status != XP_NO_ERR) printf("Release ID failed egressFilterId %d : status %d \n", count, status);
    }

    return 0;
}
#endif
void compareTrunkResolutionEntry(xpPortMaskEntry *tblEntry, xpPortMaskEntry *getTblEntry)
{
    bool passStatus = true;

    for(uint32_t j = 0; j < sizeof(xpPortMaskEntry); j++) for(uint32_t i = 0; i < 8; i++) if((((*tblEntry).portMask[j]) & (1 << i)) != (((*getTblEntry).portMask[j]) & (1 << i)))
            {
                passStatus = false;
                printf("Mismatch::type Expected Value %d Actual Value: %d at idx %d position %d\n", j, i, (((*tblEntry).portMask[j]) & (1 << i)) >> i, (((*getTblEntry).portMask[j]) & (1 << i)) >> i);
            }

    if(passStatus)
    {
        printf("compareTrunkResolutionEntry: PASS \n");
    } else
    {
        printf("compareTrunkResolutionEntry: FAIL \n");
    }
}

void printTrunkResolutionEntry(xpPortMaskEntry *entry)
{
    for(uint32_t j = 0; j < sizeof(xpPortMaskEntry); j++)
    {
        for(uint32_t i = 0; i < 8; i++)
        {
            if(((*entry).portMask[j]) & (1 << i))
            {
                printf(" |%d| ", (((*entry).portMask[j]) & (1 << i)) >> i);
            } else
            {
                printf("  %d  ", (((*entry).portMask[j]) & (1 << i)) >> i);
            }
        }
    }
    printf("\n");
}

int testTrunkResolutionMgr(xpDevice_t devId)
{
    uint32_t count;
    xpPortMaskEntry addEntryT, writeEntryT, readEntryT, entryT;
    xpPortList_t portList;

    printf("dpsdbg: Inside %s func\n", __FUNCTION__);

    xpTrunkResolutionMgr *trunkResolutionMgr = static_cast<xpTrunkResolutionMgr *>(xpIfPl::instance()->getMgr(xpTrunkResolutionMgr::getId()));

    printf("\nCompare default entries: \n");
    /* Read default entries */
    for(count = 0; count < 100; count++)
    {
        memset(&entryT, 0xFF, sizeof(entryT));
        memset(&readEntryT, 0, sizeof(readEntryT));
        printf("\nreadEntry: \n");
        trunkResolutionMgr->readEntry(devId, count, &readEntryT);
        compareTrunkResolutionEntry(&entryT, &readEntryT);
    }

    memset(&addEntryT, 0, sizeof(addEntryT));
    for(uint32_t i = 0; i < (sizeof(addEntryT) * 8); i = i + 15)
    {
        addEntryT.setPort(i);
    }

    memset(&writeEntryT, 0, sizeof(writeEntryT));
    for(uint32_t i = 0; i < (sizeof(writeEntryT) * 8); i = i + 35)
    {
        writeEntryT.setPort(i);
    }

    portList.push_back(21);
    portList.push_back(28);
    portList.push_back(13);
    portList.push_back(125);
    portList.push_back(130);
    portList.push_back(96);
    portList.push_back(89);

    for(count = 0; count < 100; count++)
    {
        memset(&readEntryT, 0, sizeof(readEntryT));
        printf("\nwriteEntry(): \n");
        trunkResolutionMgr->writeEntry(devId, count, &addEntryT);
        printf("\nreadEntry: \n");
        trunkResolutionMgr->readEntry(devId, count, &readEntryT);
        compareTrunkResolutionEntry(&addEntryT, &readEntryT);
        //printTrunkResolutionEntry(&readEntryT);

        memset(&readEntryT, 0, sizeof(readEntryT));
        printf("\nwriteEntry(): \n");
        trunkResolutionMgr->writeEntry(devId, count, &writeEntryT);
        printf("\nreadEntry: \n");
        trunkResolutionMgr->readEntry(devId, count, &readEntryT);
        compareTrunkResolutionEntry(&writeEntryT, &readEntryT);
        //printTrunkResolutionEntry(&readEntryT);

        printf("\nsetPort: \n");
        trunkResolutionMgr->setPort(devId, count, portList);
        printf("\nreadEntry: \n");
        trunkResolutionMgr->readEntry(devId, count, &readEntryT);
        printTrunkResolutionEntry(&readEntryT);

        printf("\nclearPort: \n");
        trunkResolutionMgr->clearPort(devId, count, portList);
        printf("\nreadEntry: \n");
        trunkResolutionMgr->readEntry(devId, count, &readEntryT);
        printTrunkResolutionEntry(&readEntryT);

        memset(&entryT, 0xFF, sizeof(entryT));
        memset(&readEntryT, 0, sizeof(readEntryT));
        printf("\nsetAllPort(): \n");
        trunkResolutionMgr->setAllPort(devId, count);
        printf("\nreadEntry: \n");
        trunkResolutionMgr->readEntry(devId, count, &readEntryT);
        compareTrunkResolutionEntry(&entryT, &readEntryT);
        //printTrunkResolutionEntry(&readEntryT);

        memset(&entryT, 0x00, sizeof(entryT));
        memset(&readEntryT, 0, sizeof(readEntryT));
        printf("\nclearAllPort(): \n");
        trunkResolutionMgr->clearAllPort(devId, count);
        printf("\nreadEntry: \n");
        trunkResolutionMgr->readEntry(devId, count, &readEntryT);
        compareTrunkResolutionEntry(&entryT, &readEntryT);
        //printTrunkResolutionEntry(&readEntryT);
    }

    return 0;
}

void compareBdEntry(xpBdEntry *tblEntry, xpBdEntry *getTblEntry)
{
    bool passStatus = true;

    if(tblEntry->keyBridgeDomain != getTblEntry->keyBridgeDomain)
    {
        passStatus = false;
        printf("Mismatch::keyBridgeDomain Expected Value %d Actual Value: %d \n", tblEntry->keyBridgeDomain, getTblEntry->keyBridgeDomain);
    }
    if(tblEntry->macSAmode != getTblEntry->macSAmode)
    {
        passStatus = false;
        printf("Mismatch::macSAmode Expected Value %x Actual Value: %x \n", tblEntry->macSAmode, getTblEntry->macSAmode);
    }
    if(tblEntry->setEgressPortFilter != getTblEntry->setEgressPortFilter)
    {
        passStatus = false;
        printf("Mismatch::setEgressPortFilter Expected Value %x Actual Value: %x \n", tblEntry->setEgressPortFilter, getTblEntry->setEgressPortFilter);
    }
    if(tblEntry->macSAmissCmd != getTblEntry->macSAmissCmd)
    {
        passStatus = false;
        printf("Mismatch::macSAmissCmd Expected Value %x Actual Value: %x \n", tblEntry->macSAmissCmd, getTblEntry->macSAmissCmd);
    }
    if(tblEntry->ipv4MulticastBridgeMode != getTblEntry->ipv4MulticastBridgeMode)
    {
        passStatus = false;
        printf("Mismatch::ipv4MulticastBridgeMode Expected Value %x Actual Value: %x \n", tblEntry->ipv4MulticastBridgeMode, getTblEntry->ipv4MulticastBridgeMode);
    }
    if(tblEntry->ipv6MulticastBridgeMode != getTblEntry->ipv6MulticastBridgeMode)
    {
        passStatus = false;
        printf("Mismatch::ipv6MulticastBridgeMode Expected Value %x Actual Value: %x \n", tblEntry->ipv6MulticastBridgeMode, getTblEntry->ipv6MulticastBridgeMode);
    }
    if(tblEntry->unregisteredMulticastCmd != getTblEntry->unregisteredMulticastCmd)
    {
        passStatus = false;
        printf("Mismatch::unregisteredMulticastCmd Expected Value %x Actual Value: %x \n", tblEntry->unregisteredMulticastCmd, getTblEntry->unregisteredMulticastCmd);
    }
    if(tblEntry->broadcastCmd != getTblEntry->broadcastCmd)
    {
        passStatus = false;
        printf("Mismatch::broadcastCmd Expected Value %x Actual Value: %x \n", tblEntry->broadcastCmd, getTblEntry->broadcastCmd);
    }
    if(tblEntry->unknownUnicastCmd != getTblEntry->unknownUnicastCmd)
    {
        passStatus = false;
        printf("Mismatch::unknownUnicastCmd Expected Value %x Actual Value: %x \n", tblEntry->unknownUnicastCmd, getTblEntry->unknownUnicastCmd);
    }
    if(tblEntry->ipv4ARPBCCmd != getTblEntry->ipv4ARPBCCmd)
    {
        passStatus = false;
        printf("Mismatch::ipv4ARPBCCmd Expected Value %x Actual Value: %x \n", tblEntry->ipv4ARPBCCmd, getTblEntry->ipv4ARPBCCmd);
    }
    if(tblEntry->ipv4IgmpCmd != getTblEntry->ipv4IgmpCmd)
    {
        passStatus = false;
        printf("Mismatch::ipv4IgmpCmd Expected Value %x Actual Value: %x \n", tblEntry->ipv4IgmpCmd, getTblEntry->ipv4IgmpCmd);
    }
    if(tblEntry->ipv6IcmpCmd != getTblEntry->ipv6IcmpCmd)
    {
        passStatus = false;
        printf("Mismatch::ipv6IcmpCmd Expected Value %x Actual Value: %x \n", tblEntry->ipv6IcmpCmd, getTblEntry->ipv6IcmpCmd);
    }
    if(tblEntry->setVRF != getTblEntry->setVRF)
    {
        passStatus = false;
        printf("Mismatch::setVRF Expected Value %x Actual Value: %x \n", tblEntry->setVRF, getTblEntry->setVRF);
    }
    if(tblEntry->ipv4UnicastRouteEn != getTblEntry->ipv4UnicastRouteEn)
    {
        passStatus = false;
        printf("Mismatch::ipv4UnicastRouteEn Expected Value %x Actual Value: %x \n", tblEntry->ipv4UnicastRouteEn, getTblEntry->ipv4UnicastRouteEn);
    }
    if(tblEntry->ipv6UnicastRouteEn != getTblEntry->ipv6UnicastRouteEn)
    {
        passStatus = false;
        printf("Mismatch::ipv6UnicastRouteEn Expected Value %x Actual Value: %x \n", tblEntry->ipv6UnicastRouteEn, getTblEntry->ipv6UnicastRouteEn);
    }
    if(tblEntry->floodVif != getTblEntry->floodVif)
    {
        passStatus = false;
        printf("Mismatch::floodVif Expected Value %x Actual Value: %x \n", tblEntry->floodVif, getTblEntry->floodVif);
    }
    if(tblEntry->VRF != getTblEntry->VRF)
    {
        passStatus = false;
        printf("Mismatch::VRF Expected Value %x Actual Value: %x \n", tblEntry->VRF, getTblEntry->VRF);
    }
    if(tblEntry->egressPortFilterID != getTblEntry->egressPortFilterID)
    {
        passStatus = false;
        printf("Mismatch::egressPortFilterID Expected Value %x Actual Value: %x \n", tblEntry->egressPortFilterID, getTblEntry->egressPortFilterID);
    }
    if(tblEntry->ipv4MulticastRouteEn != getTblEntry->ipv4MulticastRouteEn)
    {
        passStatus = false;
        printf("Mismatch::ipv4MulticastRouteEn Expected Value %x Actual Value: %x \n", tblEntry->ipv4MulticastRouteEn, getTblEntry->ipv4MulticastRouteEn);
    }
    if(tblEntry->ipv6MulticastRouteEn != getTblEntry->ipv6MulticastRouteEn)
    {
        passStatus = false;
        printf("Mismatch::ipv6MulticastRouteEn Expected Value %x Actual Value: %x \n", tblEntry->ipv6MulticastRouteEn, getTblEntry->ipv6MulticastRouteEn);
    }
    if(tblEntry->ipv4MulticastRouteMode != getTblEntry->ipv4MulticastRouteMode)
    {
        passStatus = false;
        printf("Mismatch::ipv4MulticastRouteMode Expected Value %x Actual Value: %x \n", tblEntry->ipv4MulticastRouteMode, getTblEntry->ipv4MulticastRouteMode);
    }
    if(tblEntry->ipv6MulticastRouteMode != getTblEntry->ipv6MulticastRouteMode)
    {
        passStatus = false;
        printf("Mismatch::ipv6MulticastRouteMode Expected Value %x Actual Value: %x \n", tblEntry->ipv6MulticastRouteMode, getTblEntry->ipv6MulticastRouteMode);
    }
    if(tblEntry->policyBasedRoutingEn != getTblEntry->policyBasedRoutingEn)
    {
        passStatus = false;
        printf("Mismatch::policyBasedRoutingEn Expected Value %x Actual Value: %x \n", tblEntry->policyBasedRoutingEn, getTblEntry->policyBasedRoutingEn);
    }
    if(tblEntry->mirrorToAnalyzerMask != getTblEntry->mirrorToAnalyzerMask)
    {
        passStatus = false;
        printf("Mismatch::mirrorToAnalyzerMask Expected Value %x Actual Value: %x \n", tblEntry->mirrorToAnalyzerMask, getTblEntry->mirrorToAnalyzerMask);
    }
    if(tblEntry->countMode != getTblEntry->countMode)
    {
        passStatus = false;
        printf("Mismatch::countMode Expected Value %x Actual Value: %x \n", tblEntry->countMode, getTblEntry->countMode);
    }
    if(tblEntry->counterIndex != getTblEntry->counterIndex)
    {
        passStatus = false;
        printf("Mismatch::counterIndex Expected Value %x Actual Value: %x \n", tblEntry->counterIndex, getTblEntry->counterIndex);
    }

    if(passStatus)
    {
        printf("compareBdEntry: PASS \n");
    } else
    {
        printf("compareBdEntry: FAIL \n");
    }
}

void printBdEntry(xpBdEntry *entry)
{
    printf("keyBridgeDomain :: %x \n", entry->keyBridgeDomain);
    printf("macSAmode :: %x \n", entry->macSAmode);
    printf("setEgressPortFilter :: %x \n", entry->setEgressPortFilter);
    printf("macSAmissCmd :: %x \n", entry->macSAmissCmd);
    printf("ipv4MulticastBridgeMode :: %x \n", entry->ipv4MulticastBridgeMode);
    printf("ipv6MulticastBridgeMode :: %x \n", entry->ipv6MulticastBridgeMode);
    printf("unregisteredMulticastCmd :: %x \n", entry->unregisteredMulticastCmd);
    printf("broadcastCmd :: %x \n", entry->broadcastCmd);
    printf("unknownUnicastCmd :: %x \n", entry->unknownUnicastCmd);
    printf("ipv4ARPBCCmd :: %x \n", entry->ipv4ARPBCCmd);
    printf("ipv4IgmpCmd :: %x \n", entry->ipv4IgmpCmd);
    printf("ipv6IcmpCmd :: %x \n", entry->ipv6IcmpCmd);
    printf("setVRF :: %x \n", entry->setVRF);
    printf("ipv4UnicastRouteEn :: %x \n", entry->ipv4UnicastRouteEn);
    printf("ipv6UnicastRouteEn :: %x \n", entry->ipv6UnicastRouteEn);
    printf("floodVif :: %x \n", entry->floodVif);
    printf("VRF :: %x \n", entry->VRF);
    printf("egressPortFilterID :: %x \n", entry->egressPortFilterID);
    printf("ipv4MulticastRouteEn :: %x \n", entry->ipv4MulticastRouteEn);
    printf("ipv6MulticastRouteEn :: %x \n", entry->ipv6MulticastRouteEn);
    printf("ipv4MulticastRouteMode :: %x \n", entry->ipv4MulticastRouteMode);
    printf("ipv6MulticastRouteMode :: %x \n", entry->ipv6MulticastRouteMode);
    printf("policyBasedRoutingEn :: %x \n", entry->policyBasedRoutingEn);
    printf("mirrorToAnalyzerMask :: %x \n", entry->mirrorToAnalyzerMask);
    printf("countMode :: %x \n", entry->countMode);
    printf("counterIndex :: %x \n", entry->counterIndex);
}
#if 0
int testBdMgr(xpDevice_t devId, XP_PIPE_MODE mode)
{
    uint32_t bdId;
    uint32_t count;
    xpBdEntry addEntryT, readEntryT, lookupEntryT;
    uint32_t index = 0;
    int32_t lIndex = -1;
    uint8_t tblCopyNum = tableProfile->getXpBdTblCopyNum();
    uint32_t depth = MAX_BD_ID(mode);
    uint32_t indexArr[depth];
    XP_STATUS status;

    printf("dpsdbg: Inside %s func\n", __FUNCTION__);

    xpBdMgr *bdMgr = static_cast<xpBdMgr *>(xpL2Pl::instance()->getMgr(xpBdMgr::getId()));

    // Allocate Id
    // here are the function calls to allocate id, then use this id and call the APIs with corresponding directions

    memset(&addEntryT, 0x00, sizeof(addEntryT));
    addEntryT.macSAmode = 0x1;
    addEntryT.setEgressPortFilter = 0x0;
    addEntryT.macSAmissCmd = 0x3;
    addEntryT.ipv4MulticastBridgeMode = 0x1;
    addEntryT.ipv6MulticastBridgeMode = 0x2;
    addEntryT.unregisteredMulticastCmd = 0x3;
    addEntryT.broadcastCmd = 0x1;
    addEntryT.unknownUnicastCmd = 0x0;
    addEntryT.ipv4ARPBCCmd = 0x1;
    addEntryT.ipv4IgmpCmd = 0x3;
    addEntryT.ipv6IcmpCmd = 0x2;
    addEntryT.setVRF = 0x0;
    addEntryT.ipv4UnicastRouteEn = 0x1;
    addEntryT.ipv6UnicastRouteEn = 0x0;
    addEntryT.floodVif = 0xFFEE;
    addEntryT.VRF = 0xEEAD;
    addEntryT.egressPortFilterID = 0xA2;
    addEntryT.ipv4MulticastRouteEn = 0x1;
    addEntryT.ipv6MulticastRouteEn = 0x0;
    addEntryT.ipv4MulticastRouteMode = 0x1;
    addEntryT.ipv6MulticastRouteMode = 0x1;
    addEntryT.policyBasedRoutingEn = 0x0;
    addEntryT.mirrorToAnalyzerMask = 0x3;
    addEntryT.countMode = 0x5;
    addEntryT.counterIndex = 0xDFAF;

    for(count = 0; count < depth; count++)
    {
        bdMgr->allocateId(bdId);
        printf("allocateId(): bdId: %d\n", bdId);

        memset(&readEntryT, 0, sizeof(readEntryT));
        memset(&lookupEntryT, 0, sizeof(lookupEntryT));

        printf("\naddEntry(): \n");
        addEntryT.keyBridgeDomain = bdId;
        bdMgr->addEntry(devId, &addEntryT, index);
        indexArr[count] = index;
        printf("\nreadEntry at index %d: \n", indexArr[count]);
        for(int i = 0; i < tblCopyNum; i++)
        {
            printf("*********** %d ************\n", i);
            memset(&readEntryT, 0, sizeof(readEntryT));
            bdMgr->readEntry(devId, index, &readEntryT, i);
            compareBdEntry(&addEntryT, &readEntryT);
            //printBdEntry(&readEntryT);
        }
        printf("\nlookup: \n");
        lookupEntryT.keyBridgeDomain = bdId;
        bdMgr->lookup(devId, &lookupEntryT, lIndex);
        compareBdEntry(&addEntryT, &lookupEntryT);
        //printBdEntry(&lookupEntryT);
        if((int)index != lIndex)
        {
            printf("Lookup FAIL::Expected Value %d Actual Value: %d \n", index, lIndex);
        }
    }
    printf("Inserting all entries\n");
    for(count = 0; count < depth; count++)
    {
        printf("\nreadEntry at index %d: \n",indexArr[count]);
        for(int i = 0; i < tblCopyNum; i++)
        {
            addEntryT.keyBridgeDomain = count;
            printf("*********** %d ************\n", i);
            memset(&readEntryT, 0, sizeof(readEntryT));
            bdMgr->readEntry(devId, indexArr[count], &readEntryT, i);
            compareBdEntry(&addEntryT, &readEntryT);
            //printBdEntry(&readEntryT);
        }
    }

    for(count = 0; count < depth; count++)
    {
        //printf("\nreleaseId(): bdId: %d\n", count);
        status = bdMgr->releaseId(count);
        if(status != XP_NO_ERR) printf("Release ID failed bdId %d : status %d \n", count, status);
    }

    return 0;
}

void compareFdbEntry(xpFdbTblEntry *tblEntry, xpFdbTblEntry *getTblEntry)
{
    bool passStatus = true;

    if(tblEntry->keyMACAddress[0] != getTblEntry->keyMACAddress[0])
    {
        passStatus = false;
        printf("Mismatch::keyMACAddress[0] Expected Value %x Actual Value: %x \n", tblEntry->keyMACAddress[0], getTblEntry->keyMACAddress[0]);
    }
    if(tblEntry->keyMACAddress[1] != getTblEntry->keyMACAddress[1])
    {
        passStatus = false;
        printf("Mismatch::keyMACAddress[1] Expected Value %x Actual Value: %x \n", tblEntry->keyMACAddress[1], getTblEntry->keyMACAddress[1]);
    }
    if(tblEntry->keyMACAddress[2] != getTblEntry->keyMACAddress[2])
    {
        passStatus = false;
        printf("Mismatch::keyMACAddress[2] Expected Value %x Actual Value: %x \n", tblEntry->keyMACAddress[2], getTblEntry->keyMACAddress[2]);
    }
    if(tblEntry->keyMACAddress[3] != getTblEntry->keyMACAddress[3])
    {
        passStatus = false;
        printf("Mismatch::keyMACAddress[3] Expected Value %x Actual Value: %x \n", tblEntry->keyMACAddress[3], getTblEntry->keyMACAddress[3]);
    }
    if(tblEntry->keyMACAddress[4] != getTblEntry->keyMACAddress[4])
    {
        passStatus = false;
        printf("Mismatch::keyMACAddress[4] Expected Value %x Actual Value: %x \n", tblEntry->keyMACAddress[4], getTblEntry->keyMACAddress[4]);
    }
    if(tblEntry->keyMACAddress[5] != getTblEntry->keyMACAddress[5])
    {
        passStatus = false;
        printf("Mismatch::keyMACAddress[5] Expected Value %x Actual Value: %x \n", tblEntry->keyMACAddress[5], getTblEntry->keyMACAddress[5]);
    }
    if(tblEntry->keyBridgeDomain != getTblEntry->keyBridgeDomain)
    {
        passStatus = false;
        printf("Mismatch::keyBridgeDomain Expected Value %x Actual Value: %x \n", tblEntry->keyBridgeDomain, getTblEntry->keyBridgeDomain);
    }
    if(tblEntry->pktCmd != getTblEntry->pktCmd)
    {
        passStatus = false;
        printf("Mismatch::pktCmd Expected Value %x Actual Value: %x \n", tblEntry->pktCmd, getTblEntry->pktCmd);
    }
    if(tblEntry->mirrorMask != getTblEntry->mirrorMask)
    {
        passStatus = false;
        printf("Mismatch::mirrorMask Expected Value %x Actual Value: %x \n", tblEntry->mirrorMask, getTblEntry->mirrorMask);
    }
    if(tblEntry->macDAIsControl != getTblEntry->macDAIsControl)
    {
        passStatus = false;
        printf("Mismatch::macDAIsControl Expected Value %x Actual Value: %x \n", tblEntry->macDAIsControl, getTblEntry->macDAIsControl);
    }
    if(tblEntry->staticMAC != getTblEntry->staticMAC)
    {
        passStatus = false;
        printf("Mismatch::staticMAC Expected Value %x Actual Value: %x \n", tblEntry->staticMAC, getTblEntry->staticMAC);
    }
    if(tblEntry->countMode != getTblEntry->countMode)
    {
        passStatus = false;
        printf("Mismatch::countMode Expected Value %x Actual Value: %x \n", tblEntry->countMode, getTblEntry->countMode);
    }
    if(tblEntry->routerMAC != getTblEntry->routerMAC)
    {
        passStatus = false;
        printf("Mismatch::routerMAC Expected Value %x Actual Value: %x \n", tblEntry->routerMAC, getTblEntry->routerMAC);
    }
    if(tblEntry->ingressEgressVif != getTblEntry->ingressEgressVif)
    {
        passStatus = false;
        printf("Mismatch::ingressEgressVif Expected Value %x Actual Value: %x \n", tblEntry->ingressEgressVif, getTblEntry->ingressEgressVif);
    }
    if(tblEntry->ecmpSize != getTblEntry->ecmpSize)
    {
        passStatus = false;
        printf("Mismatch::ecmpSize Expected Value %x Actual Value: %x \n", tblEntry->ecmpSize, getTblEntry->ecmpSize);
    }
    if(passStatus)
    {
        printf("compareFdbEntry: PASS \n");
    } else
    {
        printf("compareFdbEntry: FAIL \n");
    }
}

void printFdbEntry(xpFdbTblEntry *entry)
{
    printf("keyMACAddress :: %x:%x:%x:%x:%x:%x \n", entry->keyMACAddress[0], entry->keyMACAddress[1], entry->keyMACAddress[2], entry->keyMACAddress[3], entry->keyMACAddress[4], entry->keyMACAddress[5]);
    printf("keyBridgeDomain :: %x \n", entry->keyBridgeDomain);
    printf("pktCmd :: %x \n", entry->pktCmd);
    printf("mirrorMask :: %x \n", entry->mirrorMask);
    printf("macDAIsControl :: %x \n", entry->macDAIsControl);
    printf("staticMAC :: %x \n", entry->staticMAC);
    printf("countMode :: %x \n", entry->countMode);
    printf("routerMAC :: %x \n", entry->routerMAC);
    printf("ingressEgressVif :: %x \n", entry->ingressEgressVif);
    printf("ecmpSize :: %x \n", entry->ecmpSize);
}

int testFDB(xpDevice_t deviceId, XP_PIPE_MODE mode)
{
    macAddr_t macAddr = { 06, 0x5A, 23, 00, 01, 02 };
    macAddr_t macAddr1 = { 0x19, 0x23, 0x4A, 0xFF, 0x45, 0xDF };
    macAddr_t getMacAddr = { 0, };
    xpFdbTblEntry tblEntry;
    xpFdbTblEntry getTblEntry;
    uint32_t index = 0, reHashIndex = 0;

    memcpy(tblEntry.keyMACAddress, macAddr, sizeof(macAddr_t));
    tblEntry.keyBridgeDomain = 256U;
    tblEntry.pktCmd = 3U;
    tblEntry.mirrorMask = 2U;
    tblEntry.macDAIsControl = 1U;
    tblEntry.staticMAC = 1U;
    tblEntry.countMode = 3U;
    tblEntry.routerMAC = 1U;
    tblEntry.ingressEgressVif = 9999U;
    tblEntry.ecmpSize = 123U;

    printf("dpsdbg: Inside %s func\n", __FUNCTION__);

    xpFdbMgr *xpPmMgr = static_cast<xpFdbMgr *>(xpL2Pl::instance()->getMgr(xpFdbMgr::getId()));

    /* Add an entry */
    if(xpPmMgr->addEntry(deviceId, (void *)&tblEntry, index, reHashIndex) != XP_NO_ERR)
    {
        printf("ERROR in inserting an entry. Exiting...\n");
        return -1;
    }

    tblEntry.pktCmd = 1U;
    tblEntry.mirrorMask = 2U;
    tblEntry.macDAIsControl = 1U;
    tblEntry.staticMAC = 1U;
    tblEntry.countMode = 7U;
    tblEntry.routerMAC = 0U;
    tblEntry.ingressEgressVif = 234U;
    tblEntry.ecmpSize = 78U;

    /* Set whole entry */
    if(xpPmMgr->writeEntry(deviceId, index, (void *)&tblEntry) != XP_NO_ERR)
    {
        printf("ERROR in updating an entry. Exiting...\n");
        return -1;
    }

    /* Read whole Entry */
    if(xpPmMgr->readEntry(deviceId, index, (void *)&getTblEntry) != XP_NO_ERR)
    {
        printf("ERROR in reading an entry. Exiting...\n");
        return -1;
    }
    compareFdbEntry(&tblEntry, &getTblEntry);

    uint32_t ecmpSize = 75U, getEcmpSize = 0;
    xpPmMgr->setField(deviceId, index, FDB_ECMP_SIZE, (void *)&ecmpSize);
    xpPmMgr->getField(deviceId, index, FDB_ECMP_SIZE, (void *)&getEcmpSize);
    if(ecmpSize != getEcmpSize)
    {
        printf("Compare ecmpSize :: FAIL\n");
    } else
    {
        printf("Compare ecmpSize :: PASS\n");
    }

    if(xpPmMgr->setField(deviceId, index, FDB_KEY_MAC_ADDRESS, (void *)macAddr1) != XP_NO_ERR)
    {
        printf("ERROR in setting field %d.\n", FDB_KEY_MAC_ADDRESS);
    }

    xpPmMgr->getField(deviceId, index, FDB_KEY_MAC_ADDRESS, (void *)getMacAddr);
    for(int i = 0; i < 6; i++)
    {
        if(macAddr[i] != getMacAddr[i])
        {
            printf("Compare macAddr :: FAIL\n");
        } else
        {
            printf("Compare macAddr :: PASS\n");
        }
    }

    return 0;
}
#endif

void compareMplsTunnelEntry(xpMplsTunnelEntry *tblEntry, xpMplsTunnelEntry *getTblEntry)
{
    bool passStatus = true;

    if(tblEntry->KeyOuterVlanId != getTblEntry->KeyOuterVlanId)
    {
        passStatus = false;
        printf("Mismatch::KeyOuterVlanId Expected Value %x Actual Value: %x \n", tblEntry->KeyOuterVlanId, getTblEntry->KeyOuterVlanId);
    }
    if(tblEntry->KeyMplsLabel != getTblEntry->KeyMplsLabel)
    {
        passStatus = false;
        printf("Mismatch::KeyMplsLabel Expected Value %x Actual Value: %x \n", tblEntry->KeyMplsLabel, getTblEntry->KeyMplsLabel);
    }
    if(tblEntry->isP2MPBudNode != getTblEntry->isP2MPBudNode)
    {
        passStatus = false;
        printf("Mismatch::isP2MPBudNode Expected Value %x Actual Value: %x \n", tblEntry->isP2MPBudNode, getTblEntry->isP2MPBudNode);
    }
    if(tblEntry->innerTerminateLabelPos != getTblEntry->innerTerminateLabelPos)
    {
        passStatus = false;
        printf("Mismatch::innerTerminateLabelPos Expected Value %x Actual Value: %x \n", tblEntry->innerTerminateLabelPos, getTblEntry->innerTerminateLabelPos);
    }
    if(tblEntry->countMode != getTblEntry->countMode)
    {
        passStatus = false;
        printf("Mismatch::countMode Expected Value %x Actual Value: %x \n", tblEntry->countMode, getTblEntry->countMode);
    }
    if(tblEntry->copyTTLfromTunnelHeader != getTblEntry->copyTTLfromTunnelHeader)
    {
        passStatus = false;
        printf("Mismatch::copyTTLfromTunnelHeader Expected Value %x Actual Value: %x \n", tblEntry->copyTTLfromTunnelHeader, getTblEntry->copyTTLfromTunnelHeader);
    }
    if(tblEntry->floodVif != getTblEntry->floodVif)
    {
        passStatus = false;
        printf("Mismatch::floodVif Expected Value %x Actual Value: %x \n", tblEntry->floodVif, getTblEntry->floodVif);
    }
    if(tblEntry->counterIndex != getTblEntry->counterIndex)
    {
        passStatus = false;
        printf("Mismatch::counterIndex Expected Value %x Actual Value: %x \n", tblEntry->counterIndex, getTblEntry->counterIndex);
    }
    if(passStatus)
    {
        printf("compareMplsTunnelEntry: PASS \n");
    } else
    {
        printf("compareMplsTunnelEntry: FAIL \n");
    }
}

int testMplsTunnel(xpDevice_t deviceId, XP_PIPE_MODE mode)
{
    xpMplsTunnelEntry tblEntry;
    xpMplsTunnelEntry getTblEntry;
    uint32_t index = 0, reHashIndex = 0;
    int32_t lookupIdx = 0;

    printf("dbg: Inside %s func\n", __FUNCTION__);

    xpMplsTunnelMgr *xpPmMgr = static_cast<xpMplsTunnelMgr *>(xpTunnelPl::instance()->getMgr(xpMplsTunnelMgr::getId()));


    memset((void *)&tblEntry, 0 , sizeof(xpMplsTunnelEntry));
    for (int i = 0; i < 5; i++)
    {
        tblEntry.KeyOuterVlanId = 0;
        tblEntry.KeyMplsLabel = i + 29;
        if(xpPmMgr->addEntry(deviceId, (void *)&tblEntry, index, reHashIndex) != XP_NO_ERR)
        {
            printf("ERROR in inserting an entry. Exiting...\n");
            //return -1;
        }
        printf("index of %dth entry : %d \n",i+1, index);
    }

    tblEntry.KeyOuterVlanId = 25U;
    tblEntry.KeyMplsLabel = 40U;
    tblEntry.isP2MPBudNode = 1U;
    tblEntry.innerTerminateLabelPos = 2U;
    tblEntry.countMode = 5U;
    tblEntry.copyTTLfromTunnelHeader = 1U;
    tblEntry.floodVif = 3215U;
    tblEntry.counterIndex = 2567U;

    /* Add an entry */
    if(xpPmMgr->addEntry(deviceId, (void *)&tblEntry, index, reHashIndex) != XP_NO_ERR)
    {
        printf("ERROR in inserting an entry. Exiting...\n");
        return -1;
    }

    getTblEntry.KeyOuterVlanId = 25U;
    getTblEntry.KeyMplsLabel = 40U;

    xpPmMgr->lookup(deviceId, (void *)&getTblEntry, lookupIdx);
    if(lookupIdx != (int32_t)index)
    {
        printf("Lookup Mismatch:: Expected Value %d Actual Value %d \n", index, lookupIdx);
    }
    else
    {
        printf("Lookup PASS:: Index %d Actual Value %d \n", lookupIdx, index);
        compareMplsTunnelEntry(&tblEntry, &getTblEntry);
    }

    tblEntry.isP2MPBudNode = 1U;
    tblEntry.innerTerminateLabelPos = 2U;
    tblEntry.countMode = 3U;
    tblEntry.copyTTLfromTunnelHeader = 1U;
    tblEntry.floodVif = 1234U;
    tblEntry.counterIndex = 23567U;

    /* Set whole entry */
    if(xpPmMgr->writeEntry(deviceId, index, (void *)&tblEntry) != XP_NO_ERR)
    {
        printf("ERROR in updating an entry. Exiting...\n");
        return -1;
    }

    /* Read whole Entry */
    if(xpPmMgr->readEntry(deviceId, index, (void *)&getTblEntry) != XP_NO_ERR)
    {
        printf("ERROR in reading an entry. Exiting...\n");
        return -1;
    }
    compareMplsTunnelEntry(&tblEntry, &getTblEntry);

    uint32_t floodVif = 123U, getFloodVif = 0;
    xpPmMgr->setField(deviceId, index, MPLSTUNNEL_FLOOD_VIF, (void *)&floodVif);
    xpPmMgr->getField(deviceId, index, MPLSTUNNEL_FLOOD_VIF, (void *)&getFloodVif);
    if(floodVif != getFloodVif)
    {
        printf("Compare floodVif :: FAIL\n");
    } else
    {
        printf("Compare floodVif :: PASS\n");
    }

    return 0;
}
void compareIpv4RouteMcEntry(xpIpv4RouteMcEntry * tblEntry, xpIpv4RouteMcEntry * getTblEntry)
{
    bool passStatus = true;

    for(int idx = 0; idx < 4 ; idx++)
    {
        if(tblEntry->keySIP[0] != getTblEntry->keySIP[0])
        {
            passStatus = false;
            printf("Mismatch::keySIP[0] Expected Value %x Actual Value: %x \n", tblEntry->keySIP[0], getTblEntry->keySIP[0]);
        }
        if(tblEntry->keyGroupAddress[0] != getTblEntry->keyGroupAddress[0])
        {
            passStatus = false;
            printf("Mismatch::keyGroupAddress[0] Expected Value %x Actual Value: %x \n", tblEntry->keyGroupAddress[0], getTblEntry->keyGroupAddress[0]);
        }
    }
    if(tblEntry->keyVRF != getTblEntry->keyVRF)
    {
        passStatus = false;
        printf("Mismatch::keyVRF Expected Value %x Actual Value: %x \n", tblEntry->keyVRF, getTblEntry->keyVRF);
    }
    if(tblEntry->pktCmd != getTblEntry->pktCmd)
    {
        passStatus = false;
        printf("Mismatch::pktCmd Expected Value %x Actual Value: %x \n", tblEntry->pktCmd, getTblEntry->pktCmd);
    }
    if(tblEntry->mirrorMask != getTblEntry->mirrorMask)
    {
        passStatus = false;
        printf("Mismatch::mirrorMask Expected Value %x Actual Value: %x \n", tblEntry->mirrorMask, getTblEntry->mirrorMask);
    }
    if(tblEntry->decTTL != getTblEntry->decTTL)
    {
        passStatus = false;
        printf("Mismatch::decTTL Expected Value %x Actual Value: %x \n", tblEntry->decTTL, getTblEntry->decTTL);
    }
    if(tblEntry->networkScope != getTblEntry->networkScope)
    {
        passStatus = false;
        printf("Mismatch::networkScope Expected Value %x Actual Value: %x \n", tblEntry->networkScope, getTblEntry->networkScope);
    }
    if(tblEntry->countMode != getTblEntry->countMode)
    {
        passStatus = false;
        printf("Mismatch::countMode Expected Value %x Actual Value: %x \n", tblEntry->countMode, getTblEntry->countMode);
    }
    if(tblEntry->RPFCheckType != getTblEntry->RPFCheckType)
    {
        passStatus = false;
        printf("Mismatch::RPFCheckType Expected Value %x Actual Value: %x \n", tblEntry->RPFCheckType, getTblEntry->RPFCheckType);
    }
    if(tblEntry->RPFCheckValue != getTblEntry->RPFCheckValue)
    {
        passStatus = false;
        printf("Mismatch::RPFCheckValue Expected Value %x Actual Value: %x \n", tblEntry->RPFCheckValue, getTblEntry->RPFCheckValue);
    }
    if(tblEntry->RPFCheckFailCmd != getTblEntry->RPFCheckFailCmd)
    {
        passStatus = false;
        printf("Mismatch::RPFCheckFailCmd Expected Value %x Actual Value: %x \n", tblEntry->RPFCheckFailCmd, getTblEntry->RPFCheckFailCmd);
    }
    if(tblEntry->egressVif != getTblEntry->egressVif)
    {
        passStatus = false;
        printf("Mismatch::egressVif Expected Value %x Actual Value: %x \n", tblEntry->egressVif, getTblEntry->egressVif);
    }
    if(passStatus)
    {
        printf("compareIpv4RouteMcEntry: PASS \n");
    }
    else
    {
        printf("compareIpv4RouteMcEntry: FAIL \n");
    }
}

int testIpv4RouteMcMgr(xpDevice_t deviceId, XP_PIPE_MODE mode)
{
    ipv4Addr_t ipAddr = {06,0x5A,23,02};
    ipv4Addr_t ipAddr1 = {0x19,0x23,0x45,0xDF};
    xpIpv4RouteMcEntry tblEntry;
    xpIpv4RouteMcEntry getTblEntry;
    uint32_t index = 0, reHashIndex = 0;

    memcpy(tblEntry.keySIP, ipAddr, sizeof(ipv4Addr_t));
    memcpy(tblEntry.keyGroupAddress, ipAddr1, sizeof(ipv4Addr_t));
    tblEntry.keyVRF = 0x25;
    tblEntry.pktCmd = 0x3;
    tblEntry.mirrorMask = 0x2;
    tblEntry.decTTL = 0x1;
    tblEntry.networkScope = 0x1;
    tblEntry.countMode = 0x7;
    tblEntry.RPFCheckType = 0x3;
    tblEntry.RPFCheckValue = 0xACB1;
    tblEntry.RPFCheckFailCmd = 0x1;
    tblEntry.egressVif = 0x4FDA;

    printf("dpsdbg: Inside %s func\n", __FUNCTION__);

    xpIpv4RouteMcMgr *xpPmMgr = static_cast<xpIpv4RouteMcMgr *>(xpMcastPl::instance()->getMgr(xpIpv4RouteMcMgr::getId()));

    /* Add an entry */
    if(xpPmMgr->addEntry(deviceId, (void *)&tblEntry, index, reHashIndex) != XP_NO_ERR)
    {
        printf("ERROR in inserting an entry. Exiting...\n");
        return -1;
    }

    tblEntry.pktCmd = 0x1;
    tblEntry.mirrorMask = 0x2;
    tblEntry.decTTL = 0x0;
    tblEntry.networkScope = 0x1;
    tblEntry.countMode = 0x2;
    tblEntry.RPFCheckType = 0x1;
    tblEntry.RPFCheckValue = 0x11FF;
    tblEntry.RPFCheckFailCmd = 0x2;
    tblEntry.egressVif = 0xF1B2;

    /* Set whole entry */
    if(xpPmMgr->writeEntry(deviceId, index, (void *)&tblEntry) != XP_NO_ERR)
    {
        printf("ERROR in updating an entry. Exiting...\n");
        return -1;
    }

    /* Read whole Entry */
    if(xpPmMgr->readEntry(deviceId, index, (void *)&getTblEntry) != XP_NO_ERR)
    {
        printf("ERROR in reading an entry. Exiting...\n");
        return -1;
    }
    compareIpv4RouteMcEntry(&tblEntry, &getTblEntry);
    tblEntry.printEntry();
    getTblEntry.printEntry();

    return 0;
}

void compareIpv6RouteMcEntry(xpIpv6RouteMcEntry * tblEntry, xpIpv6RouteMcEntry * getTblEntry)
{
    bool passStatus = true;

    for(int idx = 0; idx < 16 ; idx++)
    {
        if(tblEntry->keyIpv6SIP[idx] != getTblEntry->keyIpv6SIP[idx])
        {
            passStatus = false;
            printf("Mismatch::keyIpv6SIP[%d] Expected Value %x Actual Value: %x \n", idx, tblEntry->keyIpv6SIP[idx], getTblEntry->keyIpv6SIP[idx]);
        }
        if(tblEntry->keyIpv6DIP[idx] != getTblEntry->keyIpv6DIP[idx])
        {
            passStatus = false;
            printf("Mismatch::keyIpv6DIP[%d] Expected Value %x Actual Value: %x \n", idx, tblEntry->keyIpv6DIP[idx], getTblEntry->keyIpv6DIP[idx]);
        }
    }
    if(tblEntry->keyVRFID != getTblEntry->keyVRFID)
    {
        passStatus = false;
        printf("Mismatch::keyVRFID Expected Value %x Actual Value: %x \n", tblEntry->keyVRFID, getTblEntry->keyVRFID);
    }
    if(tblEntry->pktCmd != getTblEntry->pktCmd)
    {
        passStatus = false;
        printf("Mismatch::pktCmd Expected Value %x Actual Value: %x \n", tblEntry->pktCmd, getTblEntry->pktCmd);
    }
    if(tblEntry->mirrorMask != getTblEntry->mirrorMask)
    {
        passStatus = false;
        printf("Mismatch::mirrorMask Expected Value %x Actual Value: %x \n", tblEntry->mirrorMask, getTblEntry->mirrorMask);
    }
    if(tblEntry->decTTL != getTblEntry->decTTL)
    {
        passStatus = false;
        printf("Mismatch::decTTL Expected Value %x Actual Value: %x \n", tblEntry->decTTL, getTblEntry->decTTL);
    }
    if(tblEntry->networkScope != getTblEntry->networkScope)
    {
        passStatus = false;
        printf("Mismatch::networkScope Expected Value %x Actual Value: %x \n", tblEntry->networkScope, getTblEntry->networkScope);
    }
    if(tblEntry->countMode != getTblEntry->countMode)
    {
        passStatus = false;
        printf("Mismatch::countMode Expected Value %x Actual Value: %x \n", tblEntry->countMode, getTblEntry->countMode);
    }
    if(tblEntry->RPFCheckType != getTblEntry->RPFCheckType)
    {
        passStatus = false;
        printf("Mismatch::RPFCheckType Expected Value %x Actual Value: %x \n", tblEntry->RPFCheckType, getTblEntry->RPFCheckType);
    }
    if(tblEntry->RPFCheckValue != getTblEntry->RPFCheckValue)
    {
        passStatus = false;
        printf("Mismatch::RPFCheckValue Expected Value %x Actual Value: %x \n", tblEntry->RPFCheckValue, getTblEntry->RPFCheckValue);
    }
    if(tblEntry->RPFCheckFailCmd != getTblEntry->RPFCheckFailCmd)
    {
        passStatus = false;
        printf("Mismatch::RPFCheckFailCmd Expected Value %x Actual Value: %x \n", tblEntry->RPFCheckFailCmd, getTblEntry->RPFCheckFailCmd);
    }
    if(tblEntry->egressVif != getTblEntry->egressVif)
    {
        passStatus = false;
        printf("Mismatch::egressVif Expected Value %x Actual Value: %x \n", tblEntry->egressVif, getTblEntry->egressVif);
    }
    if(passStatus)
    {
        printf("compareIpv6RouteMcEntry: PASS \n");
    }
    else
    {
        printf("compareIpv6RouteMcEntry: FAIL \n");
    }
}

int testIpv6RouteMcMgr(xpDevice_t deviceId, XP_PIPE_MODE mode)
{
    ipv6Addr_t ipAddr = {06,0x5A,23,02,0xDA,0x12,0xFA,0x12,06,0x5A,23,02,0xDA,0x12,0xFA,0x12};
    ipv6Addr_t ipAddr1 = {0x19,0x23,0x45,0xDF,96,0x5A,63,42,0x1A,0x12,0xFA,0x13,0x1A,0x9A,0xA3,0x23};
    xpIpv6RouteMcEntry tblEntry;
    xpIpv6RouteMcEntry getTblEntry;
    uint32_t index = 0, reHashIndex = 0;
    int lookupIdx = -1;

    memcpy(tblEntry.keyIpv6SIP, ipAddr, sizeof(ipv6Addr_t));
    memcpy(tblEntry.keyIpv6DIP, ipAddr1, sizeof(ipv6Addr_t));
    tblEntry.keyVRFID = 0x25;
    tblEntry.pktCmd = 0x3;
    tblEntry.mirrorMask = 0x2;
    tblEntry.decTTL = 0x1;
    tblEntry.networkScope = 0x1;
    tblEntry.countMode = 0x7;
    tblEntry.RPFCheckType = 0x3;
    tblEntry.RPFCheckValue = 0xACB1;
    tblEntry.RPFCheckFailCmd = 0x1;
    tblEntry.egressVif = 0x4FDA;

    printf("dpsdbg: Inside %s func\n", __FUNCTION__);

    xpIpv6RouteMcMgr *xpPmMgr = static_cast<xpIpv6RouteMcMgr *>(xpMcastPl::instance()->getMgr(xpIpv6RouteMcMgr::getId()));

    /* Add an entry */
    if(xpPmMgr->addEntry(deviceId, (void *)&tblEntry, index, reHashIndex) != XP_NO_ERR)
    {
        printf("ERROR in inserting an entry. Exiting...\n");
        return -1;
    }

    tblEntry.pktCmd = 0x1;
    tblEntry.mirrorMask = 0x2;
    tblEntry.decTTL = 0x0;
    tblEntry.networkScope = 0x1;
    tblEntry.countMode = 0x2;
    tblEntry.RPFCheckType = 0x1;
    tblEntry.RPFCheckValue = 0x11FF;
    tblEntry.RPFCheckFailCmd = 0x2;
    tblEntry.egressVif = 0xF1B2;

    /* Set whole entry */
    if(xpPmMgr->writeEntry(deviceId, index, (void *)&tblEntry) != XP_NO_ERR)
    {
        printf("ERROR in updating an entry. Exiting...\n");
        return -1;
    }

    /* Read whole Entry */
    if(xpPmMgr->readEntry(deviceId, index, (void *)&getTblEntry) != XP_NO_ERR)
    {
        printf("ERROR in reading an entry. Exiting...\n");
        return -1;
    }
    compareIpv6RouteMcEntry(&tblEntry, &getTblEntry);
    tblEntry.printEntry();
    getTblEntry.printEntry();

    memset(&getTblEntry, 0x0, sizeof(getTblEntry));
    memcpy(getTblEntry.keyIpv6SIP, ipAddr, sizeof(ipv6Addr_t));
    memcpy(getTblEntry.keyIpv6DIP, ipAddr1, sizeof(ipv6Addr_t));
    getTblEntry.keyVRFID = 0x25;
    xpPmMgr->lookup(deviceId, (void *)&getTblEntry, lookupIdx);
    if(lookupIdx != (int32_t)index)
    {
        printf("Lookup Mismatch:: Expected Value %d Actual Value %d \n", index, lookupIdx);
    }
    else
    {
        printf("Lookup PASS:: Index %d Actual Value %d \n", lookupIdx, index);
        compareIpv6RouteMcEntry(&tblEntry, &getTblEntry);
    }

    return 0;
}

#if 0
void compareIpv4HostEntry(xpIpv4HostEntry * tblEntry, xpIpv4HostEntry * getTblEntry)
{
    bool passStatus = true;

    for(int idx = 0; idx < 4 ; idx++)
    {
        if(tblEntry->keyIPAddress[idx] != getTblEntry->keyIPAddress[idx])
        {
            passStatus = false;
            printf("Mismatch::keyIPAddress[%d] Expected Value %x Actual Value: %x \n", idx, tblEntry->keyIPAddress[idx], getTblEntry->keyIPAddress[idx]);
        }
    }
    if(tblEntry->keyVRF != getTblEntry->keyVRF)
    {
        passStatus = false;
        printf("Mismatch::keyVRF Expected Value %x Actual Value: %x \n", tblEntry->keyVRF, getTblEntry->keyVRF);
    }
    if(tblEntry->pktCmd != getTblEntry->pktCmd)
    {
        passStatus = false;
        printf("Mismatch::pktCmd Expected Value %x Actual Value: %x \n", tblEntry->pktCmd, getTblEntry->pktCmd);
    }
    if(tblEntry->mirrorMask != getTblEntry->mirrorMask)
    {
        passStatus = false;
        printf("Mismatch::mirrorMask Expected Value %x Actual Value: %x \n", tblEntry->mirrorMask, getTblEntry->mirrorMask);
    }
    if(tblEntry->decTTL != getTblEntry->decTTL)
    {
        passStatus = false;
        printf("Mismatch::decTTL Expected Value %x Actual Value: %x \n", tblEntry->decTTL, getTblEntry->decTTL);
    }
    if(tblEntry->networkScope != getTblEntry->networkScope)
    {
        passStatus = false;
        printf("Mismatch::networkScope Expected Value %x Actual Value: %x \n", tblEntry->networkScope, getTblEntry->networkScope);
    }
    if(tblEntry->countMode != getTblEntry->countMode)
    {
        passStatus = false;
        printf("Mismatch::countMode Expected Value %x Actual Value: %x \n", tblEntry->countMode, getTblEntry->countMode);
    }
    if(tblEntry->l2HdrInsertPtr != getTblEntry->l2HdrInsertPtr)
    {
        passStatus = false;
        printf("Mismatch::l2HdrInsertPtr Expected Value %x Actual Value: %x \n", tblEntry->l2HdrInsertPtr, getTblEntry->l2HdrInsertPtr);
    }
    if(tblEntry->egressVif != getTblEntry->egressVif)
    {
        passStatus = false;
        printf("Mismatch::egressVif Expected Value %x Actual Value: %x \n", tblEntry->egressVif, getTblEntry->egressVif);
    }
    for(int idx = 0; idx < 6 ; idx++)
    {
        if(tblEntry->macDA[idx] != getTblEntry->macDA[idx])
        {
            passStatus = false;
            printf("Mismatch::macDA[%d] Expected Value %x Actual Value: %x \n", idx, tblEntry->macDA[idx], getTblEntry->macDA[idx]);
        }
    }
    for(int idx = 0; idx < 2 ; idx++)
    {
        if(tblEntry->macSALSBs[idx] != getTblEntry->macSALSBs[idx])
        {
            passStatus = false;
            printf("Mismatch::macSALSBs[%d] Expected Value %x Actual Value: %x \n", idx, tblEntry->macSALSBs[idx], getTblEntry->macSALSBs[idx]);
        }
    }
    if(passStatus)
    {
        printf("compareIpv4HostEntry: PASS \n");
    }
    else
    {
        printf("compareIpv4HostEntry: FAIL \n");
    }
}

int testIpv4HostMgr(xpDevice_t deviceId, XP_PIPE_MODE mode)
{
    ipv4Addr_t ipAddr = {06,0x5A,23,02};
    macAddr_t macAddr = {0x19,0x23,0x45,0xDF,96,0x5A};
    macAddrLow_t macAddrLow = {0xA9,0xF3};
    xpIpv4HostEntry tblEntry;
    xpIpv4HostEntry getTblEntry;
    uint32_t index[100] = {0,}, reHashIndex[100] = {0,};
    int lookupIdx = -1;

    memcpy(tblEntry.keyIPAddress, ipAddr, sizeof(ipv4Addr_t));
    tblEntry.keyVRF = 0x25AA;
    tblEntry.pktCmd = 0x3;
    tblEntry.mirrorMask = 0x2;
    tblEntry.decTTL = 0x1;
    tblEntry.networkScope = 0x1;
    tblEntry.countMode = 0x7;
    tblEntry.l2HdrInsertPtr = 0xA1;
    tblEntry.egressVif = 0x4FDA;
    memcpy(tblEntry.macDA, macAddr, sizeof(macAddr_t));
    memcpy(tblEntry.macSALSBs, macAddrLow, sizeof(macAddrLow_t));

    printf("dpsdbg: Inside %s func\n", __FUNCTION__);

    xpIpv4HostMgr *xpPmMgr = static_cast<xpIpv4HostMgr *>(xpL3Pl::instance()->getMgr(xpIpv4HostMgr::getId()));

    for(int count = 0; count < 100; count++)
    {
        tblEntry.keyVRF = tblEntry.keyVRF + 5;
        /* Add an entry */
        if(xpPmMgr->addEntry(deviceId, (void *)&tblEntry, index[count], reHashIndex[count]) != XP_NO_ERR)
        {
            printf("ERROR in inserting an entry. Exiting...\n");
            //return -1;
        }
    }

    tblEntry.keyVRF = 0x25AA;
    tblEntry.pktCmd = 0x1;
    tblEntry.mirrorMask = 0x2;
    tblEntry.decTTL = 0x0;
    tblEntry.networkScope = 0x1;
    tblEntry.countMode = 0x2;
    tblEntry.l2HdrInsertPtr = 0x11;
    tblEntry.egressVif = 0xF1B2;

    for(int count = 0; count < 100; count++)
    {
        tblEntry.egressVif++;
        memset(&getTblEntry, 0x0, sizeof(getTblEntry));

        /* Set whole entry */
        if(xpPmMgr->writeEntry(deviceId, index[count], (void *)&tblEntry) != XP_NO_ERR)
        {
            printf("ERROR in updating an entry. Exiting...\n");
            return -1;
        }

        /* Read whole Entry */
        if(xpPmMgr->readEntry(deviceId, index[count], (void *)&getTblEntry) != XP_NO_ERR)
        {
            printf("ERROR in reading an entry. Exiting...\n");
            return -1;
        }
        tblEntry.keyVRF = tblEntry.keyVRF + 5;
        compareIpv4HostEntry(&tblEntry, &getTblEntry);
        //tblEntry.printEntry();
        //getTblEntry.printEntry();
    }

    memset(&getTblEntry, 0x0, sizeof(getTblEntry));
    memcpy(getTblEntry.keyIPAddress, ipAddr, sizeof(ipv4Addr_t));
    getTblEntry.keyVRF = 0x25AA + 5;
    xpPmMgr->lookup(deviceId, (void *)&getTblEntry, lookupIdx);
    if(lookupIdx != (int32_t)index[0])
    {
        printf("Lookup Mismatch:: Expected Value %d Actual Value %d \n", index[0], lookupIdx);
    }
    else
    {
        printf("Lookup PASS:: Index %d Actual Value %d \n", lookupIdx, index[0]);
        tblEntry.keyVRF = 0x25AA + 5;
        tblEntry.pktCmd = 0x1;
        tblEntry.mirrorMask = 0x2;
        tblEntry.decTTL = 0x0;
        tblEntry.networkScope = 0x1;
        tblEntry.countMode = 0x2;
        tblEntry.l2HdrInsertPtr = 0x11;
        tblEntry.egressVif = 0xF1B3;
        compareIpv4HostEntry(&tblEntry, &getTblEntry);
    }

    return 0;
}
#endif
void comparePipeSchedShaperCfgMgr(xpPipeSchedShaperCfgEntry *tblEntry, xpPipeSchedShaperCfgEntry *getTblEntry, uint8_t dataSzInBits)
{
    bool passStatus = true;
    int i = 0;
    for(i = 0; i < (dataSzInBits / SIZEOF_BYTE); i++)   {
        if(tblEntry->data[i] != getTblEntry->data[i])
        {
            passStatus = false;
            printf("Mismatch::data[%d] Expected Value %x Actual Value: %x \n", i, tblEntry->data[i], getTblEntry->data[i]);
        }
    }

    int bits = (dataSzInBits % SIZEOF_BYTE);

    if (bits > 0)
    {
        int mask = (1 << bits) - 1;
        if (((tblEntry->data[i]) & mask) != ((getTblEntry->data[i]) & mask))
        {
            passStatus = false;
            printf("Mismatch::data[%d] Expected Value %x Actual Value: %x \n", i, tblEntry->data[i], getTblEntry->data[i]);
        }
    }

    if(passStatus)
    {
        printf("comparePipeSchedShaperCfgMgr:PASS dataSzInBits : %d \n", dataSzInBits);
    }
    else
    {
        printf("comparePipeSchedShaperCfgMgr: FAIL dataSzInBits : %d \n", dataSzInBits);
    }
}

int testPipeSchedShaperCfgMgr(xpDevice_t deviceId)
{
    uint8_t entryData[32] = {0x06,0x5A,0x23,0x02,0x05,0x07,0x09,0x11,0x13,0x15,0x17,0x19,0x21,0x23,0x25,0x27,0x29,0x31,0x33,0x35,0x02,0x04,0x06,0x08,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x12};
    xpPipeSchedShaperCfgEntry tblEntry;
    xpPipeSchedShaperCfgEntry getTblEntry;

    /*   test XP_PIPE_SCH_TM_CFG */
    uint8_t dataSzInBits = 253; 
    memcpy(tblEntry.data, entryData, sizeof(entryData));
    tblEntry.dataSzInBits = dataSzInBits;
    printf("dpsdbg: Inside %s func\n", __FUNCTION__);

    xpPipeSchedShaperCfgType_t type = XP_PIPE_SCH_TM_CFG;

    for(int count = 0; count < 10; count++)
    {
        memset(&getTblEntry, 0x0, sizeof(getTblEntry));
        getTblEntry.dataSzInBits = dataSzInBits;

        /* Add an entry */
        if(pipeSchedShaperCfgMgr->writeEntry(deviceId, type, count, (void *)&tblEntry) != XP_NO_ERR)
        {
            printf("ERROR in writing an entry. Exiting...\n");
            return -1;
        }

        /* Read whole Entry */
        if(pipeSchedShaperCfgMgr->readEntry(deviceId, type, count, (void *)&getTblEntry) != XP_NO_ERR)
        {
            printf("ERROR in reading an entry. Exiting...\n");
            return -1;
        }
        comparePipeSchedShaperCfgMgr(&tblEntry, &getTblEntry, dataSzInBits);
    }


    /* test XP_PIPE_SCH_CB_FAST_UPD_SHAPERS */
    dataSzInBits = 53;  
    memset(&tblEntry, 0x0, sizeof(tblEntry));
    memcpy(tblEntry.data, entryData, sizeof(entryData));
    tblEntry.dataSzInBits = dataSzInBits;
    type = XP_PIPE_SCH_CB_FAST_UPD_SHAPERS;

    for(int count = 0; count < 10; count++)
    {
        memset(&getTblEntry, 0x0, sizeof(getTblEntry));
        getTblEntry.dataSzInBits = dataSzInBits;

        /* Add an entry */
        if(pipeSchedShaperCfgMgr->writeEntry(deviceId, type, count, (void *)&tblEntry) != XP_NO_ERR)
        {
            printf("ERROR in writing an entry. Exiting...\n");
            return -1;
        }

        /* Read whole Entry */
        if(pipeSchedShaperCfgMgr->readEntry(deviceId, type, count, (void *)&getTblEntry) != XP_NO_ERR)
        {
            printf("ERROR in reading an entry. Exiting...\n");
            return -1;
        }
        comparePipeSchedShaperCfgMgr(&tblEntry, &getTblEntry, dataSzInBits);
    }

    return 0;
}

void comparePortSchedShaperCfgMgr(xpPortSchedShaperCfgEntry *tblEntry, xpPortSchedShaperCfgEntry *getTblEntry, uint8_t dataSzInBits)
{
    bool passStatus = true;
    int i = 0;
    for(i = 0; i < (dataSzInBits / SIZEOF_BYTE); i++)   {
        if(tblEntry->data[i] != getTblEntry->data[i])
        {
            passStatus = false;
            printf("Mismatch::data[%d] Expected Value %x Actual Value: %x \n", i, tblEntry->data[i], getTblEntry->data[i]);
        }
    }

    int bits = (dataSzInBits % SIZEOF_BYTE);

    if (bits > 0)
    {
        int mask = (1 << bits) - 1;
        if (((tblEntry->data[i]) & mask) != ((getTblEntry->data[i]) & mask))
        {
            passStatus = false;
            printf("Mismatch::data[%d] Expected Value %x Actual Value: %x \n", i, tblEntry->data[i], getTblEntry->data[i]);
        }
    }

    if(passStatus)
    {
        printf("comparePortSchedShaperCfgMgr:PASS dataSzInBits : %d \n", dataSzInBits);
    }
    else
    {
        printf("comparePortSchedShaperCfgMgr: FAIL dataSzInBits : %d \n", dataSzInBits);
    }
}

int testPortSchedShaperCfgMgr(xpDevice_t deviceId)
{
    uint8_t entryData[8] = {0x06,0x5A,0x23,0x02,0x05,0x07,0x09,0x11};
    xpPortSchedShaperCfgEntry tblEntry;
    xpPortSchedShaperCfgEntry getTblEntry;

    /*   test XP_PORT_SCH_TM_CFG */
    uint8_t dataSzInBits = 60; 
    memcpy(tblEntry.data, entryData, sizeof(entryData));
    tblEntry.dataSzInBits = dataSzInBits;
    printf("dpsdbg: Inside %s func\n", __FUNCTION__);

    xpPortSchedShaperCfgType_t type = XP_PORT_SCH_TM_CFG;

    for(int count = 0; count < 10; count++)
    {
        memset(&getTblEntry, 0x0, sizeof(getTblEntry));
        getTblEntry.dataSzInBits = dataSzInBits;

        /* Add an entry */
        if(portSchedShaperCfgMgr->writeEntry(deviceId, type, count, (void *)&tblEntry) != XP_NO_ERR)
        {
            printf("ERROR in writing an entry. Exiting...\n");
            return -1;
        }

        /* Read whole Entry */
        if(portSchedShaperCfgMgr->readEntry(deviceId, type, count, (void *)&getTblEntry) != XP_NO_ERR)
        {
            printf("ERROR in reading an entry. Exiting...\n");
            return -1;
        }
        comparePortSchedShaperCfgMgr(&tblEntry, &getTblEntry, dataSzInBits);
    }


    /* test XP_PORT_SCH_TM_STATE_CFG */
    dataSzInBits = 2;  
    memset(&tblEntry, 0x0, sizeof(tblEntry));
    memcpy(tblEntry.data, entryData, sizeof(entryData));
    tblEntry.dataSzInBits = dataSzInBits;
    type =  XP_PORT_SCH_TM_STATE_CFG;

    for(int count = 0; count < 10; count++)
    {
        memset(&getTblEntry, 0x0, sizeof(getTblEntry));
        getTblEntry.dataSzInBits = dataSzInBits;

        /* Add an entry */
        if(portSchedShaperCfgMgr->writeEntry(deviceId, type, count, (void *)&tblEntry) != XP_NO_ERR)
        {
            printf("ERROR in writing an entry. Exiting...\n");
            return -1;
        }

        /* Read whole Entry */
        if(portSchedShaperCfgMgr->readEntry(deviceId, type, count, (void *)&getTblEntry) != XP_NO_ERR)
        {
            printf("ERROR in reading an entry. Exiting...\n");
            return -1;
        }
        comparePortSchedShaperCfgMgr(&tblEntry, &getTblEntry, dataSzInBits);
    }

    return 0;
}

void compareQmappingMgr(xpQmappingEntry *tblEntry, xpQmappingEntry *getTblEntry, xpQmappingType_t type)
{
    bool passStatus = true;
    if ( type == XP_QMAP)
    {
        if(tblEntry->QMappingData.QmapTbl.q0 != getTblEntry->QMappingData.QmapTbl.q0)
        {
            passStatus = false;
            printf("Mismatch::QmapTbl.q0 Expected Value %x Actual Value: %x \n", tblEntry->QMappingData.QmapTbl.q0, getTblEntry->QMappingData.QmapTbl.q0);
        }

        if(tblEntry->QMappingData.QmapTbl.q1 != getTblEntry->QMappingData.QmapTbl.q1)
        {
            passStatus = false;
            printf("Mismatch::QmapTbl.q1 Expected Value %x Actual Value: %x \n", tblEntry->QMappingData.QmapTbl.q1, getTblEntry->QMappingData.QmapTbl.q1);
        }

        if(tblEntry->QMappingData.QmapTbl.q2 != getTblEntry->QMappingData.QmapTbl.q2)
        {
            passStatus = false;
            printf("Mismatch::QmapTbl.q2 Expected Value %x Actual Value: %x \n", tblEntry->QMappingData.QmapTbl.q2, getTblEntry->QMappingData.QmapTbl.q2);
        }

        if(tblEntry->QMappingData.QmapTbl.q3 != getTblEntry->QMappingData.QmapTbl.q3)
        {
            passStatus = false;
            printf("Mismatch::QmapTbl.q3 Expected Value %x Actual Value: %x \n", tblEntry->QMappingData.QmapTbl.q3, getTblEntry->QMappingData.QmapTbl.q3);
        }

    }
    else if ( type == XP_QMAPTBLINDEX)
    {
        if(tblEntry->QMappingData.QmapTblIndexCfg.arrayIndex != getTblEntry->QMappingData.QmapTblIndexCfg.arrayIndex)
        {
            passStatus = false;
            printf("Mismatch::QmapTblIndexCfg.array_index Expected Value %x Actual Value: %x \n", tblEntry->QMappingData.QmapTblIndexCfg.arrayIndex, getTblEntry->QMappingData.QmapTblIndexCfg.arrayIndex);
        }
    }

    if(passStatus)
    {
        printf("compareQmappingMgr: PASS, type : %d \n", type);
    }
    else
    {
        printf("compareQmappingMgr: FAIL, type : %d \n", type);
    }
}


int testQmappingMgr(xpDevice_t deviceId)
{
    xpQmappingEntry tblEntry;
    xpQmappingEntry getTblEntry;

    /*   test XP_QMAP */
    tblEntry.QMappingData.QmapTbl.q0 = 0x123;
    tblEntry.QMappingData.QmapTbl.q1 = 0x246;
    tblEntry.QMappingData.QmapTbl.q0 = 0x492;
    tblEntry.QMappingData.QmapTbl.q0 = 0x984;
    xpQmappingType_t type = XP_QMAP;
    printf("dpsdbg: Inside %s func\n", __FUNCTION__);

    for(int count = 0; count < 10; count++)
    {
        memset(&getTblEntry, 0x0, sizeof(getTblEntry));

        /* Add an entry */
        if(qmappingMgr->writeEntry(deviceId, type, count, (void *)&tblEntry) != XP_NO_ERR)
        {
            printf("ERROR in writing an entry. Exiting...\n");
            return -1;
        }

        /* Read whole Entry */
        if(qmappingMgr->readEntry(deviceId, type, count, (void *)&getTblEntry) != XP_NO_ERR)
        {
            printf("ERROR in reading an entry. Exiting...\n");
            return -1;
        }
        compareQmappingMgr(&tblEntry, &getTblEntry, type);
    }

    /* XP_QMAPTBLINDEX */
    tblEntry.QMappingData.QmapTblIndexCfg.arrayIndex = 0x33;
    type = XP_QMAPTBLINDEX;

    for(int count = 0; count < 10; count++)
    {
        memset(&getTblEntry, 0x0, sizeof(getTblEntry));

        /* Add an entry */
        if(qmappingMgr->writeEntry(deviceId, type, count, (void *)&tblEntry) != XP_NO_ERR)
        {
            printf("ERROR in writing an entry. Exiting...\n");
            return -1;
        }

        /* Read whole Entry */
        if(qmappingMgr->readEntry(deviceId, type, count, (void *)&getTblEntry) != XP_NO_ERR)
        {
            printf("ERROR in reading an entry. Exiting...\n");
            return -1;
        }
        compareQmappingMgr(&tblEntry, &getTblEntry, type);
    }

    return 0;
}

void comparePortMappingCfgMgr(xpPortMappingCfgEntry *tblEntry, xpPortMappingCfgEntry *getTblEntry, uint8_t dataSzInBits)
{
    bool passStatus = true;
    int bits = (dataSzInBits % SIZEOF_BYTE);

    if (bits > 0)
    {
        int mask = (1 << bits) - 1;
        if (((tblEntry->data) & mask) != ((getTblEntry->data) & mask))
        {
            passStatus = false;
            printf("Mismatch::data Expected Value %x Actual Value: %x \n", tblEntry->data, getTblEntry->data);
        }
    }

    if(passStatus)
    {
        printf("comparePortMappingCfgMgr:PASS dataSzInBits : %d \n", dataSzInBits);
    }
    else
    {
        printf("comparePortMappingCfgMgr: FAIL dataSzInBits : %d \n", dataSzInBits);
    }
}

int testPortMappingCfgMgr(xpDevice_t deviceId)
{
    xpPortMappingCfgEntry tblEntry;
    xpPortMappingCfgEntry getTblEntry;

    /*   test XP_P2PG_MAP_CFG */
    uint8_t dataSzInBits = 3; 
    tblEntry.data = 0x7;
    tblEntry.dataSzInBits = dataSzInBits;
    printf("dpsdbg: Inside %s func\n", __FUNCTION__);

    xpPortMappingCfgType_t type = XP_P2PG_MAP_CFG;

    for(int count = 0; count < 10; count++)
    {
        memset(&getTblEntry, 0x0, sizeof(getTblEntry));
        getTblEntry.dataSzInBits = dataSzInBits;

        /* Add an entry */
        if(portMappingCfgMgr->writeEntry(deviceId, type, count, (void *)&tblEntry) != XP_NO_ERR)
        {
            printf("ERROR in writing an entry. Exiting...\n");
            return -1;
        }

        /* Read whole Entry */
        if(portMappingCfgMgr->readEntry(deviceId, type, count, (void *)&getTblEntry) != XP_NO_ERR)
        {
            printf("ERROR in reading an entry. Exiting...\n");
            return -1;
        }
        comparePortMappingCfgMgr(&tblEntry, &getTblEntry, dataSzInBits);
    }


    /* test XP_PORT_SPEED_CFG */
    dataSzInBits = 4;  
    tblEntry.data = 0x9;
    tblEntry.dataSzInBits = dataSzInBits;
    type = XP_PORT_SPEED_CFG;

    for(int count = 0; count < 10; count++)
    {
        memset(&getTblEntry, 0x0, sizeof(getTblEntry));
        getTblEntry.dataSzInBits = dataSzInBits;

        /* Add an entry */
        if(portMappingCfgMgr->writeEntry(deviceId, type, count, (void *)&tblEntry) != XP_NO_ERR)
        {
            printf("ERROR in writing an entry. Exiting...\n");
            return -1;
        }

        /* Read whole Entry */
        if(portMappingCfgMgr->readEntry(deviceId, type, count, (void *)&getTblEntry) != XP_NO_ERR)
        {
            printf("ERROR in reading an entry. Exiting...\n");
            return -1;
        }
        comparePortMappingCfgMgr(&tblEntry, &getTblEntry, dataSzInBits);
    }

    return 0;
}


void compareIaclData(xpIaclEntry *tblEntry, xpIaclEntry *getTblEntry)
{
    bool passStatus = true;

    if(tblEntry->data.lookupPrio != getTblEntry->data.lookupPrio)
    {
        passStatus = false;
        printf("Mismatch::lookupPrio Expected Value %x Actual Value: %x \n", tblEntry->data.lookupPrio, getTblEntry->data.lookupPrio);
    }
    if(tblEntry->data.enPktCmdUpd != getTblEntry->data.enPktCmdUpd)
    {
        passStatus = false;
        printf("Mismatch::enPktCmdUpd Expected Value %x Actual Value: %x \n", tblEntry->data.enPktCmdUpd, getTblEntry->data.enPktCmdUpd);
    }
    if(tblEntry->data.enRedirectToEvif != getTblEntry->data.enRedirectToEvif)
    {
        passStatus = false;
        printf("Mismatch::enRedirectToEvif Expected Value %x Actual Value: %x \n", tblEntry->data.enRedirectToEvif, getTblEntry->data.enRedirectToEvif);
    }
    if(tblEntry->data.enRsnCodeUpd != getTblEntry->data.enRsnCodeUpd)
    {
        passStatus = false;
        printf("Mismatch::enRsnCodeUpd Expected Value %x Actual Value: %x \n", tblEntry->data.enRsnCodeUpd, getTblEntry->data.enRsnCodeUpd);
    }
    if(tblEntry->data.enPolicer != getTblEntry->data.enPolicer)
    {
        passStatus = false;
        printf("Mismatch::enPolicer Expected Value %x Actual Value: %x \n", tblEntry->data.enPolicer, getTblEntry->data.enPolicer);
    }
    if(tblEntry->data.enCnt != getTblEntry->data.enCnt)
    {
        passStatus = false;
        printf("Mismatch::enCnt Expected Value %x Actual Value: %x \n", tblEntry->data.enCnt, getTblEntry->data.enCnt);
    }
    if(tblEntry->data.enMirrorSsnUpd != getTblEntry->data.enMirrorSsnUpd)
    {
        passStatus = false;
        printf("Mismatch::enMirrorSsnUpd Expected Value %x Actual Value: %x \n", tblEntry->data.enMirrorSsnUpd, getTblEntry->data.enMirrorSsnUpd);
    }
    if(tblEntry->data.enTcUpd != getTblEntry->data.enTcUpd)
    {
        passStatus = false;
        printf("Mismatch::enTcUpd Expected Value %x Actual Value: %x \n", tblEntry->data.enTcUpd, getTblEntry->data.enTcUpd);
    }
    if(tblEntry->data.pktCmd != getTblEntry->data.pktCmd)
    {
        passStatus = false;
        printf("Mismatch::pktCmd Expected Value %x Actual Value: %x \n", tblEntry->data.pktCmd, getTblEntry->data.pktCmd);
    }
    if(tblEntry->data.eVifId != getTblEntry->data.eVifId)
    {
        passStatus = false;
        printf("Mismatch::eVifId Expected Value %x Actual Value: %x \n", tblEntry->data.eVifId, getTblEntry->data.eVifId);
    }
    if(tblEntry->data.rsnCode != getTblEntry->data.rsnCode)
    {
        passStatus = false;
        printf("Mismatch::rsnCode Expected Value %x Actual Value: %x \n", tblEntry->data.rsnCode, getTblEntry->data.rsnCode);
    }
    if(tblEntry->data.policerId != getTblEntry->data.policerId)
    {
        passStatus = false;
        printf("Mismatch::policerId Expected Value %x Actual Value: %x \n", tblEntry->data.policerId, getTblEntry->data.policerId);
    }
    if(tblEntry->data.cntId != getTblEntry->data.cntId)
    {
        passStatus = false;
        printf("Mismatch::cntId Expected Value %x Actual Value: %x \n", tblEntry->data.cntId, getTblEntry->data.cntId);
    }
    if(tblEntry->data.TC != getTblEntry->data.TC)
    {
        passStatus = false;
        printf("Mismatch::TC Expected Value %x Actual Value: %x \n", tblEntry->data.TC, getTblEntry->data.TC);
    }
      
    if(passStatus)
    {
        printf("compareIaclData: PASS \n");
    } else
    {
        printf("compareIaclData: FAIL \n");
    }
}

void compareIaclData(xpIaclData_t *tblEntry, xpIaclData_t *getTblEntry)
{
    bool passStatus = true;

    if(tblEntry->lookupPrio != getTblEntry->lookupPrio)
    {
        passStatus = false;
        printf("Mismatch::lookupPrio Expected Value %x Actual Value: %x \n", tblEntry->lookupPrio, getTblEntry->lookupPrio);
    }
    if(tblEntry->enPktCmdUpd != getTblEntry->enPktCmdUpd)
    {
        passStatus = false;
        printf("Mismatch::enPktCmdUpd Expected Value %x Actual Value: %x \n", tblEntry->enPktCmdUpd, getTblEntry->enPktCmdUpd);
    }
    if(tblEntry->enRedirectToEvif != getTblEntry->enRedirectToEvif)
    {
        passStatus = false;
        printf("Mismatch::enRedirectToEvif Expected Value %x Actual Value: %x \n", tblEntry->enRedirectToEvif, getTblEntry->enRedirectToEvif);
    }
    if(tblEntry->enRsnCodeUpd != getTblEntry->enRsnCodeUpd)
    {
        passStatus = false;
        printf("Mismatch::enRsnCodeUpd Expected Value %x Actual Value: %x \n", tblEntry->enRsnCodeUpd, getTblEntry->enRsnCodeUpd);
    }
    if(tblEntry->enPolicer != getTblEntry->enPolicer)
    {
        passStatus = false;
        printf("Mismatch::enPolicer Expected Value %x Actual Value: %x \n", tblEntry->enPolicer, getTblEntry->enPolicer);
    }
    if(tblEntry->enCnt != getTblEntry->enCnt)
    {
        passStatus = false;
        printf("Mismatch::enCnt Expected Value %x Actual Value: %x \n", tblEntry->enCnt, getTblEntry->enCnt);
    }
    if(tblEntry->enMirrorSsnUpd != getTblEntry->enMirrorSsnUpd)
    {
        passStatus = false;
        printf("Mismatch::enMirrorSsnUpd Expected Value %x Actual Value: %x \n", tblEntry->enMirrorSsnUpd, getTblEntry->enMirrorSsnUpd);
    }
    if(tblEntry->enTcUpd != getTblEntry->enTcUpd)
    {
        passStatus = false;
        printf("Mismatch::enTcUpd Expected Value %x Actual Value: %x \n", tblEntry->enTcUpd, getTblEntry->enTcUpd);
    }
    if(tblEntry->pktCmd != getTblEntry->pktCmd)
    {
        passStatus = false;
        printf("Mismatch::pktCmd Expected Value %x Actual Value: %x \n", tblEntry->pktCmd, getTblEntry->pktCmd);
    }
    if(tblEntry->eVifId != getTblEntry->eVifId)
    {
        passStatus = false;
        printf("Mismatch::eVifId Expected Value %x Actual Value: %x \n", tblEntry->eVifId, getTblEntry->eVifId);
    }
    if(tblEntry->rsnCode != getTblEntry->rsnCode)
    {
        passStatus = false;
        printf("Mismatch::rsnCode Expected Value %x Actual Value: %x \n", tblEntry->rsnCode, getTblEntry->rsnCode);
    }
    if(tblEntry->policerId != getTblEntry->policerId)
    {
        passStatus = false;
        printf("Mismatch::policerId Expected Value %x Actual Value: %x \n", tblEntry->policerId, getTblEntry->policerId);
    }
    if(tblEntry->cntId != getTblEntry->cntId)
    {
        passStatus = false;
        printf("Mismatch::cntId Expected Value %x Actual Value: %x \n", tblEntry->cntId, getTblEntry->cntId);
    }
    if(tblEntry->TC != getTblEntry->TC)
    {
        passStatus = false;
        printf("Mismatch::TC Expected Value %x Actual Value: %x \n", tblEntry->TC, getTblEntry->TC);
    }

    if(passStatus)
    {
        printf("compareIaclData: PASS \n");
    } else
    {
        printf("compareIaclData: FAIL \n");
    }
}

int testIaclMgr(xpDevice_t devId, XP_PIPE_MODE mode)
{
    macAddr_t macAddr = { 06, 0x5A, 23, 00, 01, 02 };
    macAddr_t macAddr1 = { 16, 0xAA, 93, 10, 51, 82 };
    //ipv4Addr_t ipAddr = {06,0x5A,23,02};
    //ipv4Addr_t ipAddr1 = {0x19,0x23,0x45,0xDF};
    ipv6Addr_t ipv6Addr = { 0x56, 0x85, 0x76, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33 };
    ipv6Addr_t ipv6Addr1 = { 0x56, 0x45, 0x96, 0xFF, 0xEE, 0xAA, 0xCC, 0xBB, 0xAF, 0x99, 0x80, 0x77, 0x68, 0x55, 0x44, 0x90 };
    xpIaclEntry tblEntry;
    xpIaclEntry getTblEntry;
    xpIaclKeyMask_t tblKey, tblMask;
    xpIaclKeyMask_t getTblKey, getTblMask;
    xpIaclData_t tblData;
    xpIaclData_t getTblData;
    uint32_t index = 0;
    int32_t lookupIdx = -1;
    uint32_t policerId = 1, getPolicerId = 0;

    printf("dpsdbg: Inside %s func\n", __FUNCTION__);

    iaclMgr = static_cast<xpIaclMgr *>(xpAclPl::instance()->getMgr(xpIaclMgr::getId()));
    //iaclMgr->add
    tblEntry.key.isValid = 1;
    tblEntry.key.type = 0;
    tblEntry.key.keyType.l2Ipv4.bd = 20;
    memcpy(tblEntry.key.keyType.l2Ipv4.macDA, macAddr, sizeof(macAddr_t));
    tblEntry.mask.type = 0;
    memset(tblEntry.mask.keyType.l2Ipv4.macDA, 0x0, sizeof(macAddr_t));
    tblEntry.data.policerId = 0xAD2;
    tblEntry.data.eVifId = 0xF1;

    tblKey.type = 0;
    tblKey.isValid = 1;
    tblKey.keyType.l2Ipv4.bd = 50;
    memcpy(tblKey.keyType.l2Ipv4.macDA, macAddr1, sizeof(macAddr_t));
    memset((void *)&tblMask, 0x0, sizeof(tblMask));
    tblMask.isValid = 1;
    tblData.policerId = 0xFA4;
    tblData.eVifId = 0x123;

    xpIaclType_e iaclType = XP_IACL0;

    for(index = 0; index < 512; index++)
    {
        memset(&getTblEntry, 0x0, sizeof(getTblEntry));
        tblEntry.key.keyType.l2Ipv4.bd += 0x10;

     
        printf("\nwriteEntry(): \n");
        //tblEntry.printEntry();
        if(iaclMgr->writeEntry(devId, iaclType, index, (void *)&tblEntry) != XP_NO_ERR)
        {
            printf("ERROR in inserting an entry. Exiting...\n");
            return -1;
        }
        printf("\nreadEntry(): \n");
        iaclMgr->readEntry(devId, iaclType, index, &getTblEntry);
        compareIaclData(&tblEntry, &getTblEntry);
        //getTblEntry.printEntry();
    }

    memset((void *)&tblEntry, 0x0, sizeof(tblEntry));
    tblEntry.key.type = 0;
    tblEntry.key.keyType.l2Ipv4.bd = 20;
    memcpy(tblEntry.key.keyType.l2Ipv4.macDA, macAddr1, sizeof(macAddr_t));

    tblEntry.key.keyType.l2Ipv4.bd += 0x10;
    printf("isValid bit not set in mask\n");
    iaclMgr->lookup(devId, iaclType, (void *)&tblEntry, lookupIdx);
    if(lookupIdx != 0)
    {
        printf("Lookup Mismatch:: Expected Value 0 Actual Value %d \n", lookupIdx);
    }

    for(index = 0; index < 512; index++)
    {
        memset((void *)&getTblKey, 0x0, sizeof(getTblKey));
        memset((void *)&getTblMask, 0x0, sizeof(getTblMask));
        tblKey.keyType.l2Ipv4.bd += 0x10;

        printf("\nwriteKeyMask(): \n");
        if(iaclMgr->writeKeyMask(devId, iaclType, index, &tblKey, &tblMask) != XP_NO_ERR)
        {
            printf("ERROR in inserting an entry. Exiting...\n");
            return -1;
        }

        printf("\nreadKeyMask(): \n");
        iaclMgr->readKeyMask(devId, iaclType, index, &getTblKey, &getTblMask);

        memset((void *)&getTblData, 0x0, sizeof(getTblData));

        printf("\nwriteData(): \n");
        if(iaclMgr->writeData(devId, iaclType, index, &tblData) != XP_NO_ERR)
        {
            printf("ERROR in inserting an entry. Exiting...\n");
            return -1;
        }
        printf("\nreadData(): \n");
        iaclMgr->readData(devId, iaclType, index, &getTblData);
        compareIaclData(&tblData, &getTblData);

        printf("\nsetPolicerId(): \n");
        if(iaclMgr->setField(devId, iaclType, index, IACL_POLICER_ID, (void *)&policerId) != XP_NO_ERR)
        {
            printf("ERROR in setting field policerId. Exiting...\n");
            return -1;
        }
        printf("\ngetPolicerId(): \n");
        iaclMgr->getField(devId, iaclType, index, IACL_POLICER_ID, (void *)&getPolicerId);
        if(policerId != getPolicerId)
        {
            printf("Mismatch::policerId Expected Value %x Actual Value: %x \n", policerId, getPolicerId);
            printf("Set/getPolicerId: FAIL \n");
        } else
        {
            printf("Set/getPolicerId: PASS \n");
        }
    }

    memset((void *)&tblEntry, 0x0, sizeof(tblEntry));
    tblEntry.key.type = 0;
    tblEntry.key.keyType.l2Ipv4.bd = 50;
    memcpy(tblEntry.key.keyType.l2Ipv4.macDA, macAddr1, sizeof(macAddr_t));

    for(index = 0; index < 512; index++)
    {
        tblEntry.key.keyType.l2Ipv4.bd += 0x10;
        iaclMgr->lookup(devId, iaclType, (void *)&tblEntry, lookupIdx);

        if(lookupIdx != (int32_t)index)
        {
            printf("Lookup Mismatch:: Expected Value %d Actual Value %d \n", index, lookupIdx);
            return -1;
        }

        memset(&getTblEntry, 0x0, sizeof(getTblEntry));
        printf("\nremoveEntry(): \n");
        if(iaclMgr->removeEntry(devId, iaclType, index) != XP_NO_ERR)
        {
            printf("ERROR in removing an entry. Exiting...\n");
            return -1;
        }
        printf("\nreadEntry(): \n");
        iaclMgr->readEntry(devId, iaclType, index, &getTblEntry);
        //getTblEntry.printEntry();
    }
    #if 1
    iaclType = XP_IACL1;
    tblEntry.key.type = 1;
    tblEntry.key.keyType.ipv6.BD = 0xAA;
    memcpy(tblEntry.key.keyType.ipv6.DIP, ipv6Addr, sizeof(ipv6Addr_t));
    tblEntry.mask.type = 0;
    memset(tblEntry.mask.keyType.ipv6.DIP, 0x0, sizeof(ipv6Addr_t));
    tblEntry.data.policerId = 0xD41;
    tblEntry.data.eVifId = 0x9F1;

    memset((void *)&tblKey, 0x0, sizeof(tblKey));
    tblKey.type = 2;
    tblKey.keyType.ipv6.BD = 0x7D;
    memcpy(tblKey.keyType.ipv6.DIP, ipv6Addr1, sizeof(ipv6Addr_t));
    memset((void *)&tblMask, 0x0, sizeof(tblMask));
    tblData.policerId = 0xFA4;
    tblData.eVifId = 0x765;

    for(index = 0; index < 100; index++)
    {
        memset(&getTblEntry, 0x0, sizeof(getTblEntry));
        tblEntry.key.keyType.ipv6.BD += 0x30;

        printf("\nwriteEntry(): \n");
        if(iaclMgr->writeEntry(devId, iaclType, index, (void *)&tblEntry) != XP_NO_ERR)
        {
            printf("ERROR in inserting an entry. Exiting...\n");
            return -1;
        }
        printf("\nreadEntry(): \n");
        iaclMgr->readEntry(devId, iaclType, index, &getTblEntry);
        compareIaclData(&tblEntry, &getTblEntry);
        //getTblEntry.printEntry();

        memset((void *)&getTblKey, 0x0, sizeof(getTblKey));
        memset((void *)&getTblMask, 0x0, sizeof(getTblMask));
        tblKey.keyType.ipv6.BD += 0x30;

        printf("\nwriteKeyMask(): \n");
        if(iaclMgr->writeKeyMask(devId, iaclType, index, &tblKey, &tblMask) != XP_NO_ERR)
        {
            printf("ERROR in inserting an entry. Exiting...\n");
            return -1;
        }
        printf("\nreadKeyMask(): \n");
        iaclMgr->readKeyMask(devId, iaclType, index, &getTblKey, &getTblMask);

        memset((void *)&getTblData, 0x0, sizeof(getTblData));

        printf("\nwriteData(): \n");
        if(iaclMgr->writeData(devId, iaclType, index, &tblData) != XP_NO_ERR)
        {
            printf("ERROR in inserting an entry. Exiting...\n");
            return -1;
        }
        printf("\nreadData(): \n");
        iaclMgr->readData(devId, iaclType, index, &getTblData);
        compareIaclData(&tblData, &getTblData);

        printf("\nsetPolicerId(): \n");
        if(iaclMgr->setField(devId, iaclType, index, IACL_POLICER_ID, (void *)&policerId) != XP_NO_ERR)
        {
            printf("ERROR in setting field policerId. Exiting...\n");
            return -1;
        }
        printf("\ngetPolicerId(): \n");
        iaclMgr->getField(devId, iaclType, index, IACL_POLICER_ID, (void *)&getPolicerId);
        if(policerId != getPolicerId)
        {
            printf("Mismatch::policerId Expected Value %x Actual Value: %x \n", policerId, getPolicerId);
            printf("Set/getPolicerId: FAIL \n");
        } else
        {
            printf("Set/getPolicerId: PASS \n");
        }

    }
    #endif
    return 0;
}

void compareNATIpv4Data(xpNATIpv4Data_t *tblData, xpNATIpv4Data_t *getTblData)
{
    bool passStatus = true;
    if(memcmp(&tblData->SIPAddress[0], &getTblData->SIPAddress[0], sizeof(tblData->SIPAddress[0])) < 0)
    {
        passStatus = false;
        printf("Mismatch field: Expected value: %d, Actual value: %d\n", tblData->SIPAddress[0], getTblData->SIPAddress[0]);
    }
    if(memcmp(&tblData->SIPAddress[1], &getTblData->SIPAddress[1], sizeof(tblData->SIPAddress[1])) < 0)
    {
        passStatus = false;
        printf("Mismatch field: Expected value: %d, Actual value: %d\n", tblData->SIPAddress[0], getTblData->SIPAddress[0]);
    }
    if(memcmp(&tblData->SIPAddress[2], &getTblData->SIPAddress[2], sizeof(tblData->SIPAddress[2])) < 0)
    {
        passStatus = false;
        printf("Mismatch field: Expected value: %d, Actual value: %d\n", tblData->SIPAddress[0], getTblData->SIPAddress[0]);
    }
    if(memcmp(&tblData->SIPAddress[3], &getTblData->SIPAddress[3], sizeof(tblData->SIPAddress[3])) < 0)
    {
        passStatus = false;
        printf("Mismatch field: Expected value: %d, Actual value: %d\n", tblData->SIPAddress[0], getTblData->SIPAddress[0]);
    }
    if(tblData->srcPort != getTblData->srcPort)
    {
        passStatus = false;
        printf("Mismatch fielExpected value: %d, Actual value: %d\n", tblData->srcPort, getTblData->srcPort);
    }
    if(memcmp(&tblData->DIPAddress[0], &getTblData->DIPAddress[0], sizeof(tblData->DIPAddress[0])) < 0)
    {
        passStatus = false;
        printf("Mismatch field: Expected value: %d, Actual value: %d\n", tblData->DIPAddress[0], getTblData->DIPAddress[0]);
    }
    if(memcmp(&tblData->DIPAddress[1], &getTblData->DIPAddress[1], sizeof(tblData->DIPAddress[1])) < 0)
    {
        passStatus = false;
        printf("Mismatch field: Expected value: %d, Actual value: %d\n", tblData->DIPAddress[0], getTblData->DIPAddress[0]);
    }
    if(memcmp(&tblData->DIPAddress[2], &getTblData->DIPAddress[2], sizeof(tblData->DIPAddress[2])) < 0)
    {
        passStatus = false;
        printf("Mismatch field: Expected value: %d, Actual value: %d\n", tblData->DIPAddress[0], getTblData->DIPAddress[0]);
    }
    if(memcmp(&tblData->DIPAddress[3], &getTblData->DIPAddress[3], sizeof(tblData->DIPAddress[3])) < 0)
    {
        passStatus = false;
        printf("Mismatch field: Expected value: %d, Actual value: %d\n", tblData->DIPAddress[0], getTblData->DIPAddress[0]);
    }
    if(tblData->destPort != getTblData->destPort)
    {
        passStatus = false;
        printf("Mismatch fieldExpected value: %d, Actual value: %d\n", tblData->destPort, getTblData->destPort);
    }
    if(tblData->vif != getTblData->vif)
    {
        passStatus = false;
        printf("Mismatch Expected value: %d, Actual value: %d\n", tblData->vif, getTblData->vif);
    }
    if(tblData->pktCmd != getTblData->pktCmd)
    {
        passStatus = false;
        printf("Mismatch fieExpected value: %d, Actual value: %d\n", tblData->pktCmd, getTblData->pktCmd);
    }
    if(passStatus)
    {
        printf("compareNATIpv4Data: PASS \n");
    } else
    {
        printf("compareNATIpv4Data: FAIL \n");
    }

}

int testNATIpv4Mgr(xpDevice_t devId, XP_PIPE_MODE mode)
{
    ipv4Addr_t srcIpAddr = {192, 168, 2, 1};
    ipv4Addr_t destIpAddr = {192, 168, 2, 5};
    xpNATIpv4Entry tblEntry;
    xpNATIpv4Entry getTblEntry;
    xpNATIpv4KeyMask_t tblKey, tblMask, getTblKey, getTblMask;
    xpNATIpv4Data_t tblData, getTblData;
    uint32_t index = 0;
    int32_t lookupIdx = -1;
    uint32_t srcPort = 1, getSrcPort = 0;

    printf("dpsdbg: Inside %s func\n", __FUNCTION__);

    natipv4Mgr = static_cast<xpNATIpv4Mgr *>(xpL3Pl::instance()->getMgr(xpNATIpv4Mgr::getId()));
    memset(&tblEntry, 0x0, sizeof(tblEntry));
    /* Update key */
    tblEntry.key.IsValid = 1;
    tblEntry.key.SrcPort = 15;
    memcpy(tblEntry.key.SrcAddress, srcIpAddr, sizeof(ipv4Addr_t));
    tblEntry.key.DestPort = 20;
    memcpy(tblEntry.key.DestAddress, destIpAddr, sizeof(ipv4Addr_t));
    tblEntry.key.Bd = 10;

    /* Update mask */
    tblEntry.mask.SrcPort = 20;
    memset(tblEntry.mask.SrcAddress, 0, sizeof(ipv4Addr_t));
    tblEntry.mask.DestPort = 30;
    memset(tblEntry.mask.DestAddress, 0, sizeof(ipv4Addr_t));

    /* Update data */
    memcpy(tblEntry.data.SIPAddress, srcIpAddr, sizeof(ipv4Addr_t));
    memcpy(tblEntry.data.DIPAddress, destIpAddr, sizeof(ipv4Addr_t));
    tblEntry.data.vif = 0x1234;
    tblEntry.data.pktCmd = 1;

    /* Test write entry */
    for(index = 0; index < 512; index++) //TODO What is the depth?
    {
        memset(&getTblEntry, 0x0, sizeof(getTblEntry));
        tblEntry.key.Bd += 10;

        printf("\nwriteEntry(): \n");
        if(natipv4Mgr->writeEntry(devId, index, (void *)&tblEntry) != XP_NO_ERR)
        {
            printf("ERROR in inserting an entry. Exiting...\n");
            return -1;
        }
        printf("\nreadEntry(): \n");
        natipv4Mgr->readEntry(devId, index, &getTblEntry);
        compareNATIpv4Data(&tblEntry.data, &getTblEntry.data);
    }

    //TODO Why is this used?
    #if 0 
    memset((void *)&tblEntry, 0x0, sizeof(tblEntry));
    tblEntry.key.type = 0;
    tblEntry.key.keyType.l2Ipv4.bd = 20;
    memcpy(tblEntry.key.keyType.l2Ipv4.macDA, ipAddr1, sizeof(ipAddr_t));

    tblEntry.key.keyType.l2Ipv4.bd += 0x10;
    printf("isValid bit not set in mask\n");
    natipv4Mgr->lookup(devId, natipv4Type, (void *)&tblEntry, lookupIdx);
    if(lookupIdx != 0)
    {
        printf("Lookup Mismatch:: Expected Value 0 Actual Value %d \n", lookupIdx);
    }
    #endif

    /* Update key */
    tblKey.IsValid = 1;
    tblKey.SrcPort = 15;
    memcpy(tblKey.SrcAddress, srcIpAddr, sizeof(ipv4Addr_t));
    tblKey.DestPort = 20;
    memcpy(tblKey.DestAddress, destIpAddr, sizeof(ipv4Addr_t));
    tblKey.Bd = 10;

    /* Update mask */
    tblMask.IsValid = 1;

    /* Update data */
    memcpy(&tblData.SIPAddress, srcIpAddr, sizeof(ipv4Addr_t));
    memcpy(&tblData.DIPAddress, destIpAddr, sizeof(ipv4Addr_t));
    tblData.vif = 0x1266;

    for(index = 0; index < 512; index++)
    {
        //TODO what do these APIs do?
        memset((void *)&getTblKey, 0x0, sizeof(getTblKey));
        memset((void *)&getTblMask, 0x0, sizeof(getTblMask));

        /* Read and write key mask */
        printf("\nwriteKeyMask(): \n");
        if(natipv4Mgr->writeKeyMask(devId, index, &tblKey, &tblMask) != XP_NO_ERR)
        {
            printf("ERROR in inserting an entry. Exiting...\n");
            return -1;
        }

        printf("\nreadKeyMask(): \n");
        natipv4Mgr->readKeyMask(devId, index, &getTblKey, &getTblMask);

        /* Read and write data */
        memset((void *)&getTblData, 0x0, sizeof(getTblData));

        printf("\nwriteData(): \n");
        if(natipv4Mgr->writeData(devId, index, &tblData) != XP_NO_ERR)
        {
            printf("ERROR in inserting an entry. Exiting...\n");
            return -1;
        }
        printf("\nreadData(): \n");
        natipv4Mgr->readData(devId, index, &getTblData);
        compareNATIpv4Data(&tblData, &getTblData);

        /* Set/Get field */
        printf("\nsetsrcPort(): \n");
        if(natipv4Mgr->setField(devId, index, NATIPV4_KEY_SRC_PORT, (void *)&srcPort) != XP_NO_ERR)
        {
            printf("ERROR in setting field srcPort. Exiting...\n");
            return -1;
        }
        printf("\ngetSrcPort(): \n");
        natipv4Mgr->getField(devId, index, NATIPV4_KEY_SRC_PORT, (void *)&getSrcPort);
        if(srcPort != getSrcPort)
        {
            printf("Mismatch::srcPort Expected Value %x Actual Value: %x \n", srcPort, getSrcPort);
            printf("Set/getSrcPort: FAIL \n");
        } else
        {
            printf("Set/getSrcPort: PASS \n");
        }
    }

    /* Update key */
    memset(&tblEntry, 0x0, sizeof(tblEntry));
    tblEntry.key.IsValid = 1;
    tblEntry.key.SrcPort = 15;
    memcpy(tblEntry.key.SrcAddress, srcIpAddr, sizeof(ipv4Addr_t));
    tblEntry.key.DestPort = 20;
    memcpy(tblEntry.key.DestAddress, destIpAddr, sizeof(ipv4Addr_t));
    tblEntry.key.Bd = 10;

    /* Update mask */
    tblEntry.mask.SrcPort = 20;
    memset(tblEntry.mask.SrcAddress, 0, sizeof(ipv4Addr_t));
    tblEntry.mask.DestPort = 30;
    memset(tblEntry.mask.DestAddress, 0, sizeof(ipv4Addr_t));

    /* Lookup and remove entries */
    for(index = 0; index < 512; index++)
    {
        tblEntry.key.Bd += 10;
        natipv4Mgr->lookup(devId, (void *)&tblEntry, lookupIdx);

        if(lookupIdx != (int32_t)index)
        {
            printf("Lookup Mismatch:: Expected Value %d Actual Value %d \n", index, lookupIdx);
            return -1;
        }

        printf("\nremoveEntry(): \n");
        if(natipv4Mgr->removeEntry(devId, index) != XP_NO_ERR)
        {
            printf("ERROR in removing an entry. Exiting...\n");
            return -1;
        }
        printf("\nreadEntry(): \n");
        memset(&getTblEntry, 0x0, sizeof(getTblEntry));
        natipv4Mgr->readEntry(devId, index, &getTblEntry);
    }
    /* ============== Re-test all APIs ========================== */
    /* Update key */
    tblEntry.key.IsValid = 1;
    tblEntry.key.SrcPort = 15;
    memcpy(tblEntry.key.SrcAddress, srcIpAddr, sizeof(ipv4Addr_t));
    tblEntry.key.DestPort = 20;
    memcpy(tblEntry.key.DestAddress, destIpAddr, sizeof(ipv4Addr_t));
    tblEntry.key.Bd = 10;

    /* Update mask */
    tblEntry.mask.SrcPort = 20;
    memset(tblEntry.mask.SrcAddress, 0, sizeof(ipv4Addr_t));
    tblEntry.mask.DestPort = 30;
    memset(tblEntry.mask.DestAddress, 0, sizeof(ipv4Addr_t));

    /* Update data */
    memcpy(tblEntry.data.SIPAddress, srcIpAddr, sizeof(ipv4Addr_t));
    memcpy(tblEntry.data.DIPAddress, destIpAddr, sizeof(ipv4Addr_t));
    tblEntry.data.vif = 0x1234;
    tblEntry.data.pktCmd = 1;

    for(index = 0; index < 100; index++)
    {
        /* Write and read entry */
        memset(&getTblEntry, 0x0, sizeof(getTblEntry));
        
        tblEntry.key.Bd += 10;

        printf("\nwriteEntry(): \n");
        if(natipv4Mgr->writeEntry(devId, index, (void *)&tblEntry) != XP_NO_ERR)
        {
            printf("ERROR in inserting an entry. Exiting...\n");
            return -1;
        }
        printf("\nreadEntry(): \n");
        natipv4Mgr->readEntry(devId, index, &getTblEntry);
        compareNATIpv4Data(&tblEntry.data, &getTblEntry.data);

        /* Write and read key mask */
        memset((void *)&getTblKey, 0x0, sizeof(getTblKey));
        memset((void *)&getTblMask, 0x0, sizeof(getTblMask));

        printf("\nwriteKeyMask(): \n");
        if(natipv4Mgr->writeKeyMask(devId, index, &tblKey, &tblMask) != XP_NO_ERR)
        {
            printf("ERROR in inserting an entry. Exiting...\n");
            return -1;
        }

        printf("\nreadKeyMask(): \n");
        natipv4Mgr->readKeyMask(devId, index, &getTblKey, &getTblMask);

        /* Read and write data */
        memset((void *)&getTblData, 0x0, sizeof(getTblData));

        printf("\nwriteData(): \n");
        if(natipv4Mgr->writeData(devId, index, &tblData) != XP_NO_ERR)
        {
            printf("ERROR in inserting an entry. Exiting...\n");
            return -1;
        }
        printf("\nreadData(): \n");
        natipv4Mgr->readData(devId, index, &getTblData);
        compareNATIpv4Data(&tblData, &getTblData);

        /* Set/Get field */
        printf("\nsetsrcPort(): \n");
        if(natipv4Mgr->setField(devId, index, NATIPV4_KEY_SRC_PORT, (void *)&srcPort) != XP_NO_ERR)
        {
            printf("ERROR in setting field srcPort. Exiting...\n");
            return -1;
        }
        printf("\ngetSrcPort(): \n");
        natipv4Mgr->getField(devId, index, NATIPV4_KEY_SRC_PORT, (void *)&getSrcPort);
        if(srcPort != getSrcPort)
        {
            printf("Mismatch::srcPort Expected Value %x Actual Value: %x \n", srcPort, getSrcPort);
            printf("Set/getSrcPort: FAIL \n");
        } else
        {
            printf("Set/getSrcPort: PASS \n");
        }
    }
    return 0;
}

void primitiveTableUnitTests(xpDevice_t devId, XP_PIPE_MODE mode)
{
#if 0
    cout << "####### Before calling testVifMgr #######" << endl;
    testVifMgr(devId, mode);   // Added to test vif manager basic functionality
    cout << "####### After testVifMgr completes #######" << endl;

    cout << "####### Before calling testHeaderModificationMgr #######" << endl;
    testHeaderModificationMgr(devId, mode); // Added to test header Modification manager basic functionality
    cout << "####### After testHeaderModificationMgr completes #######" << endl;

    cout << "####### Before calling testControlMacMgr #######" << endl;
    testControlMacMgr(devId);    // Added to test Control Mac manager basic functionality
    cout << "####### After testControlMacMgr completes #######" << endl;

    cout << "####### Before calling testEgressFilterMgr #######" << endl;
    testEgressFilterMgr(devId);  // Added to test Egress Filter manager basic functionality
    cout << "####### After testEgressFilterMgr completes #######" << endl;

    cout << "####### Before calling testTrunkResolutionMgr #######" << endl;
    testTrunkResolutionMgr(devId);   // Added to test Trunk Resolution manager basic functionality
    cout << "####### After testTrunkResolutionMgr completes #######" << endl;

    cout << "####### Before calling testBdMgr #######" << endl;
    testBdMgr(devId, mode);    // Added to test BD manager basic functionality
    cout << "####### After testBdMgr completes #######" << endl;

    cout << "####### Before calling testFDB #######" << endl;
    testFDB(devId, mode);  // Added to test FDB manager basic functionality
    cout << "####### After testFDB completes #######" << endl;

    cout << "####### Before calling testIpv4RouteMcMgr #######" << endl;
    testIpv4RouteMcMgr(devId, mode);    // Added to test Ipv4RouteMc manager basic functionality
    cout << "####### After testIpv4RouteMcMgr completes #######" << endl;

    cout << "####### Before calling testIpv6RouteMcMgr #######" << endl;
    //testIpv6RouteMcMgr(devId, mode);    // Added to test Ipv6RouteMc manager basic functionality
    cout << "####### After testIpv6RouteMcMgr completes #######" << endl;

    cout << "####### Before calling testIpv4HostMgr #######" << endl;
    testIpv4HostMgr(devId, mode);    // Added to test Ipv4Host manager basic functionality
    cout << "####### After testIpv4HostMgr completes #######" << endl;

    cout << "####### Before calling testIaclMgr #######" << endl;
    testIaclMgr(devId, mode);    // Added to test IACL manager basic functionality
    cout << "####### After testIaclMgr completes #######" << endl;
#endif

    cout << "####### Before calling testPipeSchedShaperCfgMgr #######" << endl;
    testPipeSchedShaperCfgMgr(devId);    // Added to test PipeSchedShaperCfgMgr manager basic functionality
    cout << "####### After testPipeSchedShaperCfgMgr completes #######" << endl;

    cout << "####### Before calling testPortSchedShaperCfgMgr #######" << endl;
    testPortSchedShaperCfgMgr(devId);    // Added to test PortSchedShaperCfgMgr manager basic functionality
    cout << "####### After testPortSchedShaperCfgMgr completes #######" << endl;

    cout << "####### Before calling testPortMappingCfgMgr #######" << endl;
    testPortMappingCfgMgr(devId);    // Added to test testPortMappingCfgMgr manager basic functionality
    cout << "####### After testPortMappingCfgMgr completes #######" << endl;

    cout << "####### Before calling testQmapingMgr #######" << endl;
    testQmappingMgr(devId);    // Added to test QmappingMgr manager basic functionality
    cout << "####### After testQmappinMgr completes #######" << endl;
    
    cout << "####### Before calling testMPLSTunnel #######" << endl;
    testMplsTunnel(devId, mode);  // Added to test MPLS Tunnel manager basic functionality
    cout << "####### After testMplsTunnel completes #######" << endl;

    cout << "####### Before calling testNATIpv4Mgr #######" << endl;
    testNATIpv4Mgr(devId, mode);  // Added to test MPLS Tunnel manager basic functionality
    cout << "####### After testNATIpv4Mgr completes #######" << endl;
}

#define XP_IF_ERR(EXPR)        { if ((status = (EXPR)) != XP_NO_ERR) printf("UNIT TEST: %s, error code = %1d.\n", #EXPR, (int)status); }
void testXp80IsmeMgr()
{
    printf("------------------------------ xp80IsmeMgr Test ------------------------------\n");
    XP_STATUS status = XP_NO_ERR;
    uint32_t setValue = 0;
    uint32_t getValue = 0;

    for (uint32_t sde = 0; sde < XP_SDE_NUM; ++sde)
    {
        for (uint32_t isme = 0; isme < XP_PG_MAX_NUM_ISME; ++ isme)
        {
            for (uint32_t lm = 0; lm < XP_ISME_MAX_LM_TABLES; ++lm)
            {
                for (setValue = 0; setValue <= 1; ++setValue)
                {
                    getValue = 100;     // Value is intentionally out of range.
                    XP_IF_ERR(xp80IsmeMgr::instance()->setLmLookupBypass(XP_TEST_DEV_NUM, sde, isme, lm, setValue));
                    XP_IF_ERR(xp80IsmeMgr::instance()->getLmLookupBypass(XP_TEST_DEV_NUM, sde, isme, lm, getValue));
                    if (setValue != getValue)
                    {
                        printf("Failed to write %1d to SDE %1d, ISME %1d, LM %1d lookup bypass register.\n",
                               (int) setValue, (int) sde, (int) isme, (int) lm);
                    }
                }
            }
        }
    }

    printf("testXp80IsmeMgr: DONE -------------------\n");
}

#if 0
void testMirrorMgr()
{
    cout << "------------------------------ Mirror Mgr Test ------------------------------" << endl;
    XP_STATUS status = XP_NO_ERR;
    const int numUserSessions = XP_MIRROR_MAX_USER_SESSION - XP_MIRROR_MIN_USER_SESSION + 1;
    std::vector<std::pair<xpPort_t, xpVif_t> > sessionMap[numUserSessions];
    std::vector<xpPort_t> portList(XP_MAX_TOTAL_PORTS);
    for (int i= 0; i < XP_MAX_TOTAL_PORTS; ++i)
    {
        portList[i]= (xpPort_t) i;
    }
    std::random_shuffle(portList.begin(), portList.end());

    xpPortConfigMgr* portCfgMgr = (xpPortConfigMgr*) xpIfPl::instance()->getMgr(xpPortConfigMgr::getId());
    if (portCfgMgr == NULL)
    {
        printf("testMirrorMgr: Failed to access xpPortConfigMgr.\n");
        return;
    }

    xpMirrorMgr* mirrorMgr = xpMirrorMgr::instance();
    if (mirrorMgr == NULL)
    {
        printf("testMirrorMgr: Failed to access xpMirrorMgr.\n");
        return;
    }

    //XP_IF_ERR(mirrorMgr->initDevice(devId));
    xpVif_t vif;
    int port = 0;
    int count = 0;
    uint32_t session= XP_MIRROR_MIN_USER_SESSION;
    while (session <= XP_MIRROR_MAX_USER_SESSION && port < XP_MAX_TOTAL_PORTS)
    {
        printf("testMirrorMgr: bindAnalyzer(session = %1d) -----------------\n", (int) session);
        count = std::rand() % 4;
        for (int i= 0; i < count && port < XP_MAX_TOTAL_PORTS; ++i)
        {
            portCfgMgr->getField(XP_TEST_DEV_NUM, portList[port], PORTCONFIG_INGRESS_VIF, (void*) &vif);
            sessionMap[session - XP_MIRROR_MIN_USER_SESSION].push_back(std::make_pair(portList[port], vif));
            XP_IF_ERR(mirrorMgr->bindAnalyzer(XP_TEST_DEV_NUM, session, vif));
            ++port;
        }
        ++session;
    }

#if 0
    for (int session= XP_MIRROR_MAX_USER_SESSION; session >= XP_MIRROR_MIN_USER_SESSION; --session)
    {
        printf("testMirrorMgr: unbindAnalyzer(session = %1d) -----------------\n", session);
        std::random_shuffle(sessionMap[session - XP_MIRROR_MIN_USER_SESSION].begin(), sessionMap[session - XP_MIRROR_MIN_USER_SESSION].end());
        while (sessionMap[session - XP_MIRROR_MIN_USER_SESSION].size() > 0)
        {
            XP_IF_ERR(mirrorMgr->unbindAnalyzer(XP_TEST_DEV_NUM, (uint32_t) session, sessionMap[session - XP_MIRROR_MIN_USER_SESSION][0].second));
            sessionMap[session - XP_MIRROR_MIN_USER_SESSION].erase(sessionMap[session - XP_MIRROR_MIN_USER_SESSION].begin());
        }
    }
#endif

    int firstMirroredPortIndex = port;
    for (int session= XP_MIRROR_MIN_USER_SESSION; session <= XP_MIRROR_MAX_USER_SESSION && port < XP_MAX_TOTAL_PORTS; ++session, ++port)
    {
        printf("testMirrorMgr: enablePortMirror(deviceId=%1d, sessionId=%1d, portId=%1d)\n", 0, (int) session, portList[port]);
        XP_IF_ERR(mirrorMgr->enablePortMirror(XP_TEST_DEV_NUM, (uint32_t) session, portList[port]));
    }

#if 0
    xpVlanCtx_t vlanCtx;
    xpVlan_t vlan = 99;
    uint32_t stgIdx;
    XP_IF_ERR(xpVlanMgr::instance()->allocateVlan(vlan, vlanCtx));
    XP_IF_ERR(xpVlanMgr::instance()->createStg(stgIdx));
    XP_IF_ERR(xpVlanMgr::instance()->createVlan(XP_TEST_DEV_NUM, stgIdx, vlanCtx));
    XP_IF_ERR(xpVlanMgr::instance()->setStgFilterEn(XP_TEST_DEV_NUM, vlanCtx, 1));
    for (int session= XP_MIRROR_MIN_USER_SESSION; session <= XP_MIRROR_MAX_USER_SESSION; ++session)
    {
        printf("testMirrorMgr: enableVlanMirror(deviceId=%1d, sessionId=%1d, vlanCtx=%p)\n",
               0, (int) session, &vlanCtx);
        XP_IF_ERR(mirrorMgr->enableVlanMirror(XP_TEST_DEV_NUM, session, &vlanCtx));
    }
#endif

    printf("testMirrorMgr: enableLagMirror(deviceId=%1d, sessionId=%1d, lagId=%1d)\n",
           0, (int) XP_MIRROR_MIN_USER_SESSION,   1);
    XP_IF_ERR(mirrorMgr->enableLagMirror(XP_TEST_DEV_NUM, XP_MIRROR_MIN_USER_SESSION,   1));
    printf("testMirrorMgr: enableLagMirror(deviceId=%1d, sessionId=%1d, lagId=%1d)\n",
           0, (int) XP_MIRROR_MIN_USER_SESSION+1, 2);
    XP_IF_ERR(mirrorMgr->enableLagMirror(XP_TEST_DEV_NUM, XP_MIRROR_MIN_USER_SESSION+1, 2));
    printf("testMirrorMgr: enableLagMirror(deviceId=%1d, sessionId=%1d, lagId=%1d)\n",
           0, (int) XP_MIRROR_MIN_USER_SESSION+2, 3);
    XP_IF_ERR(mirrorMgr->enableLagMirror(XP_TEST_DEV_NUM, XP_MIRROR_MIN_USER_SESSION+2, 3));

    port= firstMirroredPortIndex;
    for (int session= XP_MIRROR_MIN_USER_SESSION; session <= XP_MIRROR_MAX_USER_SESSION && port < XP_MAX_TOTAL_PORTS; ++session, ++port)
    {
        printf("testMirrorMgr: disablePortMirror(deviceId=%1d, sessionId=%1d, portId=%1d)\n", 0, (int) session, portList[port]);
        XP_IF_ERR(mirrorMgr->disablePortMirror(XP_TEST_DEV_NUM, (uint32_t) session, portList[port]));
    }

#if 0
    for (int session= XP_MIRROR_MIN_USER_SESSION; session <= XP_MIRROR_MAX_USER_SESSION; ++session)
    {
        printf("testMirrorMgr: disableVlanMirror(deviceId=%1d, sessionId=%1d, vlanCtx=%p)\n",
               0, (int) session, &vlanCtx);
        XP_IF_ERR(mirrorMgr->disableVlanMirror(XP_TEST_DEV_NUM, session, &vlanCtx));
    }
#endif

    printf("testMirrorMgr: disableLagMirror(deviceId=%1d, sessionId=%1d, lagId=%1d)\n",
           0, (int) XP_MIRROR_MIN_USER_SESSION,   1);
    XP_IF_ERR(mirrorMgr->disableLagMirror(XP_TEST_DEV_NUM, XP_MIRROR_MIN_USER_SESSION,   1));
    printf("testMirrorMgr: disableLagMirror(deviceId=%1d, sessionId=%1d, lagId=%1d)\n",
           0, (int) XP_MIRROR_MIN_USER_SESSION+1, 2);
    XP_IF_ERR(mirrorMgr->disableLagMirror(XP_TEST_DEV_NUM, XP_MIRROR_MIN_USER_SESSION+1, 2));
    printf("testMirrorMgr: disableLagMirror(deviceId=%1d, sessionId=%1d, lagId=%1d)\n",
           0, (int) XP_MIRROR_MIN_USER_SESSION+2, 3);
    XP_IF_ERR(mirrorMgr->disableLagMirror(XP_TEST_DEV_NUM, XP_MIRROR_MIN_USER_SESSION+2, 3));

#if 0
    XP_IF_ERR(mirrorMgr->enableAclMirror(XP_TEST_DEV_NUM, uint32_t sessionId, uint32_t ruleIndex));
    XP_IF_ERR(mirrorMgr->disableAclMirror(XP_TEST_DEV_NUM, uint32_t sessionId, uint32_t ruleIndex));
#endif

    printf("testMirrorMgr: REPORT -----------------\n");
    for (int session= XP_MIRROR_MIN_USER_SESSION; session <= XP_MIRROR_MAX_USER_SESSION; ++session)
    {
        mirrorMgr->printSession(XP_TEST_DEV_NUM, (uint32_t) session, 0);
    }

    printf("testMirrorMgr: DONE -------------------\n");
}
#endif
//
// Top Level Init for all Feature Layer Mgrs
//
XP_STATUS xpAppPLInit(xpDevice_t devNum, XP_PIPE_MODE mode)
{
    XP_STATUS status = XP_NO_ERR;
    bool initFail = false;

    //
    // Init the VIF and relatives
    //
    if((status = xpIfPl::instance()->createMgr(xpHdrModificationMgr::getId())) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for xpHdrModMgr .\n Exiting...");
        return XP_ERR_INIT;
    }
    xpHdrModificationMgr *hdrModMgr = static_cast<xpHdrModificationMgr *>(xpIfPl::instance()->getMgr(xpHdrModificationMgr::getId()));
    if(hdrModMgr == NULL)
    {
        printf("ERROR in getMgr for xpHdrModMgr .\n Exiting...");
        return XP_ERR_INIT;
    }

    printf("xpHdrModMgr created successfully\n");
    std::vector<xpIdRangeInfo *> idDepth;

    for(uint32_t i = 0; i < 3; i++)
    {
        idDepth.push_back(new xpIdRangeInfo());
    }
    idDepth[0]->index = XP_INSERTION0; //256bit entry width
    idDepth[0]->start = 0;
    idDepth[0]->depth = MAX_INS0_ID(mode);

    idDepth[1]->index = XP_INSERTION1; //256bit entry width
    idDepth[1]->start = idDepth[0]->depth;
    idDepth[1]->depth = MAX_INS1_ID(mode);

    idDepth[2]->index = XP_INSERTION2; //256bit entry width
    idDepth[2]->start = idDepth[1]->start + idDepth[1]->depth;
    idDepth[2]->depth = MAX_INS2_ID(mode);

    if(hdrModMgr->init((idDepth[2]->start + idDepth[2]->depth), idDepth) != XP_NO_ERR)
    {
        printf("ERROR in hdrModMgr->init()\n");
        initFail = true;
        //return XP_ERR_INIT;
    }

    for(uint32_t i = 0; i < 3; i++)
    {
        delete idDepth[i];
    }
    idDepth.clear();
    printf("Header Mod manager initialized successfully!!!\n");

    // 
    // Creation and initialization for vif manager
    //
    if((status = xpIfPl::instance()->createMgr(xpVifMgr::getId())) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for VIF()\n");
        return status;
    }

    printf("xpVifMgr created successfully\n");

    xpVifMgr *vifMgr = static_cast<xpVifMgr *>(xpIfPl::instance()->getMgr(xpVifMgr::getId())) ;
    if(vifMgr == NULL)
    {
        printf("ERROR in getMgr for VIF()\n");
        return status;
    }

    for(uint32_t i = 0; i < 5; i++)
    {
        idDepth.push_back(new xpIdRangeInfo());
    }

    idDepth[0]->index = XP_VIF_SINGLE_PORT_DATA;
    idDepth[0]->start = 0;
    idDepth[0]->depth = MAX_VIF_DATA_ID(mode);

    idDepth[1]->index = XP_VIF_SINGLE_PORT_LARGE_DATA;
    idDepth[1]->start = idDepth[0]->start + idDepth[0]->depth;
    idDepth[1]->depth = MAX_VIF_LARGE_DATA_ID(mode);

    idDepth[2]->index = XP_VIF_TUNNEL_ENCAP;
    idDepth[2]->start = idDepth[1]->start + idDepth[1]->depth;
    idDepth[2]->depth = MAX_VIF_TUNNEL_ID(mode);

    idDepth[3]->index = XP_VIF_LAG;
    idDepth[3]->start = idDepth[2]->start + idDepth[2]->depth;
    idDepth[3]->depth = MAX_VIF_LAG_ID(mode);

    idDepth[4]->index = XP_VIF_MULTI_PORT;
    idDepth[4]->start = idDepth[3]->start + idDepth[3]->depth;
    idDepth[4]->depth = MAX_VIF_MULTI_ID(mode);

    if((status = vifMgr->init((idDepth[4]->start + idDepth[4]->depth), idDepth)) != XP_NO_ERR)
    {
        printf("ERROR in vifMgr Init\n");
        initFail = true;
        //return XP_ERR_INIT;
    }

     for(uint32_t i = 0; i < 5; i++)
    {
        delete idDepth[i];
    }
    idDepth.clear();
    printf("xpVifMgr initialized successfully\n");

    //
    // Initialize egress filter table manager
    //
    if(xpIfPl::instance()->createMgr(xpEgressFilterMgr::getId()) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for egFltMgr manager.\n Exiting...");
        return XP_ERR_INIT;
    }
    egFltMgr = static_cast<xpEgressFilterMgr *>(xpIfPl::instance()->getMgr(xpEgressFilterMgr::getId()));

    if(egFltMgr == NULL)
    {
        printf("egFltMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(egFltMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: egFltMgr manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }

    //
    //Init BD manager
    //
    if(xpL2Pl::instance()->createMgr(xpBdMgr::getId()) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for BD manager.\n Exiting...");
        return XP_ERR_INIT;
    }

    bdMgr = (xpBdMgr *)xpL2Pl::instance()->getMgr(xpBdMgr::getId());

    if(bdMgr == NULL)
    {
        printf("bdMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }

    if(bdMgr->init(MAX_BD_ID(mode)) != XP_NO_ERR)
    {
        printf("ERROR: BD manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }

    //
    //Init MPLS Tunnel manager
    //
    if(xpTunnelPl::instance()->createMgr(xpMplsTunnelMgr::getId()) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for BD manager.\n Exiting...");
        return XP_ERR_INIT;
    }

    mplsTunnelMgr = (xpMplsTunnelMgr *)xpTunnelPl::instance()->getMgr(xpMplsTunnelMgr::getId());

    if(mplsTunnelMgr == NULL)
    {
        printf("mplsTunnelMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }

    if(mplsTunnelMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: mplsTunnelMgr init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }

#if 0
    if(xpAcmPl::instance()->createMgr(xpAcmBankMgr::getId()) != XP_NO_ERR)
    {
        printf("Error in createMgr for ACM Bank manager.\n Exiting...");
        return XP_ERR_INIT;
    }
    /* initialize ACM bank manager */
    acmBnkMgr = (xpAcmBankMgr *)xpAcmPl::instance()->getMgr(xpAcmBankMgr::getId());

    if(xpAcmPl::instance()->createMgr(xpAcmRsltMgr::getId()) != XP_NO_ERR)
    {
        printf("Error in createMgr for ACM RSLT manager.\n Exiting...");
        return XP_ERR_INIT;
    }
    /* initialize ACM RSLT manager */
    acmRstMgr = (xpAcmRsltMgr *)xpAcmPl::instance()->getMgr(xpAcmRsltMgr::getId());

    if ((acmRstMgr == NULL) || (acmBnkMgr == NULL) || (acmBnkMgr == acmRstMgr))
    {
        printf("acmBnkMgr or acmRstMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }

    /* initialize ACM bank manager */
    if (acmBnkMgr->init(devNum) != XP_NO_ERR)
    {
        printf("ERROR: ACM BANK Init failed. Exiting...\n");
        initFail = true;
    }

    /* initialize ACM RSLT manager */
    if (acmRstMgr->init(devNum) != XP_NO_ERR)
    {
        printf("ERROR: ACM RSLT Init failed. Exiting...\n");
        initFail = true;
    }
#endif
    //
    // Init Control Mac Manager
    //
    xpIfPl::instance()->createMgr(xpControlMacMgr::getId());
    controlMacMgr = static_cast<xpControlMacMgr *>(xpIfPl::instance()->getMgr(xpControlMacMgr::getId()));

    if(controlMacMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: Init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }

    //
    // Init FDB Mgr
    //
    if(xpL2Pl::instance()->createMgr(xpFdbMgr::getId()) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for Fdb manager.\n Exiting...");
        return XP_ERR_INIT;
    }

    fdbMgr = (xpFdbMgr *)xpL2Pl::instance()->getMgr(xpFdbMgr::getId());

    if(fdbMgr == NULL)
    {
        printf("fdbMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(fdbMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: Fdb manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }

    //
    //Init the MDT Manager
    // 
    if(xpIfPl::instance()->createMgr(xpMdtMgr::getId()) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for MDT manager.\n Exiting...");
        return XP_ERR_INIT;
    }
    mdtMgr = (xpMdtMgr *)xpIfPl::instance()->getMgr(xpMdtMgr::getId());

    if(mdtMgr == NULL)
    {
        printf("xpMdtMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }

    if(mdtMgr->init(MAX_MDT_ID(mode)) != XP_NO_ERR) //Intialize MDT manager with maxIds as 4096
    {
        printf("ERROR: MDT manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }

    //
    // Initialize Trunk Resolution Table
    // 
    if((status = (xpIfPl::instance()->createMgr(xpTrunkResolutionMgr::getId()))) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for trunkResolutionMgr\n Exiting...");
        return XP_ERR_INIT;
    }

    trunkResolutionMgr = (xpTrunkResolutionMgr *)xpIfPl::instance()->getMgr(xpTrunkResolutionMgr::getId());

    if(trunkResolutionMgr == NULL)
    {
        printf("trunkResolutionMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if((status = trunkResolutionMgr->init()) != XP_NO_ERR)
    {
        printf("ERROR: trunkResolutionMgr init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }

    //
    // Initialize portConfig manager
    //
    if((status = xpIfPl::instance()->createMgr(xpPortConfigMgr::getId())) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for portConfig manager.\n Exiting...");
        return status;
    }
    portCfgMgr = static_cast<xpPortConfigMgr *>(xpIfPl::instance()->getMgr(xpPortConfigMgr::getId()));

    if(portCfgMgr == NULL)
    {
        printf("portCfgMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if((status = portCfgMgr->init()) != XP_NO_ERR)
    {
        printf("ERROR: portConfig manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }

    //
    //Init the PortVlan Manager 
    //
    if(xpL2Pl::instance()->createMgr(xpPortVlanIvifMgr::getId()) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for portVlanMgr\n Exiting...");
        return XP_ERR_INIT;
    }

    portVlanMgr = (xpPortVlanIvifMgr *)xpL2Pl::instance()->getMgr(xpPortVlanIvifMgr::getId());

    if(portVlanMgr == NULL)
    {
        printf("portVlanMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(portVlanMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: portVlanMgr init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }

    //
    //Init the tunnel manager
    //
    if(xpTunnelPl::instance()->createMgr(xpTunnelIvifMgr::getId()) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for tunnellIvifMgr.\n Exiting...");
        return XP_ERR_INIT;
    }
    tunnelIvifMgr = (xpTunnelIvifMgr *)xpTunnelPl::instance()->getMgr(xpTunnelIvifMgr::getId());
    if(tunnelIvifMgr == NULL)
    {
        printf("tunnelIvifMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(tunnelIvifMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: TunnelIvifMgr init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }
    printf("Created and Initialized TnlIvif PL Mgr successfully\n");

   //create ipv4 PL manager
    status = xpL3Pl::instance()->createMgr(xpIpRouteLpmMgr::getId(xpIpRouteLpmMgr::IPV4_ROUTE_MGR));
    if (status)
    {
        return status;
    }
    //get reference to ipv4-route PL manager
    ipv4RouteMgr = (xpIpRouteLpmMgr*) xpL3Pl::instance()->getMgr(xpIpRouteLpmMgr::getId(xpIpRouteLpmMgr::IPV4_ROUTE_MGR));
    if (!ipv4RouteMgr)
    {
        return XP_ERR_INIT;
    }
    //initialize PL manager
    status = ipv4RouteMgr->init(XP_PREFIX_TYPE_IPV4, 512*1024, 16*1024);
    if (status)
    {
        printf("ERROR: ipv4Route manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }
    printf("Created and Initialized Ipv4Route PL Mgr successfully\n");

    //
    // Create ipv4Host manager
    //
    if(xpL3Pl::instance()->createMgr(xpIpv4HostMgr::getId()) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for ipv4Host manager.\n Exiting...");
        return XP_ERR_INIT;
    }
    //
    // PL Init for IPv4-Host Manager    
    //
    ipv4HostMgr = static_cast<xpIpv4HostMgr *>(xpL3Pl::instance()->getMgr(xpIpv4HostMgr::getId()));
    if(ipv4HostMgr == NULL)
    {
        printf("ipv4HostMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(ipv4HostMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: ipv4Host manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }
    printf("Created and Initialized Ipv4Host PL Mgr successfully\n");

    //
    // create ipv6Route PL manager
    //
    status = xpL3Pl::instance()->createMgr(xpIpRouteLpmMgr::getId(xpIpRouteLpmMgr::IPV6_ROUTE_MGR));
    if (status)
    {
        return status;
    }
    //get reference to ipv6 PL manager
    ipv6RouteMgr = (xpIpRouteLpmMgr*) xpL3Pl::instance()->getMgr(xpIpRouteLpmMgr::getId(xpIpRouteLpmMgr::IPV6_ROUTE_MGR));
    if (!ipv6RouteMgr)
    {
        return XP_ERR_INIT;
    }
    //initialize PL manager
    status = ipv6RouteMgr->init(XP_PREFIX_TYPE_IPV6, 64*1024, 16*1024);
    if (status)
    {
        printf("ERROR: ipv6Route manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }
    printf("Created and Initialized Ipv6Route PL Mgr successfully\n");

    //
    // Create Ipv6Host PL Mgr 
    //
    if(xpL3Pl::instance()->createMgr(xpIpv6HostMgr::getId()) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for ipv6Host manager.\n Exiting...");
        return XP_ERR_INIT;
    }
    //
    // PL Init for Ipv6-Host Manager
    //
    ipv6HostMgr = static_cast<xpIpv6HostMgr *>(xpL3Pl::instance()->getMgr(xpIpv6HostMgr::getId()));
    if(ipv6HostMgr == NULL)
    {
        printf("ipv6HostMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(ipv6HostMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: ipv6Host manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }
    printf("Created and Initialized Ipv6Host PL Mgr successfully\n");

    //
    // Create IACL PL Mgr 
    //
    if((status = xpAclPl::instance()->createMgr(xpIaclMgr::getId())) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for IACL manager.\n Exiting...");
        return status;
    }
    //
    // PL Init for IACL Manager
    //
    iaclMgr = static_cast<xpIaclMgr *>(xpAclPl::instance()->getMgr(xpIaclMgr::getId()));
    if(iaclMgr == NULL)
    {
        printf("iaclMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(iaclMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: iaclMgr manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }
    printf("Created and Initialized IACL PL Mgr successfully\n");

    // Creation and initialization of Ipv4 Route Mc
    if((status = xpMcastPl::instance()->createMgr(xpIpv4RouteMcMgr::getId())) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for Ipv4RouteMc manager.\n Exiting...");
        return status;
    }
    xpIpv4RouteMcMgr *ipv4RouteMcMgr = static_cast<xpIpv4RouteMcMgr *>(xpMcastPl::instance()->getMgr(xpIpv4RouteMcMgr::getId()));
    if(ipv4RouteMcMgr == NULL)
    {
        printf("ipv4RouteMcMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(ipv4RouteMcMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: ipv4RouteMc manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }
    printf("Created and Initialized Ipv4RouteMc PL Mgr successfully\n");
    
    // Creation and initialization of Ipv6 Route Mc
    xpMcastPl::instance()->createMgr(xpIpv6RouteMcMgr::getId());
    xpIpv6RouteMcMgr *ipv6RouteMcMgr = static_cast<xpIpv6RouteMcMgr *>(xpMcastPl::instance()->getMgr(xpIpv6RouteMcMgr::getId()));
    if(ipv6RouteMcMgr == NULL)
    {
        printf("ipv6RouteMcMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(ipv6RouteMcMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: ipv6RouteMc manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }
    printf("Created and Initialized Ipv6RouteMc PL Mgr successfully\n");

    // Creation and initialization of Ipv4 Bridge Mc
    if((status = xpMcastPl::instance()->createMgr(xpIpv4BridgeMcMgr::getId())) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for Ipv4BridgeMc manager.\n Exiting...");
        return status;
    }
    xpIpv4BridgeMcMgr *ipv4BridgeMcMgr = static_cast<xpIpv4BridgeMcMgr *>(xpMcastPl::instance()->getMgr(xpIpv4BridgeMcMgr::getId()));
    if(ipv4BridgeMcMgr == NULL)
    {
        printf("ipv4BridgeMcMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(ipv4BridgeMcMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: ipv4BridgeMc manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }
    printf("Created and Initialized Ipv4BridgeMc PL Mgr successfully\n");
    
    // Creation and initialization of Ipv6 Bridge Mc
    xpMcastPl::instance()->createMgr(xpIpv6BridgeMcMgr::getId());
    xpIpv6BridgeMcMgr *ipv6BridgeMcMgr = static_cast<xpIpv6BridgeMcMgr *>(xpMcastPl::instance()->getMgr(xpIpv6BridgeMcMgr::getId()));
    if(ipv6BridgeMcMgr == NULL)
    {
        printf("ipv6BridgeMcMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(ipv6BridgeMcMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: ipv6BridgeMc manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }
    printf("Created and Initialized Ipv6BridgeMc PL Mgr successfully\n");

    // Creation and initialization of Ipv4 PIM RPF
    if((status = xpMcastPl::instance()->createMgr(xpIpv4PIMBiDirRPFMgr::getId())) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for ipv4Pim manager.\n Exiting...");
        return status;
    }
    xpIpv4PIMBiDirRPFMgr *ipv4PimMgr = static_cast<xpIpv4PIMBiDirRPFMgr *>(xpMcastPl::instance()->getMgr(xpIpv4PIMBiDirRPFMgr::getId()));
    if(ipv4PimMgr == NULL)
    {
        printf("ipv4PimMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(ipv4PimMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: ipv4Pim manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }
    printf("Created and Initialized Ipv4Pim PL Mgr successfully\n");
    
    // Creation and initialization of Ipv6 Pim RPF
    xpMcastPl::instance()->createMgr(xpIpv6PIMBiDirRPFMgr::getId());
    xpIpv6PIMBiDirRPFMgr *ipv6PimMgr = static_cast<xpIpv6PIMBiDirRPFMgr *>(xpMcastPl::instance()->getMgr(xpIpv6PIMBiDirRPFMgr::getId()));
    if(ipv6PimMgr == NULL)
    {
        printf("ipv6PimMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(ipv6PimMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: ipv6Pim manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }
    printf("Created and Initialized Ipv6Pim PL Mgr successfully\n");

    // Creation and initialization of NAT Ipv4
    if((status = xpL3Pl::instance()->createMgr(xpNATIpv4Mgr::getId())) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for natIpv4Mgr manager.\n Exiting...");
        return status;
    }
    xpNATIpv4Mgr *natIpv4Mgr = static_cast<xpNATIpv4Mgr *>(xpL3Pl::instance()->getMgr(xpNATIpv4Mgr::getId()));
    if(natIpv4Mgr == NULL)
    {
        printf("natIpv4Mgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(natIpv4Mgr->init() != XP_NO_ERR)
    {
        printf("ERROR: natIpv4Mgr manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }
    printf("Created and Initialized natIpv4Mgr PL Mgr successfully\n");
    
    // Creation and initialization of NAT Ipv6
    xpL3Pl::instance()->createMgr(xpNATCompIpv6Mgr::getId());
    xpNATCompIpv6Mgr *natIpv6Mgr = static_cast<xpNATCompIpv6Mgr *>(xpL3Pl::instance()->getMgr(xpNATCompIpv6Mgr::getId()));
    if(natIpv6Mgr == NULL)
    {
        printf("natIpv6Mgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(natIpv6Mgr->init() != XP_NO_ERR)
    {
        printf("ERROR: natIpv6Mgr manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }
    printf("Created and Initialized natIpv6Mgr PL Mgr successfully\n");
    
    if(xpIfPl::instance()->createMgr(xpReasonCodeMgr::getId()) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for rctMgr manager.\n Exiting...");
        return XP_ERR_INIT;
    }
    xpReasonCodeMgr* rctMgr = static_cast<xpReasonCodeMgr *>(xpIfPl::instance()->getMgr(xpReasonCodeMgr::getId()));
    if(rctMgr == NULL)
    {
        printf("rctMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(rctMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: rctMgr manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }
    printf("Created and Initialized rctMgr PL Mgr successfully\n");
    
    if(xpQosPl::instance()->createMgr(xpQosMapMgr::getId()) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for rctMgr manager.\n Exiting...");
        return XP_ERR_INIT;
    }
    xpQosMapMgr* qosMgr = static_cast<xpQosMapMgr *>(xpQosPl::instance()->getMgr(xpQosMapMgr::getId()));
    if(qosMgr == NULL)
    {
        printf("qosMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(qosMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: qosMgr manager init failed. Exiting...\n");
        initFail = true;
        //return XP_ERR_INIT;
    }
    printf("Created and Initialized qosMgr PL Mgr successfully\n");

    if(xpQosPl::instance()->createMgr(XP_PIPE_SCHED_SHAPER_MGR) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for pipeSchedShaperCfgMgr manager.\n Exiting...");
        return XP_ERR_INIT;
    }
    
    pipeSchedShaperCfgMgr = static_cast<xpPipeSchedShaperCfgMgr *>(xpQosPl::instance()->getMgr(XP_PIPE_SCHED_SHAPER_MGR));
    if(pipeSchedShaperCfgMgr == NULL)
    {
        printf("pipeSchedShaperCfgMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }

    if(pipeSchedShaperCfgMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: pipeSchedShaperCfgMgr manager init failed. Exiting...\n");
        initFail = true;
    }
    printf("Created and Initialized pipeSchedShaperCfgMgr PL Mgr successfully\n");
    
    if(xpQosPl::instance()->createMgr(xpQmappingMgr::getId()) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for rctMgr manager.\n Exiting...");
        return XP_ERR_INIT;
    }
    
    qmappingMgr = static_cast<xpQmappingMgr *>(xpQosPl::instance()->getMgr(xpQmappingMgr::getId()));
    if(qmappingMgr == NULL)
    {
        printf("qmappingMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }

    if(qmappingMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: qmappingMgr manager init failed. Exiting...\n");
        initFail = true;
    }
    printf("Created and Initialized qmappingMgr PL Mgr successfully\n");

    if(xpQosPl::instance()->createMgr(XP_PORT_MAPPING_CFG_MGR) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for rctMgr manager.\n Exiting...");
        return XP_ERR_INIT;
    }
    
    portMappingCfgMgr = static_cast<xpPortMappingCfgMgr *>(xpQosPl::instance()->getMgr(XP_PORT_MAPPING_CFG_MGR));
    if(portMappingCfgMgr == NULL)
    {
        printf("portMappingCfgMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(portMappingCfgMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: portMappingCfgMgr manager init failed. Exiting...\n");
        initFail = true;
    }
    printf("Created and Initialized portMappingCfgMgr PL Mgr successfully\n");

    if(xpQosPl::instance()->createMgr(XP_PORT_SCHED_SHAPER_MGR) != XP_NO_ERR)
    {
        printf("ERROR in createMgr for portSchedShaperCfgMgr manager.\n Exiting...");
        return XP_ERR_INIT;
    }
    
    portSchedShaperCfgMgr = static_cast<xpPortSchedShaperCfgMgr *>(xpQosPl::instance()->getMgr(XP_PORT_SCHED_SHAPER_MGR));
    if(portSchedShaperCfgMgr == NULL)
    {
        printf("portSchedShaperCfgMgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(portSchedShaperCfgMgr->init() != XP_NO_ERR)
    {
        printf("ERROR: portSchedShaperCfgMgr manager init failed. Exiting...\n");
        initFail = true;
    }
    printf("Created and Initialized portSchedShaperCfgMgr PL Mgr successfully\n");
    
    natipv4Mgr = static_cast<xpNATIpv4Mgr*>(xpL3Pl::instance()->getMgr(xpNATIpv4Mgr::getId()));

    if(natipv4Mgr == NULL)
    {
        printf("natipv4Mgr is NULL. Exiting...\n");
        return XP_ERR_INIT;
    }
    if(natipv4Mgr->init() != XP_NO_ERR)
    {
        printf("Error: natipv4 manager init failed. Exiting...\n");
        return XP_ERR_INIT;
    }
    printf("Created and Initialized natIpv4Mgr PL Mgr successfully\n");

    

    if(initFail)
        return XP_ERR_INIT;
    else
        return status;
}

XP_STATUS xpAppPLAddDevice(xpDevice_t devId, XP_PIPE_MODE mode, XP_TABLE_MODE_T tblMode)
{
    XP_STATUS status = XP_NO_ERR;
    uint8_t tblCopyNum;
   
    // Initialize all primitive managers here which creates table in HW

    //
    // Dev Init for VIF Mgr 
    //
    xpVifMgr *vifMgr = static_cast<xpVifMgr *>(xpIfPl::instance()->getMgr(xpVifMgr::getId()));

    tblCopyNum = tableProfile->getXpDaTblCopyNum(IVIF32_TBL);
    xpDirectTableContext* vifTblCtx[5][tblCopyNum];

    std::vector<xpTblCtxPerIdx *> tblCtxPerIdx;
    for(uint32_t i = 0; i < 5; i++)
    {
        tblCtxPerIdx.push_back(new xpTblCtxPerIdx());
    }

    // 32 bit ivif
    tblCtxPerIdx[0]->index = 1;
    // 64 bit evif
    tblCtxPerIdx[1]->index = 2;
    // 128 bit evif
    tblCtxPerIdx[2]->index = 4;
    // 256 bit evif
    tblCtxPerIdx[3]->index = 6;
    // 256 bit ivif
    tblCtxPerIdx[4]->index = 7;
    
    for(uint8_t type = 0; type < 5; type++)
    {
        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            vifTblCtx[type][i] = tableProfile->getXpDaCfg((xpPlDaDynamicTblList)(EVIF32_TBL + tblCtxPerIdx[type]->index), i);
        }
        tblCtxPerIdx[type]->tblCtx = (xpTableContext**) &vifTblCtx[type];
    }

    if(vifMgr->addDevice(devId, tblCtxPerIdx, tblCopyNum, tblMode) != XP_NO_ERR)
    {
        printf("ERROR: Cannot addDevice for vif manager.\n");
        return XP_ERR_INIT;
    }
    printf("VIF manager initialized successfully!!!\n");

    for(uint32_t i = 0; i < 5; i++)
    {
        delete tblCtxPerIdx[i];
    }
    tblCtxPerIdx.clear();

    printf("<dev = %d >VIF manager initialized successfully!!!\n",devId);

    //
    // Dev Init for the HDR MOD Manager
    //

    xpHdrModificationMgr *hdrModMgr = static_cast<xpHdrModificationMgr *>(xpIfPl::instance()->getMgr(xpHdrModificationMgr::getId()));

    if(hdrModMgr == NULL)
    {
        printf("<dev = %d>hdrModMgr is NULL. Exiting...\n",devId);
        return XP_ERR_INIT;
    }

    for(uint32_t i = 0; i < 3; i++)
    {
        tblCtxPerIdx.push_back(new xpTblCtxPerIdx());
    }

    // ASSUMPTION: tblCopyNum is same for all Insertion types
    tblCopyNum = tableProfile->getXpDaTblCopyNum(INS0_256_TBL);
    xpDirectTableContext* insTblCtx[3][tblCopyNum];

    for(uint8_t type = 0; type < 3; type++)
    {
        tblCtxPerIdx[type]->index = type;
        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            insTblCtx[type][i] = tableProfile->getXpDaCfg((xpPlDaDynamicTblList)(INS0_256_TBL + tblCtxPerIdx[type]->index), i);
        }
        tblCtxPerIdx[type]->tblCtx = (xpTableContext**) &insTblCtx[type];
    }

    if(hdrModMgr->addDevice(devId, tblCtxPerIdx, tblCopyNum, tblMode) != XP_NO_ERR)
    {
        printf("ERROR in hdrModMgr->addDevice()\n");
        return XP_ERR_INIT;
    }

    for(uint32_t i = 0; i < 3; i++)
    {
        delete tblCtxPerIdx[i];
    }

    tblCtxPerIdx.clear();

    printf("<dev = %d> Header Mod manager initialized successfully!!!\n",devId);

    //
    // Dev Init for the TRT  Manager
    //
    trunkResolutionMgr = (xpTrunkResolutionMgr *)xpIfPl::instance()->getMgr(xpTrunkResolutionMgr::getId());

    if(trunkResolutionMgr == NULL)
    {
        printf("<dev = %d> trunkResolutionMgr is NULL. Exiting...\n",devId);
        return XP_ERR_INIT;
    }

    /* Add Device */
    if((status = trunkResolutionMgr->addDevice(devId)) != XP_NO_ERR)
    {
        printf("ERROR <dev = %d>: Adding Device for trunkResolutionMgr failed. Exiting...\n",devId);
        return status;
    }
    printf("<dev = %d> trunkResolutionMgr initialized successfully!!!\n",devId);

    //
    // Dev Init for  egress filter table manager
    //
    egFltMgr = static_cast<xpEgressFilterMgr *>(xpIfPl::instance()->getMgr(xpEgressFilterMgr::getId()));

    if(egFltMgr == NULL)
    {
        printf("<dev = %d>egFltMgr is NULL. Exiting...\n",devId);
        return XP_ERR_INIT;
    }
    if(egFltMgr->addDevice(devId) != XP_NO_ERR)
    {
        printf("ERROR<dev = %d>: Adding Device for egFltMgr manager failed. Exiting...\n",devId);
        return XP_ERR_INIT;
    }
    printf("<dev = %d> Egress Filter Manager initialized successfully\n",devId);

    //
    // Dev Init for the BD  Manager
    //
    bdMgr = (xpBdMgr *)xpL2Pl::instance()->getMgr(xpBdMgr::getId());

    if(bdMgr == NULL)
    {
        printf("ERROR <dev = %d> :bdMgr is NULL. Exiting...\n",devId);
        return XP_ERR_INIT;
    }
    tblCopyNum = tableProfile->getXpIsmeTblCopyNum(BD_TBL);
    xpIsmeTableContext* bdTblCtx[tblCopyNum];
    for(uint8_t i = 0; i < tblCopyNum; i++)
    {
        bdTblCtx[i] = tableProfile->getXpIsmeCfg(BD_TBL, i);
    }

    /* Add Device */
    if(bdMgr->addDevice(devId, (xpTableContext**)&bdTblCtx, tblCopyNum, TABLE_INIT_WITH_DYNAMIC_INFO) != XP_NO_ERR) //create BD table in HW
    {
        printf("ERROR <dev = %d> : Adding Device for BD table failed. Exiting...\n",devId);
        return XP_ERR_INIT;
    }
    printf("<dev = %d> BD manager initialized successfully!!!\n",devId);
    //
    // Dev Init for the MPLS  Manager
    //
    mplsTunnelMgr = (xpMplsTunnelMgr *)xpTunnelPl::instance()->getMgr(xpMplsTunnelMgr::getId());

    if(mplsTunnelMgr == NULL)
    {
        printf("ERROR <dev = %d> :mplsTunnelMgr is NULL. Exiting...\n",devId);
        return XP_ERR_INIT;
    }
    tblCopyNum = tableProfile->getXpIsmeTblCopyNum(MPLS_TNL_TBL);
    xpIsmeTableContext* mplsTunnelTblCtx[tblCopyNum];
    for(uint8_t i = 0; i < tblCopyNum; i++)
    {
        mplsTunnelTblCtx[i] = tableProfile->getXpIsmeCfg(MPLS_TNL_TBL, i);
    }

    /* Add Device */
    if(mplsTunnelMgr->addDevice(devId, (xpTableContext**)&mplsTunnelTblCtx, tblCopyNum, TABLE_INIT_WITH_DYNAMIC_INFO) != XP_NO_ERR) //create MPLS tunnel table in HW
    {
        printf("ERROR <dev = %d> : Adding Device for MPLS tunnel table failed. Exiting...\n",devId);
        return XP_ERR_INIT;
    }
    printf("<dev = %d> MPLS tunnel manager initialized successfully!!!\n",devId);

#if 0
    acmBnkMgr = (xpAcmBankMgr *)xpAcmPl::instance()->getMgr(xpAcmBankMgr::getId());
    acmRstMgr = (xpAcmRsltMgr *)xpAcmPl::instance()->getMgr(xpAcmRsltMgr::getId());

    if ((acmBnkMgr == NULL) || (acmRstMgr == NULL) || (acmBnkMgr == acmRstMgr))
    {
        printf("ERROR <dev = %d> :acmBnkkMgr or acmRstMgr is NULL. Exiting...\n",devId);
        return XP_ERR_INIT;
    }

    /* add device to ACM bank manager */
    if (acmBnkMgr->addDevice(devId) != XP_NO_ERR)
    {
        printf("ERROR <dev = %d> : Adding Device for ACM Bank table failed. Exiting...\n",devId);
        return XP_ERR_INIT;
    }

    /* add device to ACM bank manager */
    if (acmRstMgr->addDevice(devId) != XP_NO_ERR)
    {
        printf("ERROR <dev = %d> : Adding Device for ACM Rslt table failed. Exiting...\n",devId);
        return XP_ERR_INIT;
    }
    printf("<dev = %d> ACM Bank and Rslt manager initialized successfully!!!\n",devId);
#endif
    //
    // Dev Init for the MDT  Manager
    //
    mdtMgr = (xpMdtMgr *)xpIfPl::instance()->getMgr(xpMdtMgr::getId());

    if(mdtMgr == NULL)
    {
        printf("ERROR <dev = %d > :xpMdtMgr is NULL. Exiting...\n", devId);
        return XP_ERR_INIT;
    }
    tblCopyNum = tableProfile->getXpDaTblCopyNum(MDT_TBL);
    xpDirectTableContext* mdtTblCtx[tblCopyNum];
    for(uint8_t i = 0; i < tblCopyNum; i++)
    {
        mdtTblCtx[i] = tableProfile->getXpDaCfg(MDT_TBL, i);
    }

    if((status = mdtMgr->addDevice(devId, (xpTableContext**)&mdtTblCtx, tblCopyNum, tblMode)) != XP_NO_ERR)
    {
        printf("ERROR <dev = %d>: Adding Device for mdt failed. Exiting...\n", devId);
        return status;
    }
    printf("<dev = %d> MDT manager initialized successfully!!!\n",devId);

    //
    // Dev Init for the PORT_VLAN  Manager
    //

    portVlanMgr = (xpPortVlanIvifMgr *)xpL2Pl::instance()->getMgr(xpPortVlanIvifMgr::getId());

    if(portVlanMgr == NULL)
    {
        printf(" ERROR <dev = %d> : portVlanMgr is NULL. Exiting...\n",devId);
        return XP_ERR_INIT;
    }

    tblCopyNum = tableProfile->getXpHashTblCopyNum(PORTVLANIVIF_TBL);
    xpHashTableContext* portVlanTblCtx[tblCopyNum];
    for(uint8_t i = 0; i < tblCopyNum; i++)
    {
        portVlanTblCtx[i] = tableProfile->getXpHashCfg(PORTVLANIVIF_TBL, i);
    }
    /* Add Device */
    if((status = portVlanMgr->addDevice(devId, (xpTableContext**)&portVlanTblCtx, tblCopyNum, tblMode)) != XP_NO_ERR)
    {
        printf("ERROR <dev = %d>: Adding Device for portVlanMgr failed. Exiting...\n",devId);
        return status;
    }
    printf("<dev = %d> portVlanMgr initialized successfully!!!\n",devId);

    //
    // Dev Init for the TUNNEL  Manager
    //
    tunnelIvifMgr = (xpTunnelIvifMgr *)xpTunnelPl::instance()->getMgr(xpTunnelIvifMgr::getId());

    if(tunnelIvifMgr == NULL)
    {
        printf("Erro <dev = %d> : tunnelIvifMgr is NULL. Exiting...\n", devId);
        return XP_ERR_INIT;
    }

    tblCopyNum = tableProfile->getXpHashTblCopyNum(TNL_IVIF_TBL);
    xpHashTableContext* tnlTblCtx[tblCopyNum];
    for(uint8_t i = 0; i < tblCopyNum; i++)
    {
        tnlTblCtx[i] = tableProfile->getXpHashCfg(TNL_IVIF_TBL, i);
    }

    if(tunnelIvifMgr->addDevice(devId, (xpTableContext**)&tnlTblCtx, tblCopyNum, tblMode) != XP_NO_ERR)
    {
        printf("ERROR: Adding Device for Tunnel failed. Exiting...\n");
        return XP_ERR_INIT;
    }

    printf("<dev = %d> TunnelIvifMgr initialized successfully!!!\n",devId);

    //
    // Dev Init for the FDB  Manager
    //
    fdbMgr = (xpFdbMgr *)xpL2Pl::instance()->getMgr(xpFdbMgr::getId());

    if(fdbMgr == NULL)
    {
        printf("ERROR <dev = %d> :fdbMgr is NULL. Exiting...\n",devId);
        return XP_ERR_INIT;
    }

    /* Add Device */
    tblCopyNum = tableProfile->getXpHashTblCopyNum(FDB_TBL);
    xpHashTableContext* fdbTblCtx[tblCopyNum];

    for(uint8_t i = 0; i < tblCopyNum; i++)
    {
        fdbTblCtx[i] = tableProfile->getXpHashCfg(FDB_TBL, i);
    }

    /* Add Device */
    if((status = fdbMgr->addDevice(devId, (xpTableContext**)&fdbTblCtx, tblCopyNum, tblMode)) != XP_NO_ERR)
    {
        printf("ERROR <dev=%d>: Adding Device for Fdb failed. Exiting...\n",devId);
        return status;
    }
    printf("<dev = %d> xpFdbMgr initialized successfully!!!\n",devId);

    tblCopyNum = tableProfile->getXpLpmTblCopyNum();
    xpLpmTableContext* lpmV4Ctx[tblCopyNum];
    xpLpmTableContext* lpmV6Ctx[tblCopyNum];

    for(uint8_t i = 0; i < tblCopyNum; i++)
    {
        lpmV4Ctx[i] = tableProfile->getXpLpmV4Cfg(i);
        lpmV6Ctx[i] = tableProfile->getXpLpmV6Cfg(i);
    }

    //
    //PL : Add device for Ipv4 Route Manager
    //
    if (!ipv4RouteMgr)
    {
        return XP_ERR_INIT;
    }

    status = ipv4RouteMgr->addDevice(devId, (xpTableContext**)&lpmV4Ctx, tblCopyNum);
    if (status)
    {
        return status;
    }

    //
    // PL : Add device for IPv4 host Manager
    //
    if (!ipv4HostMgr)
    {
        return XP_ERR_INIT;
    }

    tblCopyNum = tableProfile->getXpHashTblCopyNum(IPV4HOST_TBL);
    xpHashTableContext* ipv4TblCtx[tblCopyNum];

    for(uint8_t i = 0; i < tblCopyNum; i++)
    {   
        ipv4TblCtx[i] = tableProfile->getXpHashCfg(IPV4HOST_TBL, i);
    }   
    if(ipv4HostMgr->addDevice(devId, (xpTableContext**)&ipv4TblCtx, tblCopyNum, tblMode) != XP_NO_ERR)
    {   
        printf("ERROR: Adding Device for ipv4Host manager failed. Exiting...\n");
        return XP_ERR_INIT;
    }   
        
    printf("Ipv4Host manager initialized successfully!!!\n");

    //
    //PL : Add device for Ipv6 Route Manager
    //
    if (!ipv6RouteMgr)
    {
        return XP_ERR_INIT;
    }

    //PL : Add device for Ipv6 Route Manager
    status = ipv6RouteMgr->addDevice(devId, (xpTableContext**)&lpmV6Ctx, tblCopyNum);
    if (status)
    {
        return status;
    }

    if (!ipv6HostMgr)
    {
        return XP_ERR_INIT;
    }

    tblCopyNum = tableProfile->getXpHashTblCopyNum(IPV6HOST_TBL);
    xpHashTableContext* ipv6TblCtx[tblCopyNum];

    for(uint8_t i = 0; i < tblCopyNum; i++)
    {   
        ipv6TblCtx[i] = tableProfile->getXpHashCfg(IPV6HOST_TBL, i);
    }   

    if(ipv6HostMgr->addDevice(devId, (xpTableContext**)&ipv6TblCtx, tblCopyNum, tblMode) != XP_NO_ERR)
    {   
        printf("ERROR: Adding Device for ipv6Host manager failed. Exiting...\n");
        return XP_ERR_INIT;
    }   
        
    printf("Ipv6Host manager initialized successfully!!!\n");

    //
    // Dev Init for Port Config manager
    //

    portCfgMgr = static_cast<xpPortConfigMgr *>(xpIfPl::instance()->getMgr(xpPortConfigMgr::getId()));

    if(portCfgMgr == NULL)
    {
        printf("ERROR <dev = %d> :portCfgMgr is NULL. Exiting...\n",devId);
        return XP_ERR_INIT;
    }

    if((status = portCfgMgr->addDevice(devId)) != XP_NO_ERR)
    {
        printf("ERROR <dev= %d>: Adding Device for portConfig manager failed. Exiting...\n",devId);
        return status;
    }

    printf("<dev = %d> Port Config manager initialized successfully!!!\n",devId);
    
    // Creation and initialization of controlMac PM
    controlMacMgr = static_cast<xpControlMacMgr *>(xpIfPl::instance()->getMgr(xpControlMacMgr::getId()));

    if(controlMacMgr->addDevice(devId) != XP_NO_ERR)
    {
        printf("ERROR <dev=%d> : controlMacMgr->addDevice failed. Exiting...\n", devId);
        return XP_ERR_INIT;
    }

    printf("<dev = %d> controlMac Manager initialized successfully\n",devId);
    
   // Creation and initialization of IACL manager
    iaclMgr = static_cast<xpIaclMgr *>(xpAclPl::instance()->getMgr(xpIaclMgr::getId()));

    /* Add Device*/
    for(uint32_t i = 0; i < 3; i++)
    {
        tblCtxPerIdx.push_back(new xpTblCtxPerIdx());
    }

    tblCopyNum = tableProfile->getXpTcamTblCopyNum(IACL0_TBL);
    xpMatchTableContext* iaclTblCtx[3][tblCopyNum];

    tblCtxPerIdx[0]->index = XP_IACL0;
    tblCtxPerIdx[1]->index = XP_IACL1;
    tblCtxPerIdx[2]->index = XP_IACL2;

    for(uint8_t type = 0; type < 3; type++)
    {
        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            iaclTblCtx[type][i] = tableProfile->getXpTcamCfg((xpPlTcamDynamicTblList)(IACL0_TBL + tblCtxPerIdx[type]->index), i);
        }
        tblCtxPerIdx[type]->tblCtx = (xpTableContext**) &iaclTblCtx[type];
    }

    status = iaclMgr->addDevice(devId, tblCtxPerIdx, tblCopyNum, TABLE_INIT_WITH_DYNAMIC_INFO);
    if (status)
    {
        printf("ERROR <dev=%d> <ERROR = %d> : iaclMgr->addDevice failed. Exiting...\n", devId, status);
        return XP_ERR_INIT;
    }
    
    printf("<dev = %d> iaclMac Manager initialized successfully\n",devId);
    for(uint32_t i = 0; i < 3; i++)
    {
        delete tblCtxPerIdx[i];
    }

    tblCtxPerIdx.clear();

    // Creation and initialization for ipv4 Route Mc
    tblCopyNum = tableProfile->getXpHashTblCopyNum(IPV4ROUTEMC_TBL);
    xpIpv4RouteMcMgr *ipv4RouteMcMgr = static_cast<xpIpv4RouteMcMgr *>(xpMcastPl::instance()->getMgr(xpIpv4RouteMcMgr::getId()));

    /* Add Device */
    xpHashTableContext* ipv4RouteMcTblCtx[tblCopyNum];
    for(uint8_t i = 0; i < tblCopyNum; i++)
    {
        ipv4RouteMcTblCtx[i] = tableProfile->getXpHashCfg(IPV4ROUTEMC_TBL, i);
    }
    if(ipv4RouteMcMgr->addDevice(devId, (xpTableContext**)&ipv4RouteMcTblCtx, tblCopyNum, tblMode) != XP_NO_ERR)
    {
        printf("ERROR <dev=%d> : ipv4RouteMcMgr->addDevice failed. Exiting...\n", devId);
        return XP_ERR_INIT;
    }
    printf("<dev = %d> ipv4RouteMc Manager initialized successfully\n",devId);   

    // Creation and initialization for ipv6 route mc
    tblCopyNum = tableProfile->getXpHashTblCopyNum(IPV6ROUTEMC_TBL);
    xpIpv6RouteMcMgr *ipv6RouteMcMgr = static_cast<xpIpv6RouteMcMgr *>(xpMcastPl::instance()->getMgr(xpIpv6RouteMcMgr::getId()));

    /* Add Device */
    xpHashTableContext* ipv6RouteMcTblCtx[tblCopyNum];
    for(uint8_t i = 0; i < tblCopyNum; i++)
    {
        ipv6RouteMcTblCtx[i] = tableProfile->getXpHashCfg(IPV6ROUTEMC_TBL, i);
    }

    if(ipv6RouteMcMgr->addDevice(devId, (xpTableContext**)&ipv6RouteMcTblCtx, tblCopyNum, tblMode) != XP_NO_ERR)
    {
        printf("ERROR <dev=%d> : ipv6RouteMcMgr->addDevice failed. Exiting...\n", devId);
        return XP_ERR_INIT;
    }
    printf("<dev = %d> ipv6RouteMc Manager initialized successfully\n",devId);

    if(pipeSchedShaperCfgMgr->addDevice(devId) != XP_NO_ERR)
    {
        printf("ERROR <dev=%d> : pipeSchedShaperCfgMgr->addDevice failed. Exiting...\n", devId);
        return XP_ERR_INIT;
    }
    printf("<dev = %d> pipeSchedShaperCfgMgr Manager initialized successfully\n",devId);
    
    if(qmappingMgr->addDevice(devId) != XP_NO_ERR)
    {
        printf("ERROR <dev=%d> : qmappingMgr->addDevice failed. Exiting...\n", devId);
        return XP_ERR_INIT;
    }
    printf("<dev = %d> qmappingMgr Manager initialized successfully\n",devId);

    if(portMappingCfgMgr->addDevice(devId) != XP_NO_ERR)
    {
        printf("ERROR <dev=%d> : portMappingCfgMgr->addDevice failed. Exiting...\n", devId);
        return XP_ERR_INIT;
    }
    printf("<dev = %d> portMappingCfgMgr Manager initialized successfully\n",devId);

    if(portSchedShaperCfgMgr->addDevice(devId) != XP_NO_ERR)
    {
        printf("ERROR <dev=%d> : portSchedShaperCfgMgr->addDevice failed. Exiting...\n", devId);
        return XP_ERR_INIT;
    }
    printf("<dev = %d> portSchedShaperCfgMgr Manager initialized successfully\n",devId);

    // Creation and initialization for ipv4 Bridge Mc
    tblCopyNum = tableProfile->getXpTcamTblCopyNum(NATIPV4_TBL);
    xpNATIpv4Mgr *natIpv4Mgr = static_cast<xpNATIpv4Mgr *>(xpL3Pl::instance()->getMgr(xpNATIpv4Mgr::getId()));

    /* Add Device */
    xpMatchTableContext* natIpv4TblCtx[tblCopyNum];
    for(uint8_t i = 0; i < tblCopyNum; i++)
    {
        natIpv4TblCtx[i] = tableProfile->getXpTcamCfg(NATIPV4_TBL, i);
    }
    if(natIpv4Mgr->addDevice(devId, (xpTableContext**)&natIpv4TblCtx, tblCopyNum, tblMode) != XP_NO_ERR)
    {
        printf("ERROR <dev=%d> : natIpv4Mgr->addDevice failed. Exiting...\n", devId);
        return XP_ERR_INIT;
    }

    return XP_NO_ERR; //Success
}

/* Used to test addDevice of all PMs with different tblMode */
XP_STATUS xpAppPLAddDevice(xpDevice_t devId, XP_PIPE_MODE mode, XP_TABLE_MODE_T tblMode[])
{
    XP_STATUS status = XP_NO_ERR;
    uint8_t tblCopyNum;
    std::vector<xpTblCtxPerIdx *> tblCtxPerIdx;
    bool addFail = false;

    // Initialize all primitive managers here which creates table in HW

    if(tblMode[xpVifMgr::getId()] != TABLE_INVALID_INFO)
    {
        //
        // Dev Init for VIF Mgr 
        //
        xpVifMgr *vifMgr = static_cast<xpVifMgr *>(xpIfPl::instance()->getMgr(xpVifMgr::getId()));
        tblCopyNum = tableProfile->getXpDaTblCopyNum(IVIF32_TBL);
        xpDirectTableContext* vifTblCtx[5][tblCopyNum];
        
        for(uint32_t i = 0; i < 5; i++)
        {
            tblCtxPerIdx.push_back(new xpTblCtxPerIdx());
        }

        // 32 bit ivif
        tblCtxPerIdx[0]->index = 1;
        // 64 bit evif
        tblCtxPerIdx[1]->index = 2;
        // 128 bit evif
        tblCtxPerIdx[2]->index = 4;
        // 256 bit evif
        tblCtxPerIdx[3]->index = 6;
        // 256 bit ivif
        tblCtxPerIdx[4]->index = 7;
        for(uint8_t type = 0; type < 5; type++)
        {
            for(uint8_t i = 0; i < tblCopyNum; i++)
            {
                vifTblCtx[type][i] = tableProfile->getXpDaCfg((xpPlDaDynamicTblList)(EVIF32_TBL + tblCtxPerIdx[type]->index), i);
            }
            tblCtxPerIdx[type]->tblCtx = (xpTableContext**) &vifTblCtx[type];
        }

        if(vifMgr->addDevice(devId, tblCtxPerIdx, tblCopyNum, tblMode[xpVifMgr::getId()]) != XP_NO_ERR)
        {
            printf("ERROR: Cannot addDevice for vif manager.\n");
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d >VIF manager initialized successfully!!!\n",devId);
        }

        for(uint32_t i = 0; i < 5; i++)
        {
            delete tblCtxPerIdx[i];
        }
        tblCtxPerIdx.clear();
    }
    
    if(tblMode[xpHdrModificationMgr::getId()] != TABLE_INVALID_INFO)
    {
        //
        // Dev Init for the HDR MOD Manager
        //

        xpHdrModificationMgr *hdrModMgr = static_cast<xpHdrModificationMgr *>(xpIfPl::instance()->getMgr(xpHdrModificationMgr::getId()));

        if(hdrModMgr == NULL)
        {
            printf("<dev = %d>hdrModMgr is NULL. Exiting...\n",devId);
            return XP_ERR_INIT;
        }

        for(uint32_t i = 0; i < 3; i++)
        {
            tblCtxPerIdx.push_back(new xpTblCtxPerIdx());
        }

        // ASSUMPTION: tblCopyNum is same for all Insertion types
        tblCopyNum = tableProfile->getXpDaTblCopyNum(INS0_256_TBL);
        xpDirectTableContext* insTblCtx[3][tblCopyNum];

        for(uint8_t type = 0; type < 3; type++)
        {
            tblCtxPerIdx[type]->index = type;
            for(uint8_t i = 0; i < tblCopyNum; i++)
            {
                insTblCtx[type][i] = tableProfile->getXpDaCfg((xpPlDaDynamicTblList)(INS0_256_TBL + tblCtxPerIdx[type]->index), i);
            }
            tblCtxPerIdx[type]->tblCtx = (xpTableContext**) &insTblCtx[type];
        }

        if(hdrModMgr->addDevice(devId, tblCtxPerIdx, tblCopyNum, tblMode[xpHdrModificationMgr::getId()]) != XP_NO_ERR)
        {
            printf("ERROR in hdrModMgr->addDevice()\n");
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> Header Mod manager initialized successfully!!!\n",devId);
        }
        for(uint32_t i = 0; i < 3; i++)
        {
            delete tblCtxPerIdx[i];
        }

        tblCtxPerIdx.clear();
    }

    if(tblMode[xpTrunkResolutionMgr::getId()] != TABLE_INVALID_INFO)
    {
        //
        // Dev Init for the TRT  Manager
        //
        trunkResolutionMgr = (xpTrunkResolutionMgr *)xpIfPl::instance()->getMgr(xpTrunkResolutionMgr::getId());

        if(trunkResolutionMgr == NULL)
        {
            printf("<dev = %d> trunkResolutionMgr is NULL. Exiting...\n",devId);
            return XP_ERR_INIT;
        }

        /* Add Device */
        if((status = trunkResolutionMgr->addDevice(devId)) != XP_NO_ERR)
        {
            printf("ERROR <dev = %d>: Adding Device for trunkResolutionMgr failed. Exiting...\n",devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> trunkResolutionMgr initialized successfully!!!\n",devId);
        }
    }

    if(tblMode[xpEgressFilterMgr::getId()] != TABLE_INVALID_INFO)
    {
        //
        // Dev Init for  egress filter table manager
        //
        egFltMgr = static_cast<xpEgressFilterMgr *>(xpIfPl::instance()->getMgr(xpEgressFilterMgr::getId()));

        if(egFltMgr == NULL)
        {
            printf("<dev = %d>egFltMgr is NULL. Exiting...\n",devId);
            return XP_ERR_INIT;
        }
        if(egFltMgr->addDevice(devId) != XP_NO_ERR)
        {
            printf("ERROR<dev = %d>: Adding Device for egFltMgr manager failed. Exiting...\n",devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> Egress Filter Manager initialized successfully\n",devId);
        }
    }

    if(tblMode[xpBdMgr::getId()] != TABLE_INVALID_INFO)
    {
        //
        // Dev Init for the BD  Manager
        //
        bdMgr = (xpBdMgr *)xpL2Pl::instance()->getMgr(xpBdMgr::getId());

        if(bdMgr == NULL)
        {
            printf("ERROR <dev = %d> :bdMgr is NULL. Exiting...\n",devId);
            return XP_ERR_INIT;
        }

        tblCopyNum = tableProfile->getXpIsmeTblCopyNum(BD_TBL);
        xpIsmeTableContext* bdTblCtx[tblCopyNum];
        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            bdTblCtx[i] = tableProfile->getXpIsmeCfg(BD_TBL, i);
        }
        
        /* Add Device */
        if(bdMgr->addDevice(devId, (xpTableContext**)&bdTblCtx, tblCopyNum, TABLE_INIT_WITH_DYNAMIC_INFO) != XP_NO_ERR) //create BD table in HW
        {
            printf("ERROR <dev = %d> : Adding Device for BD table failed. Exiting...\n",devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> BD manager initialized successfully!!!\n",devId);
        }
    }

    if(tblMode[xpMdtMgr::getId()] != TABLE_INVALID_INFO)
    {
        //
        // Dev Init for the MDT  Manager
        //
        mdtMgr = (xpMdtMgr *)xpIfPl::instance()->getMgr(xpMdtMgr::getId());

        if(mdtMgr == NULL)
        {
            printf("ERROR <dev = %d > :xpMdtMgr is NULL. Exiting...\n", devId);
            return XP_ERR_INIT;
        }
        tblCopyNum = tableProfile->getXpDaTblCopyNum(MDT_TBL);
        xpDirectTableContext* mdtTblCtx[tblCopyNum];
        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            mdtTblCtx[i] = tableProfile->getXpDaCfg(MDT_TBL, i);
        }

        if((status = mdtMgr->addDevice(devId, (xpTableContext**)&mdtTblCtx, tblCopyNum, tblMode[xpMdtMgr::getId()])) != XP_NO_ERR)
        {
            printf("ERROR <dev = %d>: Adding Device for mdt failed. Exiting...\n", devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> MDT manager initialized successfully!!!\n",devId);
        }
    }

    if(tblMode[xpPortVlanIvifMgr::getId()] != TABLE_INVALID_INFO)
    {
        //
        // Dev Init for the PORT_VLAN  Manager
        //
        portVlanMgr = (xpPortVlanIvifMgr *)xpL2Pl::instance()->getMgr(xpPortVlanIvifMgr::getId());

        if(portVlanMgr == NULL)
        {
            printf(" ERROR <dev = %d> : portVlanMgr is NULL. Exiting...\n",devId);
            return XP_ERR_INIT;
        }

        tblCopyNum = tableProfile->getXpHashTblCopyNum(PORTVLANIVIF_TBL);
        xpHashTableContext* portVlanTblCtx[tblCopyNum];
        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            portVlanTblCtx[i] = tableProfile->getXpHashCfg(PORTVLANIVIF_TBL, i);
        }
        /* Add Device */
        if((status = portVlanMgr->addDevice(devId, (xpTableContext**)&portVlanTblCtx, tblCopyNum, tblMode[xpPortVlanIvifMgr::getId()])) != XP_NO_ERR)
        {
            printf("ERROR <dev = %d>: Adding Device for portVlanMgr failed. Exiting...\n",devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> portVlanMgr initialized successfully!!!\n",devId);
        }
    }

    if(tblMode[xpTunnelIvifMgr::getId()] != TABLE_INVALID_INFO)
    {
        //
        // Dev Init for the TUNNEL  Manager
        //
        tunnelIvifMgr = (xpTunnelIvifMgr *)xpTunnelPl::instance()->getMgr(xpTunnelIvifMgr::getId());

        if(tunnelIvifMgr == NULL)
        {
            printf("Erro <dev = %d> : tunnelIvifMgr is NULL. Exiting...\n", devId);
            return XP_ERR_INIT;
        }

        tblCopyNum = tableProfile->getXpHashTblCopyNum(TNL_IVIF_TBL);
        xpHashTableContext* tnlTblCtx[tblCopyNum];
        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            tnlTblCtx[i] = tableProfile->getXpHashCfg(TNL_IVIF_TBL, i);
        }

        if(tunnelIvifMgr->addDevice(devId, (xpTableContext**)&tnlTblCtx, tblCopyNum, tblMode[xpTunnelIvifMgr::getId()]) != XP_NO_ERR)
        {
            printf("ERROR: Adding Device for Tunnel failed. Exiting...\n");
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> TunnelIvifMgr initialized successfully!!!\n",devId);
        }
    }

    if(tblMode[xpFdbMgr::getId()] != TABLE_INVALID_INFO)
    {
        //
        // Dev Init for the FDB  Manager
        //
        fdbMgr = (xpFdbMgr *)xpL2Pl::instance()->getMgr(xpFdbMgr::getId());

        if(fdbMgr == NULL)
        {
            printf("ERROR <dev = %d> :fdbMgr is NULL. Exiting...\n",devId);
            return XP_ERR_INIT;
        }

        /* Add Device */
        tblCopyNum = tableProfile->getXpHashTblCopyNum(FDB_TBL);
        xpHashTableContext* fdbTblCtx[tblCopyNum];

        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            fdbTblCtx[i] = tableProfile->getXpHashCfg(FDB_TBL, i);
        }

        /* Add Device */
        if((status = fdbMgr->addDevice(devId, (xpTableContext**)&fdbTblCtx, tblCopyNum, tblMode[xpFdbMgr::getId()])) != XP_NO_ERR)
        {
            printf("ERROR <dev=%d>: Adding Device for Fdb failed. Exiting...\n",devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> xpFdbMgr initialized successfully!!!\n",devId);
        }
    }

    if(tblMode[xpIpRouteLpmMgr::getId(xpIpRouteLpmMgr::IPV4_ROUTE_MGR)] != TABLE_INVALID_INFO)
    {
        tblCopyNum = tableProfile->getXpLpmTblCopyNum();
        xpLpmTableContext* lpmV4Ctx[tblCopyNum];

        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            lpmV4Ctx[i] = tableProfile->getXpLpmV4Cfg(i);
        }

        //
        //PL : Add device for Ipv4 Route Manager
        //
        ipv4RouteMgr = (xpIpRouteLpmMgr*) xpL3Pl::instance()->getMgr(xpIpRouteLpmMgr::getId(xpIpRouteLpmMgr::IPV4_ROUTE_MGR));
        if (!ipv4RouteMgr)
        {
            return XP_ERR_INIT;
        }

        status = ipv4RouteMgr->addDevice(devId, (xpTableContext**)&lpmV4Ctx, tblCopyNum);
        if (status)
        {
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("Ipv4Route manager initialized successfully!!!\n");
        }
    }

    if(tblMode[xpIpv4HostMgr::getId()] != TABLE_INVALID_INFO)
    {
        //
        // PL : Add device for IPv4 host Manager
        //
        ipv4HostMgr = static_cast<xpIpv4HostMgr *>(xpL3Pl::instance()->getMgr(xpIpv4HostMgr::getId()));
        if (!ipv4HostMgr)
        {
            return XP_ERR_INIT;
        }

        tblCopyNum = tableProfile->getXpHashTblCopyNum(IPV4HOST_TBL);
        xpHashTableContext* ipv4TblCtx[tblCopyNum];

        for(uint8_t i = 0; i < tblCopyNum; i++)
        {   
            ipv4TblCtx[i] = tableProfile->getXpHashCfg(IPV4HOST_TBL, i);
        }   
        if(ipv4HostMgr->addDevice(devId, (xpTableContext**)&ipv4TblCtx, tblCopyNum, tblMode[xpIpv4HostMgr::getId()]) != XP_NO_ERR)
        {   
            printf("ERROR: Adding Device for ipv4Host manager failed. Exiting...\n");
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("Ipv4Host manager initialized successfully!!!\n");
        }
    }

    if(tblMode[xpIpRouteLpmMgr::getId(xpIpRouteLpmMgr::IPV6_ROUTE_MGR)] != TABLE_INVALID_INFO)
    {
        tblCopyNum = tableProfile->getXpLpmTblCopyNum();
        xpLpmTableContext* lpmV6Ctx[tblCopyNum];

        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            lpmV6Ctx[i] = tableProfile->getXpLpmV6Cfg(i);
        }

        //
        //PL : Add device for Ipv6 Route Manager
        //
        ipv6RouteMgr = (xpIpRouteLpmMgr*) xpL3Pl::instance()->getMgr(xpIpRouteLpmMgr::getId(xpIpRouteLpmMgr::IPV6_ROUTE_MGR));
        if (!ipv6RouteMgr)
        {
            return XP_ERR_INIT;
        }

        //PL : Add device for Ipv6 Route Manager
        status = ipv6RouteMgr->addDevice(devId, (xpTableContext**)&lpmV6Ctx, tblCopyNum);
        if (status)
        {
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("Ipv6Route manager initialized successfully!!!\n");
        }
    }

    if(tblMode[xpIpv6HostMgr::getId()] != TABLE_INVALID_INFO)
    {
        ipv6HostMgr = static_cast<xpIpv6HostMgr *>(xpL3Pl::instance()->getMgr(xpIpv6HostMgr::getId()));
        if (!ipv6HostMgr)
        {
            return XP_ERR_INIT;
        }

        tblCopyNum = tableProfile->getXpHashTblCopyNum(IPV6HOST_TBL);
        xpHashTableContext* ipv6TblCtx[tblCopyNum];

        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            ipv6TblCtx[i] = tableProfile->getXpHashCfg(IPV6HOST_TBL, i);
        }

        if(ipv6HostMgr->addDevice(devId, (xpTableContext**)&ipv6TblCtx, tblCopyNum, tblMode[xpIpv6HostMgr::getId()]) != XP_NO_ERR)
        {
            printf("ERROR: Adding Device for ipv6Host manager failed. Exiting...\n");
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("Ipv6Host manager initialized successfully!!!\n");
        }
    }

    if(tblMode[xpPortConfigMgr::getId()] != TABLE_INVALID_INFO)
    {
        //
        // Dev Init for Port Config manager
        //
        portCfgMgr = static_cast<xpPortConfigMgr *>(xpIfPl::instance()->getMgr(xpPortConfigMgr::getId()));
        if(portCfgMgr == NULL)
        {
            printf("ERROR <dev = %d> :portCfgMgr is NULL. Exiting...\n",devId);
            return XP_ERR_INIT;
        }
        if((status = portCfgMgr->addDevice(devId)) != XP_NO_ERR)
        {
            printf("ERROR <dev= %d>: Adding Device for portConfig manager failed. Exiting...\n",devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> Port Config manager initialized successfully!!!\n",devId);
        }
    }
   
    if(tblMode[xpControlMacMgr::getId()] != TABLE_INVALID_INFO)
    {
        // Creation and initialization of controlMac PM
        controlMacMgr = static_cast<xpControlMacMgr *>(xpIfPl::instance()->getMgr(xpControlMacMgr::getId()));

        if(controlMacMgr->addDevice(devId) != XP_NO_ERR)
        {
            printf("ERROR <dev=%d> : controlMacMgr->addDevice failed. Exiting...\n", devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> controlMac Manager initialized successfully\n",devId);
        }
    }

    if(tblMode[xpIaclMgr::getId()] != TABLE_INVALID_INFO)
    {
        // Creation and initialization of IACL manager
        iaclMgr = static_cast<xpIaclMgr *>(xpAclPl::instance()->getMgr(xpIaclMgr::getId()));

        /* Add Device*/
        for(uint32_t i = 0; i < 3; i++)
        {
            tblCtxPerIdx.push_back(new xpTblCtxPerIdx());
        }

        tblCopyNum = tableProfile->getXpTcamTblCopyNum(IACL0_TBL);
        xpMatchTableContext* iaclTblCtx[3][tblCopyNum];

        tblCtxPerIdx[0]->index = XP_IACL0;
        tblCtxPerIdx[1]->index = XP_IACL1;
        tblCtxPerIdx[2]->index = XP_IACL2;
        for(uint8_t type = 0; type < 3; type++)
        {
            for(uint8_t i = 0; i < tblCopyNum; i++)
            {
                iaclTblCtx[type][i] = tableProfile->getXpTcamCfg((xpPlTcamDynamicTblList)(IACL0_TBL + tblCtxPerIdx[type]->index), i);
            }
            tblCtxPerIdx[type]->tblCtx = (xpTableContext**) &iaclTblCtx[type];
        }

        if(iaclMgr->addDevice(devId, tblCtxPerIdx, tblCopyNum, tblMode[xpIaclMgr::getId()]) != XP_NO_ERR)
        {
            printf("ERROR <dev=%d> : iaclMgr->addDevice failed. Exiting...\n", devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> iaclMac Manager initialized successfully\n",devId);
        }
        for(uint32_t i = 0; i < 3; i++)
        {
            delete tblCtxPerIdx[i];
        }

        tblCtxPerIdx.clear();
    }

    if(tblMode[xpIpv4RouteMcMgr::getId()] != TABLE_INVALID_INFO)
    {
        // Creation and initialization for ipv4 Route Mc
        tblCopyNum = tableProfile->getXpHashTblCopyNum(IPV4ROUTEMC_TBL);
        xpIpv4RouteMcMgr *ipv4RouteMcMgr = static_cast<xpIpv4RouteMcMgr *>(xpMcastPl::instance()->getMgr(xpIpv4RouteMcMgr::getId()));

        /* Add Device */
        xpHashTableContext* ipv4RouteMcTblCtx[tblCopyNum];
        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            ipv4RouteMcTblCtx[i] = tableProfile->getXpHashCfg(IPV4ROUTEMC_TBL, i);
        }
        if(ipv4RouteMcMgr->addDevice(devId, (xpTableContext**)&ipv4RouteMcTblCtx, tblCopyNum, tblMode[xpIpv4RouteMcMgr::getId()]) != XP_NO_ERR)
        {
            printf("ERROR <dev=%d> : ipv4RouteMcMgr->addDevice failed. Exiting...\n", devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> ipv4RouteMc Manager initialized successfully\n",devId);
        }
    }

    if(tblMode[xpIpv6RouteMcMgr::getId()] != TABLE_INVALID_INFO)
    {
        // Creation and initialization for ipv6 route mc
        tblCopyNum = tableProfile->getXpHashTblCopyNum(IPV6ROUTEMC_TBL);
        xpIpv6RouteMcMgr *ipv6RouteMcMgr = static_cast<xpIpv6RouteMcMgr *>(xpMcastPl::instance()->getMgr(xpIpv6RouteMcMgr::getId()));

        /* Add Device */
        xpHashTableContext* ipv6RouteMcTblCtx[tblCopyNum];
        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            ipv6RouteMcTblCtx[i] = tableProfile->getXpHashCfg(IPV6ROUTEMC_TBL, i);
        }

        if(ipv6RouteMcMgr->addDevice(devId, (xpTableContext**)&ipv6RouteMcTblCtx, tblCopyNum, tblMode[xpIpv6RouteMcMgr::getId()]) != XP_NO_ERR)
        {
            printf("ERROR <dev=%d> : ipv6RouteMcMgr->addDevice failed. Exiting...\n", devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> ipv6RouteMc Manager initialized successfully\n",devId);
        }
    }

    if(tblMode[xpIpv4BridgeMcMgr::getId()] != TABLE_INVALID_INFO)
    {
        // Creation and initialization for ipv4 Bridge Mc
        tblCopyNum = tableProfile->getXpHashTblCopyNum(IPV4BRIDGEMC_TBL);
        xpIpv4BridgeMcMgr *ipv4BridgeMcMgr = static_cast<xpIpv4BridgeMcMgr *>(xpMcastPl::instance()->getMgr(xpIpv4BridgeMcMgr::getId()));

        /* Add Device */
        xpHashTableContext* ipv4BridgeMcTblCtx[tblCopyNum];
        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            ipv4BridgeMcTblCtx[i] = tableProfile->getXpHashCfg(IPV4BRIDGEMC_TBL, i);
        }
        if(ipv4BridgeMcMgr->addDevice(devId, (xpTableContext**)&ipv4BridgeMcTblCtx, tblCopyNum, tblMode[xpIpv4BridgeMcMgr::getId()]) != XP_NO_ERR)
        {
            printf("ERROR <dev=%d> : ipv4BridgeMcMgr->addDevice failed. Exiting...\n", devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> ipv4BridgeMc Manager initialized successfully\n",devId);
        }
    }

    if(tblMode[xpIpv6BridgeMcMgr::getId()] != TABLE_INVALID_INFO)
    {
        // Creation and initialization for ipv6 bridge mc
        tblCopyNum = tableProfile->getXpHashTblCopyNum(IPV6BRIDGEMC_TBL);
        xpIpv6BridgeMcMgr *ipv6BridgeMcMgr = static_cast<xpIpv6BridgeMcMgr *>(xpMcastPl::instance()->getMgr(xpIpv6BridgeMcMgr::getId()));

        /* Add Device */
        xpHashTableContext* ipv6BridgeMcTblCtx[tblCopyNum];
        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            ipv6BridgeMcTblCtx[i] = tableProfile->getXpHashCfg(IPV6BRIDGEMC_TBL, i);
        }

        if(ipv6BridgeMcMgr->addDevice(devId, (xpTableContext**)&ipv6BridgeMcTblCtx, tblCopyNum, tblMode[xpIpv6BridgeMcMgr::getId()]) != XP_NO_ERR)
        {
            printf("ERROR <dev=%d> : ipv6BridgeMcMgr->addDevice failed. Exiting...\n", devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> ipv6BridgeMc Manager initialized successfully\n",devId);
        }
    }

    if(tblMode[xpIpv4PIMBiDirRPFMgr::getId()] != TABLE_INVALID_INFO)
    {
        // Creation and initialization for ipv4 Bridge Mc
        tblCopyNum = tableProfile->getXpHashTblCopyNum(IPV4PIMBIDIRRPF_TBL);
        xpIpv4PIMBiDirRPFMgr *ipv4PimMgr = static_cast<xpIpv4PIMBiDirRPFMgr *>(xpMcastPl::instance()->getMgr(xpIpv4PIMBiDirRPFMgr::getId()));

        /* Add Device */
        xpHashTableContext* ipv4PimTblCtx[tblCopyNum];
        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            ipv4PimTblCtx[i] = tableProfile->getXpHashCfg(IPV4PIMBIDIRRPF_TBL, i);
        }
        if(ipv4PimMgr->addDevice(devId, (xpTableContext**)&ipv4PimTblCtx, tblCopyNum, TABLE_INIT_WITH_DYNAMIC_INFO) != XP_NO_ERR)
        {
            printf("ERROR <dev=%d> : ipv4PimMgr->addDevice failed. Exiting...\n", devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> ipv4PimMgr Manager initialized successfully\n",devId);
        }
    }

#if 0
    if(tblMode[xpIpv6PIMBiDirRPFMgr::getId()] != TABLE_INVALID_INFO)
    {
        // Creation and initialization for ipv6 pim
        tblCopyNum = tableProfile->getXpHashTblCopyNum(IPV6PIMBIDIRRPF_TBL);
        xpIpv6PIMBiDirRPFMgr *ipv6PimMgr = static_cast<xpIpv6PIMBiDirRPFMgr *>(xpMcastPl::instance()->getMgr(xpIpv6PIMBiDirRPFMgr::getId()));

        /* Add Device */
        xpHashTableContext* ipv6PimTblCtx[tblCopyNum];
        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            ipv6PimTblCtx[i] = tableProfile->getXpHashCfg(IPV6PIMBIDIRRPF_TBL, i);
        }

        if(ipv6PimMgr->addDevice(devId, (xpTableContext**)&ipv6PimTblCtx, tblCopyNum, TABLE_INIT_WITH_DYNAMIC_INFO) != XP_NO_ERR)
        {
            printf("ERROR <dev=%d> : ipv6PimMgr->addDevice failed. Exiting...\n", devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> ipv6PimMgr Manager initialized successfully\n",devId);
        }
    }
#endif
    if(tblMode[xpNATIpv4Mgr::getId()] != TABLE_INVALID_INFO)
    {
        // Creation and initialization for ipv4 Bridge Mc
        tblCopyNum = tableProfile->getXpTcamTblCopyNum(NATIPV4_TBL);
        xpNATIpv4Mgr *natIpv4Mgr = static_cast<xpNATIpv4Mgr *>(xpL3Pl::instance()->getMgr(xpNATIpv4Mgr::getId()));

        /* Add Device */
        xpMatchTableContext* natIpv4TblCtx[tblCopyNum];
        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            natIpv4TblCtx[i] = tableProfile->getXpTcamCfg(NATIPV4_TBL, i);
        }
        if(natIpv4Mgr->addDevice(devId, (xpTableContext**)&natIpv4TblCtx, tblCopyNum, tblMode[xpNATIpv4Mgr::getId()]) != XP_NO_ERR)
        {
            printf("ERROR <dev=%d> : natIpv4Mgr->addDevice failed. Exiting...\n", devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> natIpv4Mgr Manager initialized successfully\n",devId);
        }
    }

    if(tblMode[xpNATCompIpv6Mgr::getId()] != TABLE_INVALID_INFO)
    {
        // Creation and initialization for ipv6 pim
        tblCopyNum = tableProfile->getXpHashTblCopyNum(NATCOMPIPV6_TBL);
        xpNATCompIpv6Mgr *natIpv6Mgr = static_cast<xpNATCompIpv6Mgr *>(xpL3Pl::instance()->getMgr(xpNATCompIpv6Mgr::getId()));

        /* Add Device */
        xpHashTableContext* natIpv6TblCtx[tblCopyNum];
        for(uint8_t i = 0; i < tblCopyNum; i++)
        {
            natIpv6TblCtx[i] = tableProfile->getXpHashCfg(NATCOMPIPV6_TBL, i);
        }

        if(natIpv6Mgr->addDevice(devId, (xpTableContext**)&natIpv6TblCtx, tblCopyNum, tblMode[xpNATCompIpv6Mgr::getId()]) != XP_NO_ERR)
        {
            printf("ERROR <dev=%d> : natIpv6Mgr->addDevice failed. Exiting...\n", devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> natIpv6Mgr Manager initialized successfully\n",devId);
        }
    }

    if(tblMode[xpReasonCodeMgr::getId()] != TABLE_INVALID_INFO)
    {
        // Creation and initialization of RCT PM
        xpReasonCodeMgr* rctMgr = static_cast<xpReasonCodeMgr *>(xpIfPl::instance()->getMgr(xpReasonCodeMgr::getId()));

        if(rctMgr->addDevice(devId) != XP_NO_ERR)
        {
            printf("ERROR <dev=%d> : rctMgr->addDevice failed. Exiting...\n", devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> rctMgr Manager initialized successfully\n",devId);
        }
    }

    if(tblMode[xpQosMapMgr::getId()] != TABLE_INVALID_INFO)
    {
        // Creation and initialization of RCT PM
        xpQosMapMgr* qosMgr = static_cast<xpQosMapMgr *>(xpQosPl::instance()->getMgr(xpQosMapMgr::getId()));

        if(qosMgr->addDevice(devId) != XP_NO_ERR)
        {
            printf("ERROR <dev=%d> : qosMgr->addDevice failed. Exiting...\n", devId);
            addFail = true;
            //return XP_ERR_INIT;
        }
        else
        {
            printf("<dev = %d> qosMgr Manager initialized successfully\n",devId);
        }
    }

    if(addFail)
        return XP_ERR_INIT;
    else
        return XP_NO_ERR; //Success
}

//
// Device Level sdk APP init
//
XP_STATUS xdkInitDevice(xpDevice_t devNum, XP_DEV_TYPE_T devType, xpDevConfigStruct *devConfig)
{
    XP_STATUS status;
#if 0
    //
    //Add the device to the Device Manager Layer
    //
    status = xpDeviceMgr::instance()->addDevice(devNum, devType, devConfig);
    if(status)
    {
        LOGFN(xpLogModWmTop, XP_SUBMOD_MAIN, XP_LOG_CRITICAL, "DM Add Device failed. ERROR #%1d", status);
        return status;
    }

   //
    //Initialize the Device Manager Layer
    //
    status = xpDeviceMgr::instance()->init(devNum, devConfig);
    if(status)
    {
        LOGFN(xpLogModWmTop, XP_SUBMOD_MAIN, XP_LOG_CRITICAL, "Device Manager Init failed. ERROR #%1d", status);
        return status;
    }
#endif

    xpInitMgr  *initMgr;
    initMgr = new xpInitMgr(devNum);
    status = initMgr->xpDeviceInit(devNum,devType,devConfig);
    //status = xpDeviceMgr::instance()->init(devNum, devConfig);
    if(status) 
    {
        printf("Device Manager Init failed. Error #%1d", status);
        return status;
    }

    // Get tableProfile for the device
    tableProfile = xpDeviceMgr::instance()->getDeviceObj(devNum)->getTableProfile();

    //
    //Init the Primitive Tables
    //
    status = xpAppPLAddDevice(devNum, devConfig->pipeLineMode, TABLE_INIT_WITH_FIXED_INFO);
    if(status != 0)
    {
        cout << "ERROR: Can not initialize Primitive Managers !!!" << endl;
        return status;
    }

    // Added to test initialization and addDevice of all PMs with different tblMode
#if 0
    XP_TABLE_MODE_T tblMode[XP_PL_MGR_MAX];
    for(int i = 0; i < XP_PL_MGR_MAX; i++)
    {
        tblMode[i] = TABLE_INIT_WITH_FIXED_INFO;
    }
    //
    //Init the Primitive Tables
    //
    status = xpAppPLAddDevice(devNum, mode, tblMode);
    if(status != 0)
    {
        cout << "ERROR: Can not initialize Primitive Managers !!!" << endl;
        return status;
    }
#endif

    return status;
}

//  
// Device level shadow init
//  
XP_STATUS xdkInitShadow(xpDevice_t devNum, XP_DEV_TYPE_T devType, xpDevConfigStruct *devConfig)
{
    XP_STATUS status = XP_NO_ERR;
    //
    //Add the device to the Device Manager Layer
    //  
    status = xpDeviceMgr::instance()->addDevice(devNum, devType, devConfig);
    if(status) 
    {
        printf("DM Add Device failed. Error #%1d", status);
        return status;
    }

    return status;
}

int main(int argc, char *argv[])
{
    XP_STATUS status;
    xpDevice_t devNum = XP_TEST_DEV_NUM;

    processCLI(argc, argv);

    //Program the HW Tables for demo - using the config data input
    XP_PIPE_MODE mode = (XP_PIPE_MODE)(XP_CONFIG()->pipeLineNum);
    devDefaultConfig.pipeLineMode = mode;
    devDefaultConfig.mode = XP_CONFIG()->mode;
        
    //Init the shadow/device manager which is kinda used by both WM and SDK (modules)
    if((status = xdkInitShadow(devNum, XP80, &devDefaultConfig)) != XP_NO_ERR)
    {
        printf("xdkInitShadow Failed.. Error #%1d", status);
        return status;
    }

    //Init the xp80 WhiteModel
    xpWmInit(XP80);

    //Init the Logger
    if((status = xpInitSdkLogger()) != XP_NO_ERR)
    {
        printf("xdkInit : xpInitSdkLogger Failed... ERROR #%1d", status);
        return status;
    }
    xpBaseLog* logPtr = xpLogMgr::instance()->find(XP_CONSOLE_LOG, (char*) XP_DEFAULT_CONSOLE_NAME);
    if (logPtr == NULL)
    {
        logPtr = xpLogMgr::instance()->create(XP_CONSOLE_LOG, (char*) XP_DEFAULT_CONSOLE_NAME, 0, false);
    }
    xpLogMgr::instance()->connectAll(logPtr);

    //
    // Init PL
    //
    status = xpAppPLInit(devNum, mode);
    if(status != XP_NO_ERR)
    {
        LOGFN(xpLogModWmTop, XP_SUBMOD_MAIN, XP_LOG_CRITICAL, "xdkInit : xpAppPLInit Failed.. ERROR #%1d", status);
        return status;
    }

    status = xdkInitDevice (devNum, XP80, &devDefaultConfig);
    if (status)
    {
        LOGFN(xpLogModWmTop, XP_SUBMOD_MAIN, XP_LOG_CRITICAL, "xdkInit : xdkInitDevice Failed.. ERROR #%1d", status);
        return status;
    }
    
    // Unit tests of PL
    primitiveTableUnitTests(devNum, mode);
   // testMirrorMgr();
}

