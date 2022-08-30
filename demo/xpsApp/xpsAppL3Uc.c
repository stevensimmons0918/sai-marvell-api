// xpAppL3Uc.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsAppL3Uc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "xpsVlan.h"
#include "xpsL3.h"

XP_STATUS xpAppL3UcAddPrefix(xpDevice_t devId, uint32_t ingressPort,
                             uint32_t* egressPorts, uint32_t egressPortSize, uint32_t numNhGroups,
                             uint32_t ecmpSize)
{
    XP_STATUS status = XP_NO_ERR;
    int i = 0;
    int j = 0;
    int k = 0;
    uint32_t nhIdxList[numNhGroups];
    xpsL3NextHopEntry_t nhEntry1;
    uint32_t prefixHandle = 0;
    uint32_t prefixIPCount = 0;
    uint32_t* prefixIPAddr = NULL;
    uint32_t* prefixIPMask =  NULL;
    uint32_t prefixFullCount[32];
    uint32_t subTrieFailCount[32];
    uint32_t subTrieNhiFailCount[32];
    FILE *routeFile = NULL;

    /* TODO no corresponding XPS API exists.  Need to get XPS API to differentiate profiles
        XP_PROFILE_TYPE pType;
        if((status = xpInterfaceGetTableProfileTypeFromDevId(devId, &pType)) != XP_NO_ERR)
        {
            return status;
        }
        if (pType == XP_ROUTE_CENTRIC_SINGLE_PIPE_PROFILE)
        {
    */
    //        routeFile = fopen("./pipeline-profiles/xpDefault/demo/common/route4_2Mill.txt", "r");
    /*    }
        else
        {
    */
    routeFile = fopen("./pipeline-profiles/xpDefault/demo/common/route4.txt", "r");
    /*    }
    */
    if (!routeFile)
    {
        printf("Fail top open the BGP Profile \n");
        return XP_ERR_INIT;
    }

    if (!fscanf(routeFile, "%u", &prefixIPCount))
    {
        printf("Error Fail to read total Route Count \n");
        fclose(routeFile);
        return XP_ERR_INIT;
    }

    memset(prefixFullCount, 0, sizeof(prefixFullCount));
    memset(subTrieFailCount, 0, sizeof(subTrieFailCount));
    memset(subTrieNhiFailCount, 0, sizeof(subTrieNhiFailCount));

    prefixIPAddr = (uint32_t*) malloc(sizeof(uint32_t) * prefixIPCount);
    prefixIPMask = (uint32_t*) malloc(sizeof(uint32_t) * prefixIPCount);

    if ((!prefixIPAddr) || (!prefixIPMask))
    {
        printf("Route Memory Allocation Failed \n");
        fclose(routeFile);
        free(prefixIPAddr);
        free(prefixIPMask);
        return XP_ERR_INIT;
    }
    memset(prefixIPAddr, 0, sizeof(uint32_t) * prefixIPCount);
    memset(prefixIPMask, 0, sizeof(uint32_t) * prefixIPCount);

    uint32_t count = 0;
    uint32_t prefixWithInSubtrieCount[32];
    uint32_t prefixWithSubTrieCountt[32];

    memset(prefixWithInSubtrieCount, 0, (sizeof(prefixWithInSubtrieCount)));
    memset(prefixWithSubTrieCountt, 0, (sizeof(prefixWithSubTrieCountt)));

    while (count < prefixIPCount)
    {
        if (2 != fscanf(routeFile, "%u%x", (prefixIPMask + count),
                        (prefixIPAddr + count)))
        {
            printf("Error Fail to read total Route IP mask and IP addr \n");
            fclose(routeFile);
            free(prefixIPAddr);
            free(prefixIPMask);
            return XP_ERR_INIT;
        }
        //printf("(%d): %u.%u.%u.%u/%u \n", count, *((uint8_t*)(prefixIPAddr+count)+3), *((uint8_t*)(prefixIPAddr+count)+2),
        //        *((uint8_t*)(prefixIPAddr+count)+1), *((uint8_t*)(prefixIPAddr+count)+0), *(prefixIPMask+count));
        count++;
    }

    if (prefixIPCount != count)
    {
        printf("Warning: Invalid parameter for max prefixes and number of prefixes read\n");
    }
    fclose(routeFile);

    prefixIPCount = count;
    /*************************************************
     * XPS APIs/Methods
     *************************************************/

    /* Remove Ingress & Egress ports from default BD */
    status = xpsVlanRemoveInterface(devId, 100, ingressPort);
    if (status != XP_NO_ERR)
    {
        printf("%s:Error: xpsVlanRemoveInterface() for devId %d failed with error code: %d\n",
               __FUNCTION__, devId, status);
        free(prefixIPAddr);
        free(prefixIPMask);
        return status;
    }
    for (k = 0; k < egressPortSize; k++)
    {
        status = xpsVlanRemoveInterface(devId, 100, egressPorts[k]);
        if (status != XP_NO_ERR)
        {
            printf("%s:Error: xpsVlanRemoveInterface() for devId %d failed with error code: %d\n",
                   __FUNCTION__, devId, status);
            free(prefixIPAddr);
            free(prefixIPMask);
            return status;
        }
    }

    /* Create L3 Port Interfaces into idList */
    xpsInterfaceId_t idList[egressPortSize + 1];

    for (k = 0; k < egressPortSize + 1; k++)
    {
        xpsL3CreatePortIntf(&(idList[k]));
        xpsL3InitPortIntf(devId, idList[k]);

        if (k == 0)
        {
            xpsL3BindPortIntf(ingressPort, idList[k]);
        }
        else
        {
            xpsL3BindPortIntf(egressPorts[k-1], idList[k]);
        }
        status = xpsL3SetIntfIpv4UcRoutingEn(devId, idList[k], 1);
        if (status != XP_NO_ERR)
        {
            printf("%s:Error: xpsL3SetIntfIpv4UcRoutingEn() for devId %d failed with error code: %d\n",
                   __FUNCTION__, devId, status);
            free(prefixIPAddr);
            free(prefixIPMask);
            return status;
        }
    }

    /* Hardcod Ingress Router Mac Address 00:01:02:03:04:05 */
    macAddr_t mac1;
    mac1[0] = 0x05;
    mac1[1] = 0x04;
    mac1[2] = 0x03;
    mac1[3] = 0x02;
    mac1[4] = 0x01;
    mac1[5] = 0x00;
    xpsL3AddIngressRouterMac(devId, mac1);

    /* Hardcode Egress Router Mac Address Bits 00:44:33:22:11:xx */
    macAddrHigh_t macHi1;
    macHi1[0] = 0x11;
    macHi1[1] = 0x22;
    macHi1[2] = 0x33;
    macHi1[3] = 0x44;
    macHi1[4] = 0x00;
    xpsL3SetEgressRouterMacMSbs(devId, macHi1);

    for (k = 1; k < egressPortSize + 1; k++)
    {
        status = xpsL3SetIntfEgressRouterMacLSB(devId, idList[k], k);
        if (status != XP_NO_ERR)
        {
            printf("%s:Error: xpsL3SetIntfEgressRouterMacLSB() for devId %d failed with error code: %d\n",
                   __FUNCTION__, devId, status);
            free(prefixIPAddr);
            free(prefixIPMask);
            return status;
        }
    }

    /* Create NH Entries with ecmpSize */
    for (k = 0; k < numNhGroups; k++)
    {
        status = xpsL3CreateRouteNextHop(ecmpSize, &(nhIdxList[k]));
        if (status)
        {
            printf("NH1 allocation failed\n");
            free(prefixIPAddr);
            free(prefixIPMask);
            return status;
        }
    }

    /* Set nhEntry to forward & macDa = {0x00, 0x22, 0x44,0x66,0x88,0xxx} */
    nhEntry1.pktCmd = XP_PKTCMD_FWD;
    nhEntry1.serviceInstId = 0;
    nhEntry1.vpnLabel = 0;
    nhEntry1.propTTL = 1;
    nhEntry1.reasonCode = 0;
    nhEntry1.nextHop.macDa[1] = 0x22;
    nhEntry1.nextHop.macDa[2] = 0x44;
    nhEntry1.nextHop.macDa[3] = 0x66;
    nhEntry1.nextHop.macDa[4] = 0x88;

    for (k = 0; k < numNhGroups; k++)
    {
        for (j = 0; j < ecmpSize; j++)
        {
            nhEntry1.nextHop.macDa[5] = j;

            i = (j % egressPortSize) + 1;
            nhEntry1.nextHop.l3InterfaceId = idList[i];
            nhEntry1.nextHop.egressIntfId = egressPorts[i-1];

            status = xpsL3SetRouteNextHop(devId, nhIdxList[k] + j, &nhEntry1);
            if (status != XP_NO_ERR)
            {
                printf("xpsL3SetRouteNextHop failed\n");
                free(prefixIPAddr);
                free(prefixIPMask);
                return status;
            }
        }
    }

    Float sCnt=0, fCnt=0;

    xpsL3RouteEntry_t pL3RouteEntry1;
    pL3RouteEntry1.vrfId = 0;
    pL3RouteEntry1.type = XP_PREFIX_TYPE_IPV4;

    for (count = 0; count < prefixIPCount; count++)
    {
        prefixHandle = 0;
        i = count % numNhGroups;

        pL3RouteEntry1.ipv4Addr[0] = (*(prefixIPAddr + count) & 0x000000ff);
        pL3RouteEntry1.ipv4Addr[1] = (*(prefixIPAddr + count) & 0x0000ff00) >> 8;
        pL3RouteEntry1.ipv4Addr[2] = (*(prefixIPAddr + count) & 0x00ff0000) >> 16;
        pL3RouteEntry1.ipv4Addr[3] = (*(prefixIPAddr + count) & 0xff000000) >> 24;
        pL3RouteEntry1.ipMaskLen = *(prefixIPMask + count);
        pL3RouteEntry1.nhEcmpSize = ecmpSize;
        pL3RouteEntry1.nhId = nhIdxList[i];

        status = xpsL3AddIpRouteEntry(devId, &pL3RouteEntry1, &prefixHandle);

        if (count % 1000 == 0)
        {
            printf("Count: %u \n", count);
        }
        if (status)
        {
            fCnt++;

            /* No Resource for NHI When Creating SubTrie */
            if (status == XP_ERR_RESOURCE_NOT_AVAILABLE)
            {
                subTrieFailCount[(*(prefixIPMask + count)) - 1]++;
            }
            /* No Resource for Creating Sub-Trie Itself */
            if (status == XP_ERR_OUT_OF_IDS)
            {
                subTrieNhiFailCount[(*(prefixIPMask + count)) - 1]++;
            }
            /* No Resource for Entry within the Sub Trie . Valid
             * only in Sparse Mode where all Leaf Gets Occupied */
            if (status == XP_ERR_OUT_OF_MEM)
            {
                prefixFullCount[(*(prefixIPMask + count)) - 1]++;
            }
        }
        else
        {
            prefixWithInSubtrieCount[(*(prefixIPMask + count)) - 1]++;
            //cout << " Prefix Add Sucessful with Handle : " << prefixHandle << endl;
            sCnt++;
        }
    }

    printf("Num\t\t\tTread\t\t\tStride\t\t\t#SubTries\t\t\t#Routes\t\t\tNhiFull\t\t\tHashColl\n");

    printf("\n\n");

    printf("LPM STATS: (Total: %u); Success(%.0f), Failure(%.0f);\n", prefixIPCount,
           sCnt, fCnt);
    printf("           (Success percentage: %3.4f);\n",
           ((Float)((100*sCnt)/(sCnt+fCnt))));


    free(prefixIPAddr);
    free(prefixIPMask);

    return XP_NO_ERR;
}

/*
XP_STATUS xpAppL3UcDisplayPrefixes (xpDevice_t devId)
{
    XP_STATUS status = XP_NO_ERR;
    xpLpmTable *lpmTbl = NULL;
    xpTableManager *tblMgr = xpDeviceMgr::instance()->getTableMgr(devId);

    if (!tblMgr)
    {
        return XP_ERR_NULL_POINTER;
    }

    lpmTbl = ( xpLpmTable *) tblMgr->getTableObj(160, SE_GLOBAL);

    if (!lpmTbl)
    {
        return XP_ERR_NULL_POINTER;
    }

    for (int i = 0; i < lpmTbl->getMaxPrefixTables(); i++)
    {
        for (unsigned int j =0; j < lpmTbl->getPrefixTableNumEntries(i); j++)
        {
            uint32_t vrfId = 0x0;
            std::vector<XpBitVector> prefixIpAddr;
            std::vector<uint32_t> prefixIpMask;
            std::vector<uint32_t> nhId;
            std::vector<uint32_t> nhEcmpSize;

            status = lpmTbl->readEntry(i, j, vrfId, prefixIpAddr, prefixIpMask, nhId, nhEcmpSize);

            if (status)
            {
               return status;
            }

            if (!prefixIpAddr.size())
            {
                continue;
            }

            for (unsigned int k =0; k < prefixIpAddr.size(); k++)
            {
                cout << "Vrf Id:" << vrfId << endl;
                cout << "Ip Address : ";
                for (int l = (prefixIpAddr[k].size() - 1); l >= 7; l = l - 8)
                {
                    cout << (prefixIpAddr[k](l - 7  , l)).toUint32()<<".";
                }
                cout << "Ip Mask " << (int)prefixIpMask[k] << endl;
                cout << "Next Hop Base Id " << nhId[k] << endl;
                cout << "Ecmp Size " << nhEcmpSize[k] << endl;
            }
        }
    }

    return XP_NO_ERR;
}
*/

XP_STATUS xpsAppL3UcAddIpv6Prefix(xpDevice_t devId)
{
    //XP_STATUS status = XP_NO_ERR;
    //uint32_t vrfId =  0x4;
    //uint32_t ecmpSize = 0x1;
    //uint32_t nhIdx = 0;
    //uint32_t prefixHandle = 0;
    //uint32_t prefixIPCount = 0;
    //uint32_t*prefixIPAddr = NULL;
    //uint32_t* prefixIPMask =  NULL;
    //uint32_t prefixFullCount[128];
    //uint32_t subTrieFailCount[128];
    //uint32_t subTrieNhiFailCount[128];
    //uint8_t isNewSubTrie = 0;
    //FILE *routeFile = fopen("./xpApp/common/route6.txt", "r");
    //
    //if (!routeFile)
    //{
    //    printf ("Fail top open the route6.txt Profile \n");
    //    return XP_ERR_INIT;
    //}

    //if (!fscanf(routeFile,"%u",&prefixIPCount))
    //{
    //    printf ("Error Fail to read total Route Count \n");
    //    fclose(routeFile);
    //    return XP_ERR_INIT;
    //}

    //memset (prefixFullCount, 0, sizeof (prefixFullCount));
    //memset (subTrieFailCount, 0, sizeof (subTrieFailCount));
    //memset (subTrieNhiFailCount, 0, sizeof (subTrieNhiFailCount));
    //
    //prefixIPAddr =  (uint32_t*) malloc (4* sizeof(uint32_t) * prefixIPCount);
    //prefixIPMask =  (uint32_t*) malloc (sizeof(uint32_t) * prefixIPCount);
    //
    //if ((!prefixIPAddr) || (!prefixIPMask))
    //{
    //    printf ("Route Memory Allocation Failed \n");
    //    fclose(routeFile);
    //    return XP_ERR_INIT;
    //}
    //memset (prefixIPAddr, 0, 4* sizeof(uint32_t) * prefixIPCount);
    //memset (prefixIPMask, 0, sizeof(uint32_t) * prefixIPCount);
    //
    //uint32_t count = 0;
    //uint32_t prefixWithInSubtrieCount[32];
    //uint32_t prefixWithSubTrieCountt[32];

    //memset (prefixWithInSubtrieCount, 0, (sizeof(prefixWithInSubtrieCount)));
    //memset (prefixWithSubTrieCountt, 0, (sizeof(prefixWithSubTrieCountt)));

    //char prefixStr[33];
    //int enablePrint = 0;

    //zubin: restrict the scope of this macro.
    ////#define XP_ASCII_TO_HEX(CHAR, HEX)
    //    //if ((CHAR >= '0') && (CHAR <= '9')) HEX = (CHAR - '0');
    //    //else if ((CHAR >= 'a') && (CHAR <= 'f')) HEX = (CHAR - 0x57);
    //    //else if ((CHAR >= 'A') && (CHAR <= 'F')) HEX = (CHAR - 0x37);

    //while(count < prefixIPCount)
    //{
    //    memset(prefixStr, 0x0, 33);
    //    fscanf(routeFile,"%s %u", prefixStr, (prefixIPMask + count));

    //    char *str1 = prefixStr;
    //    char *str2 = str1+1;
    //  unsigned int i;
    //    for (i = 0; i < 16; i++)
    //    {
    //        //*str1 = ((*str1 - '0') & 0xF);
    //        //*str2 = ((*str2 - '0') & 0xF);

    //        XP_ASCII_TO_HEX(*str1, *str1);
    //        XP_ASCII_TO_HEX(*str2, *str2);

    //        *((uint8_t*) (prefixIPAddr+(4*count)) + 15 - i) = (*str1 << 4) | *str2;
    //        //*((uint8_t*) (prefixIPAddr+ (4*count)) + i) = (*str1 << 4) | *str2;

    //        if (enablePrint)
    //        {
    //            if (i==0)
    //            {
    //                //printf("[%.6d] %.2x", count, *((uint8_t*) (prefixIPAddr+(4*count)) + 16 - i));
    //                printf("[%.6d] %.2x", count, *((uint8_t*) (prefixIPAddr+(4*count)) + i));
    //            }
    //            else
    //            {
    //                //printf("%.2x", *((uint8_t*) (prefixIPAddr+(4*count)) + 16 - i));
    //                printf("%.2x", *((uint8_t*) (prefixIPAddr+(4*count)) + i));
    //            }
    //            if (i && (i%2) && (i<15)) printf(":");
    //        }

    //        str1+=2;
    //        str2+=2;
    //    }
    //    if (enablePrint) printf("/%d\n", *(prefixIPMask+count));
    //    count++;
    //}

    //if (prefixIPCount != count)
    //{
    //    printf("Warning: Invalid parameter for max prefixes and number of prefixes read\n");
    //}
    //fclose(routeFile);

    //printf("Total count read (%d) and actual prefixes/netMask read (%d)\n", prefixIPCount, count);
    ///*************************************************
    // * FL APIs/Methods
    // *************************************************/
    //prefixIPCount = count;
    //status = xpAllocateNextHop(ecmpSize, &nhIdx);
    //if (status)
    //{
    //    printf("NH allocation failed\n");
    //    free(prefixIPAddr);
    //    free(prefixIPMask);
    //    return XP_ERR_INIT;
    //}

    //Float sCnt=0, fCnt=0;
    //for (count = 0; count < prefixIPCount; count++)
    //{
    //    prefixHandle = 0;
    //    //xpLpmPrefixEntry prefixEntry(XP_PREFIX_TYPE_IPV6, 0xFFFF, (uint8_t*) (prefixIPAddr+(4*count)));
    //    //prefixEntry.print();

    //    ////status = ipv6RouteMgr->addEntry(devId, &prefixEntry, prefixIPMask[count], ecmpSize, nhIdx, prefixHandle);
    //    //for (unsigned int i = 0; i < 16; i++)
    //    //{
    //    //    if (i==0)
    //    //    {
    //    //        //printf("[%.6d] %.2x", count, *((uint8_t*) (prefixIPAddr+(4*count)) + 16 - i));
    //    //        printf("[%.6d] %.2x", count, *((uint8_t*) (prefixIPAddr+(4*count)) + i));
    //    //    }
    //    //    else
    //    //    {
    //    //        //printf("%.2x", *((uint8_t*) (prefixIPAddr+(4*count)) + 16 - i));
    //    //        printf("%.2x", *((uint8_t*) (prefixIPAddr+(4*count)) + i));
    //    //    }
    //    //    if (i && (i%2) && (i<15)) printf(":");
    //    //}
    //    //printf("/%d\n", *(prefixIPMask+count));
    //    //

    //    status = xpIpv6RouteAddRoute(devId,
    //                                                  vrfId,
    //                                                  (uint8_t*) (prefixIPAddr+(4*count)),
    //                                                  *(prefixIPMask + count),
    //                                                  ecmpSize,
    //                                                  nhIdx,
    //                                                  &prefixHandle, &isNewSubTrie);
    //    if (status)
    //    {
    //        //cout << "Prefix Entry Add Failure " << endl;
    //        fCnt++;

    //        if (status == XP_ERR_RESOURCE_NOT_AVAILABLE)
    //        {
    //            prefixFullCount[(*(prefixIPMask + count)) - 1]++;
    //        }
    //        if (status == XP_ERR_OUT_OF_MEM)
    //        {

    //            subTrieFailCount[(*(prefixIPMask + count)) - 1]++;
    //        }
    //    }
    //    else
    //    {
    //        //cout << " Prefix Add Sucessful with Handle : " << prefixHandle << endl;
    //        sCnt++;
    //    }
    //}

    //printf("Num\t\t\tTread\t\t\tStride\t\t\t#SubTries\t\t\t#Routes\t\t\tRouteFail\t\t\tHashColl\n");

    //uint32_t tread, subTrieCnt, prfxCnt, prfxNhiFullCnt, hashCollCnt;
    //uint32_t cCntTotal=0, fCntTotal=0, subTrieCount = 0;
    //uint8_t i;
    //for (i = 0; i < 32; i++)
    //{
    //    tread = subTrieCnt = prfxCnt = prfxNhiFullCnt = hashCollCnt = 0xFFFFFFFF;
    //    if (!(xpIpv6RouteGetPrefixProfileInfo(devId, i, &tread, &subTrieCnt, &prfxCnt, &prfxNhiFullCnt, &hashCollCnt)))
    //    {
    //        printf("%3d\t\t\t%5d\t\t\t%6d\t\t\t%9d\t\t\t%7d\t\t\t%9d\t\t\t%8d\n",
    //                i,
    //                tread - 16,
    //                0,
    //                subTrieCnt,
    //                prfxCnt,
    //                prfxNhiFullCnt,
    //                hashCollCnt);

    //        cCntTotal += prfxCnt;
    //        fCntTotal += prfxNhiFullCnt;
    //        subTrieCount += subTrieCnt;
    //    }
    //}

    //printf("Prefix\t\t\tSubTrieFail\t\t\tSubTrieNhiFail\t\t\tPrefixEntryFail\n");
    //printf("Num    \t\t\t(No SubTrie)\t\t\t(No Nhi)    \t\t\t(No Leaf)\n");

    //for (i = 0; i < 128; i++)
    //{
    //    printf("%6d\t\t\t%6u\t\t\t\t%6u\t\t\t\t%6u\n", (i + 1), subTrieFailCount[i], subTrieNhiFailCount[i], prefixFullCount[i]);
    //}
    //
    //printf ("Total Sub Trie Count : %u \n", subTrieCount);
    //if (cCntTotal != sCnt)
    //{
    //    printf ("WARNING!!! Cumulative Sucess Count Not Matching Overall Sucess Count. Something Wrong !!!" );
    //}

    //printf ("Cummulative Sucess Count : %u Overall Sucess Count: %.0f \n", cCntTotal, sCnt);
    //
    //printf ("Total Fail  Count : %.0f Fail (NHI Full): %u \n", fCnt, fCntTotal);

    //printf("\n\n");

    //printf("LPM STATS: (Total: %u); Success(%.0f), Failure(%.0f);\n", prefixIPCount, sCnt, fCnt);
    //printf("           (Success percentage: %3.4f);\n", ((Float)( (100*sCnt)/(sCnt+fCnt))));

    //free(prefixIPAddr);
    //free(prefixIPMask);

    return XP_NO_ERR;
}

XP_STATUS xpsAppL3UcNhProgramming(xpDevice_t devId)
{
#define NHPKTCMD 0x3
#define NHMIRRMASK 0x0
#define NHDECTTL 0x1
#define NHNS 0x1
#define NHCNTMODE 0x2
#define NHEGRVIF 0x1234
#define NHMACSALSB 0x5678
#define NHISTAG 0x1
#define NHCVID 0x12
#define NHSVID 0x23
#define NHECMP 0x3
#if 0
    XP_STATUS status = XP_NO_ERR;
    xpNh_t *nhData = (xpNh_t*) malloc(sizeof(xpNh_t));
    nhData->pktCmd = (xpPktCmd_e) NHPKTCMD;
    nhData->mirrorMask = NHMIRRMASK;
    nhData->decTtl = NHDECTTL;
    nhData->networkScope = NHNS;
    nhData->countMode = (xpCountMode_e) NHCNTMODE;
    nhData->egressVif = NHEGRVIF;
    //nhData->macDa[] = { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff} ;
    //memset(&nhData->macDa, 0x01, sizeof(macAddr_t));
    nhData->macSaLsb = (uint8_t)NHMACSALSB & 0xFF;
    //    nhData->macSaLsb[1] = (uint8_t)NHMACSALSB >> 8;
    nhData->isTagged = NHISTAG;
    nhData->cTagVid = NHCVID;
    nhData->virtualId = NHSVID;
    nhData->layer2Ecmp = NHECMP;

    xpNh_t nhDataRead;
    memset(&nhDataRead, 0x0, sizeof(xpNh_t));

    unsigned int i;
    for (i = 0; i < (16*1024); i++)
    {
        status = xpIpv4RouteMgr::instance()->setNh(devId, i, *nhData);
        if (status)
        {
            return status;
        }

        memset(&nhDataRead, 0xFF, sizeof(xpNh_t));
        status = xpIpv4RouteMgr::instance()->getNh(devId, i, nhDataRead);
        if (status)
        {
            return status;
        }

        if ((nhDataRead.pktCmd != NHPKTCMD)
            || (nhDataRead.mirrorMask != NHMIRRMASK)
            || (nhDataRead.decTtl != NHDECTTL)
            || (nhDataRead.networkScope != NHNS)
            || (nhDataRead.countMode != NHCNTMODE)
            || (nhDataRead.egressVif != NHEGRVIF)
            || ((memcmp(nhDataRead.macDa, nhData->macDa, sizeof(macAddr_t))) != 0)
            || (nhDataRead.macSaLsb != (uint8_t)(NHMACSALSB & 0xFF))
            || (nhDataRead.isTagged != NHISTAG)
            || (nhDataRead.cTagVid != NHCVID)
            || (nhDataRead.virtualId != NHSVID)
            || (nhDataRead.layer2Ecmp != NHECMP))
        {
            printf("ERROR: \n");
            nhData->printEntry();;
            nhDataRead.printEntry();
        }
        //printf("comparing nhData: %d, %s\n", i, (status)?"FAILURE":"SUCCESS");
    }
    printf("Successfully wrote and read 0x%x NH entries\n", i);



#define NH_PKTCMD_VAL 0x3
    for (i = 0; i < (16*1024); i++)
    {
        status = xpIpv4RouteMgr::instance()->setNhPktCmd(devId, i,
                                                         (xpPktCmd_e)(NH_PKTCMD_VAL));
        if (status)
        {
            printf("Error setPktCmd for NH(0x%x)\n", i);
            return status;
        }
    }
    printf("Successfully wrote pktCmd for 0x%x NH entries\n", i);


    for (i = 0; i < (16*1024); i++)
    {
        xpPktCmd_e pktCmd;
        status = xpIpv4RouteMgr::instance()->getNhPktCmd(devId, i, pktCmd);
        if (status)
        {
            printf("Error setPktCmd for NH(0x%x)\n", i);
            return status;
        }
        if (pktCmd != NH_PKTCMD_VAL)
        {
            printf("error reading packet command for nhid: 0x%x", i);
        }
    }
    printf("Successfully read pktCmd for 0x%x NH entries\n", i);

    for (int i = 0; i < 8; i++)
    {
        uint32_t mtuValue = (16383/(i+1));
        status = xpIpMtuProfileMgr::instance()->setMtuProfile(devId, i, mtuValue);
        if (status)
        {
            printf("error writing profile value in urw \n");
            return status;
        }
        //printf("writing of ip mtu profile value: [%d] %x (%d)\n", i, mtuValue, mtuValue);
    }

    for (int i = 0; i < 8; i++)
    {
        uint32_t mtuValue=0;
        status = xpIpMtuProfileMgr::instance()->getMtuProfile(devId, i, mtuValue);
        if (status)
        {
            printf("error reading profile value in urw \n");
            return status;
        }
        //printf("reading of ip mtu profile value: [%d] %x (%d)\n", i, mtuValue, mtuValue);
    }
    free(nhData);
    return status;
#endif
    return XP_NO_ERR;
}
