/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

//
// file cpssHalCascadeMgr.c
//


#include <stdlib.h>
#include <stdio.h>

#include "xpsCommon.h"
#include <cpssHalProfile.h>

#include <cpssHalSys.h>
#include <cpssHalDev.h>
#include <cpssHalExt.h>


#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/policer/cpssDxChPolicer.h>
#include <cpss/generic/cpssTypes.h>
#include <cpss/extServices/cpssExtServices.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include "cpssHalDevice.h"
#include "cpssHalCascadeMgr.h"

#include <gtOs/gtOsGen.h>
#include <gtExtDrv/drivers/gtIntDrv.h>
#include <gtOs/gtEnvDep.h>

#include <cpssHalInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

GT_STATUS cpssHalCascadeMgrInitCascadeTrunk
(
    uint32_t cscdTrunkId
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     devNum;
    int       portIdx, port;
    int       numberOfCscdPorts = 0;
    GT_16     stpId;
    uint32_t  maxStp = 0;
    /* the device to be reached via cascade port for analyzing */
    GT_U8     targetDevNum;

    /* the HW num for device to be reached via cascade port for analyzing */
    GT_HW_DEV_NUM  targetHwDevNum, sourceHwDevNum;
    CPSS_CSCD_PORT_TYPE_ENT
    cscdPortType; /* Enum of cascade port or network port */
    GT_BOOL             egressAttributesLocallyEn;
    CPSS_PORTS_BMP_STC
    cscdTrunkBmp;                    /* bmp of ports members in the cascade trunk */
    CPSS_CSCD_LINK_TYPE_STC     cpssCascadeLink;
    CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT  currentSrcPortTrunkHashEn;


    rc = cpssDxChCfgTableNumEntriesGet(0, CPSS_DXCH_CFG_TABLE_STG_E, &maxStp);
    if (rc !=GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "[%s:%d]  cpssDxChCfgTableNumEntriesGet: ret[%d]"
              , __func__, __LINE__, rc);

        return rc;
    }

    XPS_DEVICES_PER_SWITCH_ITER(0, devNum)
    {
        cpssHalGetMaxCascadedPorts(devNum, &numberOfCscdPorts);
        if (numberOfCscdPorts == 0)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                  "[%s:%d] - no cascase ports in device. devNum=%u"
                  , __func__, __LINE__, devNum);
            continue;
        }
        /* clear the local ports bmp for the new cascade trunk */
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&cscdTrunkBmp);
        XPS_CSCD_PORT_ITER(devNum, portIdx, port, numberOfCscdPorts)
        {
            /* Configure ports to be cascade */
            cscdPortType = CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E;

            rc = cpssDxChCscdPortTypeSet(devNum, port, CPSS_PORT_DIRECTION_BOTH_E,
                                         cscdPortType);
            if (GT_OK != rc)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "[%s:%d] - cpssDxChCscdPortTypeSet failed. devNum=%u port=%u rc[%d]"
                      , __func__, __LINE__, devNum, port, rc);
                return rc;
            }

            /* disable policy engine for cascading ports with extended DSA tag packets */
            rc = cpssDxChPclPortIngressPolicyEnable(devNum, port, GT_FALSE);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "[%s:%d] - cpssDxChPclPortIngressPolicyEnable failed. devNum=%u port=%u rc[%d]"
                      , __func__, __LINE__, devNum, port, rc);
                return rc;
            }

            /* disable learning of new source MAC addresses for packets received
               on specified port*/
            rc = cpssDxChBrgFdbPortLearnStatusSet(devNum, port, GT_FALSE,
                                                  CPSS_LOCK_FRWRD_E);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "[%s:%d]  cpssDxChBrgFdbPortLearnStatusSet: ret[%d]"
                      , __func__, __LINE__, rc);
                return rc;
            }

            /* Disable New Address messages Storm Prevention */
            rc = cpssDxChBrgFdbNaStormPreventSet(devNum, port, GT_FALSE);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "[%s:%d]  cpssDxChBrgFdbNaStormPreventSet: ret[%d]"
                      , __func__, __LINE__, rc);
                return rc;
            }

            /* Disable forwarding a new mac address message to CPU */
            rc = cpssDxChBrgFdbNaToCpuPerPortSet(devNum, port, GT_FALSE);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "[%s:%d]  cpssDxChBrgFdbNaToCpuPerPortSet: ret[%d]"
                      , __func__, __LINE__, rc);
                return rc;
            }

            for (stpId=0; stpId < maxStp; stpId++)
            {
                rc = cpssDxChBrgStpStateSet(devNum, portIdx, stpId, CPSS_STP_DISABLED_E);
                if (rc !=GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "[%s:%d]  cpssDxChBrgStpStateSet: ret[%d]"
                          , __func__, __LINE__, rc);

                    return rc;
                }
            }


            rc = cpssDxChPortEnableSet(devNum, port, GT_TRUE);
            if (rc !=GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "[%s:%d]  cpssDxChPortEnableSet: ret[%d]"
                      , __func__, __LINE__, rc);

                return rc;
            }
            cpssOsPrintf("cpssDxChPortEnableSet devNum %d cpssCscdPort %d rc=%d\n", devNum,
                         port, rc);

            /*Make Link partner down*/
            //rc = cpssDxChPortSerdesTxEnableSet(devNum, port, cpssEnable);
            //cpssOsPrintf("cpssDxChPortSerdesTxEnableSet devNum %d cpssCscdPort %d rc=%d\n", devNum, cpssCscdPort, rc);

            rc = cpssDxChPortForceLinkDownEnableSet(devNum, port, GT_FALSE);
            if (rc !=GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "[%s:%d]  cpssDxChPortForceLinkDownEnableSet: ret[%d]"
                      , __func__, __LINE__, rc);

                return rc;
            }
            cpssOsPrintf("cpssDxChPortForceLinkDownEnableSet devNum %d cpssCscdPort %d rc=%d\n",
                         devNum, port, rc);

            /* Configure ports to be cascade ports */
            CPSS_PORTS_BMP_PORT_SET_MAC(&cscdTrunkBmp, port);
        }

        /* Cascade trunk-id must be less than
           PRV_CPSS_DXCH_PP_HW_INFO_TRUNK_MAC(_devNum).nonTrunkTable1NumEntries. In M0, this is 256. */
        rc = cpssDxChTrunkCascadeTrunkPortsSet(devNum, (GT_TRUNK_ID)cscdTrunkId,
                                               &cscdTrunkBmp);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[%s:%d] - cpssDxChTrunkCascadeTrunkPortsSet failed. devNum=%u cscdTrunkId=%u rc[%d]"
                  , __func__, __LINE__, devNum, cscdTrunkId, rc);
            return rc;
        }

        /* Configure the cascade map table */
        /*TODO this has to be handled in loop for multi devices*/
        targetDevNum = (devNum==B2B_FIRST_DEV)?B2B_SECOND_DEV:B2B_FIRST_DEV;

        rc = cpssDxChCfgHwDevNumGet(targetDevNum, &targetHwDevNum);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[%s:%d] - cpssDxChCfgHwDevNumGet failed. targetdevNum=%u rc[%d]"
                  , __func__, __LINE__, targetDevNum, rc);
            return rc;
        }

        rc = cpssDxChCfgHwDevNumGet(devNum, &sourceHwDevNum);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[%s:%d] - cpssDxChCfgHwDevNumGet failed. devNum=%u rc[%d]"
                  , __func__, __LINE__, devNum, rc);
            return rc;
        }

        rc = cpssDxChCscdDevMapTableGet(devNum, targetHwDevNum, sourceHwDevNum, 0, 0,
                                        &cpssCascadeLink,
                                        &currentSrcPortTrunkHashEn, &egressAttributesLocallyEn);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[%s:%d] - cpssDxChCscdDevMapTableGet failed. devNum=%u rc[%d]"
                  , __func__, __LINE__, devNum, rc);
            return rc;
        }

        cpssCascadeLink.linkNum = cscdTrunkId;
        cpssCascadeLink.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
        currentSrcPortTrunkHashEn = CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;
        egressAttributesLocallyEn = GT_FALSE;

        rc = cpssDxChCscdDevMapTableSet(devNum, targetHwDevNum, sourceHwDevNum, 0, 0,
                                        &cpssCascadeLink,
                                        currentSrcPortTrunkHashEn, egressAttributesLocallyEn);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[%s:%d] - cpssDxChCscdDevMapTableSet failed. devNum=%u rc[%d]"
                  , __func__, __LINE__, devNum, rc);
            return rc;
        }

#if 0
        /* Set Dsa ingress filter bit */
        rc = cpssDxChCscdDsaSrcDevFilterSet(devNum, GT_TRUE);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[%s:%d] - cpssDxChCscdDsaSrcDevFilterSet failed. devNum=%u  rc[%d]"
                  , __func__, __LINE__, devNum, rc);
            return rc;
        }
#endif
    }

    cpssHalCascadeMgrDumpTrunkPorts(0);

    return GT_OK;
}

void cpssHalCascadeMgrDumpTrunkPorts(int switchId)
{
    CPSS_TRUNK_MEMBER_STC members [CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    CPSS_PORTS_BMP_STC       cscdMem;
    GT_U32 numberOfMem, i;
    CPSS_TRUNK_TYPE_ENT trunkType;
    GT_U8                devNum;
    GT_TRUNK_ID             trunkId;
    GT_STATUS rc;


    for (devNum=0; devNum < cpssHalGetMaxDevices(); devNum++)
    {
        for (trunkId =0;
             trunkId <= PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->numberOfTrunks ; trunkId++)
        {
            rc = cpssDxChTrunkDbTrunkTypeGet(devNum, trunkId, &trunkType);
            if (rc)
            {
                continue;
            }
            switch (trunkType)
            {
                case    CPSS_TRUNK_TYPE_REGULAR_E:
                    cpssDxChTrunkTableEntryGet(devNum, trunkId, &numberOfMem, members);
                    printf("Regular Trunk: %d num of mem = %d\n", trunkId, numberOfMem);
                    for (i=0; i < numberOfMem; i++)
                    {
                        printf("%d/%d ", members[i].hwDevice, members[i].port);
                    }
                    printf("\n");
                    cpssDxChTrunkDbEnabledMembersGet(devNum, trunkId, &numberOfMem, members);
                    printf("num of enabled mem = %d\n", numberOfMem);
                    for (i=0; i < numberOfMem; i++)
                    {
                        printf("%d/%d ", members[i].hwDevice, members[i].port);
                    }
                    printf("\n");
                    break;
                case    CPSS_TRUNK_TYPE_CASCADE_E:
                    printf("Cascade Trunk: %d\n", trunkId);
                    cpssDxChTrunkCascadeTrunkPortsGet(devNum, trunkId, &cscdMem);
                    {
                        int portIdx;
                        int cpssCscdPort;
                        int cpssCscdMaxPorts;
                        XPS_CSCD_PORT_ITER(devNum, portIdx, cpssCscdPort, cpssCscdMaxPorts)
                        {
                            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&cscdMem, cpssCscdPort))
                            {
                                printf("%d/%d ", devNum, cpssCscdPort);
                            }
                        }
                    }
                    printf("\n");
                    break;
                default:
                    break;
            }
        }
    }
}

XP_STATUS cpssHalCascadeAddPortsToVlan(xpsVlan_t vlanId)
{
    int devNum;
    int portNum;
    int cpssCscdPort;
    int cpssCscdMaxPorts;
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT cpssTagType =
        CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
    GT_BOOL         tag = GT_TRUE;

    XPS_B2B_CSCD_PORTS_ITER(devNum, portNum, cpssCscdPort, cpssCscdMaxPorts)
    {
        if ((cpssDxChBrgVlanMemberAdd(devNum, vlanId, cpssCscdPort, tag,
                                      cpssTagType)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpss vlanmember add failed for devNum %d vlan %d port %d", devNum, vlanId,
                  cpssCscdPort);
        }
    }
    return XP_NO_ERR;
}


XP_STATUS cpssHalCascadeRemovePortsFromVlan(xpsVlan_t vlanId)
{
    int devNum;
    int portNum;
    int cpssCscdPort;
    int cpssCscdMaxPorts;
    XPS_B2B_CSCD_PORTS_ITER(devNum, portNum, cpssCscdPort, cpssCscdMaxPorts)
    {
        if ((cpssDxChBrgVlanPortDelete(devNum, vlanId, cpssCscdPort)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpss vlanmember add failed for devNum %d vlan %d port %d", devNum, vlanId,
                  cpssCscdPort);
        }
    }
    return XP_NO_ERR;
}

#if 0
XP_STATUS xpsCascadeMgrEnablePort(void)
{
    int devNum;
    int portNum;
    int cpssCscdPort;
    int cpssCscdMaxPorts;
    GT_STATUS rc;
    GT_BOOL cpssEnable, forceDown;
    cpssEnable = GT_TRUE;
    forceDown = GT_FALSE;


    XPS_B2B_CSCD_PORTS_ITER(devNum, portNum, cpssCscdPort, cpssCscdMaxPorts)
    {
        rc = cpssDxChPortEnableSet(devNum, cpssCscdPort, cpssEnable);
        cpssOsPrintf("cpssDxChPortEnableSet devNum %d cpssCscdPort %d rc=%d\n", devNum,
                     cpssCscdPort, rc);

        /*Make Link partner down*/
        //rc = cpssDxChPortSerdesTxEnableSet(devNum, cpssCscdPort, cpssEnable);
        //cpssOsPrintf("cpssDxChPortSerdesTxEnableSet devNum %d cpssCscdPort %d rc=%d\n", devNum, cpssCscdPort, rc);

        rc = cpssDxChPortForceLinkDownEnableSet(devNum, cpssCscdPort, forceDown);
        cpssOsPrintf("cpssDxChPortForceLinkDownEnableSet devNum %d cpssCscdPort %d rc=%d\n",
                     devNum, cpssCscdPort, rc);
    }
    return XP_NO_ERR;
}
#endif
