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
* @file prvCpssDxChBrgFdbManager_debug.c
*
* @brief FDB manager support - debug functions
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_utils.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_db.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_hw.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_debug.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

static void prvCpssDxChFdbManagerDebugPrintValidEntries_start(
    IN GT_U32   withMetadata
)
{
    cpssOsPrintf("-- start print DB --- [%s metadata]\n",
        (char*)(withMetadata ? "with" : "without"));
}

static void prvCpssDxChFdbManagerDebugPrintValidEntries_end(void)
{
    cpssOsPrintf("-- ended print DB --- \n");
}

static void prvCpssDxChFdbManagerDebugPrintValidEntries_iterator_dstInterface(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr,
    IN CPSS_INTERFACE_TYPE_ENT                          dstInterface_type,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INTERFACE_INFO_UNT *dstInterfacePtr
)
{

    switch(dstInterface_type)
    {
        case CPSS_INTERFACE_PORT_E :
            if(GT_TRUE == prvCpssDxChFdbManagerDbIsGlobalEport(fdbManagerPtr,
                dstInterfacePtr->devPort.portNum))
            {
                PRINT_s_FIELD_MAC(g{);/*global eport*/
            }
            else
            {
                PRINT_s_FIELD_MAC(p{);
            }
            PRINT_x_FIELD_MAC(dstInterfacePtr->devPort.hwDevNum);
            PRINT_s_FIELD_MAC(-);
            PRINT_x_FIELD_MAC(dstInterfacePtr->devPort.portNum);
            PRINT_s_FIELD_MAC(});
            break;
        case CPSS_INTERFACE_TRUNK_E:
            PRINT_s_FIELD_MAC(t);
            PRINT_x_FIELD_MAC(dstInterfacePtr->trunkId);
            break;
        case CPSS_INTERFACE_VIDX_E :
            PRINT_s_FIELD_MAC(x);
            PRINT_x_FIELD_MAC(dstInterfacePtr->vidx);
            break;
        case CPSS_INTERFACE_VID_E  :
            PRINT_s_FIELD_MAC(v --);
            break;
        default:
            break;
    }
}

static void prvCpssDxChFdbManagerDebugPrintValidEntries_iterator_macAddr(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr,
    IN GT_BOOL                                                printHeader,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC *dbEntryPtr
)
{
    if(printHeader == GT_TRUE)
    {
        cpssOsPrintf(" -- mac Address -- ");
        PRINT_s_FIELD_MAC(v);
        PRINT_s_FIELD_MAC(k);    /*skip*/
        PRINT_s_FIELD_MAC(a);    /*age*/  PRINT_space_MAC;
        PRINT_s_FIELD_MAC(fid);           PRINT_space_MAC;
        PRINT_s_FIELD_MAC(vid1);          PRINT_space_MAC;
        PRINT_s_FIELD_MAC(srcId);         PRINT_space_MAC;
        PRINT_s_FIELD_MAC(dC);   /*daCommand*/
        PRINT_s_FIELD_MAC(sC);   /*saCommand*/
        PRINT_s_FIELD_MAC(dL);   /*daAccessLevel*/
        PRINT_s_FIELD_MAC(sL);   /*saAccessLevel*/
        PRINT_s_FIELD_MAC(s);    /*isStatic*/
        PRINT_s_FIELD_MAC(r);    /*daRoute*/
        PRINT_s_FIELD_MAC(sp);   /*spUnknown*/               PRINT_space_MAC;
        PRINT_s_FIELD_MAC(udb);  /*userDefined*/             PRINT_space_MAC;
        PRINT_s_FIELD_MAC(c);   /*appSpecificCpuCode*/
        PRINT_s_FIELD_MAC(dstInterface); /* dstInterface */

        return;
    }

    cpssOsPrintf("[%4.4x%8.8x] ",
        dbEntryPtr->macAddr_high_16,
        dbEntryPtr->macAddr_low_32);
    PRINT_d_FIELD_MAC(dbEntryPtr->valid);
    PRINT_d_FIELD_MAC(dbEntryPtr->skip);
    PRINT_d_FIELD_MAC(dbEntryPtr->age);
    PRINT_3x_FIELD_MAC(dbEntryPtr->fid);
    PRINT_3x_FIELD_MAC(dbEntryPtr->vid1);
    PRINT_3x_FIELD_MAC(dbEntryPtr->srcId);          PRINT_space_MAC;
    PRINT_d_FIELD_MAC(dbEntryPtr->daCommand);       PRINT_space_MAC;
    PRINT_d_FIELD_MAC(dbEntryPtr->saCommand);       PRINT_space_MAC;
    PRINT_d_FIELD_MAC(dbEntryPtr->daAccessLevel);   PRINT_space_MAC;
    PRINT_d_FIELD_MAC(dbEntryPtr->saAccessLevel);   PRINT_space_MAC;
    PRINT_d_FIELD_MAC(dbEntryPtr->isStatic);
    PRINT_d_FIELD_MAC(dbEntryPtr->daRoute);
    PRINT_d_FIELD_MAC(dbEntryPtr->spUnknown);       PRINT_space_MAC;

    PRINT_3x_FIELD_MAC(dbEntryPtr->userDefined);
    PRINT_d_FIELD_MAC(dbEntryPtr->appSpecificCpuCode);

    prvCpssDxChFdbManagerDebugPrintValidEntries_iterator_dstInterface(
        fdbManagerPtr,
        dbEntryPtr->dstInterface_type,
        &dbEntryPtr->dstInterface);
}

static void prvCpssDxChFdbManagerDebugPrintValidEntries_iterator_ipMc(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr,
    IN GT_BOOL                                                printHeader,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC *dbEntryPtr
)
{
    if(printHeader == GT_TRUE)
    {
        cpssOsPrintf(" -- SIP MC Address -- DIP MC Address -- ");
        PRINT_s_FIELD_MAC(v);    /*valid*/
        PRINT_s_FIELD_MAC(k);    /*skip*/
        PRINT_s_FIELD_MAC(a);    /*age*/  PRINT_space_MAC;
        PRINT_s_FIELD_MAC(fid);           PRINT_space_MAC;
        PRINT_s_FIELD_MAC(srcId);         PRINT_space_MAC;
        PRINT_s_FIELD_MAC(dC);   /*daCommand*/
        PRINT_s_FIELD_MAC(dL);   /*daAccessLevel*/
        PRINT_s_FIELD_MAC(s);    /*isStatic*/
        PRINT_s_FIELD_MAC(r);    /*daRoute*/
        PRINT_s_FIELD_MAC(sp);   /*spUnknown*/               PRINT_space_MAC;
        PRINT_s_FIELD_MAC(udb);  /*userDefined*/             PRINT_space_MAC;
        PRINT_s_FIELD_MAC(c);   /*appSpecificCpuCode*/
        PRINT_s_FIELD_MAC(dstInterface); /* dstInterface */

        return;
    }
    PRINT_x_FIELD_MAC(dbEntryPtr->sipAddr);
    PRINT_x_FIELD_MAC(dbEntryPtr->dipAddr);
    PRINT_d_FIELD_MAC(dbEntryPtr->valid);
    PRINT_d_FIELD_MAC(dbEntryPtr->skip);
    PRINT_d_FIELD_MAC(dbEntryPtr->age);
    PRINT_3x_FIELD_MAC(dbEntryPtr->fid);
    PRINT_3x_FIELD_MAC(dbEntryPtr->srcId);          PRINT_space_MAC;
    PRINT_d_FIELD_MAC(dbEntryPtr->daCommand);       PRINT_space_MAC;
    PRINT_d_FIELD_MAC(dbEntryPtr->daAccessLevel);   PRINT_space_MAC;
    PRINT_d_FIELD_MAC(dbEntryPtr->isStatic);
    PRINT_d_FIELD_MAC(dbEntryPtr->daRoute);
    PRINT_d_FIELD_MAC(dbEntryPtr->spUnknown);       PRINT_space_MAC;

    PRINT_3x_FIELD_MAC(dbEntryPtr->userDefined);
    PRINT_d_FIELD_MAC(dbEntryPtr->appSpecificCpuCode);

    prvCpssDxChFdbManagerDebugPrintValidEntries_iterator_dstInterface(
        fdbManagerPtr,
        dbEntryPtr->dstInterface_type,
        &dbEntryPtr->dstInterface);
}

static void prvCpssDxChFdbManagerDebugPrintValidEntries_iterator_ipv4Uc(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr,
    IN GT_BOOL                                                printHeader,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV4_UC_FORMAT_STC  *dbEntryPtr
)
{
    if(printHeader == GT_TRUE)
    {
        cpssOsPrintf(" IPv4 UC Addr ");
        PRINT_s_FIELD_MAC(vrf);
        PRINT_s_FIELD_MAC(v);
        PRINT_s_FIELD_MAC(k);    /*skip*/
        PRINT_s_FIELD_MAC(a);    /*age*/

        PRINT_s_FIELD_MAC(type PTR);  /*ucRouteExtType , pointerInfo*/

        PRINT_s_FIELD_MAC(dec); /*ttlHopLimitDecEnable*/
        PRINT_s_FIELD_MAC(bp);  /*ttlHopLimDecOptionsExtChkByPass*/
        PRINT_s_FIELD_MAC(ic);  /*ICMPRedirectEnable*/
        PRINT_s_FIELD_MAC(mt);  /*mtuProfileIndex*/
        PRINT_s_FIELD_MAC(cs);  /*countSet*/
        PRINT_s_FIELD_MAC(dstInterface); /* dstInterface */

        return;
    }


    PRINT_x_FIELD_MAC(dbEntryPtr->ipAddr);
    PRINT_x_FIELD_MAC(dbEntryPtr->ipUcCommonInfo.vrfId);

    PRINT_d_FIELD_MAC(dbEntryPtr->ipUcCommonInfo.valid);
    PRINT_d_FIELD_MAC(dbEntryPtr->ipUcCommonInfo.skip);
    PRINT_d_FIELD_MAC(dbEntryPtr->ipUcCommonInfo.age);


    switch(dbEntryPtr->ipUcCommonInfo.ucRouteExtType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E:
            PRINT_s_FIELD_MAC(TS);
            PRINT_x_FIELD_MAC(dbEntryPtr->ipUcCommonInfo.pointerInfo);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E         :
            PRINT_s_FIELD_MAC(NAT);
            PRINT_x_FIELD_MAC(dbEntryPtr->ipUcCommonInfo.pointerInfo);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E         :
            PRINT_s_FIELD_MAC(ARP);
            PRINT_x_FIELD_MAC(dbEntryPtr->ipUcCommonInfo.pointerInfo);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E                :
            PRINT_s_FIELD_MAC(R-NH);
            PRINT_x_FIELD_MAC(dbEntryPtr->ipUcCommonInfo.pointerInfo);
            return;/* other fields are not valid */
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E                    :
            PRINT_s_FIELD_MAC(R-ECMP);
            PRINT_x_FIELD_MAC(dbEntryPtr->ipUcCommonInfo.pointerInfo);
            return;/* other fields are not valid */
        default:
            break;
    }

    PRINT_x_FIELD_MAC(dbEntryPtr->ipUcCommonInfo.nextHopVlanId);

    PRINT_d_FIELD_MAC(dbEntryPtr->ipUcCommonInfo.ttlHopLimitDecEnable);
    PRINT_d_FIELD_MAC(dbEntryPtr->ipUcCommonInfo.ttlHopLimDecOptionsExtChkByPass);
    PRINT_d_FIELD_MAC(dbEntryPtr->ipUcCommonInfo.ICMPRedirectEnable);
    PRINT_d_FIELD_MAC(dbEntryPtr->ipUcCommonInfo.mtuProfileIndex);
    PRINT_d_FIELD_MAC(dbEntryPtr->ipUcCommonInfo.countSet);

    prvCpssDxChFdbManagerDebugPrintValidEntries_iterator_dstInterface(
        fdbManagerPtr,
        dbEntryPtr->ipUcCommonInfo.dstInterface_type,
        &dbEntryPtr->ipUcCommonInfo.dstInterface);
}

static void prvCpssDxChFdbManagerDebugPrintValidEntries_iterator_ipv6Uc(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr,
    IN GT_BOOL                                                printHeader,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_DATA_FORMAT_STC  *dbEntryDataPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_KEY_FORMAT_STC   *dbEntryKeyPtr
)
{
    GT_U8 ipv6Addr[16];

    if(printHeader == GT_TRUE)
    {
        cpssOsPrintf(" IPv6 UC Addr                     ");
        PRINT_s_FIELD_MAC(vrf);
        PRINT_s_FIELD_MAC(v);    /*valid*/
        PRINT_s_FIELD_MAC(k);    /*skip*/
        PRINT_s_FIELD_MAC(a);    /*age*/

        PRINT_s_FIELD_MAC(type PTR);  /*ucRouteExtType , pointerInfo*/

        PRINT_s_FIELD_MAC(dec); /*ttlHopLimitDecEnable*/
        PRINT_s_FIELD_MAC(bp);  /*ttlHopLimDecOptionsExtChkByPass*/
        PRINT_s_FIELD_MAC(ic);  /*ICMPRedirectEnable*/
        PRINT_s_FIELD_MAC(mt);  /*mtuProfileIndex*/
        PRINT_s_FIELD_MAC(cs);  /*countSet*/
        PRINT_s_FIELD_MAC(dstInterface); /* dstInterface */

        return;
    }

    ipv6Addr[ 0] =  (dbEntryDataPtr->ipAddr_127_106 >> 14) & 0xFF;
    ipv6Addr[ 1] =  (dbEntryDataPtr->ipAddr_127_106 >>  6) & 0xFF;
    ipv6Addr[ 2] = ((dbEntryDataPtr->ipAddr_127_106) & 0x3F) | ((dbEntryKeyPtr->ipAddr_105_96 >> 8) & 0x3);
    ipv6Addr[ 3] =  (dbEntryKeyPtr->ipAddr_105_96 & 0xFF);
    ipv6Addr[ 4] =  (dbEntryKeyPtr->ipAddr_95_64 >> 24) & 0xFF;
    ipv6Addr[ 5] =  (dbEntryKeyPtr->ipAddr_95_64 >> 16) & 0xFF;
    ipv6Addr[ 6] =  (dbEntryKeyPtr->ipAddr_95_64 >>  8) & 0xFF;
    ipv6Addr[ 7] =  (dbEntryKeyPtr->ipAddr_95_64 >>  0) & 0xFF;
    ipv6Addr[ 8] =  (dbEntryKeyPtr->ipAddr_63_32 >> 24) & 0xFF;
    ipv6Addr[ 9] =  (dbEntryKeyPtr->ipAddr_63_32 >> 16) & 0xFF;
    ipv6Addr[10] =  (dbEntryKeyPtr->ipAddr_63_32 >>  8) & 0xFF;
    ipv6Addr[11] =  (dbEntryKeyPtr->ipAddr_63_32 >>  0) & 0xFF;
    ipv6Addr[12] =  (dbEntryKeyPtr->ipAddr_31_0  >> 24) & 0xFF;
    ipv6Addr[13] =  (dbEntryKeyPtr->ipAddr_31_0  >> 16) & 0xFF;
    ipv6Addr[14] =  (dbEntryKeyPtr->ipAddr_31_0  >>  8) & 0xFF;
    ipv6Addr[15] =  (dbEntryKeyPtr->ipAddr_31_0  >>  0) & 0xFF;

    PRINT_IPV6_ADDRESS_MAC(ipv6Addr);
    PRINT_3x_FIELD_MAC(dbEntryDataPtr->ipUcCommonInfo.vrfId);

    PRINT_d_FIELD_MAC(dbEntryDataPtr->ipUcCommonInfo.valid);
    PRINT_d_FIELD_MAC(dbEntryDataPtr->ipUcCommonInfo.skip);
    PRINT_d_FIELD_MAC(dbEntryDataPtr->ipUcCommonInfo.age);


    switch(dbEntryDataPtr->ipUcCommonInfo.ucRouteExtType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E:
            PRINT_s_FIELD_MAC(TS);
            PRINT_x_FIELD_MAC(dbEntryDataPtr->ipUcCommonInfo.pointerInfo);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E         :
            PRINT_s_FIELD_MAC(NAT);
            PRINT_x_FIELD_MAC(dbEntryDataPtr->ipUcCommonInfo.pointerInfo);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E         :
            PRINT_s_FIELD_MAC(ARP);
            PRINT_x_FIELD_MAC(dbEntryDataPtr->ipUcCommonInfo.pointerInfo);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E                :
            PRINT_s_FIELD_MAC(R-NH);
            PRINT_x_FIELD_MAC(dbEntryDataPtr->ipUcCommonInfo.pointerInfo);
            return;/* other fields are not valid */
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E                    :
            PRINT_s_FIELD_MAC(R-ECMP);
            PRINT_x_FIELD_MAC(dbEntryDataPtr->ipUcCommonInfo.pointerInfo);
            return;/* other fields are not valid */
        default:
            break;
    }

    PRINT_x_FIELD_MAC(dbEntryDataPtr->ipUcCommonInfo.nextHopVlanId);

    PRINT_d_FIELD_MAC(dbEntryDataPtr->ipUcCommonInfo.ttlHopLimitDecEnable);
    PRINT_d_FIELD_MAC(dbEntryDataPtr->ipUcCommonInfo.ttlHopLimDecOptionsExtChkByPass);
    PRINT_d_FIELD_MAC(dbEntryDataPtr->ipUcCommonInfo.ICMPRedirectEnable);
    PRINT_d_FIELD_MAC(dbEntryDataPtr->ipUcCommonInfo.mtuProfileIndex);
    PRINT_d_FIELD_MAC(dbEntryDataPtr->ipUcCommonInfo.countSet);

    prvCpssDxChFdbManagerDebugPrintValidEntries_iterator_dstInterface(
        fdbManagerPtr,
        dbEntryDataPtr->ipUcCommonInfo.dstInterface_type,
        &dbEntryDataPtr->ipUcCommonInfo.dstInterface);

    PRINT_d_FIELD_MAC(dbEntryDataPtr->ipv6DestSiteId);
    PRINT_d_FIELD_MAC(dbEntryDataPtr->ipv6ScopeCheck);
}

static void prvCpssDxChFdbManagerDebugPrintValidEntries_iterator_metadata(
    IN GT_BOOL                                         printHeader,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr
)
{
    GT_U32  nextEntryPointer;

    if(printHeader == GT_TRUE)
    {
        cpssOsPrintf("    ");
        PRINT_s_FIELD_MAC(hwId);    PRINT_3space_MAC;
        PRINT_s_FIELD_MAC(ageBin);  PRINT_3space_MAC;
        PRINT_s_FIELD_MAC(prev);    PRINT_3space_MAC;
        PRINT_s_FIELD_MAC(next);    PRINT_3space_MAC;
        PRINT_s_FIELD_MAC(aPrev);   PRINT_3space_MAC;
        PRINT_s_FIELD_MAC(aNext);
        return;
    }

    PRINT_index_FIELD_MAC(dbEntryPtr->hwIndex);
    PRINT_index_FIELD_MAC(dbEntryPtr->ageBinIndex);
    if(dbEntryPtr->isValid_prevEntryPointer)
    {
        PRINT_index_FIELD_MAC(dbEntryPtr->prevEntryPointer);
    }
    else
    {
        PRINT_NA_PTR;
    }

    if(dbEntryPtr->isValid_nextEntryPointer)
    {
        GET_nextEntryPointer_MAC(dbEntryPtr,nextEntryPointer);
        PRINT_index_FIELD_MAC(nextEntryPointer);
    }
    else
    {
        PRINT_NA_PTR;
    }

    if(dbEntryPtr->isValid_age_prevEntryPointer)
    {
        PRINT_index_FIELD_MAC(dbEntryPtr->age_prevEntryPointer);
    }
    else
    {
        PRINT_NA_PTR;
    }

    if(dbEntryPtr->isValid_age_nextEntryPointer)
    {
        PRINT_index_FIELD_MAC(dbEntryPtr->age_nextEntryPointer);
    }
    else
    {
        PRINT_NA_PTR;
    }
}


static void prvCpssDxChFdbManagerDebugPrintValidEntries_iterator(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr,
    IN GT_U32                                 iterator,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr,
    IN GT_U32                                 withMetadata,
    IN GT_BOOL                                printHeader
)
{
    if(!dbEntryPtr->isUsedEntry)
    {
        cpssOsPrintf("%d - invalid entry (error) \n",iterator);
        return;
    }

    if(printHeader == GT_TRUE)
    {
        cpssOsPrintf("Iter ");
    }

    if (dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E)
    {
        /* special treatment for IpV6 UC entries */
        return;
    }

    switch (dbEntryPtr->hwFdbEntryType)
    {
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E: /*PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC*/
            if(printHeader == GT_FALSE)
            {
                cpssOsPrintf("%3.3d MAC  ",iterator);
            }
            prvCpssDxChFdbManagerDebugPrintValidEntries_iterator_macAddr(fdbManagerPtr,printHeader,&dbEntryPtr->specificFormat.prvMacEntryFormat);
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E : /*PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV4_UC_FORMAT_STC*/
            if(printHeader == GT_FALSE)
            {
                cpssOsPrintf("%3.3d V4UC ",iterator);
            }
            prvCpssDxChFdbManagerDebugPrintValidEntries_iterator_ipv4Uc(fdbManagerPtr,printHeader,&dbEntryPtr->specificFormat.prvIpv4UcEntryFormat);
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            if(printHeader == GT_FALSE)
            {
                cpssOsPrintf("%3.3d V4MC ",iterator);
            }
            prvCpssDxChFdbManagerDebugPrintValidEntries_iterator_ipMc(fdbManagerPtr,printHeader, &dbEntryPtr->specificFormat.prvIpv4McEntryFormat);
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            if(printHeader == GT_FALSE)
            {
                cpssOsPrintf("%3.3d V6MC ",iterator);
            }
            prvCpssDxChFdbManagerDebugPrintValidEntries_iterator_ipMc(fdbManagerPtr,printHeader, &dbEntryPtr->specificFormat.prvIpv6McEntryFormat);
            break;
        default:
            cpssOsPrintf("%d - unknown format of [%d] \n",
                iterator,
                dbEntryPtr->hwFdbEntryType);
            break;
    }

    if(withMetadata)
    {
        prvCpssDxChFdbManagerDebugPrintValidEntries_iterator_metadata(printHeader,dbEntryPtr);
    }

    cpssOsPrintf("\n");
}

static void prvCpssDxChFdbManagerDebugPrintValidIpV6UcEntries_iterator(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr,
    IN GT_U32                                 iterator,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryExtPtr,
    IN GT_U32                                 withMetadata
)
{
    if(!dbEntryPtr->isUsedEntry)
    {
        cpssOsPrintf("%d - invalid entry (error) \n",iterator);
        return;
    }

    cpssOsPrintf("%3.3d V6UC ",iterator);

    prvCpssDxChFdbManagerDebugPrintValidEntries_iterator_ipv6Uc(fdbManagerPtr,GT_FALSE,
                                                                &dbEntryExtPtr->specificFormat.prvIpv6UcDataEntryFormat,
                                                                &dbEntryPtr->specificFormat.prvIpv6UcKeyEntryFormat);

    if(withMetadata)
    {
        prvCpssDxChFdbManagerDebugPrintValidEntries_iterator_metadata(GT_FALSE, dbEntryPtr);
    }

    cpssOsPrintf("\n");
}

/* debug function to print the FDB manager valid entries (from the DB) ...
   no HW access

   fdbManagerId - the manager Id
   withMetadata - indication to print per entry the 'metadata' in addition to 'entry format'
                  0 - print only 'entry format' without 'metadata'
                  1 - print      'entry format' and the 'metadata'
*/
static GT_STATUS internal_prvCpssDxChFdbManagerDebugPrintValidEntries
(
    IN GT_U32   fdbManagerId,
    IN GT_U32   withMetadata,
    IN PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_ENT specificType
)
{
    GT_U32  ii;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryDataPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryKeyPtr;
    GT_BOOL     getFirst = GT_TRUE;
    GT_STATUS   rc;

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    prvCpssDxChFdbManagerDebugPrintValidEntries_start(withMetadata);

    ii = 0;

    /* loop on all entries in the FDB manager */
    while(GT_OK ==
          prvCpssDxChFdbManagerDbFdbEntryIteratorGetNext(fdbManagerPtr,getFirst,&dbEntryPtr))
    {
        getFirst = GT_FALSE;/* get next from here on ... */

        if(specificType != GT_NA)
        {
            if(dbEntryPtr->hwFdbEntryType != (GT_U32)specificType)
            {
                /* skip this entry */
                continue;
            }
        }

        if(ii == 0)
        {
            /* print header */
            prvCpssDxChFdbManagerDebugPrintValidEntries_iterator(fdbManagerPtr,ii,dbEntryPtr,
                withMetadata,
                GT_TRUE/* print header */);
        }

        if (dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E)
        {
            dbEntryDataPtr = dbEntryPtr;
            rc = prvCpssDxChFdbManagerDbFdbEntryIteratorGetNext(fdbManagerPtr,getFirst, &dbEntryPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            dbEntryKeyPtr = dbEntryPtr;
            /* print info on this iteration*/
            prvCpssDxChFdbManagerDebugPrintValidIpV6UcEntries_iterator(fdbManagerPtr,ii, dbEntryDataPtr, dbEntryKeyPtr, withMetadata);
        }
        else
        {
            /* print info on this iteration*/
            prvCpssDxChFdbManagerDebugPrintValidEntries_iterator(fdbManagerPtr,ii,dbEntryPtr,
                withMetadata,
                GT_FALSE/* not to print header */);
        }

        ii++;
    }

    prvCpssDxChFdbManagerDebugPrintValidEntries_end();

    return GT_OK;
}

/* debug function to print the FDB manager valid entries (from the DB) ...
   no HW access

   fdbManagerId - the manager Id
   withMetadata - indication to print per entry the 'metadata' in addition to 'entry format'
                  0 - print only 'entry format' without 'metadata'
                  1 - print      'entry format' and the 'metadata'
*/
GT_STATUS prvCpssDxChFdbManagerDebugPrintValidEntries
(
    IN GT_U32   fdbManagerId,
    IN GT_U32   withMetadata
)
{
    GT_STATUS   rc;

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    rc = internal_prvCpssDxChFdbManagerDebugPrintValidEntries(fdbManagerId,withMetadata,GT_NA);

    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/* debug function to print the FDB manager valid MAC entries (from the DB) ...
   no HW access

   fdbManagerId - the manager Id
*/
GT_STATUS prvCpssDxChFdbManagerDebugPrintValidEntries_mac
(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS   rc;

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    rc = internal_prvCpssDxChFdbManagerDebugPrintValidEntries(fdbManagerId,0,PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E);

    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/* debug function to print the FDB manager valid MAC entries (from the DB) ...
   no HW access

   fdbManagerId - the manager Id
*/
GT_STATUS prvCpssDxChFdbManagerDebugPrintValidEntries_ipV4Mc
(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS   rc;

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    rc = internal_prvCpssDxChFdbManagerDebugPrintValidEntries(fdbManagerId,0,PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E);

    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/* debug function to print the FDB manager valid MAC entries (from the DB) ...
   no HW access

   fdbManagerId - the manager Id
*/
GT_STATUS prvCpssDxChFdbManagerDebugPrintValidEntries_ipV6Mc
(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS   rc;

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    rc = internal_prvCpssDxChFdbManagerDebugPrintValidEntries(fdbManagerId,0,PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E);

    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/* debug function to print the FDB manager valid IPV4UC entries (from the DB) ...
   no HW access

   fdbManagerId - the manager Id
*/
GT_STATUS prvCpssDxChFdbManagerDebugPrintValidEntries_ipv4Uc
(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS   rc;

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    rc = internal_prvCpssDxChFdbManagerDebugPrintValidEntries(fdbManagerId,0,PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E);

    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/* debug function to print the FDB manager valid IPV6UC entries (from the DB) ...
   no HW access

   fdbManagerId - the manager Id
*/
GT_STATUS prvCpssDxChFdbManagerDebugPrintValidEntries_ipv6Uc
(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS   rc;

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    rc = internal_prvCpssDxChFdbManagerDebugPrintValidEntries(fdbManagerId,0,PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E);

    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

static GT_STATUS internal_prvCpssDxChFdbManagerDebugPrintCounters(IN GT_U32   fdbManagerId)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC counters;
    GT_U32    bank;

    cpssOsMemSet(&counters, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC));

    rc = cpssDxChBrgFdbManagerCountersGet(fdbManagerId, &counters);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf(" -- Counters -- ");

    cpssOsPrintf("\n");

    PRINT_8s_FIELD_MAC(mac_uc_d);
    PRINT_8s_FIELD_MAC(mac_mc_d);
    PRINT_8s_FIELD_MAC(mac_mc_d);
    PRINT_8s_FIELD_MAC(ipv4_mc);
    PRINT_8s_FIELD_MAC(ipv6_mc);
    PRINT_8s_FIELD_MAC(ipv4_uc_r);
    PRINT_8s_FIELD_MAC(ipv6_uc_k);
    PRINT_8s_FIELD_MAC(ipv6_uc_d);
    PRINT_8s_FIELD_MAC(used_e);
    PRINT_8s_FIELD_MAC(free_e);

    cpssOsPrintf("\n");

    PRINT_8d_FIELD_MAC(counters.macUnicastDynamic);
    PRINT_8d_FIELD_MAC(counters.macMulticastDynamic);
    PRINT_8d_FIELD_MAC(counters.macMulticastStatic);
    PRINT_8d_FIELD_MAC(counters.ipv4Multicast);
    PRINT_8d_FIELD_MAC(counters.ipv6Multicast);
    PRINT_8d_FIELD_MAC(counters.ipv4UnicastRoute);
    PRINT_8d_FIELD_MAC(counters.ipv6UnicastRouteKey);
    PRINT_8d_FIELD_MAC(counters.ipv6UnicastRouteData);
    PRINT_8d_FIELD_MAC(counters.usedEntries);
    PRINT_8d_FIELD_MAC(counters.freeEntries);

    cpssOsPrintf("\n");

    cpssOsPrintf(" -- Bank Counters -- ");

    cpssOsPrintf("\n");

    PRINT_8s_FIELD_MAC(b0);
    PRINT_8s_FIELD_MAC(b1);
    PRINT_8s_FIELD_MAC(b2);
    PRINT_8s_FIELD_MAC(b3);
    PRINT_8s_FIELD_MAC(b4);
    PRINT_8s_FIELD_MAC(b5);
    PRINT_8s_FIELD_MAC(b6);
    PRINT_8s_FIELD_MAC(b7);
    PRINT_8s_FIELD_MAC(b8);
    PRINT_8s_FIELD_MAC(b9);
    PRINT_8s_FIELD_MAC(b10);
    PRINT_8s_FIELD_MAC(b11);
    PRINT_8s_FIELD_MAC(b12);
    PRINT_8s_FIELD_MAC(b13);
    PRINT_8s_FIELD_MAC(b14);
    PRINT_8s_FIELD_MAC(b15);

    cpssOsPrintf("\n");

    for (bank = 0; bank < CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_BANKS_CNS; bank++)
    {
        PRINT_8d_FIELD_MAC(counters.bankCounters[bank]);
    }

    cpssOsPrintf("\n");

    return GT_OK;
}

static GT_STATUS internal_prvCpssDxChFdbManagerDebugPrintStatistics(IN GT_U32   fdbManagerId)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC statistics;

    cpssOsMemSet(&statistics, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC));

    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf(" -- Statistics -- ");

    cpssOsPrintf("\n");

    PRINT_8s_FIELD_MAC(a_ok_0);
    PRINT_8s_FIELD_MAC(a_ok_1);
    PRINT_8s_FIELD_MAC(a_ok_2);
    PRINT_8s_FIELD_MAC(a_ok_3);
    PRINT_8s_FIELD_MAC(a_ok_4);
    PRINT_8s_FIELD_MAC(a_ok_m);
    PRINT_8s_FIELD_MAC(a_er_i);
    PRINT_8s_FIELD_MAC(a_er_b);
    PRINT_8s_FIELD_MAC(a_er_e);
    PRINT_8s_FIELD_MAC(a_er_f);

    PRINT_8s_FIELD_MAC(d_ok);
    PRINT_8s_FIELD_MAC(d_er_i);
    PRINT_8s_FIELD_MAC(d_er_n);
    PRINT_8s_FIELD_MAC(d_t_ok);
    PRINT_8s_FIELD_MAC(d_t_er);

    cpssOsPrintf("\n");

    PRINT_8d_FIELD_MAC(statistics.entryAddOkRehashingStage0        );
    PRINT_8d_FIELD_MAC(statistics.entryAddOkRehashingStage1        );
    PRINT_8d_FIELD_MAC(statistics.entryAddOkRehashingStage2        );
    PRINT_8d_FIELD_MAC(statistics.entryAddOkRehashingStage3        );
    PRINT_8d_FIELD_MAC(statistics.entryAddOkRehashingStage4        );
    PRINT_8d_FIELD_MAC(statistics.entryAddOkRehashingStageMoreThanFour);
    PRINT_8d_FIELD_MAC(statistics.entryAddErrorInputInvalid        );
    PRINT_8d_FIELD_MAC(statistics.entryAddErrorBadState            );
    PRINT_8d_FIELD_MAC(statistics.entryAddErrorEntryExist          );
    PRINT_8d_FIELD_MAC(statistics.entryAddErrorTableFull           );
    PRINT_8d_FIELD_MAC(statistics.entryDeleteOk                    );
    PRINT_8d_FIELD_MAC(statistics.entryDeleteErrorInputInvalid     );
    PRINT_8d_FIELD_MAC(statistics.entryDeleteErrorNotfound         );
    PRINT_8d_FIELD_MAC(statistics.entryTempDeleteOk                );
    PRINT_8d_FIELD_MAC(statistics.entryTempDeleteErrorInputInvalid );

    cpssOsPrintf("\n");

    return GT_OK;
}

/* debug function to print the FDB manager counters (from the DB) ...
   no HW access

   fdbManagerId - the manager Id
*/
GT_STATUS prvCpssDxChFdbManagerDebugPrintCounters
(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS   rc;

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    rc = internal_prvCpssDxChFdbManagerDebugPrintCounters(fdbManagerId);

    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/* debug function to print the FDB manager counters (from the DB) ...
   no HW access

   fdbManagerId - the manager Id
*/
GT_STATUS prvCpssDxChFdbManagerDebugPrintStatistics
(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS   rc;

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    rc = internal_prvCpssDxChFdbManagerDebugPrintStatistics(fdbManagerId);

    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal prvCpssDxChFdbManagerDbHashBanksStepsCalculate function
* @endinternal
*
* @brief   This function calculates banks steps for the banks in calculated hash.
*
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[inout] calcInfoPtr          - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
static GT_STATUS  prvCpssDxChFdbManagerDbHashBanksStepsCalculate
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr
)
{
    GT_STATUS                   rc;
    GT_U32                      bankId;
    GT_U32                      bankStep = 1;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr, *dbEntryExtPtr;

    for(bankId = 0; bankId < fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes; bankId += bankStep)
    {
        /* Get data base entry - First bank */
        rc = prvCpssDxChBrgFdbManagerDbEntryGet(fdbManagerPtr, bankId, calcInfoPtr, &dbEntryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        if((bankId % 2) == 0)
        {
            /* Get data base entry - Second bank */
            rc = prvCpssDxChBrgFdbManagerDbEntryGet(fdbManagerPtr, bankId+1, calcInfoPtr, &dbEntryExtPtr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        bankStep = 1;
        if(dbEntryPtr && dbEntryExtPtr)
        {
            if(dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E && 
               dbEntryExtPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E)
            {
                bankStep = 2;
            }
        }

        calcInfoPtr->bankStepArr[bankId] = bankStep;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChBrgFdbManagerDbEntryGetByHwIndex function
* @endinternal
*
* @brief   Returns DB entry per bank ID by HW index
*           
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB manager 
* @param[in] hwIndex                - FDB entry HW index
* @param[out] dbEntryPtrPtr         - (pointer to)(pointer to) the entry in memory pool 
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - wrong parameter
*/
static GT_STATUS prvCpssDxChBrgFdbManagerDbEntryGetByHwIndex
(
    IN    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN    GT_U32                                            hwIndex,
    OUT   PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    **dbEntryPtrPtr
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INDEX_STC  *dbIndexPtr = NULL;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC  *dbEntryPtr = NULL;  

    dbIndexPtr = &fdbManagerPtr->indexArr[hwIndex];
    if(dbIndexPtr->isValid_entryPointer)
    {
        dbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbIndexPtr->entryPointer];
    }
    *dbEntryPtrPtr = dbEntryPtr;

    return GT_OK;
}

/* debug function to print MAC addresses from calcInfo structure
   no HW access

   fdbManagerPtr - (pointer to) FDB manager
   calcInfoPtr   - (pointer to) calcInfo structure
   selectedBank  - selected bank
*/
GT_STATUS prvCpssDxChBrgFdbManagerAllBanksAddressesPrint
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr,
    IN  GT_U32                                           selectedBank
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC    calcInfo;
    GT_STATUS                                           rc;
    GT_U32                                              bankId;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                 currentEntry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                 *currentEntryPtr = &currentEntry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT            fdbEntryType;
    GT_U32                                              hwIndex;

    calcInfo = *calcInfoPtr;

    rc = prvCpssDxChFdbManagerDbHashBanksStepsCalculate(fdbManagerPtr, &calcInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    for (bankId = 0; bankId < fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes; bankId += calcInfo.bankStepArr[bankId])
    {
        rc = prvCpssDxChBrgFdbManagerBankCalcInfoGet(fdbManagerPtr, bankId, calcInfo.bankStepArr[bankId], NULL, NULL, &calcInfo);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (calcInfo.dbEntryPtr)
        {
            if (calcInfo.bankStepArr[bankId] > 1)
            {
                /* convert DB format to application format - IPv6 UC */
                rc = prvCpssDxChBrgFdbManagerRestoreEntryIpV6UcFormatParamsFromDb(fdbManagerPtr, calcInfo.dbEntryPtr, calcInfo.dbEntryExtPtr, currentEntryPtr);
                if(rc != GT_OK)
                {
                    return rc;
                }
                fdbEntryType = currentEntryPtr->fdbEntryType;
            }
            else
            {
                if(calcInfo.dbEntryPtr->hwFdbEntryType < PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E)
                {
                    /* convert DB format to application format */
                    rc = prvCpssDxChBrgFdbManagerRestoreEntryFormatParamsFromDb(fdbManagerPtr, calcInfo.dbEntryPtr, currentEntryPtr);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    fdbEntryType = currentEntryPtr->fdbEntryType;
                }
                else
                {
                    if (calcInfo.dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E)
                    {
                        hwIndex = calcInfo.crcMultiHashArr[bankId];
                        rc = prvCpssDxChBrgFdbManagerDbEntryGetByHwIndex(fdbManagerPtr, hwIndex+1, &calcInfo.dbEntryExtPtr);
                        if(rc != GT_OK)
                        {
                            return rc;
                        }
                        /* convert DB format to application format - IPv6 UC */
                        rc = prvCpssDxChBrgFdbManagerRestoreEntryIpV6UcFormatParamsFromDb(fdbManagerPtr, calcInfo.dbEntryPtr, calcInfo.dbEntryExtPtr, currentEntryPtr);
                        if(rc != GT_OK)
                        {
                            return rc;
                        }
                    }

                    fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE__LAST__E;
                }
            }

            switch(fdbEntryType)
            {
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
                    PRINT_IPV6_UC_MAC((currentEntryPtr));
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
                    PRINT_MAC(currentEntryPtr);
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
                    PRINT_IPV4_UC(currentEntryPtr);
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
                    PRINT_IPV4_MC(currentEntryPtr);
                    break;
                default:
                    if(calcInfo.dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E)
                    {
                        PRINT_IPV6_UC_KEY_MAC(currentEntryPtr);
                    }
                    else
                    {
                        cpssOsPrintf("[Bad Format]");
                    }

            }
        }
        else
        {
            if (calcInfo.bankStepArr[bankId] == 1)
            {
                cpssOsPrintf("[--:--:--:--:--:--] ");
            }
            else
            {
                cpssOsPrintf("[---.---.---.---.---.---.---.---.---.---.---.---.---.---.---.---] ");
            }
        }

        if (bankId == selectedBank)
        {
            cpssOsPrintf("bankId[%d]*\n", bankId);
        }
        else
        {
            cpssOsPrintf("bankId[%d]\n", bankId);
        }
    }

    return GT_OK;
}

/* macro to allocate memory and copy from the src */
#define MEM_CALLOC_AND_CPY_MAC(_dst, _src, _sizeInBytes)                 \
   {                                                                     \
       _dst = cpssOsMalloc(_sizeInBytes);                                \
       if(NULL != _dst)                                                  \
       {                                                                 \
           cpssOsMemCpy(_dst, _src, (_sizeInBytes));                     \
       }                                                                 \
   }

/* macro to free memory (associated with the manager) */
#define MEM_FREE_MAC(_pointer)                                          \
    if(_pointer)                                                        \
    {                                                                   \
        FREE_PTR_MAC(_pointer);                                         \
    }

/* debug function to Back up FDB manager memory

   fdbManagerId - FDB Manager instance ID
*/
GT_STATUS prvCpssDxChBrgFdbManagerBackUp
(
    IN GT_U32           fdbManagerId
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC      *fdbManagerPtr;
    GT_U32                                              sizeInBytes;

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /* Clean last backUp */
    if(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr))
    {
        MEM_FREE_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->agingBinInfo.ageBinListPtr);
        MEM_FREE_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->agingBinInfo.level1UsageBitmapPtr);
        MEM_FREE_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->indexArr);
        MEM_FREE_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->entryPoolPtr);
        MEM_FREE_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr));
    }

    /* BackUp to global memory */
    sizeInBytes = sizeof(PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC);
    MEM_CALLOC_AND_CPY_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr), fdbManagerPtr, sizeInBytes);

    /* BackUp ageBinListPtr */
    sizeInBytes = fdbManagerPtr->agingBinInfo.totalAgeBinAllocated * sizeof(PRV_CPSS_DXCH_BRG_FDB_MANAGER_AGING_BIN_STC);
    MEM_CALLOC_AND_CPY_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->agingBinInfo.ageBinListPtr,
            fdbManagerPtr->agingBinInfo.ageBinListPtr,
            sizeInBytes);

    /* BackUp level1UsageBitmapPtr */
    sizeInBytes = sizeof(GT_U32) * (1 + (fdbManagerPtr->agingBinInfo.totalAgeBinAllocated >> 5));
    MEM_CALLOC_AND_CPY_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->agingBinInfo.level1UsageBitmapPtr,
            fdbManagerPtr->agingBinInfo.level1UsageBitmapPtr,
            sizeInBytes);

    /* BackUp indexArr */
    sizeInBytes = fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes*sizeof(PRV_CPSS_DXCH_FDB_MANAGER_DB_INDEX_STC);
    MEM_CALLOC_AND_CPY_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->indexArr, fdbManagerPtr->indexArr, sizeInBytes);

    /* BackUp entryPoolPtr */
    sizeInBytes = fdbManagerPtr->capacityInfo.maxTotalEntries*sizeof(PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC);
    MEM_CALLOC_AND_CPY_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->entryPoolPtr, fdbManagerPtr->entryPoolPtr, sizeInBytes);
    return GT_OK;
}

/* debug function to Verify the current DB mismatch before and after HA is an exceptional scenario or not

   fdbManagerId - FDB Manager instance ID
*/
GT_VOID prvCpssDxChBrgFdbManagerVerifyAfterHa_exceptionScenario
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC     *fdbManagerPtr,
    IN  GT_U32                                              hwIndex,
    OUT GT_BOOL                                            *isExpected
)
{
    GT_U32                                          dbIndex, oldDbIndex;

    *isExpected = GT_FALSE;

    dbIndex         = fdbManagerPtr->indexArr[hwIndex].entryPointer;
    oldDbIndex      = PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->indexArr[hwIndex].entryPointer;

    /* Ignore dstInterface compare in case of vidx = FLOOD_VIDX_CNS
     * Ignore age-bit */
    switch(fdbManagerPtr->entryPoolPtr[dbIndex].hwFdbEntryType)
    {
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E:
            if(fdbManagerPtr->entryPoolPtr[dbIndex].specificFormat.prvMacEntryFormat.dstInterface.vidx == FLOOD_VIDX_CNS)
            {
                PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->entryPoolPtr[oldDbIndex].specificFormat.prvMacEntryFormat.dstInterface_type =
                    fdbManagerPtr->entryPoolPtr[dbIndex].specificFormat.prvMacEntryFormat.dstInterface_type;
            }
            PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->entryPoolPtr[oldDbIndex].specificFormat.prvMacEntryFormat.age =
                fdbManagerPtr->entryPoolPtr[dbIndex].specificFormat.prvMacEntryFormat.age;
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            if(fdbManagerPtr->entryPoolPtr[dbIndex].specificFormat.prvIpv4McEntryFormat.dstInterface.vidx == FLOOD_VIDX_CNS)
            {
                PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->entryPoolPtr[oldDbIndex].specificFormat.prvIpv4McEntryFormat.dstInterface_type =
                    fdbManagerPtr->entryPoolPtr[dbIndex].specificFormat.prvIpv4McEntryFormat.dstInterface_type;
            }
            PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->entryPoolPtr[oldDbIndex].specificFormat.prvIpv4McEntryFormat.age =
                fdbManagerPtr->entryPoolPtr[dbIndex].specificFormat.prvIpv4McEntryFormat.age;
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            if(fdbManagerPtr->entryPoolPtr[dbIndex].specificFormat.prvIpv6McEntryFormat.dstInterface.vidx == FLOOD_VIDX_CNS)
            {
                PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->entryPoolPtr[oldDbIndex].specificFormat.prvIpv6McEntryFormat.dstInterface_type =
                    fdbManagerPtr->entryPoolPtr[dbIndex].specificFormat.prvIpv6McEntryFormat.dstInterface_type;
            }
            PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->entryPoolPtr[oldDbIndex].specificFormat.prvIpv6McEntryFormat.age =
                fdbManagerPtr->entryPoolPtr[dbIndex].specificFormat.prvIpv6McEntryFormat.age;
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E:
            PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->entryPoolPtr[oldDbIndex].specificFormat.prvIpv4UcEntryFormat.ipUcCommonInfo.age =
                fdbManagerPtr->entryPoolPtr[dbIndex].specificFormat.prvIpv4UcEntryFormat.ipUcCommonInfo.age;
            break;

        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E:
            PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->entryPoolPtr[oldDbIndex].specificFormat.prvIpv6UcKeyEntryFormat.age =
                fdbManagerPtr->entryPoolPtr[dbIndex].specificFormat.prvIpv6UcKeyEntryFormat.age;
            break;

        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E:
            PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->entryPoolPtr[oldDbIndex].specificFormat.prvIpv6UcDataEntryFormat.ipUcCommonInfo.age =
                fdbManagerPtr->entryPoolPtr[dbIndex].specificFormat.prvIpv6UcDataEntryFormat.ipUcCommonInfo.age;
            break;

        default:
            return;
    }
    if(cpssOsMemCmp(&fdbManagerPtr->entryPoolPtr[dbIndex].specificFormat,
                &PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->entryPoolPtr[oldDbIndex].specificFormat,
                sizeof(fdbManagerPtr->entryPoolPtr[dbIndex].specificFormat)))
    {
        return;
    }
    *isExpected = GT_TRUE;
    return;
}

/* debug function to Verify the current FDB Manager Instance entries, with the backed-up FDB manager instance
   Support for HA verification & HW corruption (SER) test cases

   fdbManagerId - FDB Manager instance ID
*/
GT_STATUS prvCpssDxChBrgFdbManagerBackUpInstanceVerify
(
    IN GT_U32           fdbManagerId
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC      *fdbManagerPtr;
    GT_U32                                              dbIndex, oldDbIndex;
    GT_U32                                              hwIndex;
    GT_U32                                              sizeInBytes;
    GT_U32                                              countertype;
    GT_U32                                              bankId;
    GT_BOOL                                             isExpected;

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    if (PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr) == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Aging info(Usage bitmap and age-bin ID) verification */
    if(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->agingBinInfo.level2UsageBitmap != fdbManagerPtr->agingBinInfo.level2UsageBitmap)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "FDB Manager HA: Level2 Usage Bitmap recovered wrongly\n");
    }

    sizeInBytes = sizeof(GT_U32) * (1 + (fdbManagerPtr->agingBinInfo.totalAgeBinAllocated >> 5));
    if(cpssOsMemCmp(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->agingBinInfo.level1UsageBitmapPtr,
                fdbManagerPtr->agingBinInfo.level1UsageBitmapPtr,
                sizeInBytes))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "FDB Manager HA: Level1 Usage Bitmap recovered wrongly\n");
    }

    /* Index and entry Pool verification */
    for(hwIndex = 0; hwIndex < fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes; hwIndex++)
    {
        /* Validate entry content in case entry is valid */
        if(fdbManagerPtr->indexArr[hwIndex].isValid_entryPointer)
        {
            dbIndex     = fdbManagerPtr->indexArr[hwIndex].entryPointer;
            oldDbIndex  = PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->indexArr[hwIndex].entryPointer;
            if(cpssOsMemCmp(&fdbManagerPtr->entryPoolPtr[dbIndex].specificFormat,
                        &PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->entryPoolPtr[oldDbIndex].specificFormat,
                        sizeof(fdbManagerPtr->entryPoolPtr[dbIndex].specificFormat)))
            {
                prvCpssDxChBrgFdbManagerVerifyAfterHa_exceptionScenario(fdbManagerPtr, hwIndex, &isExpected);
                if(!isExpected)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "FDB Manager HA not recovered for hwIndex[%d]\n", hwIndex);
                }
            }
        }
        else if(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->indexArr[hwIndex].isValid_entryPointer == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "FDB Manager HA not recovered for hwIndex[%d]\n", hwIndex);
        }
    }

    /* Type Counter verification */
    for(countertype = 0; countertype < PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE__LAST__E; countertype++)
    {
        if(fdbManagerPtr->typeCountersArr[countertype].entryTypePopulation !=
                PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->typeCountersArr[countertype].entryTypePopulation)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "FDB Manager HA: Entry Type Counter recovered wrongly\n");
        }
    }
    /* Bank Counter verification */
    for(bankId = 0; bankId < CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_BANKS_CNS; bankId++)
    {
        if(fdbManagerPtr->bankInfoArr[bankId].bankPopulation !=
                PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->bankInfoArr[bankId].bankPopulation)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "FDB Manager HA: Entry Bank Counter recovered wrongly\n");
        }
    }

    /* Release memory after after verification */
    MEM_FREE_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->agingBinInfo.ageBinListPtr);
    MEM_FREE_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->agingBinInfo.level1UsageBitmapPtr);
    MEM_FREE_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->indexArr);
    MEM_FREE_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr)->entryPoolPtr);
    MEM_FREE_MAC(PRV_SHARED_FDB_MANAGER_DB_VAR_GET(globalFdbManagerPtr));

    return GT_OK;
}

/* debug function to simulate HW corruption

   fdbManagerId - FDB Manager instance ID
   indexesArray - Array of hwIndexs
   indexescount - total number of indexes present in indexesArray
*/
GT_STATUS prvCpssDxChBrgFdbManagerInjectHwCorruption
(
    IN GT_U32           fdbManagerId,
    IN GT_U32           indexesArray[],
    IN GT_U32           indexesCount
)
{
    GT_STATUS                                           rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC      *fdbManagerPtr;
    GT_U8                                               devNum, firstDevNum;
    GT_U32                                              indexNum;
    GT_U32                                              hwData[FDB_ENTRY_WORDS_SIZE_CNS];
    GT_U32                                              portGroupId;
    GT_U32                                              tileId;
    GT_U32                                              numOfTiles;

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_TRUE, &firstDevNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    for(indexNum=0; indexNum<indexesCount; indexNum++)
    {
        portGroupId = 0;
        LOCK_DEV_NUM(firstDevNum);
        rc = prvCpssDxChPortGroupReadTableEntry(firstDevNum,
                portGroupId,
                CPSS_DXCH_TABLE_FDB_E,
                indexesArray[indexNum],
                &hwData[0]);
        UNLOCK_DEV_NUM(firstDevNum);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Toggle 5th bit of first byte in HW Entry */
        hwData[0] ^= (1 << 5);

        /* Write the Corrupted entry to all devices */
        rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_TRUE, &devNum);
        while(rc == GT_OK)
        {
            LOCK_DEV_NUM(devNum);
            numOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles ?
                PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles:1;
            for(tileId = 0; tileId < numOfTiles; tileId++)
            {
                portGroupId = tileId * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;
                rc = prvCpssDxChPortGroupWriteTableEntry(devNum,
                        portGroupId,
                        CPSS_DXCH_TABLE_FDB_E,
                        indexesArray[indexNum],
                        &hwData[0]);
                if(rc != GT_OK)
                {
                    UNLOCK_DEV_NUM(devNum);
                    return rc;
                }
            }
            UNLOCK_DEV_NUM(devNum);
            
            rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_FALSE, &devNum);
        }
    }
    return GT_OK;
}

/* debug function to print entry to be rehashed and bank entries according to calcInfo structure
   no HW access

   fdbManagerPtr        - (pointer to) FDB manager
   currentEntryPtr      - (pointer to) FDB entry to be rehashed
   currentCalcInfoPtr   - (pointer to) calcInfo structure
   totalStages          - number of total rehashed stages
   currentStage         - current rehash stage
   selectedBank         - selected bank
*/
GT_STATUS prvCpssDxChBrgFdbManagerRehashInfoPrint
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     *currentEntryPtr,
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC        *calcInfoPtr,
    IN  GT_U32                                                  currentStage,
    IN  GT_U32                                                  selectedBank
)
{
    GT_STATUS                                                   rc;
    GT_U32                                                      entryHwIndex = 0;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC            currentCalcInfo;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC           *currentCalcInfoPtr = &currentCalcInfo;

    currentCalcInfo = *calcInfoPtr;

    if(currentStage > 0)
    {
        entryHwIndex = fdbManagerPtr->rehashingTraceArray[currentStage-1].entryHwIndex;
    }

    switch(currentEntryPtr->fdbEntryType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
            PRINT_CURRENT_IPV6_UC_BANK_STAGE(currentEntryPtr, selectedBank, currentStage, entryHwIndex);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
            PRINT_CURRENT_MAC_BANK_STAGE(currentEntryPtr, selectedBank, currentStage, entryHwIndex);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
            PRINT_CURRENT_IPV4_MC_BANK_STAGE(currentEntryPtr, selectedBank, currentStage, entryHwIndex);
            break;
        default:
            PRINT_CURRENT_IPV4_UC_BANK_STAGE(currentEntryPtr, selectedBank, currentStage, entryHwIndex);
    }

    PRINT_BANKS_ADDRESSES(currentCalcInfoPtr, selectedBank);

    return GT_OK;
}

/* debug function to Verify the current FDB Manager banks population, with the total FDB Manager banks population.
   Checks only FDB manager data base counters, HW counters are not checked.
*/
GT_STATUS prvCpssDxChBrgFdbManagerBanksPopulationCheck
(
    IN GT_U32                                           fdbManagerId
)
{
    GT_U32  bankId;
    GT_U32  totalPopulation = 0;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    for(bankId = 0; bankId < fdbManagerPtr->numOfBanks; bankId++)
    {
        /* coherency check */
        CHECK_X_NOT_MORE_THAN_Y_MAC(
            fdbManagerPtr->bankInfoArr[bankId].bankPopulation,
            fdbManagerPtr->maxPopulationPerBank);

        totalPopulation += fdbManagerPtr->bankInfoArr[bankId].bankPopulation;
    }

    if(totalPopulation != fdbManagerPtr->totalPopulation)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "FDB Manager inconsistent: total banks population [%d], current banks population\n", 
                                      fdbManagerPtr->totalPopulation, totalPopulation);
    }

    return GT_OK;
}

